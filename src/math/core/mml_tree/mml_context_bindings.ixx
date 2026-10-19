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
public:
    class SourceAllocationDomain {
        friend class MmlContextBindings;
        std::weak_ptr<MmlContextBindings> owner_;
        std::shared_ptr<MmlFactory> factory_;
        options::ObjectRef factoryIdentity_{};
        bool retired_=false;
        SourceAllocationDomain(std::weak_ptr<MmlContextBindings>,std::shared_ptr<MmlFactory>,
                               options::ObjectRef);
    public:
        SourceAllocationDomain(SourceAllocationDomain&&)=default;
        SourceAllocationDomain& operator=(SourceAllocationDomain&&)=delete;
        SourceAllocationDomain(const SourceAllocationDomain&)=delete;
        SourceAllocationDomain& operator=(const SourceAllocationDomain&)=delete;
        MmlFactory& factory()const;
    };
    class ActiveSourceAllocation {
        friend class MmlContextBindings;
        std::weak_ptr<MmlContextBindings> owner_;
        std::uint64_t activationId_=0;
        ActiveSourceAllocation(std::weak_ptr<MmlContextBindings>,std::uint64_t)noexcept;
    public:
        ActiveSourceAllocation(ActiveSourceAllocation&&)noexcept;
        ActiveSourceAllocation& operator=(ActiveSourceAllocation&&)=delete;
        ActiveSourceAllocation(const ActiveSourceAllocation&)=delete;
        ActiveSourceAllocation& operator=(const ActiveSourceAllocation&)=delete;
        ~ActiveSourceAllocation();
    };
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
    std::vector<std::pair<std::uint64_t,std::shared_ptr<MmlFactory>>> activeSourceBackends_;
    struct CapturedSourceObject {
        options::ObjectRef receiver{},object{};
        std::shared_ptr<PropertyStorage> storage;
        std::shared_ptr<MmlFactory> expectedOwner;
    };
    struct PrivateRendererProjectionCapture {
        RequestIdentityCapture token;
        std::vector<CapturedSourceObject> sourceObjects;
        std::optional<CapturedSourceObject> pendingBirth;
        bool attributesRetired=false;
    };
    struct ReceiverInitializationScope {
        std::uint64_t id=0;
        options::ObjectRef receiver{};
        std::shared_ptr<MmlFactory> expectedOwner;
        std::uint64_t captureId=0;
    };
    // Renderer is non-reentrant; one inline token avoids losing cleanup state
    // if allocating a registry node fails after Attributes has begun capture.
    std::optional<PrivateRendererProjectionCapture> privateRendererProjectionCapture_;
    std::optional<RequestIdentityCapture> activePrivateRendererProjectionCapture_;
    std::vector<ReceiverInitializationScope> receiverInitializationScopes_;
    std::uint64_t nextReceiverScopeId_=1;
    std::uint64_t nextSourceActivationId_=1;
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
    std::shared_ptr<MmlFactory> selectedSourceBackend()const;
    void deactivateSourceAllocation(std::uint64_t)noexcept;
    std::shared_ptr<MmlFactory> resolveProductOwner(options::ObjectRef,MmlNode&)const;
    void observeTreeOwnedObject(options::ObjectRef,options::Key,options::ObjectRef,
                                tree::TreeOwnedObjectHandoff&);
    std::uint64_t beginReceiverInitialization(options::ObjectRef,SourceReceiverProvenance,bool);
    void endReceiverInitialization(std::uint64_t)noexcept;
public:
    static std::shared_ptr<MmlContextBindings> create(options::Context&,
        util::RuntimeModules&,
        std::shared_ptr<MmlNodeContextModule>,
        std::shared_ptr<options::MapContextBindings>);
    ~MmlContextBindings();
    MmlContextBindings(const MmlContextBindings&)=delete;
    MmlContextBindings& operator=(const MmlContextBindings&)=delete;
    options::Context& context()const;
    std::array<std::size_t,3> nativeOwnerCountsForDiagnostics()const;
    RequestIdentityCapture beginPrivateRendererProjectionCapture();
    void retirePrivateRendererProjectionCapture(RequestIdentityCapture);
    NativeIdentityBindings<MmlNode*>& nodes();
    NativeIdentityBindings<MmlFactory*>& factories();
    NativeIdentityBindings<Children>& arrays();
    tree::TreeNodeContextBindings<MmlNode>& treeNodes();
    MmlNodeContextFields& nodeFields();
    MmlFactoryContextMethods& factoryMethods();
    MmlFactoryContextFields& factoryFields();
    MmlNodeContextMethods& nodeMethods();
    MmlSourceAllocator& sourceAllocator();
    SourceAllocationDomain makePrivateSourceAllocationDomain();
    void bindSourceAllocationDomain(SourceAllocationDomain&);
    void retireSourceAllocationDomain(SourceAllocationDomain&);
    ActiveSourceAllocation activateSourceAllocation(SourceAllocationDomain&);
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
