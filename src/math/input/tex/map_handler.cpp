/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 input/tex/MapHandler.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * Source SHA256: 7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2
 *************************************************************/
module folia.math.input.tex.map_handler;

import folia.math.util.property_host;
import folia.math.util.utf16;
import std;

namespace folia::math::tex {
namespace {

using Registry = std::vector<std::pair<Property, TokenMapRef>>;

Registry& mapRegistry() {
    static Registry maps;
    return maps;
}

bool sameValueZero(const Property& left, const Property& right) {
    return property_host::sameValueZero(left, right);
}

bool strictEqual(const Property& left, const Property& right) {
    if (const auto* a = std::get_if<double>(&left.value)) {
        if (const auto* b = std::get_if<double>(&right.value)) return *a == *b;
    }
    return left == right;
}

bool jsNull(const ParseResult& result) {
    return std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::same_as<T, Property>) {
            if (std::holds_alternative<PropertyNull>(value.value)) return true;
            if (const auto* object = std::get_if<std::shared_ptr<PropertyStorage>>(&value.value)) return !*object;
            return false;
        } else {
            return !value;
        }
    }, result);
}

bool strictEqual(const ParseResult& left, const ParseResult& right) {
    if (jsNull(left) || jsNull(right)) return jsNull(left) && jsNull(right);
    if (left.index() != right.index()) return false;
    return std::visit([](const auto& a, const auto& b) {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;
        if constexpr (std::same_as<A, B>) {
            if constexpr (std::same_as<A, Property>) return strictEqual(a, b);
            else return a == b;
        } else {
            return false;
        }
    }, left, right);
}

bool truthy(const ParseMethod& method) {
    if (const auto* function = std::get_if<FunctionRef>(&method)) return bool(*function);
    return property_host::truthy(std::get<Property>(method));
}

bool truthy(const ParseResult& result) {
    return std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::same_as<T, Property>) return property_host::truthy(value);
        else return bool(value);
    }, result);
}

Property mapName(const TokenMapRef& map) {
    if (!map) property_host::typeError("Cannot read name of null token map");
    return map->name();
}

std::u16string interpolation(const Property& value) {
    return property_host::string(value);
}

bool fallbackIdentity(const ParseResult& result) {
    return strictEqual(result, SubHandler::FALLBACK);
}

Property characterOf(const TokenLookup& lookup) {
    if (const auto* token = std::get_if<TokenRef>(&lookup)) {
        if (!*token) property_host::typeError("Cannot read character of null token");
        return (*token)->character();
    }
    if (const auto* macro = std::get_if<MacroRef>(&lookup)) {
        if (!*macro) property_host::typeError("Cannot read character of null macro");
        return Missing{};
    }
    return property_host::get(std::get<Property>(lookup), "char");
}

std::u16string joinMapNames(const std::vector<Property>& names) {
    std::u16string result;
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (i) result += u", ";
        if (property_host::nullish(names[i])) continue;
        result += property_host::string(names[i]);
    }
    return result;
}

} // namespace

ParseResult SubHandler::FALLBACK = ParseSymbolRef(std::make_shared<ParseSymbol>(ParseSymbol{u"fallback"}));

void MapHandler::registerMap(TokenMapRef map) {
    const Property name = mapName(map); // Source reads map.name once before Map.set.
    auto& maps = mapRegistry();
    for (auto& entry : maps) {
        if (sameValueZero(entry.first, name)) {
            entry.second = std::move(map);
            return;
        }
    }
    maps.emplace_back(name, std::move(map));
}

TokenMapRef MapHandler::getMap(const Property& name) {
    for (const auto& [key, map] : mapRegistry()) {
        if (sameValueZero(key, name)) return map;
    }
    return {};
}

SubHandler::SubHandler(WarningSink warning) : warning_(std::move(warning)) {}

void SubHandler::warn(std::u16string message) {
    std::u16string output = u"TexParser Warning: ";
    output += message;
    if (warning_) {
        warning_(output);
        return;
    }
    std::cout << property_host::debugText(output) << '\n';
}

void SubHandler::add(const Property& maps, ParseMethod fallback, double priority) {
    auto input = PropertyArray::array(maps);
    if (!input) property_host::typeError("MapHandler.add expects an array of map names");

    const PropertyArray snapshot = input->spread();
    for (std::size_t remaining = snapshot.size(); remaining > 0; --remaining) {
        const Property name = snapshot.get(remaining - 1);
        const TokenMapRef map = MapHandler::getMap(name);
        if (!map) {
            std::u16string message = u"Configuration '";
            message += interpolation(name);
            message += u"' not found! Omitted.";
            warn(std::move(message));
            return;
        }
        configuration_.add(map, priority);
    }
    if (truthy(fallback)) fallback_.add(std::move(fallback), priority);
}

void SubHandler::remove(const Property& maps, ParseMethod fallback) {
    auto input = PropertyArray::array(maps);
    if (!input) property_host::typeError("MapHandler.remove expects an array of map names");

    for (std::size_t index = 0; index < input->size(); ++index) {
        const Property name = input->get(index);
        const TokenMapRef map = retrieve(name);
        if (map) configuration_.remove(map);
    }
    if (truthy(fallback)) fallback_.remove(fallback);
}

