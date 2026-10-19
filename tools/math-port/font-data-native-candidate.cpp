// Full-table native candidate for tools/math-port/compare-font-data.cjs.
// This is a small standalone executable source; Root owns its build target.
import std;
import folia.math.fonts.newcm_font_data;
import folia.math.fonts.newcm_font_module_provider;
import folia.math.fonts.svg_extensions;
import folia.math.util.property_host;
import folia.math.util.option_functions;
import folia.math.util.context_property_bindings;
import folia.math.util.map_bindings;
import folia.math.output.common.font_data_bindings;
import folia.math.output.common.font_data_options;

namespace {
using namespace folia::math;
using namespace folia::math::output::common;
using namespace folia::math::output::svg;
using namespace folia::math::fonts;
std::string currentPhase="startup";

void quoted(std::ostream& out,std::string_view value){
    out.put('"');
    constexpr char hex[]="0123456789abcdef";
    for(unsigned char c:value){
        switch(c){
        case '"':out<<"\\\"";break;
        case '\\':out<<"\\\\";break;
        case '\b':out<<"\\b";break;
        case '\f':out<<"\\f";break;
        case '\n':out<<"\\n";break;
        case '\r':out<<"\\r";break;
        case '\t':out<<"\\t";break;
        default:if(c<0x20){out<<"\\u00"<<hex[c>>4]<<hex[c&15];}else out.put(static_cast<char>(c));
        }
    }
    out.put('"');
}
void number(std::ostream& out,double n){
    if(std::isnan(n)){out<<"{\"$\":\"NaN\"}";return;}
    if(std::isinf(n)){out<<(n<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}");return;}
    if(n==0&&std::signbit(n)){out<<"{\"$\":\"-0\"}";return;}
    out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<n;
}
void property(std::ostream&,const Property&);
void record(std::ostream& out,const PropertyList& value){
    out.put('{');bool first=true;
    auto keys=value.keys();std::sort(keys.begin(),keys.end());
    for(const auto& key:keys){if(!first)out.put(',');first=false;quoted(out,key);out.put(':');property(out,value.getOwn(key));}
    out.put('}');
}
void property(std::ostream& out,const Property& value){
    std::visit([&](const auto& v){using T=std::decay_t<decltype(v)>;
        if constexpr(std::is_same_v<T,Missing>)out<<"{\"$\":\"undefined\"}";
        else if constexpr(std::is_same_v<T,PropertyNull>)out<<"null";
        else if constexpr(std::is_same_v<T,bool>)out<<(v?"true":"false");
        else if constexpr(std::is_same_v<T,double>)number(out,v);
        else if constexpr(std::is_same_v<T,std::string>)quoted(out,v);
        else if constexpr(std::is_same_v<T,std::shared_ptr<PropertyStorage>>){
            if(!v)throw std::runtime_error("null PropertyStorage in source table");
            if(v->array){
                Property arrayProperty;arrayProperty.value=v;
                const auto array=PropertyArray::array(arrayProperty);
                if(!array)throw std::runtime_error("array PropertyStorage lost its array view");
                out.put('[');
                for(std::size_t i=0;i<array->size();++i){
                    if(i)out.put(',');
                    if(!array->has(i))out<<"{\"$\":\"hole\"}";
                    else property(out,array->get(i));
                }
                out.put(']');
            }
            else {Property p;p.value=v;record(out,PropertyList::record(p).value());}
        }else throw std::runtime_error("unsupported Property alternative in source table");
    },value.value);
}
void chars(std::ostream& out,const CharMapMap& maps){
    out.put('{');bool firstVariant=true;
    for(const auto&[variant,map]:maps){if(!firstVariant)out.put(',');firstVariant=false;quoted(out,variant);out<<":{";
        bool firstGlyph=true;
        for(const auto&[cp,data]:map){
            const auto* glyph=std::get_if<Property>(&data);
            if(!glyph)throw std::runtime_error("non-glyph data in static character table");
            if(!firstGlyph)out.put(',');firstGlyph=false;out<<'"'<<cp<<"\":";
            property(out,*glyph);
        }
        out.put('}');
    }
    out.put('}');
}
void glyphValue(std::ostream& out,const Property& glyph){
    property(out,glyph);
}
void glyphMap(std::ostream& out,const CharacterMap& map){
    out.put('{');bool first=true;
    for(const auto&[cp,data]:map.own){
        const auto* ref=std::get_if<Property>(&data);
        if(!ref)throw std::runtime_error("non-glyph data remains in fully loaded character table");
        if(!first)out.put(',');first=false;out<<'"'<<cp<<"\":";
        glyphValue(out,*ref);
    }
    out.put('}');
}
void loadedGlyphTables(std::ostream& out,const FontVariantMap& variants){
    out.put('{');bool first=true;
    for(const auto&[name,data]:variants){if(!first)out.put(',');first=false;quoted(out,name);out.put(':');glyphMap(out,*data.chars);}
    out.put('}');
}
void delimiterValue(std::ostream& out,const DelimiterData& d){
    out<<"{\"dir\":";quoted(out,d.dir);
    auto vector=[&](std::string_view name,const std::optional<std::vector<double>>& values){if(!values)return;out.put(',');quoted(out,name);out.put(':');out.put('[');for(std::size_t i=0;i<values->size();++i){if(i)out.put(',');number(out,(*values)[i]);}out.put(']');};
    auto pair=[&](std::string_view name,const std::optional<std::array<double,2>>& values){if(!values)return;out.put(',');quoted(out,name);out<<":[";number(out,(*values)[0]);out.put(',');number(out,(*values)[1]);out.put(']');};
    auto triple=[&](std::string_view name,const std::optional<std::array<double,3>>& values){if(!values)return;out.put(',');quoted(out,name);out<<":[";for(std::size_t i=0;i<3;++i){if(i)out.put(',');number(out,(*values)[i]);}out.put(']');};
    vector("sizes",d.sizes);vector("variants",d.variants);vector("schar",d.schar);vector("stretch",d.stretch);vector("stretchv",d.stretchv);
    triple("HDW",d.HDW);pair("hd",d.hd);
    if(d.ext){out<<",\"ext\":";number(out,*d.ext);}if(d.min){out<<",\"min\":";number(out,*d.min);}if(d.c){out<<",\"c\":";number(out,*d.c);}pair("fullExt",d.fullExt);out.put('}');
}
void delimiters(std::ostream& out,const DelimiterMap& values){
    out.put('{');bool first=true;
    for(const auto&[cp,entry]:values){
        const auto* ref=std::get_if<DelimiterDataRef>(&entry);if(!ref||!*ref)throw std::runtime_error("non-delimiter data in resolved delimiter table");
        if(!first)out.put(',');first=false;out<<'"'<<cp<<"\":";
        delimiterValue(out,**ref);
    }
    out.put('}');
}
void optionValue(std::ostream& out,options::Context& context,const options::Value& value){
    std::visit([&](const auto& v){using T=std::decay_t<decltype(v)>;
        if constexpr(std::is_same_v<T,options::Undefined>)out<<"{\"$\":\"undefined\"}";
        else if constexpr(std::is_same_v<T,options::Null>)out<<"null";
        else if constexpr(std::is_same_v<T,bool>)out<<(v?"true":"false");
        else if constexpr(std::is_same_v<T,double>)number(out,v);
        else if constexpr(std::is_same_v<T,std::u16string>)quoted(out,property_host::debugText(v));
        else if constexpr(std::is_same_v<T,options::ObjectRef>){
            if(context.kind(v)==options::ObjectKind::array){
                out.put('[');const auto length=context.arrayLength(v);
                for(std::uint32_t i=0;i<length;++i){if(i)out.put(',');const options::Key key(property_host::text(std::to_string(i)));
                    if(!context.hasOwn(options::Value(v),key))out<<"{\"$\":\"hole\"}";
                    else optionValue(out,context,context.get(options::Value(v),key));}
                out.put(']');
            }else{
                auto keys=context.enumerableStringKeys(options::Value(v));std::sort(keys.begin(),keys.end());
                out.put('{');bool first=true;
                for(const auto& key:keys){if(!first)out.put(',');first=false;quoted(out,property_host::debugText(key));out.put(':');
                    optionValue(out,context,context.get(options::Value(v),options::Key(key)));}
                out.put('}');
            }
        }else throw std::runtime_error("unsupported symbol in option object dump");
    },value.data);
}
void stringVector(std::ostream& out,const PropertyArray& values){
    property(out,Property(values));
}
void stringVector(std::ostream& out,const std::vector<std::string>& values){
    out.put('[');for(std::size_t i=0;i<values.size();++i){if(i)out.put(',');quoted(out,values[i]);}out.put(']');
}
void stringVector(std::ostream& out,const std::vector<std::u16string>& values){
    out.put('[');for(std::size_t i=0;i<values.size();++i){if(i)out.put(',');quoted(out,property_host::debugText(values[i]));}out.put(']');
}
void cssFonts(std::ostream& out,const CssFontMap& values){
    out.put('{');bool first=true;
    for(const auto&[name,font]:values){if(!first)out.put(',');first=false;quoted(out,name);out.put(':');
        property(out,Property(font));}
    out.put('}');
}
void remapMap(std::ostream& out,const RemapMapMap& maps){
    out.put('{');bool firstMap=true;for(const auto&[name,map]:maps){if(!firstMap)out.put(',');firstMap=false;quoted(out,name);out.put(':');
        out.put('{');bool first=true;for(const auto&[cp,value]:map){if(!first)out.put(',');first=false;quoted(out,std::to_string(cp));out.put(':');quoted(out,property_host::debugText(value));}out.put('}');}
    out.put('}');
}
void remapEntries(std::ostream& out,const RemapMap& map){
    out.put('{');bool first=true;for(const auto&[cp,value]:map){if(!first)out.put(',');first=false;quoted(out,std::to_string(cp));out.put(':');quoted(out,property_host::debugText(value));}out.put('}');
}
void variantSmp(std::ostream& out,const OrderedMap<std::string,SmpSource>& values){
    out.put('{');bool firstMap=true;for(const auto&[name,source]:values){if(!firstMap)out.put(',');firstMap=false;quoted(out,name);out.put(':');
        std::visit([&](const auto& v){using T=std::decay_t<decltype(v)>;
            if constexpr(std::is_same_v<T,std::string>)quoted(out,v);
            else property(out,Property(v));
        },source);}
    out.put('}');
}
void variantDefinitions(std::ostream& out,const PropertyArray& values){
    property(out,Property(values));
}
void variantDefinitions(std::ostream& out,const std::vector<VariantDefinition>& values){
    out.put('[');for(std::size_t i=0;i<values.size();++i){if(i)out.put(',');property(out,Property(values[i]));}out.put(']');
}
void variantPatch(std::ostream& out,const std::optional<ListPatch>& patch){
    if(!patch){out<<"{\"$\":\"undefined\"}";return;}
    out.put('{');bool first=true;
    auto field=[&](std::string_view key,const std::optional<PropertyArray>& values){
        if(!values)return;if(!first)out.put(',');first=false;quoted(out,key);out.put(':');property(out,Property(*values));
    };
    field("[=]",patch->replace);field("[-]",patch->remove);field("[+]",patch->append);out.put('}');
}
void parameters(std::ostream& out,const FontParameters& p){
    out.put('{');bool first=true;auto field=[&](std::string_view key,double value){if(!first)out.put(',');first=false;quoted(out,key);out.put(':');number(out,value);};
    field("x_height",p.x_height);field("quad",p.quad);field("num1",p.num1);field("num2",p.num2);field("num3",p.num3);field("denom1",p.denom1);field("denom2",p.denom2);
    field("sup1",p.sup1);field("sup2",p.sup2);field("sup3",p.sup3);field("sub1",p.sub1);field("sub2",p.sub2);field("sup_drop",p.sup_drop);field("sub_drop",p.sub_drop);
    field("delim1",p.delim1);field("delim2",p.delim2);field("axis_height",p.axis_height);field("rule_thickness",p.rule_thickness);
    field("big_op_spacing1",p.big_op_spacing1);field("big_op_spacing2",p.big_op_spacing2);field("big_op_spacing3",p.big_op_spacing3);field("big_op_spacing4",p.big_op_spacing4);field("big_op_spacing5",p.big_op_spacing5);
    field("surd_height",p.surd_height);field("scriptspace",p.scriptspace);field("nulldelimiterspace",p.nulldelimiterspace);field("delimiterfactor",p.delimiterfactor);
    field("delimitershortfall",p.delimitershortfall);field("rule_factor",p.rule_factor);field("min_rule_thickness",p.min_rule_thickness);field("separation_factor",p.separation_factor);
    field("extra_ic",p.extra_ic);field("extender_factor",p.extender_factor);
    for(const auto&[key,value]:p.extra){const auto* n=std::get_if<double>(&value.value);if(!n)throw std::runtime_error("non-numeric font parameter in source table");field(key,*n);}
    out.put('}');
}
void dynamicExtensions(std::ostream& out,folia::math::options::Context& context,
    const folia::math::options::Value& map){
    using namespace folia::math::options;
    const auto method=context.get(map,Key(u"values"));
    const auto* callable=std::get_if<ObjectRef>(&method.data);
    if(!callable)throw std::runtime_error("dynamicExtensions.values is not callable");
    const auto iterable=context.call(*callable,map);
    out.put('[');bool first=true;
    context.forEachIterable(iterable,[&](Value entry){
        const auto* object=std::get_if<ObjectRef>(&entry.data);
        if(!object)throw std::runtime_error("dynamic extension value is not an object");
        auto read=[&](std::u16string_view key){return context.get(entry,Key(std::u16string(key)));};
        const auto name=read(u"name"),prefix=read(u"prefix"),sizeN=read(u"sizeN"),stretchN=read(u"stretchN");
        const auto filesValue=read(u"files");
        if(!first)out.put(',');first=false;out<<"{\"name\":";optionValue(out,context,name);out<<",\"prefix\":";optionValue(out,context,prefix);
        out<<",\"sizeN\":";optionValue(out,context,sizeN);out<<",\"stretchN\":";optionValue(out,context,stretchN);out<<",\"files\":";
        const auto fileNames=context.enumerableStringKeys(filesValue);stringVector(out,fileNames);out.put('}');
    });
    out.put(']');
}
void cacheIds(std::ostream& out,const std::map<std::string,std::string,std::less<>>& ids){
    out.put('{');bool first=true;for(const auto&[name,id]:ids){if(!first)out.put(',');first=false;quoted(out,name);out.put(':');quoted(out,id);}out.put('}');
}
void dynamicFileNames(std::ostream& out,const DynamicFileList& files){std::vector<std::string> names;for(const auto&[name,_]:files)names.push_back(name);stringVector(out,names);}
void classState(std::ostream& out,options::Context& context,const folia::math::output::svg::SvgFontDataClass& font){
    out<<"{\"chars\":";chars(out,font.defaultChars);out<<",\"delimiters\":";delimiters(out,font.defaultDelimiters);
    out<<",\"cacheIds\":";cacheIds(out,font.variantCacheIds);out<<",\"options\":";optionValue(out,context,font.OPTIONS);
    out<<",\"variants\":";variantDefinitions(out,PropertyArray::array(font.defaultVariants).value());out<<",\"cssFonts\":";cssFonts(out,font.defaultCssFonts);
    out<<",\"variantSmp\":";variantSmp(out,font.VariantSmp);
    out<<",\"accentMap\":";remapEntries(out,font.defaultAccentMap);out<<",\"moMap\":";remapEntries(out,font.defaultMoMap);out<<",\"mnMap\":";remapEntries(out,font.defaultMnMap);
    out<<",\"sizeVariants\":";stringVector(out,PropertyArray::array(font.defaultSizeVariants).value());out<<",\"stretchVariants\":";stringVector(out,PropertyArray::array(font.defaultStretchVariants).value());
    out<<",\"parameters\":";parameters(out,font.defaultParams);out<<",\"dynamicFiles\":";dynamicFileNames(out,font.dynamicFiles);
    out<<",\"dynamicExtensions\":";
    if(!font.sourceIdentity)throw std::runtime_error("font class has no Source constructor identity");
    dynamicExtensions(out,context,context.get(folia::math::options::Value(*font.sourceIdentity),
        folia::math::options::Key(u"dynamicExtensions")));out.put('}');
}
void cssQuery(std::ostream& out,const CssFontDataRef& font){
    property(out,Property(font));
}
void instanceState(std::ostream& out,options::Context& context,FontData& font){
    out<<"{\"options\":";optionValue(out,context,font.options());out<<",\"parameters\":";parameters(out,font.params());
    out<<",\"sizeVariants\":";stringVector(out,PropertyArray::array(font.sizeVariants()).value());out<<",\"stretchVariants\":";stringVector(out,PropertyArray::array(font.stretchVariants()).value());
    out<<",\"cssFamilyPrefix\":";quoted(out,font.cssFamilyPrefix);out<<",\"cssFontPrefix\":";quoted(out,font.cssFontPrefix);
    out<<",\"cssFonts\":";cssFonts(out,font.cssFonts());out<<",\"remaps\":";remapMap(out,font.remapChars());
    out<<",\"variantNames\":[";bool first=true;for(const auto&[name,_]:font.variants()){if(!first)out.put(',');first=false;quoted(out,name);}out<<"],\"cacheIds\":{";
    first=true;for(const auto&[name,variant]:font.variants()){if(!first)out.put(',');first=false;quoted(out,name);out.put(':');if(variant.cacheID)quoted(out,*variant.cacheID);else out<<"{\"$\":\"undefined\"}";}out<<'}';
    const auto normal=font.getVariant("normal");const auto normalRecord=font.variants().find("normal");
    out<<",\"variantRecordIdentity\":"<<(normal&&normalRecord!=font.variants().end() && &normal->get()==&normalRecord->second?"true":"false");
    const auto normalZero=font.getChar("normal",48),boldZero=font.getChar("bold",48),italicZero=font.getChar("italic",48);
    const auto normalA=font.getChar("normal",65),boldA=font.getChar("bold",65);
    const auto normalBar=font.getChar("normal",8213),boldBar=font.getChar("bold",8213),missing=font.getChar("normal",0x10ffff);
    auto same=[](const auto& a,const auto& b){return a&&b&&*a==*b;};
    out<<",\"sharedCharacterRecords\":{\"normalBoldZero\":"<<(same(normalZero,boldZero)?"true":"false")
       <<",\"normalItalicZero\":"<<(same(normalZero,italicZero)?"true":"false")<<",\"normalBoldA\":"<<(same(normalA,boldA)?"true":"false")
       <<",\"normalBold8213\":"<<(same(normalBar,boldBar)?"true":"false")<<",\"missingIsUndefined\":"<<(!missing?"true":"false")<<'}';
    out<<",\"cssQueries\":{\"normal\":";cssQuery(out,font.getCssFont("normal"));out<<",\"unknown\":";cssQuery(out,font.getCssFont("missing"));out<<",\"family\":";quoted(out,font.getFamily("serif"));out.put('}');
    const auto initialStyles=font.styles();out<<",\"styles\":{\"initial\":";property(out,initialStyles);
    auto storage=std::make_shared<PropertyStorage>();storage->entries.emplace_back("test",Property(std::string("identity")));Property style;style.value=storage;
    font.setStyles(style);const auto after=font.styles();out<<",\"setAndRead\":{\"value\":";property(out,after);out<<",\"sameIdentity\":";
    const auto* a=std::get_if<std::shared_ptr<PropertyStorage>>(&style.value);const auto* b=std::get_if<std::shared_ptr<PropertyStorage>>(&after.value);
    out<<((a&&b&&*a==*b)?"true":"false")<<"}}}";
}
void table(std::ostream& out,std::string_view name,const CharMapMap& c,const DelimiterMap& d){
    out<<"{\"name\":";quoted(out,name);out<<",\"chars\":";chars(out,c);out<<",\"delimiters\":";delimiters(out,d);out.put('}');
}
}

int main(){
    try {
    currentPhase="create options context and NewCM class";
    folia::math::options::Context context;
    folia::math::OptionFunctionBindings functions(context);
    folia::math::ContextPropertyBindings properties(context,functions);
    FontDataContextBindings identities(context);
    folia::math::options::MapContextBindings maps(context);
    auto newcmOwner=std::make_shared<folia::math::fonts::NewcmFontDataClass>(context);
    const auto bbm=folia::math::fonts::BbmFontExtensionData();
    const auto bboldx=folia::math::fonts::BboldxFontExtensionData();
    const auto dsfont=folia::math::fonts::DsfontFontExtensionData();
    auto queue=std::make_shared<folia::math::async::JobQueue>();
    auto hostAnchor=std::make_shared<FontLoadHost>();
    auto& host=*hostAnchor;
    host.queue=queue;host.asyncIsSynchronous=[] {return true;};
    const auto registerFontClass=[&](std::shared_ptr<folia::math::fonts::NewcmFontDataClass> owner,
                                     std::optional<folia::math::options::ObjectRef> parent={}){
        auto constructor=identities.registerClass(u"MathJaxNewcmFont",owner,*hostAnchor,
            std::static_pointer_cast<void>(hostAnchor),
            [](FontDataClass& actual,FontLoadHost& loadHost,folia::math::options::Context& cx,
               folia::math::options::Value options){
                return std::make_shared<folia::math::fonts::NewcmFontData>(
                    static_cast<folia::math::fonts::NewcmFontDataClass&>(actual),loadHost,cx,std::move(options));
            },parent);
        owner->bindSourceIdentity(properties,constructor,maps);
        owner->dynamicExtensions=folia::math::options::Value(maps.create());
        context.bindData(constructor,folia::math::options::Key(u"dynamicExtensions"),
            std::make_shared<folia::math::options::NativeDataField>(std::static_pointer_cast<void>(owner),
                [owner]{return owner->dynamicExtensions;},
                [owner](folia::math::options::Value value){owner->dynamicExtensions=std::move(value);}));
        owner->initializeSourceData();
        bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
            u"defaultVariants",owner->defaultVariants);
        bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
            u"defaultSizeVariants",owner->defaultSizeVariants);
        bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
            u"defaultStretchVariants",owner->defaultStretchVariants);
        return constructor;
    };
    const auto classConstructor=registerFontClass(newcmOwner);
    auto& newcm=*newcmOwner;
    auto moduleProvider=std::make_shared<folia::math::fonts::NewcmFontModuleProvider>(newcmOwner,queue);
    host.syncLoad=[moduleProvider](FontData& font,const DynamicFileRef& file,std::string request){
        moduleProvider->loadSync(font,file,std::move(request));
    };
    currentPhase="construct static font instance";
    folia::math::fonts::NewcmFontData instance(newcm,host,context);
    std::cout<<"{\"tables\":[";
    table(std::cout,"@mathjax/mathjax-newcm-font",newcm.defaultChars,newcm.defaultDelimiters);
    auto extension=[&](std::string_view name,const auto& value){
        const auto charsValue=value.chars.value_or(CharMapMap{});
        const auto delimitersValue=value.delimiters.value_or(DelimiterMap{});
        std::cout<<',';table(std::cout,name,charsValue,delimitersValue);
    };
    extension("@mathjax/mathjax-bbm-font-extension",bbm);
    extension("@mathjax/mathjax-bboldx-font-extension",bboldx);
    extension("@mathjax/mathjax-dsfont-font-extension",dsfont);
    std::ostringstream samples;
    const auto asciiA=instance.getChar("normal",65),minus=instance.getChar("normal",0x2212),missing=instance.getChar("normal",0x10ffff);
    const auto doubleStruckA=instance.getChar("double-struck",65);
    auto optionalGlyph=[&](std::ostream& out,const std::optional<Property>& glyph){if(!glyph)out<<"{\"$\":\"undefined\"}";else glyphValue(out,*glyph);};
    samples<<"{\"asciiA\":";optionalGlyph(samples,asciiA);samples<<",\"minus\":";optionalGlyph(samples,minus);samples<<",\"missing\":";optionalGlyph(samples,missing);
    samples<<",\"doubleStruckA\":";optionalGlyph(samples,doubleStruckA);samples<<",\"family\":";quoted(samples,instance.getFamily("serif"));
    samples<<",\"missingCssFont\":";cssQuery(samples,instance.getCssFont("missing"));
    samples<<",\"remappedAccent\":";if(auto value=instance.getRemappedChar("accent",0x2c6))quoted(samples,property_host::debugText(*value));else samples<<"{\"$\":\"undefined\"}";
    samples<<",\"delimiterParen\":";if(auto value=instance.getDelimiter(0x28)){if(const auto* ref=std::get_if<DelimiterDataRef>(&*value);ref&&*ref)delimiterValue(samples,**ref);else samples<<"null";}else samples<<"{\"$\":\"undefined\"}";
    samples<<",\"sizeVariantParen\":";if(auto value=instance.getSizeVariant(0x28,0))quoted(samples,*value);else samples<<"{\"$\":\"undefined\"}";
    samples<<",\"stretchVariantsParen\":[";const auto stretch=instance.getStretchVariants(0x28);for(std::size_t i=0;i<stretch.size();++i){if(i)samples.put(',');if(stretch[i])quoted(samples,*stretch[i]);else samples<<"{\"$\":\"undefined\"}";}samples.put(']');
    samples.put('}');
    std::ostringstream instanceDump;instanceState(instanceDump,context,instance);
    // The original oracle uses a fresh second instance for dynamic loading.
    // This keeps the static instance observations independent of shard setup.
    currentPhase="construct dynamic font instance";
    folia::math::fonts::NewcmFontData loaded(newcm,host,context);
    const auto dynamicLatin=loaded.getChar("normal",0xc0);
    samples.seekp(-1,std::ios_base::end);samples<<",\"dynamicLatinAfterLoad\":";optionalGlyph(samples,dynamicLatin);
    currentPhase="load all dynamic font files synchronously";
    loaded.loadDynamicFilesSync();
    samples<<",\"dynamicFileSetup\":[";bool firstFile=true;for(const auto&[name,file]:newcm.dynamicFiles){if(!firstFile)samples.put(',');firstFile=false;samples<<"{\"name\":";quoted(samples,name);samples<<",\"failed\":"<<(file&&file->failed?"true":"false")<<'}';}samples<<"]}";
    std::cout<<"],\"samples\":"<<samples.str()<<",\"instance\":"<<instanceDump.str();
    std::cout<<",\"dynamic\":{\"chars\":";loadedGlyphTables(std::cout,loaded.variants());
    std::cout<<",\"delimiters\":";delimiters(std::cout,loaded.delimiters());std::cout<<'}';
    currentPhase="merge font extension classes";
    // The pinned source observes all static extension merges on the exact
    // constructor already used by both instances above. Keep its constructor
    // identity and live static field cells throughout this sequence.
    auto& merged=newcm;
    std::cout<<",\"classExtensionMerges\":[{\"name\":\"initial\",\"returnValue\":{\"$\":\"undefined\"},\"state\":";classState(std::cout,context,merged);std::cout<<'}';
    auto merge=[&](std::string_view name,const auto& extension){
        std::cout<<",{\"name\":";quoted(std::cout,name);std::cout<<",\"before\":";classState(std::cout,context,merged);
        merged.addExtension(extension);
        std::cout<<",\"returnValue\":{\"$\":\"undefined\"},\"state\":";classState(std::cout,context,merged);std::cout.put('}');
    };
    merge("@mathjax/mathjax-bbm-font-extension",bbm);merge("@mathjax/mathjax-bboldx-font-extension",bboldx);merge("@mathjax/mathjax-dsfont-font-extension",dsfont);
    std::cout<<"]}\n";
    return 0;
    } catch(const folia::math::options::Exception& e) {
        std::cerr<<"font dump phase="<<currentPhase<<" exception="<<e.name<<":"<<property_host::debugText(e.message)<<"\n";
    } catch(const std::exception& e) {
        std::cerr<<"font dump phase="<<currentPhase<<" exception=Error:"<<e.what()<<"\n";
    } catch(...) {
        std::cerr<<"font dump phase="<<currentPhase<<" exception=unknown\n";
    }
    return 2;
}
