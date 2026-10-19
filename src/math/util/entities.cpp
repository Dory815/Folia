module folia.math.util.entities;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import folia.math.util.property_host;
import folia.math.util.retries;
import folia.math.util.utf16;
import std;

#include "entities_data.inc"

namespace folia::math::entities {
namespace {
void copyEntries(PropertyList& target, std::span<const generated::Entry> entries) {
    for (const auto& entry : entries) {
        target.setOwn(property_host::debugText(entry.name), Property(property_host::debugText(entry.value)));
    }
}

bool installGeneratedShard(PropertyList entities, PropertyList loaded, std::string_view name) {
    for (const auto& shard : generated::shardEntries) {
        if (name != shard.name) continue;
        PropertyList additions;
        copyEntries(additions, shard.entries);
        for (const auto& key : additions.keys()) entities.set(key, additions.getOwn(key));
        // Each Source shard calls Entities.add(..., 'fr'/'scr'/...), so the
        // loaded table is keyed by that short module name, not its import path.
        loaded.set(std::string(name), Property(true));
        return true;
    }
    return false;
}

std::u16string fromCodePoint(double point) {
    if (!std::isfinite(point) || std::trunc(point) != point || point < 0 || point > 0x10FFFF) {
        throw options::Exception("RangeError", u"Invalid code point " + property_host::text(js_number::toString(point)));
    }
    const auto cp = static_cast<std::uint32_t>(point);
    if (cp <= 0xFFFF) return std::u16string(1, static_cast<char16_t>(cp));
    const auto scalar = cp - 0x10000;
    return {static_cast<char16_t>(0xD800 + (scalar >> 10)),
            static_cast<char16_t>(0xDC00 + (scalar & 0x3FF))};
}

std::u16string firstCodeUnit(std::u16string_view value) {
    if (value.empty()) return {};
    return std::u16string(value.substr(0, 1));
}

std::u16string suffixFile(std::u16string_view entity) {
    static const std::u16string suffixPattern = u"^([a-zA-Z])(fr|scr|opf)$";
    regexp_host::RegExp suffix(suffixPattern);
    auto match = suffix.exec(std::u16string(entity));
    if (match) {
        if (auto group = match->at(2)) return *group;
    }
    const auto first = firstCodeUnit(entity);
    if (first.empty()) return {};
    const auto c = first[0];
    return std::u16string(1, c >= u'A' && c <= u'Z' ? static_cast<char16_t>(c + 0x20) : c);
}

std::u16string replaceEntity(const Property& entityValue) {
    return property_host::string(entityValue);
}
}

EntityModule::EntityModule(async::QueueRef queue, EntityLoader loader)
    : options_{{"loadMissingEntities", Property(true)}},
      queue_(std::move(queue)), asyncLoad_(std::move(loader)) {
    if (!queue_) queue_ = std::make_shared<async::JobQueue>();
    copyEntries(entities_, generated::baseEntries);
}

void EntityModule::add(const PropertyList& additions, std::u16string_view file) {
    // Object.assign is an own-enumerable-key snapshot followed by live Get and
    // target Set. `keys()` and `getOwn()` preserve this Property graph's order
    // and accessors, while mutations affect the one exported table object.
    for (const auto& key : additions.keys()) entities_.set(key, additions.getOwn(key));
    loaded_.set(property_host::debugText(file), Property(true));
}

void EntityModule::remove(std::u16string_view entity) {
    entities_.erase(property_host::debugText(entity));
}

bool EntityModule::installShard(std::u16string_view file) {
    for (const auto& shard : generated::shardEntries) {
        const auto sourcePath = property_host::text(std::string("./util/entities/") +
                                                     std::string(shard.name) + ".js");
        if (file != sourcePath && file != property_host::text(shard.name)) continue;
        return installGeneratedShard(entities_, loaded_, shard.name);
    }
    return false;
}

std::u16string EntityModule::numeric(std::u16string_view entity) const {
    const auto number = !entity.empty() && entity.front() == u'x'
        ? js_input::parseInt(entity.substr(1), js_input::IntegerRadix::Hex)
        : js_input::parseInt(entity, js_input::IntegerRadix::Automatic);
    return fromCodePoint(number);
}

void EntityModule::requestShard(std::u16string_view file) {
    const auto sourcePath = std::string("./util/entities/") + property_host::debugText(file) + ".js";
    if (!asyncLoad_) {
        auto failed = async::Promise::create(queue_);
        failed->reject(std::u16string(u"Can't load '" + property_host::text(sourcePath) +
                                      u"': No mathjax.asyncLoad method specified"));
        retries::retryAfter(std::move(failed));
    }

    async::PromiseRef loaded;
    try {
        loaded = asyncLoad_(sourcePath);
    } catch (...) {
        // AsyncLoad wraps a synchronous host throw in a rejected Promise before
        // Entities passes it to retryAfter.
        loaded = async::Promise::create(queue_);
        loaded->reject(async::currentReason());
    }
    if (!loaded) {
        loaded = async::Promise::create(queue_);
        loaded->resolve();
    }
    if (!loaded->belongsTo(queue_)) {
        throw options::Exception("TypeError", u"Entity shard Promise belongs to a different session");
    }
    retries::retryAfter(std::move(loaded));
}

std::u16string EntityModule::translate(std::u16string_view input) {
    // This is the exact global, case-insensitive, UTF-16 regex used by Source.
    regexp_host::RegExp expression(u"&([a-z][a-z0-9]*|#(?:[0-9]+|x[0-9a-f]+));", u"gi");
    const std::u16string text(input);
    std::u16string output;
    std::size_t copied = 0;
    while (const auto match = expression.exec(text)) {
        const auto whole = match->at(0);
        const auto capture = match->at(1);
        if (!whole || !capture) continue;
        output.append(text, copied, match->index - copied);
        const auto& entity = *capture;
        if (!entity.empty() && entity.front() == u'#') {
            output += numeric(std::u16string_view(entity).substr(1));
        } else {
            const auto key = property_host::debugText(entity);
            const auto value = entities_.get(key);
            if (property_host::truthy(value)) {
                output += replaceEntity(value);
            } else {
                if (property_host::truthy(options_.get("loadMissingEntities"))) {
                    const auto file = suffixFile(entity);
                    const auto keyFile = property_host::debugText(file);
                    if (!property_host::truthy(loaded_.get(keyFile))) {
                        // Source marks the shard loaded before invoking
                        // asyncLoad; a rejected retry is therefore not retried
                        // by the next synchronous translate call.
                        loaded_.set(keyFile, Property(true));
                        requestShard(file);
                    }
                }
                output += *whole;
            }
        }
        copied = match->index + whole->size();
    }
    output.append(text, copied, std::u16string::npos);
    return output;
}
}
