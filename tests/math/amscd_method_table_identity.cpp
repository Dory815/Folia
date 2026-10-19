#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.amscd.methods;
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
    throw std::runtime_error("AmsCdMethods export is not callable");
  return *function;
}

suite amscdMethodTableIdentity = [] {
  "amscd_method_exports_are_canonical_per_runtime"_test = [] {
    Graph graph;
    auto& runtime = graph.runtime;

    const Property methods = t::AmsCdMethods::initializeModule(runtime);
    const auto cd = method(methods, u"CD");
    const auto arrow = method(methods, u"arrow");
    const auto cell = method(methods, u"cell");
    const auto minWidth = method(methods, u"minCDarrowwidth");
    const auto minHeight = method(methods, u"minCDarrowheight");

    const auto mapped = t::AmsCdMethods::create(runtime);
    expect(mapped.cd == cd);
    expect(mapped.arrow == arrow);
    expect(mapped.cell == cell);
    expect(mapped.minWidth == minWidth);
    expect(mapped.minHeight == minHeight);

    property_host::set(methods, "sourceModuleMutation", Property(19));
    expect(t::AmsCdMethods::initializeModule(runtime) == methods);
    expect(t::AmsCdMethods::create(runtime).cd == cd);
    expect(property_host::get(t::AmsCdMethods::initializeModule(runtime),
                              "sourceModuleMutation") == Property(19));

    Graph other;
    const auto otherMethods = t::AmsCdMethods::initializeModule(other.runtime);
    expect(otherMethods != methods);
    expect(method(otherMethods, u"CD") != cd);
  };
};
} // namespace
