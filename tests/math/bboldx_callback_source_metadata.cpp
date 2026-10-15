#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.bboldx.configuration;
import folia.math.input.tex.setoptions.set_options_configuration;
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

suite bboldxCallbackSourceMetadata = [] {
  "bboldx_configuration_callback_matches_pinned_source_function_metadata"_test = [] {
    Graph graph;
    (void)t::BasePackage::initializeModule(graph.runtime);
    const auto configuration = t::BboldxConfiguration::initializeModule(graph.runtime);
    const auto callback = configuration->config();
    const auto *identity = std::get_if<o::ObjectRef>(&callback.data);
    expect(identity != nullptr);
    if (!identity) return;

    const auto name = graph.context.get(*identity, o::Key(u"name"));
    const auto length = graph.context.get(*identity, o::Key(u"length"));
    expect(std::get<std::u16string>(name.data).empty());
    expect(graph.context.toNumber(length) == 2.0);
    expect(configuration->config() == callback);
  };
  "setoptions_configuration_callback_matches_pinned_source_function_metadata"_test = [] {
    Graph graph;
    const auto configuration = t::SetOptionsConfiguration::initializeModule(graph.runtime);
    const auto callback = configuration->config();
    const auto *identity = std::get_if<o::ObjectRef>(&callback.data);
    expect(identity != nullptr);
    if (!identity) return;

    const auto name = graph.context.get(*identity, o::Key(u"name"));
    const auto length = graph.context.get(*identity, o::Key(u"length"));
    expect(std::get<std::u16string>(name.data) == u"setoptionsConfig");
    expect(graph.context.toNumber(length) == 2.0);
    expect(configuration->config() == callback);
  };
};
} // namespace
