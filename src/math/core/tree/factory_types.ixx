// Root-frozen native constructor-service contracts. No node/renderer algorithms.
export module folia.math.core.tree.factory_types;
export import folia.math.core.tree.property;
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
template<class Product> struct FactoryClass {
    // Constructor object identity is the shared record, never a kind string.
    std::type_index nativeType{typeid(void)};
    std::function<Product*(FactoryServices<Product>&,FactoryArgs)> construct;
    std::function<bool(const Product*)> isInstance;
    std::any staticData; // Later families supply their root-frozen typed metadata.
};
template<class Product> using FactoryClassRef = std::shared_ptr<FactoryClass<Product>>;
template<class Product> using FactoryClassMap = std::vector<std::pair<std::string,FactoryClassRef<Product>>>;
template<class Product> struct FactoryCreator {
    std::function<Product*(FactoryArgs)> call;
};
template<class Product> using FactoryCreatorRef = std::shared_ptr<FactoryCreator<Product>>;
template<class Product> struct FactoryCreatorObject {
    // Live ordinary-object own callable entries; distinct from Map registrations.
    std::vector<std::pair<std::string,FactoryCreatorRef<Product>>> entries;
    FactoryCreatorRef<Product> prototypeCreator;
    bool prototypeAssigned = false;
};
template<class Product> using FactoryCreatorView = std::shared_ptr<FactoryCreatorObject<Product>>;
template<class Product> using FactoryChildren = std::shared_ptr<std::vector<Product*>>;
template<class Product> class FactoryServices {
public:
    virtual ~FactoryServices() = default;
    virtual Product* create(std::string_view kind,FactoryArgs args)=0;
    virtual void setNodeClass(std::string kind,FactoryClassRef<Product> type)=0;
    virtual FactoryClassRef<Product> getNodeClass(std::string_view kind) const=0;
    virtual void deleteNodeClass(std::string_view kind)=0;
    virtual bool nodeIsKind(const Product* node,std::string_view kind) const=0;
    virtual std::vector<std::string> getKinds() const=0;
    // Factory/session owns complete and partially initialized native products.
    // Creators adopt before invoking JS-equivalent virtual initialization hooks.
    virtual Product* adopt(std::unique_ptr<Product> product)=0;
};
}
