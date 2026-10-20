module folia.math.adaptors.lite_adaptor;
import :document;
import :element;
import :list;
import :parser;
import :text;
import :window;
import folia.math.util.js_numeric_input;
import folia.math.util.styles;
import std;

namespace folia::math::adaptors::lite {
namespace {

LiteElementRef element(DOMNodeRef const &node) {
    auto result = std::dynamic_pointer_cast<LiteElement>(node);
    if (!result)
        throw std::invalid_argument("LiteAdaptor operation requires an element");
    return result;
}

LiteTextRef textNode(DOMNodeRef const &node) {
    auto result = std::dynamic_pointer_cast<LiteText>(node);
    if (!result)
        throw std::invalid_argument("LiteAdaptor operation requires a text node");
    return result;
}

LiteDocumentRef documentNode(DOMDocumentRef const &document) {
    auto result = std::dynamic_pointer_cast<LiteDocument>(document);
    if (!result)
        throw std::invalid_argument("LiteAdaptor operation requires a document");
    return result;
}

std::u16string trimEcma(std::u16string_view text) {
    std::size_t first = 0;
    std::size_t last = text.size();
    while (first < last && js_input::isWhitespace(text[first]))
        ++first;
    while (last > first && js_input::isWhitespace(text[last - 1]))
        --last;
    return std::u16string(text.substr(first, last - first));
}

std::vector<std::u16string> splitSingleSpaces(std::u16string_view text) {
    std::vector<std::u16string> result;
    std::size_t start = 0;
    for (;;) {
        const auto end = text.find(u' ', start);
        if (end == std::u16string_view::npos) {
            result.emplace_back(text.substr(start));
            return result;
        }
        result.emplace_back(text.substr(start, end - start));
        start = end + 1;
    }
}

std::vector<std::u16string> classSearchWords(std::u16string_view text) {
    const auto trimmed = trimEcma(text);
    std::vector<std::u16string> result;
    std::size_t start = 0;
    while (start < trimmed.size()) {
        while (start < trimmed.size() && trimmed[start] == u' ')
            ++start;
        if (start == trimmed.size())
            break;
        const auto end = trimmed.find(u' ', start);
        result.emplace_back(trimmed.substr(start, end == std::u16string::npos
                                                      ? end
                                                      : end - start));
        if (end == std::u16string::npos)
            break;
        start = end + 1;
    }
    if (result.empty())
        result.emplace_back();
    return result;
}

std::u16string joinWithSpace(std::span<std::u16string const> values) {
    std::u16string result;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i)
            result.push_back(u' ');
        result += values[i];
    }
    return result;
}

void appendScalar(std::u16string &out, std::uint32_t value) {
    if (value <= 0xFFFF) {
        out.push_back(static_cast<char16_t>(value));
        return;
    }
    value -= 0x10000;
    out.push_back(static_cast<char16_t>(0xD800 + (value >> 10)));
    out.push_back(static_cast<char16_t>(0xDC00 + (value & 0x3FF)));
}

std::u16string nodeMixinCjkPattern() {
    std::u16string pattern = u"[";
    constexpr std::array<std::pair<std::uint32_t, std::uint32_t>, 16> ranges{{
        {0x1100, 0x115F}, {0x2329, 0x232A}, {0x2E80, 0x303E}, {0x3040, 0x3247},
        {0x3250, 0x4DBF}, {0x4E00, 0xA4C6}, {0xA960, 0xA97C}, {0xAC00, 0xD7A3},
        {0xF900, 0xFAFF}, {0xFE10, 0xFE19}, {0xFE30, 0xFE6B}, {0xFF01, 0xFF60},
        {0xFFE0, 0xFFE6}, {0x1B000, 0x1B001}, {0x1F200, 0x1F251}, {0x20000, 0x3FFFD},
    }};
    for (const auto &[first, last] : ranges) {
        appendScalar(pattern, first);
        if (first != last) {
            pattern.push_back(u'-');
            appendScalar(pattern, last);
        }
    }
    pattern.push_back(u']');
    return pattern;
}

