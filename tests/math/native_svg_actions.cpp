#include "support/folia_test.hpp"

import folia.math.output.scene.builder;
import folia.math.output.svg.writer;

namespace {
using namespace boost::ut;
namespace scene = folia::math::output::scene;
namespace svg = folia::math::output::svg;

scene::Scene tooltip_scene() {
    scene::Builder builder;
    builder.set_metrics({0.4, 0.3, 0.1});
    builder.set_font_metrics({1, 0.5, 0.5, 0.7});
    auto metadata = builder.metadata(0);
    metadata.source_tag = "math";
    builder.set_metadata(0, std::move(metadata));
    builder.add_primitive(0, scene::Title{"nested"});
    return std::move(builder).freeze();
}

scene::Scene action_scene(std::shared_ptr<scene::Scene const> tooltip) {
    scene::Builder builder;
    builder.set_metrics({0.572, 0.442, 0.011});
    builder.set_font_metrics({1, 0.5, 0.5, 0.7});
    const auto group = builder.add_group();
    auto metadata = builder.metadata(group);
    metadata.source_tag = "maction";
    builder.set_metadata(group, std::move(metadata));
    scene::Rule hitbox;
    hitbox.bounds = {0, -0.011, 0.572, 0.453};
    hitbox.paint.fill = scene::NoPaint{};
    if (tooltip)
        builder.set_action(group, scene::MathTooltipAction{});
    (void)builder.add_action_region(group, {std::move(hitbox), std::move(tooltip)});
    auto selected_metadata = scene::SemanticMetadata{};
    selected_metadata.source_tag = "mi";
    (void)builder.add_group(group, {}, {}, {}, {}, std::move(selected_metadata));
    return std::move(builder).freeze();
}
} // namespace

suite native_svg_actions = [] {
    "none mode emits action hitbox and nested tooltip SVG without a container"_test = [] {
        auto nested = std::make_shared<scene::Scene const>(tooltip_scene());
        auto output =
            svg::write(action_scene(std::move(nested)), {.font_cache = svg::FontCacheMode::none});
        const auto action =
            output.find("<rect width=\"572\" height=\"453\" x=\"0\" y=\"-11\" fill=\"none\" "
                        "pointer-events=\"all\"><foreignObject style=\"display: none;\"><mjx-tool>"
                        "<mjx-tip><svg");
        expect(action != output.npos);
        expect(output.find("</svg></mjx-tip></mjx-tool></foreignObject></rect>") != output.npos);
        expect(output.find("<mjx-container", output.find("<foreignObject")) == output.npos);
        expect(output.find("</rect><g data-mml-node=\"mi\"/>") != output.npos);
    };

    "local mode rejects math tooltip until shared cache state is available"_test = [] {
        auto nested = std::make_shared<scene::Scene const>(tooltip_scene());
        bool rejected = false;
        try {
            (void)svg::write(action_scene(std::move(nested)),
                             {.font_cache = svg::FontCacheMode::local});
        } catch (svg::UnsupportedError const &) {
            rejected = true;
        }
        expect(rejected);
    };

    "local mode retains text tooltip toggle and statusline actions"_test = [] {
        scene::Builder builder;
        builder.set_metrics({1, 0.5, 0.1});
        builder.set_font_metrics({1, 0.5, 0.5, 0.7});
        const auto text_group = builder.add_group();
        auto text_metadata = builder.metadata(text_group);
        text_metadata.source_tag = "maction";
        builder.set_metadata(text_group, std::move(text_metadata));
        builder.add_primitive(text_group, scene::Title{"Tip & < \" '"});
        scene::Rule text_hitbox;
        text_hitbox.bounds = {0, -0.011, 0.572, 0.453};
        (void)builder.add_action_region(text_group, {std::move(text_hitbox), {}});
        const auto toggle_group = builder.add_group();
        auto toggle_metadata = builder.metadata(toggle_group);
        toggle_metadata.source_tag = "maction";
        toggle_metadata.ordered_attributes.push_back({"data-toggle", "2"});
        builder.set_metadata(toggle_group, std::move(toggle_metadata));
        builder.set_action(toggle_group, scene::ToggleAction{2, 3, false});

        const auto status_group = builder.add_group();
        auto status_metadata = builder.metadata(status_group);
        status_metadata.source_tag = "maction";
        status_metadata.ordered_attributes.push_back({"data-statusline", "status"});
        builder.set_metadata(status_group, std::move(status_metadata));
        builder.set_action(status_group, scene::StatuslineAction{"status"});

        auto output =
            svg::write(std::move(builder).freeze(), {.font_cache = svg::FontCacheMode::local});
        expect(output.find("<g data-mml-node=\"maction\"><title>Tip & < \" '</title>"
                           "<rect width=\"572\" height=\"453\" x=\"0\" y=\"-11\" "
                           "fill=\"none\" pointer-events=\"all\"/></g>") != output.npos);
        expect(output.find("data-toggle=\"2\"") != output.npos);
        expect(output.find("data-statusline=\"status\"") != output.npos);
    };
};
