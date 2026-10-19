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
        auto parent = builder.add_group(0, {}, clip, scene::Link{"https://example.test/?a&b", "_blank", "noopener"}, scene::Tooltip{"x<&"}, metadata);
        builder.add_primitive(parent, scene::Rule{{0, 0, 0.25, 0.05}, scene::Paint{.fill = scene::Color{255, 0, 0, 255}}});
        scene::Paint path_paint;
        path_paint.fill_rule = scene::FillRule::evenodd;
        builder.add_primitive(parent, scene::Path{scene::PathGeometry{{
            scene::MoveTo{{0, 0}}, scene::ArcTo{0.1, 0.2, 30, true, false, {0.4, 0.3}}}}, path_paint});
        builder.add_group(parent, {.e = 0.5, .f = 0.25});
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
        expect(output.find("transform=\"scale(1,-1)\"") != output.npos);
        expect(anchor < rule && rule < child);
        expect(output.find("class=\"outer\"") != output.npos);
    };

    "glyph resources serialize as cached paths and references"_test = [] {
        scene::Builder builder;
        builder.set_metrics({1, 0.5, 0.1});
        builder.set_font_metrics({1, 0.5, 0.5, 0.7});
        auto resource = scene::GlyphResource::make("NewCM-Roman", scene::GlyphOutline{
            scene::PathGeometry{{scene::MoveTo{{0, 0}}, scene::LineTo{{0.5, 0}},
                scene::LineTo{{0.5, 0.7}}, scene::ClosePath{}}}});
        builder.add_primitive(0, scene::GlyphRun{{scene::Glyph{resource, 0x41, {0.125, 0.25}, 0.6, {}}}});
        auto source = std::move(builder).freeze();
        auto output = svg::write(source, {.font_cache = svg::FontCacheMode::local, .use_xlink = true, .local_id = "1"});
        expect(output.find("<defs><path id=\"MJX-1-NewCM-Roman-41\" d=\"M0,0L500,0L500,700Z\"/></defs>") != output.npos);
        expect(output.find("<use data-c=\"41\" xlink:href=\"#MJX-1-NewCM-Roman-41\" transform=\"translate(125 250)\"") != output.npos);
        auto uncached = svg::write(source, {.font_cache = svg::FontCacheMode::none});
        expect(uncached.find("<path data-c=\"41\" d=\"M0,0L500,0L500,700Z\"") != uncached.npos);
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

    "invalid x-height and missing glyph outline fail explicitly"_test = [] {
        scene::Builder bad_metrics;
        bad_metrics.set_metrics({1, 0.5, 0.1});
        bad_metrics.set_font_metrics({1, 0.5, 0, 0.7});
        bool metric_error = false;
        try { (void)svg::write(std::move(bad_metrics).freeze()); }
        catch (...) { metric_error = true; }
        expect(metric_error);

        scene::Builder bad_glyph;
        bad_glyph.set_metrics({1, 0.5, 0.1});
        bad_glyph.set_font_metrics({1, 0.5, 0.5, 0.7});
        bad_glyph.add_primitive(0, scene::GlyphRun{{scene::Glyph{}}});
        bool glyph_error = false;
        try { (void)svg::write(std::move(bad_glyph).freeze()); }
        catch (...) { glyph_error = true; }
        expect(glyph_error);
    };
};
