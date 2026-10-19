/*************************************************************
 * MathJax 4.1.3 ts/output/svg/FontData.ts
 * Copyright (c) 2017-2026 The MathJax Consortium; Apache-2.0.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.svg.font_data;
export import folia.math.output.common.font_data;
import std;

export namespace folia::math::output::svg {
using common::CharDataArray;
using common::CharMap;
using common::CharMapMap;
using common::CharOptions;
using common::DelimiterData;
using common::FontData;
using common::FontDataClass;

struct SvgCharOptions : CharOptions {
    // SVG fields are stored in the same CharDataArray options record; the
    // common output layer treats those keys as opaque.
};
using CharStringMap = std::map<std::uint32_t,std::string>;
using SvgCharMap = CharMap;
using SvgCharData = CharDataArray;
using SvgVariantData = common::FontVariantData;
using SvgDelimiterData = DelimiterData;
using SvgFontExtensionData = common::FontExtensionData;

struct SvgFontDataClass : FontDataClass {
    std::map<std::string,std::string,std::less<>> variantCacheIds;
    explicit SvgFontDataClass(options::Context&);
    void addExtension(const SvgFontExtensionData&,std::string prefix={});
};

class SvgFontData : public FontData {
public:
    static constexpr std::string_view JAX = "SVG";
    SvgFontData(SvgFontDataClass&,common::FontLoadHost&,options::Context&,options::Value options=options::Undefined{});
    static Property charOptions(SvgCharMap& font,std::uint32_t n);
};
SvgCharMap& AddPaths(SvgCharMap& font,const CharStringMap& paths,const CharStringMap& content);
}
