// folia.core.inline_document — per-content-node inline source + lossless CST.
//
// An editable content node (Paragraph, Heading, TableCell, and the
// paragraph/heading bodies inside list items, task items, block quotes,
// callouts, footnotes) owns an `InlineDocument`: the character-exact Markdown
// source of its inline content plus a lossless editable CST over that source.
//
// Editing mutates `source` (via TextEdit) and re-parses the CST of *this node
// only*; the tree is structural/analysis state, never a second coordinate
// system. Saving emits `source` verbatim.
export module folia.core.inline_document;
import std;
import folia.core.ids;
import folia.core.dialect;
import folia.core.inline_cst;
import folia.core.text_edit;
import folia.core.utf;
import folia.core.html_entities;

export namespace folia {

// Context the lossless inline parser needs: dialect, an id allocator seed,
// and (optionally) a link-definition resolver for reference links. This is a
// concrete type so InlineDocument can hold/forward it without a circular import.
struct InlineLinkDef {
    std::string href;
    std::optional<std::string> title;
};

enum class InlineSyntaxMode : std::uint8_t {
    Markdown,
    HtmlText,
};

struct InlineParseContext {
    MarkdownDialect dialect{};
    InlineSyntaxMode syntax_mode = InlineSyntaxMode::Markdown;
    NodeId next_id{1};
    std::function<NodeId()> allocate_id;
    std::function<std::optional<InlineLinkDef>(const std::string&)> resolve_link_label;
};

struct InlineDocument {
    std::u32string source;
    InlineCstTree tree;
    InlineSyntaxMode syntax_mode = InlineSyntaxMode::Markdown;

    std::u32string serialize() const { return source; }
};

// ---- queries over a CST (used by renderer / hit-test / format-state) ----

// Find the deepest node whose range covers `offset`. Returns nullptr if none.
// Descends into container children so the returned node is the most specific
// covering node (leaf or innermost container).
inline const InlineCstNode* node_at_offset(const InlineCstTree& tree, std::size_t offset) {
    const InlineCstNode* deepest = nullptr;
    std::function<const InlineCstNode*(const std::vector<InlineCstNode>&)> search =
        [&](const std::vector<InlineCstNode>& nodes) -> const InlineCstNode* {
        for (const auto& node : nodes) {
            if (!node.range.covers(offset)) continue;
            deepest = &node;
            search(node.children);
            return deepest;
        }
        return nullptr;
    };
    search(tree.nodes);
    return deepest;
}

// Is the offset inside a marker (opening or closing) of a delimited node?
inline bool offset_in_marker(const InlineCstNode& node, std::size_t offset) {
    const auto& delim = node.delimiter_ranges();
    if (delim.opening.contains(offset)) return true;
    if (delim.closing && delim.closing->contains(offset)) return true;
    return false;
}

// Does the node's content range cover the offset (half-open; the boundary at
// content.start counts as "in content", content.end counts as just past)?
inline bool offset_in_content(const InlineCstNode& node, std::size_t offset) {
    const auto& content = node.delimiter_ranges().content;
    return offset >= content.start && offset <= content.end;
}

inline std::u32string inline_source_slice(const InlineDocument& document, SourceRange range) {
    if (!range.valid_for(document.source.size())) return {};
    return document.source.substr(range.start, range.length());
}

inline std::u32string decode_inline_entity(std::u32string_view raw) {
    if (auto decoded = decode_html_character_reference(raw)) return std::move(*decoded);
    return std::u32string(raw);
}

// CommonMark's code-span text projection: line endings become spaces, then
// one outer ASCII space is removed only when both ends are spaces and the
// result contains a non-space. The source and its CST range remain untouched.
inline std::u32string normalize_inline_code_span(std::u32string_view raw) {
    std::u32string normalized;
    normalized.reserve(raw.size());
    for (std::size_t index = 0; index < raw.size(); ++index) {
        const auto ch = raw[index];
        if (ch == U'\r') {
            if (index + 1 < raw.size() && raw[index + 1] == U'\n') ++index;
            normalized.push_back(U' ');
        } else if (ch == U'\n') {
            normalized.push_back(U' ');
        } else {
            normalized.push_back(ch);
        }
    }
    if (normalized.size() >= 2 && normalized.front() == U' ' && normalized.back() == U' '
        && std::ranges::any_of(normalized, [](char32_t ch) { return ch != U' '; })) {
        normalized.erase(normalized.begin());
        normalized.pop_back();
    }
    return normalized;
}

inline void append_inline_visible_text(
    const InlineDocument& document,
    const InlineCstNodes& nodes,
    std::u32string& output) {
    for (const auto& node : nodes) {
        using K = InlineCstKind;
        switch (node.kind) {
            case K::Emphasis:
            case K::Strong:
            case K::Strikethrough:
            case K::Link:
                append_inline_visible_text(document, node.children, output);
                break;
            case K::HtmlElement:
                if (node.semantics().html_tag == "img") {
                    output += utf8_to_cps(node.semantics().alt);
                } else if (node.semantics().html_tag == "br") {
                    output.push_back(U'\n');
                } else {
                    append_inline_visible_text(document, node.children, output);
                }
                break;
            case K::HtmlComment:
                break;
            case K::Image:
                output += utf8_to_cps(node.semantics().alt);
                break;
            case K::WikiLink:
                output += utf8_to_cps(node.semantics().alias.value_or(node.semantics().target));
                break;
            case K::FootnoteRef:
                output += U"[^" + utf8_to_cps(node.semantics().label) + U"]";
                break;
            case K::CodeSpan:
                output += normalize_inline_code_span(inline_source_slice(document, node.delimiter_ranges().content));
                break;
            case K::InlineMath:
            case K::Autolink:
                output += inline_source_slice(document, node.delimiter_ranges().content);
                break;
            case K::Escape: {
                const auto raw = inline_source_slice(document, node.range);
                if (!raw.empty()) output.push_back(raw.back());
                break;
            }
            case K::Entity:
                output += decode_inline_entity(inline_source_slice(document, node.range));
                break;
            case K::SoftBreak:
            case K::HardBreak:
                output.push_back(U'\n');
                break;
            default:
                output += inline_source_slice(document, node.range);
                break;
        }
    }
}

inline std::u32string inline_visible_text(const InlineDocument& document) {
    std::u32string output;
    append_inline_visible_text(document, document.tree.nodes, output);
    return output;
}

inline bool inline_contains_kind(const InlineCstNodes& nodes, InlineCstKind kind) {
    for (const auto& node : nodes) {
        if (node.kind == kind || inline_contains_kind(node.children, kind)) return true;
    }
    return false;
}

inline bool inline_contains_kind(const InlineDocument& document, InlineCstKind kind) {
    return inline_contains_kind(document.tree.nodes, kind);
}

} // namespace folia
