module folia.math.fonts.newcm_font_data;
import folia.math.output.common.font_path;
import folia.math.output.svg.font_data;
import std;

namespace folia::math::fonts {
using namespace output::common;
using namespace output::svg;
#include "newcm_font_data_typed.inc"

void installNewcmFontData(output::common::FontRepositoryBuilder& builder,
                          output::svg::SvgFontResourcesBuilder& svgBuilder) {
    auto face = makeNewcmFace(svgBuilder);
    builder.addFace(std::move(face));
}
}
