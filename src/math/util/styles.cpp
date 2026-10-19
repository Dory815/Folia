// MathJax 4.1.3 util/Styles.ts; Apache-2.0.
module folia.math.util.styles;
import folia.math.util.property_host;
import folia.math.util.js_number;
import std;

namespace folia::math::styles {
std::vector<std::string> TRBL{"top", "right", "bottom", "left"};
std::vector<std::string> WSC{"width", "style", "color"};

struct StylesAccess {
    static void setStyle(Styles& style, std::string_view name, std::u16string_view value) {
        style.setStyle(name, value);
    }
    static void setRaw(Styles& style, std::string_view name, std::u16string_view value) {
        style.styles_.set(name, Property(property_host::debugText(value)));
    }
    static Property raw(const Styles& style, std::string_view name) {
        return style.styles_.getOwn(name);
    }
    static void erase(Styles& style, std::string_view name) { style.styles_.erase(name); }
    static std::string childName(const Styles& style, std::string_view name, std::string_view child) {
        return style.childName(name, child);
    }
    static void combineChildren(Styles& style, std::string_view name) { style.combineChildren(name); }
    static void combineParent(Styles& style, std::string_view name) { style.combineParent(name); }
};

namespace {
using regexp_host::RegExp;
using regexp_host::RegExpRef;

RegExpRef regex(std::u16string pattern, std::u16string flags = {}) {
    return std::make_shared<RegExp>(std::move(pattern), std::move(flags));
}

bool matches(const RegExpRef& expression, std::u16string_view input) {
    if (!expression) return false;
    RegExp test(expression->pattern(), expression->flags());
    return test.exec(std::u16string(input)).has_value();
}

bool lineTerminator(char16_t c) {
    return c == u'\n' || c == u'\r' || c == 0x2028 || c == 0x2029;
}

bool jsWhitespace(char16_t c) {
    return c == u'\t' || c == u'\n' || c == u'\v' || c == u'\f' || c == u'\r' ||
        c == u' ' || c == 0x00A0 || c == 0x1680 || (c >= 0x2000 && c <= 0x200A) ||
        c == 0x2028 || c == 0x2029 || c == 0x202F || c == 0x205F ||
        c == 0x3000 || c == 0xFEFF;
}

std::u16string withoutEscapedCharacters(std::u16string_view input) {
    std::u16string result;
    for (std::size_t i = 0; i < input.size();) {
        if (input[i] == u'\\' && i + 1 < input.size() && !lineTerminator(input[i + 1]))
            i += 2;
        else
            result.push_back(input[i++]);
    }
    return result;
}

std::u16string withoutQuotedStrings(std::u16string_view input) {
    std::u16string result;
    for (std::size_t i = 0; i < input.size();) {
        if (input[i] == u'\'' || input[i] == u'"') {
            const auto quote = input[i];
            auto end = i + 1;
            while (end < input.size() && !lineTerminator(input[end]) && input[end] != quote) ++end;
            if (end < input.size() && input[end] == quote) {
                i = end + 1;
                continue;
            }
        }
        result.push_back(input[i++]);
    }
    return result;
}

std::vector<std::u16string> splitSpaces(std::u16string_view text) {
    // Reproduce String.split's capture array for the Source expression, then
    // apply its shift/push loop. In particular, unmatched whitespace gaps and
    // empty capture slots are observable in the resulting parts.
    static const auto splitter = regex(
        uR"STY(((?:'[^'\n]*'|"[^"\n]*"|,[\s\n]|[^\s\n])*))STY", u"g");
    if (text.empty()) return {};
    // String.prototype.split clones a global separator as sticky and advances
    // its search cursor itself after an empty match. RegExp.exec deliberately
    // leaves lastIndex at the end of an empty match, so keep that split-only
    // advancement here rather than changing the shared RegExp primitive.
    RegExp scan(splitter->pattern(), u"y");
    const std::u16string input(text);
    std::vector<std::u16string> split;
    const std::size_t length = input.size();
    std::size_t p = 0;
    std::size_t q = 0;
    while (q < length) {
        scan.lastIndex = static_cast<double>(q);
        const auto match = scan.exec(input);
        if (!match) {
            ++q; // AdvanceStringIndex with non-Unicode semantics.
            continue;
        }
        const auto e = static_cast<std::size_t>(std::get<double>(scan.lastIndex.value.data));
        if (e == p) {
            ++q; // Empty separator at p must not emit a split element.
            continue;
        }
        split.emplace_back(input.substr(p, q - p));
        split.push_back(match->at(1).value_or(std::u16string{}));
        p = e;
        q = p;
    }
    split.emplace_back(input.substr(p));
    std::vector<std::u16string> result;
    while (split.size() > 1) {
        split.erase(split.begin());
        result.push_back(std::move(split.front()));
        split.erase(split.begin());
    }
    return result;
}

std::vector<std::u16string> splitTRBL(std::u16string_view value) {
    auto parts = splitSpaces(value);
    if (parts.empty()) parts.emplace_back();
    if (parts.size() == 1) parts.push_back(parts[0]);
    if (parts.size() == 2) parts.push_back(parts[0]);
    if (parts.size() == 3) parts.push_back(parts[1]);
    return parts;
}

std::u16string join(const std::vector<std::u16string>& parts, std::u16string_view separator) {
    std::u16string result;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i) result.append(separator);
        result.append(parts[i]);
    }
    return result;
}

