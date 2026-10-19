/*************************************************************
 * MathJax 4.1.3 NewCM SVG font class/instance specialization.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.fonts.newcm_font_data;
import std;

namespace folia::math::fonts {
using output::common::FontDataClass;
namespace {
using namespace output::common;
using namespace output::svg;
#include "newcm_font_data.inc"
}
#include "newcm_dynamic_setup.inc"

NewcmFontDataClass::NewcmFontDataClass(options::Context& context)
    :SvgFontDataClass(context){
    // Source NewCM class options spread the SVG defaults and then override
    // dynamicPrefix; retain every inherited option key.
    context.set(OPTIONS,options::Key(u"dynamicPrefix"),
                options::Value(u"@mathjax/mathjax-newcm-font/js/svg/dynamic"));
    NAME="MathJaxNewcm";
    JAX="SVG";
}

void NewcmFontDataClass::initializeSourceData(){
    installGeneratedFontData(*this,&variantCacheIds);
    installGeneratedNewcmDynamicSetup(*this);
}

NewcmFontData::NewcmFontData(NewcmFontDataClass& fontClass,
                             output::common::FontLoadHost& host,
                             options::Context& context,options::Value supplied)
    :SvgFontData(fontClass,host,context,std::move(supplied)){
    for(auto&[name,variant]:variant_){
        const auto found=fontClass.variantCacheIds.find(name);
        variant.cacheID="NCM-"+(found==fontClass.variantCacheIds.end()?std::string("N"):found->second);
    }
}
}
