// Native observer for the pinned MathJax Bussproofs package corpus.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.bussproofs.configuration;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.token;
import folia.math.core.math_item;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.output.common.font_data_surface;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.property_host;
import folia.math.util.map_bindings;
import folia.math.util.option_functions;

namespace {
namespace tex = folia::math::tex;
namespace opts = folia::math::options;
namespace svg = folia::math::output::svg;
namespace common = folia::math::output::common;
namespace handlers = folia::math::handlers;
using namespace folia::math;

std::string q(std::string_view value) {
  std::string out = "\"";
  constexpr char h[] = "0123456789abcdef";
  for (unsigned char c : value) {
    switch (c) {
    case '\\':
      out += "\\\\";
      break;
    case '"':
      out += "\\\"";
      break;
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    default:
      if (c < 32) {
        out += "\\u00";
        out += h[c >> 4];
        out += h[c & 15];
      } else
        out += static_cast<char>(c);
    }
  }
  return out + '"';
}
std::string text(std::u16string_view value) {
  return property_host::debugText(value);
}
std::string num(double value) {
  if (std::isnan(value))
    return "{\"$\":\"NaN\"}";
  if (std::isinf(value))
    return value < 0 ? "{\"$\":\"-Infinity\"}" : "{\"$\":\"Infinity\"}";
  if (value == 0 && std::signbit(value))
    return "{\"$\":\"-0\"}";
  std::ostringstream out;
  out << std::setprecision(17) << value;
  return out.str();
}
struct Serializer {
  opts::Context *context = nullptr;
  OptionFunctionBindings *functions = nullptr;
  tex::ParserRuntime *runtime = nullptr;
  std::string value(const opts::Value &v,
                    std::vector<opts::ObjectRef> *seen = nullptr) {
    if (std::holds_alternative<opts::Undefined>(v.data))
      return "{\"$\":\"undefined\"}";
    if (std::holds_alternative<opts::Null>(v.data))
      return "null";
    if (const auto *b = std::get_if<bool>(&v.data))
      return *b ? "true" : "false";
    if (const auto *n = std::get_if<double>(&v.data))
      return num(*n);
    if (const auto *s = std::get_if<std::u16string>(&v.data))
      return q(text(*s));
    if (std::holds_alternative<opts::SymbolRef>(v.data))
      return "{\"$\":\"symbol\"}";
    const auto *object = std::get_if<opts::ObjectRef>(&v.data);
    if (!object)
      return "null";
    if (!context)
      throw std::runtime_error("Context is required for object serialization");
    if (context->kind(*object) == opts::ObjectKind::function) {
      const auto name = context->get(*object, opts::Key(u"name"));
      const auto length = context->get(*object, opts::Key(u"length"));
      const auto *n = std::get_if<std::u16string>(&name.data);
      const auto *l = std::get_if<double>(&length.data);
      return "{\"$\":\"function\",\"name\":" + q(n ? text(*n) : "") +
             ",\"length\":" + num(l ? *l : 0) + "}";
    }
    struct ActiveObject {
      std::vector<opts::ObjectRef> *stack;
      ~ActiveObject() {
        if (stack)
          stack->pop_back();
      }
    } active{nullptr};
    if (seen) {
      if (std::find(seen->begin(), seen->end(), *object) != seen->end())
        return "{\"$\":\"cycle\"}";
      seen->push_back(*object);
      active.stack = seen;
    }
    if (context->kind(*object) == opts::ObjectKind::array) {
      const auto length = context->arrayLength(*object);
      std::string out = "[";
      for (std::uint32_t i = 0; i < length; ++i) {
        if (i)
          out += ',';
        const auto index = std::to_string(i);
        const opts::Key k(std::u16string(index.begin(), index.end()));
        out += context->hasOwn(*object, k)
                   ? value(context->get(*object, k), seen)
                   : "{\"$\":\"hole\"}";
      }
      return out + "]";
    }
    if (const auto *map =
            std::any_cast<opts::MapStorageRef>(&context->payload(*object));
        map && *map) {
      std::string out = "{\"$\":\"map\",\"entries\":[";
      bool first = true;
      for (const auto &entry : (*map)->entries()) {
        if (!entry.live)
          continue;
        if (!first)
          out += ',';
        first = false;
        out += '[';
        out += value(entry.key, seen) + ',' + value(entry.value, seen) + ']';
      }
      return out + "]}";
    }
    std::string out = "{";
    bool first = true;
    for (const auto &key : context->ownKeys(opts::Value(*object))) {
      if (!first)
        out += ',';
      first = false;
      out += q(text(context->keyString(key))) + ":" +
             value(context->get(opts::Value(*object), key), seen);
    }
    return out + "}";
  }
  std::string packageData(tex::TeX &jax) {
    const auto options = jax.parseOptions();
    const auto *parsed = std::get_if<tex::ParseOptionsRef>(&options);
    if (!parsed || !*parsed)
      return "null";
    const auto data = (*parsed)->packageData;
    const auto *map =
        std::get_if<std::shared_ptr<SourceStringMap<opts::Value>>>(&data);
    if (!map || !*map)
      return "null";
    std::vector<opts::ObjectRef> seen;
    std::string out = "{\"$\":\"map\",\"entries\":[";
    bool first = true;
    for (auto it = (*map)->keys(); auto keyValue = it.next();) {
      if (!first)
        out += ',';
      first = false;
      out += '[' + q(text(*keyValue)) + ',' +
             value((*map)->get(*keyValue).value(), &seen) + ']';
    }
    return out + "]}";
  }
  std::string property(const Property &p) {
    if (std::holds_alternative<Missing>(p.value))
      return "{\"$\":\"undefined\"}";
    if (std::holds_alternative<PropertyNull>(p.value))
      return "null";
    if (const auto *v = std::get_if<bool>(&p.value))
      return *v ? "true" : "false";
    if (const auto *v = std::get_if<double>(&p.value))
      return num(*v);
    if (const auto *v = std::get_if<std::string>(&p.value))
      return q(*v);
    if (std::holds_alternative<options::SymbolRef>(p.value))
      return "{\"$\":\"symbol\"}";
    if (const auto *f = std::get_if<FunctionRef>(&p.value)) {
      if (!functions)
        throw std::runtime_error("FunctionRef serializer has no bindings");
      const auto identity = functions->find(*f);
      if (!identity)
        throw std::runtime_error("Unbound FunctionRef in token map");
      const auto name = context->get(*identity, opts::Key(u"name")),
                 length = context->get(*identity, opts::Key(u"length"));
      const auto *n = std::get_if<std::u16string>(&name.data);
      const auto *l = std::get_if<double>(&length.data);
      if (!n || !l)
        throw std::runtime_error("FunctionRef metadata missing");
      return "{\"$\":\"function\",\"name\":" + q(text(*n)) +
             ",\"length\":" + num(*l) + "}";
    }
    if (std::holds_alternative<BuiltinProperty>(p.value))
      return "{\"$\":\"builtin\"}";
    const auto storage = std::get<std::shared_ptr<PropertyStorage>>(p.value);
    if (!storage)
      return "null";
    if (storage->array) {
      auto a = PropertyArray::array(p);
      std::string out = "[";
      for (std::size_t i = 0; i < a->size(); ++i) {
        if (i)
          out += ',';
        out += a->has(i) ? property(a->get(i)) : "{\"$\":\"hole\"}";
      }
      return out + "]";
    }
    auto r = PropertyList::record(p);
    if (!r)
      return "null";
    return record(*r);
  }
  std::string record(const PropertyList &value) {
    std::string out = "{";
    bool first = true;
    for (const auto &k : value.keys()) {
      if (!first)
        out += ',';
      first = false;
      out += q(k) + ":" + property(value.getOwn(k));
    }
    return out + "}";
  }
  std::string lookup(const tex::TokenLookup &item) {
    if (const auto *p = std::get_if<Property>(&item))
      return property(*p);
    if (const auto *m = std::get_if<tex::MacroRef>(&item)) {
      if (!*m)
        throw std::runtime_error("null macro in token map");
      const auto method = (*m)->func();
      const auto fn =
          std::get_if<FunctionRef>(&method)
              ? property(Property(*std::get_if<FunctionRef>(&method)))
              : property(std::get<Property>(method));
      return "{\"_token\":" + property((*m)->token()) + ",\"_func\":" + fn +
             ",\"_args\":" + property((*m)->args()) + "}";
    }
    if (const auto *t = std::get_if<tex::TokenRef>(&item)) {
      if (!*t)
        throw std::runtime_error("null token in token map");
      return "{\"_token\":" + property((*t)->token()) +
             ",\"_char\":" + property((*t)->character()) +
             ",\"_attributes\":" + property((*t)->attributes()) + "}";
    }
    throw std::runtime_error("unsupported token map lookup variant");
  }
  std::string tokenMap(const tex::TokenMapRef &source) {
    const auto parsed =
        std::dynamic_pointer_cast<tex::AbstractParseMap>(source);
    if (!parsed)
      throw std::runtime_error("registered map is not an AbstractParseMap");
    const auto &entries = parsed->mapStorage();
    std::string out = "{\"$\":\"map\",\"entries\":[";
    bool first = true;
    if (entries)
      for (const auto &[key, value] : *entries) {
        if (!first)
          out += ',';
        first = false;
        out += '[' + property(key) + ',' + lookup(value) + ']';
      }
    return out + "]}";
  }
  std::string maps() {
    if (!runtime)
      throw std::runtime_error("ParserRuntime is required for map observation");
    constexpr std::string_view names[]{"Bussproofs-macros",
                                       "Bussproofs-environments"};
    std::string out = "{";
    bool first = true;
    for (const auto name : names) {
      const auto map = runtime->mapHandler.getMap(Property(std::string(name)));
      if (!map)
        continue;
      if (!first)
        out += ',';
      first = false;
      out += q(name) + ":" + tokenMap(map);
    }
    return out + "}";
  }
  std::string node(mml::NodeValue value) {
    if (!value.isNode())
      return "null";
    auto *n = value.requireNode();
    const auto attrs = n->attributes();
    std::string out = "{\"kind\":" + q(n->kind());
    out += ",\"attrs\":" +
           record(attrs ? attrs->getAllAttributes() : PropertyList{}) +
           ",\"properties\":" + record(n->getAllProperties());
    if (const auto *textNode = dynamic_cast<mml::TextNode *>(n))
      out += ",\"text\":" + q(text(textNode->getText()));
    else if (const auto *token = dynamic_cast<mml::AbstractMmlTokenNode *>(n))
      out += ",\"text\":" + q(text(token->getText()));
    out += ",\"children\":[";
    const auto children = n->childNodes;
    if (children)
      for (std::size_t i = 0; i < children->size(); ++i) {
        if (i)
          out += ',';
        out += node(children->get(i));
      }
    return out + "]}";
  }
};
opts::Value strings(opts::Context &cx,
                    std::span<const std::u16string_view> values) {
  const auto a = cx.array(static_cast<std::uint32_t>(values.size()));
  std::uint32_t i = 0;
  for (auto s : values) {
    const auto n = std::to_string(i++);
    cx.set(a, opts::Key(std::u16string(n.begin(), n.end())),
           opts::Value(std::u16string(s)));
  }
  return a;
}
struct Fixture {
  std::unique_ptr<svg::NativeRenderSession> session;
  opts::Context &cx;
  OptionFunctionBindings &functions;
  tex::ParserRuntime &runtime;
  std::shared_ptr<mml::MmlFactory> factory;
  std::shared_ptr<handlers::html::HTMLMathDocument> document;
  tex::TeXRef inputJax;
  opts::ObjectRef inputJaxIdentity{};
  opts::ObjectRef proofTreeClass{};
  opts::ObjectRef packageConfiguration{};
  Fixture()
      : session([&] {
          common::FontLoadHost host;
          host.asyncIsSynchronous = [] { return true; };
          host.warn = [](const std::any &) {};
          host.syncLoad = [](common::FontData &, const common::DynamicFileRef &,
                             std::string) {
            throw opts::Exception(
                "Unimplemented",
                u"Bussproofs observer has no NewCM source provider");
          };
          host.asyncLoad = [](std::shared_ptr<common::FontData>,
                              common::DynamicFileRef,
                              std::string) -> async::PromiseRef {
            throw opts::Exception(
                "Unimplemented",
                u"Bussproofs observer has no NewCM source provider");
          };
          return std::make_unique<svg::NativeRenderSession>(
              std::move(host),
              svg::NewcmFontProfile{svg::NewcmFontExtension::bboldx,
                                    svg::NewcmFontExtension::bbm,
                                    svg::NewcmFontExtension::dsfont});
        }()),
        cx(session->context()), functions(session->services().functions()),
        runtime(session->services().parser()), factory(session->mathFactory()) {
    (void)tex::BasePackage::initializeModule(runtime);
    const auto package = tex::BussproofsConfiguration::initializeModule(
        runtime, session->services().modules());
    const auto packageIdentity = runtime.configurationIdentities.find(package);
    if (!packageIdentity)
      throw std::runtime_error("Bussproofs configuration identity missing");
    packageConfiguration = *packageIdentity;
    const auto itemMap =
        cx.get(opts::Value(*packageIdentity), opts::Key(u"items"));
    const auto proofTree = cx.get(itemMap, opts::Key(u"proofTree"));
    const auto *classIdentity = std::get_if<opts::ObjectRef>(&proofTree.data);
    if (!classIdentity)
      throw std::runtime_error("ProofTreeItem configuration entry missing");
    proofTreeClass = *classIdentity;
    const auto config = cx.object(), packages = cx.array(2);
    cx.set(packages, opts::Key(u"0"), opts::Value(u"base"));
    cx.set(packages, opts::Key(u"1"), opts::Value(u"bussproofs"));
    cx.set(config, opts::Key(u"packages"), opts::Value(packages));
    const auto outputOptions = cx.object();
    cx.set(outputOptions, opts::Key(u"fontCache"), opts::Value(u"none"));
    cx.set(outputOptions, opts::Key(u"exFactor"), opts::Value(0.5));
    (void)session->construct(opts::Value(outputOptions));
    document = svg::createNativeSvgDocument(
        *session, opts::Value(opts::Undefined{}), opts::Value(config));
    const opts::Value documentValue(document->sourceIdentity());
    const auto inputJaxArray = cx.get(documentValue, opts::Key(u"inputJax"));
    const auto inputJaxValue = cx.get(inputJaxArray, opts::Key(u"0"));
    const auto *identity = std::get_if<opts::ObjectRef>(&inputJaxValue.data);
    if (!identity)
      throw std::runtime_error("Document input Jax identity is unavailable");
    inputJaxIdentity = *identity;
    inputJax = runtime.texIdentities.resolve(inputJaxIdentity);
    if (!inputJax)
      throw std::runtime_error("Document input Jax owner is unavailable");
  }
  mml::NodeValue compile(std::u16string_view source) {
    const opts::Value documentValue(document->sourceIdentity());
    const auto options = cx.get(documentValue, opts::Key(u"options"));
    const auto constructor = cx.get(options, opts::Key(u"MathItem"));
    const auto *itemConstructor =
        std::get_if<opts::ObjectRef>(&constructor.data);
    if (!itemConstructor)
      throw std::runtime_error("Document MathItem constructor is unavailable");
    const std::array<opts::Value, 3> arguments{
        opts::Value(std::u16string(source)),
        opts::Value(inputJaxIdentity), opts::Value(true)};
    const opts::ObjectRef itemIdentity = cx.construct(*itemConstructor, arguments);
    const auto *native =
        std::any_cast<std::shared_ptr<NativeObject>>(&cx.payload(itemIdentity));
    const auto item = native ? std::dynamic_pointer_cast<core::MathItem>(*native)
                             : nullptr;
    if (!item)
      throw std::runtime_error("Document MathItem has no canonical native owner");
    return inputJax->compile(*item, core::MathDocumentReference{document.get()});
  }
};
struct Case {
  const char *name;
  std::u16string_view input;
};
constexpr Case cases[] = {
    {"proof-axiom-short",
     uR"bsp(\begin{prooftree}\AxiomC{A}\end{prooftree})bsp"},
    {"proof-axiom-alias", uR"bsp(\begin{prooftree}\AXC{A}\end{prooftree})bsp"},
    {"proof-unary",
     uR"bsp(\begin{prooftree}\AxiomC{A}\UnaryInfC{B}\end{prooftree})bsp"},
    {"proof-binary",
     uR"bsp(\begin{prooftree}\AxiomC{A}\AxiomC{B}\BinaryInfC{C}\end{prooftree})bsp"},
    {"proof-ternary",
     uR"bsp(\begin{prooftree}\AxiomC{A}\AxiomC{B}\AxiomC{C}\TrinaryInfC{D}\end{prooftree})bsp"},
    {"proof-quaternary",
     uR"bsp(\begin{prooftree}\AxiomC{A}\AxiomC{B}\AxiomC{C}\AxiomC{D}\QuaternaryInfC{E}\end{prooftree})bsp"},
    {"proof-quinary",
     uR"bsp(\begin{prooftree}\AxiomC{A}\AxiomC{B}\AxiomC{C}\AxiomC{D}\AxiomC{E}\QuinaryInfC{F}\end{prooftree})bsp"},
    {"proof-unary-alias",
     uR"bsp(\begin{prooftree}\AXC{A}\UIC{B}\end{prooftree})bsp"},
    {"proof-binary-alias",
     uR"bsp(\begin{prooftree}\AXC{A}\AXC{B}\BIC{C}\end{prooftree})bsp"},
    {"proof-ternary-alias",
     uR"bsp(\begin{prooftree}\AXC{A}\AXC{B}\AXC{C}\TIC{D}\end{prooftree})bsp"},
    {"proof-labels-left-right",
     uR"bsp(\begin{prooftree}\AxiomC{A}\LeftLabel{l}\RightLabel{r}\UnaryInfC{B}\end{prooftree})bsp"},
    {"proof-label-left-alias",
     uR"bsp(\begin{prooftree}\AXC{A}\LL{L}\UIC{B}\end{prooftree})bsp"},
    {"proof-label-right-alias",
     uR"bsp(\begin{prooftree}\AXC{A}\RL{R}\UIC{B}\end{prooftree})bsp"},
    {"proof-no-line",
     uR"bsp(\begin{prooftree}\AxiomC{A}\noLine\UnaryInfC{B}\end{prooftree})bsp"},
    {"proof-single-line",
     uR"bsp(\begin{prooftree}\AxiomC{A}\singleLine\UnaryInfC{B}\end{prooftree})bsp"},
    {"proof-dashed-line",
     uR"bsp(\begin{prooftree}\AxiomC{A}\dashedLine\UnaryInfC{B}\end{prooftree})bsp"},
    {"proof-persistent-dashed",
     uR"bsp(\begin{prooftree}\alwaysDashedLine\AxiomC{A}\UnaryInfC{B}\UnaryInfC{C}\end{prooftree})bsp"},
    {"proof-root-at-top",
     uR"bsp(\begin{prooftree}\rootAtTop\AxiomC{A}\UnaryInfC{B}\end{prooftree})bsp"},
    {"proof-root-at-bottom",
     uR"bsp(\begin{prooftree}\rootAtBottom\AxiomC{A}\UnaryInfC{B}\end{prooftree})bsp"},
    {"proof-always-root-at-top",
     uR"bsp(\begin{prooftree}\alwaysRootAtTop\AxiomC{A}\UnaryInfC{B}\end{prooftree})bsp"},
    {"proof-fcenter-axiom",
     uR"bsp(\begin{prooftree}\Axiom$A\fCenter B$\end{prooftree})bsp"},
    {"proof-fcenter-extra-separator",
     uR"bsp(\begin{prooftree}\Axiom$A\fCenter B\fCenter C$\end{prooftree})bsp"},
    {"proof-fcenter-inference",
     uR"bsp(\begin{prooftree}\Axiom$A\fCenter B$\UnaryInf$B\fCenter C$\end{prooftree})bsp"},
    {"proof-recursive-binary",
     uR"bsp(\begin{prooftree}\AxiomC{A}\AxiomC{B}\BinaryInfC{A\land B}\AxiomC{C}\BinaryInfC{(A\land B)\land C}\end{prooftree})bsp"},
    {"proof-leftover-formula",
     uR"bsp(\begin{prooftree}\AxiomC{A}x\end{prooftree})bsp"},
    {"proof-command-outside", uR"bsp(\AxiomC{A})bsp"},
    {"proof-inference-too-few",
     uR"bsp(\begin{prooftree}\AxiomC{A}\BinaryInfC{B}\end{prooftree})bsp"},
    {"proof-unknown-command",
     uR"bsp(\begin{prooftree}\BadProofCommand{A}\end{prooftree})bsp"},
    {"proof-missing-end", uR"bsp(\begin{prooftree}\AxiomC{A})bsp"},
    {"proof-fcenter-missing",
     uR"bsp(\begin{prooftree}\Axiom$A=B$\end{prooftree})bsp"},
    {"proof-fcenter-no-math-shift",
     uR"bsp(\begin{prooftree}\Axiom A\fCenter B\end{prooftree})bsp"},
};
std::string descriptors(Serializer &serializer, opts::ObjectRef object) {
  auto &cx = *serializer.context;
  std::string out = "[";
  bool first = true;
  for (const auto &key : cx.ownKeys(opts::Value(object))) {
    if (!first)
      out += ',';
    first = false;
    const auto descriptor = cx.descriptor(opts::Value(object), key);
    if (!descriptor)
      throw std::runtime_error("missing own descriptor");
    out += "{\"name\":" + q(text(cx.keyString(key))) +
           ",\"enumerable\":" + (descriptor->enumerable ? "true" : "false") +
           ",\"configurable\":" + (descriptor->configurable ? "true" : "false");
    if (descriptor->accessor) {
      if (descriptor->getter)
        out += ",\"get\":" + serializer.value(opts::Value(*descriptor->getter));
      if (descriptor->setter)
        out += ",\"set\":" + serializer.value(opts::Value(*descriptor->setter));
    } else {
      out += ",\"writable\":";
      out += descriptor->writable ? "true" : "false";
      out += ",\"value\":";
      out += serializer.value(descriptor->value);
    }
    out += '}';
  }
  return out + "]";
}
std::string surface(Fixture &fixture, Serializer &serializer, tex::TeX &jax) {
  auto &cx = fixture.cx;
  const opts::ObjectRef constructor = fixture.proofTreeClass;
  const auto protoValue =
      cx.get(opts::Value(constructor), opts::Key(u"prototype"));
  const auto *proto = std::get_if<opts::ObjectRef>(&protoValue.data);
  if (!proto)
    throw std::runtime_error("ProofTreeItem prototype missing");
  const auto parseOptions = jax.parseOptions();
  const auto *options = std::get_if<tex::ParseOptionsRef>(&parseOptions);
  if (!options || !*options)
    throw std::runtime_error("TeX ParseOptions are unavailable");
  const auto *factory =
      std::get_if<std::shared_ptr<tex::StackItemFactory>>(&(*options)->itemFactory);
  if (!factory || !*factory)
    throw std::runtime_error("TeX StackItemFactory is unavailable");
  const auto factoryIdentity = fixture.runtime.stackItemFactoryIdentities.find(
      factory->get());
  if (!factoryIdentity)
    throw std::runtime_error("TeX StackItemFactory identity is unavailable");
  const std::array<opts::Value, 1> constructorArguments{
      opts::Value(*factoryIdentity)};
  const opts::Value instance(cx.construct(constructor, constructorArguments));
  std::string out =
      "{\"configuration\":" +
      serializer.value(opts::Value(fixture.packageConfiguration)) +
      ",\"itemClass\":{";
  out += "\"constructor\":" + serializer.value(opts::Value(constructor));
  out += ",\"static\":" + descriptors(serializer, constructor);
  out += ",\"prototype\":" + descriptors(serializer, *proto);
  out += ",\"instanceFields\":[{\"name\":\"leftLabel\",\"value\":" +
         serializer.value(cx.get(instance, opts::Key(u"leftLabel"))) +
         "},{\"name\":\"rigthLabel\",\"value\":" +
         serializer.value(cx.get(instance, opts::Key(u"rigthLabel"))) + "}]}}";
  return out;
}
} // namespace
int main() {
  try {
    Fixture fixture;
    Serializer serializer{&fixture.cx, &fixture.functions, &fixture.runtime};
    std::cout << "{\"schema\":1,\"source\":\"MathJax-src 4.1.3 "
                 "fb987178c2d279a99b0db5ea02e751691703955e\",\"files\":["
              << "{\"ts\":\"ts/input/tex/bussproofs/"
                 "BussproofsConfiguration.ts\",\"tsSha256\":"
                 "\"2aefb3c006598ccc5ef08a4f5d5e2b596aec048737f8e2aaf571cf63c2f"
                 "075df\",\"cjs\":\"cjs/input/tex/bussproofs/"
                 "BussproofsConfiguration.js\",\"cjsSha256\":"
                 "\"249ab0f53fb4ed0c4bcabe8bc20ba39be5c65dc6ab0912acd252b202261"
                 "310c9\"},"
              << "{\"ts\":\"ts/input/tex/bussproofs/"
                 "BussproofsItems.ts\",\"tsSha256\":"
                 "\"133a085cbeea74c316cee57432876e15c2142b5cc54c5625cb108d9a950"
                 "f1a7f\",\"cjs\":\"cjs/input/tex/bussproofs/"
                 "BussproofsItems.js\",\"cjsSha256\":"
                 "\"e1b0a23de54c9ed334e30d1c1e679c6dcde24a2d1558ac7645abc21a041"
                 "ab13a\"},"
              << "{\"ts\":\"ts/input/tex/bussproofs/"
                 "BussproofsMappings.ts\",\"tsSha256\":"
                 "\"76ef61358cac14baa65bfcfcaf7d862d2e068fbac89d48f2dc9d5e60a58"
                 "e9c9c\",\"cjs\":\"cjs/input/tex/bussproofs/"
                 "BussproofsMappings.js\",\"cjsSha256\":"
                 "\"237e8be03370951f7dac389d79980454d930fcb82be5d4d821dc93e95fe"
                 "8aff9\"},"
              << "{\"ts\":\"ts/input/tex/bussproofs/"
                 "BussproofsMethods.ts\",\"tsSha256\":"
                 "\"25e6a7c51def3f5912df78d8822b6d92b370c9cc9b472f9ffc852c357b7"
                 "727e5\",\"cjs\":\"cjs/input/tex/bussproofs/"
                 "BussproofsMethods.js\",\"cjsSha256\":"
                 "\"c671b2b31b229e73454c70c794a82923f8d059f43b5f1af134588b53005"
                 "5c054\"},"
              << "{\"ts\":\"ts/input/tex/bussproofs/"
                 "BussproofsUtil.ts\",\"tsSha256\":"
                 "\"f6e3f1f95cd8e0961f3ddbf28c4bf1d4f3f0b3c30e488f68ae125341014"
                 "992bd\",\"cjs\":\"cjs/input/tex/bussproofs/"
                 "BussproofsUtil.js\",\"cjsSha256\":"
                 "\"8ed15ce9672065e5d4ae2e3f06703bbb456f836287fa1fb6a0aa45ecfb1"
                 "a305d\"}],\"surface\":"
              << surface(fixture, serializer, *fixture.inputJax)
              << ",\"cases\":[";
    bool first = true;
    for (const auto &test : cases) {
      if (!first)
        std::cout << ',';
      first = false;
      std::cout << "{\"name\":" << q(test.name)
                << ",\"input\":" << q(text(test.input)) << ",\"result\":";
      try {
        std::cout << serializer.node(fixture.compile(test.input))
                  << ",\"error\":null";
      } catch (const tex::TexError &e) {
        std::cout << "null,\"error\":{\"name\":\"TexError\",\"id\":"
                  << q(text(e.id)) << ",\"message\":" << q(text(e.message))
                  << "}";
      } catch (const opts::Exception &e) {
        std::cout << "null,\"error\":{\"name\":" << q(e.name)
                  << ",\"id\":null,\"message\":" << q(text(e.message)) << "}";
      } catch (const std::exception &e) {
        std::cout << "null,\"error\":{\"name\":\"NativeException\",\"id\":null,"
                     "\"message\":"
                  << q(e.what()) << "}";
      } catch (...) {
        std::cout << "null,\"error\":{\"name\":\"UnclassifiedNativeException\","
                     "\"id\":null,\"message\":\"\"}";
      }
      std::cout << ",\"packageData\":"
                << serializer.packageData(*fixture.inputJax) << "}";
    }
    std::cout << "],\"maps\":" << serializer.maps() << "}\n";
    return 0;
  } catch (const opts::Exception &e) {
    std::cerr << "fixture-init " << e.name << ": " << text(e.message) << '\n';
  } catch (const std::exception &e) {
    std::cerr << "fixture-init NativeException: " << e.what() << '\n';
  } catch (...) {
    std::cerr << "fixture-init UnclassifiedNativeException\n";
  }
  return 2;
}