const StylesConnection* connection(std::string_view name) {
    const auto found = std::find_if(Styles::connect.begin(), Styles::connect.end(),
        [name](const auto& item) { return item.first == name; });
    return found == Styles::connect.end() ? nullptr : &found->second;
}

void splitTRBLStyle(Styles& style, std::string_view name) {
    const auto* connect = connection(name);
    if (!connect) return;
    auto parts = splitTRBL(property_host::string(StylesAccess::raw(style, name)));
    for (std::size_t i = 0; i < connect->children.size(); ++i) {
        const std::string child = StylesAccess::childName(style, name, connect->children[i]);
        StylesAccess::setStyle(style, child, parts[i]);
    }
}

void combineTRBLStyle(Styles& style, std::string_view name) {
    const auto* connect = connection(name);
    if (!connect) return;
    std::vector<std::u16string> parts;
    for (const auto& child : connect->children) {
        const Property value = StylesAccess::raw(style, StylesAccess::childName(style, name, child));
        if (!property_host::truthy(value)) {
            StylesAccess::erase(style, name);
            return;
        }
        parts.push_back(property_host::string(value));
    }
    if (parts.size() == 4 && parts[3] == parts[1]) {
        parts.pop_back();
        if (parts[2] == parts[0]) {
            parts.pop_back();
            if (parts[1] == parts[0]) parts.pop_back();
        }
    }
    StylesAccess::setRaw(style, name, join(parts, u" "));
}

void splitSameStyle(Styles& style, std::string_view name) {
    const auto* connect = connection(name);
    if (!connect) return;
    const auto value = property_host::string(StylesAccess::raw(style, name));
    for (const auto& child : connect->children)
        StylesAccess::setStyle(style, StylesAccess::childName(style, name, child), value);
}

void combineSameStyle(Styles& style, std::string_view name) {
    const auto* connect = connection(name);
    if (!connect || connect->children.empty()) return;
    const Property value = StylesAccess::raw(style, StylesAccess::childName(style, name, connect->children[0]));
    for (std::size_t i = 1; i < connect->children.size(); ++i) {
        if (StylesAccess::raw(style, StylesAccess::childName(style, name, connect->children[i])) != value) {
            StylesAccess::erase(style, name);
            return;
        }
    }
    if (property_host::truthy(value)) StylesAccess::setRaw(style, name, property_host::string(value));
}