std::u16string replaceCjkMatches(regexp_host::RegExp &expression,
                                 std::u16string_view input) {
    const std::u16string text(input);
    const bool global = expression.flags().find(u'g') != std::u16string::npos;
    expression.last_index = 0;
    std::size_t copied = 0;
    std::u16string output;
    while (const auto match = expression.exec(text)) {
        const auto whole = match->at(0);
        if (!whole)
            break;
        const auto end = match->index + whole->size();
        if (match->index > text.size() || end > text.size())
            throw std::logic_error("CJK regexp returned an invalid match range");
        output.append(text, copied, match->index - copied);
        copied = end;
        if (!global)
            break;
        if (whole->empty()) {
            auto next = match->index;
            const bool unicode = expression.flags().find(u'u') != std::u16string::npos ||
                                 expression.flags().find(u'v') != std::u16string::npos;
            if (next < text.size()) {
                if (unicode && text[next] >= 0xD800 && text[next] <= 0xDBFF &&
                    next + 1 < text.size() && text[next + 1] >= 0xDC00 &&
                    text[next + 1] <= 0xDFFF)
                    next += 2;
                else
                    ++next;
            } else {
                ++next;
            }
            expression.last_index = next;
        }
    }
    output.append(text, copied, std::u16string::npos);
    if (global)
        expression.last_index = 0;
    return output;
}

std::size_t codePointLength(std::u16string_view text) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] >= 0xD800 && text[i] <= 0xDBFF && i + 1 < text.size() &&
            text[i + 1] >= 0xDC00 && text[i + 1] <= 0xDFFF)
            ++i;
        ++count;
    }
    return count;
}

std::u16string jsSlice(std::u16string_view text, double start, double end) {
    const auto length = static_cast<std::int64_t>(text.size());
    const auto relativeIndex = [length](double value) -> std::int64_t {
        if (std::isnan(value) || value == 0)
            return 0;
        if (std::isinf(value))
            return value < 0 ? -length : length;
        value = std::trunc(value);
        if (value <= -static_cast<double>(length))
            return 0;
        if (value < 0)
            return length + static_cast<std::int64_t>(value);
        if (value >= static_cast<double>(length))
            return length;
        return static_cast<std::int64_t>(value);
    };
    auto first = std::clamp<std::int64_t>(relativeIndex(start), 0, length);
    auto last = std::clamp<std::int64_t>(relativeIndex(end), 0, length);
    if (last < first)
        last = first;
    return std::u16string(text.substr(static_cast<std::size_t>(first),
                                      static_cast<std::size_t>(last - first)));
}

std::optional<std::pair<std::u16string, std::u16string>>
parseAttributeSelector(std::u16string_view selector) {
    if (selector.size() < 5 || selector.front() != u'[' || selector.back() != u']')
        return std::nullopt;
    const auto equals = selector.find(u"=\"");
    if (equals == std::u16string_view::npos)
        return std::nullopt;
    const auto closing = selector.find(u"\"]", equals + 2);
    if (closing != selector.size() - 2)
        return std::nullopt;
    const auto hasLineTerminator = [](std::u16string_view text) {
        return std::any_of(text.begin(), text.end(), [](char16_t ch) {
            return ch == u'\n' || ch == u'\r' || ch == 0x2028 || ch == 0x2029;
        });
    };
    if (hasLineTerminator(selector.substr(1, equals - 1)) ||
        hasLineTerminator(selector.substr(equals + 2, closing - equals - 2)))
        return std::nullopt;
    return std::pair{std::u16string(selector.substr(1, equals - 1)),
                     std::u16string(selector.substr(equals + 2, closing - equals - 2))};
}

bool tagName(std::u16string_view name) {
    const auto alpha = [](char16_t ch) {
        return (ch >= u'a' && ch <= u'z') || (ch >= u'A' && ch <= u'Z');
    };
    if (name.empty() || (!alpha(name.front()) && name.front() != u'-'))
        return false;
    for (const auto ch : name)
        if (!alpha(ch) && !(ch >= u'0' && ch <= u'9') && ch != u'-')
            return false;
    return true;
}

const std::u16string *attribute(DOMAttributes const &attributes, std::u16string_view name) {
    const auto it = std::find_if(attributes.begin(), attributes.end(), [&](auto const &entry) {
        return entry.name == name;
    });
    return it == attributes.end() ? nullptr : &it->value;
}

