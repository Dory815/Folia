// Native port of MathJax 4.1.3 input/tex/mhchem/MhchemConfiguration.ts.
// Copyright (c) 2018-2026 The MathJax Consortium. Apache-2.0.
module folia.math.input.tex.mhchem.configuration;
import folia.math.dependencies.mhchem_parser;
import folia.math.input.tex.ams.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.token_map;
import folia.math.util.function_types;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import folia.math.util.regexp_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
TexParser& parser(ParserContext& source) {
    auto* result = dynamic_cast<TexParser*>(&source);
    if (!result) property_host::typeError("Mhchem method requires a TexParser");
    return *result;
}
Value array(Context& context, std::initializer_list<Value> values) {
    const auto result = context.array(static_cast<std::uint32_t>(values.size()));
    std::uint32_t index = 0;
    for (const auto& value : values) {
        const auto number = std::to_string(index++);
        context.set(result, Key(std::u16string(number.begin(), number.end())), value);
    }
    return result;
}
Value record(Context& context, std::initializer_list<std::pair<std::u16string_view, Value>> values) {
    const Value result(context.object());
    for (const auto& [name, value] : values) context.set(result, Key(std::u16string(name)), value);
    return result;
}
FunctionRef machine(std::weak_ptr<MhchemModule> module) {
    return makeParseCallable([module = std::move(module)](const FunctionReceiver&, ParserContext& source, const ParseAtom& atom, ParseRest rest) -> ParseResult {
        const auto owner = module.lock();
        if (!owner) property_host::typeError("Mhchem module is no longer available");
        auto& current = parser(source);
        const auto name = property_host::string(std::get<Property>(atom));
        std::u16string type;
        if (!rest.empty()) {
            if (const auto* value = std::any_cast<Property>(&rest.front())) type = property_host::string(*value);
            else if (const auto* value = std::any_cast<std::u16string>(&rest.front())) type = *value;
        }
        const auto argument = property_host::string(current.GetArgument(name));
        std::u16string tex;
        try {
            tex = owner->replacements(mhchem::toTex(argument, type));
        } catch (const mhchem::Error& error) {
            throw TexError(error.id, error.message);
        }
        const auto input = current.string();
        const auto offset = std::isfinite(current.i) && current.i > 0
            ? std::min(input.size(), static_cast<std::size_t>(current.i)) : std::size_t{};
        current.setString(tex + input.substr(offset));
        current.i = 0;
        return Property(Missing{});
    });
}
FunctionRef makeRelation() {
    return makeParseCallable([](const FunctionReceiver&, ParserContext& source, const ParseAtom& atom, ParseRest) -> ParseResult {
        auto& current = parser(source);
        const auto* value = std::get_if<TokenRef>(&atom);
        if (!value || !*value) property_host::typeError("Mhchem relmo requires a Token");
        PropertyList attributes{{"stretchy", Property(true)}, {"texClass", Property(3)}, {"mathvariant", Property("-mhchem")}};
        if (const auto extra = PropertyList::record((*value)->attributes()))
            for (const auto& key : extra->keys()) attributes.setOwn(key, extra->getOwn(key));
        const std::array<std::any, 3> arguments{std::any(Property("mo")), std::any(Property(attributes)), std::any((*value)->character())};
        current.Push(current.create(u"token", FunctionArgs(arguments)));
        return Property(Missing{});
    });
}
void bindMethod(ParserRuntime& runtime, const FunctionRef& function, std::u16string name, double length) {
    const auto value = BaseMethods::bind(runtime, function, std::move(name));
    const auto* identity = std::get_if<ObjectRef>(&value.data);
    if (!identity) throw Exception("TypeError", u"Mhchem method has no Context identity");
    runtime.context().define(*identity, Key(u"length"), Descriptor::data(Value(length), false, false, true));
}
bool lookupTruthy(const TokenMapRef& map, std::u16string_view name) {
    if (!map) return false;
    const auto value = map->lookup(Property(property_host::debugText(name)));
    return std::visit([](const auto& entry) {
        using T = std::decay_t<decltype(entry)>;
        if constexpr (std::is_same_v<T, Property>) return property_host::truthy(entry);
        else return static_cast<bool>(entry);
    }, value);
}
FunctionRef arrowReplacement(ParserRuntime& runtime, std::weak_ptr<MhchemModule> module) {
    auto function = std::make_shared<Function>();
    function->call = [module = std::move(module)](const FunctionReceiver&, FunctionArgs arguments) -> std::any {
        const auto owner = module.lock();
        if (!owner) property_host::typeError("Mhchem module is no longer available");
        const auto* whole = arguments.size() > 0 ? std::any_cast<std::u16string>(&arguments[0]) : nullptr;
        const auto* arrow = arguments.size() > 1 ? std::any_cast<std::u16string>(&arguments[1]) : nullptr;
        if (!whole || !arrow) property_host::typeError("Mhchem replacement requires string arguments");
        const auto name = u"mhchem" + *arrow;
        return std::any(lookupTruthy(owner->characters(), name) || lookupTruthy(owner->macros(), name)
            ? u"\\" + name : *whole);
    };
    OptionCallBinding conversion;
    conversion.receiver = [](Context&, Value) -> FunctionReceiver { return {}; };
    conversion.arguments = [](Context&, std::span<const Value> values) {
        std::vector<std::any> arguments;
        for (std::size_t i = 0; i < 2; ++i) {
            const auto* value = i < values.size() ? std::get_if<std::u16string>(&values[i].data) : nullptr;
            if (!value) property_host::typeError("Mhchem replacement requires string arguments");
            arguments.emplace_back(*value);
        }
        return arguments;
    };
    conversion.result = [](Context&, const std::any& result) -> Value {
        const auto* text = std::any_cast<std::u16string>(&result);
        if (!text) property_host::typeError("Mhchem replacement did not return a string");
        return Value(*text);
    };
    const auto identity = runtime.functions().bind(function, std::move(conversion), u"");
    const auto* object = std::get_if<ObjectRef>(&identity.data);
    if (!object) property_host::typeError("Mhchem replacement has no Context identity");
    runtime.context().define(*object, Key(u"length"), Descriptor::data(Value(2.0), false, false, true));
    return function;
}
}
MhchemModule::MhchemModule(ParserRuntime& runtime) : runtime_(runtime), rules_{{
        {uR"rx(\\underset{\\lower2mu{(.*?)}}{\\overset{(.*?)}{\\long(.*?)}})rx", u"\\mhchemx$3[$1]{$2}"},
        {uR"rx(\\overset{(.*?)}{\\long(.*?)})rx", u"\\mhchemx$2{$1}"},
        {uR"rx(\\rlap\{\\lower\.1em\{-\}\}\\raise\.1em\{\\tripledash\})rx", u"\\mhchemBondTD"},
        {uR"rx(\\rlap\{\\lower\.2em\{-\}\}\\rlap\{\\raise\.2em\{\\tripledash\}\}-)rx", u"\\mhchemBondTDD"},
        {uR"rx(\\rlap\{\\lower\.2em\{-\}\}\\rlap\{\\raise.2em\{-\}\}\\tripledash)rx", u"\\mhchemBondDTD"},
        {uR"rx(\\(x?(?:long)?(?:left|right|[Ll]eftright|[Rr]ightleft)(?:arrow|harpoons)))rx", FunctionRef{}}
    }} {}
