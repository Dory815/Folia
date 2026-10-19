// Native observations for the pinned MathJax Mathtools package corpus.
// The output includes all 42 Source-pinned parser cases and all five maps.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.ams.configuration;
import folia.math.input.tex.mathtools.methods;
import folia.math.input.tex.mathtools.mappings;
import folia.math.input.tex.mathtools.items;
import folia.math.input.tex.mathtools.tags;
import folia.math.input.tex.mathtools.util;
import folia.math.input.tex.mathtools.configuration;
import folia.math.input.tex.extpfeil.configuration;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.token;
import folia.math.core.math_item;
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
import folia.math.output.svg.native_document;
import folia.math.output.common.font_data;

namespace {
namespace tex = folia::math::tex;
namespace opts = folia::math::options;
namespace svg = folia::math::output::svg;
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
  std::string activeOptions(tex::TeX &jax) {
    const auto shared = jax.parseOptions();
    const auto *parsed = std::get_if<tex::ParseOptionsRef>(&shared);
    if (!parsed || !*parsed)
      return "null";
    return value(context->get((*parsed)->options, opts::Key(u"mathtools")));
  }
  std::string activeTags(tex::TeX &jax) {
    const auto shared = jax.parseOptions();
    const auto *parsed = std::get_if<tex::ParseOptionsRef>(&shared);
    if (!parsed || !*parsed)
      return "null";
    return value(context->get((*parsed)->options, opts::Key(u"tags")));
  }
  std::string configuration(tex::ConfigurationRef config) {
    constexpr std::u16string_view keys[]{
        u"name",           u"handler",    u"fallback",     u"items",
        u"tags",           u"options",    u"nodes",        u"preprocessors",
        u"postprocessors", u"initMethod", u"configMethod", u"priority",
        u"parser"};
    std::string out = "{";
    bool first = true;
    for (const auto key : keys) {
      if (!first)
        out += ',';
      first = false;
      out += q(text(key)) + ":" + value(config->field(key));
    }
    return out + "}";
  }
  std::string classShape(opts::ObjectRef constructor) {
    const auto prototypeValue =
        context->get(opts::Value(constructor), opts::Key(u"prototype"));
    const auto *prototype = std::get_if<opts::ObjectRef>(&prototypeValue.data);
    if (!prototype)
      throw std::runtime_error("MultlinedItem constructor has no prototype");
    std::string out = "{\"constructor\":" + value(opts::Value(constructor)) +
                      ",\"prototype\":[";
    bool first = true;
    for (const auto &key : context->ownKeys(opts::Value(*prototype))) {
      if (!first)
        out += ',';
      first = false;
      const auto descriptor = context->descriptor(opts::Value(*prototype), key);
      if (!descriptor)
        throw std::runtime_error(
            "MultlinedItem prototype descriptor disappeared");
      const std::string name = q(text(context->keyString(key)));
      const std::string member = descriptor->accessor
                                     ? "{\"$\":\"undefined\"}"
                                     : value(descriptor->value);
      const std::string getter =
          descriptor->getter ? value(opts::Value(*descriptor->getter)) : "null";
      const std::string setter =
          descriptor->setter ? value(opts::Value(*descriptor->setter)) : "null";
      out += "{\"name\":" + name + ",\"value\":" + member +
             ",\"get\":" + getter + ",\"set\":" + setter + "}";
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
  std::string maps(tex::ParserRuntime& runtime) {
    constexpr std::string_view names[]{
        "mathtools-macros", "mathtools-legacycolonsymbols",
        "mathtools-environments", "mathtools-delimiters",
        "mathtools-characters"};
    std::string out = "{";
    bool first = true;
    for (const auto name : names) {
      const auto map = runtime.mapHandler.getMap(Property(std::string(name)));
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
  bool packageOrder = false;
  svg::NativeRenderSession session{fontHost()};
  opts::Context &cx;
  OptionFunctionBindings &functions;
  tex::ParserRuntime &runtime;
  std::unordered_map<const tex::TeX *,
                     std::shared_ptr<folia::math::handlers::html::HTMLMathDocument>>
      documents;
  Property tagExports;
  static folia::math::output::common::FontLoadHost fontHost() {
    folia::math::output::common::FontLoadHost host;
    host.asyncIsSynchronous = [] { return true; };
    host.warn = [](const std::any &) {};
    return host;
  }
  explicit Fixture(bool usePackageOrder = false)
      : packageOrder(usePackageOrder), cx(session.context()),
        functions(session.services().functions()),
        runtime(session.services().parser()) {
    (void)session.construct();
    tagExports = tex::MathtoolsTags::moduleExports(runtime);
    const Property repeatedTagExports =
        tex::MathtoolsTags::moduleExports(runtime);
    const Property tagFormat = property_host::get(
        repeatedTagExports, property_host::debugText(u"MathtoolsTagFormat"));
    const auto *tagFunction = std::get_if<FunctionRef>(&tagFormat.value);
    const Property firstTagFormat = property_host::get(
        tagExports, property_host::debugText(u"MathtoolsTagFormat"));
    const auto *firstTagFunction =
        std::get_if<FunctionRef>(&firstTagFormat.value);
    if (!tagFunction || !firstTagFunction || *tagFunction != *firstTagFunction)
      throw std::runtime_error("MathtoolsTags module export identity changed");
    (void)tex::BasePackage::initializeModule(runtime);
    (void)tex::AmsConfiguration::initializeModule(runtime);
    if (packageOrder)
      (void)tex::ExtpfeilConfiguration::initializeModule(runtime);
    (void)tex::MathtoolsConfiguration::initializeModule(runtime);
  }
  tex::TeXRef jax() {
    const auto config = cx.object();
    const auto packageArray = cx.array(packageOrder ? 4 : 3);
    cx.set(packageArray, opts::Key(u"0"), opts::Value(std::u16string(u"base")));
    cx.set(packageArray, opts::Key(u"1"), opts::Value(std::u16string(u"ams")));
    if (packageOrder) {
      cx.set(packageArray, opts::Key(u"2"),
             opts::Value(std::u16string(u"extpfeil")));
      cx.set(packageArray, opts::Key(u"3"),
             opts::Value(std::u16string(u"mathtools")));
    } else
      cx.set(packageArray, opts::Key(u"2"),
             opts::Value(std::u16string(u"mathtools")));
    cx.set(config, opts::Key(u"packages"), opts::Value(packageArray));
    auto document = svg::createNativeSvgDocument(
        session, opts::Undefined{}, opts::Value(config));
    const opts::Value documentValue(document->sourceIdentity());
    const auto inputJaxArray =
        cx.get(documentValue, opts::Key(u"inputJax"));
    const auto inputJaxValue = cx.get(inputJaxArray, opts::Key(u"0"));
    const auto *identity = std::get_if<opts::ObjectRef>(&inputJaxValue.data);
    if (!identity)
      throw std::runtime_error("Document input Jax identity is unavailable");
    auto result = runtime.texIdentities.resolve(*identity);
    result->setMmlFactory(session.mathFactory());
    documents.emplace(result.get(), std::move(document));
    return result;
  }
  mml::NodeValue compile(std::u16string_view source, tex::TeX &jax) {
    const auto found = documents.find(&jax);
    if (found == documents.end())
      throw std::runtime_error("Mathtools HTMLMathDocument owner is missing");
    auto &document = *found->second;
    const opts::Value documentValue(document.sourceIdentity());
    const auto options = cx.get(documentValue, opts::Key(u"options"));
    const auto constructor = cx.get(options, opts::Key(u"MathItem"));
    const auto *itemConstructor = std::get_if<opts::ObjectRef>(&constructor.data);
    if (!itemConstructor)
      throw opts::Exception(
          "TypeError", u"HTMLMathDocument.options.MathItem is not a constructor");
    const auto inputJax = cx.get(documentValue, opts::Key(u"inputJax"));
    const auto input = cx.get(inputJax, opts::Key(u"0"));
    const std::array<opts::Value, 3> args{
        opts::Value(std::u16string(source)), input, opts::Value(true)};
    const auto identity = cx.construct(*itemConstructor, args);
    const auto *payload =
        std::any_cast<std::shared_ptr<NativeObject>>(&cx.payload(identity));
    auto item = payload ? std::dynamic_pointer_cast<core::MathItem>(*payload)
                        : nullptr;
    if (!item)
      throw opts::Exception("TypeError",
                            u"Mathtools MathItem has no canonical HTML owner");
    return jax.compile(*item, core::MathDocumentReference{&document});
  }
};
struct Case {
  const char *name;
  std::u16string_view input;
};
constexpr Case cases[] = {
    {"mt-matrix", uR"mt(\begin{matrix*}[r]a&b\\c&d\end{matrix*})mt"},
    {"mt-smallmatrix",
     uR"mt(\begin{psmallmatrix*}a&b\\c&d\end{psmallmatrix*})mt"},
    {"mt-multlined", uR"mt(\begin{multlined}[t][4em]a\\b\end{multlined})mt"},
    {"mt-shove",
     uR"mt(\begin{align}a\shoveleft{+b}\\c\shoveright{+d}\end{align})mt"},
    {"mt-spreadlines",
     uR"mt(\begin{spreadlines}{2pt}\begin{matrix}a\\b\end{matrix}\end{spreadlines})mt"},
    {"mt-cases", uR"mt(\begin{dcases}x&x>0\\-x&x<0\end{dcases})mt"},
    {"mt-mathlap", uR"mt(\mathllap{x}+\mathrlap{y}+\mathclap{z})mt"},
    {"mt-cramped", uR"mt(\cramped{\frac{a}{b}})mt"},
    {"mt-textlap", uR"mt(\textllap{left}\textclap{center}\textrlap{right})mt"},
    {"mt-makebox", uR"mt(\mathmakebox[2em][c]{x})mt"},
    {"mt-mbox", uR"mt(\mathmbox{x})mt"},
    {"mt-brackets", uR"mt(\overbracket{x}\underbracket{y})mt"},
    {"mt-refeq", uR"mt(\refeq{eq:x})mt"},
    {"mt-aboxed", uR"mt(\begin{align}\Aboxed{a=b}\end{align})mt"},
    {"mt-aboxed-declare",
     uR"mt(\MakeAboxedCommand{\mybox}\begin{align}\mybox{a=b}\end{align})mt"},
    {"mt-arrow-between-lines",
     uR"mt(\begin{multline}a\\\ArrowBetweenLines\\b\end{multline})mt"},
    {"mt-vdotswithin",
     uR"mt(\begin{matrix}a&b\\\vdotswithin{a}&c\end{matrix})mt"},
    {"mt-shortvdotswithin",
     uR"mt(\begin{matrix}a&b\\\shortvdotswithin{a}&c\end{matrix})mt"},
    {"mt-flush-space",
     uR"mt(\MTFlushSpaceAbove{2pt}\MTFlushSpaceBelow{1pt})mt"},
    {"mt-paired-delimiter",
     uR"mt(\DeclarePairedDelimiter{\norm}{\lVert}{\rVert}\norm{x})mt"},
    {"mt-paired-delimiter-x",
     uR"mt(\DeclarePairedDelimiterX{\set}[1]{\lbrace}{\rbrace}{#1}\set{x})mt"},
    {"mt-paired-delimiter-xpp",
     uR"mt(\DeclarePairedDelimiterXPP{\abs}[1]{\lvert}{#1}{\rvert}{#1}\abs{x})mt"},
    {"mt-paired-delimiter-typo",
     uR"mt(\DeclarePairedDelimiters{\myabs}{|}{|}\myabs{x})mt"},
    {"mt-colon", uR"mt(a\vcentercolon b\ordinarycolon c\MTThinColon d)mt"},
    {"mt-relations",
     uR"mt(a\coloneqq b\Coloneqq c\eqqcolon d\Colonapprox e\colonsim f\dblcolon g\dashcolon h)mt"},
    {"mt-narrow-arrows", uR"mt(\nuparrow\ndownarrow)mt"},
    {"mt-splitfrac", uR"mt(\splitfrac{a+b}{c+d}+\splitdfrac{e}{f})mt"},
    {"mt-xmathstrut", uR"mt(\xmathstrut{(})mt"},
    {"mt-prescript", uR"mt(\prescript{a}{b}{X})mt"},
    {"mt-tagform",
     uR"mt(\newtagform{brackets}{[}{]}\usetagform{brackets}\begin{equation}x\end{equation})mt"},
    {"mt-adjustlimits", uR"mt(\adjustlimits\sum_{a}^{b})mt"},
    {"mt-mathtoolsset",
     uR"mt(\mathtoolsset{centercolon=true,thincolon-dx=.1em}a\coloneqq b)mt"},
    {"mt-xmapsto", uR"mt(\xmapsto{f})mt"},
    {"mt-xmapsto-after-ams", uR"mt(\xmapsto[below]{above})mt"},
    {"mt-cases-env",
     uR"mt(\begin{cases*}x&positive\\-x&negative\end{cases*})mt"},
    {"mt-cramped-array",
     uR"mt(\begin{crampedsubarray}{c}a\\b\end{crampedsubarray})mt"},
    {"mt-gathered", uR"mt(\begin{lgathered}a\\b\end{lgathered})mt"},
    {"mt-delimiter", uR"mt(\left\lparen x\right\rparen)mt"},
    {"mt-invalid-width", uR"mt(\begin{multlined}[t][bogus]x\end{multlined})mt"},
    {"mt-invalid-tag-form", uR"mt(\newtagform{bad}{[}{]})mt"},
    {"mt-undefined-paired", uR"mt(\DeclarePairedDelimiter{\frac}{(}{)}x)mt"},
    {"mt-unknown", uR"mt(\mathtoolsUnknown{x})mt"},
};
} // namespace
int main(int argc, char **argv) {
  try {
    const bool packageOrder =
        argc > 1 && std::string_view(argv[1]) == "--package-order";
    Fixture fixture(packageOrder);
    Serializer serializer{&fixture.cx, &fixture.functions};
    if (packageOrder) {
      const auto map = fixture.runtime.mapHandler.getMap(Property("mathtools-macros"));
      if (!map)
        throw std::runtime_error("Mathtools macro map is not registered");
      const auto override = map->lookup(Property("xmapsto"));
      constexpr Case orderCases[] = {
          {"folia-xmapsto-0", u"\\xmapsto{f}"},
          {"folia-xmapsto-1", u"\\xmapsto[below]{above}"}};
      std::cout << "{\"schema\":1,\"source\":\"MathJax 4.1.3 "
                   "fb987178c2d279a99b0db5ea02e751691703955e\","
                   "\"packages\":[\"base\",\"ams\",\"extpfeil\",\"mathtools\"],"
                   "\"override\":"
                << serializer.lookup(override) << ",\"cases\":[";
      bool first = true;
      for (const auto &test : orderCases) {
        if (!first)
          std::cout << ',';
        first = false;
        const auto jax = fixture.jax();
        std::cout << "{\"name\":" << q(test.name)
                  << ",\"input\":" << q(text(test.input))
                  << ",\"options\":{},\"result\":";
        try {
          std::cout << serializer.node(fixture.compile(test.input, *jax))
                    << ",\"error\":null";
        } catch (const tex::TexError &e) {
          std::cout << "null,\"error\":{\"name\":\"TexError\",\"id\":"
                    << q(text(e.id)) << ",\"message\":" << q(text(e.message))
                    << "}";
        } catch (const opts::Exception &e) {
          std::cout << "null,\"error\":{\"name\":" << q(e.name)
                    << ",\"id\":null,\"message\":" << q(text(e.message)) << "}";
        } catch (const std::exception &e) {
          std::cout
              << "null,\"error\":{\"name\":\"NativeException\",\"id\":null,"
                 "\"message\":"
              << q(e.what()) << "}";
        } catch (...) {
          std::cout
              << "null,\"error\":{\"name\":\"UnclassifiedNativeException\","
                 "\"id\":null,\"message\":\"\"}";
        }
        std::cout << ",\"packageData\":" << serializer.packageData(*jax)
                  << ",\"activeOptions\":" << serializer.activeOptions(*jax)
                  << ",\"activeTags\":" << serializer.activeTags(*jax) << "}";
      }
      std::cout << "]}\n";
      return 0;
    }
    const auto configExport =
        tex::MathtoolsConfiguration::initializeModule(fixture.runtime);
    const auto itemClass =
        tex::MultlinedItem::initializeModule(fixture.runtime);
    const auto itemIdentity =
        fixture.runtime.itemClassIdentities.find(itemClass);
    if (!itemIdentity)
      throw std::runtime_error(
          "MultlinedItem constructor identity is not bound");
    const auto methodsExport =
        tex::MathtoolsMethods::initializeModule(fixture.runtime);
    const auto utilExport = tex::MathtoolsUtil::moduleExports(fixture.runtime);
    const std::string files =
        R"files([{"ts":"ts/input/tex/mathtools/MathtoolsConfiguration.ts","tsSha256":"7b14b1d6e4324397ab315fa012ceada7999df6cbf3b4c9f09fc3742f8b5a30ae","cjs":"cjs/input/tex/mathtools/MathtoolsConfiguration.js","cjsSha256":"863415dfbc384852e5b3b39724d50cc453e1bb6e54f8f6a1073b9e5e43b8734d"},{"ts":"ts/input/tex/mathtools/MathtoolsItems.ts","tsSha256":"61cadb6dcd19a00d7a6e3bbd7b6eb61a9e9fb72edc8982c83890f08f37a013bb","cjs":"cjs/input/tex/mathtools/MathtoolsItems.js","cjsSha256":"89bc6d7f2597a48d231743d06d031c7042f9963798cb514a84dabc52de4e023d"},{"ts":"ts/input/tex/mathtools/MathtoolsMappings.ts","tsSha256":"57522ac1c99b3154cbe19057c61d365e027a41f3946efa9093d3f1f05f28b1df","cjs":"cjs/input/tex/mathtools/MathtoolsMappings.js","cjsSha256":"2bc22670070f957d770adb74bdb0e6795c6373560adadfc62fbe82fec42fc03b"},{"ts":"ts/input/tex/mathtools/MathtoolsMethods.ts","tsSha256":"ad6562e23ba65cafdb480afc33387982c7f7b14a2886ae7844dfbe64ec44e918","cjs":"cjs/input/tex/mathtools/MathtoolsMethods.js","cjsSha256":"ec67f90855dec0ab60f4933beffe081799247a2ac01572429b696e4e02ed8a5a"},{"ts":"ts/input/tex/mathtools/MathtoolsTags.ts","tsSha256":"2437a2beea0f0c04fac8697ab407a09672b2395d483b1942aefa51e9df057e47","cjs":"cjs/input/tex/mathtools/MathtoolsTags.js","cjsSha256":"bcdd1c9f8592d5640fa2b8bb8cade19c29e2d8f732e19a2dfbcb5415135e3e2d"},{"ts":"ts/input/tex/mathtools/MathtoolsUtil.ts","tsSha256":"e8f02143609f1201b5c9c39638d0eb3f14a3d5adcb93fc5a6b4c1e888b3903e0","cjs":"cjs/input/tex/mathtools/MathtoolsUtil.js","cjsSha256":"c132df85a45b50812466c8643f9dabd156f5205e5699d514f01292e02469e2d5"}])files";
    const std::string tagProperties = serializer.property(fixture.tagExports);
    const std::string tagExports =
        "{\"__esModule\":true," +
        tagProperties.substr(1, tagProperties.size() - 2) + "}";
    const std::string exports =
        "{\"configuration\":" + serializer.configuration(configExport) +
        ",\"itemClass\":" + serializer.classShape(*itemIdentity) +
        ",\"mappingExports\":[],\"methods\":" +
        serializer.property(methodsExport) + ",\"tagMethods\":" + tagExports +
        ",\"utilities\":" + serializer.property(utilExport) + "}";
    std::cout << "{\"schema\":1,\"source\":\"MathJax 4.1.3 "
                 "fb987178c2d279a99b0db5ea02e751691703955e\",\"files\":"
              << files << ",\"exports\":" << exports << ",\"cases\":[";
    bool first = true;
    for (const auto &test : cases) {
      if (!first)
        std::cout << ',';
      first = false;
      const auto jax = fixture.jax();
      std::cout << "{\"name\":" << q(test.name)
                << ",\"input\":" << q(text(test.input)) << ",\"options\":";
      std::cout << "{}";
      std::cout << ",\"result\":";
      try {
        std::cout << serializer.node(fixture.compile(test.input, *jax))
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
      std::cout << ",\"packageData\":" << serializer.packageData(*jax)
                << ",\"activeOptions\":" << serializer.activeOptions(*jax)
                << ",\"activeTags\":" << serializer.activeTags(*jax) << "}";
    }
    std::cout << "],\"maps\":" << serializer.maps(fixture.runtime) << "}\n";
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