std::optional<std::uint32_t> arrayIndex(std::u16string_view name) {
    if (name.empty() || (name.size() > 1 && name.front() == u'0'))
        return std::nullopt;
    std::uint64_t value = 0;
    for (const auto ch : name) {
        if (ch < u'0' || ch > u'9')
            return std::nullopt;
        value = value * 10 + static_cast<unsigned>(ch - u'0');
        if (value > 0xFFFFFFFEu)
            return std::nullopt;
    }
    return static_cast<std::uint32_t>(value);
}

bool containsNode(DOMNodeRef const &root, DOMNodeRef const &target) {
    if (!root || !target)
        return false;
    std::vector<DOMNodeRef> pending{root};
    std::unordered_set<DOMNode const *> visited;
    while (!pending.empty()) {
        auto current = std::move(pending.back());
        pending.pop_back();
        if (current == target)
            return true;
        if (!visited.insert(current.get()).second)
            continue;
        if (auto currentElement = std::dynamic_pointer_cast<LiteElement>(current))
            for (const auto &child : currentElement->children())
                pending.push_back(child);
    }
    return false;
}

std::vector<LiteElementRef> search(
    DOMNodeRef const &root, std::function<bool(LiteElementRef const &)> const &predicate,
    std::optional<std::size_t> stop = {}) {
    std::vector<LiteElementRef> result;
    std::vector<DOMNodeRef> stack{root};
    while (!stack.empty()) {
        auto current = std::move(stack.front());
        stack.erase(stack.begin());
        auto currentElement = std::dynamic_pointer_cast<LiteElement>(current);
        if (!currentElement)
            continue;
        if (predicate(currentElement)) {
            result.push_back(currentElement);
            if (stop && result.size() == *stop)
                return result;
        }
        const auto &children = currentElement->children();
        stack.insert(stack.begin(), children.begin(), children.end());
    }
    return result;
}

std::size_t insertionIndex(std::ptrdiff_t index, std::size_t length) {
    auto start = index;
    if (start < 0)
        start = std::max<std::ptrdiff_t>(0, static_cast<std::ptrdiff_t>(length) + start);
    return std::min(static_cast<std::size_t>(start), length);
}

void validateAcyclicReparent(DOMNodeRef const &child, DOMNodeRef const &parent) {
    if (!child || !parent)
        throw std::invalid_argument("LiteAdaptor reparent requires non-null nodes");
    if (containsNode(child, parent))
        throw std::invalid_argument("LiteAdaptor reparent would create a child cycle");
}

} // namespace

LiteBase::LiteBase(LiteWindowRef window, std::shared_ptr<entities::EntityModule> entities)
    : window_(window ? std::move(window) : LiteWindow::create()),
      parser_(std::make_shared<LiteParser>(std::move(entities))) {
    canMeasureNodes = false;
}

void LiteBase::window(LiteWindowRef value) { window_ = std::move(value); }

DOMNodeRef LiteBase::create(std::u16string_view kind, std::u16string_view) {
    return LiteElement::create(std::u16string(kind));
}

DOMDocumentRef LiteBase::parse(std::u16string_view text, std::u16string_view format) {
    return parser_->parseFromString(text, format, *this);
}

DOMNodeRef LiteBase::text(std::u16string_view text) {
    return LiteText::create(std::u16string(text));
}

DOMNodeRef LiteBase::comment(std::u16string_view text) {
    return LiteComment::create(std::u16string(text));
}

LiteDocumentRef LiteBase::createDocument() { return LiteDocument::create(); }

DOMNodeRef LiteBase::head(DOMDocumentRef doc) const {
    if (!doc)
        doc = document;
    return documentNode(doc)->head();
}

DOMNodeRef LiteBase::body(DOMDocumentRef doc) const {
    if (!doc)
        doc = document;
    return documentNode(doc)->body();
}

DOMNodeRef LiteBase::root(DOMDocumentRef doc) const {
    if (!doc)
        doc = document;
    return documentNode(doc)->root();
}

std::u16string LiteBase::doctype(DOMDocumentRef doc) const {
    if (!doc)
        doc = document;
    return documentNode(doc)->type();
}

