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
struct Key {
    std::variant<std::u16string, SymbolRef> data;
    Key(const char16_t* v) : data(std::u16string(v)) {}
    Key(std::u16string v) : data(std::move(v)) {}
    Key(SymbolRef v) : data(v) {}
    bool operator==(const Key&) const = default;
};
enum class ObjectKind { object, array, function, instance };
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
struct Exception : std::exception {
    std::string name;
    std::u16string message;
    Exception(std::string n, std::u16string m) : name(std::move(n)),message(std::move(m)) {}
    const char* what() const noexcept override { return "Native MathJax option exception"; }
};
class Context;
using NativeCall = std::function<Value(Context&, Value, std::span<const Value>)>;
struct Intrinsics {
    ObjectRef objectPrototype, functionPrototype, arrayPrototype, expandablePrototype;
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
    ObjectRef allocate(ObjectKind kind, std::optional<ObjectRef> prototype);
    void initializeArrayIntrinsics();
public:
    Context();
    ~Context();
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    const Intrinsics& intrinsics() const;
    ObjectRef object();
    ObjectRef nullObject();
    ObjectRef objectWithPrototype(ObjectRef prototype);
    ObjectRef array(std::uint32_t length=0);
    ObjectRef function(std::u16string label, NativeCall callable={}, std::any payload={});
    ObjectRef constructor(std::u16string label, NativeCall callable={}, std::any payload={});
    ObjectRef instance(ObjectRef constructor, std::any payload={});
    std::any& payload(ObjectRef object);
    ObjectKind kind(ObjectRef object) const;
    SymbolRef symbol(std::optional<std::u16string> description=std::nullopt);
    std::u16string symbolString(SymbolRef symbol) const;
    std::u16string keyString(const Key& key) const;
    std::optional<ObjectRef> prototype(ObjectRef object) const;
    void setPrototype(ObjectRef object, std::optional<ObjectRef> prototype);
    std::vector<Key> ownKeys(Value object) const;
    std::vector<std::u16string> enumerableStringKeys(Value object) const;
    std::vector<SymbolRef> ownSymbols(Value object) const;
    std::optional<Descriptor> descriptor(Value object, const Key& key) const;
    bool hasOwn(Value object, const Key& key) const;
    bool has(Value object, const Key& key) const;
    Value get(Value object, const Key& key);
    void set(Value object, const Key& key, Value value);
    void define(ObjectRef object, const Key& key, Descriptor descriptor);
    void erase(ObjectRef object, const Key& key);
    Value call(ObjectRef function, Value receiver, std::span<const Value> arguments={});
    ObjectRef descriptorObject(const Descriptor& descriptor);
    Descriptor descriptorFromObject(Value object);
    std::uint32_t arrayLength(ObjectRef object) const;
    std::vector<Value> arrayValues(ObjectRef object);
    std::ptrdiff_t arrayIndexOf(ObjectRef object, const Value& needle);
    ObjectRef arrayFilter(ObjectRef object, const std::function<bool(Value)>& predicate);
};
}
