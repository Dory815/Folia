/*************************************************************
 * Borrowed Source notation services used by shared Menclose placement.
 * A concrete output owner supplies live method/Map dispatch and scoped
 * targets; mathematical geometry remains in the Scene placement owner.
 *************************************************************/
export module folia.math.output.scene.menclose_source;
export import folia.math.output.scene.foundation;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class MencloseSourceService {
public:
    using Renderer=std::function<options::Value(common::CommonMenclose&,
        std::u16string_view,std::span<const options::Value>)>;
    using PrimitiveRenderer=Renderer;
    virtual ~MencloseSourceService()=default;
    virtual options::Context& context()const=0;
    virtual bool isSceneNodeHandle(const options::Value&)const=0;
    virtual common::CommonWrapper& resolveWrapper(options::ObjectRef)const=0;
    virtual void bindRenderer(Renderer)=0;
    virtual Renderer exchangeRenderer(Renderer)=0;
    virtual void bindPrimitiveRenderer(PrimitiveRenderer)=0;
    virtual PrimitiveRenderer exchangePrimitiveRenderer(PrimitiveRenderer)=0;
    virtual options::ObjectRef createSceneNodeHandle(NodeRef)=0;
    virtual void setSceneNodeDefinition(options::ObjectRef,options::ObjectRef)=0;
    virtual NodeRef sceneNodePayload(options::ObjectRef)const=0;
    virtual std::optional<options::ObjectRef> activeSceneNodeHandle()const=0;
    // Geometry is installed before its Source formatting call. This scope
    // marks only the corresponding presentation write, leaving other writes
    // and replacement methods on the normal live attribute path.
    virtual void withPresentationAttribute(options::ObjectRef,std::u16string_view,
        const options::Value&,const std::function<void()>&)=0;
    virtual void withSceneTarget(common::CommonMenclose&,Builder&,
        const std::function<void()>&)=0;
    virtual void withSceneAppend(common::CommonMenclose&,Builder&,options::ObjectRef,
        const std::function<void()>&)=0;
    virtual options::Value appendSceneNode(common::CommonMenclose&,options::ObjectRef,
        options::ObjectRef)=0;
};
}
