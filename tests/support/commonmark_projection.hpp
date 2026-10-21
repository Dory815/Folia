#pragma once

// Semantic HTML-shaped projection for the official CommonMark conformance
// tests. This consumes Folia's parsed tree and lossless inline CST; it is not
// a Markdown parser or a serializer.

namespace folia::test::commonmark {

struct Projection {
    std::string strict_tree;
    std::string relaxed_tree;
    std::vector<std::string> limitations;
};

namespace projection_detail {

struct Node;
struct Child {
    std::string text;
    std::unique_ptr<Node> node;
    Child(std::string value) : text(std::move(value)) {}
    Child(Node value);
    ~Child();
    Child(Child&&) noexcept;
    Child& operator=(Child&&) noexcept;
    Child(Child const&) = delete;
    Child& operator=(Child const&) = delete;
};
struct Node {
    std::string tag;
    std::vector<std::pair<std::string, std::string>> attrs;
    std::vector<Child> children;
};
inline Child::Child(Node value) : node(std::make_unique<Node>(std::move(value))) {}
inline Child::~Child() = default;
inline Child::Child(Child&&) noexcept = default;
inline Child& Child::operator=(Child&&) noexcept = default;

inline std::string utf8(std::u32string_view value) { return cps_to_utf8(value); }

inline std::string json_string(std::string_view value) {
    static constexpr char hex[] = "0123456789abcdef";
    std::string out;
    out.push_back('"');
    for (unsigned char ch : value) {
        switch (ch) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (ch < 0x20) {
                    out += "\\u00";
                    out.push_back(hex[ch >> 4]);
                    out.push_back(hex[ch & 0xf]);
                } else {
                    out.push_back(static_cast<char>(ch));
                }
        }
    }
    out.push_back('"');
    return out;
}

inline void normalize_attrs(Node& node) {
    std::ranges::sort(node.attrs, {}, [](auto const& attr) -> auto const& { return attr.first; });
}

inline Node element(std::string tag, std::vector<std::pair<std::string, std::string>> attrs = {}) {
    Node out{std::move(tag), std::move(attrs), {}};
    normalize_attrs(out);
    return out;
}

inline void append_text(Node& node, std::string value) {
    if (value.empty()) return;
    if (!node.children.empty() && !node.children.back().node) {
        node.children.back().text += value;
    } else {
        node.children.emplace_back(std::move(value));
    }
}

inline void append_node(Node& parent, Node child) {
    parent.children.emplace_back(std::move(child));
}

inline Node clone(Node const& source) {
    Node out{source.tag, source.attrs, {}};
    for (auto const& child : source.children) {
        if (child.node) out.children.emplace_back(clone(*child.node));
        else out.children.emplace_back(child.text);
    }
    return out;
}

inline std::string block_kind_name(BlockKind kind) {
    switch (kind) {
        case BlockKind::Document: return "Document";
        case BlockKind::Paragraph: return "Paragraph";
        case BlockKind::Heading: return "Heading";
        case BlockKind::CalloutTitle: return "CalloutTitle";
        case BlockKind::BlockQuote: return "BlockQuote";
        case BlockKind::List: return "List";
        case BlockKind::TaskList: return "TaskList";
        case BlockKind::ListItem: return "ListItem";
        case BlockKind::TaskListItem: return "TaskListItem";
        case BlockKind::CodeBlock: return "CodeBlock";
        case BlockKind::MathBlock: return "MathBlock";
        case BlockKind::Table: return "Table";
        case BlockKind::TableRow: return "TableRow";
        case BlockKind::TableCell: return "TableCell";
        case BlockKind::ImageBlock: return "ImageBlock";
        case BlockKind::Callout: return "Callout";
        case BlockKind::FootnoteDefinition: return "FootnoteDefinition";
        case BlockKind::Toc: return "Toc";
        case BlockKind::Frontmatter: return "Frontmatter";
        case BlockKind::ThematicBreak: return "ThematicBreak";
        case BlockKind::LinkDefinition: return "LinkDefinition";
        case BlockKind::UnsupportedMarkup: return "UnsupportedMarkup";
        case BlockKind::HtmlContainer: return "HtmlContainer";
        case BlockKind::Extension: return "Extension";
    }
    return "Unknown";
}

inline void limitation(std::vector<std::string>& out, std::string value) {
    if (std::ranges::find(out, value) == out.end()) out.push_back(std::move(value));
}

inline Node placeholder(std::string tag, std::string key, std::string value) {
    return element(std::move(tag), {{std::move(key), std::move(value)}});
}

inline std::string code_block_text(BlockSourceDocument const& source) {
    auto text = utf8(block_source_content(source));
    auto const& tree = source.tree();
    auto const boundary = tree.content_to_source.empty() ? std::size_t{0}
        : tree.content_to_source.back();
    bool separated_ending = std::ranges::any_of(tree.tokens, [&](auto const& token) {
        return token.kind == BlockSourceTokenKind::LineBreak
            && token.source_range.start == boundary;
    });
    // Indented-code normalization trims trailing blank lines but retains their
    // source offsets in the terminal mapping. Fenced code instead keeps the
    // separator before its closing fence as the ending of its final content
    // line, so that ending must remain part of the projected code text.
    if (tree.kind == BlockSourceKind::IndentedCode) {
        if (!text.empty() && text.back() != '\n') text.push_back('\n');
    } else if (separated_ending) {
        // For fenced code this token is the ending of the last content line,
        // including an otherwise empty final code line before the closing fence.
        text.push_back('\n');
    } else if (!text.empty() && text.back() != '\n') {
        text.push_back('\n');
    }
    return text;
}

inline void project_inline_nodes(
    Node& parent,
    InlineDocument const& doc,
    InlineCstNodes const& nodes,
    std::vector<std::string>& limitations) {
    using K = InlineCstKind;
    for (auto const& item : nodes) {
        auto raw = [&] { return utf8(inline_source_slice(doc, item.range)); };
        auto content = [&] { return utf8(inline_source_slice(doc, item.delimiter_ranges().content)); };
        switch (item.kind) {
            case K::Emphasis: {
                auto n = element("em");
                project_inline_nodes(n, doc, item.children, limitations);
                append_node(parent, std::move(n));
                break;
            }
            case K::Strong: {
                auto n = element("strong");
                project_inline_nodes(n, doc, item.children, limitations);
                append_node(parent, std::move(n));
                break;
            }
            case K::Strikethrough: {
                auto n = element("del");
                project_inline_nodes(n, doc, item.children, limitations);
                append_node(parent, std::move(n));
                limitation(limitations, "gfm-strikethrough-extension");
                break;
            }
            case K::CodeSpan: {
                auto n = element("code");
                append_text(n, cps_to_utf8(normalize_inline_code_span(inline_source_slice(doc, item.delimiter_ranges().content))));
                append_node(parent, std::move(n));
                break;
            }
            case K::Link: {
                auto attrs = std::vector<std::pair<std::string, std::string>>{{"href", item.semantics().href}};
                if (item.semantics().title) attrs.emplace_back("title", *item.semantics().title);
                auto n = element("a", std::move(attrs));
                project_inline_nodes(n, doc, item.children, limitations);
                append_node(parent, std::move(n));
                break;
            }
            case K::Image: {
                auto attrs = std::vector<std::pair<std::string, std::string>>{
                    {"alt", item.semantics().alt}, {"src", item.semantics().href}};
                if (item.semantics().title) attrs.emplace_back("title", *item.semantics().title);
                append_node(parent, element("img", std::move(attrs)));
                break;
            }
            case K::Entity:
                append_text(parent, utf8(decode_inline_entity(inline_source_slice(doc, item.range))));
                break;
            case K::Escape: {
                auto source = inline_source_slice(doc, item.range);
                append_text(parent, source.empty() ? std::string{} : utf8(std::u32string_view(source).substr(source.size() - 1)));
                break;
            }
            case K::SoftBreak:
                if (item.range.end == doc.source.size()) break;
                append_text(parent, "\n");
                break;
            case K::HardBreak:
                if (item.range.end == doc.source.size()) break;
                append_node(parent, element("br"));
                append_text(parent, "\n");
                break;
            case K::Autolink:
                // The CST exposes content and the source range; use semantic href
                // where available and keep the visible spelling from source.
                append_node(parent, element("a", {{"href", item.semantics().href}}));
                if (!parent.children.empty() && parent.children.back().node) {
                    append_text(*parent.children.back().node, content());
                }
                break;
            case K::HtmlElement:
            case K::HtmlComment:
                append_node(parent, placeholder("html-policy/projection", "source", raw()));
                limitation(limitations, "inline-html-policy/projection");
                break;
            case K::Raw:
                append_text(parent, raw());
                break;
            case K::Incomplete:
            case K::Error:
                // Preserve the exact incomplete/error spelling as literal text.
                append_text(parent, raw());
                break;
            case K::InlineMath:
            case K::FootnoteRef:
            case K::WikiLink:
            case K::Extension:
                append_node(parent, placeholder("extension", "kind", std::to_string(static_cast<int>(item.kind))));
                limitation(limitations, "inline-extension:" + std::to_string(static_cast<int>(item.kind)));
                break;
            case K::Text:
            case K::Whitespace:
            case K::Delimiter:
                append_text(parent, raw());
                break;
        }
    }
}

inline bool project_block(
    Node& parent,
    BlockNode const& block,
    std::vector<std::string>& limitations) {
    auto add_children = [&](Node& node) {
        for (auto const& child : block.children) project_block(node, child, limitations);
    };
    switch (block.kind) {
        case BlockKind::Document:
            add_children(parent);
            return false;
        case BlockKind::Paragraph: {
            // Empty editable paragraphs encode preserved blank source lines.
            // They are not Markdown paragraphs in the specification's HTML.
            if (block.inline_content.source.empty()) return false;
            auto n = element("p");
            project_inline_nodes(n, block.inline_content, block.inline_content.tree.nodes, limitations);
            append_node(parent, std::move(n));
            return true;
        }
        case BlockKind::Heading: {
            auto const level = static_cast<int>(block.text_special().level);
            auto n = element("h" + std::to_string(level));
            project_inline_nodes(n, block.inline_content, block.inline_content.tree.nodes, limitations);
            append_node(parent, std::move(n));
            return true;
        }
        case BlockKind::BlockQuote: {
            auto n = element("blockquote");
            add_children(n);
            append_node(parent, std::move(n));
            return true;
        }
        case BlockKind::List: {
            auto const& list = block.list_special();
            std::vector<std::pair<std::string, std::string>> attrs;
            if (list.ordered && list.start != 1) attrs.emplace_back("start", std::to_string(list.start));
            auto n = element(list.ordered ? "ol" : "ul", std::move(attrs));
            add_children(n);
            append_node(parent, std::move(n));
            limitation(limitations, "list-tightness-not-exposed");
            return true;
        }
        case BlockKind::ListItem: {
            auto n = element("li");
            add_children(n);
            append_node(parent, std::move(n));
            return true;
        }
        case BlockKind::CodeBlock: {
            auto pre = element("pre");
            std::vector<std::pair<std::string, std::string>> attrs;
            auto const& language = block.block_source.tree().language;
            if (language && !language->empty()) attrs.emplace_back("class", "language-" + *language);
            auto code = element("code", std::move(attrs));
            append_text(code, code_block_text(block.block_source));
            append_node(pre, std::move(code));
            append_node(parent, std::move(pre));
            return true;
        }
        case BlockKind::ThematicBreak:
            append_node(parent, element("hr"));
            return true;
        case BlockKind::LinkDefinition:
            // CommonMark link definitions affect inline resolution but render no node.
            return false;
        case BlockKind::ImageBlock: {
            auto const& image = block.image_special();
            std::vector<std::pair<std::string, std::string>> attrs{
                {"alt", image.image_alt}, {"src", image.src}};
            if (image.image_title) attrs.emplace_back("title", *image.image_title);
            auto p = element("p");
            auto image_node = element("img", std::move(attrs));
            if (image.image_link) {
                auto link = element("a", {{"href", *image.image_link}});
                append_node(link, std::move(image_node));
                append_node(p, std::move(link));
            } else {
                append_node(p, std::move(image_node));
            }
            append_node(parent, std::move(p));
            return true;
        }
        case BlockKind::TaskList:
        case BlockKind::TaskListItem:
        case BlockKind::CalloutTitle:
        case BlockKind::MathBlock:
        case BlockKind::Table:
        case BlockKind::TableRow:
        case BlockKind::TableCell:
        case BlockKind::Callout:
        case BlockKind::FootnoteDefinition:
        case BlockKind::Toc:
        case BlockKind::Frontmatter:
        case BlockKind::UnsupportedMarkup:
        case BlockKind::HtmlContainer:
        case BlockKind::Extension:
            if (block.kind == BlockKind::HtmlContainer || block.has_html_source()) {
                append_node(parent, placeholder("html-policy/projection", "kind", block_kind_name(block.kind)));
                limitation(limitations, "block-html-policy/projection");
            } else {
                append_node(parent, placeholder("extension", "kind", block_kind_name(block.kind)));
                limitation(limitations, "block-extension:" + block_kind_name(block.kind));
            }
            return true;
    }
    append_node(parent, placeholder("extension", "kind", block_kind_name(block.kind)));
    limitation(limitations, "block-extension:" + block_kind_name(block.kind));
    return true;
}

inline bool is_container(std::string_view tag) {
    return tag == "root" || tag == "blockquote" || tag == "ul" || tag == "ol" || tag == "li";
}
inline bool is_block(std::string_view tag) {
    return is_container(tag) || tag == "p" || tag == "pre" || tag == "hr"
        || (tag.size() == 2 && tag[0] == 'h' && tag[1] >= '1' && tag[1] <= '6');
}
inline bool whitespace_only(std::string const& text) {
    // Match the explicit ASCII whitespace set used by the HTML canonicalizer
    // contract, including vertical tab; UTF-8 non-ASCII bytes stay significant.
    return std::ranges::all_of(text, [](unsigned char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
    });
}

inline void clean(Node& node, bool relax) {
    std::vector<Child> cleaned;
    for (auto& child : node.children) {
        if (!child.node) {
            if (is_container(node.tag) && whitespace_only(child.text)) continue;
            if (!cleaned.empty() && !cleaned.back().node) cleaned.back().text += child.text;
            else cleaned.emplace_back(std::move(child.text));
            continue;
        }
        clean(*child.node, relax);
        if (relax && node.tag == "li" && child.node->tag == "p") {
            for (auto& grandchild : child.node->children) {
                if (!grandchild.node) {
                    if (!cleaned.empty() && !cleaned.back().node) cleaned.back().text += grandchild.text;
                    else cleaned.emplace_back(std::move(grandchild.text));
                } else cleaned.emplace_back(std::move(*grandchild.node));
            }
        } else {
            cleaned.emplace_back(std::move(*child.node));
        }
    }
    node.children = std::move(cleaned);
    if (!is_container(node.tag)) return;
    for (std::size_t i = 0; i < node.children.size(); ++i) {
        auto& child = node.children[i];
        if (child.node) continue;
        auto value = child.text;
        bool previous_block = i == 0;
        bool next_block = i + 1 == node.children.size();
        if (i > 0 && node.children[i - 1].node) previous_block = is_block(node.children[i - 1].node->tag);
        if (i + 1 < node.children.size() && node.children[i + 1].node) next_block = is_block(node.children[i + 1].node->tag);
        if (previous_block) while (!value.empty() && value.front() == '\n') value.erase(value.begin());
        if (next_block) while (!value.empty() && value.back() == '\n') value.pop_back();
        child.text = std::move(value);
    }
    std::erase_if(node.children, [](Child const& child) { return !child.node && child.text.empty(); });
}

inline void encode(Node const& node, std::string& out) {
    out.push_back('[');
    out += json_string(node.tag);
    out += ",[";
    for (std::size_t i = 0; i < node.attrs.size(); ++i) {
        if (i) out.push_back(',');
        out.push_back('[');
        out += json_string(node.attrs[i].first);
        out.push_back(',');
        out += json_string(node.attrs[i].second);
        out.push_back(']');
    }
    out += "],[";
    for (std::size_t i = 0; i < node.children.size(); ++i) {
        if (i) out.push_back(',');
        auto const& child = node.children[i];
        if (child.node) encode(*child.node, out);
        else out += json_string(child.text);
    }
    out += "]]";
}

inline bool inline_lossless_nodes(InlineCstNodes const& nodes, InlineDocument const& doc) {
    for (auto const& node : nodes) {
        if (!node.range.valid_for(doc.source.size()) || node.range.empty()) return false;
        if (!inline_lossless_nodes(node.children, doc)) return false;
    }
    return true;
}

} // namespace projection_detail

