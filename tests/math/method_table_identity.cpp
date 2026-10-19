#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.base_parse_methods;
import folia.math.input.tex.ams.methods;
import folia.math.input.tex.newcommand.methods;
import folia.math.input.tex.mathtools.methods;
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

FunctionRef method(const Property &table, std::u16string_view name) {
  const Property member = property_host::get(table, property_host::debugText(name));
  const auto *function = std::get_if<FunctionRef>(&member.value);
  if (!function || !*function)
    throw std::runtime_error("method table member is not a FunctionRef");
  return *function;
}

suite methodTableIdentity = [] {
  "math_method_tables_keep_actual_runtime_exports_and_forwarded_functions"_test = [] {
    Graph graph;
    auto &runtime = graph.runtime;
    const Property base = t::BaseMethods::initializeModule(runtime);
    const Property parse = t::ParseMethods::initializeModule(runtime);
    const Property ams = t::AmsMethods::initializeModule(runtime);
    const Property newcommand = t::NewcommandMethods::initializeModule(runtime);
    const Property mathtools = t::MathtoolsMethods::initializeModule(runtime);

    const FunctionRef baseMacro = method(base, u"Macro");
    const FunctionRef parseVariable = method(parse, u"variable");
    expect(method(ams, u"Macro") == baseMacro);
    expect(method(newcommand, u"Macro") == baseMacro);
    expect(method(mathtools, u"Macro") == baseMacro);
    expect(method(mathtools, u"MacroWithTemplate") ==
           method(newcommand, u"MacroWithTemplate"));

    property_host::set(base, "sourceModuleMutation", Property(19));
    expect(t::BaseMethods::initializeModule(runtime) == base);
    expect(t::ParseMethods::initializeModule(runtime) == parse);
    expect(t::AmsMethods::initializeModule(runtime) == ams);
    expect(t::NewcommandMethods::initializeModule(runtime) == newcommand);
    expect(method(t::BaseMethods::initializeModule(runtime), u"Macro") ==
           baseMacro);
    expect(method(t::ParseMethods::initializeModule(runtime), u"variable") ==
           parseVariable);
    expect(property_host::get(t::BaseMethods::initializeModule(runtime),
                              "sourceModuleMutation") == Property(19));

    Graph other;
    const Property otherBase = t::BaseMethods::initializeModule(other.runtime);
    expect(method(otherBase, u"Macro") != baseMacro);
  };
};
} // namespace
