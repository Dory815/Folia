/*************************************************************
 * Native port of MathJax 4.1.3 handlers/html/HTMLDomStrings.ts.
 *************************************************************/
export module folia.math.handlers.html.html_dom_strings;
export import folia.math.core.dom_adaptor;
import folia.math.util.regexp_host;
import std;

export namespace folia::math::handlers::html {
using IncludeHtmlHandler = std::u16string (*)(DOMNodeRef const&, AbstractDOMAdaptor&);
using IncludeHtmlTag = std::variant<std::u16string, IncludeHtmlHandler>;

struct IncludedHtmlTag {
    std::u16string name;
    IncludeHtmlTag value;
};

struct HTMLDomStringsOptions {
    std::vector<std::u16string> skipHtmlTags{
        u"script", u"noscript", u"style", u"textarea", u"pre",
        u"code", u"math", u"select", u"option", u"mjx-container"};
    std::vector<IncludedHtmlTag> includeHtmlTags{
        {u"br", std::u16string(u"\n")},
        {u"wbr", std::u16string{}},
        {u"#comment", std::u16string{}}};
    std::variant<std::u16string, std::vector<std::u16string>> ignoreHtmlClass{
        std::u16string(u"mathjax_ignore")};
    std::variant<std::u16string, std::vector<std::u16string>> processHtmlClass{
        std::u16string(u"mathjax_process")};
};

using HTMLNodeList = std::vector<std::pair<DOMNodeRef, std::size_t>>;
struct HTMLDomStringsResult {
    std::vector<std::u16string> strings;
    std::vector<HTMLNodeList> nodes;
};

class HTMLDomStrings {
    AbstractDOMAdaptor* adaptor_;
    HTMLDomStringsOptions options_;
    std::vector<std::u16string> strings_;
    std::u16string string_;
    HTMLNodeList snodes_;
    std::vector<HTMLNodeList> nodes_;
    std::vector<std::pair<DOMNodeRef, bool>> stack_;
    std::unique_ptr<regexp_host::RegExp> skipHtmlTags_;
    std::unique_ptr<regexp_host::RegExp> ignoreHtmlClass_;
    std::unique_ptr<regexp_host::RegExp> processHtmlClass_;

    void init();
    void getPatterns();
    void pushString();
    void extendString(DOMNodeRef const&, std::u16string_view);
    DOMNodeRef handleText(DOMNodeRef const&, bool);
    DOMNodeRef handleTag(DOMNodeRef const&, bool);
    std::pair<DOMNodeRef, bool> handleContainer(DOMNodeRef const&, bool);
    DOMNodeRef handleOther(DOMNodeRef const&, bool);

public:
    explicit HTMLDomStrings(AbstractDOMAdaptor&, HTMLDomStringsOptions options = {});
    HTMLDomStringsOptions& options() noexcept { return options_; }
    HTMLDomStringsOptions const& options() const noexcept { return options_; }
    HTMLDomStringsResult find(DOMNodeRef);
};
}
