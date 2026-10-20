#include "support/folia_test.hpp"
import folia.math.output.scene.builder;

namespace {
using namespace boost::ut;
using namespace folia::math::output::scene;

suite scene_freeze = [] {
    "scene freezes owned hierarchy and metadata"_test = [] {
        Builder builder;
        builder.set_metrics({2.5, 0.8, 0.2});
        builder.set_font_metrics({1.0, 0.43, 0.71, 0.68});
        SemanticMetadata root_meta;
        root_meta.source_tag = "math";
        root_meta.math_role = "inline-expression";
        root_meta.ordered_attributes.push_back({"display", "inline"});
        auto const parent = builder.add_group(
            0, Affine{1, 0, 0, 1, 2, 3},
            Clip{PathGeometry{{MoveTo{{0, 0}}, LineTo{{1, 0}},
                               ArcTo{0.5, 0.25, 30, true, false, {1, 1}}, ClosePath{}}}},
            Link{"https://example.test/", "", ""}, Tooltip{"formula"}, std::move(root_meta));
        SemanticMetadata child_meta;
        child_meta.source_tag = "mi";
        auto const child =
            builder.add_group(parent, Affine{0, 1, -1, 0, 4, 5}, {}, {}, {}, std::move(child_meta));
        builder.add_primitive(child, TextRun{"x", {0, 0}, {0.6, 0.7, 0.1}, {}, {}});

        auto scene = std::move(builder).freeze();
        expect(scene.metrics() == Metrics{2.5, 0.8, 0.2});
        expect(scene.font_metrics() == FontMetrics{1.0, 0.43, 0.71, 0.68});
        expect(scene.root().entries.size() == 1_u);
        auto const &parent_ptr =
            std::get<std::shared_ptr<Group const>>(scene.root().entries.front());
        expect(parent_ptr->id == parent);
        expect(parent_ptr->clip.has_value());
        expect(std::holds_alternative<ArcTo>(parent_ptr->clip->geometry.segments[2]));
        expect(parent_ptr->link->href == "https://example.test/");
        expect(parent_ptr->tooltip->text == "formula");
        expect(parent_ptr->metadata.math_role == "inline-expression");
        expect(parent_ptr->metadata.ordered_attributes.front().name == "display");
        expect(parent_ptr->entries.size() == 1_u);
        auto const &child_ptr = std::get<std::shared_ptr<Group const>>(parent_ptr->entries.front());
        expect(child_ptr->id == child);
        expect(child_ptr->metadata.source_tag == "mi");
        expect(child_ptr->entries.size() == 1_u);
        expect(std::holds_alternative<Primitive>(child_ptr->entries.front()));
        expect(std::holds_alternative<TextRun>(std::get<Primitive>(child_ptr->entries.front())));
    };

    "affine composition follows parent times local"_test = [] {
        auto const result = compose(Affine{1, 0, 0, 1, 2, 3}, Affine{0, 1, -1, 0, 4, 5});
        expect(result == Affine{0, 1, -1, 0, 6, 8});
        auto const point = Point{1, 2};
        expect(result.a * point.x + result.c * point.y + result.e == 4.0_d);
        expect(result.b * point.x + result.d * point.y + result.f == 9.0_d);
    };

    "ordered transforms preserve operations and derive SVG-order matrix"_test = [] {
        Transform identity;
        expect(identity.operations().empty());
        expect(identity.matrix() == Affine{});
        Transform transform{std::vector<TransformOperation>{
            Translation{3, 4}, Scaling{2, 3}, Rotation{90, {1, 2}}, Affine{1, 0, 0, 1, -2, 1}}};
        auto const operations = transform.operations();
        expect(operations.size() == 4_u);
        expect(std::holds_alternative<Translation>(operations[0]));
        expect(std::holds_alternative<Scaling>(operations[1]));
        expect(std::holds_alternative<Rotation>(operations[2]));
        expect(std::holds_alternative<Affine>(operations[3]));

        auto const translation =
            Transform{std::vector<TransformOperation>{Translation{3, 4}}}.matrix();
        auto const scaling = Transform{std::vector<TransformOperation>{Scaling{2, 3}}}.matrix();
        auto const rotation =
            Transform{std::vector<TransformOperation>{Rotation{90, {1, 2}}}}.matrix();
        auto expected = compose(Affine{}, translation);
        expected = compose(expected, scaling);
        expected = compose(expected, rotation);
        expected = compose(expected, Affine{1, 0, 0, 1, -2, 1});
        expect(transform.matrix() == expected);

        // SVG list order gives T*S, so the point is scaled then translated.
        Transform ordered{std::vector<TransformOperation>{Translation{3, 4}, Scaling{2, 3}}};
        auto const result = ordered.matrix();
        expect(result.a * 1 + result.c * 1 + result.e == 5.0_d);
        expect(result.b * 1 + result.d * 1 + result.f == 7.0_d);

        Transform reflection{std::vector<TransformOperation>{Scaling{-2, 1}}};
        auto const reflected = reflection.matrix();
        expect(reflected.a * 2 + reflected.c * 3 + reflected.e == -4.0_d);
        expect(reflected.b * 2 + reflected.d * 3 + reflected.f == 3.0_d);
    };

    "center rotation is counterclockwise in y-up scene coordinates"_test = [] {
        Transform rotation{std::vector<TransformOperation>{Rotation{90, {1, 2}}}};
        auto const matrix = rotation.matrix();
        Point const source{2, 2};
        auto const x = matrix.a * source.x + matrix.c * source.y + matrix.e;
        auto const y = matrix.b * source.x + matrix.d * source.y + matrix.f;
        expect(std::abs(x - 1) < 1e-12);
        expect(std::abs(y - 3) < 1e-12);
    };

    "affine convenience creates one explicit operation"_test = [] {
        Affine const affine{1, 0, 0, 1, 2, 3};
        Transform transform = affine;
        expect(transform.operations().size() == 1_u);
        expect(std::holds_alternative<Affine>(transform.operations().front()));
        expect(transform.matrix() == affine);
    };

    "scene rejects non-finite transform data at freeze"_test = [] {
        auto const infinity = std::numeric_limits<Unit>::infinity();
        auto rejects = [infinity](Transform transform) {
            Builder builder;
            (void)builder.add_group(0, std::move(transform));
            try {
                [[maybe_unused]] auto scene = std::move(builder).freeze();
            } catch (std::invalid_argument const &) {
                return true;
            }
            return false;
        };
        expect(rejects(Transform{std::vector<TransformOperation>{Translation{infinity, 0}}}));
        expect(rejects(Transform{std::vector<TransformOperation>{Scaling{1, infinity}}}));
        expect(rejects(Transform{std::vector<TransformOperation>{Rotation{infinity, {0, 0}}}}));
        expect(rejects(Transform{std::vector<TransformOperation>{Rotation{0, {0, infinity}}}}));
        expect(rejects(Transform{Affine{infinity, 0, 0, 1, 0, 0}}));
    };

    "scene rejects a glyph without an immutable outline resource"_test = [] {
        Builder builder;
        builder.add_primitive(0, GlyphRun{{Glyph{}}});
        bool rejected = false;
        try {
            [[maybe_unused]] auto scene = std::move(builder).freeze();
        } catch (std::invalid_argument const &) {
            rejected = true;
        }
        expect(rejected);
    };

    "scene retains immutable glyph resource beyond builder lifetime"_test = [] {
        std::weak_ptr<GlyphResource const> weak;
        std::shared_ptr<GlyphOutline> caller_outline;
        Scene scene = [&] {
            Builder builder;
            caller_outline = std::make_shared<GlyphOutline>(
                GlyphOutline{PathGeometry{{MoveTo{{0, 0}}, LineTo{{0.5, 1}}, ClosePath{}}}});
            auto resource = GlyphResource::make("NewCM", *caller_outline);
            weak = resource;
            Glyph glyph{resource, GlyphKey{"normal", "NCM-N", 0x78}, {0.1, 0}, -0.55, {}};
            builder.add_primitive(0, GlyphRun{{std::move(glyph)}});
            return std::move(builder).freeze();
        }();
        expect(!weak.expired());
        caller_outline->outline.segments[0] = MoveTo{{99, 99}};
        auto const &run = std::get<GlyphRun>(std::get<Primitive>(scene.root().entries.front()));
        expect(run.glyphs.front().resource->font_id() == "NewCM");
        expect(run.glyphs.front().key == GlyphKey{"normal", "NCM-N", 0x78});
        expect(run.glyphs.front().advance == -0.55_d);
        expect(run.glyphs.front().resource->outline().outline.segments.size() == 3_u);
        auto const &first =
            std::get<MoveTo>(run.glyphs.front().resource->outline().outline.segments.front());
        expect(first.to == Point{0, 0});
    };

    "foreign HTML keeps owned measured runs and semantic metadata"_test = [] {
        Builder builder;
        SemanticMetadata metadata;
        metadata.source_tag = "annotation-xml";
        metadata.annotation_encoding = "text/html";
        metadata.annotation = "<b>owned annotation</b>";
        ForeignContent foreign{"<span>result</span>",
                               {0, -0.7},
                               {ForeignRun{TextRun{"result", {0, 0}, {2.0, 0.7, 0.1}, {}, {}}}},
                               {2.0, 0.7, 0.1},
                               std::move(metadata)};
        builder.add_primitive(0, std::move(foreign));
        auto scene = std::move(builder).freeze();
        auto const &saved =
            std::get<ForeignContent>(std::get<Primitive>(scene.root().entries.front()));
        expect(saved.content == "<span>result</span>");
        expect(saved.measured_runs.front().run.utf8 == "result");
        expect(saved.metadata.annotation_encoding == "text/html");
        expect(saved.metrics.width == 2.0_d);
    };

    "interleaved primitives and groups keep insertion order"_test = [] {
        Builder builder;
        builder.add_primitive(0, TextRun{"before", {}, {}, {}, {}});
        (void)builder.add_group(0);
        builder.add_primitive(0, TextRun{"after", {}, {}, {}, {}});
        auto scene = std::move(builder).freeze();
        auto const &entries = scene.root().entries;
        expect(entries.size() == 3_u);
        expect(std::get<TextRun>(std::get<Primitive>(entries[0])).utf8 == "before");
        expect(std::holds_alternative<std::shared_ptr<Group const>>(entries[1]));
        expect(std::get<TextRun>(std::get<Primitive>(entries[2])).utf8 == "after");
    };

    "paint preserves inherited none currentColor and literal color"_test = [] {
        Paint paint;
        expect(std::holds_alternative<InheritedPaint>(paint.fill));
        paint.fill = NoPaint{};
        paint.stroke = CurrentColorPaint{};
        expect(std::holds_alternative<NoPaint>(paint.fill));
        expect(std::holds_alternative<CurrentColorPaint>(paint.stroke));
        paint.fill = Color{12, 34, 56, 255};
        expect(std::get<Color>(paint.fill) == Color{12, 34, 56, 255});
    };

    "scene preserves signed completed mathematical metrics"_test = [] {
        Builder builder;
        builder.set_metrics({-0.5, -0.2, 0.3});
        builder.add_primitive(0, TextRun{"negative bbox", {}, {-0.4, -0.1, 0.2}, {}, {}});
        auto scene = std::move(builder).freeze();
        expect(scene.metrics() == Metrics{-0.5, -0.2, 0.3});
        auto const &text = std::get<TextRun>(std::get<Primitive>(scene.root().entries.front()));
        expect(text.metrics.width == -0.4_d);
        expect(text.metrics.height == -0.1_d);
    };

    "scene snapshots retained mutable group aliases"_test = [] {
        Group root;
        auto child = std::make_shared<Group>();
        child->id = 1;
        child->transform = Transform{Affine{1, 0, 0, 1, 2, 0}};
        child->entries.emplace_back(Primitive{TextRun{"before", {}, {}, {}, {}}});
        root.entries.emplace_back(std::shared_ptr<Group const>{child});
        Scene scene{root, Metrics{1, 1, 0}, FontMetrics{}};

        child->transform = Transform{Affine{1, 0, 0, 1, 99, 0}};
        child->entries.clear();
        auto const &frozen = std::get<std::shared_ptr<Group const>>(scene.root().entries.front());
        expect(frozen->transform.matrix().e == 2.0_d);
        expect(frozen->entries.size() == 1_u);
        expect(std::get<TextRun>(std::get<Primitive>(frozen->entries.front())).utf8 == "before");
    };

    "builder edits existing group values and wraps contents in place"_test = [] {
        Builder builder;
        auto id = builder.add_group();
        builder.add_primitive(id, TextRun{"a", {}, {}, {}, {}});
        builder.add_primitive(id, TextRun{"b", {}, {}, {}, {}});
        builder.set_transform(id, Transform{std::vector<TransformOperation>{Translation{2, 3}}});
        SemanticMetadata metadata;
        metadata.math_role = "row";
        builder.set_metadata(id, metadata);
        Paint paint;
        paint.opacity = 0.5;
        builder.set_paint(id, paint);
        expect(builder.transform(id).matrix().e == 2.0_d);
        expect(builder.metadata(id).math_role == "row");
        expect(builder.paint(id).opacity == 0.5_d);
        builder.insert_primitive(id, 1, TextRun{"middle", {}, {}, {}, {}});
        expect(builder.entry_count(id) == 3_u);
        auto wrapped =
            builder.wrap_contents(id, Transform{std::vector<TransformOperation>{Translation{0, 1}}},
                                  {}, ElementRole::anonymous_group);
        expect(builder.entry_count(id) == 1_u);
        expect(builder.entry_count(wrapped) == 3_u);
        auto scene = std::move(builder).freeze();
        auto const &group = *std::get<std::shared_ptr<Group const>>(scene.root().entries.front());
        expect(group.id == id);
        expect(group.entries.size() == 1_u);
        auto const &wrapper = *std::get<std::shared_ptr<Group const>>(group.entries.front());
        expect(wrapper.id == wrapped);
        expect(wrapper.entries.size() == 3_u);
        expect(std::get<TextRun>(std::get<Primitive>(wrapper.entries[1])).utf8 == "middle");
    };

    "builder reports moved from and invalid ids or insertion positions"_test = [] {
        Builder builder;
        bool invalid_id = false, invalid_index = false;
        try {
            (void)builder.entry_count(999);
        } catch (std::out_of_range const &) {
            invalid_id = true;
        }
        try {
            builder.insert_primitive(0, 1, TextRun{});
        } catch (std::out_of_range const &) {
            invalid_index = true;
        }
        expect(invalid_id && invalid_index);
        Builder moved{std::move(builder)};
        bool moved_from = false;
        try {
            (void)builder.display_layout();
        } catch (std::logic_error const &) {
            moved_from = true;
        }
        expect(moved_from);
        expect(moved.display_layout().alignment == "center");
    };

    "display and text font values are frozen and validated"_test = [] {
        Builder builder;
        builder.set_display_layout({true, "left", 0.25, 4.0, true});
        TextRun run{"x", {}, {}, {}, {}};
        run.font = TextFont{"MathJax_Main", 1.25, true, false, "normal"};
        builder.add_primitive(0, run);
        auto scene = std::move(builder).freeze();
        expect(scene.display_layout().display);
        expect(scene.display_layout().alignment == "left");
        expect(std::get<TextRun>(std::get<Primitive>(scene.root().entries.front())).font->family ==
               "MathJax_Main");
        Group root;
        TextRun invalid{};
        invalid.font = TextFont{"x", std::numeric_limits<Unit>::infinity()};
        root.entries.emplace_back(Primitive{invalid});
        bool rejected = false;
        try {
            Scene bad{root, {}, {}};
        } catch (std::invalid_argument const &) {
            rejected = true;
        }
        expect(rejected);
    };

    "scene rejects a cyclic shared group before recursive copying"_test = [] {
        Group root;
        auto cycle = std::make_shared<Group>();
        cycle->id = 1;
        cycle->entries.emplace_back(std::shared_ptr<Group const>{cycle});
        root.entries.emplace_back(std::shared_ptr<Group const>{cycle});
        bool threw = false;
        try {
            Scene scene{std::move(root), Metrics{}, FontMetrics{}};
        } catch (std::invalid_argument const &) {
            threw = true;
        }
        expect(threw);
        cycle->entries.clear(); // break the deliberately constructed shared_ptr cycle
    };
};
} // namespace
