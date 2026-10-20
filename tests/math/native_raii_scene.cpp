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

    "viewport values are validated and deeply frozen with groups and glyphs"_test = [] {
        Viewport viewport{{0.1, -0.2, 0.8, 0.7},
                          {-0.3, -0.4, 1.2, 1.1},
                          ViewportAspectRatio{ViewportAlignment::max, ViewportAlignment::min,
                                              ViewportFit::slice}};
        Group root;
        auto child = std::make_shared<Group>();
        child->id = 1;
        child->viewport = viewport;
        root.entries.emplace_back(std::shared_ptr<Group const>{child});
        Scene scene{root, {1, 0.5, 0.1}, {}};
        child->viewport->bounds.x = 99;
        child->viewport->aspect_ratio->fit = ViewportFit::none;
        auto const &frozen = *std::get<std::shared_ptr<Group const>>(scene.root().entries.front());
        expect(frozen.viewport->bounds.x == 0.1_d);
        expect(frozen.viewport->aspect_ratio->fit == ViewportFit::slice);

        auto rejects_viewport = [](Viewport value) {
            Group invalid_root;
            auto invalid = std::make_shared<Group>();
            invalid->id = 1;
            invalid->viewport = std::move(value);
            invalid_root.entries.emplace_back(std::shared_ptr<Group const>{invalid});
            try {
                Scene rejected{invalid_root, {}, {}};
            } catch (std::invalid_argument const &) {
                return true;
            }
            return false;
        };
        auto invalid = viewport;
        invalid.bounds.width = -1;
        auto invalid_view_box = viewport;
        invalid_view_box.view_box.height = 0;
        auto invalid_aspect = viewport;
        invalid_aspect.aspect_ratio->x = static_cast<ViewportAlignment>(99);
        expect(rejects_viewport(invalid));
        expect(rejects_viewport(invalid_view_box));
        expect(rejects_viewport(invalid_aspect));

        auto resource = GlyphResource::make(
            "font", GlyphOutline{PathGeometry{{MoveTo{{0, 0}}, LineTo{{1, 0}}, ClosePath{}}}});
        Glyph glyph{resource, {"normal", "font", 65},
                    {},       1,
                    {},       Transform{std::vector<TransformOperation>{Scaling{2, 3}}}};
        Group glyph_root;
        glyph_root.entries.emplace_back(Primitive{GlyphRun{{glyph}}});
        Scene glyph_scene{glyph_root, {}, {}};
        glyph.transform = Transform{
            std::vector<TransformOperation>{Translation{std::numeric_limits<Unit>::infinity(), 0}}};
        auto const &saved_glyph =
            std::get<GlyphRun>(std::get<Primitive>(glyph_scene.root().entries.front()))
                .glyphs.front();
        expect(saved_glyph.transform.matrix() ==
               Transform{std::vector<TransformOperation>{Scaling{2, 3}}}.matrix());
        auto invalid_glyph = glyph;
        invalid_glyph.transform = Transform{
            std::vector<TransformOperation>{Translation{std::numeric_limits<Unit>::infinity(), 0}}};
        Group invalid_glyph_root;
        invalid_glyph_root.entries.emplace_back(Primitive{GlyphRun{{invalid_glyph}}});
        bool rejected_glyph = false;
        try {
            Scene rejected{invalid_glyph_root, {}, {}};
        } catch (std::invalid_argument const &) {
            rejected_glyph = true;
        }
        expect(rejected_glyph);

        Builder builder;
        auto id = builder.add_group(0, {}, {}, {}, {}, {}, ElementRole::math_wrapper, {}, viewport);
        expect(builder.child_groups(0) == std::vector<NodeId>{id});
        auto frozen_builder = std::move(builder).freeze();
        expect(std::get<std::shared_ptr<Group const>>(frozen_builder.root().entries.front())
                   ->viewport->view_box == viewport.view_box);
    };

    "table builder geometry edits stay typed and reparenting validates ancestry"_test = [] {
        Builder background_builder;
        auto background = background_builder.add_group();
        SemanticMetadata background_metadata;
        background_metadata.ordered_attributes.push_back({"data-bgcolor", "red"});
        Rect const original{0.1, -0.2, 1.3, 0.4};
        background_builder.add_primitive(
            background, Rule{original, {}, background_metadata, ElementRole::background_box});
        expect(background_builder.background_bounds(background) == original);
        Rect const adjusted{0.2, -0.3, 1.4, 0.5};
        expect(background_builder.set_background_bounds(background, adjusted));
        expect(background_builder.set_background_width(background, 2.25));
        auto resized = adjusted;
        resized.width = 2.25;
        expect(background_builder.background_bounds(background) == resized);
        auto no_background = background_builder.add_group();
        expect(!background_builder.set_background_bounds(no_background, adjusted));
        bool invalid_bounds = false;
        try {
            (void)background_builder.set_background_bounds(background, Rect{0, 0, -1, 1});
        } catch (std::invalid_argument const &) {
            invalid_bounds = true;
        }
        expect(invalid_bounds);

        Builder width_builder;
        auto width_group = width_builder.add_group();
        Path width_path;
        width_path.role = ElementRole::background_box;
        width_builder.add_primitive(width_group, width_path);
        expect(width_builder.set_background_width(width_group, 3.049));
        auto non_background_first = width_builder.add_group();
        width_builder.add_primitive(non_background_first, Path{});
        Path later_background;
        later_background.role = ElementRole::background_box;
        width_builder.add_primitive(non_background_first, later_background);
        expect(!width_builder.set_background_width(non_background_first, 9));
        bool negative_width = false;
        try {
            (void)width_builder.set_background_width(width_group, -1);
        } catch (std::invalid_argument const &) {
            negative_width = true;
        }
        bool nonfinite_width = false;
        try {
            (void)width_builder.set_background_width(
                width_group, std::numeric_limits<Unit>::quiet_NaN());
        } catch (std::invalid_argument const &) {
            nonfinite_width = true;
        }
        expect(negative_width && nonfinite_width);
        auto frozen_width_scene = std::move(width_builder).freeze();
        auto const &frozen_width_group =
            *std::get<std::shared_ptr<Group const>>(frozen_width_scene.root().entries.front());
        expect(std::get<Path>(std::get<Primitive>(frozen_width_group.entries.front())).layout_width ==
               3.049);

        Path owned_width_path;
        owned_width_path.layout_width = 2.75;
        Group owned_width_root;
        owned_width_root.entries.emplace_back(Primitive{owned_width_path});
        Scene owned_width_scene{owned_width_root, {}, {}};
        std::get<Path>(std::get<Primitive>(owned_width_root.entries.front())).layout_width = 8;
        expect(std::get<Path>(std::get<Primitive>(owned_width_scene.root().entries.front()))
                   .layout_width == 2.75);
        auto rejects_layout_width = [](Unit invalid_width) {
            Path invalid_path;
            invalid_path.layout_width = invalid_width;
            Group invalid_root;
            invalid_root.entries.emplace_back(Primitive{std::move(invalid_path)});
            try {
                Scene invalid_scene{std::move(invalid_root), {}, {}};
            } catch (std::invalid_argument const &) {
                return true;
            }
            return false;
        };
        expect(rejects_layout_width(-1));
        expect(rejects_layout_width(std::numeric_limits<Unit>::quiet_NaN()));

        Builder builder;
        auto owner = builder.add_group();
        auto child = builder.add_group(owner);
        auto destination = builder.add_group();
        builder.add_primitive(child, TextRun{"owned"});
        auto viewport =
            builder.add_group(0, {}, {}, {}, {}, {}, ElementRole::table_content_viewport, {},
                              Viewport{{0, 0, 1, 1}, {0, 0, 1, 1}, {}, false});
        Rect const completed{0.4, -0.7, 2.2, 1.1};
        builder.set_viewport_bounds(ElementRole::table_content_viewport, completed);
        builder.reparent_group(child, destination);
        expect(builder.child_groups(owner).empty());
        expect(builder.child_groups(destination) == std::vector<NodeId>{child});
        bool cycle = false;
        try {
            builder.reparent_group(destination, child);
        } catch (std::invalid_argument const &) {
            cycle = true;
        }
        bool root = false;
        try {
            builder.reparent_group(0, destination);
        } catch (std::invalid_argument const &) {
            root = true;
        }
        bool missing = false;
        try {
            builder.reparent_group(999, destination);
        } catch (std::out_of_range const &) {
            missing = true;
        }
        expect(cycle && root && missing);
        auto frozen = std::move(builder).freeze();
        auto const find_group = [&](auto const &self, Group const &group,
                                    NodeId target) -> Group const * {
            if (group.id == target)
                return &group;
            for (auto const &entry : group.entries)
                if (auto const *nested = std::get_if<std::shared_ptr<Group const>>(&entry);
                    nested && *nested)
                    if (auto const *found = self(self, **nested, target))
                        return found;
            return nullptr;
        };
        auto const *viewport_group = find_group(find_group, frozen.root(), viewport);
        auto const *saved_child = find_group(find_group, frozen.root(), child);
        expect(viewport_group && viewport_group->viewport->bounds == completed);
        expect(viewport_group && !viewport_group->viewport->clip_content);
        expect(saved_child &&
               std::get<TextRun>(std::get<Primitive>(saved_child->entries.front())).utf8 ==
                   "owned");
    };

    "percentage frame values are owned and validated"_test = [] {
        auto rejects = [](FrameLayout layout) {
            try {
                Scene invalid{Group{}, {}, {}, {}, std::move(layout)};
            } catch (std::invalid_argument const &) {
                return true;
            }
            return false;
        };
        FrameLayout valid{0.001, {}, PercentageFrame{80, "80%", {2, 0, 4, 1}, 0.375}};
        Scene saved{Group{}, {}, {}, {}, valid};
        valid.percentage->width_text = "mutated";
        expect(saved.frame_layout().percentage->width_text == "80%");
        auto bad_percent = FrameLayout{0.001, {}, PercentageFrame{-1, "80%", {2, 0, 4, 1}, 1}};
        auto nonfinite_percent = FrameLayout{
            0.001,
            {},
            PercentageFrame{std::numeric_limits<Unit>::infinity(), "80%", {2, 0, 4, 1}, 1}};
        auto bad_scale = FrameLayout{0.001, {}, PercentageFrame{80, "80%", {2, 0, 4, 1}, 0}};
        auto bad_bounds = FrameLayout{0.001, {}, PercentageFrame{80, "80%", {2, 0, -1, 1}, 1}};
        auto mixed = FrameLayout{0.001, {{1, {}, {}}}, PercentageFrame{80, "80%", {}, 1}};
        expect(rejects(bad_percent));
        expect(rejects(nonfinite_percent));
        expect(rejects(bad_scale));
        expect(rejects(bad_bounds));
        expect(rejects(mixed));
        Group invalid_scale;
        invalid_scale.transform =
            Transform{std::vector<TransformOperation>{Scaling{1, 1, false, 13}}};
        bool rejected_precision = false;
        try {
            Scene invalid{invalid_scale, {}, {}};
        } catch (std::invalid_argument const &) {
            rejected_precision = true;
        }
        expect(rejected_precision);
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

    "discard group removes its subtree and releases owned values in place"_test = [] {
        Builder builder;
        auto left = builder.add_group();
        builder.add_primitive(left, TextRun{"left", {}, {}, {}, {}});
        auto discarded = builder.add_group();
        auto child = builder.add_group(discarded);
        auto grandchild = builder.add_group(child);
        std::weak_ptr<GlyphResource const> discarded_resource;
        {
            auto resource = GlyphResource::make(
                "temporary", GlyphOutline{PathGeometry{{MoveTo{{0, 0}}, LineTo{{1, 1}}}}});
            discarded_resource = resource;
            builder.add_primitive(
                grandchild, GlyphRun{{Glyph{resource, {"normal", "temporary", 0x61}, {}, 1, {}}}});
        }
        auto right = builder.add_group();
        builder.add_primitive(right, TextRun{"right", {}, {}, {}, {}});
        auto const last_allocated = right;
        expect(!discarded_resource.expired());
        expect(builder.child_groups(0) == (std::vector<NodeId>{left, discarded, right}));

        builder.discard_group(discarded);
        expect(discarded_resource.expired());
        expect(builder.child_groups(0) == (std::vector<NodeId>{left, right}));
        expect(builder.entry_count(left) == 1_u && builder.entry_count(right) == 1_u);
        for (auto removed : {discarded, child, grandchild}) {
            bool missing = false;
            try {
                (void)builder.entry_count(removed);
            } catch (std::out_of_range const &) {
                missing = true;
            }
            expect(missing);
        }

        auto const next = builder.add_group();
        expect(next > last_allocated);
        builder.discard_group(next);
        auto scene = std::move(builder).freeze();
        expect(scene.root().entries.size() == 2_u);
        auto const &left_group = *std::get<std::shared_ptr<Group const>>(scene.root().entries[0]);
        auto const &right_group = *std::get<std::shared_ptr<Group const>>(scene.root().entries[1]);
        expect(left_group.id == left && right_group.id == right);
        expect(std::get<TextRun>(std::get<Primitive>(left_group.entries[0])).utf8 == "left");
        expect(std::get<TextRun>(std::get<Primitive>(right_group.entries[0])).utf8 == "right");
    };

    "discard group rejects root and unknown ids without changing the builder"_test = [] {
        Builder builder;
        auto keep = builder.add_group();
        auto drop = builder.add_group();
        auto child = builder.add_group(drop);
        bool root_rejected = false;
        bool unknown_rejected = false;
        try {
            builder.discard_group(0);
        } catch (std::invalid_argument const &) {
            root_rejected = true;
        }
        try {
            builder.discard_group(999);
        } catch (std::out_of_range const &) {
            unknown_rejected = true;
        }
        expect(root_rejected && unknown_rejected);
        expect(builder.child_groups(0) == (std::vector<NodeId>{keep, drop}));
        expect(builder.child_groups(drop) == std::vector<NodeId>{child});
        builder.add_primitive(keep, TextRun{"kept", {}, {}, {}, {}});
        builder.discard_group(drop);
        expect(builder.child_groups(0) == std::vector<NodeId>{keep});
        expect(builder.entry_count(keep) == 1_u);
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

    "split group contents atomically extracts descendant drawing ids in order"_test = [] {
        Builder builder;
        Viewport viewport{{0.1, -0.2, 1, 2}, {-0.3, -0.4, 2, 3}, {}};
        auto shell =
            builder.add_group(0, Transform{std::vector<TransformOperation>{Translation{0.1, 0.2}}},
                              {}, {}, {}, {}, ElementRole::math_wrapper, {}, viewport);
        auto row = builder.add_group(shell);
        auto line_a = builder.add_group(row);
        auto drawing_a = builder.add_group(line_a);
        builder.add_primitive(drawing_a, TextRun{"A", {}, {0.4, 0.5, 0.1}, {}, {}});
        auto line_b = builder.add_group(row);
        auto drawing_b = builder.add_group(line_b);
        builder.add_primitive(drawing_b, TextRun{"B", {}, {0.3, 0.4, 0.1}, {}, {}});
        std::array<NodeId, 2> sources{line_a, line_b};
        auto roots = builder.split_group_contents(shell, sources);
        expect(roots.size() == 2_u);
        expect(roots[0] == shell);
        expect(roots[1] != shell);
        expect(builder.child_groups(0) == roots);
        expect(builder.child_groups(roots[0]) == std::vector<NodeId>{drawing_a});
        expect(builder.child_groups(roots[1]) == std::vector<NodeId>{drawing_b});
        builder.set_frame_layout(
            {0.2,
             {{roots[0], {0.4, 0.5, 0.1}, {}},
              {roots[1], {0.3, 0.4, 0.1}, InlineBreak{InlineBreakKind::opportunity, 0}}}});
        auto scene = std::move(builder).freeze();
        expect(scene.frame_layout().inline_fragments.size() == 2_u);
        expect(scene.frame_layout().inline_fragments[0].root == roots[0]);
        expect(scene.frame_layout().inline_fragments[1].root == roots[1]);
        auto const &first_shell = *std::get<std::shared_ptr<Group const>>(scene.root().entries[0]);
        auto const &second_shell = *std::get<std::shared_ptr<Group const>>(scene.root().entries[1]);
        expect(first_shell.viewport && first_shell.viewport->bounds == viewport.bounds);
        expect(second_shell.viewport && second_shell.viewport->view_box == viewport.view_box);
        expect(first_shell.id == shell && second_shell.id == roots[1]);
        expect(first_shell.entries.size() == 1_u && second_shell.entries.size() == 1_u);
        auto const &first_drawing = *std::get<std::shared_ptr<Group const>>(first_shell.entries[0]);
        auto const &second_drawing =
            *std::get<std::shared_ptr<Group const>>(second_shell.entries[0]);
        expect(first_drawing.id == drawing_a && second_drawing.id == drawing_b);
        expect(std::get<TextRun>(std::get<Primitive>(first_drawing.entries[0])).utf8 == "A");
        expect(std::get<TextRun>(std::get<Primitive>(second_drawing.entries[0])).utf8 == "B");
    };

    "split group contents rejects invalid ancestry and overlaps without mutation"_test = [] {
        Builder builder;
        auto shell = builder.add_group(0);
        auto line = builder.add_group(shell);
        auto drawing = builder.add_group(line);
        builder.add_primitive(drawing, TextRun{"kept", {}, {}, {}, {}});
        auto outside = builder.add_group(0);
        bool missing = false, wrong_ancestry = false, overlap = false;
        std::array<NodeId, 1> bad_id{999};
        try {
            (void)builder.split_group_contents(shell, bad_id);
        } catch (std::out_of_range const &) {
            missing = true;
        }
        std::array<NodeId, 1> wrong{outside};
        try {
            (void)builder.split_group_contents(shell, wrong);
        } catch (std::invalid_argument const &) {
            wrong_ancestry = true;
        }
        std::array<NodeId, 2> overlapping{line, drawing};
        try {
            (void)builder.split_group_contents(shell, overlapping);
        } catch (std::invalid_argument const &) {
            overlap = true;
        }
        expect(missing && wrong_ancestry && overlap);
        expect(builder.child_groups(0) == (std::vector<NodeId>{shell, outside}));
        expect(builder.child_groups(shell) == std::vector<NodeId>{line});
        expect(builder.child_groups(line) == std::vector<NodeId>{drawing});
        expect(builder.entry_count(drawing) == 1_u);
        builder.add_primitive(drawing, TextRun{"still usable", {}, {}, {}, {}});
        expect(builder.entry_count(drawing) == 2_u);
    };

    "frame layout freeze rejects dangling roots and non-finite break spacing"_test = [] {
        auto rejects = [](FrameLayout frames) {
            Group root;
            auto child = std::make_shared<Group>();
            child->id = 1;
            root.entries.emplace_back(std::shared_ptr<Group const>{child});
            try {
                Scene scene{root, {1, 0.5, 0.1}, {}, {}, std::move(frames)};
            } catch (std::invalid_argument const &) {
                return true;
            }
            return false;
        };
        expect(rejects(FrameLayout{0.1, {{2, {1, 0.5, 0.1}, {}}}}));
        expect(rejects(FrameLayout{
            0.1,
            {{1,
              {1, 0.5, 0.1},
              InlineBreak{InlineBreakKind::forced, std::numeric_limits<Unit>::infinity()}}}}));
        expect(rejects(FrameLayout{0, {}}));
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