const RegExpRef& borderPattern(std::string_view name) {
    static const auto width = regex(u"^(?:[\\d.]+(?:[a-z]+)|thin|medium|thick|inherit|initial|unset)$");
    static const auto style = regex(u"^(?:none|hidden|dotted|dashed|solid|double|groove|ridge|inset|outset|inherit|initial|unset)$");
    return name == "width" ? width : style;
}

void splitWSCStyle(Styles& style, std::string_view name) {
    const auto* connect = connection(name);
    if (!connect) return;
    std::map<std::string, std::u16string, std::less<>> values{{"width", {}}, {"style", {}}, {"color", {}}};
    for (const auto& part : splitSpaces(property_host::string(StylesAccess::raw(style, name)))) {
        const auto width = values["width"];
        const auto lineStyle = values["style"];
        if (matches(borderPattern("width"), part) && width.empty()) values["width"] = part;
        else if (matches(borderPattern("style"), part) && lineStyle.empty()) values["style"] = part;
        else values["color"] = part;
    }
    for (const auto& child : connect->children)
        StylesAccess::setStyle(style, StylesAccess::childName(style, name, child), values[child]);
}

void combineWSCStyle(Styles& style, std::string_view name) {
    const auto* connect = connection(name);
    if (!connect) return;
    std::vector<std::u16string> parts;
    for (const auto& child : connect->children) {
        const Property value = StylesAccess::raw(style, StylesAccess::childName(style, name, child));
        if (property_host::truthy(value)) parts.push_back(property_host::string(value));
    }
    if (parts.size() > 1) StylesAccess::setRaw(style, name, join(parts, u" "));
    else StylesAccess::erase(style, name);
}

bool fontMatch(std::string_view name, std::u16string_view part) {
    static const std::map<std::string, RegExpRef, std::less<>> patterns{
        {"style", regex(u"^(?:normal|italic|oblique|inherit|initial|unset)$")},
        {"variant", regex(u"^(?:normal|none|inherit|initial|unset|common-ligatures|no-common-ligatures|discretionary-ligatures|no-discretionary-ligatures|historical-ligatures|no-historical-ligatures|contextual|no-contextual|(?:stylistic|character-variant|swash|ornaments|annotation)\\([^)]*\\)|small-caps|all-small-caps|petite-caps|all-petite-caps|unicase|titling-caps|lining-nums|oldstyle-nums|proportional-nums|tabular-nums|diagonal-fractions|stacked-fractions|ordinal|slashed-zero|jis78|jis83|jis90|jis04|simplified|traditional|full-width|proportional-width|ruby)$")},
        {"weight", regex(u"^(?:normal|bold|bolder|lighter|[1-9]00|inherit|initial|unset)$")},
        {"stretch", regex(u"^(?:normal|(?:(?:ultra|extra|semi)-)?(?:condensed|expanded)|inherit|initial|unset)$")},
        {"size", regex(u"^(?:xx-small|x-small|small|medium|large|x-large|xx-large|larger|smaller|[\\d.]+%|[\\d.]+[a-z]+|inherit|initial|unset)(?:/(?:normal|[\\d.]+(?:%|[a-z]+)?))?$")},
    };
    const auto found = patterns.find(name);
    return found != patterns.end() && matches(found->second, part);
}

