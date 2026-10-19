// Complete package observations for pinned MathJax 4.1.3 MhchemConfiguration.
// The generated include contains only inputs and provenance, never results.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.ams.configuration;
import folia.math.input.tex.ams.methods;
import folia.math.input.tex.mhchem.configuration;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.token;
import folia.math.input.tex.macro;
import folia.math.core.math_item;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.handlers.html.html_math_document;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.property_host;
import folia.math.util.map_bindings;
import folia.math.util.option_functions;
import folia.math.util.runtime_modules;

namespace {
namespace tex = folia::math::tex;
namespace opts = folia::math::options;
namespace output = folia::math::output;
namespace common = folia::math::output::common;
using namespace folia::math;

std::string quote(std::string_view value) {
    std::string out = "\"";
    constexpr char hex[] = "0123456789abcdef";
    for (unsigned char c : value) {
        switch (c) {
        case '\\': out += "\\\\"; break;
        case '"': out += "\\\""; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 32) {
                out += "\\u00";
                out += hex[c >> 4];
                out += hex[c & 15];
            } else out += static_cast<char>(c);
        }
    }
    return out + '"';
}
std::string text(std::u16string_view value) { return property_host::debugText(value); }
std::string number(double value) {
    if (std::isnan(value)) return "{\"$\":\"NaN\"}";
    if (std::isinf(value)) return value < 0 ? "{\"$\":\"-Infinity\"}" : "{\"$\":\"Infinity\"}";
    if (value == 0 && std::signbit(value)) return "{\"$\":\"-0\"}";
    std::ostringstream out;
    out << std::setprecision(17) << value;
    return out.str();
}

struct Serializer {
    opts::Context& context;
    OptionFunctionBindings& functions;
    std::size_t nodeCount=0;

    std::string value(const opts::Value& input, std::vector<opts::ObjectRef>& active) {
        if (std::holds_alternative<opts::Undefined>(input.data)) return "{\"$\":\"undefined\"}";
        if (std::holds_alternative<opts::Null>(input.data)) return "null";
        if (const auto* b = std::get_if<bool>(&input.data)) return *b ? "true" : "false";
        if (const auto* n = std::get_if<double>(&input.data)) return number(*n);
        if (const auto* s = std::get_if<std::u16string>(&input.data)) return quote(text(*s));
        if (std::holds_alternative<opts::SymbolRef>(input.data)) return "{\"$\":\"symbol\"}";
        const auto* object = std::get_if<opts::ObjectRef>(&input.data);
        if (!object) throw std::runtime_error("Unserializable Context value");
        if (context.kind(*object) == opts::ObjectKind::function) {
            const auto name = context.get(*object, opts::Key(u"name"));
            const auto length = context.get(*object, opts::Key(u"length"));
            const auto* n = std::get_if<std::u16string>(&name.data);
            const auto* l = std::get_if<double>(&length.data);
            if (!n || !l) throw std::runtime_error("Function metadata missing");
            return "{\"$\":\"function\",\"name\":" + quote(text(*n)) + ",\"length\":" + number(*l) + "}";
        }
        if (std::ranges::find(active, *object) != active.end()) return "{\"$\":\"cycle\"}";
        active.push_back(*object);
        struct Pop {
            std::vector<opts::ObjectRef>& active;
            ~Pop() { active.pop_back(); }
        } pop{active};
        if (context.kind(*object) == opts::ObjectKind::array) {
            std::string out = "[";
            const auto count = context.arrayLength(*object);
            for (std::uint32_t i = 0; i < count; ++i) {
                if (i) out += ',';
                const auto index = std::to_string(i);
                const opts::Key key(std::u16string(index.begin(), index.end()));
                out += context.hasOwn(*object, key) ? value(context.get(*object, key), active) : "{\"$\":\"hole\"}";
            }
            return out + ']';
        }
        if (const auto* map = std::any_cast<opts::MapStorageRef>(&context.payload(*object)); map && *map) {
            std::string out = "{\"$\":\"map\",\"entries\":[";
            bool first = true;
            for (const auto& entry : (*map)->entries()) {
                if (!entry.live) continue;
                if (!first) out += ',';
                first = false;
                out += '[' + value(entry.key, active) + ',' + value(entry.value, active) + ']';
            }
            return out + "]}";
        }
        std::string out = "{";
        bool first = true;
        for (const auto& key : context.ownKeys(opts::Value(*object))) {
            if (!first) out += ',';
            first = false;
            out += quote(text(context.keyString(key))) + ':' + value(context.get(opts::Value(*object), key), active);
        }
        return out + '}';
    }