inline std::string json_string(std::string_view value) {
    return projection_detail::json_string(value);
}

inline bool inline_document_lossless(BlockNode const& block) {
    auto const& doc = block.inline_content;
    if (!(tokens_partition_source(doc.tree, doc.source.size())
        && roots_partition_source(doc.tree, doc.source.size())
        && flatten_tokens(doc.tree, doc.source) == doc.source
        && serialize_lossless(doc.tree, doc.source) == doc.source
        && projection_detail::inline_lossless_nodes(doc.tree.nodes, doc))) return false;
    for (auto const& child : block.children) {
        if (!inline_document_lossless(child)) return false;
    }
    return true;
}

inline bool inline_lossless(BlockNode const& block) {
    return inline_document_lossless(block);
}

inline MarkdownDialect dialect() {
    MarkdownDialect d{};
    d.commonmark = true;
    d.gfm = {false, false, false, false};
    d.math = {false, false, false, false, false, MathRenderBackend::PlainTextFallback};
    d.toc = {false, false, false};
    d.frontmatter = {false, false, false};
    d.footnotes = false;
    d.definition_lists = false;
    d.callouts = false;
    d.wiki_links = false;
    d.tables = {false, false};
    d.diagrams = {false, false, false};
    return d;
}

inline Projection project(BlockNode const& block) {
    using namespace projection_detail;
    Node strict = element("root");
    Projection result;
    if (block.kind == BlockKind::Document) {
        for (auto const& child : block.children) project_block(strict, child, result.limitations);
    } else {
        project_block(strict, block, result.limitations);
    }
    Node relaxed = clone(strict);
    clean(strict, false);
    clean(relaxed, true);
    encode(strict, result.strict_tree);
    encode(relaxed, result.relaxed_tree);
    std::ranges::sort(result.limitations);
    return result;
}

} // namespace folia::test::commonmark
