// MathJax 4.1.3 input/tex/base/BaseMappings.ts; Apache-2.0.
export module folia.math.input.tex.base_mappings;
export import folia.math.input.tex.base_methods;
export import folia.math.input.tex.token_map;
import std;
export namespace folia::math::tex {
struct BaseMappings {
    static void initializeModule(ParserRuntime&,const Property& methods,const Property& parseMethods);
    static RegExpRef regexpContext(ParserRuntime&,const options::Value&);
};
}
