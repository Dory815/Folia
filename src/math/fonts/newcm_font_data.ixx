/*************************************************************
 * Pinned NewCM font class data for MathJax 4.1.3.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.fonts.newcm_font_data;
export import folia.math.output.svg.font_data;
import std;

export namespace folia::math::fonts {
class NewcmFontDataClass final : public output::svg::SvgFontDataClass {
public:
    explicit NewcmFontDataClass(options::Context&);
    // Source class statics are installed only after the actual Context
    // constructor identity has been registered.
    void initializeSourceData();
};

class NewcmFontData final : public output::svg::SvgFontData {
public:
    NewcmFontData(NewcmFontDataClass&,output::common::FontLoadHost&,
                  options::Context&,options::Value options=options::Undefined{});
};
}
