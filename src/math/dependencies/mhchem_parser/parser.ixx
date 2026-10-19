// Native port of mhchemparser 4.2.1, src/mhchemParser.ts and its declarations.
// Copyright (c) 2015-2023 Martin Hensel. Apache-2.0.
export module folia.math.dependencies.mhchem_parser;
import std;
export namespace folia::math::mhchem {
struct Error : std::exception {
    std::u16string id;
    std::u16string message;
    Error(std::u16string id, std::u16string message)
        : id(std::move(id)), message(std::move(message)) {}
    const char* what() const noexcept override { return "mhchem parser error"; }
};
// ParsedWithoutString has a closed set of fields in the original declaration.
// String fields and recursively parsed fields retain their distinct types.
struct Parsed {
    bool raw = false;
    std::u16string text, type, p1, p2, kind, color, color1, r, dType;
    std::vector<Parsed> a, b, p, o, q, d, rd, rq, p1Nodes, p2Nodes, color2;
    static Parsed literal(std::u16string value) {
        Parsed result; result.raw = true; result.text = std::move(value); return result;
    }
    static Parsed node(std::u16string type) {
        Parsed result; result.type = std::move(type); return result;
    }
};
using Output = std::vector<Parsed>;
struct Buffer {
    std::u16string a, b, p, o, q, d, dType, r, rdt, rd, rqt, rq, text, rm;
    double parenthesisLevel = 0;
    bool sb = false, beginsWithBond = false;
};
struct Match {
    bool array = false;
    std::u16string text, remainder;
    std::vector<std::optional<std::u16string>> captures;
    std::u16string at(std::size_t i) const {
        return i < captures.size() && captures[i] ? *captures[i] : std::u16string{};
    }
};
using Option = std::variant<std::monostate, std::u16string, double, bool>;
struct Action { std::u16string type; Option option; };
struct Transition {
    std::u16string pattern;
    std::vector<Action> actions;
    std::u16string nextState;
    bool revisit = false, toContinue = false;
};
struct State { std::u16string name; std::vector<Transition> transitions; };
struct Machine { std::u16string name; std::vector<State> states; };
struct RegexPattern { std::u16string name, source, flags; };
const std::vector<Machine>& machines();
const std::vector<RegexPattern>& regexPatterns();
std::optional<Match> match(std::u16string_view pattern, std::u16string_view input);
Output parse(std::u16string input, std::u16string_view machine = u"ce");
Output action(std::u16string_view machine, std::u16string_view name,
              Buffer&, const Match&, const Option& = {});
std::u16string texify(Output&, bool addOuterBraces = false);
std::u16string toTex(std::u16string input, std::u16string_view machine = u"ce");
// Shared native regexp utilities used by pattern and action routines.
std::optional<Match> regexMatch(std::u16string_view input, std::u16string_view pattern,
                               bool captureArray = false);
std::u16string replaceRegex(std::u16string input, std::u16string_view pattern,
                           std::u16string_view replacement, bool global = false);
}