std::vector<DOMNodeRef> LiteBase::tags(DOMNodeRef const &node, std::u16string_view name,
                                      std::u16string_view ns) const {
    return tags(node, name, ns, {});
}

std::vector<DOMNodeRef> LiteBase::tags(DOMNodeRef const &node, std::u16string_view name,
                                      std::u16string_view ns,
                                      std::optional<std::size_t> stop) const {
    if (!ns.empty())
        return {};
    auto found = search(node, [&](LiteElementRef const &candidate) {
        return candidate->kind() == name;
    }, stop);
    return std::vector<DOMNodeRef>(found.begin(), found.end());
}

LiteElementRef LiteBase::elementById(DOMNodeRef const &node, std::u16string_view id) const {
    auto found = search(node, [&](LiteElementRef const &candidate) {
        const auto *value = attribute(candidate->attributes(), u"id");
        return value && *value == id;
    }, 1);
    return found.empty() ? LiteElementRef{} : found.front();
}

std::vector<LiteElementRef>
LiteBase::elementsByClass(DOMNodeRef const &node, std::u16string_view name,
                          std::optional<std::size_t> stop) const {
    return search(node, [&](LiteElementRef const &candidate) {
        const auto *value = attribute(candidate->attributes(), u"class");
        const auto words = classSearchWords(value ? std::u16string_view(*value)
                                                  : std::u16string_view{});
        return std::find(words.begin(), words.end(), name) != words.end();
    }, stop);
}

std::vector<LiteElementRef>
LiteBase::elementsByAttribute(DOMNodeRef const &node, std::u16string_view name,
                              std::u16string_view value,
                              std::optional<std::size_t> stop) const {
    return search(node, [&](LiteElementRef const &candidate) {
        const auto *found = attribute(candidate->attributes(), name);
        return found && *found == value;
    }, stop);
}

std::vector<DOMNodeRef> LiteBase::getElements(std::span<LiteSearchInput const> selectors,
                                              DOMDocumentRef const &doc) const {
    const auto searchRoot = body(doc);
    std::vector<DOMNodeRef> result;
    for (const auto &selector : selectors) {
        if (const auto *textSelector = std::get_if<std::u16string>(&selector)) {
            const auto &value = *textSelector;
            if (value.empty())
                continue;
            if (value.front() == u'#') {
                auto node = elementById(searchRoot, std::u16string_view(value).substr(1));
                if (node)
                    result.push_back(std::move(node));
            } else if (value.front() == u'.') {
                auto nodes = elementsByClass(searchRoot, std::u16string_view(value).substr(1));
                result.insert(result.end(), nodes.begin(), nodes.end());
            } else if (tagName(value)) {
                auto nodes = tags(searchRoot, value);
                result.insert(result.end(), nodes.begin(), nodes.end());
            } else if (auto parsed = parseAttributeSelector(value)) {
                auto nodes = elementsByAttribute(searchRoot, parsed->first, parsed->second);
                result.insert(result.end(), nodes.begin(), nodes.end());
            }
        } else if (const auto *nodes = std::get_if<std::vector<DOMNodeRef>>(&selector)) {
            result.insert(result.end(), nodes->begin(), nodes->end());
        } else if (const auto *node = std::get_if<DOMNodeRef>(&selector)) {
            result.push_back(*node);
        }
    }
    return result;
}

DOMNodeRef LiteBase::getElement(std::u16string_view selector, DOMDocumentRef doc) const {
    if (!doc)
        doc = document;
    const auto searchRoot = body(doc);
    if (selector.starts_with(u"#"))
        return elementById(searchRoot, selector.substr(1));
    if (selector.starts_with(u".")) {
        auto found = elementsByClass(searchRoot, selector.substr(1), 1);
        return found.empty() ? DOMNodeRef{} : found.front();
    }
    if (tagName(selector)) {
        auto found = tags(searchRoot, selector, {}, 1);
        return found.empty() ? DOMNodeRef{} : found.front();
    }
    if (auto parsed = parseAttributeSelector(selector)) {
        auto found = elementsByAttribute(searchRoot, parsed->first, parsed->second, 1);
        return found.empty() ? DOMNodeRef{} : found.front();
    }
    return {};
}

