/*************************************************************
 * Native constructor graph for MathJax's SVG wrapper classes.
 *
 * The Context objects created here are the actual constructor/prototype
 * objects consumed by SvgWrapperFactory.defaultNodes.  Instances allocated
 * by them are the same CommonWrapper products adopted by the current layout
 * job; the Context payload is only a lifetime-pinned view of that product.
 *************************************************************/
export module folia.math.output.svg.wrapper_constructors;
export import folia.math.output.svg.wrapper_classes;
export import folia.math.input.tex.parser;
export import folia.math.output.common;
export import folia.math.util.runtime_modules;
import std;

export namespace folia::math::output::svg {
struct SvgWrapperConstructorSet {
    options::ObjectRef wrappers;
    std::vector<std::pair<std::u16string,options::ObjectRef>> constructors;
};

class SvgWrapperConstructorBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    SvgWrapperConstructorBindings(options::Context&,
        math::tex::ParserRuntime&,
        common::CommonFactoryContextBindings&);
    ~SvgWrapperConstructorBindings();
    SvgWrapperConstructorBindings(const SvgWrapperConstructorBindings&)=delete;
    SvgWrapperConstructorBindings& operator=(const SvgWrapperConstructorBindings&)=delete;
    options::Context& context()const;
    SvgWrapperConstructorSet initialize();
};

// Build the Source constructor/prototype graph using real Context constructors.
// The returned `wrappers` object is the actual Source defaultNodes record;
// its property order is the pinned SvgWrappers declaration order.
SvgWrapperConstructorSet loadSvgWrapperConstructors(
    util::RuntimeModules&, math::tex::ParserRuntime&,
    common::CommonFactoryContextBindings&);
}
