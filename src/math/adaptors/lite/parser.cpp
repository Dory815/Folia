module folia.math.adaptors.lite_adaptor;
import :parser;
import folia.math.util.js_case;
import std;

namespace folia::math::adaptors::lite {
namespace {

// Keep these patterns in the original LiteParser owner. Their grammar is the
// pinned Source grammar, including its restricted whitespace and UTF-16 class.
std::u16string nameClass() {
    std::u16string result = u"[^";
    result.push_back(0);
    result += u"-";
    result.push_back(0x001F);
    result += u" \"'>/=";
    result.push_back(0x007F);
    result += u"-";
    result.push_back(0x009F);
    result.push_back(u']');
    return result;
}

std::u16string spacePattern() { return u"[ \\n]+"; }
std::u16string optionalSpacePattern() { return u"[ \\n]*"; }
std::u16string tagNamePattern() {
    return std::u16string(u"[A-Za-z]") + nameClass() + u"*";
}
std::u16string attributeNamePattern() { return nameClass() + u"+"; }
std::u16string valuePattern() {
    return u"(?:'[^']*'|\"[^\"]*\"|" + spacePattern() + u")";
}
std::u16string valueSplitPattern() {
    return u"(?:'([^']*)'|\"([^\"]*)\"|(" + spacePattern() + u"))";
}
std::u16string attributePattern() {
    return attributeNamePattern() + u"(?:" + optionalSpacePattern() + u"=" +
           optionalSpacePattern() + valuePattern() + u")?";
}
std::u16string attrSplitPattern() {
    return u"(" + attributeNamePattern() + u")(?:" + optionalSpacePattern() +
           u"=" + optionalSpacePattern() + valueSplitPattern() + u")?";
}
std::u16string tagPattern() {
    const auto space = spacePattern();
    const auto optional = optionalSpacePattern();
    const auto tag = tagNamePattern();
    const auto attribute = attributePattern();
    return u"(<(?:" + tag + u"(?:" + space + attribute + u")*" + optional +
           u"/?|/" + tag + u"|!--[^]*?--|![^]*?)(?:>|$))";
}

std::vector<std::optional<std::u16string>> splitCaptures(
    std::u16string_view input, std::u16string_view pattern) {
    return regexp_host::split(input, std::u16string(pattern), u"u");
}

std::deque<std::optional<std::u16string>> splitTags(std::u16string_view input) {
    // Source removes processing instructions before splitting tags.
    const std::u16string text(input);
    regexp_host::RegExp pi(u"<\\?.*?\\?>", u"g");
    std::u16string withoutPI;
    std::size_t copied = 0;
    while (const auto match = pi.exec(text)) {
        const auto whole = match->at(0);
        if (!whole) break;
        withoutPI.append(text, copied, match->index - copied);
        copied = match->index + whole->size();
    }
    withoutPI.append(text, copied, std::u16string::npos);

    const auto split = splitCaptures(withoutPI, tagPattern());
    return {split.begin(), split.end()};
}

void replaceAll(std::u16string& text, std::u16string_view from,
                std::u16string_view to) {
    std::size_t at = 0;
    while ((at = text.find(from, at)) != std::u16string::npos) {
        text.replace(at, from.size(), to);
        at += to.size();
    }
}

std::u16string lower(std::u16string_view value) {
    return js_case::lowerCase(value);
}

std::u16string joinPresent(std::span<std::optional<std::u16string> const> parts) {
    std::u16string result;
    for (const auto& part : parts)
        if (part) result += *part;
    return result;
}

LiteElementRef asElement(DOMNodeRef const& node) {
    return std::dynamic_pointer_cast<LiteElement>(node);
}

} // namespace

LiteParser::LiteParser(std::shared_ptr<entities::EntityModule> entities)
    : entities_(entities ? std::move(entities)
                         : std::make_shared<entities::EntityModule>()) {}

bool LiteParser::selfClosingTag(std::u16string_view kind) noexcept {
    constexpr std::array<std::u16string_view, 17> tags{
        u"area", u"base", u"br", u"col", u"command", u"embed", u"hr",
        u"img", u"input", u"keygen", u"link", u"menuitem", u"meta",
        u"param", u"source", u"track", u"wbr"};
    return std::ranges::find(tags, kind) != tags.end();
}

bool LiteParser::pcdataTag(std::u16string_view kind) noexcept {
    constexpr std::array<std::u16string_view, 6> tags{
        u"option", u"textarea", u"fieldset", u"title", u"style", u"script"};
    return std::ranges::find(tags, kind) != tags.end();
}

std::optional<std::u16string_view>
LiteParser::namespaceFor(std::u16string_view kind) noexcept {
    if (kind == u"svg") return u"http://www.w3.org/2000/svg";
    if (kind == u"math") return u"http://www.w3.org/1998/Math/MathML";
    if (kind == u"html") return u"http://www.w3.org/1999/xhtml";
    return {};
}

LiteTextRef LiteParser::addText(LiteBase& adaptor, LiteElementRef const& node,
                                std::u16string_view text) {
    auto result = adaptor.text(entities_->translate(text));
    auto typed = std::dynamic_pointer_cast<LiteText>(result);
    if (!typed) throw std::logic_error("Lite adaptor text() returned a non-text node");
    adaptor.append(node, std::move(result));
    return typed;
}

LiteCommentRef LiteParser::addComment(LiteBase& adaptor,
                                      LiteElementRef const& node,
                                      std::u16string_view comment) {
    auto result = adaptor.comment(comment);
    auto typed = std::dynamic_pointer_cast<LiteComment>(result);
    if (!typed) throw std::logic_error("Lite adaptor comment() returned a non-comment node");
    adaptor.append(node, std::move(result));
    return typed;
}

LiteElementRef LiteParser::closeTag(LiteBase& adaptor, LiteElementRef node,
                                    std::u16string_view tag) {
    const auto kind = lower(tag.substr(2, tag.size() >= 3 ? tag.size() - 3 : 0));
    while (node) {
        const auto parent = asElement(adaptor.parent(node));
        if (!parent || adaptor.kind(node) == kind) break;
        node = parent;
    }
    return asElement(adaptor.parent(node));
}

void LiteParser::addAttributes(
    LiteBase& adaptor, LiteElementRef const& node,
    std::span<std::optional<std::u16string> const> attributes) {
    for (std::size_t i = 0; i < attributes.size();) {
        const auto nameIndex = i + 1;
        const auto v1 = i + 2;
        const auto v2 = i + 3;
        const auto v3 = i + 4;
        if (nameIndex >= attributes.size() || !attributes[nameIndex])
            throw std::invalid_argument("malformed Lite attribute capture sequence");
        std::u16string_view raw;
        if (v1 < attributes.size() && attributes[v1] && !attributes[v1]->empty())
            raw = *attributes[v1];
        else if (v2 < attributes.size() && attributes[v2] && !attributes[v2]->empty())
            raw = *attributes[v2];
        else if (v3 < attributes.size() && attributes[v3] && !attributes[v3]->empty())
            raw = *attributes[v3];
        adaptor.setAttribute(node, *attributes[nameIndex], entities_->translate(raw));
        i += 5;
    }
}

LiteElementRef LiteParser::openTag(
    LiteBase& adaptor, LiteElementRef node, std::u16string_view tag,
    std::deque<std::optional<std::u16string>>& parts) {
    regexp_host::RegExp kindPattern(u"<(.*?)[\\s\\n>/]");
    const auto match = kindPattern.exec(std::u16string(tag));
    if (!match || match->captures.size() < 2 || !match->captures[1])
        throw std::invalid_argument("invalid Lite opening tag");
    const auto kind = lower(*match->captures[1]);
    auto child = std::dynamic_pointer_cast<LiteElement>(adaptor.node(kind));
    if (!child) throw std::logic_error("Lite adaptor node() returned a non-element node");

    regexp_host::RegExp attributeStart(u"^<.*?[\\s\\n>]");
    const auto start = attributeStart.exec(std::u16string(tag));
    const auto prefix = start && start->at(0) ? start->at(0)->size() : std::size_t{0};
    const auto attributeText = tag.substr(prefix);
    auto captures = splitCaptures(attributeText, attrSplitPattern());

    // String.split includes the unmatched tail as the final item. Source pops
    // that item and treats tags with fewer than one complete capture group as
    // complete even when that final token is not a closing angle bracket.
    std::optional<std::u16string> tail;
    if (!captures.empty()) {
        tail = std::move(captures.back());
        captures.pop_back();
    }
    const bool complete = tail && regexp_host::RegExp(u">$").exec(*tail).has_value();
    if (complete || captures.size() < 5) {
        addAttributes(adaptor, child, captures);
        adaptor.append(node, child);
        if (!selfClosingTag(kind) && !(tag.size() >= 2 && tag.substr(tag.size() - 2) == u"/>")) {
            if (pcdataTag(kind)) handlePCDATA(adaptor, child, kind, parts);
            else node = child;
        }
    }
    return node;
}

void LiteParser::handlePCDATA(
    LiteBase& adaptor, LiteElementRef const& node, std::u16string_view kind,
    std::deque<std::optional<std::u16string>>& parts) {
    const std::u16string endTag = u"</" + std::u16string(kind) + u">";
    std::u16string previousTag;
    std::vector<std::optional<std::u16string>> content;
    while (!parts.empty() && previousTag != endTag) {
        content.emplace_back(previousTag);
        content.push_back(std::move(parts.front()));
        parts.pop_front();
        if (parts.empty()) previousTag.clear();
        else {
            auto next = std::move(parts.front());
            parts.pop_front();
            previousTag = next ? std::move(*next) : std::u16string{};
        }
    }
    auto text = joinPresent(content);
    adaptor.append(node, adaptor.text(text));
}

LiteElementRef LiteParser::getOnlyChild(LiteBase& adaptor,
                                         LiteElementRef const& body) const {
    LiteElementRef result;
    for (const auto& child : adaptor.childNodes(body)) {
        auto element = std::dynamic_pointer_cast<LiteElement>(child);
        if (!element) continue;
        if (result) return {};
        result = std::move(element);
    }
    return result;
}

void LiteParser::checkDocument(LiteBase& adaptor,
                               LiteDocumentRef const& document) {
    if (!document) throw std::invalid_argument("Lite parser requires a document");
    const auto body = document->body();
    const auto node = getOnlyChild(adaptor, body);
    if (!node) return;

    for (const auto& child : adaptor.childNodes(body)) {
        if (child == node) break;
        if (auto comment = std::dynamic_pointer_cast<LiteComment>(child);
            comment && comment->value().starts_with(u"<!DOCTYPE"))
            document->type(comment->value());
    }

    const auto kind = adaptor.kind(node);
    if (kind == u"html") {
        for (const auto& child : node->children()) {
            if (auto element = asElement(child)) {
                if (adaptor.kind(element) == u"head") document->head_ = element;
                else if (adaptor.kind(element) == u"body") document->body_ = element;
            }
        }
        document->root_ = node;
        adaptor.remove(node);
        if (adaptor.parent(document->body()) != node)
            adaptor.append(node, document->body());
        if (adaptor.parent(document->head()) != node)
            adaptor.insert(document->head(), document->body());
    } else if (kind == u"head") {
        document->head_ = asElement(adaptor.replace(node, document->head()));
    } else if (kind == u"body") {
        document->body_ = asElement(adaptor.replace(node, document->body()));
    }
}

LiteDocumentRef LiteParser::parseFromString(std::u16string_view input,
                                           std::u16string_view,
                                           LiteBase& adaptor) {
    auto document = adaptor.createDocument();
    auto node = document->body();
    auto parts = splitTags(input);
    while (!parts.empty()) {
        auto text = std::move(parts.front());
        parts.pop_front();
        std::optional<std::u16string> tag;
        if (!parts.empty()) {
            tag = std::move(parts.front());
            parts.pop_front();
        }
        if (text && !text->empty()) addText(adaptor, node, *text);
        if (tag && !tag->empty() && tag->back() == u'>') {
            if (tag->size() > 1 && (*tag)[1] == u'!') {
                addComment(adaptor, node, *tag);
            } else if (tag->size() > 1 && (*tag)[1] == u'/') {
                node = closeTag(adaptor, node, *tag);
            } else {
                node = openTag(adaptor, node, *tag, parts);
            }
        }
    }
    checkDocument(adaptor, document);
    return document;
}

folia::math::DOMAttributes LiteParser::allAttributes(
    LiteBase& adaptor, LiteElementRef const& node, bool xml) const {
    auto attributes = adaptor.allAttributes(node);
    if (!xml) return attributes;
    const auto kind = adaptor.kind(node);
    const auto nameSpace = namespaceFor(kind);
    if (!nameSpace) return attributes;
    if (std::ranges::any_of(attributes, [](const auto& attribute) {
            return attribute.name == u"xmlns";
        })) return attributes;
    attributes.push_back({u"xmlns", std::u16string(*nameSpace)});
    return attributes;
}

std::u16string LiteParser::serialize(LiteBase& adaptor,
                                     LiteElementRef const& node,
                                     bool xml) const {
    const auto tag = std::u16string(adaptor.kind(node));
    const auto attributes = allAttributes(adaptor, node, xml);
    std::u16string attributeText;
    for (const auto& attribute : attributes) {
        if (!attributeText.empty()) attributeText.push_back(u' ');
        attributeText += attribute.name;
        attributeText += u"=\"";
        attributeText += protectAttribute(attribute.value, xml);
        attributeText.push_back(u'\"');
    }
    const auto content = serializeInner(adaptor, node, xml);
    std::u16string result = u"<" + tag;
    if (!attributeText.empty()) result += u" " + attributeText;
    if ((!xml || !content.empty()) && !selfClosingTag(tag))
        result += u">" + content + u"</" + tag + u">";
    else if (xml)
        result += u"/>";
    else
        result += u">";
    return result;
}

std::u16string LiteParser::serializeInner(LiteBase& adaptor,
                                          LiteElementRef const& node,
                                          bool xml) const {
    std::u16string result;
    const auto children = adaptor.childNodes(node);
    if (pcdataTag(adaptor.kind(node))) {
        for (const auto& child : children) result += adaptor.value(child);
        return result;
    }
    for (const auto& child : children) {
        const auto kind = adaptor.kind(child);
        if (kind == u"#text") {
            result += protectHTML(adaptor.value(child));
        } else if (kind == u"#comment") {
            if (auto comment = std::dynamic_pointer_cast<LiteComment>(child))
                result += comment->value();
        } else if (auto element = asElement(child)) {
            result += serialize(adaptor, element, xml);
        }
    }
    return result;
}

std::u16string LiteParser::protectHTML(std::u16string_view input) const {
    std::u16string text(input);
    replaceAll(text, u"&", u"&amp;");
    replaceAll(text, u"<", u"&lt;");
    replaceAll(text, u">", u"&gt;");
    return text;
}

std::u16string LiteParser::protectAttribute(std::u16string_view input,
                                           bool xml) const {
    std::u16string text(input);
    replaceAll(text, u"&", u"&amp;");
    replaceAll(text, u"\"", u"&quot;");
    if (xml) {
        replaceAll(text, u"<", u"&lt;");
        replaceAll(text, u">", u"&gt;");
    }
    return text;
}

} // namespace folia::math::adaptors::lite
