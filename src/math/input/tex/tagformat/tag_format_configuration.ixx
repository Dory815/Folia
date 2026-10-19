// MathJax 4.1.3 input/tex/tagformat/TagFormatConfiguration.ts.
export module folia.math.input.tex.tagformat.tag_format_configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
void tagformatConfig(ParserRuntime&,ParserConfiguration&,TeX&);
struct TagFormatConfiguration { static ConfigurationRef initializeModule(ParserRuntime&); };
}
