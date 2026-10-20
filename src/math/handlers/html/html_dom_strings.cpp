module folia.math.handlers.html.html_dom_strings;
import folia.math.util.regexp_host;
import std;

namespace folia::math::handlers::html {
namespace {
std::vector<std::u16string> makeArray(
    std::variant<std::u16string, std::vector<std::u16string>> const& value) {
    if (const auto* scalar = std::get_if<std::u16string>(&value)) return {*scalar};
    return std::get<std::vector<std::u16string>>(value);
}

std::u16string join(std::span<std::u16string const> values, std::u16string_view separator) {
    std::u16string result;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) result += separator;
        result += values[i];
    }
    return result;
}
}

HTMLDomStrings::HTMLDomStrings(AbstractDOMAdaptor& adaptor, HTMLDomStringsOptions options)
    : adaptor_(&adaptor), options_(std::move(options)) {
    init();
    getPatterns();
}

void HTMLDomStrings::init() {
    strings_.clear();
    string_.clear();
    snodes_.clear();
    nodes_.clear();
    stack_.clear();
}

void HTMLDomStrings::getPatterns() {
    const auto skip = join(options_.skipHtmlTags, u"|");
    const auto ignoreValues = makeArray(options_.ignoreHtmlClass);
    const auto processValues = makeArray(options_.processHtmlClass);
    const auto ignore = join(ignoreValues, u"|");
    // Source's processHtmlClass pattern concatenates the makeArray result
    // directly, so arrays stringify with commas here.
    const auto process = join(processValues, u",");
    skipHtmlTags_ = std::make_unique<regexp_host::RegExp>(u"^(?:" + skip + u")$", u"i");
    ignoreHtmlClass_ = std::make_unique<regexp_host::RegExp>(
        u"(?:^| )(?:" + ignore + u")(?: |$)");
    processHtmlClass_ = std::make_unique<regexp_host::RegExp>(
        u"(?:^| )(?:" + process + u")(?: |$)");
}

void HTMLDomStrings::pushString() {
    regexp_host::RegExp nonspace(u"\\S");
    if (nonspace.exec(string_)) {
        strings_.push_back(std::move(string_));
        nodes_.push_back(std::move(snodes_));
    }
    string_.clear();
    snodes_.clear();
}

void HTMLDomStrings::extendString(DOMNodeRef const& node, std::u16string_view text) {
    snodes_.emplace_back(node, text.size());
    string_.append(text);
}

DOMNodeRef HTMLDomStrings::handleText(DOMNodeRef const& node, bool ignore) {
    if (!ignore) extendString(node, adaptor_->value(node));
    return adaptor_->next(node);
}

DOMNodeRef HTMLDomStrings::handleTag(DOMNodeRef const& node, bool ignore) {
    if (!ignore) {
        const auto kind = adaptor_->kind(node);
        const auto found = std::find_if(options_.includeHtmlTags.begin(), options_.includeHtmlTags.end(),
            [kind](IncludedHtmlTag const& entry) { return entry.name == kind; });
        if (found != options_.includeHtmlTags.end()) {
            if (const auto* text = std::get_if<std::u16string>(&found->value)) {
                extendString(node, *text);
            } else {
                const auto handler = std::get<IncludeHtmlHandler>(found->value);
                if (handler) extendString(node, handler(node, *adaptor_));
                else extendString(node, {});
            }
        }
    }
    return adaptor_->next(node);
}

std::pair<DOMNodeRef, bool> HTMLDomStrings::handleContainer(DOMNodeRef const& node, bool ignore) {
    pushString();
    const auto className = adaptor_->getAttribute(node, u"class").value_or(u"");
    const auto tagName = adaptor_->kind(node);
    const bool process = processHtmlClass_->exec(className).has_value();
    auto next = adaptor_->firstChild(node);
    const auto mathJax = adaptor_->getAttribute(node, u"data-MJX");
    if (next && (!mathJax || mathJax->empty()) &&
        (process || !skipHtmlTags_->exec(std::u16string(tagName)).has_value())) {
        if (auto sibling = adaptor_->next(node)) stack_.emplace_back(std::move(sibling), ignore);
        ignore = (ignore || ignoreHtmlClass_->exec(className).has_value()) && !process;
    } else {
        next = adaptor_->next(node);
    }
    return {std::move(next), ignore};
}

DOMNodeRef HTMLDomStrings::handleOther(DOMNodeRef const& node, bool) {
    pushString();
    return adaptor_->next(node);
}

HTMLDomStringsResult HTMLDomStrings::find(DOMNodeRef node) {
    init();
    try {
        const auto stop = node ? adaptor_->next(node) : DOMNodeRef{};
        bool ignore = false;
        while (node && node != stop) {
            const auto kind = adaptor_->kind(node);
            if (kind == u"#text") {
                node = handleText(node, ignore);
            } else if (std::any_of(options_.includeHtmlTags.begin(), options_.includeHtmlTags.end(),
                           [kind](IncludedHtmlTag const& entry) { return entry.name == kind; })) {
                node = handleTag(node, ignore);
            } else if (!kind.empty()) {
                auto [next, childIgnore] = handleContainer(node, ignore);
                node = std::move(next);
                ignore = childIgnore;
            } else {
                node = handleOther(node, ignore);
            }
            if (!node && !stack_.empty()) {
                pushString();
                auto frame = std::move(stack_.back());
                stack_.pop_back();
                node = std::move(frame.first);
                ignore = frame.second;
            }
        }
        pushString();
        HTMLDomStringsResult result{std::move(strings_), std::move(nodes_)};
        init();
        return result;
    } catch (...) {
        init();
        throw;
    }
}
}
