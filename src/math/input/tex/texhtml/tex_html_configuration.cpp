/*************************************************************
 * Native port of MathJax 4.1.3 input/tex/texhtml/TexHtmlConfiguration.ts.
 * TeX HTML parsing uses the active request's borrowed DOM adaptor.
 *************************************************************/
module folia.math.input.tex.texhtml.tex_html_configuration;
import folia.math.core.dom_adaptor;
import folia.math.core.mml_tree.mml_nodes.html_node;
import folia.math.core.mml_tree.xml_dom_adaptor;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.token;
import std;

namespace folia::math::tex {
namespace {
using handlers::html::HTMLDomStringsOptions;
using handlers::html::IncludeHtmlHandler;
using handlers::html::IncludedHtmlTag;
using folia::math::DOMNodeRef;

class TexHtmlMap final : public MacroMap {
    std::shared_ptr<HTMLDomStringsOptions> options_;
    std::shared_ptr<mml::XmlDomAdaptor> xml_;

  public:
    TexHtmlMap(MacroMapInit init, std::shared_ptr<HTMLDomStringsOptions> options)
        : MacroMap(std::move(init)), options_(std::move(options)),
          xml_(mml::XmlDomAdaptor::create()) {}

    HTMLDomStringsOptions &options() const noexcept { return *options_; }
    std::shared_ptr<mml::XmlDomAdaptor> const &xml() const noexcept { return xml_; }
};

TexHtmlMap const *configuredMap(TexParser &parser) {
    auto *handler = parser.configuration.handlers.get(HandlerKind::character);
    if (!handler)
        return nullptr;
    const auto selected = handler->retrieve(u"tex-html");
    return dynamic_cast<TexHtmlMap const *>(selected.get());
}

bool jsSpace(char16_t c) {
    return c == u' ' || c == u'\t' || c == u'\n' || c == u'\v' || c == u'\f' ||
           c == u'\r' || c == 0x00a0 || c == 0x1680 || (c >= 0x2000 && c <= 0x200a) ||
           c == 0x2028 || c == 0x2029 || c == 0x202f || c == 0x205f || c == 0x3000 ||
           c == 0xfeff;
}

std::u16string trimHtml(std::u16string_view text) {
    std::size_t first = 0, last = text.size();
    while (first < last && jsSpace(text[first]))
        ++first;
    while (last > first && jsSpace(text[last - 1]))
        --last;
    return std::u16string(text.substr(first, last - first));
}

struct StartTag {
    std::u16string text;
    std::u16string number;
    std::size_t length{};
};

std::optional<StartTag> startTag(std::u16string_view input) {
    constexpr std::u16string_view prefix = u"tex-html";
    if (!input.starts_with(prefix))
        return {};
    std::size_t at = prefix.size();
    std::u16string number;
    if (at < input.size() && input[at] == u' ') {
        constexpr std::u16string_view attribute = u" n=\"";
        if (input.substr(at, attribute.size()) != attribute)
            return {};
        at += attribute.size();
        const auto begin = at;
        while (at < input.size() && input[at] >= u'0' && input[at] <= u'9')
            ++at;
        if (at == begin || at >= input.size() || input[at] != u'\"')
            return {};
        number = std::u16string(input.substr(begin, at - begin));
        ++at;
    }
    if (at >= input.size() || input[at] != u'>')
        return {};
    ++at;
    return StartTag{std::u16string(input.substr(0, at)), std::move(number), at};
}

std::u16string includeTexHtml(DOMNodeRef const &node, AbstractDOMAdaptor &adaptor) {
    const auto html = adaptor.innerHTML(node);
    std::size_t count = 1;
    constexpr std::u16string_view end = u"</tex-html>";
    for (std::size_t at = 0; (at = html.find(end, at)) != std::u16string::npos;
         at += end.size())
        ++count;
    std::u16string digits;
    if (count > 1)
        for (const auto digit : std::to_string(count))
            digits.push_back(static_cast<char16_t>(digit));
    const auto marker = count > 1 ? u" n=\"" + digits + u"\"" : std::u16string{};
    auto result = u"<tex-html" + marker + u">" + html;
    if (count > 1)
        result += u"<!" + digits + u">";
    result += u"</tex-html>";
    return result;
}

void installIncludeHook(HTMLDomStringsOptions &options) {
    auto found = std::find_if(options.includeHtmlTags.begin(), options.includeHtmlTags.end(),
                              [](IncludedHtmlTag const &entry) {
                                  return entry.name == u"tex-html";
                              });
    if (found == options.includeHtmlTags.end()) {
        options.includeHtmlTags.push_back({u"tex-html", IncludeHtmlHandler{&includeTexHtml}});
        return;
    }
    const bool configured = std::visit(
        [](auto const &value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::u16string>)
                return !value.empty();
            else
                return value != nullptr;
        },
        found->value);
    if (!configured)
        found->value = IncludeHtmlHandler{&includeTexHtml};
}

void configureTexHtml(ParserConfiguration &configuration, TeX &) {
    auto *handler = configuration.handlers.get(HandlerKind::character);
    const auto selected = handler ? handler->retrieve(u"tex-html") : TokenMapRef{};
    const auto *map = dynamic_cast<TexHtmlMap const *>(selected.get());
    if (map)
        installIncludeHook(map->options());
}

void texHtmlPreFilter(TexFilterData &data) {
    data.data.request().texhtmlAdaptor = data.adaptor;
}

void texHtmlPostFilter(TexFilterData &data) {
    data.data.request().texhtmlAdaptor = nullptr;
}

} // namespace

