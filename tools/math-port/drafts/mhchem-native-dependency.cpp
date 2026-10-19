// Direct native dependency observer. Expected output never enters this program.
import std;
import folia.math.dependencies.mhchem_parser;
namespace {
using namespace folia::math::mhchem;
std::u16string decode(std::string_view value) {
    std::u16string result;
    if (value == "-") return result;
    for (std::size_t i = 0; i < value.size(); i += 4) {
        unsigned unit = 0; const auto end = std::min(value.size(), i + 4);
        const auto parsed = std::from_chars(value.data() + i, value.data() + end, unit, 16);
        if (parsed.ec != std::errc{}) throw std::runtime_error("Invalid UTF-16 input");
        result.push_back(static_cast<char16_t>(unit));
    }
    return result;
}
std::string quote(std::u16string_view value) {
    constexpr char digits[] = "0123456789abcdef";
    std::string result = "\"";
    for (const auto unit : value) {
        result += "\\u";
        for (int shift = 12; shift >= 0; shift -= 4) result += digits[(unit >> shift) & 15];
    }
    return result + '"';
}
std::string output(const Output&);
std::string node(const Parsed& value) {
    if (value.raw) return quote(value.text);
    std::string result = "{\"type_\":" + quote(value.type);
    const auto scalar = [&](std::string_view field, const std::u16string& text) { result += ",\"" + std::string(field) + "\":" + quote(text); };
    const auto children = [&](std::string_view field, const Output& nodes) { result += ",\"" + std::string(field) + "\":" + output(nodes); };
    if (value.type == u"chemfive") {
        children("a", value.a); children("b", value.b); children("p", value.p);
        children("o", value.o); children("q", value.q); children("d", value.d);
        result += ",\"dType\":" + (value.dType.empty() ? std::string("{\"$\":\"undefined\"}") : quote(value.dType));
    } else if (value.type == u"arrow") {
        scalar("r", value.r); children("rd", value.rd); children("rq", value.rq);
    } else if (value.type == u"bond" || value.type == u"operator") scalar("kind_", value.kind);
    else if (value.type == u"color") { scalar("color1", value.color1); children("color2", value.color2); }
    else if (value.type == u"color0") scalar("color", value.color);
    else if (value.type == u"state of aggregation" || value.type == u"state of aggregation subscript") children("p1", value.p1Nodes);
    else if (value.type == u"pu-frac" || value.type == u"frac-ce" || value.type == u"overset" || value.type == u"underset" || value.type == u"underbrace") {
        children("p1", value.p1Nodes); children("p2", value.p2Nodes);
    } else if (value.type == u"frac") { scalar("p1", value.p1); scalar("p2", value.p2); }
    else if (value.type == u"rm" || value.type == u"text" || value.type == u"roman numeral" || value.type == u"tex-math" ||
             value.type == u"1st-level escape" || value.type == u"comma enumeration L" || value.type == u"comma enumeration M" || value.type == u"comma enumeration S") scalar("p1", value.p1);
    return result + '}';
}
std::string output(const Output& values) {
    std::string result = "[";
    for (const auto& value : values) { if (result.size() > 1) result += ','; result += node(value); }
    return result + ']';
}
std::string option(const Option& value) {
    if (const auto* text = std::get_if<std::u16string>(&value)) return quote(*text);
    if (const auto* number = std::get_if<double>(&value)) return std::to_string(*number);
    if (const auto* boolean = std::get_if<bool>(&value)) return *boolean ? "true" : "false";
    return "{\"$\":\"undefined\"}";
}
std::string tables() {
    std::string result = "{";
    for (const auto& machine : machines()) {
        if (result.size() > 1) result += ',';
        result += quote(machine.name) + ":{";
        bool firstState = true;
        for (const auto& state : machine.states) {
            if (!firstState) result += ','; firstState = false;
            result += quote(state.name) + ":[";
            bool firstRow = true;
            for (const auto& row : state.transitions) {
                if (!firstRow) result += ','; firstRow = false;
                result += "{\"pattern\":" + quote(row.pattern) + ",\"actions\":[";
                for (std::size_t i = 0; i < row.actions.size(); ++i) {
                    if (i) result += ',';
                    result += "{\"type\":" + quote(row.actions[i].type) + ",\"option\":" + option(row.actions[i].option) + '}';
                }
                result += "],\"nextState\":" + quote(row.nextState) + ",\"revisit\":" + (row.revisit ? "true" : "false") +
                    ",\"toContinue\":" + (row.toContinue ? "true" : "false") + '}';
            }
            result += ']';
        }
        result += '}';
    }
    return result + '}';
}
std::string observe(std::string mode, const std::u16string& name, const std::u16string& input) {
    if (mode == "tables") return tables();
    if (mode == "regexes") {
        std::string result = "[";
        for (const auto& pattern : regexPatterns()) {
            if (result.size() > 1) result += ',';
            result += "{\"name\":" + quote(pattern.name) + ",\"source\":" + quote(pattern.source) +
                ",\"flags\":" + quote(pattern.flags) + '}';
        }
        return result + ']';
    }
    if (mode == "pattern") {
        const auto found = match(name, input);
        if (!found) return "null";
        std::string captured;
        if (found->array) {
            captured = "[";
            for (std::size_t i = 0; i < found->captures.size(); ++i) {
                if (i) captured += ',';
                captured += found->captures[i] ? quote(*found->captures[i]) : "{\"$\":\"undefined\"}";
            }
            captured += ']';
        } else captured = quote(found->text);
        return "{\"match_\":" + captured + ",\"remainder\":" + quote(found->remainder) + '}';
    }
    auto parsed = parse(input, name);
    const auto before = output(parsed);
    const auto first = texify(parsed, name != u"tex");
    const auto after = output(parsed);
    const auto second = texify(parsed, name != u"tex");
    return "{\"parsed\":" + before + ",\"tex\":" + quote(first) + ",\"after\":" + after +
        ",\"repeat\":" + quote(second) + ",\"afterRepeat\":" + output(parsed) +
        ",\"toTex\":" + quote(toTex(input, name)) + '}';
}
}
int main() {
    std::string mode, name, input;
    while (std::cin >> mode >> name >> input) {
        try { const auto result = observe(mode, decode(name), decode(input)); std::cout << "{\"result\":" << result << ",\"error\":null}\n"; }
        catch (const Error& error) { std::cout << "{\"result\":null,\"error\":[" << quote(error.id) << ',' << quote(error.message) << "]}\n"; }
        catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 2; }
    }
}