void splitFontStyle(Styles& style, std::string_view name) {
    struct Value { bool array = false; std::vector<std::u16string> list; std::u16string text; };
    std::map<std::string, Value, std::less<>> values;
    values["style"] = {};
    values["variant"].array = true;
    values["weight"] = {};
    values["stretch"] = {};
    values["size"] = {};
    values["family"] = {};
    values["line-height"] = {};
    const std::array<std::string_view, 5> patternNames{"style", "variant", "weight", "stretch", "size"};
    for (const auto& part : splitSpaces(property_host::string(StylesAccess::raw(style, name)))) {
        if (values["family"].text.empty()) values["family"].text = part;
        for (const auto field : patternNames) {
            auto& value = values[std::string(field)];
            if ((!value.array && !value.text.empty()) || !fontMatch(field, part)) continue;
            if (values["family"].text == part) values["family"].text.clear();
            if (field == "size") {
                const auto slash = part.find(u'/');
                value.text = part.substr(0, slash);
                if (slash != std::u16string::npos && slash + 1 < part.size())
                    values["line-height"].text = part.substr(slash + 1);
            } else if (values["size"].text.empty()) {
                if (value.array) value.list.push_back(part);
                else if (value.text.empty()) value.text = part;
            }
        }
    }
    const auto* connect = connection(name);
    if (!connect) return;
    for (const auto& child : connect->children) {
        const auto& value = values[child];
        const std::u16string text = value.array ? join(value.list, u" ") : value.text;
        if ((value.array && !value.list.empty()) || (!value.array && !text.empty()))
            StylesAccess::setRaw(style, StylesAccess::childName(style, name, child), text);
    }
    StylesAccess::erase(style, name);
}

void combineFontStyle(Styles&, std::string_view) {}

void combinePartStyle(Styles& style, std::string_view name) {
    combineTRBLStyle(style, name);
    StylesAccess::combineChildren(style, name);
    combineSameStyle(style, name);
    StylesAccess::combineParent(style, name);
}

void initializeTables();
}