DOMNodeRef LiteBase::getElement(std::u16string_view selector,
                                DOMNodeRef const &node) const {
    if (!node)
        throw std::invalid_argument("LiteAdaptor getElement requires a node");
    const auto rootElement = element(node);
    if (selector.starts_with(u"#"))
        return elementById(rootElement, selector.substr(1));
    if (selector.starts_with(u".")) {
        auto found = elementsByClass(rootElement, selector.substr(1), 1);
        return found.empty() ? DOMNodeRef{} : found.front();
    }
    if (tagName(selector)) {
        auto found = tags(rootElement, selector, {}, 1);
        return found.empty() ? DOMNodeRef{} : found.front();
    }
    if (auto parsed = parseAttributeSelector(selector)) {
        auto found = elementsByAttribute(rootElement, parsed->first, parsed->second, 1);
        return found.empty() ? DOMNodeRef{} : found.front();
    }
    return {};
}

bool LiteBase::contains(DOMNodeRef const &container, DOMNodeRef const &node) const {
    auto current = node;
    while (current && current != container)
        current = current->parent();
    return current && current == container;
}

std::ptrdiff_t LiteBase::childIndex(DOMNodeRef const &node) const {
    if (!node)
        return -1;
    const auto parent = node->parent();
    const auto parentElement = std::dynamic_pointer_cast<LiteElement>(parent);
    if (!parentElement)
        return -1;
    const auto &children = parentElement->children();
    const auto found = std::find(children.begin(), children.end(), node);
    return found == children.end() ? -1 : std::distance(children.begin(), found);
}

DOMNodeRef LiteBase::append(DOMNodeRef const &parent, DOMNodeRef child) {
    auto parentElement = element(parent);
    validateAcyclicReparent(child, parent);
    auto oldParent = child->parent();
    if (auto oldElement = std::dynamic_pointer_cast<LiteElement>(oldParent)) {
        auto &oldChildren = oldElement->children_;
        const auto old = std::find(oldChildren.begin(), oldChildren.end(), child);
        if (old != oldChildren.end())
            oldChildren.erase(old);
    }
    child->setParent({});
    parentElement->children_.push_back(child);
    child->setParent(parentElement);
    return child;
}

DOMNodeRef LiteBase::replace(DOMNodeRef replacement, DOMNodeRef const &oldNode) {
    if (!replacement || !oldNode)
        throw std::invalid_argument("LiteAdaptor replace requires non-null nodes");
    const auto oldParent = oldNode->parent();
    const auto parentElement = std::dynamic_pointer_cast<LiteElement>(oldParent);
    if (!parentElement)
        return oldNode;
    auto &children = parentElement->children_;
    const auto found = std::find(children.begin(), children.end(), oldNode);
    if (found == children.end())
        return oldNode;
    validateAcyclicReparent(replacement, parentElement);
    *found = replacement;
    replacement->setParent(parentElement);
    oldNode->setParent({});
    return oldNode;
}

void LiteBase::insert(DOMNodeRef child, DOMNodeRef const &sibling) {
    if (!child)
        throw std::invalid_argument("LiteAdaptor insert requires a child");
    if (sibling) {
        const auto siblingParent = sibling->parent();
        if (siblingParent && child != sibling)
            validateAcyclicReparent(child, siblingParent);
    }
    auto oldParent = child->parent();
    if (auto oldElement = std::dynamic_pointer_cast<LiteElement>(oldParent)) {
        auto &oldChildren = oldElement->children_;
        const auto old = std::find(oldChildren.begin(), oldChildren.end(), child);
        if (old != oldChildren.end())
            oldChildren.erase(old);
    }
    child->setParent({});
    if (!sibling)
        return;
    const auto parent = sibling->parent();
    const auto parentElement = std::dynamic_pointer_cast<LiteElement>(parent);
    if (!parentElement)
        return;
    const auto index = childIndex(sibling);
    auto &children = parentElement->children_;
    const auto at = insertionIndex(index, children.size());
    children.insert(children.begin() + static_cast<std::ptrdiff_t>(at), child);
    child->setParent(parentElement);
}

