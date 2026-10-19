// MathJax 4.1.3 input/tex/base/BaseMethods.ts; Apache-2.0.
// Source-method callback table, kept separate from package configuration and
// the token/environment maps that retain these same FunctionRef values.
export module folia.math.input.tex.base_methods;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct BaseMethods {
    // A fresh source-owned record is populated once per ParserRuntime. Each
    // property is the canonical native ParseMethod FunctionRef used by maps.
    static Property initializeModule(ParserRuntime&);
    static Property get(const Property& table,std::u16string_view name);
    static Property get(ParserRuntime&,std::u16string_view name);
    static options::Value bind(ParserRuntime&,FunctionRef,std::u16string label);
};
// Source-exported BaseMethods helper shared by AMS package methods.
options::Value splitAlignArray(std::u16string_view align,double count=std::numeric_limits<double>::infinity());
}
