#include "../support/folia_test.hpp"
import folia.math.util.bbox;
import folia.math.output.common;
#include "support/layout_source_table.hpp"

using namespace boost::ut;
using folia::math::BBox;
using namespace folia::math::output::common;
using folia::math::test::SourceTable;

namespace {
void check_font(folia::math::test::SourceRow const& row,
                RadicalFontParameters const& font, std::string_view prefix) {
    row.check(font.rule_thickness, std::string(prefix) + ".rule_thickness");
    row.check(font.surd_height, std::string(prefix) + ".surd_height");
    row.check(font.x_height, std::string(prefix) + ".x_height");
}

void check_point(folia::math::test::SourceRow const& row,
                 RadicalPoint const& point, std::string const& prefix) {
    row.check(point.x, prefix + ".x");
    row.check(point.y, prefix + ".y");
}

void check_rect(folia::math::test::SourceRow const& row,
                RadicalRect const& rect, std::string const& prefix) {
    row.check(rect.x, prefix + ".x");
    row.check(rect.y, prefix + ".y");
    row.check(rect.width, prefix + ".width");
    row.check(rect.height, prefix + ".height");
}
}

suite native_radical_source_pairs = [] {
    "all_generated_source_radical_pairs"_test = [] {
        SourceTable{"radical_source_pairs.txt", 192}.for_each([](auto const& row) {
            RadicalInput input{row.box("input.base"), std::nullopt,
                {row.number("input.font.rule_thickness"), row.number("input.font.surd_height"),
                 row.number("input.font.x_height")},
                row.boolean("input.display"), row.number("input.wrapperScale")};
            if (row.present("input.degree.w")) input.degree = row.box("input.degree");

            auto request = request_radical(input);
            row.check_box(request.base, "input.base");
            row.require(request.degree.has_value() == row.present("input.degree.w"), "request.degree");
            if (request.degree) row.check_box(*request.degree, "input.degree");
            check_font(row, request.font, "input.font");
            row.check(request.display, "input.display");
            row.check(request.wrapper_bbox_scale, "input.wrapperScale");
            row.check(request.required_surd_height, "expected.request.requiredSurdHeight");
            row.check(request.stretch_height, "expected.request.stretchHeight");
            row.check(request.stretch_depth, "expected.request.stretchDepth");
            row.check(request.stretch_exact, "expected.request.stretchExact");
            row.check(request.root_width, "expected.request.rootWidth");

            input.base.w = 999;
            input.font.rule_thickness = 999;
            input.wrapper_bbox_scale = 999;
            if (input.degree) input.degree->w = 999;
            row.check(request.base.w, "input.base.w");
            row.check(request.font.rule_thickness, "input.font.rule_thickness");
            row.check(request.wrapper_bbox_scale, "input.wrapperScale");
            if (request.degree) row.check(request.degree->w, "input.degree.w");

            auto layout = finish_radical(request, row.box("input.surd"),
                                         static_cast<int>(row.number("input.surdSize")));
            row.check_box(layout.bbox, "expected.bbox");
            row.check(layout.p, "expected.p");
            row.check(layout.q, "expected.q");
            row.check(layout.required_surd_height, "expected.request.requiredSurdHeight");
            row.check(layout.measured_height, "expected.measuredHeight");
            row.check(layout.placed_height, "expected.placedHeight");
            row.check(layout.placed_surd_height, "expected.placedSurdHeight");
            row.check(layout.root_offset, "expected.rootOffset");
            row.check(layout.root_height, "expected.rootHeight");
            row.check(layout.root_dx, "expected.rootDx");
            row.check(layout.bbox.scale, "expected.bboxReset.scale");
            row.check(layout.bbox.rscale, "expected.bboxReset.rscale");
            row.check(layout.bbox.L, "expected.bboxReset.L");
            row.check(layout.bbox.R, "expected.bboxReset.R");
            row.check(layout.bbox.ic, "expected.bboxReset.ic");
            row.check(layout.bbox.oc, "expected.bboxReset.oc");
            row.check(layout.bbox.sk, "expected.bboxReset.sk");
            row.check(layout.bbox.dx, "expected.bboxReset.dx");
            row.check(layout.bbox.pwidth, "expected.bboxReset.pwidth");
            check_rect(row, layout.overbar, "expected.overbar");

            bool saw_surd = false, saw_base = false, saw_degree = false;
            for (int i = 0; i < 3; ++i) {
                auto prefix = std::string("expected.placements.") + std::to_string(i);
                if (!row.present(prefix + ".role")) continue;
                auto role = row.text(prefix + ".role");
                if (role == "surd") { check_point(row, layout.surd_origin, prefix); saw_surd = true; }
                if (role == "base") { check_point(row, layout.base_origin, prefix); saw_base = true; }
                if (role == "degree") {
                    row.require(layout.degree_origin.has_value(), prefix + ".role");
                    if (layout.degree_origin) check_point(row, *layout.degree_origin, prefix);
                    saw_degree = true;
                }
            }
            row.require(saw_surd && saw_base, "expected.placements");
            row.require(layout.degree_origin.has_value() == saw_degree, "expected.degreeOrigin");
        });
    };
};