DOMNodeRef LiteBase::remove(DOMNodeRef const &child) {
    if (!child)
        throw std::invalid_argument("LiteAdaptor remove requires a child");
    auto oldParent = child->parent();
    if (auto oldElement = std::dynamic_pointer_cast<LiteElement>(oldParent)) {
        auto &oldChildren = oldElement->children_;
        const auto old = std::find(oldChildren.begin(), oldChildren.end(), child);
        if (old != oldChildren.end())
            oldChildren.erase(old);
    }
    child->setParent({});
    return child;
}

DOMNodeRef LiteBase::clone(DOMNodeRef const &node, bool deep) const {
    const auto source = element(node);
    DOMAttributes attributes = source->attributes();
    std::vector<DOMNodeRef> children;
    if (deep) {
        children.reserve(source->children().size());
        for (const auto &child : source->children()) {
            if (const auto text = std::dynamic_pointer_cast<LiteText>(child)) {
                if (std::dynamic_pointer_cast<LiteComment>(child))
                    children.push_back(LiteComment::create(text->value()));
                else
                    children.push_back(LiteText::create(text->value()));
            } else {
                auto copy = clone(child, true);
                children.push_back(std::move(copy));
            }
        }
    }
    auto result = LiteElement::create(std::u16string(source->kind()), std::move(attributes),
                                      std::move(children));
    // Source assigns parents only for recursively cloned elements. Text and
    // comment child copies deliberately remain parentless.
    for (const auto &child : result->children_)
        if (std::dynamic_pointer_cast<LiteText>(child))
            child->setParent({});
    return result;
}

DOMNodeRef LiteBase::split(DOMNodeRef const &node, double index) {
    auto source = textNode(node);
    auto suffix = LiteText::create(jsSlice(source->value(), index,
                                           static_cast<double>(source->value().size())));
    source->value(jsSlice(source->value(), 0, index));
    const auto parent = source->parent();
    const auto parentElement = std::dynamic_pointer_cast<LiteElement>(parent);
    if (!parentElement)
        throw std::invalid_argument("LiteAdaptor split requires a parent element");
    const auto at = childIndex(source) + 1;
    auto &children = parentElement->children_;
    const auto insertion = insertionIndex(at, children.size());
    children.insert(children.begin() + static_cast<std::ptrdiff_t>(insertion), suffix);
    suffix->setParent(parentElement);
    return suffix;
}

DOMNodeRef LiteBase::next(DOMNodeRef const &node) const {
    if (!node)
        return {};
    const auto parent = node->parent();
    const auto parentElement = std::dynamic_pointer_cast<LiteElement>(parent);
    if (!parentElement)
        return {};
    const auto index = childIndex(node) + 1;
    const auto &children = parentElement->children();
    return index < 0 || static_cast<std::size_t>(index) >= children.size()
               ? DOMNodeRef{}
               : children[static_cast<std::size_t>(index)];
}

DOMNodeRef LiteBase::previous(DOMNodeRef const &node) const {
    if (!node)
        return {};
    const auto parent = node->parent();
    const auto parentElement = std::dynamic_pointer_cast<LiteElement>(parent);
    if (!parentElement)
        return {};
    const auto index = childIndex(node) - 1;
    const auto &children = parentElement->children();
    return index < 0 || static_cast<std::size_t>(index) >= children.size()
               ? DOMNodeRef{}
               : children[static_cast<std::size_t>(index)];
}

DOMNodeRef LiteBase::firstChild(DOMNodeRef const &node) const {
    const auto source = element(node);
    return source->children().empty() ? DOMNodeRef{} : source->children().front();
}

DOMNodeRef LiteBase::lastChild(DOMNodeRef const &node) const {
    const auto source = element(node);
    return source->children().empty() ? DOMNodeRef{} : source->children().back();
}

std::vector<DOMNodeRef> LiteBase::childNodes(DOMNodeRef const &node) const {
    return element(node)->children();
}

std::u16string LiteBase::value(DOMNodeRef const &node) const {
    if (auto text = std::dynamic_pointer_cast<LiteText>(node)) {
        if (!std::dynamic_pointer_cast<LiteComment>(node))
            return text->value();
        regexp_host::RegExp comment(u"^<!(--)?((?:.|\\n)*)\\1>$");
        const auto match = comment.exec(text->value());
        if (match)
            if (const auto capture = match->at(2))
                return *capture;
        return text->value();
    }
    return {};
}

