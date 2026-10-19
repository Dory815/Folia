#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.centernot.configuration;
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

FunctionRef exportedFilter(const Property& exports) {
  const auto value = property_host::get(exports, "filterCenterOver");
  const auto* function = std::get_if<FunctionRef>(&value.value);
  if (!function || !*function)
    throw std::runtime_error("Centernot filterCenterOver export is not a FunctionRef");
  return *function;
}

suite centernotCanonicalOwner = [] {
  "math_centernot_named_filter_is_canonical_and_shared_with_configuration"_test = [] {
    Graph graph;
    auto& runtime = graph.runtime;
    (void)t::BaseMethods::initializeModule(runtime);

    auto exports = t::CenternotMethods::initializeModule(runtime);
    const auto repeatedExports = t::CenternotMethods::initializeModule(runtime);
    expect(exports == repeatedExports) << "repeated Source module initialization reuses the export object";
    expect(property_host::ownKeys(exports) ==
           std::vector<std::string>{"filterCenterOver"}) << "only the named Source export is present";
    expect(property_host::get(exports, "CenterOver").missing()) << "private CenterOver is not exported";

    const auto filter = exportedFilter(exports);
    expect(t::CenternotMethods::filterCenterOver(runtime) == filter) << "method accessor returns the canonical export";
    const auto identity = runtime.functions().find(filter);
    expect(identity.has_value()) << "export has a Context function identity";
    if (!identity) return;
    expect(graph.context.get(*identity, u"name") == o::Value(u"filterCenterOver")) << "Context function name matches Source";
    expect(graph.context.get(*identity, u"length") == o::Value(1)) << "Context function arity matches Source";

    const auto configuration = t::CenternotConfiguration::initializeModule(runtime);
    expect(t::CenternotConfiguration::initializeModule(runtime) == configuration) << "cached configuration survives repeated initialization";
    const auto postprocessors = configuration->field(u"postprocessors");
    const auto* postprocessorArray = std::get_if<o::ObjectRef>(&postprocessors.data);
    expect(postprocessorArray != nullptr) << "Configuration.create retains postprocessors as an array";
    if (!postprocessorArray) return;
    const auto callbacks = graph.context.arrayValues(*postprocessorArray);
    expect(callbacks.size() == 1) << "Source normalizes one postprocessor entry";
    const auto* processorIdentity = callbacks.empty() ? nullptr : std::get_if<o::ObjectRef>(&callbacks[0].data);
    expect(processorIdentity != nullptr) << "normalized postprocessor is a [function, priority] tuple";
    std::vector<o::Value> processor;
    if (processorIdentity) processor = graph.context.arrayValues(*processorIdentity);
    expect(processor.size() == 2) << "postprocessor tuple contains function and priority";
    expect(processor.size() == 2 && processor[0] == o::Value(*identity)) << "postprocessor tuple reuses the named export identity";
    expect(processor.size() == 2 && processor[1] == o::Value(5)) << "Configuration.create applies Source default priority 5";
    expect(runtime.functions().find(*identity) == std::optional<FunctionRef>(filter)) << "Context identity resolves to the same canonical FunctionRef";

    property_host::set(exports, "filterCenterOver", Property(17));
    expect(t::CenternotConfiguration::initializeModule(runtime) == configuration) << "cached Source module configuration does not reread a reassigned export";

    Graph other;
    (void)t::BaseMethods::initializeModule(other.runtime);
    const auto otherExports = t::CenternotMethods::initializeModule(other.runtime);
    const auto otherFilter = exportedFilter(otherExports);
    const auto otherIdentity = other.runtime.functions().find(otherFilter);
    expect(otherExports != exports) << "separate runtime has a separate methods export object";
    expect(otherFilter != filter) << "separate runtime has a separate FunctionRef";
    expect(otherIdentity.has_value()) << "second runtime export has its own Context identity";
    expect(otherIdentity && *otherIdentity != *identity) << "separate runtimes do not share Context function identity";
  };
};
} // namespace
