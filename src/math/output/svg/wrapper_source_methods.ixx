// Context-backed Source static methods shared by the actual SVG wrapper ctor graph.
export module folia.math.output.svg.wrapper_source_methods;
export import folia.math.output.svg.wrapper_constructors;
export import folia.math.output.common.bbox_context_bindings;
export import folia.math.util.context_property_bindings;
import std;

export namespace folia::math::output::svg {
class SvgWrapperSourceMethods {
    struct State;
    std::shared_ptr<State> state_;
public:
    SvgWrapperSourceMethods(options::Context&,ContextPropertyBindings&,
                            common::CommonFactoryContextBindings&,
                            common::BBoxContextBindings&,
                            math::tex::ParserRuntime&);
    ~SvgWrapperSourceMethods();
    SvgWrapperSourceMethods(const SvgWrapperSourceMethods&)=delete;
    SvgWrapperSourceMethods& operator=(const SvgWrapperSourceMethods&)=delete;
    options::Context& context()const;
    void install(const SvgWrapperConstructorSet&);
};
}
