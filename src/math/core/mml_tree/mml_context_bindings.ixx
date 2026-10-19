// The single Context-canonical owner of Source MML identities and bindings.
// This module assembles narrow adapters; it owns no TeX or layout algorithms.
export module folia.math.core.mml_tree.mml_context_bindings;
export import folia.math.core.mml_tree.mml_node_context_module;
export import folia.math.core.mml_tree.mml_node_context_fields;
export import folia.math.core.mml_tree.mml_factory_context_fields;
export import folia.math.core.mml_tree.mml_factory_context_methods;
export import folia.math.core.mml_tree.mml_source_allocator;
export import folia.math.core.tree.node_context_bindings;
export import folia.math.util.runtime_modules;
export import folia.math.util.map_bindings;
import std;

export namespace folia::math::mml {
class MmlContextBindings:public std::enable_shared_from_this<MmlContextBindings> {
private:
    options::Context& context_;
    std::shared_ptr<MmlNodeContextModule> nodeModule_;
    std::shared_ptr<options::MapContextBindings> maps_;
    std::shared_ptr<std::atomic_bool> lease_;
    NativeIdentityBindings<MmlNode*> nodes_;
    NativeIdentityBindings<MmlFactory*> factories_;
    NativeIdentityBindings<Children> arrays_;
    std::unique_ptr<tree::TreeNodeContextBindings<MmlNode>> treeNodes_;
    std::unique_ptr<MmlNodeContextFields> nodeFields_;
    std::unique_ptr<MmlFactoryContextMethods> factoryMethods_;
    std::unique_ptr<MmlFactoryContextFields> factoryFields_;
    std::unique_ptr<MmlNodeContextMethods> nodeMethods_;
    std::shared_ptr<MmlFactory> sourceBackend_;
    std::shared_ptr<MmlSourceAllocator> sourceAllocator_;
    std::shared_ptr<tree::FactoryContextBaseModule> factoryBaseModule_;
    std::vector<std::weak_ptr<MmlFactory>> boundFactories_;
    struct NodeBindingState { bool fields=false; bool methods=false; };
    struct FactoryBindingState { bool methods=false; bool fields=false; bool binder=false; };
    bool initialized_=false;

    MmlContextBindings(options::Context&,
                       std::shared_ptr<MmlNodeContextModule>,
                       std::shared_ptr<options::MapContextBindings>);
    void initialize(util::RuntimeModules&);
    void rollbackInitialization() noexcept;
public:
    static std::shared_ptr<MmlContextBindings> create(options::Context&,
        util::RuntimeModules&,
        std::shared_ptr<MmlNodeContextModule>,
        std::shared_ptr<options::MapContextBindings>);
    ~MmlContextBindings();
    MmlContextBindings(const MmlContextBindings&)=delete;
    MmlContextBindings& operator=(const MmlContextBindings&)=delete;
    options::Context& context()const;
    NativeIdentityBindings<MmlNode*>& nodes();
    NativeIdentityBindings<MmlFactory*>& factories();
    NativeIdentityBindings<Children>& arrays();
    tree::TreeNodeContextBindings<MmlNode>& treeNodes();
    MmlNodeContextFields& nodeFields();
    MmlFactoryContextMethods& factoryMethods();
    MmlFactoryContextFields& factoryFields();
    MmlNodeContextMethods& nodeMethods();
    MmlSourceAllocator& sourceAllocator();
    options::ObjectRef bindFactory(std::shared_ptr<MmlFactory>);
    std::shared_ptr<MmlFactory> resolveFactory(options::ObjectRef)const;
    MmlNode& bindNode(options::ObjectRef,MmlNode&,
                      const std::shared_ptr<MmlFactory>&);
    MmlNode& resolveNode(options::ObjectRef)const;
    bool nodeBindingComplete(options::ObjectRef)const;
    std::optional<options::ObjectRef> findNode(MmlNode&)const;
    options::ObjectRef bindChildren(Children);
    Children resolveChildren(options::ObjectRef)const;
};
}
