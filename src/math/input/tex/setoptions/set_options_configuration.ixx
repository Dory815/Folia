// MathJax 4.1.3 input/tex/setoptions/SetOptionsConfiguration.ts.
export module folia.math.input.tex.setoptions.set_options_configuration;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
struct SetOptionsUtil {
    static bool filterPackage(TexParser &, std::u16string_view);
    static bool filterOption(TexParser &, std::u16string_view, std::u16string_view);
    static OptionInputValue filterValue(TexParser &, std::u16string_view, std::u16string_view,
                                        OptionInputValue const &);
};
DispatchOutcome SetOptions(TexParser &, MacroInvocation const &);
void setoptionsConfig(ParserConfiguration &, TeX &);
struct SetOptionsConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &);
};
} // namespace folia::math::tex