    std::string property(const Property& input) {
        if (std::holds_alternative<Missing>(input.value)) return "{\"$\":\"undefined\"}";
        if (std::holds_alternative<PropertyNull>(input.value)) return "null";
        if (const auto* b = std::get_if<bool>(&input.value)) return *b ? "true" : "false";
        if (const auto* n = std::get_if<double>(&input.value)) return number(*n);
        if (const auto* s = std::get_if<std::string>(&input.value)) return quote(*s);
        if (std::holds_alternative<opts::SymbolRef>(input.value)) return "{\"$\":\"symbol\"}";
        if (const auto* f = std::get_if<FunctionRef>(&input.value)) {
            const auto identity = functions.find(*f);
            if (!identity) throw std::runtime_error("Unbound function in observation");
            std::vector<opts::ObjectRef> active;
            return value(opts::Value(*identity), active);
        }
        if (std::holds_alternative<BuiltinProperty>(input.value)) return "{\"$\":\"builtin\"}";
        const auto storage = std::get<std::shared_ptr<PropertyStorage>>(input.value);
        if (!storage) return "null";
        if (storage->array) {
            const auto a = PropertyArray::array(input);
            std::string out = "[";
            for (std::size_t i = 0; i < a->size(); ++i) {
                if (i) out += ',';
                out += a->has(i) ? property(a->get(i)) : "{\"$\":\"hole\"}";
            }
            return out + ']';
        }
        const auto r = PropertyList::record(input);
        if (!r) throw std::runtime_error("Unserializable Property record");
        return record(*r);
    }

    std::string record(const PropertyList& input) {
        std::string out = "{";
        bool first = true;
        for (const auto& key : input.keys()) {
            if (!first) out += ',';
            first = false;
            out += quote(key) + ':' + property(input.getOwn(key));
        }
        return out + '}';
    }

    std::string method(const tex::ParseMethod& input) {
        if (const auto* f = std::get_if<FunctionRef>(&input)) return property(Property(*f));
        return property(std::get<Property>(input));
    }

    std::string lookup(const tex::TokenLookup& input) {
        if (const auto* p = std::get_if<Property>(&input)) return property(*p);
        if (const auto* m = std::get_if<tex::MacroRef>(&input)) {
            if (!*m) throw std::runtime_error("Null macro in package map");
            return "{\"_token\":" + property((*m)->token()) + ",\"_func\":" + method((*m)->func()) + ",\"_args\":" + property((*m)->args()) + '}';
        }
        if (const auto* t = std::get_if<tex::TokenRef>(&input)) {
            if (!*t) throw std::runtime_error("Null token in package map");
            return "{\"_token\":" + property((*t)->token()) + ",\"_char\":" + property((*t)->character()) + ",\"_attributes\":" + property((*t)->attributes()) + '}';
        }
        throw std::runtime_error("Unserializable token map lookup");
    }

    std::string tokenMap(const tex::TokenMapRef& source) {
        const auto map = std::dynamic_pointer_cast<tex::AbstractParseMap>(source);
        if (!map) throw std::runtime_error("Package map is not an AbstractParseMap");
        std::string out = "{\"_name\":" + property(map->name()) + ",\"_parser\":" + method(map->parser()) + ",\"map\":{\"$\":\"map\",\"entries\":[";
        bool first = true;
        if (const auto& entries = map->mapStorage(); entries) {
            for (const auto& [key, item] : *entries) {
                if (!first) out += ',';
                first = false;
                out += '[' + property(key) + ',' + lookup(item) + ']';
            }
        }
        return out + "]}}";
    }

    std::string maps(tex::ParserRuntime& runtime) {
        std::string out = "{";
        bool first = true;
        for (const auto name : {"mhchem", "mhchem-chars"}) {
            const auto map = runtime.mapHandler.getMap(Property(name));
            if (!map) throw std::runtime_error("Missing registered package map");
            if (!first) out += ',';
            first = false;
            out += quote(name) + ':' + tokenMap(map);
        }
        return out + '}';
    }

    std::string node(mml::NodeValue input) {
        if (!input.isNode()) return "null";
        ++nodeCount;
        auto* n = input.requireNode();
        const auto attrs = n->attributes();
        std::string out = "{\"kind\":" + quote(n->kind());
        out += ",\"attrs\":" + record(attrs ? attrs->getAllAttributes() : PropertyList{});
        out += ",\"inherited\":" + record(attrs ? attrs->getAllInherited() : PropertyList{});
        out += ",\"defaults\":" + record(attrs ? attrs->getAllDefaults() : PropertyList{});
        out += ",\"globals\":" + record(attrs ? attrs->getAllGlobals() : PropertyList{});
        out += ",\"properties\":" + record(n->getAllProperties());
        if (const auto* t = dynamic_cast<mml::TextNode*>(n)) out += ",\"text\":" + quote(text(t->getText()));
        else if (const auto* t = dynamic_cast<mml::AbstractMmlTokenNode*>(n)) out += ",\"text\":" + quote(text(t->getText()));
        out += ",\"children\":[";
        if (const auto children = n->childNodes; children) {
            for (std::size_t i = 0; i < children->size(); ++i) {
                if (i) out += ',';
                out += node(children->get(i));
            }
        }
        return out + "]}";
    }

