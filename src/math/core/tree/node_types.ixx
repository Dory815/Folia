// Root-frozen native interface/storage bridge for core/Tree/Node.ts.
export module folia.math.core.tree.node_types;
export import folia.math.core.tree.factory_types;
import std;

export namespace folia::math::tree {
template<class Product> using TreeVisitor = std::function<void(Product*,const std::any&)>;
template<class Product> class Node : public FactoryNode {
public:
    // One storage location for each field across normal and empty MML branches.
    FactoryServices<Product>& factory;
    Product* parent = nullptr;
    FactoryChildren<Product> childNodes = std::make_shared<std::vector<Product*>>();
    explicit Node(FactoryServices<Product>& service) : factory(service) {}
    Node(const Node&)=delete;Node& operator=(const Node&)=delete;
    Node(Node&&)=delete;Node& operator=(Node&&)=delete;
    virtual void setProperty(std::string_view name,Property value)=0;
    virtual Property getProperty(std::string_view name) const=0;
    virtual std::vector<std::string> getPropertyNames() const=0;
    virtual PropertyList getAllProperties() const=0;
    virtual void removeProperty(std::span<const std::string> names)=0;
    virtual bool isKind(std::string_view name) const=0;
    virtual void setChildren(FactoryChildren<Product> children)=0;
    virtual Product* appendChild(Product* child)=0;
    virtual Product* replaceChild(Product* newChild,Product* oldChild)=0;
    virtual Product* removeChild(Product* child)=0;
    virtual std::optional<std::size_t> childIndex(Product* child) const=0;
    // Generic Node ignores JS extra args; MML's optional keepIds overrides here.
    virtual Product* copy(bool keepIds=false)=0;
    virtual std::vector<Product*> findNodes(std::string_view kind)=0;
    virtual std::any walkTree(const TreeVisitor<Product>& visit,const std::any& data={})=0;
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
// Array.join suppresses recursion into an array already being joined.
template<class Product> class TreeJoinGuard {
    inline static thread_local std::vector<const std::vector<Product*>*> active_;
    bool entered_=false;
public:
    explicit TreeJoinGuard(const std::vector<Product*>* array){
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
