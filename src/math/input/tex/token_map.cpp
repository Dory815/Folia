/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 input/tex/TokenMap.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA256: 0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e
 *************************************************************/
module folia.math.input.tex.token_map;

import folia.math.input.tex.map_handler;
import folia.math.util.argument_expansion;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {

using property_host::typeError;

bool truthy(const ParseMethod& method) {
    if (const auto* function = std::get_if<FunctionRef>(&method)) return bool(*function);
    return property_host::truthy(std::get<Property>(method));
}

bool truthy(const TokenLookup& lookup) {
    return std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::same_as<T, Property>) return property_host::truthy(value);
        else return bool(value);
    }, lookup);
}

ParseAtom atom(const TokenLookup& lookup) {
    return std::visit([](const auto& value) -> ParseAtom {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::same_as<T, Property> || std::same_as<T, TokenRef>) {
            return value;
        } else {
            return std::static_pointer_cast<NativeObject>(value);
        }
    }, lookup);
}

Property tokenValue(const Property& token) {
    std::u16string result = u"\\";
    result += property_host::string(property_host::primitive(token, false));
    return Property(property_host::debugText(result));
}

Property prefixedToken(std::u16string_view prefix, const Property& token) {
    std::u16string result(prefix);
    result += property_host::string(property_host::primitive(token, false));
    return Property(property_host::debugText(result));
}

std::pair<Property, Property> destructureTwo(const Property& value) {
    if (const auto array = PropertyArray::array(value)) {
        const std::size_t length = array->size();
        const Property first = length > 0 ? array->get(0) : Property(Missing{});
        const Property second = length > 1 ? array->get(1) : Property(Missing{});
        return {first, second};
    }
    return {property_host::get(value, "0"), property_host::get(value, "1")};
}

Property field(const TokenLookup& lookup, std::string_view name) {
    return std::visit([&](const auto& value) -> Property {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::same_as<T, Property>) {
            return property_host::get(value, name);
        } else if constexpr (std::same_as<T, TokenRef>) {
            if (!value) typeError("Cannot read property of null token");
            if (name == "token") return value->token();
            return Missing{};
        } else {
            if (!value) typeError("Cannot read property of null macro");
            if (name == "token") return value->token();
            if (name == "args") return value->args();
            return Missing{};
        }
    }, lookup);
}

ParseMethod methodField(const TokenLookup& lookup) {
    if (const auto* macro = std::get_if<MacroRef>(&lookup)) {
        if (!*macro) typeError("Cannot read property of null macro");
        return (*macro)->func();
    }
    return functions::fromProperty(field(lookup, "func"));
}

Property methodFromConfig(const Property& value, const Property& functionMap) {
    if (const auto* name = std::get_if<std::string>(&value.value)) {
        return property_host::get(functionMap, *name);
    }
    return value;
}

std::vector<std::pair<std::string, Property>> entries(const Property& object) {
    const auto keys = property_host::ownKeys(object);
    std::vector<std::pair<std::string, Property>> result;
    result.reserve(keys.size());
    for (const auto& key : keys) result.emplace_back(key, property_host::get(object, key));
    return result;
}

std::any callableSlot(const ParseMethod& method) {
    if (const auto* function = std::get_if<FunctionRef>(&method)) return *function;
    const Property& property = std::get<Property>(method);
    if (const auto* function = std::get_if<FunctionRef>(&property.value)) return *function;
    return property;
}

ParseResult invokeBare(const ParseMethod& method, ParserContext& parser,
                       const Property& token, const Property& args) {
    const ParseAtom parseAtom(token);
    const std::vector<Property> values = arguments::expand(args);
    std::vector<std::any> rest;
    rest.reserve(values.size());
    for (const auto& value : values) rest.emplace_back(value);
    const ParseResult result = invokeParseMethod(
        method, FunctionReceiver{Missing{}}, parser, parseAtom, rest);
    return parseResult(result);
}

} // namespace