    std::string packageData(tex::TeX& jax) {
        const auto input = jax.parseOptions();
        const auto* parsed = std::get_if<tex::ParseOptionsRef>(&input);
        if (!parsed || !*parsed) return "null";
        const auto data = (*parsed)->packageData;
        const auto* map = std::get_if<std::shared_ptr<SourceStringMap<opts::Value>>>(&data);
        if (!map || !*map) return "null";
        std::string out = "{\"$\":\"map\",\"entries\":[";
        bool first = true;
        std::vector<opts::ObjectRef> active;
        for (auto keys = (*map)->keys(); auto key = keys.next();) {
            if (!first) out += ',';
            first = false;
            out += '[' + quote(text(*key)) + ',' + value((*map)->get(*key).value(), active) + ']';
        }
        return out + "]}";
    }
};

common::FontLoadHost fontHost() {
    common::FontLoadHost host;
    host.asyncIsSynchronous = [] { return true; };
    host.warn = [](const std::any&) {};
    return host;
}
struct Fixture {
    std::unique_ptr<output::svg::NativeRenderSession> session;
    opts::Context& cx;
    OptionFunctionBindings& functions;
    tex::ParserRuntime& runtime;
    util::RuntimeModules& modules;
    std::shared_ptr<tex::MhchemModule> mhchem;
    std::shared_ptr<handlers::html::HTMLMathDocument> activeDocument;
    opts::ObjectRef activeInputJaxIdentity{};
    tex::TeXRef activeInputJax;
    std::vector<tex::TexError> formattedErrors;
    opts::Value observeFormatError;
    Fixture()
        : session(std::make_unique<output::svg::NativeRenderSession>(fontHost())),
          cx(session->context()), functions(session->services().functions()),
          runtime(session->services().parser()), modules(session->services().modules()) {
        mhchem = tex::MhchemModule::initializeModule(runtime, modules);
        (void)tex::MhchemConfiguration::initializeModule(runtime, modules);
        const auto outputOptions = cx.object();
        cx.set(outputOptions, opts::Key(u"fontCache"), opts::Value(u"none"));
        cx.set(outputOptions, opts::Key(u"exFactor"), opts::Value(0.5));
        (void)session->construct(opts::Value(outputOptions));
        auto body = std::make_shared<Function>();
        body->call = [this](const FunctionReceiver&, FunctionArgs args) -> std::any {
            if (args.size() < 2) throw std::runtime_error("Incomplete formatError observation");
            const auto* jax = std::any_cast<tex::TeXRef>(&args[0]);
            const auto* failure = std::any_cast<tex::TexError>(&args[1]);
            if (!jax || !*jax || !failure) throw std::runtime_error("Invalid formatError observation arguments");
            formattedErrors.push_back(*failure);
            return std::any((*jax)->formatError(*failure));
        };
        OptionCallBinding binding;
        binding.receiver = [](opts::Context&, opts::Value receiver) -> FunctionReceiver {
            return FunctionReceiver(std::move(receiver));
        };
        binding.arguments = [this](opts::Context& context, std::span<const opts::Value> args) {
            if (args.size() < 2) throw std::runtime_error("Incomplete Context formatError observation");
            const auto* jax = std::get_if<opts::ObjectRef>(&args[0].data);
            const auto* failure = std::get_if<opts::ObjectRef>(&args[1].data);
            if (!jax || !failure) throw std::runtime_error("Invalid Context formatError observation arguments");
            const auto id = context.get(*failure, opts::Key(u"id"));
            const auto message = context.get(*failure, opts::Key(u"message"));
            const auto* i = std::get_if<std::u16string>(&id.data);
            const auto* m = std::get_if<std::u16string>(&message.data);
            if (!i || !m) throw std::runtime_error("Invalid Context TexError fields");
            return std::vector<std::any>{std::any(runtime.texIdentities.resolve(*jax)), std::any(tex::TexError(*i, *m))};
        };
        binding.result = [](opts::Context&, const std::any&) -> opts::Value {
            throw std::runtime_error("The observer invokes formatError through its native MML binding");
        };
        observeFormatError = functions.bind(body, std::move(binding), u"observeFormatError");
    }
    tex::TeXRef jax(bool ams) {
        formattedErrors.clear();
        const auto config = cx.object();
        const auto packages = cx.array(ams ? 3 : 2);
        cx.set(packages, opts::Key(u"0"), opts::Value(u"base"));
        cx.set(packages, opts::Key(u"1"), opts::Value(ams ? u"ams" : u"mhchem"));
        if (ams) cx.set(packages, opts::Key(u"2"), opts::Value(u"mhchem"));
        cx.set(config, opts::Key(u"packages"), opts::Value(packages));
        cx.set(config, opts::Key(u"formatError"), observeFormatError);
        activeDocument = output::svg::createNativeSvgDocument(*session,
            opts::Value(opts::Undefined{}), opts::Value(config));
        const opts::Value documentValue(activeDocument->sourceIdentity());
        const auto inputJaxArray = cx.get(documentValue, opts::Key(u"inputJax"));
        const auto inputJaxValue = cx.get(inputJaxArray, opts::Key(u"0"));
        const auto* identity = std::get_if<opts::ObjectRef>(&inputJaxValue.data);
        if (!identity) throw opts::Exception("TypeError", u"Mhchem document inputJax[0] is not canonical");
        activeInputJaxIdentity = *identity;
        activeInputJax = runtime.texIdentities.resolve(activeInputJaxIdentity);
        if (!activeInputJax) throw opts::Exception("TypeError", u"Mhchem document TeX owner is unavailable");
        return activeInputJax;
    }
    mml::NodeValue compile(std::u16string_view source, tex::TeX& jax) {
        if (!activeDocument || activeInputJax.get() != &jax)
            throw std::runtime_error("Mhchem HTMLMathDocument owner is missing");
        const opts::Value documentValue(activeDocument->sourceIdentity());
        const auto options = cx.get(documentValue, opts::Key(u"options"));
        const auto constructor = cx.get(options, opts::Key(u"MathItem"));
        const auto* itemConstructor = std::get_if<opts::ObjectRef>(&constructor.data);
        if (!itemConstructor) throw opts::Exception("TypeError", u"Mhchem document MathItem constructor is unavailable");
        const std::array<opts::Value, 3> arguments{
            opts::Value(std::u16string(source)), opts::Value(activeInputJaxIdentity), opts::Value(true)};
        const opts::ObjectRef itemIdentity = cx.construct(*itemConstructor, arguments);
        const auto* native = std::any_cast<std::shared_ptr<NativeObject>>(&cx.payload(itemIdentity));
        const auto item = native ? std::dynamic_pointer_cast<core::MathItem>(*native) : nullptr;
        if (!item) throw opts::Exception("TypeError", u"Mhchem MathItem has no canonical HTML owner");
        return jax.compile(*item, core::MathDocumentReference{activeDocument.get()});
    }
};

