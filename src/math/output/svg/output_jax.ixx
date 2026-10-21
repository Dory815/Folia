/*************************************************************
 * MathJax 4.1.3 output/svg.ts; SVG-specific output lifecycle and scene handoff.
 *************************************************************/
export module folia.math.output.svg.output_jax;
export import folia.math.output.common;
export import folia.math.output.svg.serializer;
export import folia.math.output.svg.font_cache;
export import folia.math.output.scene;
import folia.math.output.svg.menclose_surface;
import folia.math.output.svg.scene_context_nodes;
import folia.math.adaptors.lite.lite_module;
import std;

export namespace folia::math::output::svg {
inline constexpr std::u16string_view SVGNS=u"http://www.w3.org/2000/svg";
inline constexpr std::u16string_view XLINKNS=u"http://www.w3.org/1999/xlink";

class SvgOutputJax final : public common::CommonOutputJax {
public:
    using SceneSvgTarget=std::function<std::optional<options::Value>(std::span<const options::Value>)>;
    using TextMeasurementProvider=std::function<double(std::u16string_view)>;
    using SceneOutputConsumer=std::function<void(
        std::shared_ptr<const scene::Scene>,DOMNodeRef)>;
private:
    scene::WrapperFactory placements_;
    std::unique_ptr<FontCache> fontCache_;
    ContextPropertyBindings* propertyBindings_=nullptr;
    std::shared_ptr<scene::Scene> completedScene_;
    DOMNodeRef svgStyles_;
    bool placementsReady_=false;
    SceneSvgTarget sceneSvgTarget_;
    std::optional<options::ObjectRef> defaultHtmlMethod_;
    std::shared_ptr<SvgMencloseSourceSurface> mencloseSurface_;
    std::shared_ptr<SceneContextNodes> sceneContextNodes_;
    std::shared_ptr<adaptors::lite::LiteModule> liteModule_;
    SceneOutputConsumer sceneOutputConsumer_;
    // Borrowed only while the top-level processMath builds its Scene.
    scene::Builder* activeSceneBuilder_=nullptr;
    TextMeasurementProvider textMeasurementProvider_;
    std::string ex(double)const;
    std::vector<DOMNodeRef> materialized(const scene::Scene&);
    std::vector<scene::NodeRef> buildScene(common::CommonWrapper&,DOMNodeRef,
        scene::Builder&,bool appendToProjectedParent);
    void appendProjectedRoot(DOMNodeRef,const scene::NodeRef&);
    void insertProjectedBefore(const scene::NodeRef&,const scene::NodeRef&);
    void removeProjectedRoot(const scene::NodeRef&);
    common::UnknownBBox measuredTextWidth(double)const;
    bool findCache(core::MathDocumentReference)const;
    void handleMathDisplay(common::CommonMath&);
    std::array<scene::NodeRef,2> createSVG(scene::Builder&,double h,double d,double w,
        DOMNodeRef projectedParent={});
    std::vector<scene::NodeRef> handleInlineBreaks(common::CommonWrapper&,
        scene::Builder&,const scene::NodeRef&,const scene::NodeRef&,DOMNodeRef nestedParent={});
    void addInlineBreak(std::vector<scene::NodeRef>&,double space,bool forced);
    scene::NodeRef importXmlSvgNode(scene::Builder&,scene::NodeRef,const DOMNodeRef&);
    scene::NodeRef importXmlSvgNode(scene::NodeRef,const DOMNodeRef&);
protected:
    double getInitialScale()const override{return 1;}
    bool forceInlineBreaks()const override;
public:
    double minwidth=0,shift=0;
    SvgOutputJax(OptionFunctionBindings&,options::ObjectRef,
                 FunctionListContextBindings&,AbstractDOMAdaptor* adaptor=nullptr,
                 std::shared_ptr<core::JaxFilterRecordOwnerScope> owner={});
    void initializeSvg(options::Value supplied,options::ObjectRef defaultFactory,
                       options::ObjectRef defaultFont,common::CommonRuntimeBindings&,
                       std::shared_ptr<void> owner);
    const scene::Scene* completedScene()const noexcept{return completedScene_.get();}
    SceneOutputConsumer exchangeSceneOutputConsumer(SceneOutputConsumer consumer){
        return std::exchange(sceneOutputConsumer_,std::move(consumer));
    }
    void bindTextMeasurementProvider(TextMeasurementProvider provider){
        if(!provider)throw std::invalid_argument("Text measurement provider is empty");
        textMeasurementProvider_=std::move(provider);
    }
    TextMeasurementProvider exchangeTextMeasurementProvider(TextMeasurementProvider provider){
        return std::exchange(textMeasurementProvider_,std::move(provider));
    }
    FontCache& fontCache();
    void initialize()override;
    void reset(FunctionArgs args={})override;
    void clearFontCache();
    core::MathOutput escaped(core::MathItem&,core::MathDocumentReference)override;
    core::MathOutput styleSheet(core::MathDocumentReference)override;
    core::MathOutput pageElements(core::MathDocumentReference)override;
    void insertStyles(const style_json::StyleJson&)override;
    void processMath(common::CommonWrapper&,DOMNodeRef)override;
    std::shared_ptr<scene::Scene> produceScene(common::CommonWrapper&,DOMNodeRef);
    DOMNodeRef unknownText(std::u16string_view,std::string_view)override;
    common::UnknownBBox measureTextNode(DOMNodeRef)override;
    void setScale(DOMNodeRef,common::CommonWrapper&)override;
    void setSceneLayoutObserver(scene::LayoutEventObserverRef observer){
        placements_.setLayoutObserver(std::move(observer));
    }
    SceneSvgTarget exchangeSceneSvgTarget(SceneSvgTarget target){
        return std::exchange(sceneSvgTarget_,std::move(target));
    }
    options::Value dispatchSvg(std::span<const options::Value> args);
    DOMNodeRef sourceSvgNode(std::u16string_view,PropertyList={},PropertyArray={});
    void bindSceneContextNodes(std::shared_ptr<SceneContextNodes>,
        std::shared_ptr<adaptors::lite::LiteModule>);
    void setDefaultHtmlMethod(options::ObjectRef function){defaultHtmlMethod_=function;}
    void bindMencloseSurface(std::shared_ptr<SvgMencloseSourceSurface> surface){
        mencloseSurface_=std::move(surface);
    }
};
}
