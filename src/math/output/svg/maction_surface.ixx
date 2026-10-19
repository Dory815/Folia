/*************************************************************
 * Source-owned SVG Maction static action Map and DOM event methods.
 * This module uses the session Context and Map storage supplied by the
 * existing render service graph; it does not own a second wrapper or DOM tree.
 *************************************************************/
export module folia.math.output.svg.maction_surface;
export import folia.math.output.svg.wrapper_constructors;
export import folia.math.util.map_bindings;
export import folia.math.output.svg.timer_host;
export import folia.math.output.common;
export import folia.math.output.scene.foundation;
export import folia.math.output.svg.scene_context_nodes;
export import folia.math.adaptors.lite.lite_module;
export import folia.math.util.context_property_bindings;
import std;

export namespace folia::math::output::svg {
class SvgMactionSourceSurface {
    struct State;
    std::shared_ptr<State> state_;
public:
    SvgMactionSourceSurface(options::Context&,options::MapContextBindings&,
        std::shared_ptr<common::CommonFactoryContextBindings>,std::shared_ptr<TimerHost>,
        std::shared_ptr<SceneContextNodes>,std::shared_ptr<void> actualServiceGraph);
    ~SvgMactionSourceSurface();
    SvgMactionSourceSurface(const SvgMactionSourceSurface&)=delete;
    SvgMactionSourceSurface& operator=(const SvgMactionSourceSurface&)=delete;
    // Installs the actual per-Context Source Map and methods on SvgMaction.
    options::ObjectRef install(options::ObjectRef actualConstructor,
                               options::ObjectRef commonMactionConstructor,
                               options::ObjectRef sourceMathItemState);
    void bindLiteModule(folia::math::adaptors::lite::LiteModule&);
    void bindPropertyBindings(folia::math::ContextPropertyBindings&);
    // Called at the Source action(this, data) point after SVG child placement.
    options::Value runAction(common::CommonMaction&,folia::math::output::scene::Builder&,
        std::span<const folia::math::output::scene::NodeRef> roots);
    options::Context& context()const;
};
}
