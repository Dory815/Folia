#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.textcomp.configuration;
import folia.math.input.tex.textcomp.mappings;
import folia.math.input.tex.textcomp.methods;
import folia.math.input.tex.token_map;
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

suite textcompCanonicalOwner = [] {
  "textcomp_method_and_mapping_owners_are_canonical_per_runtime"_test = [] {
    Graph graph;
    auto &runtime = graph.runtime;
    const Property insert = t::TextcompMethods::initializeModule(runtime);
    expect(t::TextcompMethods::initializeModule(runtime) == insert);

    expect(!runtime.mapHandler.getMap(Property("textcomp-macros")));
    t::TextcompMappings::initializeModule(runtime);
    const auto map = runtime.mapHandler.getMap(Property("textcomp-macros"));
    expect(static_cast<bool>(map));
    t::TextcompMappings::initializeModule(runtime);
    expect(runtime.mapHandler.getMap(Property("textcomp-macros")) == map);

    const auto parseMap = std::dynamic_pointer_cast<t::AbstractParseMap>(map);
    expect(static_cast<bool>(parseMap));
    if (parseMap) {
      expect(parseMap->mapStorage()->size() == 117u);
      const auto lookup = parseMap->lookup(Property("textcopyleft"));
      const auto *macro = std::get_if<t::MacroRef>(&lookup);
      expect(macro && *macro);
      if (macro && *macro) {
        const auto method = (*macro)->func();
        const auto *function = std::get_if<FunctionRef>(&method);
        expect(function && *function);
        if (function && *function) expect(Property(*function) == insert);
      }
    }

    const auto config = t::TextcompConfiguration::initializeModule(runtime);
    expect(t::TextcompConfiguration::initializeModule(runtime) == config);
    Graph other;
    const Property otherInsert = t::TextcompMethods::initializeModule(other.runtime);
    expect(otherInsert != insert);
    t::TextcompMappings::initializeModule(other.runtime);
    expect(other.runtime.mapHandler.getMap(Property("textcomp-macros")) != map);
  };
};
} // namespace
