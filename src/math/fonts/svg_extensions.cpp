/*************************************************************
 * Pinned MathJax 4.1.3 SVG font extension datasets.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.fonts.svg_extensions;
import std;

namespace folia::math::fonts {
namespace {
using namespace output::common;
using namespace output::svg;
#include "font_extensions.inc"
}

output::svg::SvgFontExtensionData BbmFontExtensionData(){return makeBbmExtension();}
output::svg::SvgFontExtensionData BboldxFontExtensionData(){return makeBboldxExtension();}
output::svg::SvgFontExtensionData DsfontFontExtensionData(){return makeDsfontExtension();}
}