ParseResult SubHandler::parse(const ParseInput& input) {
    for (auto current = configuration_.begin(), finish = configuration_.end(); current != finish; ++current) {
        const auto record = current.record();
        if (!record) property_host::typeError("Null token map configuration record");
        const TokenMapRef map = record->item;
        if (!map) property_host::typeError("Null token map configuration item");
        ParseResult result = map->parse(input);
        if (fallbackIdentity(result)) break;
        if (truthy(result)) return result;
    }

    // Source materializes Array.from(_fallback) only after map parsing. Keep
    // the selected record itself so invoke supplies the real item receiver.
    std::vector<std::shared_ptr<FunctionListItem>> snapshot;
    for (auto current = fallback_.begin(), finish = fallback_.end(); current != finish; ++current) {
        snapshot.push_back(current.record());
    }
    if (snapshot.empty()) functions::typeError(u"Cannot read properties of undefined (reading 'item')");
    if (!snapshot.front()) functions::typeError(u"FunctionList item record is null");

    const ParseAtom atom(input.token);
    const std::array<std::any, 2> arguments{std::ref(input.parser), atom};
    (void)functions::invoke(snapshot.front(), arguments);
    return Property(Missing{}); // Source returns undefined, irrespective of callback result.
}

TokenLookup SubHandler::lookup(const Property& token) {
    const TokenMapRef map = applicable(token);
    if (map) return map->lookup(token);
    return Property(PropertyNull{});
}

bool SubHandler::contains(const Property& token) {
    const TokenMapRef map = applicable(token);
    if (!map) return false;
    if (!dynamic_cast<CharacterMapIdentity*>(map.get())) return true;
    return !std::holds_alternative<PropertyNull>(characterOf(map->lookup(token)).value);
}

std::string SubHandler::toString() {
    std::vector<Property> names;
    for (auto current = configuration_.begin(), finish = configuration_.end(); current != finish; ++current) {
        const auto record = current.record();
        if (!record) property_host::typeError("Null token map configuration record");
        names.push_back(mapName(record->item));
    }
    return property_host::debugText(joinMapNames(names));
}

TokenMapRef SubHandler::applicable(const Property& token) {
    for (auto current = configuration_.begin(), finish = configuration_.end(); current != finish; ++current) {
        const auto record = current.record();
        if (!record) property_host::typeError("Null token map configuration record");
        const TokenMapRef map = record->item;
        if (!map) property_host::typeError("Null token map configuration item");
        if (map->contains(token)) return map;
    }
    return {};
}

TokenMapRef SubHandler::retrieve(const Property& name) {
    for (auto current = configuration_.begin(), finish = configuration_.end(); current != finish; ++current) {
        const auto record = current.record();
        if (!record) property_host::typeError("Null token map configuration record");
        const TokenMapRef map = record->item;
        if (!map) property_host::typeError("Null token map configuration item");
        if (strictEqual(mapName(map), name)) return map;
    }
    return {};
}

SubHandlers::SubHandlers(WarningSink warning) : warning_(std::move(warning)) {}

void SubHandlers::add(const HandlerConfig& handlers, const FallbackConfig& fallbacks, double priority) {
    const std::vector<std::string> keys = handlers.keys();
    for (const std::string& name : keys) {
        SubHandlerRef handler = get(name);
        if (!handler) {
            handler = std::make_shared<SubHandler>(warning_);
            set(name, handler);
        }
        const Property maps = handlers.get(name);
        const ParseMethod fallback = fallbacks.get(name);
        handler->add(maps, fallback, priority);
    }
}

void SubHandlers::remove(const HandlerConfig& handlers, const FallbackConfig& fallbacks) {
    const std::vector<std::string> keys = handlers.keys();
    for (const std::string& name : keys) {
        const SubHandlerRef handler = get(name);
        if (handler) {
            const Property maps = handlers.get(name);
            const ParseMethod fallback = fallbacks.get(name);
            handler->remove(maps, fallback);
        }
    }
}

void SubHandlers::set(std::string name, SubHandlerRef handler) {
    for (auto& entry : *map_) {
        if (entry.first == name) {
            entry.second = std::move(handler);
            return;
        }
    }
    map_->emplace_back(std::move(name), std::move(handler));
}

SubHandlerRef SubHandlers::get(std::string_view name) {
    for (const auto& entry : *map_) if (entry.first == name) return entry.second;
    return {};
}

TokenMapRef SubHandlers::retrieve(const Property& name) {
    for (std::size_t index = 0; index < map_->size(); ++index) {
        const SubHandlerRef handler = (*map_)[index].second;
        if (!handler) property_host::typeError("Cannot read properties of null (reading 'retrieve')");
        const TokenMapRef map = handler->retrieve(name);
        if (map) return map;
    }
    return {};
}

std::optional<std::string> SubHandlers::KeyIterator::next() {
    if (done_) return std::nullopt;
    if (!storage_) property_host::typeError("SubHandlers key iterator storage is null");
    if (index_ >= storage_->size()) {
        done_ = true;
        return std::nullopt;
    }
    return (*storage_)[index_++].first;
}

} // namespace folia::math::tex
