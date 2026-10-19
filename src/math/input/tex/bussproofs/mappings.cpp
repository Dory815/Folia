// Native port of MathJax 4.1.3 input/tex/bussproofs/BussproofsMappings.ts.
module folia.math.input.tex.bussproofs.mappings;
import folia.math.input.tex.base_parse_methods;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
void BussproofsMappings::initializeModule(ParserRuntime &runtime) {
  const auto methods = BussproofsMethods::create(runtime);
  PropertyList commands;
  const auto method = [&](std::u16string_view name) {
    return property_host::get(methods.methods,
                              std::string(name.begin(), name.end()));
  };
  const auto tuple = [&](std::u16string_view name,
                         std::initializer_list<Property> args) {
    PropertyArray values{method(name)};
    for (const auto &value : args)
      values.push(value);
    return Property(values);
  };
  commands.setOwn("AxiomC", method(u"Axiom"));
  commands.setOwn("UnaryInfC", tuple(u"Inference", {1}));
  commands.setOwn("BinaryInfC", tuple(u"Inference", {2}));
  commands.setOwn("TrinaryInfC", tuple(u"Inference", {3}));
  commands.setOwn("QuaternaryInfC", tuple(u"Inference", {4}));
  commands.setOwn("QuinaryInfC", tuple(u"Inference", {5}));
  commands.setOwn("RightLabel", tuple(u"Label", {Property("right")}));
  commands.setOwn("LeftLabel", tuple(u"Label", {Property("left")}));
  commands.setOwn("AXC", method(u"Axiom"));
  commands.setOwn("UIC", tuple(u"Inference", {1}));
  commands.setOwn("BIC", tuple(u"Inference", {2}));
  commands.setOwn("TIC", tuple(u"Inference", {3}));
  commands.setOwn("RL", tuple(u"Label", {Property("right")}));
  commands.setOwn("LL", tuple(u"Label", {Property("left")}));
  for (const auto &[name, style, always] :
       std::array<std::tuple<std::string_view, std::string_view, bool>, 8>{
           std::tuple{"noLine", "none", false},
           {"singleLine", "solid", false},
           {"solidLine", "solid", false},
           {"dashedLine", "dashed", false},
           {"alwaysNoLine", "none", true},
           {"alwaysSingleLine", "solid", true},
           {"alwaysSolidLine", "solid", true},
           {"alwaysDashedLine", "dashed", true}})
    commands.setOwn(name, tuple(u"SetLine", {Property(std::string(style)),
                                             Property(always)}));
  commands.setOwn("rootAtTop", tuple(u"RootAtTop", {true}));
  commands.setOwn("alwaysRootAtTop", tuple(u"RootAtTop", {true}));
  commands.setOwn("rootAtBottom", tuple(u"RootAtTop", {false}));
  commands.setOwn("alwaysRootAtBottom", tuple(u"RootAtTop", {false}));
  commands.setOwn("fCenter", method(u"FCenter"));
  commands.setOwn("Axiom", method(u"AxiomF"));
  commands.setOwn("UnaryInf", tuple(u"InferenceF", {1}));
  commands.setOwn("BinaryInf", tuple(u"InferenceF", {2}));
  commands.setOwn("TrinaryInf", tuple(u"InferenceF", {3}));
  commands.setOwn("QuaternaryInf", tuple(u"InferenceF", {4}));
  commands.setOwn("QuinaryInf", tuple(u"InferenceF", {5}));
  (void)TokenMapHost::create<CommandMap>(
      runtime.mapHandler, Property("Bussproofs-macros"), Property(commands));
  const auto environment = Property(PropertyList{
      {"prooftree", Property(PropertyArray{Property(method(u"Prooftree")),
                                           PropertyNull{}, Property(false)})}});
  const Property parseMethods = ParseMethods::initializeModule(runtime);
  const ParseMethod environmentParser =
      functions::fromProperty(property_host::get(parseMethods, "environment"));
  (void)TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,
      Property("Bussproofs-environments"), environmentParser, environment,
      methods.methods);
}
} // namespace folia::math::tex