DispatchOutcome HtmlNodeMethods::TexHTML(TexParser &parser, MacroInvocation const &) {
    if (!parser.options().allowTexHTML.value_or(false))
        return ContinueDispatch{};
    const auto source = parser.string();
    const auto offset = parser.position();
    const auto tag = offset <= source.size()
                         ? startTag(source.substr(offset))
                         : std::optional<StartTag>{};
    if (!tag)
        return ContinueDispatch{};

    std::u16string end;
    if (!tag->number.empty())
        end = u"<!" + tag->number + u">";
    end += u"</tex-html>";
    const auto bodyStart = offset + tag->length;
    parser.setPosition(bodyStart);
    const auto found = source.find(end, bodyStart);
    if (found == std::u16string_view::npos) {
        const auto sourceTag = u"<" + tag->text;
        const std::array arguments{end, sourceTag};
        throw TexError("TokenNotFoundForCommand", u"Could not find %1 for %2", arguments);
    }
    const auto htmlSource = trimHtml(source.substr(bodyStart, found - bodyStart));
    parser.setPosition(found + end.size());

    auto *adaptor = parser.configuration.request().texhtmlAdaptor;
    const auto *map = configuredMap(parser);
    if (!adaptor || !map)
        throw std::logic_error("TexHTML requires its configured request DOM adaptor and map");

    const auto document = adaptor->parse(htmlSource);
    const auto body = adaptor->body(document);
    const auto nodes = body ? adaptor->childNodes(body) : std::vector<DOMNodeRef>{};
    if (nodes.empty())
        return Handled{};
    DOMNodeRef root;
    if (nodes.size() == 1)
        root = nodes.front();
    else
        root = adaptor->node(u"div", {}, nodes);

    auto frozen = map->xml()->fromDOM(root);
    auto &htmlNode = parser.create(mml::NodeKind::html);
    auto *html = dynamic_cast<mml::HtmlNode *>(&htmlNode);
    if (!html)
        throw std::logic_error("NodeFactory did not create an HtmlNode");
    html->setHTML(std::move(frozen), map->xml());
    std::array<mml::MmlNode *, 1> children{html};
    auto &text = parser.create(mml::NodeKind::mtext, children);
    parser.Push(&text);
    return Handled{};
}

ConfigurationRef TexHtmlConfiguration::initializeModule(
    ConfigurationHandler &registry, MapHandler &maps,
    std::shared_ptr<HTMLDomStringsOptions> options) {
    if (auto prior = registry.get(u"texhtml"))
        return prior;
    if (!options)
        throw std::invalid_argument("TexHtml requires HTMLDomStringsOptions");

    std::vector<Macro> commands{Macro(u"<", MacroMethod{&HtmlNodeMethods::TexHTML})};
    auto map = std::make_shared<TexHtmlMap>(
        MacroMapInit{{u"tex-html", std::nullopt}, std::move(commands)}, std::move(options));
    maps.registerMap(map);

    ConfigurationDefinition definition;
    definition.handler = {{HandlerKind::character, {u"tex-html"}}};
    definition.options.allowTexHTML = false;
    definition.config = ProcessorSpec<ConfigMethod>{&configureTexHtml};
    definition.preprocessors.emplace_back(&texHtmlPreFilter);
    definition.postprocessors.emplace_back(&texHtmlPostFilter);
    return Configuration::create(registry, u"texhtml", std::move(definition));
}
} // namespace folia::math::tex
