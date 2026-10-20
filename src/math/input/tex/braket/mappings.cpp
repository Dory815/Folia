// MathJax 4.1.3 input/tex/braket/BraketMappings.ts; Apache-2.0.
module folia.math.input.tex.braket.mappings;
import folia.math.input.tex.braket.methods;
import std;
namespace folia::math::tex {
void BraketMappings::initializeModule(MapHandler &maps) {
    const auto infinity = std::numeric_limits<double>::infinity();
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"Braket-macros", std::nullopt},
        {Macro(u"bra", BraketMethods::Macro, {std::u16string(u"{\\langle {#1} \\vert}"), 1.0}),
         Macro(u"ket", BraketMethods::Macro, {std::u16string(u"{\\vert {#1} \\rangle}"), 1.0}),
         Macro(u"braket", &BraketMethods::Braket,
               {std::u16string(u"\u27E8"), std::u16string(u"\u27E9"), false, infinity}),
         Macro(u"set", &BraketMethods::Braket,
               {std::u16string(u"{"), std::u16string(u"}"), false, 1.0}),
         Macro(u"Bra", BraketMethods::Macro,
               {std::u16string(u"{\\left\\langle {#1} \\right\\vert}"), 1.0}),
         Macro(u"Ket", BraketMethods::Macro,
               {std::u16string(u"{\\left\\vert {#1} \\right\\rangle}"), 1.0}),
         Macro(u"Braket", &BraketMethods::Braket,
               {std::u16string(u"\u27E8"), std::u16string(u"\u27E9"), true, infinity}),
         Macro(u"Set", &BraketMethods::Braket,
               {std::u16string(u"{"), std::u16string(u"}"), true, 1.0, true}),
         Macro(u"ketbra", BraketMethods::Macro,
               {std::u16string(u"{\\vert {#1} \\rangle\\langle {#2} \\vert}"), 2.0}),
         Macro(u"Ketbra", BraketMethods::Macro,
               {std::u16string(
                    u"{\\left\\vert {#1} \\right\\rangle\\left\\langle {#2} \\right\\vert}"),
                2.0}),
         Macro(u"|", &BraketMethods::Bar)}}}));
    maps.registerMap(std::make_shared<MacroMap>(
        MacroMapInit{{u"Braket-characters", std::nullopt}, {Macro(u"|", &BraketMethods::Bar)}}));
}
} // namespace folia::math::tex
