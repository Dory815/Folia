#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.html.configuration;
import folia.math.input.tex.html.methods;
import folia.math.input.tex.texhtml.tex_html_methods;
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

FunctionRef method(const Property& table, std::u16string_view name) {
  const auto member = property_host::get(table, property_host::debugText(name));
  const auto* function = std::get_if<FunctionRef>(&member.value);
  if (!function || !*function)
    throw std::runtime_error("HTML method export is not a FunctionRef");
  return *function;
}

FunctionRef mappedMethod(t::ParserRuntime& runtime, std::u16string_view name) {
  const auto map = std::dynamic_pointer_cast<t::AbstractParseMap>(
      runtime.mapHandler.getMap(Property("html_macros")));
  if (!map)
    throw std::runtime_error("HTML CommandMap is unavailable");
  const auto lookup = map->lookup(Property(property_host::debugText(name)));
  const auto* macro = std::get_if<t::MacroRef>(&lookup);
  if (!macro || !*macro)
    throw std::runtime_error("HTML command map entry is not a Macro");
  return functions::reference((*macro)->func());
}

suite htmlMethodsIdentity = [] {
  "html_method_tables_are_canonical_exports_shared_with_command_map"_test = [] {
    Graph graph;
    auto& runtime = graph.runtime;
    const auto configuration = t::HtmlConfiguration::initializeModule(runtime);
    const Property methods = t::HtmlMethods::initializeModule(runtime);
    const Property repeated = t::HtmlMethods::initializeModule(runtime);
    expect(methods == repeated);
    expect(t::HtmlConfiguration::initializeModule(runtime) == configuration);
    expect(property_host::ownKeys(methods) ==
           std::vector<std::string>({"data", "href", "class", "style", "cssId"}));

    const FunctionRef data = method(methods, u"data");
    const FunctionRef href = method(methods, u"href");
    const FunctionRef className = method(methods, u"class");
    const FunctionRef style = method(methods, u"style");
    const FunctionRef cssId = method(methods, u"cssId");
    expect(mappedMethod(runtime, u"data") == data);
    expect(mappedMethod(runtime, u"href") == href);
    expect(mappedMethod(runtime, u"class") == className);
    expect(mappedMethod(runtime, u"style") == style);
    expect(mappedMethod(runtime, u"cssId") == cssId);

    const auto replacement = std::make_shared<Function>();
    replacement->call = [](const FunctionReceiver&, FunctionArgs) -> std::any {
      return std::monostate{};
    };
    property_host::set(methods, "href", Property(replacement));
    expect(t::HtmlMethods::initializeModule(runtime) == methods);
    expect(method(t::HtmlMethods::initializeModule(runtime), u"href") == replacement);
    expect(mappedMethod(runtime, u"href") == href);

    Graph other;
    const Property otherMethods = t::HtmlMethods::initializeModule(other.runtime);
    expect(otherMethods != methods);
    expect(method(otherMethods, u"data") != data);
    (void)t::HtmlConfiguration::initializeModule(other.runtime);
    expect(mappedMethod(other.runtime, u"data") ==
           method(otherMethods, u"data"));
  };

  "texhtml_method_table_is_canonical_per_runtime"_test = [] {
    Graph graph;
    auto resolver = [](o::ObjectRef) -> std::shared_ptr<AbstractDOMAdaptor> {
      return {};
    };
    const Property methods = t::TexHtmlMethods::initializeModule(
        graph.runtime, resolver);
    const FunctionRef texHtml = method(methods, u"TexHTML");
    expect(t::TexHtmlMethods::initializeModule(graph.runtime, resolver) == methods);
    expect(method(t::TexHtmlMethods::initializeModule(graph.runtime, resolver),
                  u"TexHTML") == texHtml);

    Graph other;
    const Property otherMethods = t::TexHtmlMethods::initializeModule(
        other.runtime, resolver);
    expect(otherMethods != methods);
    expect(method(otherMethods, u"TexHTML") != texHtml);
  };
};
} // namespace
