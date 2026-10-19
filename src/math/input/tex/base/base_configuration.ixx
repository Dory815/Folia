// MathJax 4.1.3 input/tex/base/BaseConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.base_configuration;
export import folia.math.input.tex.base_mappings;
import std;
export namespace folia::math::tex {
class BaseTags:public AbstractTags {
public:
    using AbstractTags::AbstractTags;
};
struct BaseConfiguration {
    static ConfigurationRef initializeModule(ParserRuntime&,const Property& methods,const Property& parseMethods);
};
struct BasePackage {
    static ConfigurationRef initializeModule(ParserRuntime&);
};
}