std::string error(std::string_view name, std::string id, std::string_view message) {
    return "{\"name\":" + quote(name) + ",\"id\":" + id + ",\"message\":" + quote(message) + '}';
}

Property methodProperty(const tex::ParseMethod& method) {
    if (const auto* f = std::get_if<FunctionRef>(&method)) return Property(*f);
    return std::get<Property>(method);
}

Property macroFunction(tex::ParserRuntime& runtime, std::string_view name) {
    const auto map = runtime.mapHandler.getMap(Property("mhchem"));
    if (!map) throw std::runtime_error("Missing mhchem command map");
    const auto item = map->lookup(Property(std::string(name)));
    const auto* macro = std::get_if<tex::MacroRef>(&item);
    if (!macro || !*macro) throw std::runtime_error("Missing observed mhchem macro");
    return methodProperty((*macro)->func());
}

#include "mhchem-package-corpus.inc"


struct MutationObserver {
    using Entry = std::pair<Property, tex::TokenLookup>;
    Fixture& fixture;
    Serializer& serializer;
    std::shared_ptr<tex::MhchemModule> module;
    std::shared_ptr<tex::AbstractParseMap> macros, characters;
    Property methodsOwner, utilsOwner;
    std::vector<std::pair<std::string, Property>> originalMethods, originalUtils;
    std::vector<Entry> originalMacroEntries, originalCharacterEntries;
    tex::ParseMethod originalMacroParser, originalCharacterParser;
    Property originalMachine, originalMacro, originalArrow, originalRelmo;
    tex::ConfigurationRef originalConfiguration;

