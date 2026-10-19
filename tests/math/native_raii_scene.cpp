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
        auto const parent = builder.add_group(0, Affine{1,0,0,1,2,3},
            Clip{PathGeometry{{MoveTo{{0,0}}, LineTo{{1,0}}, ArcTo{0.5,0.25,30,true,false,{1,1}}, ClosePath{}}}},
            Link{"https://example.test/", "", ""}, Tooltip{"formula"}, std::move(root_meta));
        SemanticMetadata child_meta;
        child_meta.source_tag = "mi";
        auto const child = builder.add_group(parent, Affine{0,1,-1,0,4,5}, {}, {}, {}, std::move(child_meta));
        builder.add_primitive(child, TextRun{"x", {0,0}, {0.6,0.7,0.1}, {}, {}});

        auto scene = std::move(builder).freeze();
        expect(scene.metrics() == Metrics{2.5,0.8,0.2});
        expect(scene.font_metrics() == FontMetrics{1.0,0.43,0.71,0.68});
        expect(scene.root().entries.size() == 1_u);
        auto const& parent_ptr = std::get<std::shared_ptr<Group const>>(scene.root().entries.front());
        expect(parent_ptr->id == parent);
        expect(parent_ptr->clip.has_value());
        expect(std::holds_alternative<ArcTo>(parent_ptr->clip->geometry.segments[2]));
        expect(parent_ptr->link->href == "https://example.test/");
        expect(parent_ptr->tooltip->text == "formula");
        expect(parent_ptr->metadata.math_role == "inline-expression");
        expect(parent_ptr->metadata.ordered_attributes.front().name == "display");
        expect(parent_ptr->entries.size() == 1_u);
        auto const& child_ptr = std::get<std::shared_ptr<Group const>>(parent_ptr->entries.front());
        expect(child_ptr->id == child);
        expect(child_ptr->metadata.source_tag == "mi");
        expect(child_ptr->entries.size() == 1_u);
        expect(std::holds_alternative<Primitive>(child_ptr->entries.front()));
        expect(std::holds_alternative<TextRun>(std::get<Primitive>(child_ptr->entries.front())));
    };

    "affine composition follows parent times local"_test = [] {
        auto const result = compose(Affine{1,0,0,1,2,3}, Affine{0,1,-1,0,4,5});
        expect(result == Affine{0,1,-1,0,6,8});
        auto const point = Point{1,2};
        expect(result.a*point.x + result.c*point.y + result.e == 4.0_d);
        expect(result.b*point.x + result.d*point.y + result.f == 9.0_d);
    };

    "scene retains immutable glyph resource beyond builder lifetime"_test = [] {
        std::weak_ptr<GlyphResource const> weak;
        std::shared_ptr<GlyphOutline> caller_outline;
        Scene scene = [&] {
            Builder builder;
            caller_outline = std::make_shared<GlyphOutline>(GlyphOutline{PathGeometry{{MoveTo{{0,0}}, LineTo{{0.5,1}}, ClosePath{}}}});
            auto resource = GlyphResource::make("NewCM", *caller_outline);
            weak = resource;
            Glyph glyph{resource, 42, {0.1,0}, -0.55, {}};
            builder.add_primitive(0, GlyphRun{{std::move(glyph)}});
            return std::move(builder).freeze();
        }();
        expect(!weak.expired());
        caller_outline->outline.segments[0] = MoveTo{{99,99}};
        auto const& run = std::get<GlyphRun>(std::get<Primitive>(scene.root().entries.front()));
        expect(run.glyphs.front().resource->font_id() == "NewCM");
        expect(run.glyphs.front().advance == -0.55_d);
        expect(run.glyphs.front().resource->outline().outline.segments.size() == 3_u);
        auto const& first = std::get<MoveTo>(run.glyphs.front().resource->outline().outline.segments.front());
        expect(first.to == Point{0,0});
    };

    "foreign HTML keeps owned measured runs and semantic metadata"_test = [] {
        Builder builder;
        SemanticMetadata metadata;
        metadata.source_tag = "annotation-xml";
        metadata.annotation_encoding = "text/html";
        metadata.annotation = "<b>owned annotation</b>";
        ForeignContent foreign{"<span>result</span>", {0,-0.7},
            {ForeignRun{TextRun{"result", {0,0}, {2.0,0.7,0.1}, {}, {}}}},
            {2.0,0.7,0.1}, std::move(metadata)};
        builder.add_primitive(0, std::move(foreign));
        auto scene = std::move(builder).freeze();
        auto const& saved = std::get<ForeignContent>(std::get<Primitive>(scene.root().entries.front()));
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
        auto const& entries = scene.root().entries;
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
        paint.fill = Color{12,34,56,255};
        expect(std::get<Color>(paint.fill) == Color{12,34,56,255});
    };

    "scene preserves signed completed mathematical metrics"_test = [] {
        Builder builder;
        builder.set_metrics({-0.5,-0.2,0.3});
        builder.add_primitive(0, TextRun{"negative bbox", {}, {-0.4,-0.1,0.2}, {}, {}});
        auto scene = std::move(builder).freeze();
        expect(scene.metrics() == Metrics{-0.5,-0.2,0.3});
        auto const& text = std::get<TextRun>(std::get<Primitive>(scene.root().entries.front()));
        expect(text.metrics.width == -0.4_d);
        expect(text.metrics.height == -0.1_d);
    };

    "scene snapshots retained mutable group aliases"_test = [] {
        Group root;
        auto child = std::make_shared<Group>();
        child->id = 1;
        child->transform.e = 2;
        child->entries.emplace_back(Primitive{TextRun{"before", {}, {}, {}, {}}});
        root.entries.emplace_back(std::shared_ptr<Group const>{child});
        Scene scene{root, Metrics{1,1,0}, FontMetrics{}};

        child->transform.e = 99;
        child->entries.clear();
        auto const& frozen = std::get<std::shared_ptr<Group const>>(scene.root().entries.front());
        expect(frozen->transform.e == 2.0_d);
        expect(frozen->entries.size() == 1_u);
        expect(std::get<TextRun>(std::get<Primitive>(frozen->entries.front())).utf8 == "before");
    };

    "scene rejects a cyclic shared group before recursive copying"_test = [] {
        Group root;
        auto cycle = std::make_shared<Group>();
        cycle->id = 1;
        cycle->entries.emplace_back(std::shared_ptr<Group const>{cycle});
        root.entries.emplace_back(std::shared_ptr<Group const>{cycle});
        bool threw = false;
        try { Scene scene{std::move(root), Metrics{}, FontMetrics{}}; }
        catch (std::invalid_argument const&) { threw = true; }
        expect(threw);
        cycle->entries.clear(); // break the deliberately constructed shared_ptr cycle
    };
};
}
