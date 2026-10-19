/*************************************************************
 * SVG font-path boundary for backend-independent glyph geometry.
 * Keeps the source spelling and decodes command tuples exactly once.
 *************************************************************/
module folia.math.output.scene.foundation;
import std;

namespace folia::math::output::scene {
namespace {
bool asciiAlpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
bool separator(char c) { return c == ',' || c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
bool supported(char c) {
    switch (c) {
        case 'M': case 'm': case 'L': case 'l': case 'H': case 'h':
        case 'V': case 'v': case 'C': case 'c': case 'S': case 's':
        case 'Q': case 'q': case 'T': case 't': case 'A': case 'a':
        case 'Z': case 'z': return true;
        default: return false;
    }
}
std::size_t arity(char c) {
    switch (static_cast<char>(c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c)) {
        case 'M': case 'L': case 'T': return 2;
        case 'H': case 'V': return 1;
        case 'C': return 6;
        case 'S': case 'Q': return 4;
        case 'A': return 7;
        default: return 0;
    }
}
[[noreturn]] void malformed() { throw std::runtime_error("Malformed SVG glyph path data"); }

double numberAt(std::string_view source, std::size_t& position) {
    const auto start = position;
    if (position < source.size() && (source[position] == '+' || source[position] == '-')) ++position;
    bool digits = false;
    while (position < source.size() && source[position] >= '0' && source[position] <= '9') {
        digits = true; ++position;
    }
    if (position < source.size() && source[position] == '.') {
        ++position;
        while (position < source.size() && source[position] >= '0' && source[position] <= '9') {
            digits = true; ++position;
        }
    }
    if (!digits) malformed();
    if (position < source.size() && (source[position] == 'e' || source[position] == 'E')) {
        ++position;
        if (position < source.size() && (source[position] == '+' || source[position] == '-')) ++position;
        const auto exponent = position;
        while (position < source.size() && source[position] >= '0' && source[position] <= '9') ++position;
        if (position == exponent) malformed();
    }
    double value = 0;
    auto first = source.data() + start;
    if (*first == '+') ++first;
    const auto parsed = std::from_chars(first, source.data() + position, value, std::chars_format::general);
    if (parsed.ec != std::errc{} || parsed.ptr != source.data() + position || !std::isfinite(value)) malformed();
    return value;
}
double flagAt(std::string_view source, std::size_t& position) {
    if(position>=source.size()||(source[position]!='0'&&source[position]!='1'))malformed();
    return source[position++]=='1'?1:0;
}
}

std::shared_ptr<const GlyphOutline> decodeGlyphOutline(std::string sourcePath) {
    auto result = std::make_shared<GlyphOutline>();
    result->sourcePath = std::move(sourcePath);
    const std::string_view source(result->sourcePath);
    std::size_t position = 0;
    char command = 0;
    bool firstMove = false;
    while (position < source.size()) {
        while (position < source.size() && separator(source[position])) ++position;
        if (position == source.size()) break;
        if (asciiAlpha(source[position])) {
            command = source[position++];
            if (!supported(command)) malformed();
            if (command == 'Z' || command == 'z') {
                result->commands.push_back(PathCommand{command,{}});
                command = 0;
                firstMove = false;
                continue;
            }
            firstMove = command == 'M' || command == 'm';
        } else if (!command) {
            malformed();
        }
        const auto count = arity(command);
        if (!count) malformed();
        std::vector<double> arguments;
        arguments.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            while (position < source.size() && separator(source[position])) ++position;
            if (position == source.size() || asciiAlpha(source[position])) malformed();
            if((command=='A'||command=='a')&&(i==3||i==4))
                arguments.push_back(flagAt(source,position));
            else
                arguments.push_back(numberAt(source, position));
        }
        result->commands.push_back(PathCommand{command,std::move(arguments)});
        if (firstMove) {
            command = command == 'M' ? 'L' : 'l';
            firstMove = false;
        }
    }
    return result;
}
}
