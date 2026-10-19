#include "support/folia_test.hpp"
import folia.math.output.scene.builder;
import folia.math.output.svg.writer;

namespace {
using namespace boost::ut;
namespace scene = folia::math::output::scene;
namespace svg = folia::math::output::svg;

scene::Scene make_scene() {
    scene::Builder builder;
    builder.set_metrics({2.0, 0.8, 0.2});
    builder.set_font_metrics({1.0, 0.5, 0.45, 0.7});
    return std::move(builder).freeze();
}
}

suite native_svg_writer = [] {
    "root preserves MathJax SVG attribute order and fixed dimensions"_test = [] {
        auto source = make_scene();
        auto output = svg::write(source, {.font_cache = svg::FontCacheMode::none});
        expect(output.starts_with("<mjx-container class=\"MathJax\" jax=\"SVG\" overflow=\"linebreak\"><svg style=\"vertical-align: -0.444ex;\" xmlns=\"http://www.w3.org/2000/svg\" width=\"4.444ex\" height=\"2.222ex\" role=\"img\" focusable=\"false\" viewBox=\"0 -800 2000 1000\">") );
        expect(output.ends_with("</g></svg></mjx-container>"));
    };

    "XML text and attributes escape markup characters"_test = [] {
        scene::Builder builder;
        builder.set_metrics({1, 0.5, 0.1});
        builder.set_font_metrics({1, 0.5, 0.5, 0.7});
        scene::SemanticMetadata metadata;
        metadata.source_tag = "mtext";
        metadata.ordered_attributes.push_back({"aria-label", "a&\"<b>"});
        metadata.ordered_attributes.push_back({"data-origin", "source"});
        auto group = builder.add_group(0, {}, {}, {}, {}, metadata);
        builder.add_primitive(group, scene::TextRun{"<&>"});
        auto output = svg::write(std::move(builder).freeze(), {.font_cache = svg::FontCacheMode::none});
        expect(output.find("aria-label=\"a&amp;&quot;&lt;b&gt;\"") != output.npos);
        expect(output.find("aria-label=") < output.find("data-origin=\"source\""));
        expect(output.find("><text>&lt;&amp;&gt;</text>") != output.npos);
    };

    "ordered groups retain SVG-space arc flags under reflection and metadata"_test = [] {
        scene::Builder builder;
        builder.set_metrics({1, 0.5, 0.1});
        builder.set_font_metrics({1, 0.5, 0.5, 0.7});
        scene::SemanticMetadata metadata;
        metadata.source_tag = "mrow";
        metadata.ordered_attributes.push_back({"class", "outer"});
        scene::Clip clip{scene::PathGeometry{{scene::MoveTo{{0, 0}}, scene::LineTo{{1, 0}}, scene::LineTo{{1, 1}}, scene::ClosePath{}}}, scene::FillRule::evenodd};
        scene::Transform ordered_transform{std::vector<scene::TransformOperation>{
            scene::Translation{0.719, 0.363}, scene::Scaling{0.707, 0.707}}};
        auto parent = builder.add_group(0, ordered_transform, clip, scene::Link{"https://example.test/?a&b", "_blank", "noopener"}, scene::Tooltip{"x<&"}, metadata);
        builder.add_primitive(parent, scene::Rule{{0, 0, 0.25, 0.05}, scene::Paint{.fill = scene::Color{255, 0, 0, 255}}});
        scene::Paint path_paint;
        path_paint.fill_rule = scene::FillRule::evenodd;
        builder.add_primitive(parent, scene::Path{scene::PathGeometry{{
            scene::MoveTo{{0, 0}}, scene::ArcTo{0.1, 0.2, 30, true, false, {0.4, 0.3}}}}, path_paint});
        builder.add_group(parent, scene::Transform{scene::Affine{.e = 0.5, .f = 0.25}});
        auto output = svg::write(std::move(builder).freeze(), {.font_cache = svg::FontCacheMode::none});
        auto anchor = output.find("<a href=\"https://example.test/?a&amp;b\" target=\"_blank\" rel=\"noopener\">");
        auto rule = output.find("<rect x=\"0\" y=\"-50\" width=\"250\" height=\"50\"");
        auto child = output.find("transform=\"matrix(1 0 0 1 500 250)\"");
        expect(output.find("<title>x&lt;&amp;</title>") != output.npos);
        expect(output.find("clip-path=\"url(#clip-") != output.npos);
        expect(output.find("<clipPath id=\"clip-") != output.npos);
        expect(output.find("clip-rule=\"evenodd\"") != output.npos);
        expect(output.find("fill-rule=\"evenodd\"") != output.npos);
        expect(output.find("A100 200 30 1 0 400,300") != output.npos);
        expect(output.find("transform=\"translate(719,363) scale(0.707)\"") != output.npos);
        expect(output.find("transform=\"scale(1,-1)\"") != output.npos);
        expect(anchor < rule && rule < child);
        expect(output.find("class=\"outer\"") != output.npos);
    };

    "typed translation rotation scale and affine operations stay ordered"_test = [] {
        scene::Builder builder;
        builder.set_metrics({1, 0.5, 0.1});
        builder.set_font_metrics({1, 0.5, 0.5, 0.7});
        scene::Transform transform{std::vector<scene::TransformOperation>{
            scene::Translation{0, 0.1},
            scene::Rotation{40.575, {0.519, 0.2155}},
            scene::Scaling{0.745, 1.0},
            scene::Affine{1, 0, 0, -1, 0, 0}}};
        builder.add_group(0, transform);
        auto output = svg::write(std::move(builder).freeze(),
            {.font_cache = svg::FontCacheMode::none});
        expect(output.find("transform=\"translate(0,100) rotate(40.575 519 215.5) scale(0.745,1) matrix(1 0 0 -1 0 0)\"") != output.npos);
        expect(output.find("transform=\"scale(1,-1)\"") != output.npos);
    };

    "pinned SVG font paths serialize from exact variant keys and cache ids"_test = [] {
        scene::Builder builder;
        builder.set_metrics({1, 0.5, 0.1});
        builder.set_font_metrics({1, 0.5, 0.5, 0.7});
        auto resource = scene::GlyphResource::make("NewCM-Roman", scene::GlyphOutline{
            scene::PathGeometry{{scene::MoveTo{{0, 0}}, scene::LineTo{{0.5, 0}},
                scene::LineTo{{0.5, 0.7}}, scene::ClosePath{}}}});
        builder.add_primitive(0, scene::GlyphRun{{scene::Glyph{resource, {"normal", "NCM-N", 0x5F}, {0.125, 0.25}, 0.6, {}}}});
        auto source = std::move(builder).freeze();
        svg::SvgFontResourcesBuilder font_builder;
        font_builder.add({"normal", 0x5F}, {true, "M0-140L333-140L333-100L0-100Z", {}});
        auto font_resources = std::move(font_builder).freeze();
        auto output = svg::write(source, {.font_cache = svg::FontCacheMode::local,
            .font_resources = &font_resources, .local_id = "1"});
        expect(output.find("<defs><path id=\"MJX-1-NCM-N-5F\" d=\"M0-140L333-140L333-100L0-100Z\"/></defs>") != output.npos);
        expect(output.find("<use data-c=\"5F\" xlink:href=\"#MJX-1-NCM-N-5F\" transform=\"translate(125 250)\"") != output.npos);
        auto uncached = svg::write(source, {.font_cache = svg::FontCacheMode::none,
            .font_resources = &font_resources});
        expect(uncached.find("<path data-c=\"5F\" d=\"M0-140L333-140L333-100L0-100Z\"") != uncached.npos);
    };

    "defined empty Source paths stay empty and missing catalog keys fail"_test = [] {
        scene::Builder builder;
        builder.set_metrics({1, 0.5, 0.1});
        builder.set_font_metrics({1, 0.5, 0.5, 0.7});
        auto resource = scene::GlyphResource::make("NewCM-Roman", scene::GlyphOutline{});
        builder.add_primitive(0, scene::GlyphRun{{scene::Glyph{resource, {"normal", "NCM-N", 0x20}, {}, 0.332, {}}}});
        auto source = std::move(builder).freeze();
        svg::SvgFontResourcesBuilder font_builder;
        font_builder.add({"normal", 0x20}, {true, "", {}});
        auto font_resources = std::move(font_builder).freeze();
        auto empty_path = svg::write(source, {.font_cache = svg::FontCacheMode::none,
            .font_resources = &font_resources});
        expect(empty_path.find("<path data-c=\"20\" d=\"\"/>") != empty_path.npos);

        bool missing_failed = false;
        try { (void)svg::write(source, {.font_cache = svg::FontCacheMode::none}); }
        catch (svg::UnsupportedError const&) { missing_failed = true; }
        expect(missing_failed);

        svg::SvgFontResourcesBuilder absent_path_builder;
        absent_path_builder.add({"normal", 0x20}, {false, "", {}});
        auto absent_path = std::move(absent_path_builder).freeze();
        bool absent_path_failed = false;
        try { (void)svg::write(source, {.font_cache = svg::FontCacheMode::none,
            .font_resources = &absent_path}); }
        catch (svg::UnsupportedError const&) { absent_path_failed = true; }
        expect(absent_path_failed);
    };

    "unsupported foreign markup fails explicitly instead of returning fake SVG"_test = [] {
        scene::Builder builder;
        builder.set_metrics({2, 0.8, 0.2});
        builder.set_font_metrics({1, 0.5, 0.5, 0.7});
        builder.add_primitive(0, scene::ForeignContent{
            "<b>foreign</b>", {0.25, 0.5}, {}, {0.75, 0.4, 0.1}, {}});
        bool rejected = false;
        try { (void)svg::write(std::move(builder).freeze(), {.font_cache = svg::FontCacheMode::none}); }
        catch (svg::UnsupportedError const&) { rejected = true; }
        expect(rejected);
    };

    "paint preserves inherited none currentColor and explicit color"_test = [] {
        scene::Builder builder;
        builder.set_metrics({1, 0.5, 0.1});
        builder.set_font_metrics({1, 0.5, 0.5, 0.7});
        scene::Paint inherited;
        scene::Paint none;
        none.fill = scene::NoPaint{};
        scene::Paint current;
        current.fill = scene::CurrentColorPaint{};
        scene::Paint explicit_color;
        explicit_color.fill = scene::Color{17, 34, 51, 255};
        builder.add_primitive(0, scene::Path{scene::PathGeometry{{scene::MoveTo{{0, 0}}}}, inherited});
        builder.add_primitive(0, scene::Path{scene::PathGeometry{{scene::MoveTo{{0, 0}}}}, none});
        builder.add_primitive(0, scene::Path{scene::PathGeometry{{scene::MoveTo{{0, 0}}}}, current});
        builder.add_primitive(0, scene::Path{scene::PathGeometry{{scene::MoveTo{{0, 0}}}}, explicit_color});
        auto output = svg::write(std::move(builder).freeze(), {.font_cache = svg::FontCacheMode::none});
        expect(output.find("<path d=\"M0,0\"/>") != output.npos);
        expect(output.find("<path d=\"M0,0\" fill=\"none\"/>") != output.npos);
        expect(output.find("<path d=\"M0,0\" fill=\"currentColor\"/>") != output.npos);
        expect(output.find("<path d=\"M0,0\" fill=\"#112233\"/>") != output.npos);
    };

    "SVG fixed numbers use ECMAScript tie rounding"_test = [] {
        scene::Builder builder;
        builder.set_metrics({0.00125, 0.00125, 0});
        builder.set_font_metrics({1, 0.5, 0.5, 0.7});
        auto output = svg::write(std::move(builder).freeze(), {.font_cache = svg::FontCacheMode::none});
        expect(output.find("viewBox=\"0 -1.3 1.3 1.3\"") != output.npos);
    };

    "SVG rejects zero x-height before converting dimensions to ex"_test = [] {
        scene::Builder bad_metrics;
        bad_metrics.set_metrics({1, 0.5, 0.1});
        bad_metrics.set_font_metrics({1, 0.5, 0, 0.7});
        bool metric_error = false;
        try { (void)svg::write(std::move(bad_metrics).freeze()); }
        catch (std::invalid_argument const&) { metric_error = true; }
        expect(metric_error);
    };

    "SVG selects semantic paths and preserves first use of shared cache ids"_test = [] {
        constexpr std::string_view first_path = "M0 0L10 10Z";
        constexpr std::string_view second_path = "M20 20L30 30Z";
        constexpr std::string_view first_variant = "variant-a";
        constexpr std::string_view second_variant = "variant-b";
        auto resource = scene::GlyphResource::make("synthetic", scene::GlyphOutline{});
        auto make_scene = [&](std::string first, std::string second) {
            scene::Builder builder;
            builder.set_metrics({1, 0.5, 0.1});
            builder.set_font_metrics({1, 0.5, 0.5, 0.7});
            builder.add_primitive(0, scene::GlyphRun{{
                scene::Glyph{resource, {std::move(first), "shared", 0x21}, {}, 0.5, {}},
                scene::Glyph{resource, {std::move(second), "shared", 0x21}, {0.5, 0}, 0.5, {}}}});
            return std::move(builder).freeze();
        };
        svg::SvgFontResourcesBuilder catalog_builder;
        catalog_builder.add({std::string(first_variant), 0x21}, {true, std::string(first_path), {}});
        catalog_builder.add({std::string(second_variant), 0x21}, {true, std::string(second_path), {}});
        auto catalog = std::move(catalog_builder).freeze();
        auto write_none = [&](std::string first, std::string second) {
            return svg::write(make_scene(std::move(first), std::move(second)),
                {.font_cache = svg::FontCacheMode::none, .font_resources = &catalog});
        };
        auto none_ab = write_none(std::string(first_variant), std::string(second_variant));
        auto none_ba = write_none(std::string(second_variant), std::string(first_variant));
        auto first_ab = none_ab.find(first_path), second_ab = none_ab.find(second_path);
        auto first_ba = none_ba.find(first_path), second_ba = none_ba.find(second_path);
        expect(first_ab != none_ab.npos && second_ab != none_ab.npos && first_ab < second_ab);
        expect(first_ba != none_ba.npos && second_ba != none_ba.npos && second_ba < first_ba);

        auto check_local = [&](std::string const& output, std::string_view winner, std::string_view loser) {
            expect(output.find("<defs><path id=\"MJX-1-shared-21\" d=\"" + std::string(winner) + "\"/></defs>") != output.npos);
            expect(output.find(loser) == output.npos);
            auto use = output.find("<use data-c=\"21\" xlink:href=\"#MJX-1-shared-21\"");
            expect(use != output.npos);
            auto second_use = output.find("<use data-c=\"21\" xlink:href=\"#MJX-1-shared-21\"", use + 1);
            expect(second_use != output.npos);
            if (second_use != output.npos)
                expect(output.find("<use data-c=\"21\" xlink:href=\"#MJX-1-shared-21\"", second_use + 1) == output.npos);
        };
        auto local_ab = svg::write(make_scene(std::string(first_variant), std::string(second_variant)),
            {.font_cache = svg::FontCacheMode::local, .font_resources = &catalog, .local_id = "1"});
        auto local_ba = svg::write(make_scene(std::string(second_variant), std::string(first_variant)),
            {.font_cache = svg::FontCacheMode::local, .font_resources = &catalog, .local_id = "1"});
        check_local(local_ab, first_path, second_path);
        check_local(local_ba, second_path, first_path);
    };
};
