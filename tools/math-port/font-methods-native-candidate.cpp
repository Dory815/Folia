// Supplemental native observer for font-methods-oracle.cjs.
// Every emitted result is read from the live native FontData objects.
import std;
import folia.math.fonts.newcm_font_data;
import folia.math.fonts.svg_extensions;
import folia.math.util.property_host;
import folia.math.core.tree.factory_types;
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
namespace math_options=folia::math::options;

math_options::ObjectRef registerNewcmSourceClass(math_options::Context& context,
    OptionFunctionBindings& functions,ContextPropertyBindings& properties,
    FontDataContextBindings& identities,math_options::MapContextBindings& maps,
    const std::shared_ptr<FontLoadHost>& host,
    const std::shared_ptr<NewcmFontDataClass>& owner){
    (void)functions;
    const auto constructor=identities.registerClass(u"MathJaxNewcmFont",owner,*host,
        std::static_pointer_cast<void>(host),
        [](FontDataClass& actual,FontLoadHost& loadHost,math_options::Context& cx,
           math_options::Value options){
            return std::make_shared<NewcmFontData>(
                static_cast<NewcmFontDataClass&>(actual),loadHost,cx,std::move(options));
        });
    owner->bindSourceIdentity(properties,constructor,maps);
    owner->dynamicExtensions=math_options::Value(maps.create());
    context.bindData(constructor,math_options::Key(u"dynamicExtensions"),
        std::make_shared<math_options::NativeDataField>(std::static_pointer_cast<void>(owner),
            [owner]{return owner->dynamicExtensions;},
            [owner](math_options::Value value){owner->dynamicExtensions=std::move(value);}));
    bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
        u"defaultVariants",owner->defaultVariants);
    bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
        u"defaultSizeVariants",owner->defaultSizeVariants);
    bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
        u"defaultStretchVariants",owner->defaultStretchVariants);
    owner->initializeSourceData();
    return constructor;
}

void quote(std::ostream& out, std::string_view s) {
    out.put('"');
    for (unsigned char c : s) {
        switch (c) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default: out.put(static_cast<char>(c)); break;
        }
    }
    out.put('"');
}
void number(std::ostream& out, double x) {
    if (std::isnan(x)) out << "{\"$\":\"NaN\"}";
    else if (std::isinf(x)) out << (x < 0 ? "{\"$\":\"-Infinity\"}" : "{\"$\":\"Infinity\"}");
    else if (x == 0 && std::signbit(x)) out << "{\"$\":\"-0\"}";
    else out << std::setprecision(std::numeric_limits<double>::max_digits10) << x;
}
void prop(std::ostream& out, const Property& p) {
    std::visit([&](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, Missing>) out << "{\"$\":\"undefined\"}";
        else if constexpr (std::is_same_v<T, PropertyNull>) out << "null";
        else if constexpr (std::is_same_v<T, bool>) out << (v ? "true" : "false");
        else if constexpr (std::is_same_v<T, double>) number(out, v);
        else if constexpr (std::is_same_v<T, std::string>) quote(out, v);
        else if constexpr(std::is_same_v<T,std::shared_ptr<PropertyStorage>>){
            if(!v)throw std::runtime_error("null Property storage in supplemental font-method observer");
            if(v->array){out.put('[');for(std::size_t i=0;i<v->arrayValues.size();++i){if(i)out.put(',');if(i>=v->arrayPresent.size()||!v->arrayPresent[i])out<<"{\"$\":\"hole\"}";else prop(out,v->arrayValues[i]);}out.put(']');}
            else {auto record=PropertyList::record(p);if(!record)throw std::runtime_error("invalid object property record");out.put('{');auto keys=record->keys();std::sort(keys.begin(),keys.end());for(std::size_t i=0;i<keys.size();++i){if(i)out.put(',');quote(out,keys[i]);out.put(':');prop(out,record->getOwn(keys[i]));}out.put('}');}
        }else throw std::runtime_error("unsupported Property in supplemental font-method observer");
    }, p.value);
}
void glyph(std::ostream& out, const Property& value) {
    prop(out,value);
}
Property makeGlyph(double height,double depth,double width,std::string path){
    PropertyArray value;value.resize(4);value.set(0,Property(height));value.set(1,Property(depth));value.set(2,Property(width));
    value.set(3,Property(PropertyList{{"p",Property(std::move(path))}}));return Property(value);
}
template<class F> void observe(std::ostream& out, F&& f) {
    try { f(); }
    catch (const tree::FactoryError& e) {
        out << "{\"error\":{\"name\":"; quote(out, e.name); out << ",\"message\":";
        quote(out, property_host::debugText(e.message)); out << "}}";
    }
}
}

