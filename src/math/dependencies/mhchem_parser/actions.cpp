// Native port of mhchemparser 4.2.1 src/mhchemParser.ts: buffer actions.
// Copyright (c) 2015-2023 Martin Hensel. Apache-2.0.
module folia.math.dependencies.mhchem_parser;
import std;
namespace folia::math::mhchem {
namespace {
void append(Output& target, Output source) {
    target.insert(target.end(), std::make_move_iterator(source.begin()), std::make_move_iterator(source.end()));
}
std::u16string optionText(const Option& option) {
    if (const auto* text = std::get_if<std::u16string>(&option)) return *text;
    return {};
}
bool truthy(const Option& option) {
    return std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::monostate>) return false;
        else if constexpr (std::is_same_v<T, std::u16string>) return !value.empty();
        else return static_cast<bool>(value);
    }, option);
}
Parsed stringNode(std::u16string type, std::u16string text) {
    auto result = Parsed::node(std::move(type)); result.p1 = std::move(text); return result;
}
Output outputCe(Buffer& buffer, const Option& entityFollows) {
    Output result;
    if (buffer.r.empty()) {
        if (!buffer.a.empty() || !buffer.b.empty() || !buffer.p.empty() || !buffer.o.empty() ||
            !buffer.q.empty() || !buffer.d.empty() || truthy(entityFollows)) {
            if (buffer.sb) result.push_back(Parsed::node(u"entitySkip"));
            const bool two = std::holds_alternative<double>(entityFollows) && std::get<double>(entityFollows) == 2;
            if (buffer.o.empty() && buffer.q.empty() && buffer.d.empty() && buffer.b.empty() && buffer.p.empty() && !two) {
                buffer.o = std::move(buffer.a); buffer.a.clear();
            } else if (buffer.o.empty() && buffer.q.empty() && buffer.d.empty() && (!buffer.b.empty() || !buffer.p.empty())) {
                buffer.o = std::move(buffer.a); buffer.d = std::move(buffer.b); buffer.q = std::move(buffer.p);
                buffer.a.clear(); buffer.b.clear(); buffer.p.clear();
            } else {
                if (!buffer.o.empty() && buffer.dType == u"kv" && match(u"d-oxidation$", buffer.d)) buffer.dType = u"oxidation";
                else if (!buffer.o.empty() && buffer.dType == u"kv" && buffer.q.empty()) buffer.dType.clear();
            }
            auto node = Parsed::node(u"chemfive");
            node.a = parse(buffer.a, u"a"); node.b = parse(buffer.b, u"bd");
            node.p = parse(buffer.p, u"pq"); node.o = parse(buffer.o, u"o");
            node.q = parse(buffer.q, u"pq");
            node.d = parse(buffer.d, buffer.dType == u"oxidation" ? u"oxidation" : u"bd");
            node.dType = buffer.dType;
            result.push_back(std::move(node));
        }
    } else {
        auto node = Parsed::node(u"arrow"); node.r = buffer.r;
        node.rd = buffer.rdt == u"M" ? parse(buffer.rd, u"tex-math") : buffer.rdt == u"T"
            ? Output{stringNode(u"text", buffer.rd)} : parse(buffer.rd, u"ce");
        node.rq = buffer.rqt == u"M" ? parse(buffer.rq, u"tex-math") : buffer.rqt == u"T"
            ? Output{stringNode(u"text", buffer.rq)} : parse(buffer.rq, u"ce");
        result.push_back(std::move(node));
    }
    const auto level = buffer.parenthesisLevel;
    const auto bond = buffer.beginsWithBond;
    buffer = {};
    buffer.parenthesisLevel = level;
    buffer.beginsWithBond = bond;
    return result;
}
std::u16string temperature(std::u16string value) {
    value = replaceRegex(std::move(value), u"\u00B0C|\\^oC|\\^\\{o\\}C", u"{}^{\\circ}C", true);
    return replaceRegex(std::move(value), u"\u00B0F|\\^oF|\\^\\{o\\}F", u"{}^{\\circ}F", true);
}
}
Output action(std::u16string_view machine, std::u16string_view name,
              Buffer& buffer, const Match& matched, const Option& option) {
    const auto& m = matched.text;
    const auto parameter = optionText(option);
    if (machine == u"ce") {
        if (name == u"output") return outputCe(buffer, option);
        if (name == u"o after d") {
            Output result;
            if (regexMatch(buffer.d, u"^[1-9][0-9]*$")) {
                const auto temporary = buffer.d; buffer.d.clear();
                result = outputCe(buffer, {});
                result.push_back(Parsed::node(u"tinySkip")); buffer.b = temporary;
            } else result = outputCe(buffer, {});
            buffer.o += m; return result;
        }
        if (name == u"d= kv") { buffer.d = m; buffer.dType = u"kv"; return {}; }
        if (name == u"charge or bond") {
            if (!buffer.beginsWithBond) { buffer.d = m; return {}; }
            auto result = outputCe(buffer, {});
            auto bond = Parsed::node(u"bond"); bond.kind = u"-";
            result.push_back(std::move(bond)); return result;
        }
        if (name == u"- after o/d") {
            auto c1 = match(u"orbital", buffer.o);
            const auto c2 = match(u"one lowercase greek letter $", buffer.o);
            const auto c3 = match(u"one lowercase latin letter $", buffer.o);
            const auto c4 = match(u"$one lowercase latin letter$ $", buffer.o);
            const bool hyphenFollows = m == u"-" && ((c1 && c1->remainder.empty()) || c2 || c3 || c4);
            if (hyphenFollows && buffer.a.empty() && buffer.b.empty() && buffer.p.empty() && buffer.d.empty() && buffer.q.empty() && !c1 && c3)
                buffer.o = u"$" + buffer.o + u"$";
            Output result;
            if (hyphenFollows) {
                result = outputCe(buffer, {}); result.push_back(Parsed::node(u"hyphen"));
            } else {
                c1 = match(u"digits", buffer.d);
                if (truthy(option) && c1 && c1->remainder.empty()) {
                    buffer.d += m; result = outputCe(buffer, {});
                } else {
                    result = outputCe(buffer, {});
                    auto bond = Parsed::node(u"bond"); bond.kind = u"-"; result.push_back(std::move(bond));
                }
            }
            return result;
        }
        if (name == u"a to o") { buffer.o = std::move(buffer.a); buffer.a.clear(); return {}; }
        if (name == u"sb=true") { buffer.sb = true; return {}; }
        if (name == u"sb=false") { buffer.sb = false; return {}; }
        if (name == u"beginsWithBond=true") { buffer.beginsWithBond = true; return {}; }
        if (name == u"beginsWithBond=false") { buffer.beginsWithBond = false; return {}; }
        if (name == u"parenthesisLevel++") { ++buffer.parenthesisLevel; return {}; }
        if (name == u"parenthesisLevel--") { --buffer.parenthesisLevel; return {}; }
        if (name == u"state of aggregation") {
            auto node = Parsed::node(u"state of aggregation"); node.p1Nodes = parse(m, u"o"); return {std::move(node)};
        }
        if (name == u"comma") {
            const auto trimmed = replaceRegex(m, u"\\s*$", u"");
            return {stringNode(trimmed != m && buffer.parenthesisLevel == 0 ? u"comma enumeration L" : u"comma enumeration M", trimmed)};
        }
        if (name == u"oxidation-output") {
            Output result{Parsed::literal(u"{")}; append(result, parse(m, u"oxidation")); result.push_back(Parsed::literal(u"}")); return result;
        }
        if (name == u"frac-output" || name == u"overset-output" || name == u"underset-output" || name == u"underbrace-output") {
            auto node = Parsed::node(name == u"frac-output" ? u"frac-ce" : std::u16string(name.substr(0, name.size() - 7)));
            node.p1Nodes = parse(matched.at(0), u"ce"); node.p2Nodes = parse(matched.at(1), u"ce"); return {std::move(node)};
        }
        if (name == u"r=") { buffer.r = m; return {}; }
        if (name == u"rdt=") { buffer.rdt = m; return {}; }
        if (name == u"rd=") { buffer.rd = m; return {}; }
        if (name == u"rqt=") { buffer.rqt = m; return {}; }
        if (name == u"rq=") { buffer.rq = m; return {}; }
    }
    if (machine == u"pq" && name == u"state of aggregation") {
        auto node = Parsed::node(u"state of aggregation subscript"); node.p1Nodes = parse(m, u"o"); return {std::move(node)};
    }
    if ((machine == u"ce" || machine == u"pq" || machine == u"bd") && name == u"color-output") {
        auto node = Parsed::node(u"color"); node.color1 = matched.at(0);
        node.color2 = parse(matched.at(1), machine); return {std::move(node)};
    }
    if (machine == u"text" && name == u"output") {
        if (buffer.text.empty()) return {};
        auto node = stringNode(u"text", buffer.text); buffer = {}; return {std::move(node)};
    }
    if (machine == u"oxidation" && name == u"roman-numeral") return {stringNode(u"roman numeral", buffer.o)};
    if (machine == u"tex-math tight" && name == u"tight operator") { buffer.o += u"{" + m + u"}"; return {}; }
    if ((machine == u"tex-math" || machine == u"tex-math tight") && name == u"output") {
        if (buffer.o.empty()) return {};
        auto node = stringNode(u"tex-math", buffer.o); buffer = {}; return {std::move(node)};
    }
    if ((machine == u"9,9" || machine == u"pu-9,9") && name == u"comma") return {Parsed::node(u"commaDecimal")};
    if (machine == u"pu") {
        if (name == u"enumber" || name == u"number^") {
            Output result;
            const auto sign = matched.at(0);
            if (sign == u"+-" || sign == u"+/-") result.push_back(Parsed::literal(u"\\pm "));
            else if (!sign.empty()) result.push_back(Parsed::literal(sign));
            if (name == u"number^") {
                append(result, parse(matched.at(1), u"pu-9,9"));
                result.push_back(Parsed::literal(u"^{" + matched.at(2) + u"}"));
            } else {
                if (!matched.at(1).empty()) {
                    append(result, parse(matched.at(1), u"pu-9,9"));
                    if (!matched.at(2).empty()) {
                        if (regexMatch(matched.at(2), u"[,.]")) append(result, parse(matched.at(2), u"pu-9,9"));
                        else result.push_back(Parsed::literal(matched.at(2)));
                    }
                    if (!matched.at(3).empty() || !matched.at(4).empty())
                        result.push_back(Parsed::node(matched.at(3) == u"e" || matched.at(4) == u"*" ? u"cdot" : u"times"));
                }
                if (!matched.at(5).empty()) result.push_back(Parsed::literal(u"10^{" + matched.at(5) + u"}"));
            }
            return result;
        }
        if (name == u"space") return {Parsed::node(u"pu-space-1")};
        if (name == u"output") {
            const auto md = match(u"{(...)}", buffer.d);
            if (md && md->remainder.empty()) buffer.d = md->text;
            const auto mq = match(u"{(...)}", buffer.q);
            if (mq && mq->remainder.empty()) buffer.q = mq->text;
            if (!buffer.d.empty()) buffer.d = temperature(buffer.d);
            Output result;
            if (!buffer.q.empty()) {
                buffer.q = temperature(buffer.q);
                auto numerator = parse(buffer.d, u"pu"), denominator = parse(buffer.q, u"pu");
                if (buffer.o == u"//") {
                    auto node = Parsed::node(u"pu-frac"); node.p1Nodes = std::move(numerator); node.p2Nodes = std::move(denominator);
                    result.push_back(std::move(node));
                } else {
                    const bool spacing = numerator.size() > 1 || denominator.size() > 1;
                    result = std::move(numerator); result.push_back(Parsed::node(spacing ? u" / " : u"/"));
                    append(result, std::move(denominator));
                }
            } else result = parse(buffer.d, u"pu-2");
            buffer = {}; return result;
        }
    }
    if (machine == u"pu-2") {
        if (name == u"cdot") return {Parsed::node(u"tight cdot")};
        if (name == u"^(-1)") { buffer.rm += u"^{" + m + u"}"; return {}; }
        if (name == u"space") return {Parsed::node(u"pu-space-2")};
        if (name == u"output") {
            Output result;
            if (!buffer.rm.empty()) {
                const auto found = match(u"{(...)}", buffer.rm);
                if (found && found->remainder.empty()) result = parse(found->text, u"pu");
                else result.push_back(stringNode(u"rm", buffer.rm));
            }
            buffer = {}; return result;
        }
    }
    if (machine == u"pu-9,9" && (name == u"output-0" || name == u"output-o")) {
        Output result;
        const auto& text = buffer.text;
        if (text.size() <= 4) result.push_back(Parsed::literal(text));
        else if (name == u"output-0") {
            auto initial = text.size() % 3; if (!initial) initial = 3;
            for (std::size_t i = text.size() - 3; i > 0; i -= 3) {
                result.push_back(Parsed::literal(text.substr(i, 3))); result.push_back(Parsed::node(u"1000 separator"));
                if (i < 3) break;
            }
            result.push_back(Parsed::literal(text.substr(0, initial))); std::ranges::reverse(result);
        } else {
            const auto last = text.size() - 3;
            std::size_t i = 0;
            for (; i < last; i += 3) {
                result.push_back(Parsed::literal(text.substr(i, 3))); result.push_back(Parsed::node(u"1000 separator"));
            }
            result.push_back(Parsed::literal(text.substr(i)));
        }
        buffer = {}; return result;
    }
    // Shared actions are selected only after the current machine's actions.
    if (name == u"a=") { buffer.a += m; return {}; }
    if (name == u"b=") { buffer.b += m; return {}; }
    if (name == u"p=") { buffer.p += m; return {}; }
    if (name == u"o=") { buffer.o += m; return {}; }
    if (name == u"o=+p1") { buffer.o += parameter; return {}; }
    if (name == u"q=") { buffer.q += m; return {}; }
    if (name == u"d=") { buffer.d += m; return {}; }
    if (name == u"rm=") { buffer.rm += m; return {}; }
    if (name == u"text=") { buffer.text += m; return {}; }
    if (name == u"insert") return {Parsed::node(parameter)};
    if (name == u"insert+p1") return {stringNode(parameter, m)};
    if (name == u"insert+p1+p2") { auto node = stringNode(parameter, matched.at(0)); node.p2 = matched.at(1); return {std::move(node)}; }
    if (name == u"copy") return m.empty() ? Output{} : Output{Parsed::literal(m)};
    if (name == u"write") return parameter.empty() ? Output{} : Output{Parsed::literal(parameter)};
    if (name == u"rm") return {stringNode(u"rm", m)};
    if (name == u"text" || name == u"tex-math" || name == u"tex-math tight" || name == u"ce" || name == u"pu" || name == u"9,9") return parse(m, name);
    if (name == u"bond" || name == u"operator") {
        auto node = Parsed::node(std::u16string(name)); node.kind = parameter.empty() ? m : parameter; return {std::move(node)};
    }
    if (name == u"color0-output") { auto node = Parsed::node(u"color0"); node.color = m; return {std::move(node)}; }
    if (name == u"1/2") {
        Output result;
        std::u16string fraction = m;
        if (regexMatch(fraction, u"^[+\\-]")) { result.push_back(Parsed::literal(fraction.substr(0, 1))); fraction.erase(0, 1); }
        const auto parts = regexMatch(fraction, uR"rx(^([0-9]+|\$[a-z]\$|[a-z])\/([0-9]+)(\$[a-z]\$|[a-z])?$)rx", true);
        if (!parts) throw Error(u"MhchemBugA", u"mhchem bug A. Please report. (1/2)");
        auto node = stringNode(u"frac", replaceRegex(parts->at(0), u"\\$", u"", true)); node.p2 = parts->at(1);
        result.push_back(std::move(node));
        if (!parts->at(2).empty()) result.push_back(stringNode(u"tex-math", replaceRegex(parts->at(2), u"\\$", u"", true)));
        return result;
    }
    throw Error(u"MhchemBugA", u"mhchem bug A. Please report. (" + std::u16string(name) + u")");
}
}