    static PropertyList record(const Property& value) {
        const auto result = PropertyList::record(value);
        if (!result) throw std::runtime_error("Exported Mhchem record is missing");
        return *result;
    }
    static std::vector<std::pair<std::string, Property>> recordEntries(const Property& value) {
        std::vector<std::pair<std::string, Property>> entries;
        const auto source = record(value);
        for (const auto& key : source.keys()) entries.emplace_back(key, source.getOwn(key));
        return entries;
    }
    static std::vector<Entry> mapEntries(const tex::AbstractParseMap& value) {
        const auto& entries = value.mapStorage();
        std::vector<Entry> result;
        if (entries) for (const auto& [key, item] : *entries) result.emplace_back(key, item);
        return result;
    }
    static Property lookupFunction(const tex::TokenMapRef& map, std::string_view name) {
        const auto value = map->lookup(Property(std::string(name)));
        if (const auto* macro = std::get_if<tex::MacroRef>(&value); macro && *macro) {
            return methodProperty((*macro)->func());
        }
        return Property(Missing{});
    }

    MutationObserver(Fixture& fixture, Serializer& serializer)
        : fixture(fixture), serializer(serializer), module(fixture.mhchem),
          macros(std::dynamic_pointer_cast<tex::AbstractParseMap>(module->macros())),
          characters(std::dynamic_pointer_cast<tex::AbstractParseMap>(module->characters())),
          methodsOwner(module->methods()), utilsOwner(module->utils()),
          originalMethods(recordEntries(methodsOwner)), originalUtils(recordEntries(utilsOwner)),
          originalMacroEntries(mapEntries(*macros)), originalCharacterEntries(mapEntries(*characters)),
          originalMacroParser(macros->parser()), originalCharacterParser(characters->parser()),
          originalMachine(module->method(u"Machine")), originalMacro(module->method(u"Macro")),
          originalArrow(module->method(u"xArrow")), originalRelmo(methodProperty(module->relmo())),
          originalConfiguration(module->configuration()) {}

    static void restoreRecord(const Property& value, const std::vector<std::pair<std::string, Property>>& entries) {
        auto target = record(value);
        for (const auto& key : target.keys()) target.erase(key);
        for (const auto& [key, item] : entries) target.setOwn(key, item);
    }
    static void restoreMap(tex::AbstractParseMap& map, const std::vector<Entry>& original) {
        const auto current = mapEntries(map);
        for (const auto& [key, item] : current) map.remove(key);
        for (const auto& [key, item] : original) map.add(key, item);
    }
    void restore() {
        restoreRecord(methodsOwner, originalMethods);
        restoreRecord(utilsOwner, originalUtils);
        restoreMap(*macros, originalMacroEntries);
        restoreMap(*characters, originalCharacterEntries);
        macros->parser(originalMacroParser);
        characters->parser(originalCharacterParser);
        fixture.runtime.mapHandler.registerMap(macros);
        fixture.runtime.mapHandler.registerMap(characters);
    }

    std::string snapshot() {
        const auto imported = tex::MhchemModule::initializeModule(fixture.runtime, fixture.modules);
        const auto configuration = tex::MhchemConfiguration::initializeModule(fixture.runtime, fixture.modules);
        std::string result = "{\"methods\":" + serializer.property(module->methods())
            + ",\"utils\":" + serializer.property(module->utils()) + ",\"getters\":{";
        bool first = true;
        for (const auto name : {u"Machine", u"Macro", u"xArrow", u"mhchemUnknown", u"observerExtra"}) {
            if (!first) result += ',';
            first = false;
            result += quote(text(name)) + ':' + serializer.property(tex::MhchemMethods::get(fixture.runtime, fixture.modules, name));
        }
        result += ",\"relmo\":" + serializer.method(tex::MhchemUtils::relmo(fixture.runtime, fixture.modules));
        result += "},\"retainedMaps\":{\"mhchem\":" + serializer.tokenMap(macros)
            + ",\"mhchem-chars\":" + serializer.tokenMap(characters);
        const auto registryMacros = fixture.runtime.mapHandler.getMap(Property("mhchem"));
        const auto registryCharacters = fixture.runtime.mapHandler.getMap(Property("mhchem-chars"));
        result += "},\"registryMaps\":{\"mhchem\":" + serializer.tokenMap(registryMacros)
            + ",\"mhchem-chars\":" + serializer.tokenMap(registryCharacters) + "},\"identities\":{";
        first = true;
        const auto identity = [&](std::string_view name, bool value) {
            if (!first) result += ',';
            first = false;
            result += quote(name) + ':' + (value ? "true" : "false");
        };
        identity("moduleSame", imported == module);
        identity("configurationSame", configuration == originalConfiguration);
        identity("methodsSame", imported->methods() == methodsOwner);
        identity("utilsSame", imported->utils() == utilsOwner);
        identity("methodsMachineOriginal", module->method(u"Machine") == originalMachine);
        identity("methodsMacroOriginal", module->method(u"Macro") == originalMacro);
        identity("methodsArrowOriginal", module->method(u"xArrow") == originalArrow);
        identity("utilsRelmoOriginal", methodProperty(module->relmo()) == originalRelmo);
        identity("ceOriginal", lookupFunction(macros, "ce") == originalMachine);
        identity("puOriginal", lookupFunction(macros, "pu") == originalMachine);
        identity("characterParserOriginal", methodProperty(characters->parser()) == originalRelmo);
        identity("machineEqualsCe", module->method(u"Machine") == lookupFunction(macros, "ce"));
        identity("relmoEqualsParser", methodProperty(module->relmo()) == methodProperty(characters->parser()));
        identity("macrosSameAsRegistry", macros == registryMacros);
        identity("charactersSameAsRegistry", characters == registryCharacters);
        result += "},\"replacementStages\":[";
        auto replacement = std::u16string(mutationInput);
        for (std::size_t i = 0; i < module->replacementRules().size(); ++i) {
            if (i) result += ',';
            replacement = module->replacement(std::move(replacement), i);
            result += quote(text(replacement));
        }
        const auto pipeline = module->replacements(std::u16string(mutationInput));
        if (pipeline != replacement) throw std::runtime_error("Mutated replacement pipeline differs from ordered rules");
        return result + "],\"replacement\":" + quote(text(pipeline)) + '}';
    }

