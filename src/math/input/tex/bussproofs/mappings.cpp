// MathJax 4.1.3 input/tex/bussproofs/BussproofsMappings.ts; Apache-2.0.
module folia.math.input.tex.bussproofs.mappings;
import folia.math.input.tex.parse_methods;
import std;
namespace folia::math::tex {
void BussproofsMappings::initializeModule(MapHandler &maps) {
    maps.registerMap(std::make_shared<CommandMap>(CommandMapInit{MacroMapInit{
        {u"Bussproofs-macros", std::nullopt},
        {Macro(u"AxiomC", &BussproofsMethods::Axiom),
         Macro(u"UnaryInfC", &BussproofsMethods::Inference, {1.0}),
         Macro(u"BinaryInfC", &BussproofsMethods::Inference, {2.0}),
         Macro(u"TrinaryInfC", &BussproofsMethods::Inference, {3.0}),
         Macro(u"QuaternaryInfC", &BussproofsMethods::Inference, {4.0}),
         Macro(u"QuinaryInfC", &BussproofsMethods::Inference, {5.0}),
         Macro(u"RightLabel", &BussproofsMethods::Label, {std::u16string(u"right")}),
         Macro(u"LeftLabel", &BussproofsMethods::Label, {std::u16string(u"left")}),
         Macro(u"AXC", &BussproofsMethods::Axiom),
         Macro(u"UIC", &BussproofsMethods::Inference, {1.0}),
         Macro(u"BIC", &BussproofsMethods::Inference, {2.0}),
         Macro(u"TIC", &BussproofsMethods::Inference, {3.0}),
         Macro(u"RL", &BussproofsMethods::Label, {std::u16string(u"right")}),
         Macro(u"LL", &BussproofsMethods::Label, {std::u16string(u"left")}),
         Macro(u"noLine", &BussproofsMethods::SetLine, {std::u16string(u"none"), false}),
         Macro(u"singleLine", &BussproofsMethods::SetLine, {std::u16string(u"solid"), false}),
         Macro(u"solidLine", &BussproofsMethods::SetLine, {std::u16string(u"solid"), false}),
         Macro(u"dashedLine", &BussproofsMethods::SetLine, {std::u16string(u"dashed"), false}),
         Macro(u"alwaysNoLine", &BussproofsMethods::SetLine, {std::u16string(u"none"), true}),
         Macro(u"alwaysSingleLine", &BussproofsMethods::SetLine, {std::u16string(u"solid"), true}),
         Macro(u"alwaysSolidLine", &BussproofsMethods::SetLine, {std::u16string(u"solid"), true}),
         Macro(u"alwaysDashedLine", &BussproofsMethods::SetLine, {std::u16string(u"dashed"), true}),
         Macro(u"rootAtTop", &BussproofsMethods::RootAtTop, {true}),
         Macro(u"alwaysRootAtTop", &BussproofsMethods::RootAtTop, {true}),
         Macro(u"rootAtBottom", &BussproofsMethods::RootAtTop, {false}),
         Macro(u"alwaysRootAtBottom", &BussproofsMethods::RootAtTop, {false}),
         Macro(u"fCenter", &BussproofsMethods::FCenter),
         Macro(u"Axiom", &BussproofsMethods::AxiomF),
         Macro(u"UnaryInf", &BussproofsMethods::InferenceF, {1.0}),
         Macro(u"BinaryInf", &BussproofsMethods::InferenceF, {2.0}),
         Macro(u"TrinaryInf", &BussproofsMethods::InferenceF, {3.0}),
         Macro(u"QuaternaryInf", &BussproofsMethods::InferenceF, {4.0}),
         Macro(u"QuinaryInf", &BussproofsMethods::InferenceF, {5.0})}}}));
    maps.registerMap(std::make_shared<EnvironmentMap>(EnvironmentMapInit{
        MacroMapInit{{u"Bussproofs-environments", std::nullopt},
                     {Macro(u"prooftree", MacroMethod{&BussproofsMethods::Prooftree},
                            {NullArgument{}, false})}},
        &ParseMethods::environment}));
}
} // namespace folia::math::tex
