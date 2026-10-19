// Root-frozen native option graph and host-property contract. No JS execution.
export module folia.math.util.option_types;
import std;

export namespace folia::math::options {
struct Undefined { bool operator==(const Undefined&) const = default; };
struct Null { bool operator==(const Null&) const = default; };
struct ObjectRef {
    std::uint64_t context = 0, index = 0;
    bool operator==(const ObjectRef&) const = default;
};
struct SymbolRef {
    std::uint64_t context = 0, index = 0;
    bool operator==(const SymbolRef&) const = default;
};
struct Value {
    using Data = std::variant<Undefined, Null, bool, double, std::u16string, ObjectRef, SymbolRef>;
    Data data = Undefined{};
    Value() = default;
    Value(Undefined v) : data(v) {}
    Value(Null v) : data(v) {}
    Value(bool v) : data(v) {}
    Value(double v) : data(v) {}
    Value(int v) : data(double(v)) {}
    Value(const char16_t* v) : data(std::u16string(v)) {}
    Value(std::u16string v) : data(std::move(v)) {}
    Value(ObjectRef v) : data(v) {}
    Value(SymbolRef v) : data(v) {}
    bool operator==(const Value&) const = default; // JS strict equality, including NaN != NaN.
};
// Explicit, non-observing edge reports supplied by native owners. Implementors
// report stored Context identities only; they must not invoke Source code,
// getters, accessors, or callbacks while tracing, and must not own the Context.
using ContextEdgeVisitor=std::function<void(const Value&)>;
class NativeOwnerTrace {
public:
    virtual ~NativeOwnerTrace()=default;
    // Return false when the owner cannot prove it exposed every raw edge.
    // Known edges may still be reported; Context must additionally pin it.
    virtual bool visitContextEdges(const ContextEdgeVisitor&)const=0;
};
using NativeOwnerTraceRef=std::shared_ptr<const NativeOwnerTrace>;
// A strongly attached, mutable native owner whose actual Context edges are
// reported through NativeOwnerTrace. The Record owns this separately from
// its opaque payload and releases it when retired.
class NativeAttachedOwner : public NativeOwnerTrace {
public:
    ~NativeAttachedOwner()override=default;
};
using NativeAttachedOwnerRef=std::shared_ptr<NativeAttachedOwner>;
// One backing owner covers both indexed and record facets of a native array.
enum class NativeOwnerPart : std::uint8_t { payload,callable,construct,nativeView,nativeDataField,attachedOwner };
struct OpaqueOwnerPin { ObjectRef object; NativeOwnerPart part; std::u16string detail; };
class ContextTraceReport {
public:
    class CompleteEdgeClosure final {
        std::uint64_t contextIdentity_;
        std::vector<ObjectRef> roots_;
        std::vector<ObjectRef> objects_;
        CompleteEdgeClosure(std::uint64_t contextIdentity,const std::vector<ObjectRef>& roots,
                            const std::vector<ObjectRef>& objects)
            :contextIdentity_(contextIdentity),roots_(roots),objects_(objects){}
        friend class ContextTraceReport;
    public:
        std::uint64_t contextIdentity()const noexcept{return contextIdentity_;}
        std::span<const ObjectRef> suppliedRoots()const noexcept{return roots_;}
        std::span<const ObjectRef> objects()const noexcept{return objects_;}
    };
private:
    std::uint64_t contextIdentity_;
    std::vector<ObjectRef> roots_;
    std::vector<ObjectRef> objects_;
    std::vector<SymbolRef> symbols_;
    std::vector<OpaqueOwnerPin> opaquePins_;
    bool valid_=true;
    ContextTraceReport(std::uint64_t contextIdentity,std::span<const ObjectRef> roots)
        :contextIdentity_(contextIdentity),roots_(roots.begin(),roots.end()){}
    friend class Context;
public:
    ContextTraceReport(const ContextTraceReport&)=default;
    ContextTraceReport(ContextTraceReport&& other)noexcept
        :contextIdentity_(other.contextIdentity_),roots_(std::move(other.roots_)),
         objects_(std::move(other.objects_)),symbols_(std::move(other.symbols_)),
         opaquePins_(std::move(other.opaquePins_)),valid_(other.valid_){other.valid_=false;}
    ContextTraceReport& operator=(const ContextTraceReport&)=delete;
    ContextTraceReport& operator=(ContextTraceReport&&)=delete;
    std::uint64_t contextIdentity()const noexcept{return contextIdentity_;}
    std::span<const ObjectRef> suppliedRoots()const noexcept{return roots_;}
    std::span<const ObjectRef> objects()const noexcept{return objects_;}
    std::span<const SymbolRef> symbols()const noexcept{return symbols_;}
    std::span<const OpaqueOwnerPin> opaquePins()const noexcept{return opaquePins_;}
    bool isValid()const noexcept{return valid_;}
    bool hasOpaquePins()const noexcept{return !opaquePins_.empty();}
    // This proves only complete raw-edge observation from suppliedRoots().
    // Retirement still requires a separately validated complete RootSet and
    // live-owner/epoch policy; this token is not authorization to retire.
    std::optional<CompleteEdgeClosure> completeEdgeClosure()const{
        if(!valid_||hasOpaquePins())return std::nullopt;
        return CompleteEdgeClosure(contextIdentity_,roots_,objects_);
    }
};
struct Key {
    std::variant<std::u16string, SymbolRef> data;
    Key(const char16_t* v) : data(std::u16string(v)) {}
    Key(std::u16string v) : data(std::move(v)) {}
    Key(SymbolRef v) : data(v) {}
    bool operator==(const Key&) const = default;
};
enum class ObjectKind { object, array, function, instance };
enum class PrimitiveHint { defaultHint, numberHint, stringHint };
struct Descriptor {
    Value value;
    bool writable = true, enumerable = true, configurable = true, accessor = false;
    std::optional<ObjectRef> getter, setter;
    static Descriptor data(Value v, bool w=true, bool e=true, bool c=true) {
        return {std::move(v),w,e,c,false,{},{}};
    }
    static Descriptor access(std::optional<ObjectRef> g, std::optional<ObjectRef> s,
                             bool e=true, bool c=true) {
        return {Undefined{},false,e,c,true,g,s};
    }
};
// A live, owner-preserving array storage view. Context keeps ordinary array
// identity/prototype/named properties; this interface is the sole owner of
// indexed descriptors and length for arrays created with nativeArray().
// Providers must distinguish an absent descriptor (hole) from a present
// descriptor whose value is Undefined.
class NativeIndexedView {
public:
    virtual ~NativeIndexedView()=default;
    virtual std::uint32_t length()const=0;
    virtual Descriptor lengthDescriptor()const=0;
    virtual std::optional<Descriptor> indexDescriptor(std::uint32_t index)const=0;
    virtual std::vector<std::uint32_t> ownIndexKeys()const=0;
    virtual void defineIndex(std::uint32_t index,Descriptor descriptor)=0;
    virtual void defineLength(Descriptor descriptor)=0;
    virtual void deleteIndex(std::uint32_t index)=0;
};
using NativeIndexedViewRef=std::shared_ptr<NativeIndexedView>;
// A live ordinary-record view. Context owns identity/prototype dispatch and
// delegates own-key descriptors and mutations to the one native record
// storage. Providers must preserve key order and distinguish absent keys
// from own properties whose value is Undefined.
class NativeRecordView {
public:
    virtual ~NativeRecordView()=default;
    virtual std::optional<Descriptor> ownDescriptor(const Key& key)const=0;
    // Object.assign checks [[GetOwnProperty]].enumerable before performing its
    // separate [[Get]]. This query must not read the descriptor's value.
    virtual bool ownPropertyEnumerable(const Key& key)const=0;
    virtual std::vector<Key> ownKeys()const=0;
    virtual void defineOwn(const Key& key,Descriptor descriptor)=0;
    virtual void deleteOwn(const Key& key)=0;
    virtual std::optional<ObjectRef> prototype()const=0;
    virtual void setPrototype(std::optional<ObjectRef> prototype)=0;
};
using NativeRecordViewRef=std::shared_ptr<NativeRecordView>;
struct Exception : std::exception {
    std::string name;
    std::u16string message;
    Exception(std::string n, std::u16string m) : name(std::move(n)),message(std::move(m)) {}
    const char* what() const noexcept override { return "Native MathJax option exception"; }
};
class Context;
// Internal native data-cell view. Its value lives only in the real owner;
// this host binding is not a JavaScript accessor or a second value store.
class NativeDataField {
    std::shared_ptr<void> owner_;
    std::function<Value()> read_;
    std::function<void(Value)> write_;
    std::function<void()> remove_;
    NativeOwnerTraceRef edgeTrace_;
    bool active_=true;
public:
    NativeDataField(std::shared_ptr<void> owner,std::function<Value()> read,
                    std::function<void(Value)> write,std::function<void()> remove={},
                    NativeOwnerTraceRef edgeTrace={})
        :owner_(std::move(owner)),read_(std::move(read)),write_(std::move(write)),remove_(std::move(remove)),
         edgeTrace_(std::move(edgeTrace)){
        if(!owner_||!read_||!write_)throw Exception("TypeError",u"Native data field requires its actual owner and read/write operations");
    }
    bool hasEdgeTrace()const noexcept{return static_cast<bool>(edgeTrace_);}
    bool visitContextEdges(const ContextEdgeVisitor& visitor)const{return edgeTrace_&&edgeTrace_->visitContextEdges(visitor);}
    Value read()const{
        if(!active_)throw Exception("Unimplemented",u"Native data field has been released");
        return read_();
    }
    void write(Value value){
        if(!active_)throw Exception("Unimplemented",u"Native data field has been released");
        write_(std::move(value));
    }
    void remove(){
        if(!remove_)throw Exception("Unimplemented",u"Native data field has no Source deletion operation");
        remove_();
    }
    // Runtime teardown may invalidate a bound field while the Context handle
    // remains alive. The owner-specific callbacks must use weak captures.
    void releaseOwner()noexcept{active_=false;owner_.reset();}
};
using NativeDataFieldRef=std::shared_ptr<NativeDataField>;
using NativeCall = std::function<Value(Context&, Value, std::span<const Value>)>;
// A registered constructor allocates and returns the actual native instance.
// It does not receive a substitute plain object as an artificial `this`.
using NativeConstruct = std::function<ObjectRef(Context&, std::span<const Value>)>;
struct Intrinsics {
    SymbolRef symbolSpecies, symbolIterator, symbolIsConcatSpreadable, symbolToPrimitive, symbolToStringTag,
              symbolSplit, symbolMatch;
    ObjectRef objectPrototype, functionPrototype, arrayPrototype, iteratorPrototype,
              arrayIteratorPrototype, stringIteratorPrototype, mapIteratorPrototype,
              expandablePrototype;
    ObjectRef numberPrototype, booleanPrototype, stringPrototype, symbolPrototype;
    ObjectRef objectConstructor, functionConstructor, arrayConstructor, expandableConstructor;
    ObjectRef numberConstructor, booleanConstructor, stringConstructor, symbolConstructor;
};
struct Record;
struct ContextState;
bool truthy(const Value& value);
bool sameValue(const Value& a, const Value& b);

// Handles never own other handles; one Context owns all records. Contexts are
// engine/session-local and cannot be copied, moved or mixed. Native callbacks
// capture refs or borrowed services, never an owning Context reference.
class Context {
    std::unique_ptr<ContextState> state_;
    Record& record(ObjectRef object) const;
    Record* recordIfLive(ObjectRef object)const;
    ObjectRef allocate(ObjectKind kind, std::optional<ObjectRef> prototype);
    void setImpl(Value object,const Key& key,Value value,NativeDataFieldRef fieldForAbsent={});
    std::optional<Descriptor> rawDescriptor(Value,const Key&)const;
    NativeDataFieldRef nativeDataField(ObjectRef,const Key&)const;
    NativeDataFieldRef declaredNativeDataField(ObjectRef,const Key&)const;
    void initializeArrayIntrinsics();
public:
    Context();
    ~Context();
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    struct AllocationCounts { std::size_t objectSlots=0; std::size_t symbolSlots=0; };
    // Diagnostic only: total allocated slots, not live or reclaimable objects.
    AllocationCounts allocationCountsForDiagnostics() const noexcept;
    const Intrinsics& intrinsics() const;
    ObjectRef object();
    // Source ToObject conversion used when a property operation must retain
    // its receiver identity or box a primitive receiver.
    ObjectRef toObject(Value value);
    ObjectRef nullObject();
    ObjectRef objectWithPrototype(ObjectRef prototype);
    ObjectRef array(std::uint32_t length=0);
    ObjectRef nativeArray(NativeIndexedViewRef view);
    ObjectRef nativeRecord(NativeRecordViewRef view,
                           std::optional<ObjectRef> prototype={});
    ObjectRef function(std::u16string label, NativeCall callable={}, std::any payload={});
    ObjectRef constructor(std::u16string label, NativeCall callable={}, std::any payload={});
    ObjectRef nativeConstructor(std::u16string label, NativeConstruct construct,
                                NativeCall callable={}, std::any payload={});
    ObjectRef instance(ObjectRef constructor, std::any payload={});
    // Mutable access invalidates any installed payload edge trace before exposing an alias.
    std::any& payload(ObjectRef object);
    const std::any& payload(ObjectRef object) const;
    // Replace without exposing an alias. If a legacy mutable alias was ever
    // issued, this remains safe because payload tracing is permanently barred.
    void setPayload(ObjectRef object,std::any payload);
    // Release every edge owned by this Context record while retaining its
    // slot as a stale-handle tombstone. Callers must first prove that no live
    // Source/native root may still use this handle; this does not recycle IDs.
    void retireObjectRecord(ObjectRef object);
    ObjectKind kind(ObjectRef object) const;
    SymbolRef symbol(std::optional<std::u16string> description=std::nullopt);
    std::u16string symbolString(SymbolRef symbol) const;
    Value toPrimitive(Value value,PrimitiveHint hint=PrimitiveHint::defaultHint);
    std::u16string keyString(const Key& key) const;
    std::optional<ObjectRef> prototype(ObjectRef object) const;
    void setPrototype(ObjectRef object, std::optional<ObjectRef> prototype);
    std::vector<Key> ownKeys(Value object) const;
    std::vector<std::u16string> enumerableStringKeys(Value object) const;
    std::vector<SymbolRef> ownSymbols(Value object) const;
    // Descriptor-only enumerable check; does not read native field data.
    bool ownPropertyEnumerable(Value object,const Key& key)const;
    std::optional<Descriptor> descriptor(Value object, const Key& key) const;
    bool hasOwn(Value object, const Key& key) const;
    bool has(Value object, const Key& key) const;
    Value get(Value object, const Key& key);
    void set(Value object, const Key& key, Value value);
    // Perform a Source constructor-style own-field assignment while binding
    // the same native cell as the property's value authority. Existing own
    // descriptors still use ordinary assignment; when the own property was
    // deleted, the retained native cell is restored rather than shadowed.
    // Returns true when this assignment made the native cell the data-value
    // authority. Accessors retain their Source owner.
    bool assignNativeData(ObjectRef object,const Key& key,NativeDataFieldRef field,Value value);
    // Declare an actual owner-backed cell for an ordinary own data field that
    // is absent initially. The first successful own data assignment or data
    // definition activates the cell without changing Source property order.
    void declareNativeData(ObjectRef object,const Key& key,NativeDataFieldRef field);
    void define(ObjectRef object, const Key& key, Descriptor descriptor);
    // Install at the Source own-field initialization point. Metadata belongs
    // to the ordinary record; all data reads/writes reach the native owner.
    void bindData(ObjectRef object,const Key& key,NativeDataFieldRef,
                  bool writable=true,bool enumerable=true,bool configurable=true);
    void erase(ObjectRef object, const Key& key);
    // Unique per owner part. Duplicate registration throws. NativeView traces
    // enumerate the union of indexed/record facets; missing hooks pin.
    void registerNativeOwnerTrace(ObjectRef,NativeOwnerPart,NativeOwnerTraceRef);
    void unregisterNativeOwnerTrace(ObjectRef,NativeOwnerPart);
    NativeAttachedOwnerRef attachedOwner(ObjectRef)const;
    void attachOwner(ObjectRef,NativeAttachedOwnerRef);
    bool clearAttachedOwner(ObjectRef,const NativeAttachedOwner*);
    // Fresh observational reachability report only; no promotion or reclaim.
    ContextTraceReport traceReachable(std::span<const ObjectRef> roots)const;
    std::shared_ptr<void> getOrCreateServiceImpl(std::u16string identity,
        std::type_index type,std::function<std::shared_ptr<void>()> create);
    template<class T,class Factory>
    std::shared_ptr<T> getOrCreateService(std::u16string identity,Factory&& create){
        auto erased=[factory=std::forward<Factory>(create)]()mutable->std::shared_ptr<void>{
            auto owner=std::invoke(std::move(factory));
            if(!owner)throw Exception("TypeError",u"Context service initializer returned no owner");
            return std::static_pointer_cast<void>(std::move(owner));
        };
        return std::static_pointer_cast<T>(getOrCreateServiceImpl(
            std::move(identity),std::type_index(typeid(T)),std::move(erased)));
    }
    Value call(ObjectRef function, Value receiver, std::span<const Value> arguments={});
    Value callMember(Value function, Value receiver, std::span<const Value> arguments={},
                     std::u16string_view expression=u"value.call");
    bool isConstructor(ObjectRef constructor) const;
    ObjectRef construct(ObjectRef constructor, std::span<const Value> arguments={});
    // Numeric conversion may invoke live user getters/methods on this Context.
    double toNumber(const Value& value);
    // These implement the pinned TypeScript helper protocol: getIterator is
    // __values, iteratorNext is its dynamic next() step, and readIterable is
    // __read with its bounded-read and IteratorClose semantics.
    Value getIterator(Value iterable);
    Value iteratorNext(Value iterator);
    Value readIterable(Value iterable, std::optional<std::uint32_t> count=std::nullopt);
    void forEachIterable(Value iterable,const std::function<void(Value)>& body);
    ObjectRef nativeIterator(std::function<Value(Context&)> next);
    ObjectRef descriptorObject(const Descriptor& descriptor);
    Descriptor descriptorFromObject(Value object);
    std::uint32_t arrayLength(ObjectRef object) const;
    std::vector<Value> arrayValues(ObjectRef object);
    std::ptrdiff_t arrayIndexOf(ObjectRef object, const Value& needle);
    ObjectRef arrayFilter(ObjectRef object, const std::function<bool(Value)>& predicate);
    void arrayForEach(ObjectRef object, Value callback, Value thisArgument=Undefined{});
    ObjectRef arrayMap(ObjectRef object, Value callback, Value thisArgument=Undefined{});
    ObjectRef arraySlice(ObjectRef object, Value start=Undefined{}, Value end=Undefined{});
    ObjectRef arrayConcat(Value receiver, std::span<const Value> arguments={});
    ObjectRef arrayReverse(ObjectRef object);
    bool arrayIncludes(ObjectRef object, Value search, Value fromIndex=Undefined{});
    std::uint32_t arrayPush(ObjectRef object, std::span<const Value> values);
    Value arrayPop(ObjectRef object);
    Value arrayShift(ObjectRef object);
    ObjectRef arraySplice(ObjectRef object, Value start, std::optional<Value> deleteCount,
                          std::span<const Value> items={});
    std::u16string arrayJoin(ObjectRef object, Value separator=Undefined{});
    ObjectRef arrayFill(ObjectRef object, Value value, Value start=Undefined{}, Value end=Undefined{});
    Value arrayFind(ObjectRef object, Value callback, Value thisArgument=Undefined{});
    Value arrayFindIndex(Value receiver, Value callback, Value thisArgument=Undefined{});
};

// Ordinary ECMAScript instanceof prototype-chain semantics for Context values.
bool ordinaryInstanceOf(Context& context,const Value& value,const Value& constructor);
}
