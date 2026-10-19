export module folia.math.core.tree.factory_types;
export import folia.math.core.tree.property;
export import folia.math.core.tree.child_array;
export import folia.math.util.option_types;
import std;

export namespace folia::math::tree {
using FactoryArgs = std::span<const std::any>;
using FactoryError = options::Exception;
struct FactoryNode {
    virtual ~FactoryNode() = default;
    virtual std::string kind() const = 0;
};
template<class Product> class FactoryServices;
template<class Product> using FactoryChildren = std::shared_ptr<ChildArray<Product>>;
template<class Product> using FactorySetChildren=std::function<void(FactoryChildren<Product>)>;
template<class Product> struct FactoryClass {
    std::type_index nativeType{typeid(void)};
    std::function<Product*(FactoryServices<Product>&,FactoryArgs)> construct;
    std::function<void(FactoryServices<Product>&,Product&,FactoryArgs,FactorySetChildren<Product>)> initialize;
    std::function<void(FactoryServices<Product>&,Product&,options::Value,options::Value,options::Value)> initializeSource;
    std::function<bool(const Product*)> isInstance;
    std::any staticData;
};
template<class Product> using FactoryClassRef = std::shared_ptr<FactoryClass<Product>>;
template<class Product> using FactoryClassMap = std::vector<std::pair<std::string,FactoryClassRef<Product>>>;
template<class Product> struct FactoryCreator {
    // Source/Context dispatch supplies the retained concrete service owner.
    // This callable captures only its class descriptor, never a factory `this`.
    std::function<Product*(FactoryServices<Product>&,FactoryArgs)> callWithServices;
    // Typed C++ convenience path. Implementations guard any borrowed factory
    // receiver with its per-instance weak lifetime token.
    std::function<Product*(FactoryArgs)> call;
};
template<class Product> using FactoryCreatorRef = std::shared_ptr<FactoryCreator<Product>>;

// One value cell per own property. The descriptor owns the exact Context value
// or accessor metadata for this key; creator identity can be shared by several
// keys without sharing their descriptors. Ordinary value copies copy the cell
// value; Context NativeRecordView writes the descriptor on the actual slot.
template<class Product> struct FactoryCreatorSlot {
    FactoryCreatorRef<Product> creator;
    std::optional<options::Descriptor> descriptor;
};

template<class Product> struct FactoryCreatorObject {
    // Stable concrete owner retained by public creatorView()/Wrappers aliases.
    // Context NativeRecordView binds this exact vector and each slot cell.
    // `Key` retains the exact UTF-16 string or Symbol identity. Native typed
    // factory calls use string keys, while Context ownKeys/define/delete can
    // also address symbols and lone UTF-16 code units without normalization.
    std::vector<std::pair<options::Key,FactoryCreatorSlot<Product>>> entries;

    // Pre-bind native representation only. Once a Context node record is
    // attached, its actual ObjectRef prototype is the sole prototype owner;
    // the bridge delegates reads/writes to that Context object.
    FactoryCreatorRef<Product> prototypeCreator;
    bool prototypeAssigned = false;
};
template<class Product> using FactoryCreatorView = std::shared_ptr<FactoryCreatorObject<Product>>;
template<class Product,class Source> class WrapperServices {
public:
    virtual ~WrapperServices() = default;
    virtual Product* wrap(Source* node,FactoryArgs args={})=0;
};
template<class Product> class FactoryServices {
public:
    virtual ~FactoryServices() = default;
    virtual std::weak_ptr<std::atomic_bool> executionLease()const noexcept{return {};}
    virtual Product* create(std::string_view kind,FactoryArgs args)=0;
    virtual void setNodeClass(std::string kind,FactoryClassRef<Product> type)=0;
    virtual FactoryClassRef<Product> getNodeClass(std::string_view kind) const=0;
    virtual void deleteNodeClass(std::string_view kind)=0;
    virtual bool nodeIsKind(const Product* node,std::string_view kind) const=0;
    virtual std::vector<std::string> getKinds() const=0;
    virtual Product* adopt(std::unique_ptr<Product> product)=0;
};
}
