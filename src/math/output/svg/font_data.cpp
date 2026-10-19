/*************************************************************
 * Source port of MathJax 4.1.3 ts/output/svg/FontData.ts.
 * Copyright (c) 2017-2026 The MathJax Consortium; Apache-2.0.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.svg.font_data;
import std;
import folia.math.core.tree.factory_types;
import folia.math.util.property_host;

namespace folia::math::output::svg {
SvgFontDataClass::SvgFontDataClass(options::Context& context):FontDataClass(context){
    JAX="SVG";
    const auto object=context.object();
    for(const auto& key:context.ownKeys(OPTIONS)){
        const auto descriptor=context.descriptor(OPTIONS,key);
        if(descriptor&&descriptor->enumerable)context.set(object,key,context.get(OPTIONS,key));
    }
    OPTIONS=options::Value(object);
    context.set(OPTIONS,options::Key(u"dynamicPrefix"),options::Value(u"./svg/dynamic"));
}

void SvgFontDataClass::addExtension(const SvgFontExtensionData& data,std::string prefix){
    FontDataClass::addExtension(data,std::move(prefix));
    if(data.cacheIds)for(const auto&[name,id]:*data.cacheIds)variantCacheIds.insert_or_assign(name,id);
}

SvgFontData::SvgFontData(SvgFontDataClass& fontClass,common::FontLoadHost& host,
                         options::Context& context,options::Value options)
    :FontData(fontClass,host,context,std::move(options)){}

Property SvgFontData::charOptions(SvgCharMap& font,std::uint32_t n){
    return common::FontDataClass::charOptions(font,n);
}

SvgCharMap& AddPaths(SvgCharMap& font,const CharStringMap& paths,const CharStringMap& content){
    auto set=[&](Property receiver,std::string_view key,Property value){
        if(receiver.missing())throw tree::FactoryError("TypeError",u"Cannot set properties of undefined (setting '"+property_host::text(key)+u"')");
        if(std::holds_alternative<PropertyNull>(receiver.value))throw tree::FactoryError("TypeError",u"Cannot set properties of null (setting '"+property_host::text(key)+u"')");
        property_host::set(receiver,key,std::move(value));
    };
    for(const auto&[cp,path]:paths)set(SvgFontData::charOptions(font,cp),"p",Property(path));
    for(const auto&[cp,text]:content)set(SvgFontData::charOptions(font,cp),"c",Property(text));
    return font;
}
}
