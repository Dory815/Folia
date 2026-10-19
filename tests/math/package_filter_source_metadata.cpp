#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.output.common;
import folia.math.output.svg.render_session;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t = folia::math::tex;
namespace o = folia::math::options;
namespace common = folia::math::output::common;
namespace svg = folia::math::output::svg;

struct Graph {
  static common::FontLoadHost fontHost() {
    common::FontLoadHost host;
    host.asyncIsSynchronous = [] { return true; };
    host.warn = [](const std::any&) {};
    return host;
  }
  svg::NativeRenderSession session{fontHost()};
  auto& services() const { return session.services(); }
};

void expectProcessor(Graph& graph, const t::ConfigurationRef& configuration,
                     std::u16string_view slot, std::size_t index,
                     std::u16string_view name, std::size_t length) {
  auto& services = graph.services();
  auto& context = services.context();
  auto& functions = services.functions();
  const auto listValue = configuration->field(slot);
  const auto* list = std::get_if<o::ObjectRef>(&listValue.data);
  expect(list != nullptr);
  if (!list) return;
  const auto digits = std::to_string(index);
  const o::Value tupleValue = context.get(
      *list, o::Key(std::u16string(digits.begin(), digits.end())));
  const auto* tuple = std::get_if<o::ObjectRef>(&tupleValue.data);
  expect(tuple != nullptr);
  if (!tuple) return;
  const o::Value callback = context.get(*tuple, o::Key(u"0"));
  const auto* identity = std::get_if<o::ObjectRef>(&callback.data);
  expect(identity != nullptr);
  if (!identity) return;
  const auto functionValue = functions.functionValue(callback);
  const auto* function = std::get_if<FunctionRef>(&functionValue);
  expect(function != nullptr);
  if (!function || !*function) return;
  const auto retainedIdentity = functions.find(*function);
  expect(retainedIdentity.has_value());
  if (!retainedIdentity) return;
  expect(*retainedIdentity == *identity);
  const o::Value actualName = context.get(*identity, o::Key(u"name"));
  const o::Value actualLength = context.get(*identity, o::Key(u"length"));
  const auto* actualNameText = std::get_if<std::u16string>(&actualName.data);
  expect(actualNameText != nullptr);
  if (actualNameText) expect(*actualNameText == name);
  expect(context.toNumber(actualLength) == static_cast<double>(length));
  for (const auto key : {o::Key(u"name"), o::Key(u"length")}) {
    const auto descriptor = context.descriptor(*identity, key);
    expect(descriptor.has_value());
    if (descriptor) {
      expect(!descriptor->writable);
      expect(!descriptor->enumerable);
      expect(descriptor->configurable);
    }
  }
  expect((*function)->sourceMetadata.has_value());
  if (!(*function)->sourceMetadata) return;
  expect((*function)->sourceMetadata->name == name);
  expect((*function)->sourceMetadata->length == length);
}

suite packageFilterSourceMetadata = [] {
  "registered_tex_pre_postprocessors_match_source_metadata_and_keep_identity"_test = [] {
    Graph graph;
    auto& runtime = graph.services().parser();
    const auto base = std::get<t::ConfigurationRef>(runtime.configurationHandler.get(u"base"));
    expectProcessor(graph, base, u"postprocessors", 0, u"filterNonscript", 1);

    const auto bussproofs = std::get<t::ConfigurationRef>(runtime.configurationHandler.get(u"bussproofs"));
    expectProcessor(graph, bussproofs, u"preprocessors", 0, u"saveDocument", 1);
    expectProcessor(graph, bussproofs, u"postprocessors", 0, u"clearDocument", 1);
    expectProcessor(graph, bussproofs, u"postprocessors", 1, u"makeBsprAttributes", 1);
    expectProcessor(graph, bussproofs, u"postprocessors", 2, u"balanceRules", 1);

    const auto begingroup = std::get<t::ConfigurationRef>(runtime.configurationHandler.get(u"begingroup"));
    expectProcessor(graph, begingroup, u"preprocessors", 0, u"", 1);
    expectProcessor(graph, begingroup, u"postprocessors", 0, u"", 1);

    const auto boldsymbol = std::get<t::ConfigurationRef>(runtime.configurationHandler.get(u"boldsymbol"));
    expectProcessor(graph, boldsymbol, u"postprocessors", 0, u"rewriteBoldTokens", 1);

    const auto centernot = std::get<t::ConfigurationRef>(runtime.configurationHandler.get(u"centernot"));
    expectProcessor(graph, centernot, u"postprocessors", 0, u"filterCenterOver", 1);

    const auto mathtools = std::get<t::ConfigurationRef>(runtime.configurationHandler.get(u"mathtools"));
    expectProcessor(graph, mathtools, u"postprocessors", 0, u"fixPrescripts", 1);

    const auto texhtml = std::get<t::ConfigurationRef>(runtime.configurationHandler.get(u"texhtml"));
    expectProcessor(graph, texhtml, u"preprocessors", 0, u"", 1);
    expectProcessor(graph, texhtml, u"postprocessors", 0, u"", 1);

    const auto textmacros = std::get<t::ConfigurationRef>(runtime.configurationHandler.get(u"textmacros"));
    expectProcessor(graph, textmacros, u"preprocessors", 0, u"", 1);
  };
};
} // namespace
