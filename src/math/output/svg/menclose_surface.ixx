/*************************************************************
 * Native owner for the Source SvgMenclose static Notations Map.
 * The Map and its function values are real objects in the render Context;
 * callbacks always receive and resolve the canonical CommonMenclose object.
 *************************************************************/
export module folia.math.output.svg.menclose_surface;
export import folia.math.output.common;
export import folia.math.util.map_bindings;
export import folia.math.adaptors.lite.lite_module;
export import folia.math.output.svg.scene_context_nodes;
export import folia.math.output.scene.menclose_source;
import std;

export namespace folia::math::output::svg {
common::CommonWrapperClassRef registerSvgMencloseClass(
    common::CommonFactoryContextBindings&,options::ObjectRef actualConstructor);
class SvgMencloseSourceSurface final:public scene::MencloseSourceService {
public:
    using Renderer=scene::MencloseSourceService::Renderer;
    using PrimitiveRenderer=scene::MencloseSourceService::PrimitiveRenderer;
private:
    struct State;
    std::shared_ptr<State> state_;
public:
    SvgMencloseSourceSurface(options::Context&,options::MapContextBindings&,
                             std::shared_ptr<common::CommonFactoryContextBindings>,
                             std::shared_ptr<SceneContextNodes>,
                             std::shared_ptr<void> actualServiceGraph);
    ~SvgMencloseSourceSurface();
    SvgMencloseSourceSurface(const SvgMencloseSourceSurface&)=delete;
    SvgMencloseSourceSurface& operator=(const SvgMencloseSourceSurface&)=delete;

    // Installs the exact ordered Source notation names on the actual
    // SvgMenclose constructor. Map lookups and callback calls remain live.
    options::ObjectRef install(options::ObjectRef actualConstructor);
    options::Context& context()const override;
    ContextPropertyBindings& propertyBindings()const;
    void bindPropertyBindings(ContextPropertyBindings&);
    void bindLiteModule(folia::math::adaptors::lite::LiteModule&);
    bool isSceneNodeHandle(const options::Value&)const override;
    common::CommonWrapper& resolveWrapper(options::ObjectRef)const override;
    // Scene placement scopes this callback around the actual Source renderer
    // call. No renderer target is retained by a completed Scene.
    void bindRenderer(Renderer) override;
    Renderer exchangeRenderer(Renderer) override;
    void bindPrimitiveRenderer(PrimitiveRenderer) override;
    PrimitiveRenderer exchangePrimitiveRenderer(PrimitiveRenderer) override;
    options::ObjectRef createSceneNodeHandle(scene::NodeRef exactSceneNode) override;
    void withPresentationAttribute(options::ObjectRef,std::u16string_view,
        const options::Value&,const std::function<void()>&) override;
    void setSceneNodeDefinition(options::ObjectRef handle,options::ObjectRef definition) override;
    scene::NodeRef sceneNodePayload(options::ObjectRef handle)const override;
    std::optional<options::ObjectRef> activeSceneNodeHandle()const override;
    void withSceneTarget(common::CommonMenclose&,scene::Builder&,
                         const std::function<void()>&) override;
    void withSceneAppend(common::CommonMenclose&,scene::Builder&,options::ObjectRef rootHandle,
                         const std::function<void()>&) override;
    options::Value appendSceneNode(common::CommonMenclose&,options::ObjectRef parentHandle,
                                   options::ObjectRef childHandle) override;
    options::Value render(common::CommonMenclose&,std::u16string_view,
                          std::span<const options::Value> args={});
};
}