ParseResult parseResult(ParseResult result) {
    if (const auto* value = std::get_if<Property>(&result); value && value->missing()) {
        return Property(true);
    }
    return result;
}

void AbstractTokenMap::initializeSource(Property name, ParseMethod parser) {
    name_ = std::move(name);
    parser_ = std::move(parser);
    MapHandler::registerMap(std::static_pointer_cast<TokenMap>(shared_from_this()));
}

Property AbstractTokenMap::name() const {
    return name_;
}

ParseMethod AbstractTokenMap::parser() const {
    return parser_;
}

void AbstractTokenMap::parser(ParseMethod parser) {
    parser_ = std::move(parser);
}

ParseMethod AbstractTokenMap::parserFor(const Property& token) {
    if (!contains(token)) return Property(PropertyNull{});
    return parser();
}

ParseResult AbstractTokenMap::parse(const ParseInput& input) {
    const ParseMethod method = parserFor(input.token);
    const TokenLookup mapped = lookup(input.token);
    if (!truthy(method) || !truthy(mapped)) return Property(PropertyNull{});
    const ParseAtom parseAtom = atom(mapped);
    const ParseResult result = invokeParseMethod(
        method, FunctionReceiver{Missing{}}, input.parser, parseAtom);
    return parseResult(result);
}

void RegExpMap::initializeSource(Property name, ParseMethod parser, RegExpRef regexp) {
    AbstractTokenMap::initializeSource(std::move(name), std::move(parser));
    regexp_ = std::move(regexp);
}

bool RegExpMap::contains(const Property& token) {
    if (!regexp_) typeError("RegExpMap regular expression is uninitialized");
    return regexp_->test(token);
}

TokenLookup RegExpMap::lookup(const Property& token) {
    return contains(token) ? TokenLookup{token}
                           : TokenLookup{Property(PropertyNull{})};
}

void AbstractParseMap::initializeSource(Property name, ParseMethod parser) {
    AbstractTokenMap::initializeSource(std::move(name), std::move(parser));
    map_.emplace();
}

TokenLookup AbstractParseMap::lookup(const Property& token) {
    if (!map_) typeError("TokenMap map is uninitialized");
    for (const auto& [key, value] : *map_) {
        if (property_host::sameValueZero(key, token)) return value;
    }
    return Property(Missing{});
}

bool AbstractParseMap::contains(const Property& token) {
    if (!map_) typeError("TokenMap map is uninitialized");
    return std::any_of(map_->begin(), map_->end(), [&](const auto& entry) {
        return property_host::sameValueZero(entry.first, token);
    });
}

void AbstractParseMap::add(Property token, TokenLookup object) {
    if (!map_) typeError("TokenMap map is uninitialized");
    if (const auto* number = std::get_if<double>(&token.value); number && *number == 0) {
        token = Property(0.0);
    }
    for (auto& [key, value] : *map_) {
        if (property_host::sameValueZero(key, token)) {
            value = std::move(object);
            return;
        }
    }
    map_->emplace_back(std::move(token), std::move(object));
}

void AbstractParseMap::remove(const Property& token) {
    if (!map_) typeError("TokenMap map is uninitialized");
    const auto found = std::find_if(map_->begin(), map_->end(), [&](const auto& entry) {
        return property_host::sameValueZero(entry.first, token);
    });
    if (found != map_->end()) map_->erase(found);
}

void CharacterMap::initializeSource(Property name, ParseMethod parser, Property json) {
    AbstractParseMap::initializeSource(std::move(name), std::move(parser));
    const std::vector<std::string> keys = property_host::ownKeys(json);
    for (const auto& key : keys) {
        const Property value = property_host::get(json, key);
        Property character;
        Property attributes;
        if (std::holds_alternative<std::string>(value.value)) {
            character = value;
            attributes = Property(PropertyNull{});
        } else {
            auto [sourceCharacter, sourceAttributes] = destructureTwo(value);
            character = std::move(sourceCharacter);
            attributes = std::move(sourceAttributes);
        }
        auto token = std::make_shared<Token>(Property(key), std::move(character),
                                             std::move(attributes));
        this->add(Property(key), TokenLookup{std::move(token)});
    }
}

