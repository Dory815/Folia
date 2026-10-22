#include <algorithm>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "support/folia_test.hpp"
import folia.core.parser;
import folia.core.ast;
import folia.core.block_source;
import folia.core.inline_cst;
import folia.core.inline_document;
import folia.core.serializer;
import folia.core.utf;

#include "support/commonmark_projection.hpp"
using namespace folia;
using namespace boost::ut;

#include "support/parser_test_support.hpp"

suite commonmark_html_projection_tests = [] {

"official_image_html_uses_existing_cst_semantics"_test = [] {
    const std::string source = "*<img src=\"foo\" title=\"*\"/>\n"; // CommonMark #475
    const auto parsed = parse_text(1, source);
    const auto projection = folia::test::commonmark::project(parsed.document.root);
    expect(fatal(bool(folia::test::commonmark::inline_lossless(parsed.document.root))));
    expect(fatal(bool(serialize_markdown_cps(parsed.document) == utf8_to_cps(source))));
    expect(fatal(bool(projection.strict_tree
        == "[\"root\",[],[[\"p\",[],[\"*\",[\"img\",[[\"src\",\"foo\"],[\"title\",\"*\"]],[]]]]]]")));
    expect(fatal(bool(projection.limitations.empty())));
};

"official_comment_html_uses_delimiter_content_range"_test = [] {
    const std::string source = "foo <!-- this is a --\ncomment - with hyphens -->\n"; // CommonMark #625
    const auto parsed = parse_text(1, source);
    const auto projection = folia::test::commonmark::project(parsed.document.root);
    expect(fatal(bool(folia::test::commonmark::inline_lossless(parsed.document.root))));
    expect(fatal(bool(serialize_markdown_cps(parsed.document) == utf8_to_cps(source))));
    expect(fatal(bool(projection.strict_tree
        == "[\"root\",[],[[\"p\",[],[\"foo \",[\"comment\",[[\"text\",\" this is a --\\ncomment - with hyphens \"]],[]]]]]]")));
    expect(fatal(bool(projection.limitations.empty())));
};

"known_void_and_container_tags_project_attributes_and_html_text"_test = [] {
    const std::string source = "<span>*word*</span><br/><img src=\"x\" alt=\"*\">\n";
    const auto parsed = parse_text(1, source);
    const auto projection = folia::test::commonmark::project(parsed.document.root);
    expect(fatal(bool(projection.strict_tree
        == "[\"root\",[],[[\"p\",[],[[\"span\",[],[\"*word*\"]],[\"br\",[],[]],[\"img\",[[\"alt\",\"*\"],[\"src\",\"x\"]],[]]]]]]")));
};

"html_attribute_entity_stays_raw_and_is_reported_as_projection_limit"_test = [] {
    const std::string source = "<a href=\"&amp;\">x</a>\n";
    const auto parsed = parse_text(1, source);
    const auto projection = folia::test::commonmark::project(parsed.document.root);
    expect(fatal(bool(projection.strict_tree
        == "[\"root\",[],[[\"p\",[],[[\"a\",[[\"href\",\"&amp;\"]],[\"x\"]]]]]]")));
    expect(fatal(bool(std::ranges::find(
        projection.limitations, "inline-html-attribute-reference-not-decoded")
        != projection.limitations.end())));
};

"unknown_and_incomplete_html_do_not_project_as_recognized_elements"_test = [] {
    const auto unknown = parse_text(1, "<widget>x</widget>\n");
    expect(fatal(bool(unknown.document.root.children.size() == 1u)));
    if (unknown.document.root.children.empty()) return;
    expect(fatal(bool(first_inline(
        unknown.document.root.children.front().inline_content,
        InlineCstKind::Raw) != nullptr)));
    const auto unknown_projection = folia::test::commonmark::project(unknown.document.root);
    expect(fatal(bool(unknown_projection.strict_tree
        == "[\"root\",[],[[\"p\",[],[\"<widget>x</widget>\"]]]]")));
    expect(fatal(bool(unknown_projection.strict_tree
        != "[\"root\",[],[[\"p\",[],[[\"widget\",[],[\"x\"]]]]]]")));

    const auto incomplete = parse_text(1, "<span>unfinished\n");
    expect(fatal(bool(incomplete.document.root.children.size() == 1u)));
    if (incomplete.document.root.children.empty()) return;
    const auto* incomplete_node = first_inline(
        incomplete.document.root.children.front().inline_content,
        InlineCstKind::Incomplete);
    expect(fatal(bool(incomplete_node != nullptr)));
    if (incomplete_node) {
        expect(fatal(bool(incomplete_node->status == ParseStatus::MissingCloser)));
    }
    expect(fatal(bool(folia::test::commonmark::inline_lossless(incomplete.document.root))));
    const auto incomplete_projection = folia::test::commonmark::project(incomplete.document.root);
    expect(fatal(bool(incomplete_projection.strict_tree
        == "[\"root\",[],[[\"p\",[],[\"<span>unfinished\"]]]]")));
    expect(fatal(bool(incomplete_projection.strict_tree
        != "[\"root\",[],[[\"p\",[],[[\"span\",[],[\"unfinished\"]]]]]]")));
};

};
