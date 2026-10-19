module folia.math.output.common.font_path;
import std;

namespace folia::math::output::common {
namespace {
using Reason = FontOutlineErrorReason;
using scene::Point;
using scene::Unit;

struct Scanner {
    enum class NumberStatus { value, absent, invalid };
    std::string_view input;
    std::size_t pos{};
    bool previous_number{};

    void spaces() noexcept {
        while (pos < input.size()) {
            const char c = input[pos];
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') ++pos;
            else break;
        }
    }
    bool done() noexcept { spaces(); return pos == input.size(); }
    bool at_command() noexcept {
        spaces();
        return pos < input.size() && std::isalpha(static_cast<unsigned char>(input[pos]));
    }
    char take_command() noexcept { previous_number = false; return input[pos++]; }

    NumberStatus number(double& value, std::size_t& begin) noexcept {
        spaces();
        if (pos < input.size() && input[pos] == ',') {
            begin = pos;
            if (!previous_number) return NumberStatus::invalid;
            ++pos;
            spaces();
            if (pos == input.size() || input[pos] == ',' ||
                std::isalpha(static_cast<unsigned char>(input[pos])))
                return NumberStatus::invalid;
            previous_number = false;
        }
        begin = pos;
        if (pos == input.size() || std::isalpha(static_cast<unsigned char>(input[pos])))
            return NumberStatus::absent;

        if (input[pos] == '+' || input[pos] == '-') ++pos;
        bool has_digits = false;
        while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
            ++pos; has_digits = true;
        }
        if (pos < input.size() && input[pos] == '.') {
            ++pos;
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                ++pos; has_digits = true;
            }
        }
        if (!has_digits) return NumberStatus::invalid;
        if (pos < input.size() && (input[pos] == 'e' || input[pos] == 'E')) {
            ++pos;
            if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) ++pos;
            const auto exponent_begin = pos;
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) ++pos;
            if (pos == exponent_begin) return NumberStatus::invalid;
        }

        auto first = input.data() + begin;
        const auto last = input.data() + pos;
        // floating from_chars intentionally disallows a leading plus; SVG's
        // number grammar allows it, so skip only that sign for conversion.
        if (*first == '+') ++first;
        const auto [end, error] = std::from_chars(first, last, value);
        if (error != std::errc{} || end != last || !std::isfinite(value))
            return NumberStatus::invalid;
        previous_number = true;
        return NumberStatus::value;
    }
};

struct ParseFailure {
    FontOutlineError error;
};
}

FontOutlineDecodeResult decodeFontOutlinePrefix(std::string_view data, double unitsPerEm) {
    using namespace output::scene;
    if (!std::isfinite(unitsPerEm) || unitsPerEm <= 0)
        throw std::invalid_argument("invalid font units-per-em");

    Scanner scanner{data};
    PathGeometry geometry;
    Point current{}; // font design units, y-up
    Point subpath_start{};
    char active{};
    bool has_initial_move{};

    auto fail = [&](std::size_t offset, char command, Reason reason) -> void {
        throw ParseFailure{FontOutlineError{offset, command, reason}};
    };
    auto read_value = [&](char command) -> double {
        double value{};
        std::size_t offset{};
        switch (scanner.number(value, offset)) {
        case Scanner::NumberStatus::value: return value;
        case Scanner::NumberStatus::absent:
            fail(offset, command, Reason::incomplete_segment);
        case Scanner::NumberStatus::invalid:
            fail(offset, command, Reason::invalid_number);
        }
        std::unreachable();
    };
    auto read_point = [&](char command, bool relative, Point origin) -> Point {
        const Unit x = read_value(command);
        const Unit y = read_value(command);
        if (relative) return Point{origin.x + x, origin.y + y};
        return Point{x, y};
    };
    auto normalized = [&](Point p, char command) -> Point {
        if (!std::isfinite(p.x) || !std::isfinite(p.y))
            fail(scanner.pos, command, Reason::invalid_number);
        Point result{p.x / unitsPerEm, p.y / unitsPerEm};
        if (!std::isfinite(result.x) || !std::isfinite(result.y))
            fail(scanner.pos, command, Reason::invalid_number);
        return result;
    };

    try {
        while (!scanner.done()) {
            if (scanner.at_command()) active = scanner.take_command();
            if (!active) {
                fail(scanner.pos, '\0', Reason::missing_command);
            }
            const bool relative = active >= 'a' && active <= 'z';
            const char command = static_cast<char>(std::toupper(static_cast<unsigned char>(active)));
            if (!has_initial_move && command != 'M')
                fail(scanner.pos - 1, active, Reason::missing_command);
            if (command == 'Z') {
                geometry.segments.emplace_back(ClosePath{});
                current = subpath_start;
                active = 0;
                continue;
            }
            if (command != 'M' && command != 'L' && command != 'C' && command != 'Q')
                fail(scanner.pos - 1, active, Reason::unsupported_command);

            const Point origin = current;
            if (command == 'M') {
                const Point to = read_point(active, relative, origin);
                geometry.segments.emplace_back(MoveTo{normalized(to, active)});
                current = subpath_start = to;
                has_initial_move = true;
                // SVG applies moveto's first pair only to M; later pairs are L.
                active = relative ? 'l' : 'L';
            } else if (command == 'L') {
                const Point to = read_point(active, relative, origin);
                geometry.segments.emplace_back(LineTo{normalized(to, active)});
                current = to;
            } else if (command == 'Q') {
                const Point control = read_point(active, relative, origin);
                const Point to = read_point(active, relative, origin);
                geometry.segments.emplace_back(QuadTo{normalized(control, active), normalized(to, active)});
                current = to;
            } else {
                const Point control1 = read_point(active, relative, origin);
                const Point control2 = read_point(active, relative, origin);
                const Point to = read_point(active, relative, origin);
                geometry.segments.emplace_back(CubicTo{
                    normalized(control1, active), normalized(control2, active), normalized(to, active)});
                current = to;
            }
        }
    } catch (ParseFailure const& failure) {
        return FontOutlineDecodeResult{GlyphOutline{std::move(geometry)}, failure.error};
    }
    return FontOutlineDecodeResult{GlyphOutline{std::move(geometry)}, std::nullopt};
}

output::scene::GlyphOutline decodeFontOutline(std::string_view data, double unitsPerEm) {
    auto result = decodeFontOutlinePrefix(data, unitsPerEm);
    if (result.error) {
        throw std::invalid_argument("invalid SVG path data at byte " +
            std::to_string(result.error->byte_offset));
    }
    return std::move(result.outline);
}
}
