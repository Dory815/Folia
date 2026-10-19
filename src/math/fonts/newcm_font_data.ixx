export module folia.math.fonts.newcm_font_data;
export import folia.math.output.common.font_data;
export import folia.math.output.svg.font_data;
import std;

export namespace folia::math::fonts {
// Installs the complete pinned NewCM profile into a private builder. Generated
// shards are ordinary typed data initializers; rendering never loads JS.
void installNewcmFontData(output::common::FontRepositoryBuilder& builder,
                          output::svg::SvgFontResourcesBuilder& svgBuilder);
}
