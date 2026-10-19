#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.ams.configuration;
import folia.math.input.tex.cases.methods;
import folia.math.input.tex.cases.configuration;
import folia.math.input.tex.token_map;
import folia.math.input.tex.macro;
import folia.math.util.function_types;
import folia.math.util.property_host;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;

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

FunctionRef method(const Property& table, std::u16string_view name) {
  const auto value = property_host::get(table, property_host::debugText(name));
  const auto* function = std::get_if<FunctionRef>(&value.value);
  if (!function || !*function)
    throw std::runtime_error("CasesMethods export is not callable");
  return *function;
}

FunctionRef macroMethod(t::TokenMapRef map, std::string_view token) {
  auto* macros = dynamic_cast<t::AbstractParseMap*>(map.get());
  if (!macros) throw std::runtime_error("case token map has no parse-map lookup");
  const auto entry = macros->lookup(Property(std::string(token)));
  const auto* macro = std::get_if<t::MacroRef>(&entry);
  if (!macro || !*macro) throw std::runtime_error("Cases command is not registered");
  return functions::reference((*macro)->func());
}

suite casesMethodTableIdentity = [] {
  "cases_method_exports_are_canonical_and_maps_share_the_exported_functions"_test = [] {
    Graph graph;
    auto& runtime = graph.runtime;
    (void)t::BasePackage::initializeModule(runtime);
    (void)t::AmsConfiguration::initializeModule(runtime);

    const auto methods = t::CasesMethods::initializeModule(runtime);
    const auto numCases = method(methods, u"NumCases");
    const auto entry = method(methods, u"Entry");
    const auto environment = method(methods, u"environment");
    const auto record = PropertyList::record(methods);
    expect(record.has_value());
    expect(record->keys() ==
           std::vector<std::string>{"NumCases", "Entry", "environment"});

    property_host::set(methods, "sourceModuleMutation", Property(19));
    expect(t::CasesMethods::initializeModule(runtime) == methods);
    expect(method(t::CasesMethods::initializeModule(runtime), u"NumCases") ==
           numCases);
    expect(property_host::get(t::CasesMethods::initializeModule(runtime),
                              "sourceModuleMutation") == Property(19));

    (void)t::CasesConfiguration::initializeModule(runtime);
    const auto commandMap = runtime.mapHandler.getMap(Property("cases-macros"));
    const auto environmentMap = runtime.mapHandler.getMap(Property("cases-env"));
    expect(macroMethod(commandMap, "&") == entry);
    expect(macroMethod(environmentMap, "numcases") == numCases);
    expect(macroMethod(environmentMap, "subnumcases") == numCases);
    const auto* environmentTokenMap = dynamic_cast<t::AbstractTokenMap*>(environmentMap.get());
    expect(environmentTokenMap != nullptr);
    expect(functions::reference(environmentTokenMap->parser()) == environment);
    expect(method(runtime.casesMethods, u"environment") == environment);

    Graph other;
    (void)t::BasePackage::initializeModule(other.runtime);
    (void)t::AmsConfiguration::initializeModule(other.runtime);
    const auto otherMethods = t::CasesMethods::initializeModule(other.runtime);
    expect(otherMethods != methods);
    expect(method(otherMethods, u"NumCases") != numCases);
    (void)t::CasesMethods::initializeModule(other.runtime);
    expect(method(other.runtime.casesMethods, u"NumCases") != numCases);
  };
};
} // namespace
