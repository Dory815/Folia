// MathJax 4.1.3 input/tex/ams/AmsConfiguration.ts.
export module folia.math.input.tex.ams.configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class AmsTags:public AbstractTags { public: using AbstractTags::AbstractTags; };
struct AmsConfiguration { static ConfigurationRef initializeModule(ParserRuntime&); };
}
