/*************************************************************
 * Context constructor binding for the actual SVG output Jax.
 *
 * This is the Source class-construction path: static option/style records,
 * the SvgWrapperFactory class object, and each produced SvgOutputJax instance
 * are bound in one Context. Scene placement remains on the same common
 * wrapper graph produced by that factory.
 *************************************************************/
export module folia.math.output.svg.output_jax_bindings;
export import folia.math.output.svg.output_jax;
export import folia.math.output.svg.wrapper_factory_bindings;
export import folia.math.output.common;
export import folia.math.input.tex.parser;
export import folia.math.util.map_bindings;
export import folia.math.output.svg.menclose_surface;
export import folia.math.output.svg.scene_context_nodes;
import std;

export namespace folia::math::output::svg {
class SvgOutputJaxContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    SvgOutputJaxContextBindings(common::CommonJaxClassBindings&,
        common::CommonRuntimeBindings&,SvgWrapperFactoryContextBindings&,
        OptionFunctionBindings&,FunctionListContextBindings&,
        std::shared_ptr<core::JaxFilterRecordOwnerScope>,
        options::MapContextBindings&);
    ~SvgOutputJaxContextBindings();
    SvgOutputJaxContextBindings(const SvgOutputJaxContextBindings&)=delete;
    SvgOutputJaxContextBindings& operator=(const SvgOutputJaxContextBindings&)=delete;

    options::Context& context()const;
    options::MapContextBindings& mapBindings()const;
    void bindMencloseSurface(std::shared_ptr<SvgMencloseSourceSurface>);
    void bindSceneContextNodes(std::shared_ptr<SceneContextNodes>,
        std::shared_ptr<adaptors::lite::LiteModule>);
    options::ObjectRef registerClass(options::ObjectRef commonConstructor,
        options::ObjectRef actualSvgWrappers,options::ObjectRef defaultFont);
    options::ObjectRef registerClass(options::ObjectRef commonConstructor,
        math::tex::ParserRuntime&,options::ObjectRef defaultFont);
    options::ObjectRef constructor()const;
    std::shared_ptr<SvgOutputJax> construct(options::Value supplied=options::Undefined{});
    std::shared_ptr<SvgOutputJax> resolve(options::ObjectRef instance)const;
};
}
