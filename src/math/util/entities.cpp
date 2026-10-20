// MathJax 4.1.3 util/Entities.ts; Apache-2.0.
module folia.math.util.entities;
import folia.math.util.js_numeric_input;
import folia.math.util.string;
import std;
#include "entities_data.inc"

namespace folia::math::entities {
namespace {
bool letter(char16_t unit) {
    return (unit >= u'a' && unit <= u'z') || (unit >= u'A' && unit <= u'Z');
}
bool digit(char16_t unit) {
    return unit >= u'0' && unit <= u'9';
}
bool hexDigit(char16_t unit) {
    return digit(unit) || (unit >= u'a' && unit <= u'f') || (unit >= u'A' && unit <= u'F');
}
std::u16string ascii(std::string_view text) {
    return {text.begin(), text.end()};
}
std::u16string suffixFile(std::u16string_view entity) {
    auto suffix = entity.substr(1);
    if (letter(entity.front()) && (suffix == u"fr" || suffix == u"scr" || suffix == u"opf"))
        return std::u16string(suffix);
    auto unit = entity.front();
    if (unit >= u'A' && unit <= u'Z')
        unit = static_cast<char16_t>(unit + u'a' - u'A');
    return std::u16string(1, unit);
}
void copyEntries(EntityTable &target, std::span<generated::Entry const> entries) {
    for (auto const &entry : entries)
        target.insert_or_assign(std::u16string(entry.name), std::u16string(entry.value));
}
} // namespace
std::u16string numeric(std::u16string_view entity) {
    const double value = !entity.empty() && entity.front() == u'x'
                             ? js_input::parseInt(entity.substr(1), js_input::IntegerRadix::Hex)
                             : js_input::parseInt(entity, js_input::IntegerRadix::Automatic);
    const std::array points{value};
    return string_util::unicodeString(points);
}
EntityModule::EntityModule() {
    copyEntries(entities_, generated::baseEntries);
}
void EntityModule::add(std::span<EntityEntry const> additions, std::u16string_view file) {
    for (auto const &[key, value] : additions)
        entities_.insert_or_assign(key, value);
    loaded_.emplace(file);
}
void EntityModule::remove(std::u16string_view entity) {
    auto found = entities_.find(entity);
    if (found != entities_.end())
        entities_.erase(found);
}
bool EntityModule::installShard(std::u16string_view file) {
    for (auto const &shard : generated::shardEntries) {
        const auto name = ascii(shard.name);
        if (file != name && file != u"./util/entities/" + name + u".js")
            continue;
        copyEntries(entities_, shard.entries);
        loaded_.insert(name);
        return true;
    }
    return false;
}
std::u16string EntityModule::translate(std::u16string_view text) {
    for (;;) {
        std::u16string output;
        std::size_t copied = 0;
        bool resourceLoaded = false;
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (text[i] != u'&')
                continue;
            const auto start = i + 1;
            auto end = start;
            if (end < text.size() && text[end] == u'#') {
                ++end;
                const bool hexadecimal =
                    end < text.size() && (text[end] == u'x' || text[end] == u'X');
                if (hexadecimal)
                    ++end;
                const auto digits = end;
                while (end < text.size() && (hexadecimal ? hexDigit(text[end]) : digit(text[end])))
                    ++end;
                if (digits == end)
                    continue;
            } else {
                if (end == text.size() || !letter(text[end]))
                    continue;
                while (end < text.size() && (letter(text[end]) || digit(text[end])))
                    ++end;
            }
            if (end == text.size() || text[end] != u';')
                continue;
            output.append(text.substr(copied, i - copied));
            auto entity = text.substr(start, end - start);
            if (entity.front() == u'#')
                output += numeric(entity.substr(1));
            else {
                auto found = entities_.find(entity);
                if ((found == entities_.end() || found->second.empty()) &&
                    options_.loadMissingEntities) {
                    const auto file = suffixFile(entity);
                    if (!loaded_.contains(file)) {
                        // Loading can restore entries overridden earlier in this input.
                        // Source retries the whole translation after each first-use load.
                        loaded_.insert(file);
                        installShard(file);
                        resourceLoaded = true;
                        break;
                    }
                }
                if (found != entities_.end() && !found->second.empty())
                    output += found->second;
                else
                    output.append(text.substr(i, end + 1 - i));
            }
            copied = end + 1;
            i = end;
        }
        if (resourceLoaded)
            continue;
        output.append(text.substr(copied));
        return output;
    }
}
} // namespace folia::math::entities
