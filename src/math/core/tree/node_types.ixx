// Root-frozen native interface/storage bridge for core/Tree/Node.ts.
export module folia.math.core.tree.node_types;
export import folia.math.core.tree.factory_types;
import std;

export namespace folia::math::tree {
template<class Product> using TreeVisitor = std::function<void(Product*,const std::any&)>;
template<class Product> class Node : public FactoryNode {
    ChildValue<Product> parent_ = PropertyNull{};
    std::function<ChildValue<Product>()> parentReader_;
    std::function<void(ChildValue<Product>)> parentWriter_;
    std::function<FactoryChildren<Product>()> childReader_;
    std::function<void(FactoryChildren<Product>)> childWriter_;
    struct NativeFactoryRef{FactoryServices<Product>* service;};
    struct SourceFactoryRef{
        options::Value value;
        std::function<options::Value()> read;
        std::function<FactoryServices<Product>*(const options::Value&)> resolve;
    };
    std::variant<NativeFactoryRef,SourceFactoryRef> factoryRef_;
public:
    // One storage location for each field across normal and empty MML branches.
    FactoryChildren<Product> childNodes = std::make_shared<ChildArray<Product>>();
    explicit Node(FactoryServices<Product>& service) : factoryRef_(NativeFactoryRef{&service}) {}
    Node(const Node&)=delete;Node& operator=(const Node&)=delete;
    Node(Node&&)=delete;Node& operator=(Node&&)=delete;
    virtual void setProperty(std::string_view name,Property value)=0;
    virtual Property getProperty(std::string_view name) const=0;
    virtual std::vector<std::string> getPropertyNames() const=0;
    virtual PropertyList getAllProperties() const=0;
    virtual void removeProperty(std::span<const std::string> names)=0;
    virtual bool isKind(std::string_view name) const=0;
    virtual void setChildren(FactoryChildren<Product> children)=0;
    // All shared Tree algorithms read these hooks at the Source field access
    // point. Native nodes return their actual typed owners; Context-backed
    // nodes return views of the original descriptor-backed fields.
    virtual FactoryChildren<Product> childArrayOwner()const{return childReader_?childReader_():childNodes;}
    virtual void childArrayOwner(FactoryChildren<Product> owner){if(childWriter_)childWriter_(std::move(owner));else childNodes=std::move(owner);}
    void sourceParentAccessors(std::function<ChildValue<Product>()> read,
                               std::function<void(ChildValue<Product>)> write){
        parentReader_=std::move(read);parentWriter_=std::move(write);
    }
    // NativeDataField callbacks bypass the typed Context accessors.
    ChildValue<Product> sourceParentCell()const{return parent_;}
    void setSourceParentCell(ChildValue<Product> value){parent_=std::move(value);}
    void sourceChildArrayAccessors(std::function<FactoryChildren<Product>()> read,
                                   std::function<void(FactoryChildren<Product>)> write){
        childReader_=std::move(read);childWriter_=std::move(write);
    }
    virtual PropertyList propertyOwner()const=0;
    virtual void propertyOwner(PropertyList)=0;
    virtual FactoryServices<Product>& factoryOwner()const{
        if(const auto* source=std::get_if<SourceFactoryRef>(&factoryRef_)){
            if(!source->resolve)throw options::Exception("TypeError",u"Source factory field has no resolver");
            const auto value=source->read?source->read():source->value;
            auto* resolved=source->resolve(value);
            if(!resolved)throw options::Exception("TypeError",u"Source factory is not usable by native Tree operations");
            return *resolved;
        }
        return *std::get<NativeFactoryRef>(factoryRef_).service;
    }
    void sourceFactoryOwner(options::Value value,
        std::function<FactoryServices<Product>*(const options::Value&)> resolver){
        factoryRef_=SourceFactoryRef{std::move(value),{},std::move(resolver)};
    }
    void sourceFactoryReader(std::function<options::Value()> read,
        std::function<FactoryServices<Product>*(const options::Value&)> resolver){
        factoryRef_=SourceFactoryRef{options::Value(options::Undefined{}),std::move(read),std::move(resolver)};
    }
    std::optional<options::Value> sourceFactoryValue()const{
        if(const auto* source=std::get_if<SourceFactoryRef>(&factoryRef_))return source->read?source->read():source->value;
        return std::nullopt;
    }
    virtual options::Value sourceValue()const{
        throw options::Exception("Unimplemented",u"Native Tree node has no Source identity");
    }
    virtual ChildValue<Product> getParent() const {return parentReader_?parentReader_():parent_;}
    virtual void setParent(ChildValue<Product> value) {if(parentWriter_)parentWriter_(std::move(value));else parent_=std::move(value);}
    virtual ChildValue<Product> appendChild(ChildValue<Product> child)=0;
    virtual ChildValue<Product> replaceChild(ChildValue<Product> newChild,ChildValue<Product> oldChild)=0;
    virtual ChildValue<Product> removeChild(ChildValue<Product> child)=0;
    virtual std::optional<std::size_t> childIndex(ChildValue<Product> child) const=0;
    // Generic Node ignores JS extra args; MML's optional keepIds overrides here.
    virtual Product* copy(bool keepIds=false)=0;
    virtual std::vector<Product*> findNodes(std::string_view kind)=0;
    virtual std::any walkTree(const TreeVisitor<Product>& visit,const std::any& data={})=0;
    virtual std::any walkTreeRaw(const TreeRawVisitor<Product>& visit,const std::any& data={},
                                 const std::any& visitorIdentity={})=0;
    virtual std::string toString() const=0;
};
// Native stack resource bound, shared by recursively dispatched generic methods.
// It is not a syntax cutoff; later MML/renderer packets must share this service.
template<class Product> class TreeCallGuard {
    inline static thread_local std::size_t depth_=0;
public:
    inline static constexpr std::size_t limit=512;
    TreeCallGuard(){if(depth_>=limit)throw FactoryError("RangeError",u"Native tree call budget exceeded");++depth_;}
    ~TreeCallGuard(){--depth_;}
    TreeCallGuard(const TreeCallGuard&)=delete;
    TreeCallGuard& operator=(const TreeCallGuard&)=delete;
};
// Iterative parent walks consume the same resource budget as recursive calls.
template<class Product> class TreeWalkBudget {
    std::vector<std::unique_ptr<TreeCallGuard<Product>>> steps_;
public:
    void step(){steps_.push_back(std::make_unique<TreeCallGuard<Product>>());}
};
// Array.join suppresses recursion into an array already being joined.
template<class Product> class TreeJoinGuard {
    inline static thread_local std::vector<const ChildArray<Product>*> active_;
    bool entered_=false;
public:
    explicit TreeJoinGuard(const ChildArray<Product>* array){
        if(std::find(active_.begin(),active_.end(),array)==active_.end()){
            active_.push_back(array);entered_=true;
        }
    }
    ~TreeJoinGuard(){if(entered_)active_.pop_back();}
    bool entered()const{return entered_;}
    TreeJoinGuard(const TreeJoinGuard&)=delete;
    TreeJoinGuard& operator=(const TreeJoinGuard&)=delete;
};
}