    std::shared_ptr<tex::Macro> replacementMacro(std::string_view name, std::string_view replacement) {
        return std::make_shared<tex::Macro>(Property(std::string(name)), tex::ParseMethod{originalMacro},
            Property(PropertyArray{Property(std::string(replacement)), Property(0)}));
    }
    Property macroDefinition(std::string_view replacement) {
        return Property(PropertyArray{originalMacro, Property(std::string(replacement)), Property(0)});
    }
    void apply(std::string_view name) {
        if (name == "ce-macro-replaced") {
            macros->add(Property("ce"), tex::MacroRef(replacementMacro("ce", "observer replacement")));
        } else if (name == "character-parser-replaced") {
            characters->parser(tex::ParseMethod{originalMacro});
        } else if (name == "character-arrow-removed") {
            characters->remove(Property("mhchemrightarrow"));
        } else if (name == "character-arrow-false") {
            characters->add(Property("mhchemrightarrow"), Property(false));
        } else if (name == "character-arrow-null") {
            characters->add(Property("mhchemrightarrow"), Property(PropertyNull{}));
        } else if (name == "character-arrow-custom") {
            characters->add(Property("mhchemrightarrow"), tex::TokenRef(std::make_shared<tex::Token>(
                Property("mhchemrightarrow"), Property("Z"), Property(PropertyList{{"stretchy", Property(false)}}))));
        } else if (name == "macro-arrow-removed") {
            macros->remove(Property("mhchemxrightarrow"));
        } else if (name == "character-false-macro-fallback") {
            characters->add(Property("mhchemrightarrow"), Property(false));
            macros->add(Property("mhchemrightarrow"), tex::MacroRef(replacementMacro("mhchemrightarrow", "fallback")));
        } else if (name == "registry-maps-replaced") {
            (void)tex::TokenMapHost::create<tex::CommandMap>(fixture.runtime.mapHandler,Property("mhchem"), Property(PropertyList{
                {"observerOnly", macroDefinition("Q")}, {"mhchemxrightarrow", macroDefinition("R")}}));
            (void)tex::TokenMapHost::create<tex::CharacterMap>(fixture.runtime.mapHandler,Property("mhchem-chars"), tex::ParseMethod{originalMacro},
                Property(PropertyList{{"mhchemleftarrow", Property("L")}, {"observerOnly", Property("Y")}}));
        } else if (name == "registry-shadow-added-arrow") {
            characters->remove(Property("mhchemrightarrow"));
            macros->remove(Property("mhchemxrightarrow"));
            (void)tex::TokenMapHost::create<tex::CommandMap>(fixture.runtime.mapHandler,Property("mhchem"), Property(PropertyList{
                {"mhchemxrightarrow", macroDefinition("R")}}));
            (void)tex::TokenMapHost::create<tex::CharacterMap>(fixture.runtime.mapHandler,Property("mhchem-chars"), tex::ParseMethod{originalMacro},
                Property(PropertyList{{"mhchemrightarrow", Property("R")}}));
        } else if (name == "exported-methods-reassigned") {
            auto methods = record(methodsOwner);
            methods.setOwn("Machine", originalMacro);
            methods.setOwn("Macro", originalArrow);
            methods.setOwn("xArrow", originalMachine);
        } else if (name == "exported-utils-reassigned") {
            record(utilsOwner).setOwn("relmo", originalArrow);
        } else if (name == "exported-extra-keys") {
            record(methodsOwner).setOwn("observerExtra", originalMacro);
            record(utilsOwner).setOwn("observerExtra", originalMachine);
        } else if (name == "exported-machine-removed") {
            record(methodsOwner).erase("Machine");
        } else throw std::runtime_error("Unknown Mhchem mutation case");
    }
};
} // namespace

