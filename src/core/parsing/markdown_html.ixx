// Shared CommonMark 0.31.2 raw HTML open/closing-tag scanner.
export module folia.core.markdown_html;
import std;
import folia.core.text_edit;

export namespace folia {

struct MarkdownHtmlAttribute {
    SourceRange name;
    std::optional<SourceRange> value;
};

struct MarkdownHtmlTag {
    SourceRange range;
    std::string name;
    bool closing = false;
    bool self_closing = false;
    std::vector<MarkdownHtmlAttribute> attributes;
};

namespace markdown_html_detail {

inline bool ascii_letter(char32_t value) {
    return (value >= U'A' && value <= U'Z') || (value >= U'a' && value <= U'z');
}

inline bool tag_name_character(char32_t value) {
    return ascii_letter(value)
        || (value >= U'0' && value <= U'9')
        || value == U'-';
}

inline bool attribute_name_start(char32_t value) {
    return ascii_letter(value) || value == U'_' || value == U':';
}

inline bool attribute_name_character(char32_t value) {
    return attribute_name_start(value)
        || (value >= U'0' && value <= U'9')
        || value == U'.' || value == U'-';
}

inline bool horizontal_space(char32_t value) {
    return value == U' ' || value == U'\t';
}

inline bool line_ending(char32_t value) {
    return value == U'\r' || value == U'\n';
}

struct WhitespaceScan {
    std::size_t end = 0;
    bool present = false;
    bool valid = true;
};

inline WhitespaceScan scan_whitespace(
    std::u32string_view source,
    std::size_t start,
    std::size_t limit) {
    WhitespaceScan result{start, false, true};
    std::size_t line_endings = 0;
    while (result.end < limit) {
        const auto value = source[result.end];
        if (horizontal_space(value)) {
            result.present = true;
            ++result.end;
        } else if (line_ending(value)) {
            result.present = true;
            if (++line_endings > 1) {
                result.valid = false;
                return result;
            }
            if (value == U'\r' && result.end + 1 < limit
                && source[result.end + 1] == U'\n') {
                result.end += 2;
            } else {
                ++result.end;
            }
        } else {
            break;
        }
    }
    return result;
}

inline bool unquoted_value_character(char32_t value) {
    return !horizontal_space(value) && !line_ending(value)
        && value != U'"' && value != U'\'' && value != U'='
        && value != U'<' && value != U'>' && value != U'`';
}

inline std::optional<MarkdownHtmlTag> scan_tag(
    std::u32string_view source,
    std::size_t start,
    std::size_t limit) {
    limit = (std::min)(limit, source.size());
    if (start >= limit || source[start] != U'<') return std::nullopt;

    auto cursor = start + 1;
    bool closing = false;
    if (cursor < limit && source[cursor] == U'/') {
        closing = true;
        ++cursor;
    }

    if (cursor >= limit || !ascii_letter(source[cursor])) return std::nullopt;
    const auto name_start = cursor++;
    while (cursor < limit && tag_name_character(source[cursor])) ++cursor;
    const auto name_end = cursor;
    std::string name;
    name.reserve(name_end - name_start);
    for (auto index = name_start; index < name_end; ++index) {
        auto value = source[index];
        if (value >= U'A' && value <= U'Z') value += U'a' - U'A';
        name.push_back(static_cast<char>(value));
    }

    MarkdownHtmlTag tag;
    tag.name = std::move(name);
    tag.closing = closing;

    if (closing) {
        const auto whitespace = scan_whitespace(source, cursor, limit);
        if (!whitespace.valid) return std::nullopt;
        cursor = whitespace.end;
        if (cursor >= limit || source[cursor] != U'>') return std::nullopt;
        tag.range = {start, cursor + 1};
        return tag;
    }

    bool separator_already_consumed = false;
    while (cursor < limit) {
        if (source[cursor] == U'>') {
            tag.range = {start, cursor + 1};
            return tag;
        }
        if (source[cursor] == U'/' && cursor + 1 < limit && source[cursor + 1] == U'>') {
            tag.self_closing = true;
            tag.range = {start, cursor + 2};
            return tag;
        }

        if (!separator_already_consumed) {
            const auto whitespace = scan_whitespace(source, cursor, limit);
            if (!whitespace.valid || !whitespace.present) return std::nullopt;
            cursor = whitespace.end;
            if (cursor >= limit) return std::nullopt;
            if (source[cursor] == U'>') {
                tag.range = {start, cursor + 1};
                return tag;
            }
            if (source[cursor] == U'/' && cursor + 1 < limit && source[cursor + 1] == U'>') {
                tag.self_closing = true;
                tag.range = {start, cursor + 2};
                return tag;
            }
        }
        separator_already_consumed = false;

        if (cursor >= limit || !attribute_name_start(source[cursor])) return std::nullopt;
        const auto attribute_name_start_offset = cursor++;
        while (cursor < limit && attribute_name_character(source[cursor])) ++cursor;
        const auto attribute_name_end_offset = cursor;
        const auto name_whitespace = scan_whitespace(source, cursor, limit);
        if (!name_whitespace.valid) return std::nullopt;
        cursor = name_whitespace.end;

        MarkdownHtmlAttribute attribute;
        attribute.name = {attribute_name_start_offset, attribute_name_end_offset};
        if (cursor < limit && source[cursor] == U'=') {
            ++cursor;
            const auto value_whitespace = scan_whitespace(source, cursor, limit);
            if (!value_whitespace.valid) return std::nullopt;
            cursor = value_whitespace.end;
            if (cursor >= limit) return std::nullopt;

            const auto quote = source[cursor];
            if (quote == U'\'' || quote == U'"') {
                const auto value_start = ++cursor;
                while (cursor < limit && source[cursor] != quote) ++cursor;
                if (cursor >= limit) return std::nullopt;
                attribute.value = SourceRange{value_start, cursor};
                ++cursor;
                tag.attributes.push_back(attribute);
                continue;
            }

            const auto value_start = cursor;
            while (cursor < limit && unquoted_value_character(source[cursor])) ++cursor;
            if (cursor == value_start) return std::nullopt;
            attribute.value = SourceRange{value_start, cursor};
            tag.attributes.push_back(attribute);
            continue;
        }

        tag.attributes.push_back(attribute);
        if (name_whitespace.present) {
            separator_already_consumed = true;
        }
    }
    return std::nullopt;
}

} // namespace markdown_html_detail

inline std::optional<MarkdownHtmlTag> scan_markdown_html_tag(
    std::u32string_view source,
    std::size_t start,
    std::size_t limit) {
    return markdown_html_detail::scan_tag(source, start, limit);
}

} // namespace folia
