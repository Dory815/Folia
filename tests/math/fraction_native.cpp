#include "../support/folia_test.hpp"
import folia.math.util.bbox;
import folia.math.output.common;
#include "support/layout_source_table.hpp"

using namespace boost::ut;
using folia::math::BBox;
using namespace folia::math::output::common;
using folia::math::test::SourceRow;
using folia::math::test::SourceTable;
namespace {
FractionFontParameters font(SourceRow const& row) {
    return {row.number("input.font.axis_height"), row.number("input.font.num1"),
        row.number("input.font.num2"), row.number("input.font.num3"),
        row.number("input.font.denom1"), row.number("input.font.denom2"),
        row.number("input.font.rule_thickness"), row.number("input.font.nulldelimiterspace")};
}
FractionAlignment alignment(std::string value) {
    if (value == "left") return FractionAlignment::left;
    if (value == "right") return FractionAlignment::right;
    if (value == "center") return FractionAlignment::center;
    throw std::runtime_error("Unknown Source fraction alignment " + value);
}
FractionKind kind(std::string value) {
    if (value == "ruled") return FractionKind::ruled;
    if (value == "atop") return FractionKind::atop;
    if (value == "bevelled") return FractionKind::bevelled;
    throw std::runtime_error("Unknown Source fraction kind " + value);
}
void check_point(SourceRow const& row, FractionPoint point, std::string prefix) {
    row.check(point.x, prefix + ".x"); row.check(point.y, prefix + ".y");
}
void check_font(SourceRow const& row, FractionFontParameters const& value, std::string prefix) {
    row.check(value.axis_height, prefix + ".axis_height"); row.check(value.num1, prefix + ".num1");
    row.check(value.num2, prefix + ".num2"); row.check(value.num3, prefix + ".num3");
    row.check(value.denom1, prefix + ".denom1"); row.check(value.denom2, prefix + ".denom2");
    row.check(value.rule_thickness, prefix + ".rule_thickness");
    row.check(value.nulldelimiterspace, prefix + ".nulldelimiterspace");
}
void check_layout(SourceRow const& row, FractionLayout const& actual) {
    const bool bevelled = row.text("input.kind") == "bevelled";
    row.require(actual.kind == kind(row.text("input.kind")), "input.kind");
    row.check_box(actual.bbox, "expected.bbox");
    check_point(row, actual.numerator_origin, "expected.svg.0");
    check_point(row, actual.denominator_origin, bevelled ? "expected.svg.2" : "expected.svg.1");
    row.require(actual.slash_origin.has_value() == bevelled, "expected.svg.1.child");
    if (actual.slash_origin) check_point(row, *actual.slash_origin, "expected.svg.1");
    row.require(actual.bbox_denominator_origin.has_value() == row.present("expected.bboxDenominatorOrigin.x"), "expected.bboxDenominatorOrigin.x");
    if (actual.bbox_denominator_origin) check_point(row, *actual.bbox_denominator_origin, "expected.bboxDenominatorOrigin");
    const bool has_rule = row.present("expected.svg.2.rule.width");
    row.require(actual.rule.has_value() == has_rule, "expected.svg.2.rule.width");
    if (actual.rule) {
        row.check(actual.rule->x, "expected.svg.2.rule.x"); row.check(actual.rule->y, "expected.svg.2.rule.y");
        row.check(actual.rule->width, "expected.svg.2.rule.width"); row.check(actual.rule->height, "expected.svg.2.rule.height");
    }
    row.require(actual.content_width.has_value() == row.present("expected.contentWidth"), "expected.contentWidth");
    if (actual.content_width) row.check(*actual.content_width, "expected.contentWidth");
    row.check(actual.numerator_wrap_width, "expected.wrapWidths.0");
    row.check(actual.denominator_wrap_width, "expected.wrapWidths.1");
}
void check_request(SourceRow const& row, BevelRequest const& request) {
    row.check(request.required_slash_height, "expected.H"); row.check(request.delta, "expected.delta");
    row.check(request.numerator_y, "expected.u"); row.check(request.denominator_y, "expected.v");
    row.check(request.display, "input.display");
    row.check_box(request.numerator, "input.numerator"); row.check_box(request.denominator, "input.denominator");
    check_font(row, request.font, "input.font");
}
void run_row(SourceRow const& row) {
    FractionInput input{row.box("input.numerator"), row.box("input.denominator"), font(row),
        row.boolean("input.display"), row.boolean("input.withDelims"), row.number("input.lineThickness"),
        alignment(row.text("input.align.0")), alignment(row.text("input.align.1"))};
    const auto result = measure_fraction(input, kind(row.text("input.kind")));
    if (row.text("input.kind") != "bevelled") {
        row.require(std::holds_alternative<FractionLayout>(result), "expected.kind");
        if (const auto* layout = std::get_if<FractionLayout>(&result)) check_layout(row, *layout);
        return;
    }
    if (!row.require(std::holds_alternative<BevelRequest>(result), "expected.H")) return;
    const auto& request = std::get<BevelRequest>(result);
    input.numerator.w = 999;
    input.denominator.h = 999;
    input.font.rule_thickness = 999;
    input.display = !input.display;
    check_request(row, request);
    const auto layout = finish_bevelled_fraction(request, row.box("expected.slash"));
    check_layout(row, layout);
}
}
suite native_fraction_source_pairs = [] {
    "all_generated_source_fraction_pairs"_test = [] {
        SourceTable{"fraction_source_pairs.txt", 288}.for_each(run_row);
    };
};
