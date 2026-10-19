// Native port of mhchemparser 4.2.1 src/mhchemParser.ts: all pattern functions.
// Copyright (c) 2015-2023 Martin Hensel. Apache-2.0.
module folia.math.dependencies.mhchem_parser;
import std;
namespace folia::math::mhchem {
namespace {
struct Boundary { std::u16string text; bool regex = false; };
Boundary literal(std::u16string_view text) { return {std::u16string(text), false}; }
Boundary expression(std::u16string_view text) { return {std::u16string(text), true}; }
std::optional<std::u16string> prefix(std::u16string_view input, const Boundary& boundary) {
    if (!boundary.regex) {
        if (!input.starts_with(boundary.text)) return {};
        return boundary.text;
    }
    const auto found = regexMatch(input, boundary.text);
    if (!found) return {};
    return found->text;
}
std::optional<Match> groups(std::u16string_view input, Boundary beginExclude,
                            Boundary beginInclude, Boundary endInclude, Boundary endExclude,
                            Boundary begin2Exclude = {}, Boundary begin2Include = {},
                            Boundary end2Include = {}, Boundary end2Exclude = {}, bool combine = false) {
    const auto excluded = prefix(input, beginExclude);
    if (!excluded) return {};
    input.remove_prefix(excluded->size());
    const auto included = prefix(input, beginInclude);
    if (!included) return {};
    const Boundary& end = !endInclude.text.empty() ? endInclude : endExclude;
    int braces = 0;
    std::optional<std::pair<std::size_t, std::size_t>> finish;
    for (std::size_t i = included->size(); i < input.size(); ++i) {
        const auto found = prefix(input.substr(i), end);
        if (found && braces == 0) { finish = {i, i + found->size()}; break; }
        if (input[i] == u'{') ++braces;
        else if (input[i] == u'}') {
            if (!braces) throw Error(u"ExtraCloseMissingOpen", u"Extra close brace or missing open brace");
            --braces;
        }
    }
    if (!finish) return {};
    const auto [endBegin, endEnd] = *finish;
    const std::u16string first(input.substr(0, endInclude.text.empty() ? endBegin : endEnd));
    if (begin2Exclude.text.empty() && begin2Include.text.empty()) {
        Match result; result.text = first; result.remainder = input.substr(endEnd); return result;
    }
    const auto second = groups(input.substr(endEnd), begin2Exclude, begin2Include, end2Include, end2Exclude);
    if (!second) return {};
    Match result;
    result.remainder = second->remainder;
    result.array = !combine;
    if (combine) result.text = first + second->text;
    else result.captures = {first, second->text};
    return result;
}
std::optional<Match> group(std::u16string_view input, std::u16string_view beginExclude,
                          std::u16string_view beginInclude, std::u16string_view endInclude,
                          std::u16string_view endExclude) {
    return groups(input, literal(beginExclude), literal(beginInclude), literal(endInclude), literal(endExclude));
}
}
std::optional<Match> match(std::u16string_view name, std::u16string_view input) {
    const auto pattern = std::ranges::find(regexPatterns(), name, &RegexPattern::name);
    if (pattern != regexPatterns().end()) return regexMatch(input, pattern->source);
    if (name == u"(-)(9.,9)(e)(99)") {
        const auto found = regexMatch(input, uR"rx(^(\+\-|\+\/\-|\+|\-|\\pm\s?)?([0-9]+(?:[,.][0-9]+)?|[0-9]*(?:\.[0-9]+))?(\((?:[0-9]+(?:[,.][0-9]+)?|[0-9]*(?:\.[0-9]+))\))?(?:(?:([eE])|\s*(\*|x|\\times|\u00D7)\s*10\^)([+\-]?[0-9]+|\{[+\-]?[0-9]+\}))?)rx", true);
        return found && !found->text.empty() ? found : std::nullopt;
    }
    if (name == u"state of aggregation $") {
        const auto first = groups(input, literal(u""), expression(uR"rx(^\([a-z]{1,3}(?=[\),]))rx"), literal(u")"), literal(u""));
        if (first && regexMatch(first->remainder, uR"rx(^($|[\s,;\)\]\}]))rx")) return first;
        return regexMatch(input, uR"rx(^(?:\((?:\\ca\s?)?\$[amothc]\$\)))rx");
    }
    if (name == u"^{(...)}") return group(input, u"^{", u"", u"", u"}");
    if (name == u"^($...$)") return group(input, u"^", u"$", u"$", u"");
    if (name == u"_{(...)}") return group(input, u"_{", u"", u"", u"}");
    if (name == u"_($...$)") return group(input, u"_", u"$", u"$", u"");
    if (name == u"{...}") return group(input, u"", u"{", u"}", u"");
    if (name == u"{(...)}") return group(input, u"{", u"", u"", u"}");
    if (name == u"$...$") return group(input, u"", u"$", u"$", u"");
    if (name == u"${(...)}$__$(...)$") {
        if (const auto first = group(input, u"${", u"", u"", u"}$")) return first;
        return group(input, u"$", u"", u"", u"$");
    }
    if (name == u"\\bond{(...)}") return group(input, u"\\bond{", u"", u"", u"}");
    if (name == u"[(...)]") return group(input, u"[", u"", u"", u"]");
    if (name == u"\\color{(...)}") return group(input, u"\\color{", u"", u"", u"}");
    if (name == u"\\ce{(...)}") return group(input, u"\\ce{", u"", u"", u"}");
    if (name == u"\\pu{(...)}") return group(input, u"\\pu{", u"", u"", u"}");
    if (name == u"^\\x{}{}" || name == u"_\\x{}{}" || name == u"\\x{}{}" ||
        name == u"^\\x{}" || name == u"_\\x{}" || name == u"\\x{}") {
        const bool two = name.ends_with(u"{}{}");
        const std::u16string start = name.front() == u'^' ? u"^" : name.front() == u'_' ? u"_" : u"";
        return groups(input, literal(start), expression(uR"rx(^\\[a-zA-Z]+\{)rx"), literal(u"}"), literal(u""),
                      literal(u""), literal(two ? u"{" : u""), literal(two ? u"}" : u""), literal(u""), two);
    }
    if (name == u"\\frac{(...)}" || name == u"\\overset{(...)}" || name == u"\\underset{(...)}" || name == u"\\underbrace{(...)}") {
        const auto open = name.substr(0, name.find(u'{') + 1);
        return groups(input, literal(open), literal(u""), literal(u""), literal(name.starts_with(u"\\underbrace") ? u"}_" : u"}"),
                      literal(u"{"), literal(u""), literal(u""), literal(u"}"));
    }
    if (name == u"\\color{(...)}{(...)}") {
        if (const auto first = groups(input, literal(u"\\color{"), {}, {}, literal(u"}"), literal(u"{"), {}, {}, literal(u"}"))) return first;
        return groups(input, literal(u"\\color"), literal(u"\\"), {}, expression(uR"rx(^(?=\{))rx"), literal(u"{"), {}, {}, literal(u"}"));
    }
    if (name == u"amount" || name == u"amount2") {
        const auto first = regexMatch(input, uR"rx(^(?:(?:(?:\([+\-]?[0-9]+\/[0-9]+\)|[+\-]?(?:[0-9]+|\$[a-z]\$|[a-z])\/[0-9]+|[+\-]?[0-9]+[.,][0-9]+|[+\-]?\.[0-9]+|[+\-]?[0-9]+)(?:[a-z](?=\s*[A-Z]))?)|[+\-]?[a-z](?=\s*[A-Z])|\+(?!\s)))rx");
        if (first) return first;
        const auto dollars = group(input, u"", u"$", u"$", u"");
        if (dollars && regexMatch(dollars->text, uR"rx(^\$(?:\(?[+\-]?(?:[0-9]*[a-z]?[+\-])?[0-9]*[a-z](?:[+\-][0-9]*[a-z]?)?\)?|\+|-)\$$)rx")) return dollars;
        return {};
    }
    if (name == u"formula$") {
        if (regexMatch(input, uR"rx(^\([a-z]+\)$)rx")) return {};
        return regexMatch(input, uR"rx(^(?:[a-z]|(?:[0-9\ \+\-\,\.\(\)]+[a-z])+[0-9\ \+\-\,\.\(\)]*|(?:[a-z][0-9\ \+\-\,\.\(\)]+)+[a-z]?)$)rx");
    }
    throw Error(u"MhchemBugP", u"mhchem bug P. Please report. (" + std::u16string(name) + u")");
}
}