std::u16string LiteBase::textContent(DOMNodeRef const &node) const {
    const auto source = element(node);
    std::u16string result;
    for (const auto &child : source->children()) {
        if (const auto text = std::dynamic_pointer_cast<LiteText>(child)) {
            if (!std::dynamic_pointer_cast<LiteComment>(child))
                result += text->value();
        } else {
            result += textContent(child);
        }
    }
    return result;
}

std::u16string LiteBase::innerHTML(DOMNodeRef const &node) const {
    return parser_->serializeInner(*const_cast<LiteBase *>(this), element(node));
}

std::u16string LiteBase::outerHTML(DOMNodeRef const &node) const {
    return parser_->serialize(*const_cast<LiteBase *>(this), element(node));
}

std::u16string LiteBase::serializeXML(DOMNodeRef const &node) const {
    return parser_->serialize(*const_cast<LiteBase *>(this), element(node), true);
}

void LiteBase::setAttribute(DOMNodeRef const &node, std::u16string_view name,
                            std::u16string_view value, std::u16string_view ns) {
    auto target = element(node);
    std::u16string key(name);
    if (!ns.empty()) {
        const auto slash = ns.find_last_of(u'/');
        const auto prefix = ns.substr(slash == std::u16string_view::npos ? 0 : slash + 1);
        const auto colon = key.find_last_of(u':');
        key = std::u16string(prefix) + u":" +
              key.substr(colon == std::u16string::npos ? 0 : colon + 1);
    }
    auto &attributes = target->attributes_;
    const auto found = std::find_if(attributes.begin(), attributes.end(), [&](auto const &entry) {
        return entry.name == key;
    });
    const bool isStyle = key == u"style";
    if (found == attributes.end())
        attributes.push_back({std::move(key), std::u16string(value)});
    else
        found->value = value;
    if (isStyle)
        target->styles_.reset();
}

void LiteBase::removeAttribute(DOMNodeRef const &node, std::u16string_view name) {
    auto &attributes = element(node)->attributes_;
    const auto found = std::find_if(attributes.begin(), attributes.end(), [&](auto const &entry) {
        return entry.name == name;
    });
    if (found != attributes.end())
        attributes.erase(found);
}

std::optional<std::u16string> LiteBase::getAttribute(DOMNodeRef const &node,
                                                      std::u16string_view name) const {
    const auto *value = attribute(element(node)->attributes(), name);
    return value ? std::optional<std::u16string>(*value) : std::nullopt;
}

bool LiteBase::hasAttribute(DOMNodeRef const &node, std::u16string_view name) const {
    return attribute(element(node)->attributes(), name) != nullptr;
}

DOMAttributes LiteBase::allAttributes(DOMNodeRef const &node) const {
    auto attributes = element(node)->attributes();
    std::stable_sort(attributes.begin(), attributes.end(), [](auto const &left, auto const &right) {
        const auto leftIndex = arrayIndex(left.name);
        const auto rightIndex = arrayIndex(right.name);
        if (leftIndex && rightIndex)
            return *leftIndex < *rightIndex;
        return leftIndex.has_value() && !rightIndex.has_value();
    });
    return attributes;
}

void LiteBase::addClass(DOMNodeRef const &node, std::u16string_view name) {
    auto target = element(node);
    const auto *classValue = attribute(target->attributes(), u"class");
    auto classes = classValue ? splitSingleSpaces(*classValue) : std::vector<std::u16string>{};
    if (std::find(classes.begin(), classes.end(), name) == classes.end()) {
        classes.emplace_back(name);
        setAttribute(node, u"class", joinWithSpace(classes));
    }
}

void LiteBase::removeClass(DOMNodeRef const &node, std::u16string_view name) {
    auto target = element(node);
    const auto *classValue = attribute(target->attributes(), u"class");
    if (!classValue)
        return;
    auto classes = splitSingleSpaces(*classValue);
    const auto found = std::find(classes.begin(), classes.end(), name);
    if (found != classes.end()) {
        classes.erase(found);
        setAttribute(node, u"class", joinWithSpace(classes));
    }
}

