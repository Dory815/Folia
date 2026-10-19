#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.units.configuration;
import folia.math.input.tex.token_map;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t = folia::math::tex;
namespace o = folia::math::options;

struct Graph {
  o::Context context;
  OptionFunctionBindings functions{context};
  regexp_host::RegExpBindings regexp{context};
  Unicode16Normalization unicode;
  core::InputJaxModule input{context};
  core::FindMathModule findMath{context};
  t::FindTeXModule findTeX{context, findMath, regexp};
  t::ParserRuntime runtime{context, functions, regexp, unicode, input, findTeX};

  Graph() { runtime.initializeParserRuntime(); }
};

FunctionRef exportedMethod(const Property &methods, std::u16string_view name) {
  const Property value = property_host::get(methods, property_host::debugText(name));
  const auto *function = std::get_if<FunctionRef>(&value.value);
  if (!function || !*function)
    throw std::runtime_error("UnitsMethods export is not a FunctionRef");
  return *function;
}

FunctionRef commandMethod(t::ParserRuntime &runtime, std::u16string_view key) {
  const auto map = std::dynamic_pointer_cast<t::AbstractParseMap>(
      runtime.mapHandler.getMap(Property("units")));
  if (!map)
    throw std::runtime_error("units CommandMap is unavailable");
  const auto lookup = map->lookup(Property(property_host::debugText(key)));
  const auto *macro = std::get_if<t::MacroRef>(&lookup);
  if (!macro || !*macro)
    throw std::runtime_error("units CommandMap entry is not a Macro");
  return functions::reference((*macro)->func());
}

suite unitsMethodsIdentity = [] {
  "math_units_methods_are_canonical_exports_shared_with_command_map"_test = [] {
    Graph graph;
    auto &runtime = graph.runtime;
    const auto configuration = t::UnitsConfiguration::initializeModule(runtime);
    const Property methods = t::UnitsMethods::initializeModule(runtime);
    const Property repeatedMethods = t::UnitsMethods::initializeModule(runtime);
    expect(methods == repeatedMethods);
    expect(t::UnitsConfiguration::initializeModule(runtime) == configuration);
    expect(property_host::ownKeys(methods) ==
           std::vector<std::string>({"Unit", "UnitFrac", "NiceFrac"}));

    const FunctionRef unit = exportedMethod(methods, u"Unit");
    const FunctionRef unitFrac = exportedMethod(methods, u"UnitFrac");
    const FunctionRef niceFrac = exportedMethod(methods, u"NiceFrac");
    expect(commandMethod(runtime, u"units") == unit);
    expect(commandMethod(runtime, u"unitfrac") == unitFrac);
    expect(commandMethod(runtime, u"nicefrac") == niceFrac);
    const std::array<std::pair<FunctionRef, std::u16string_view>, 3> reflected{{
        {unit, u"Unit"}, {unitFrac, u"UnitFrac"}, {niceFrac, u"NiceFrac"}}};
    for (const auto &[method, name] : reflected) {
      const auto identity = runtime.functions().find(method);
      expect(identity.has_value());
      expect(graph.context.get(*identity, u"name") ==
             o::Value(std::u16string(name)));
      expect(graph.context.get(*identity, u"length") == o::Value(2));
    }

    const auto replacement = std::make_shared<Function>();
    replacement->call = [](const FunctionReceiver &, FunctionArgs) -> std::any {
      return std::monostate{};
    };
    property_host::set(methods, "Unit", Property(replacement));
    expect(t::UnitsMethods::initializeModule(runtime) == methods);
    expect(exportedMethod(t::UnitsMethods::initializeModule(runtime), u"Unit") ==
           replacement);
    expect(commandMethod(runtime, u"units") == unit);

    Graph other;
    const Property otherMethods = t::UnitsMethods::initializeModule(other.runtime);
    expect(otherMethods != methods);
    expect(exportedMethod(otherMethods, u"Unit") != unit);
    expect(commandMethod(other.runtime, u"units") ==
           exportedMethod(otherMethods, u"Unit"));
  };
};
} // namespace
