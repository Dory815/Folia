// MathJax 4.1.3 MapHandler.ts; Apache-2.0.
module folia.math.input.tex.parser;
import std;

namespace folia::math::tex {
void MapHandler::registerMap(TokenMapRef map) {
    if (!map) throw std::invalid_argument("Cannot register a null TeX map");
    std::u16string name(map->name());
    maps_.insert_or_assign(std::move(name), std::move(map));
}
TokenMapRef MapHandler::getMap(std::u16string_view name) const {
    auto map = maps_.find(name);
    return map == maps_.end() ? nullptr : map->second;
}
void SubHandler::add(std::span<std::u16string const> maps, FallbackHandler fallback, double priority) {
    for (auto index = maps.size(); index; --index) {
        auto map = registry_.getMap(maps[index - 1]);
        if (!map) {
            warnings_.warn(u"TexParser Warning: Configuration '" + maps[index - 1] + u"' not found! Omitted.");
            return;
        }
        configuration_.add(std::move(map), priority);
    }
    if (fallback) fallback_.add(fallback, priority);
}
void SubHandler::remove(std::span<std::u16string const> maps, FallbackHandler fallback) {
    for (auto const& name : maps) {
        auto map = retrieve(name);
        if (map) configuration_.remove(map);
    }
    if (fallback) fallback_.remove(fallback);
}
DispatchOutcome SubHandler::parse(TexParser& parser, std::u16string_view token) {
    for (auto entry : configuration_) {
        auto result = entry.item->parse(parser, token);
        if (std::holds_alternative<InvokeFallback>(result)) break;
        if (std::holds_alternative<Handled>(result) || std::holds_alternative<ReturnedFrame>(result))
            return result;
    }
    if (fallback_.entries().empty()) throw std::logic_error("No TeX fallback callback is configured");
    auto callback = fallback_.entries().front().item;
    std::visit([&](auto method) { method(parser, token); }, callback);
    return NoParseResult{};
}
TokenLookup SubHandler::lookup(std::u16string_view token) const {
    auto map = applicable(token);
    return map ? map->lookup(token) : TokenLookup{};
}
bool SubHandler::contains(std::u16string_view token) const {
    auto map = applicable(token);
    if (!map) return false;
    if (dynamic_cast<CharacterMap const*>(map.get())) {
        auto value = map->lookup(token);
        auto character = std::get_if<Token const*>(&value);
        return character && *character && (*character)->character.has_value();
    }
    return true;
}
std::u16string SubHandler::toString() const {
    std::u16string names;
    bool first = true;
    for (auto entry : configuration_) {
        if (!std::exchange(first, false)) names += u", ";
        names += entry.item->name();
    }
    return names;
}
TokenMapRef SubHandler::applicable(std::u16string_view token) const {
    for (auto entry : configuration_) if (entry.item->contains(token)) return entry.item;
    return nullptr;
}
TokenMapRef SubHandler::retrieve(std::u16string_view name) const {
    for (auto entry : configuration_) if (entry.item->name() == name) return entry.item;
    return nullptr;
}
void SubHandlers::add(std::span<HandlerRegistration const> registrations) {
    for (auto const& registration : registrations) {
        auto* handler = get(registration.kind);
        if (!handler) {
            set(registration.kind, std::make_unique<SubHandler>(registry_, warnings_));
            handler = get(registration.kind);
        }
        handler->add(registration.maps, registration.fallback, registration.priority);
    }
}
void SubHandlers::remove(std::span<HandlerRegistration const> registrations) {
    for (auto const& registration : registrations)
        if (auto* handler = get(registration.kind)) handler->remove(registration.maps, registration.fallback);
}
void SubHandlers::set(HandlerKind kind, std::unique_ptr<SubHandler> handler) {
    auto index = static_cast<std::size_t>(kind);
    if (index >= handlers_.size() || !handler) throw std::invalid_argument("Invalid TeX subhandler");
    if (!handlers_[index]) insertionOrder_.push_back(kind);
    handlers_[index] = std::move(handler);
}
SubHandler* SubHandlers::get(HandlerKind kind) noexcept {
    auto index = static_cast<std::size_t>(kind);
    return index < handlers_.size() ? handlers_[index].get() : nullptr;
}
SubHandler const* SubHandlers::get(HandlerKind kind) const noexcept {
    auto index = static_cast<std::size_t>(kind);
    return index < handlers_.size() ? handlers_[index].get() : nullptr;
}
TokenMapRef SubHandlers::retrieve(std::u16string_view name) const {
    for (auto kind : insertionOrder_) if (auto map = get(kind)->retrieve(name)) return map;
    return nullptr;
}
}