bool LiteBase::hasClass(DOMNodeRef const &node, std::u16string_view name) const {
    const auto *classValue = attribute(element(node)->attributes(), u"class");
    const auto classes = splitSingleSpaces(classValue ? std::u16string_view(*classValue)
                                                      : std::u16string_view{});
    return std::find(classes.begin(), classes.end(), name) != classes.end();
}

void LiteBase::setStyle(DOMNodeRef const &node, std::u16string_view name,
                        std::u16string_view value) {
    auto target = element(node);
    if (!target->styles_) {
        const auto *initial = attribute(target->attributes(), u"style");
        target->styles_ = std::make_shared<styles::Styles>(
            initial ? std::u16string_view(*initial) : std::u16string_view{});
    }
    target->styles_->set(name, styles::StyleValue(std::u16string(value)));
    const auto cssText = target->styles_->cssText();
    auto &attributes = target->attributes_;
    const auto found = std::find_if(attributes.begin(), attributes.end(), [](auto const &entry) {
        return entry.name == u"style";
    });
    if (found == attributes.end())
        attributes.push_back({u"style", cssText});
    else
        found->value = cssText;
}

std::u16string LiteBase::getStyle(DOMNodeRef const &node, std::u16string_view name) {
    auto target = element(node);
    if (!target->styles_) {
        const auto *initial = attribute(target->attributes(), u"style");
        if (!initial || initial->empty())
            return {};
        target->styles_ = std::make_shared<styles::Styles>(*initial);
    }
    return target->styles_->get(name);
}

std::optional<std::u16string> LiteBase::allStyles(DOMNodeRef const &node) const {
    return getAttribute(node, u"style");
}

void LiteBase::insertRules(DOMNodeRef const &node, std::span<std::u16string const> rules) {
    auto target = element(node);
    auto content = textContent(target) + u"\n\n";
    for (std::size_t i = 0; i < rules.size(); ++i) {
        if (i)
            content += u"\n\n";
        content += rules[i];
    }
    auto text = LiteText::create(std::move(content));
    target->children_ = {std::move(text)};
}

double LiteBase::fontSize(DOMNodeRef const &) const { return 0; }

std::u16string LiteBase::fontFamily(DOMNodeRef const &) const { return {}; }

std::pair<double, double> LiteBase::nodeSize(DOMNodeRef const &, double,
                                             std::optional<bool>) const {
    return {0, 0};
}

PageBBox LiteBase::nodeBBox(DOMNodeRef const &) const { return {}; }

LiteAdaptor::LiteAdaptor(LiteAdaptorOptions options, LiteWindowRef window,
                         std::shared_ptr<entities::EntityModule> entities)
    : LiteBase(std::move(window), std::move(entities)), options_(std::move(options)) {
    canMeasureNodes = false;
}

std::u16string LiteAdaptor::cjkPatternSource() { return nodeMixinCjkPattern(); }

std::u16string LiteAdaptor::replaceCjk(std::u16string_view text,
                                       regexp_host::RegExp &pattern) {
    return replaceCjkMatches(pattern, text);
}

double LiteAdaptor::fontSize(DOMNodeRef const &node) const {
    return options_.badCSS ? options_.fontSize : LiteBase::fontSize(node);
}

std::u16string LiteAdaptor::fontFamily(DOMNodeRef const &node) const {
    return options_.badCSS ? options_.fontFamily : LiteBase::fontFamily(node);
}

std::pair<double, double> LiteAdaptor::nodeSize(DOMNodeRef const &node, double,
                                                std::optional<bool>) const {
    if (!options_.badSizes)
        return LiteBase::nodeSize(node);
    const auto text = textContent(node);
    regexp_host::RegExp pattern(cjkPatternSource(), u"gu");
    const auto withoutCjk = replaceCjkMatches(pattern, text);
    const auto nonCjk = codePointLength(withoutCjk);
    const auto cjk = codePointLength(text) - nonCjk;
    return {static_cast<double>(cjk) * options_.cjkCharWidth +
                static_cast<double>(nonCjk) * options_.unknownCharWidth,
            options_.unknownCharHeight};
}

PageBBox LiteAdaptor::nodeBBox(DOMNodeRef const &node) const {
    return options_.badSizes ? PageBBox{} : LiteBase::nodeBBox(node);
}

} // namespace folia::math::adaptors::lite
