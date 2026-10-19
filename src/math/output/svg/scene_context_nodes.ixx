/*************************************************************
 * Canonical Context projection for mutable output Scene nodes.
 * The payload retains the existing Scene node; it owns no DOM attributes or
 * child list of its own.
 *************************************************************/
export module folia.math.output.svg.scene_context_nodes;
export import folia.math.output.scene.foundation;
export import folia.math.adaptors.lite.lite_module;
export import folia.math.util.context_property_bindings;
import std;

export namespace folia::math::output::svg {
class SceneContextNodes {
    struct State;
    std::shared_ptr<State> state_;
public:
    SceneContextNodes(options::Context&,std::u16string constructorName);
    ~SceneContextNodes();
    SceneContextNodes(const SceneContextNodes&)=delete;
    SceneContextNodes& operator=(const SceneContextNodes&)=delete;

    struct RequestMark { const void* owner=nullptr;std::uint64_t generation=0; };
    struct RequestOwnerCounts { std::size_t handles=0,sourceIdentities=0,sourceViews=0; };
    std::size_t handleCountForDiagnostics()const noexcept;
    RequestOwnerCounts requestOwnerCountsForDiagnostics()const noexcept;
    RequestMark beginRequestScope();
    void retireRequestScope(RequestMark);
    options::ObjectRef handle(const scene::NodeRef&,
        std::optional<options::ObjectRef> owner={},
        std::optional<options::ObjectRef> definition={});
    bool isHandle(const options::Value&)const;
    scene::NodeRef resolve(options::ObjectRef)const;
    std::optional<options::ObjectRef> owner(options::ObjectRef)const;
    std::optional<options::ObjectRef> definition(options::ObjectRef)const;
    void associateSourceNode(const scene::NodeRef&,const options::Value&);
    void setSourceChildResolver(std::function<scene::NodeRef(const options::Value&)>);
    std::optional<scene::NodeRef> sourceNode(const options::Value&)const;
    // Projects a Lite DOM identity only when an actual Scene child operation
    // needs it. Event-target discovery remains lookup-only.
    std::optional<scene::NodeRef> projectSourceChild(const options::Value&);
    // Looks up only an already-projected node; never allocates a Scene handle.
    std::optional<scene::NodeRef> findProjectedNode(const options::Value&)const;
    std::optional<options::Value> sourceIdentity(const scene::NodeRef&)const;
    void setOwner(options::ObjectRef,std::optional<options::ObjectRef>);
    void setDefinition(options::ObjectRef,std::optional<options::ObjectRef>);
    void bindPropertyBindings(folia::math::ContextPropertyBindings&);
    void bindLiteModule(adaptors::lite::LiteModule&);
    void bindServiceOwner(std::weak_ptr<void>);
    // Installs scoped operations on Lite's actual DOM methods.
    // Source method identities and descriptors remain untouched.
    class TargetScope {
        struct Impl;
        std::unique_ptr<Impl> impl_;
        explicit TargetScope(std::unique_ptr<Impl>);
        friend class SceneContextNodes;
    public:
        TargetScope(TargetScope&&)noexcept;
        TargetScope& operator=(TargetScope&&)noexcept;
        TargetScope(const TargetScope&)=delete;
        TargetScope& operator=(const TargetScope&)=delete;
        ~TargetScope();
    };
    TargetScope installTargets(adaptors::lite::LiteModule&,scene::Builder&);
    void withPresentationAttribute(options::ObjectRef,std::u16string_view,
        const options::Value&,const std::function<void()>&);
    options::Context& context()const;
};
}