int main() {
    try {
        Fixture fixture;
        Serializer serializer{fixture.cx, fixture.functions};
        std::cout << "{\"schema\":1,\"source\":{\"mathjaxVersion\":" << quote(mathjaxVersion)
            << ",\"mathjaxCommit\":" << quote(mathjaxCommit) << ",\"parserVersion\":" << quote(parserVersion)
            << "},\"corpusSha256\":" << quote(corpusSha256) << ",\"cases\":[";
        bool first = true;
        std::size_t caseIndex=0;
        for (const auto& test : cases) {
            using Clock=std::chrono::steady_clock;
            const auto caseStart=Clock::now();
            auto stageStart=caseStart;
            std::chrono::microseconds setupTime{},compileTime{},serializeTime{},packageDataTime{};
            std::size_t mmlNodes=0;
            tex::TeXRef jax;
            std::string result = "null", failure = "null";
            try {
                jax = fixture.jax(test.ams);
                const auto setupEnd=Clock::now();
                setupTime=std::chrono::duration_cast<std::chrono::microseconds>(setupEnd-stageStart);
                const auto node=fixture.compile(test.input, *jax);
                const auto compileEnd=Clock::now();
                compileTime=std::chrono::duration_cast<std::chrono::microseconds>(compileEnd-setupEnd);
                serializer.nodeCount=0;
                const auto serializeStart=Clock::now();
                result=serializer.node(node);
                const auto serializeEnd=Clock::now();
                serializeTime=std::chrono::duration_cast<std::chrono::microseconds>(serializeEnd-serializeStart);
                mmlNodes=serializer.nodeCount;
            } catch (const tex::TexError& e) {
                failure = error("TexError", quote(text(e.id)), text(e.message));
            } catch (const opts::Exception& e) {
                failure = error(e.name, "null", text(e.message));
            } catch (const std::exception& e) {
                failure = error("NativeException", "null", e.what());
            } catch (...) {
                failure = error("UnclassifiedNativeException", "null", "");
            }
            const auto inputUtf8=text(test.input);
            if (!first) std::cout << ',';
            first = false;
            std::cout << "{\"name\":" << quote(test.name) << ",\"input\":" << quote(inputUtf8)
                << ",\"packages\":" << (test.ams ? "[\"base\",\"ams\",\"mhchem\"]" : "[\"base\",\"mhchem\"]")
                << ",\"options\":{},\"result\":" << result << ",\"error\":" << failure
                << ",\"formattedErrors\":[";
            bool firstError = true;
            for (const auto& e : fixture.formattedErrors) {
                if (!firstError) std::cout << ',';
                firstError = false;
                std::cout << error("TexError", quote(text(e.id)), text(e.message));
            }
            std::cout << "],\"packageData\":";
            const auto packageDataStart=Clock::now();
            const auto packageData=jax ? serializer.packageData(*jax) : "null";
            const auto packageDataEnd=Clock::now();
            packageDataTime=std::chrono::duration_cast<std::chrono::microseconds>(packageDataEnd-packageDataStart);
            std::cout << packageData << '}';
            std::cerr<<"mhchem_case index="<<caseIndex<<" name="<<test.name
                <<" input_bytes="<<inputUtf8.size()<<" result_fragment_bytes="<<result.size()
                <<" package_data_bytes="<<packageData.size()<<" mml_nodes="<<mmlNodes
                <<" setup_us="<<setupTime.count()<<" compile_us="<<compileTime.count()
                <<" node_json_us="<<serializeTime.count()<<" package_data_us="<<packageDataTime.count()
                <<" total_us="<<std::chrono::duration_cast<std::chrono::microseconds>(packageDataEnd-caseStart).count()<<'\n';
            ++caseIndex;
        }
        const auto machine = tex::MhchemMethods::get(fixture.runtime, fixture.modules, u"Machine");
        const auto macro = tex::MhchemMethods::get(fixture.runtime, fixture.modules, u"Macro");
        const auto arrow = tex::MhchemMethods::get(fixture.runtime, fixture.modules, u"xArrow");
        const auto relmo = tex::MhchemUtils::relmo(fixture.runtime, fixture.modules);
        const auto characters = fixture.runtime.mapHandler.getMap(Property("mhchem-chars"));
        if (!characters) throw std::runtime_error("Missing mhchem character map");
        std::cout << "],\"methods\":" << serializer.property(fixture.mhchem->methods())
            << ",\"utils\":" << serializer.property(fixture.mhchem->utils()) << ",\"identities\":{";
        const auto truth = [](bool value) { return value ? "true" : "false"; };
        std::cout << "\"machineCe\":" << truth(machine == macroFunction(fixture.runtime,"ce"))
            << ",\"machinePu\":" << truth(machine == macroFunction(fixture.runtime,"pu"))
            << ",\"macroBase\":" << truth(macro == tex::BaseMethods::get(fixture.runtime, u"Macro"))
            << ",\"arrowAms\":" << truth(arrow == tex::AmsMethods::get(fixture.runtime, u"xArrow"))
            << ",\"arrowMacros\":[";
        first = true;
        for (const auto name : {"mhchemxrightarrow", "mhchemxleftarrow", "mhchemxleftrightarrow", "mhchemxleftrightarrows", "mhchemxrightleftharpoons", "mhchemxRightleftharpoons", "mhchemxLeftrightharpoons"}) {
            if (!first) std::cout << ',';
            first = false;
            std::cout << "{\"name\":" << quote(name) << ",\"same\":" << truth(arrow == macroFunction(fixture.runtime,name)) << '}';
        }
        std::cout << "],\"relmoCharacterParser\":" << truth(methodProperty(relmo) == methodProperty(characters->parser()))
            << ",\"missingMethod\":" << serializer.property(tex::MhchemMethods::get(fixture.runtime, fixture.modules, u"mhchemUnknown"))
            << "},\"replacementRules\":[";
        first = true;
        for (const auto& rule : fixture.mhchem->replacementRules()) {
            if (!first) std::cout << ',';
            first = false;
            std::cout << "{\"pattern\":" << quote(text(rule.pattern)) << ",\"flags\":\"g\",\"replacement\":";
            if (const auto* replacement = std::get_if<std::u16string>(&rule.replacement)) {
                std::cout << quote(text(*replacement));
            } else {
                std::cout << serializer.property(Property(std::get<FunctionRef>(rule.replacement)));
            }
            std::cout << '}';
        }
        std::cout << "],\"replacements\":[";
        first = true;
        for (const auto& test : replacements) {
            if (!first) std::cout << ',';
            first = false;
            std::cout << "{\"name\":" << quote(test.name) << ",\"input\":" << quote(text(test.input))
                << ",\"rules\":" << test.rules << ",\"stages\":[";
            auto result = std::u16string(test.input);
            for (std::size_t i = 0; i < fixture.mhchem->replacementRules().size(); ++i) {
                if (i) std::cout << ',';
                result = fixture.mhchem->replacement(std::move(result), i);
                std::cout << quote(text(result));
            }
            const auto pipeline = fixture.mhchem->replacements(std::u16string(test.input));
            if (pipeline != result) throw std::runtime_error("Replacement pipeline differs from its ordered rules");
            std::cout << "],\"result\":" << quote(text(pipeline)) << '}';
        }
        std::cout << "],\"maps\":" << serializer.maps(fixture.runtime) << ",\"mutations\":[";
        MutationObserver mutations{fixture, serializer};
        first = true;
        for (const auto name : mutationCases) {
            const auto before = mutations.snapshot();
            std::string during;
            try {
                mutations.apply(name);
                during = mutations.snapshot();
            } catch (...) {
                mutations.restore();
                throw;
            }
            mutations.restore();
            const auto after = mutations.snapshot();
            if (after != before) throw std::runtime_error("Mhchem mutation restoration differs");
            if (!first) std::cout << ',';
            first = false;
            std::cout << "{\"name\":" << quote(name) << ",\"before\":" << before
                << ",\"during\":" << during << ",\"after\":" << after << '}';
        }
        std::cout << "]}\n";
        return 0;
    } catch (const opts::Exception& e) {
        std::cerr << "fixture-init " << e.name << ": " << text(e.message) << '\n';
    } catch (const std::exception& e) {
        std::cerr << "fixture-init NativeException: " << e.what() << '\n';
    } catch (...) {
        std::cerr << "fixture-init UnclassifiedNativeException\n";
    }
    return 2;
}