ParseResult DelimiterMap::parse(const ParseInput& input) {
    const Property token = tokenValue(input.token);
    const ParseInput modified{input.parser, token};
    return AbstractTokenMap::parse(modified);
}

void MacroMap::initializeSource(Property name, Property json, Property functionMap) {
    if (functionMap.missing()) functionMap = Property(PropertyList{});
    AbstractParseMap::initializeSource(std::move(name), ParseMethod{Property(PropertyNull{})});

    const auto capturedEntries = entries(json);
    for (const auto& [key, value] : capturedEntries) {
        Property method;
        Property args;
        if (const auto array = PropertyArray::array(value)) {
            const Property sourceMethod = array->get(0);
            method = methodFromConfig(sourceMethod, functionMap);
            args = Property(array->slice(1));
        } else {
            method = methodFromConfig(value, functionMap);
            args = Property(PropertyArray{});
        }
        auto macro = std::make_shared<Macro>(Property(key),
                                             functions::fromProperty(method),
                                             std::move(args));
        this->add(Property(key), TokenLookup{std::move(macro)});
    }
}

ParseMethod MacroMap::parserFor(const Property& token) {
    const TokenLookup macro = lookup(token);
    if (!truthy(macro)) return Property(PropertyNull{});
    return methodField(macro);
}

ParseResult MacroMap::parse(const ParseInput& input) {
    const TokenLookup macro = lookup(input.token);
    const ParseMethod method = parserFor(input.token);
    if (!truthy(macro) || !truthy(method)) return Property(PropertyNull{});
    const Property token = field(macro, "token");
    const Property args = field(macro, "args");
    return invokeBare(method, input.parser, token, args);
}

ParseResult CommandMap::parse(const ParseInput& input) {
    const TokenLookup macro = lookup(input.token);
    const ParseMethod method = parserFor(input.token);
    if (!truthy(macro) || !truthy(method)) return Property(PropertyNull{});

    const Property savedCommand = input.parser.currentCS();
    input.parser.setCurrentCS(tokenValue(input.token));
    const Property macroToken = prefixedToken(u"\\", field(macro, "token"));
    const Property args = field(macro, "args");
    const ParseAtom parseAtom(macroToken);
    const std::vector<Property> values = arguments::expand(args);
    std::vector<std::any> rest;
    rest.reserve(values.size());
    for (const auto& value : values) rest.emplace_back(value);
    const ParseResult result = invokeParseMethod(
        method, FunctionReceiver{Missing{}}, input.parser, parseAtom, rest);
    input.parser.setCurrentCS(savedCommand);
    return parseResult(result);
}

void EnvironmentMap::initializeSource(Property name, ParseMethod parser,
                                       Property json, Property functionMap) {
    MacroMap::initializeSource(std::move(name), std::move(json), std::move(functionMap));
    this->parser(std::move(parser));
}

ParseResult EnvironmentMap::parse(const ParseInput& input) {
    const TokenLookup macro = lookup(input.token);
    const ParseMethod envParser = parserFor(input.token);
    if (!truthy(macro) || !truthy(envParser)) return Property(PropertyNull{});

    const ParseMethod generalParser = parser();
    const Property token = field(macro, "token");
    const std::any specificParser = callableSlot(envParser);
    const Property args = field(macro, "args");
    const std::array<std::any, 2> rest{specificParser, args};
    const ParseAtom parseAtom(token);
    const auto owner = std::static_pointer_cast<NativeObject>(shared_from_this());
    const ParseResult result = invokeParseMethod(
        generalParser, FunctionReceiver{owner}, input.parser, parseAtom, rest);
    return parseResult(result);
}

} // namespace folia::math::tex