std::shared_ptr<MhchemModule> MhchemModule::initializeModule(ParserRuntime& runtime, util::RuntimeModules& modules) {
    if (&modules.context() != &runtime.context())
        throw options::Exception("TypeError", u"Mhchem module belongs to a different Context");
    const auto result = modules.getOrCreate<MhchemModule>(u"input/tex/mhchem/MhchemConfiguration", [&] {
        auto owner = std::shared_ptr<MhchemModule>(new MhchemModule(runtime));
        owner->initialize(owner);
        return owner;
    });
    if (&result->runtime_ != &runtime)
        throw options::Exception("TypeError", u"Mhchem module belongs to a different ParserRuntime");
    return result;
}
options::Context& MhchemModule::context() const { return runtime_.context(); }
std::u16string MhchemModule::replacement(std::u16string tex, std::size_t index) const {
    const auto& rule = rules_[index];
    if (const auto* replacement = std::get_if<std::u16string>(&rule.replacement))
        return mhchem::replaceRegex(std::move(tex), rule.pattern, *replacement, true);
    const auto& callback = std::get<FunctionRef>(rule.replacement);
    if (!callback || !callback->call) property_host::typeError("Mhchem replacement is not callable");
    regexp_host::RegExp expression(rule.pattern, u"g");
    std::u16string result;
    std::size_t offset = 0;
    while (const auto found = expression.exec(tex)) {
        const auto whole = found->at(0).value_or(u"");
        const std::array<std::any, 2> arguments{std::any(whole), std::any(found->at(1).value_or(u""))};
        const auto value = callback->call(FunctionReceiver{}, arguments);
        const auto* replacement = std::any_cast<std::u16string>(&value);
        if (!replacement) property_host::typeError("Mhchem replacement did not return a string");
        result += tex.substr(offset, found->index - offset);
        result += *replacement;
        offset = found->index + whole.size();
    }
    return result + tex.substr(offset);
}
std::u16string MhchemModule::replacements(std::u16string tex) const {
    for (std::size_t i = 0; i < rules_.size(); ++i) tex = replacement(std::move(tex), i);
    return tex;
}
void MhchemModule::initialize(const std::shared_ptr<MhchemModule>& owner) {
    auto& runtime = runtime_;
    rules_.back().replacement = arrowReplacement(runtime, owner);
    const auto invokeMachine = machine(owner);
    const auto relation = makeRelation();
    bindMethod(runtime, invokeMachine, u"Machine", 3);
    bindMethod(runtime, relation, u"relmo", 2);
    const auto arrow = AmsMethods::get(runtime, u"xArrow");
    methods_ = Property(PropertyList{{"Machine", Property(invokeMachine)},
        {"Macro", BaseMethods::get(runtime, u"Macro")}, {"xArrow", arrow}});
    utils_ = Property(PropertyList{{"relmo", Property(relation)}});
    const PropertyList commands{
        {"ce", Property(PropertyArray{Property(invokeMachine), Property("ce")})},
        {"pu", Property(PropertyArray{Property(invokeMachine), Property("pu")})},
        {"mhchemxrightarrow", Property(PropertyArray{arrow, Property(0xe429), Property(5), Property(9)})},
        {"mhchemxleftarrow", Property(PropertyArray{arrow, Property(0xe428), Property(9), Property(5)})},
        {"mhchemxleftrightarrow", Property(PropertyArray{arrow, Property(0xe42a), Property(9), Property(9)})},
        {"mhchemxleftrightarrows", Property(PropertyArray{arrow, Property(0xe42b), Property(9), Property(9)})},
        {"mhchemxrightleftharpoons", Property(PropertyArray{arrow, Property(0xe408), Property(5), Property(9)})},
        {"mhchemxRightleftharpoons", Property(PropertyArray{arrow, Property(0xe409), Property(5), Property(9)})},
        {"mhchemxLeftrightharpoons", Property(PropertyArray{arrow, Property(0xe40a), Property(9), Property(11)})}
    };
    macros_ = TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("mhchem"), Property(commands));
    const PropertyList chars{
        {"tripledash", Property(PropertyArray{Property("\xee\x90\x90"), Property(PropertyList{{"stretchy", Property(false)}})})},
        {"mhchemBondTD", Property(PropertyArray{Property("\xee\x90\x91"), Property(PropertyList{{"stretchy", Property(false)}})})},
        {"mhchemBondTDD", Property(PropertyArray{Property("\xee\x90\x92"), Property(PropertyList{{"stretchy", Property(false)}})})},
        {"mhchemBondDTD", Property(PropertyArray{Property("\xee\x90\x93"), Property(PropertyList{{"stretchy", Property(false)}})})},
        {"mhchemlongleftarrow", Property("\xee\x90\xa8")}, {"mhchemlongrightarrow", Property("\xee\x90\xa9")},
        {"mhchemlongleftrightarrow", Property("\xee\x90\xaa")}, {"mhchemlongrightleftharpoons", Property("\xee\x90\x88")},
        {"mhchemlongRightleftharpoons", Property("\xee\x90\x89")}, {"mhchemlongLeftrightharpoons", Property("\xee\x90\x8a")},
        {"mhchemlongleftrightarrows", Property("\xee\x90\xab")}, {"mhchemrightarrow", Property("\xee\x90\xad")},
        {"mhchemleftarrow", Property("\xee\x90\xac")}, {"mhchemleftrightarrow", Property("\xee\x90\xae")}
    };
    characters_ = TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("mhchem-chars"), ParseMethod{relation}, Property(chars));
    auto& context = runtime.context();
    configuration_ = Configuration::create(runtime, u"mhchem", record(context, {{u"handler", record(context,
        {{u"macro", array(context, {Value(u"mhchem"), Value(u"mhchem-chars")})}})}}));
}
Property MhchemModule::method(std::u16string_view name) const {
    return property_host::get(methods_, property_host::debugText(name));
}
ParseMethod MhchemModule::relmo() const {
    return functions::fromProperty(property_host::get(utils_, "relmo"));
}
ConfigurationRef MhchemConfiguration::initializeModule(ParserRuntime& runtime, util::RuntimeModules& modules) {
    return MhchemModule::initializeModule(runtime, modules)->configuration();
}
Property MhchemMethods::get(ParserRuntime& runtime, util::RuntimeModules& modules, std::u16string_view name) {
    return MhchemModule::initializeModule(runtime, modules)->method(name);
}
ParseMethod MhchemUtils::relmo(ParserRuntime& runtime, util::RuntimeModules& modules) {
    return MhchemModule::initializeModule(runtime, modules)->relmo();
}
}