StylesPatterns Styles::pattern{
    regex(uR"STY(['";])STY"),
    regex(uR"STY(^((:?'(?:\\.|[^'])*(?:'|$)|"(?:\\.|[^"])*(?:"|$)|\n|\\.|[^'";])*?)[\s\n]*(?:;|$).*)STY"),
    regex(uR"STY(([-a-z]+)[\s\n]*:[\s\n]*((?:'(?:\\.|[^'])*(?:'|$)|"(?:\\.|[^"])*(?:"|$)|\n|\\.|[^'";])*?)[\s\n]*(?:;|$))STY", u"g"),
    regex(uR"STY(/\*[^]*?\*/)STY", u"g")
};
StylesConnections Styles::connect;

namespace {
void initializeTables() {
    if (!Styles::connect.empty()) return;
    auto add = [](std::string name, std::vector<std::string> children,
                  std::string splitName, std::string combineName,
                  std::function<void(Styles&, std::string_view)> split,
                  std::function<void(Styles&, std::string_view)> combine,
                  std::vector<std::string> parts = {}, bool hasParts = false, bool subPart = false) {
        Styles::connect.emplace_back(std::move(name), StylesConnection{
            std::move(children), std::move(parts), hasParts, subPart,
            std::move(splitName), std::move(combineName), std::move(split), std::move(combine)});
    };
    add("padding", TRBL, "splitTRBL", "combineTRBL", splitTRBLStyle, combineTRBLStyle);
    add("margin", TRBL, "splitTRBL", "combineTRBL", splitTRBLStyle, combineTRBLStyle);
    add("border", TRBL, "splitSame", "combineSame", splitSameStyle, combineSameStyle, WSC, true);
    add("border-top", WSC, "splitWSC", "combineWSC", splitWSCStyle, combineWSCStyle);
    add("border-right", WSC, "splitWSC", "combineWSC", splitWSCStyle, combineWSCStyle);
    add("border-bottom", WSC, "splitWSC", "combineWSC", splitWSCStyle, combineWSCStyle);
    add("border-left", WSC, "splitWSC", "combineWSC", splitWSCStyle, combineWSCStyle);
    add("border-width", TRBL, "splitTRBL", "combinePart", splitTRBLStyle, combinePartStyle, {}, false, true);
    add("border-style", TRBL, "splitTRBL", "combinePart", splitTRBLStyle, combinePartStyle, {}, false, true);
    add("border-color", TRBL, "splitTRBL", "combinePart", splitTRBLStyle, combinePartStyle, {}, false, true);
    add("font", {"style", "variant", "weight", "stretch", "line-height", "size", "family"},
        "splitFont", "combineFont", splitFontStyle, combineFontStyle);
}
const bool tablesInitialized = [] { initializeTables(); return true; }();
}

Styles::Styles(std::u16string_view cssText) : Styles(cssText, pattern) {}
Styles::Styles(std::u16string_view cssText, const StylesPatterns& classPatterns) : patterns_(&classPatterns) {
    initializeTables();
    parse(cssText);
}

std::u16string Styles::sanitizeValue(std::u16string_view input) const {
    std::u16string text(input);
    const auto& patterns = patternTable();
    RegExp sanitize(patterns.sanitize->pattern(), patterns.sanitize->flags());
    if (!sanitize.exec(text)) return text;
    RegExp valuePattern(patterns.value->pattern(), patterns.value->flags());
    if (const auto match = valuePattern.exec(text)) {
        const auto capture = match->at(1);
        if (capture) text = *capture;
    }
    const auto withoutEscapes = withoutEscapedCharacters(text);
    const auto unquoted = withoutQuotedStrings(withoutEscapes);
    const auto firstQuote = std::find_if(unquoted.begin(), unquoted.end(),
        [](char16_t c) { return c == u'\'' || c == u'"'; });
    if (firstQuote != unquoted.end()) text.push_back(*firstQuote);
    return text;
}

std::string Styles::normalizeName(std::u16string_view name) const {
    std::u16string normalized;
    normalized.reserve(name.size() * 2);
    for (const char16_t c : name) {
        if (c >= u'A' && c <= u'Z') {
            normalized.push_back(u'-');
            normalized.push_back(static_cast<char16_t>(c + (u'a' - u'A')));
        } else {
            normalized.push_back(c);
        }
    }
    return property_host::debugText(normalized);
}

std::string Styles::parentName(std::string_view name) const {
    const auto split = name.rfind('-');
    return split == std::string_view::npos ? std::string{} : std::string(name.substr(0, split));
}

std::string Styles::childName(std::string_view name, std::string_view child) const {
    if (child.find('-') != std::string_view::npos) return std::string(child);
    std::string adjusted(child);
    if (const auto* connect = connection(name); connect && connect->subPart) {
        const auto split = name.rfind('-');
        adjusted += split == std::string_view::npos ? "-" : std::string(name.substr(split));
        const std::string parent = parentName(name);
        return parent + "-" + adjusted;
    }
    return std::string(name) + "-" + adjusted;
}

void Styles::setStyle(std::string_view name, std::u16string_view value) {
    styles_.set(name, Property(property_host::debugText(sanitizeValue(value))));
    if (const auto* connect = connection(name); connect && !connect->children.empty())
        connect->split(*this, name);
    if (value.empty()) styles_.erase(name);
}

void Styles::combineChildren(std::string_view name) {
    const std::string parent = parentName(name);
    const auto* connect = connection(name);
    if (!connect) property_host::typeError("Cannot read properties of undefined (reading 'children')");
    for (const auto& child : connect->children) {
        const std::string childKey = childName(parent, child);
        const auto* childConnect = connection(childKey);
        if (!childConnect) property_host::typeError("Cannot read properties of undefined (reading 'combine')");
        childConnect->combine(*this, childKey);
    }
}

void Styles::combineParent(std::string_view name) {
    std::string current(name);
    while (current.find('-') != std::string::npos) {
        const std::string child = current;
        current = parentName(current);
        const auto* childConnection = connection(child);
        const auto* parentConnection = connection(current);
        const auto suffix = child.substr(current.size() + 1);
        const bool parentHasChild = parentConnection &&
            std::find(parentConnection->children.begin(), parentConnection->children.end(), suffix) != parentConnection->children.end();
        if (!childConnection && !parentHasChild) break;
        if (!parentConnection || !parentConnection->combine)
            property_host::typeError("Cannot read properties of undefined (reading 'combine')");
        parentConnection->combine(*this, current);
    }
    if (!property_host::truthy(styles_.get(current))) return;
    const auto* connect = connection(current);
    if (connect && connect->hasParts)
        for (const auto& part : connect->parts) styles_.erase(childName(current, part));
}

void Styles::set(std::u16string_view name, StyleValue value) {
    const std::string normalized = normalizeName(name);
    const std::u16string text = std::visit([](const auto& item) -> std::u16string {
        using T = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<T, std::u16string>) return item;
        else if constexpr (std::is_same_v<T, bool>) return item ? u"true" : u"false";
        else return property_host::text(js_number::toString(item));
    }, value.value);
    setStyle(normalized, text);
    const auto* connect = connection(normalized);
    if (connect && connect->subPart) { connect->combine(*this, normalized); return; }
    combineParent(normalized);
    if (normalized.find('-', normalized.find('-') == std::string::npos ? 0 : normalized.find('-') + 1) != std::string::npos) {
        const auto split = normalized.find('-');
        const auto end = normalized.find('-', split + 1);
        const std::string parent = normalized.substr(0, split + 1) + normalized.substr(end + 1);
        combineSameStyle(*this, parent);
    }
}

std::u16string Styles::get(std::u16string_view name) const {
    const auto value = styles_.getOwn(normalizeName(name));
    return value.missing() ? std::u16string{} : property_host::string(value);
}

std::u16string Styles::cssText() const {
    std::vector<std::u16string> styles;
    for (const auto& name : styles_.keys()) {
        const std::string parent = parentName(name);
        const auto split = name.rfind('-');
        const std::string child = split == std::string::npos ? name : name.substr(split + 1);
        const std::string parentParent = parentName(parent);
        const std::string pname = childName(parentParent, child);
        const auto* connect = connection(parent);
        const bool knownChild = connect && std::find(connect->children.begin(), connect->children.end(), child) != connect->children.end();
        const Property value = styles_.get(name);
        if (property_host::truthy(value) && !property_host::truthy(styles_.get(pname)) &&
            (!property_host::truthy(styles_.get(parent)) || !knownChild))
            styles.push_back(property_host::text(name) + u": " + property_host::string(value) + u";");
    }
    return join(styles, u" ");
}

StyleList Styles::styleList() const {
    StyleList copy;
    copy.assign(styles_);
    return copy;
}

void Styles::parse(std::u16string_view cssText) {
    styles_ = StyleList{};
    std::u16string text(cssText);
    std::replace(text.begin(), text.end(), u'\n', u' ');
    const auto& comment = patternTable().comment;
    RegExp comments(comment->pattern(), comment->flags());
    std::u16string withoutComments;
    std::size_t cursor = 0;
    while (const auto match = comments.exec(text)) {
        const auto full = match->at(0);
        if (!full) break;
        withoutComments.append(text.substr(cursor, match->index - cursor));
        cursor = match->index + full->size();
        if (full->empty()) ++cursor;
    }
    withoutComments.append(text.substr(std::min(cursor, text.size())));

    const auto& sourcePattern = patternTable().style;
    RegExp style(sourcePattern->pattern(), sourcePattern->flags());
    std::size_t prior = 0;
    while (const auto match = style.exec(withoutComments)) {
        const auto full = match->at(0);
        if (!full) break;
        const std::u16string space = withoutComments.substr(prior, match->index - prior);
        const auto name = match->at(1);
        const auto value = match->at(2);
        if (std::any_of(space.begin(), space.end(),
            [](char16_t c) { return c != u';' && !jsWhitespace(c); })) return;
        if (!name || !value) property_host::typeError("Styles pattern did not provide its Source captures");
        set(property_host::text(property_host::debugText(*name)), StyleValue(*value));
        prior = match->index + full->size();
    }
    const std::u16string remainder = withoutComments.substr(std::min(prior, withoutComments.size()));
    (void)remainder; // Source split loop ignores a final unmatched suffix.
}
}
