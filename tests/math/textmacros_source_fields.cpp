#include "support/folia_test.hpp"
import folia.math;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.textmacros.configuration;
import folia.math.output.common;
import folia.math.output.svg.render_session;
import folia.math.core.mml_tree.mml_factory;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o = folia::math::options;
namespace t = folia::math::tex;
namespace common = folia::math::output::common;
namespace svg = folia::math::output::svg;

common::FontLoadHost fontHost() {
  common::FontLoadHost host;
  host.asyncIsSynchronous = [] { return true; };
  host.warn = [](const std::any &) {};
  return host;
}

suite textmacrosFieldsCandidate = [] {
  "tex_parser_currentCS_starts_as_a_string_and_remains_live"_test = [] {
    svg::NativeRenderSession session(fontHost());
    auto &runtime = session.services().parser();
    auto &context = session.context();
    auto jax = t::TeX::make(runtime);
    jax->setMmlFactory(core::InputMmlFactoryReference{session.services().mmlFactoryOwner()});
    const auto optionValue = jax->parseOptions();
    const auto *options = std::get_if<t::ParseOptionsRef>(&optionValue);
    expect(options != nullptr && *options);
    if (!options || !*options) return;

    auto parser = t::TexParser::make(u"", PropertyList{}, **options);
    const o::Value instance(parser->instanceIdentity());
    expect(context.get(instance, o::Key(u"currentCS")) == o::Value(u""));
    expect(parser->currentCS() == Property(std::string{}));
    context.set(instance, o::Key(u"currentCS"), o::Value(u"\\live"));
    expect(parser->currentCS() == Property("\\live"));
    expect(context.get(instance, o::Key(u"currentCS")) == o::Value(u"\\live"));
  };

  "textmacros_fallback_functions_keep_computed_key_source_metadata"_test = [] {
    svg::NativeRenderSession session(fontHost());
    auto &context = session.context();
    auto &runtime = session.services().parser();
    const auto base = runtime.configurationHandler.get(u"text-base");
    const auto *configuration = std::get_if<std::shared_ptr<t::Configuration>>(&base);
    expect(configuration != nullptr && *configuration);
    if (!configuration || !*configuration) return;
    const auto fallbackValue = (*configuration)->field(u"fallback");
    const auto *fallbackObject = std::get_if<o::ObjectRef>(&fallbackValue.data);
    expect(fallbackObject != nullptr);
    if (!fallbackObject) return;
    for (const auto name : {u"character", u"macro"}) {
      const auto functionValue = context.get(*fallbackObject, o::Key(name));
      const auto *function = std::get_if<o::ObjectRef>(&functionValue.data);
      expect(function != nullptr);
      if (!function) continue;
      expect(context.get(*function, o::Key(u"name")) == o::Value(u""));
      expect(context.toNumber(context.get(*function, o::Key(u"length"))) == 2.0);
    }
  };
};
} // namespace
