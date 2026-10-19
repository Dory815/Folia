// Context-owned MML Source algorithms and canonical prototype set.
export module folia.math.core.mml_tree.mml_node_context_module;
export import folia.math.core.mml_tree.mml_node_context_methods;
export import folia.math.core.mml_tree.attributes_bindings;
export import folia.math.core.tree.node_context_classes;
export import folia.math.core.tree.node_context_bindings;
export import folia.math.util.option_functions;
import std;

export namespace folia::math::mml {
class MmlNodeContextModule {
    options::Context& context_;
    std::shared_ptr<std::atomic_bool> lease_;
    OptionFunctionBindings functions_;
    ContextPropertyBindings properties_;
    AttributesContextBindings attributes_;
    std::unique_ptr<tree::TreeNodeContextClasses<MmlNode>> treeClasses_;
    std::unique_ptr<MmlNodeContextMethods> sourceMethods_;
    tree::TreeNodeContextBindings<MmlNode>* attachedBindings_=nullptr;
    void active()const;
public:
    explicit MmlNodeContextModule(options::Context&);
    ~MmlNodeContextModule();
    MmlNodeContextModule(const MmlNodeContextModule&)=delete;
    MmlNodeContextModule& operator=(const MmlNodeContextModule&)=delete;
    options::Context& context()const;
    ContextPropertyBindings& properties();
    AttributesContextBindings& attributes();
    tree::TreeNodeContextClasses<MmlNode>& treeClasses();
    void initializeTreeClasses(tree::TreeNodeContextBindings<MmlNode>&,std::weak_ptr<std::atomic_bool>,
        tree::TreeOwnedObjectCreated observer={});
    void detachContextBindings(tree::TreeNodeContextBindings<MmlNode>&) noexcept;
    MmlNodeContextPrototypes initializeSourceMethods(
        NativeIdentityBindings<MmlNode*>&,
        tree::TreeNodeContextBindings<MmlNode>&,
        std::weak_ptr<std::atomic_bool>,
        std::function<bool(options::Value)> isAbstractMmlNode,
        std::function<bool(options::Value)> isTextNode);
    std::weak_ptr<std::atomic_bool> lease()const;
};
}