int main() {
    try {
        std::cout << "{\"results\":{";
        std::cout << "\"direction\":{\"none\":"; quote(std::cout, DIRECTION.None);
        std::cout << ",\"vertical\":"; quote(std::cout, DIRECTION.Vertical);
        std::cout << ",\"horizontal\":"; quote(std::cout, DIRECTION.Horizontal);
        std::cout << ",\"vAlias\":" << (V == DIRECTION.Vertical ? "true" : "false")
                  << ",\"hAlias\":" << (H == DIRECTION.Horizontal ? "true" : "false") << "},";

        CharMap addMap;
        auto a = makeGlyph(1,0,.5,"M0 0");
        auto b = makeGlyph(1,0,.5,"M0 0");
        PropertyList::record(property_host::get(b,"3")).value().setOwn("keep",Property(std::string("old")));
        addMap.emplace(65,a); addMap.emplace(66,b);
        const auto existingOptions=property_host::get(b,"3");
        const CharStringMap paths{{65,"M65"},{66,"M66"}}, content{{65,"A"},{66,"B"}};
        auto* mapAddress=&AddPaths(addMap,paths,content);
        std::cout << "\"addPaths\":{\"sameMap\":" << (mapAddress==&addMap?"true":"false") << ",\"chars\":{";
        bool first=true;
        for (const auto& [cp,value] : addMap) {
            const auto* ref=std::get_if<Property>(&value);
            if (!ref) throw std::runtime_error("AddPaths observer found non-glyph");
            if (!first) std::cout.put(','); first=false;
            quote(std::cout,std::to_string(cp)); std::cout.put(':'); glyph(std::cout,*ref);
        }
        std::cout << "},\"sameExistingOptions\":"
                  << (property_host::get(b,"3")==existingOptions?"true":"false") << "},";
        CharMap malformedMap{{65,Property(PropertyList{})}};
        std::cout << "\"addPathsMalformed\":";
        observe(std::cout,[&]{AddPaths(malformedMap,CharStringMap{{65,"Mbad"}},{});std::cout<<"null";});
        std::cout << ',';

        math_options::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        FontDataContextBindings identities(context);
        math_options::MapContextBindings maps(context);
        auto fontClassOwner=std::make_shared<NewcmFontDataClass>(context);
        auto queue=std::make_shared<async::JobQueue>();
        auto hostOwner=std::make_shared<FontLoadHost>();
        auto& host=*hostOwner;host.queue=queue; host.asyncIsSynchronous=[] { return false; };
        (void)registerNewcmSourceClass(context,functions,properties,identities,maps,hostOwner,fontClassOwner);
        auto& fontClass=*fontClassOwner;
        NewcmFontData font(fontClass,host,context);
        font.createVariant("linked-probe","normal","bold");
        auto tuple=makeGlyph(2,0,.75,"linked-probe");
        font.defineChars("bold",CharMap{{0xE000,tuple}});
        const auto linked=font.getChar("linked-probe",0xE000);
        std::cout << "\"linkedVariant\":{\"record\":";
        if (linked) glyph(std::cout,*linked); else std::cout << "{\"$\":\"undefined\"}";
        std::cout << ",\"sameRecord\":" << (linked && *linked==tuple?"true":"false")
                  << ",\"normalBoldZero\":" << (font.getChar("normal",48)==font.getChar("bold",48)?"true":"false")
                  << ",\"boldItalicZero\":" << (font.getChar("bold",48)==font.getChar("italic",48)?"true":"false") << "},";
        std::cout << "\"missingGlyph\":";
        const auto missing=font.getChar("normal",0x10ffff);
        if (!missing) std::cout << "{\"$\":\"undefined\"}"; else glyph(std::cout,*missing);
        std::cout << ',';

        const std::string file="__font_method_probe__.js";
        DynamicFileDef definition; definition.file=file;
        definition.variants.emplace("normal",DynamicRanges{{0xE100,0xE100,false}});
        definition.delimiters.push_back({0x28,0x28,false});
        auto defs=FontDataClass::defineDynamicFiles({definition});
        auto dynamic=defs.find(file)->second;
        fontClass.dynamicFiles.insert_or_assign(file,dynamic);
        std::vector<std::string> loadedPaths;
        host.asyncIsSynchronous=[] { return true; };
        host.syncLoad=[&](FontData&,const DynamicFileRef&,std::string url){
            loadedPaths.push_back(url);
            if (url!="probe-prefix/"+file) throw std::runtime_error("wrong dynamic path");
            CharMapMap maps; auto dynGlyph=makeGlyph(1,0,.5,"M1 0");
            maps.emplace("normal",CharMap{{0xE100,dynGlyph}});
            DelimiterMap delims; auto d=std::make_shared<DelimiterData>();d->dir="v";d->variants=std::vector<double>{0};d->stretchv=std::vector<double>{0};delims.emplace(0x28,d);
            fontClass.dynamicSetup("",file,std::move(maps),std::move(delims));
        };
        // This class option is the source of dynamicPrefix; set it before constructing
        // the observer instance so all emitted results come from the real path.
        const auto customOptions=context.object(); context.set(options::Value(customOptions),options::Key(u"dynamicPrefix"),options::Value(u"probe-prefix"));
        NewcmFontData dynamicFont(fontClass,host,context,options::Value(customOptions));
        const auto record=dynamicFont.getChar("normal",0xE100);
        std::cout << "\"dynamicSync\":{\"paths\":[";
        for(std::size_t i=0;i<loadedPaths.size();++i){if(i)std::cout.put(',');quote(std::cout,loadedPaths[i]);}
        std::cout << "],\"record\":";
        if(record)glyph(std::cout,*record);else std::cout<<"{\"$\":\"undefined\"}";
        std::cout << ",\"failed\":" << (dynamic->failed?"true":"false")
                  << ",\"setupInstalled\":"; quote(std::cout,dynamic->setup?"function":"undefined");
        std::cout << ",\"delimiter\":{";
        auto delim=dynamicFont.getDelimiter(0x28);
        if(delim){if(auto* d=std::get_if<DelimiterDataRef>(&*delim);d&&*d){std::cout<<"\"dir\":";quote(std::cout,(**d).dir);if((**d).stretchv){std::cout<<",\"stretchv\":[";for(std::size_t i=0;i<(**d).stretchv->size();++i){if(i)std::cout.put(',');number(std::cout,(**d).stretchv->at(i));}std::cout.put(']');}if((**d).variants){std::cout<<",\"variants\":[";for(std::size_t i=0;i<(**d).variants->size();++i){if(i)std::cout.put(',');number(std::cout,(**d).variants->at(i));}std::cout.put(']');}}}
        std::cout << "},\"sizeVariant\":"; if(auto v=dynamicFont.getSizeVariant(0x28,0))quote(std::cout,*v);else std::cout<<"null";
        std::cout << ",\"stretchVariant\":"; if(auto v=dynamicFont.getStretchVariant(0x28,0))quote(std::cout,*v);else std::cout<<"null";std::cout << "},";

        std::cout << "\"invalidQueries\":{\"missingSize\":";
        if(auto v=font.getSizeVariant(0x10ffff,0))quote(std::cout,*v);else std::cout<<"null";
        std::cout << ",\"missingStretch\":";
        observe(std::cout,[&]{auto v=font.getStretchVariant(0x10ffff,0);if(v)quote(std::cout,*v);else std::cout<<"null";});
        std::cout << ",\"missingVariant\":";
        observe(std::cout,[&]{auto v=font.getChar("no-such-variant",65);if(v)glyph(std::cout,*v);else std::cout<<"null";});
        std::cout << ",\"invalidRemap\":";
        if(auto v=font.getRemappedChar("not-a-map",65))quote(std::cout,property_host::debugText(*v));else std::cout<<"{\"$\":\"undefined\"}";
        std::cout << "}}}\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "font-methods native observer: " << e.what() << '\n'; return 2;
    } catch (...) {
        std::cerr << "font-methods native observer: unknown failure\n"; return 3;
    }
}
