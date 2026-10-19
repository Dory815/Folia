// MathJax 4.1.3 input/tex/setoptions/SetOptionsConfiguration.ts.
export module folia.math.input.tex.setoptions.set_options_configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct SetOptionsUtil {
    static bool filterPackage(TexParser&,std::u16string_view);
    static bool filterOption(TexParser&,std::u16string_view,std::u16string_view);
    static options::Value filterValue(TexParser&,std::u16string_view,std::u16string_view,options::Value);
};
struct SetOptionsConfiguration { static ConfigurationRef initializeModule(ParserRuntime&); };
}
