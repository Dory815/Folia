// Native observer candidate for tools/math-port/entities-original-oracle.cjs.
// Root owns registration and independent comparison.
import std;
import folia.math.util.entities;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
using namespace folia::math::entities;

std::u16string widen(std::string_view value) { return property_host::text(value); }

void quote(std::ostream& out, std::u16string_view value) {
    constexpr char hex[] = "0123456789abcdef";
    out.put('"');
    for (char16_t unit : value) {
        switch (unit) {
        case u'"': out << "\\\""; break;
        case u'\\': out << "\\\\"; break;
        case u'\b': out << "\\b"; break;
        case u'\f': out << "\\f"; break;
        case u'\n': out << "\\n"; break;
        case u'\r': out << "\\r"; break;
        case u'\t': out << "\\t"; break;
        default:
            if (unit < 0x20 || unit > 0x7e) {
                out << "\\u" << hex[(unit >> 12) & 15] << hex[(unit >> 8) & 15]
                    << hex[(unit >> 4) & 15] << hex[unit & 15];
            } else out.put(static_cast<char>(unit));
        }
    }
    out.put('"');
}

void table(std::ostream& out, const PropertyList& values,
           const PropertyList* before = nullptr) {
    out.put('{');
    bool first = true;
    for (const auto& key : values.keys()) {
        const auto current = values.getOwn(key);
        if (before) {
            const auto previous = before->getOwn(key);
            if (!previous.missing() && previous == current) continue;
        }
        if (!first) out.put(',');
        first = false;
        quote(out, widen(key));
        out.put(':');
        if (current.missing() || !std::holds_alternative<std::string>(current.value))
            throw std::runtime_error("entity table contains a non-string own value");
        quote(out, widen(std::get<std::string>(current.value)));
    }
    out.put('}');
}

void error(std::ostream& out, const options::Exception& exception) {
    out << "{\"name\":\"" << exception.name << "\",\"message\":";
    quote(out, exception.message);
    out.put('}');
}

void numericRow(std::ostream& out, std::u16string_view entity) {
    out.put('[');
    quote(out, entity);
    out.put(',');
    try { quote(out, EntityModule{}.numeric(entity)); }
    catch (const options::Exception& exception) { error(out, exception); }
    out.put(']');
}

void translateRow(std::ostream& out, std::u16string_view input) {
    out.put('[');
    quote(out, input);
    out.put(',');
    try {
        EntityModule module;
        module.options().set("loadMissingEntities", Property(false));
        quote(out, module.translate(input));
    } catch (const options::Exception& exception) { error(out, exception); }
    out.put(']');
}
}

int main() {
    const std::array<std::string_view, 29> shardNames = {
        "a","b","c","d","e","f","g","h","i","j","k","l","m","n",
        "o","p","q","r","s","t","u","v","w","x","y","z","fr","opf","scr"};
    std::cout << "{\"base\":";
    EntityModule base;
    table(std::cout, base.table());
    std::cout << ",\"shards\":{";
    for (std::size_t i = 0; i < shardNames.size(); ++i) {
        if (i) std::cout.put(',');
        quote(std::cout, widen(shardNames[i]));
        std::cout << ':';
        EntityModule module;
        PropertyList previous;
        for (const auto& key : module.table().keys())
            previous.setOwn(key, module.table().getOwn(key));
        if (!module.installShard(widen(shardNames[i])))
            throw std::runtime_error("pinned native entity shard was not installed");
        table(std::cout, module.table(), &previous);
    }
    std::cout << "},\"directNumeric\":[";
    const std::array<std::u16string_view, 10> numerics = {
        u"0",u"65",u"x41",u"x1F600",u"xD800",u"x110000",u"X41",u"",u"1.9",
        u"999999999999999999999999999999999999999999"};
    for (std::size_t i = 0; i < numerics.size(); ++i) {
        if (i) std::cout.put(',');
        numericRow(std::cout, numerics[i]);
    }
    std::cout << "],\"numeric\":[";
    const std::array<std::u16string_view, 5> numericInputs = {
        u"&#65;",u"&#x41;",u"&#x1F600;",u"&#0;",u"&#xD800;"};
    for (std::size_t i = 0; i < numericInputs.size(); ++i) {
        if (i) std::cout.put(',');
        translateRow(std::cout, numericInputs[i]);
    }
    std::cout << "],\"replacement\":[";
    const std::array<std::u16string_view, 6> replacements = {
        u"&alpha;",u"&Alpha;",u"&unknown;",u"&amp;",u"&bad_name;",u"&#xZZ;"};
    for (std::size_t i = 0; i < replacements.size(); ++i) {
        if (i) std::cout.put(',');
        translateRow(std::cout, replacements[i]);
    }
    std::cout << "]}\n";
}
