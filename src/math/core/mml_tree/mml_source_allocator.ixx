// Context-owned allocation seam for Source-selected MML descriptors.
// This service is deliberately narrower than MmlFactory: it does not select
// classes or implement TeX factory behavior.
export module folia.math.core.mml_tree.mml_source_allocator;
export import folia.math.core.mml_tree.mml_factory;
export import folia.math.core.tree.node_context_classes;
export import folia.math.util.native_identities;
import std;

export namespace folia::math::mml {
class MmlSourceAllocator {
public:
    using ResolveNode=std::function<MmlNode*(options::ObjectRef)>;
    // Canonical, idempotent Context binding callback; the backend's adopt
    // hook uses this same callback when a descriptor produces a new node.
    using BindProduct=std::function<void(options::ObjectRef,MmlNode&,
                                         const std::shared_ptr<MmlFactory>&)>;
    using InitializeTreeBase=std::function<void(options::ObjectRef,options::Value)>;
private:
    options::Context& context_;
    std::shared_ptr<MmlFactory> backend_;
    ResolveNode resolveNode_;
    BindProduct bindProduct_;
    InitializeTreeBase initializeTreeBase_;
    std::weak_ptr<std::atomic_bool> contextLease_;
    void requireActive()const;
public:
    MmlSourceAllocator(options::Context&,std::shared_ptr<MmlFactory>,
        std::weak_ptr<std::atomic_bool>,ResolveNode,BindProduct,InitializeTreeBase);
    MmlSourceAllocator(const MmlSourceAllocator&)=delete;
    MmlSourceAllocator& operator=(const MmlSourceAllocator&)=delete;
    MmlFactory& backend()const;
    void validate()const;
    // The caller passes the exact raw Source factory value. It is only used by
    // the Source base constructor and descriptor initializer; it is never
    // resolved to a native factory. The selected descriptor remains Source's
    // class-map result and the backend only supplies FactoryServices/adoption.
    MmlNode& initializeReceiver(options::ObjectRef receiver,
        const tree::FactoryClassRef<MmlNode>& descriptor,
        options::Value lexicalFactory,options::Value rawAttributes,
        options::Value rawChildren);
};
}
