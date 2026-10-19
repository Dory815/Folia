// Observe the native FontData instance after the same ordered Jax calls as the
// pinned Source oracle.  All output fields below are read from the live native
// objects; extension data is supplied by the session's canonical owners.
import std;
import folia.math.output.svg.render_session;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
using namespace folia::math::output::common;
using namespace folia::math::output::svg;
using namespace folia::math::options;

void quote(std::ostream& out,std::string_view value){
    out.put('"');
    for(unsigned char c:value){
        switch(c){case '"':out<<"\\\"";break;case '\\':out<<"\\\\";break;
        case '\b':out<<"\\b";break;case '\f':out<<"\\f";break;
        case '\n':out<<"\\n";break;case '\r':out<<"\\r";break;case '\t':out<<"\\t";break;
        default:if(c<0x20){constexpr char hex[]="0123456789abcdef";out<<"\\u00"<<hex[c>>4]<<hex[c&15];}else out.put(static_cast<char>(c));}
    }
    out.put('"');
}
void number(std::ostream& out,double value){
    if(std::isnan(value)){out<<"{\"$\":\"NaN\"}";return;}
    if(std::isinf(value)){out<<(value<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}");return;}
    if(value==0&&std::signbit(value)){out<<"{\"$\":\"-0\"}";return;}
    out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<value;
}
void property(std::ostream&,const Property&);
void propertyArray(std::ostream& out,const PropertyArray& array){
    out<<"{\"$\":\"array\",\"length\":"<<array.size()<<",\"slots\":[";
    for(std::size_t i=0;i<array.size();++i){if(i)out.put(',');if(array.has(i))property(out,array.get(i));else out<<"{\"$\":\"hole\"}";}
    out<<"]}";
}
void property(std::ostream& out,const Property& item){
    std::visit([&](const auto& value){using T=std::decay_t<decltype(value)>;
        if constexpr(std::is_same_v<T,Missing>)out<<"{\"$\":\"undefined\"}";
        else if constexpr(std::is_same_v<T,PropertyNull>)out<<"null";
        else if constexpr(std::is_same_v<T,bool>)out<<(value?"true":"false");
        else if constexpr(std::is_same_v<T,double>)number(out,value);
        else if constexpr(std::is_same_v<T,std::string>)quote(out,value);
        else if constexpr(std::is_same_v<T,std::shared_ptr<PropertyStorage>>){
            if(!value)throw std::runtime_error("null PropertyStorage in native observer");
            Property wrapped;wrapped.value=value;
            if(value->array){auto array=PropertyArray::array(wrapped);if(!array)throw std::runtime_error("missing live PropertyArray view");propertyArray(out,*array);}
            else {auto record=PropertyList::record(wrapped);if(!record)throw std::runtime_error("missing live PropertyList view");out<<"{\"$\":\"object\",\"entries\":[";auto keys=record->keys();for(std::size_t i=0;i<keys.size();++i){if(i)out.put(',');out.put('[');quote(out,keys[i]);out.put(',');property(out,record->getOwn(keys[i]));out.put(']');}out<<"]}";}
        }else throw std::runtime_error("unsupported native Property value");
    },item.value);
}
void value(std::ostream& out,Context& context,const Value& item){
    std::visit([&](const auto& data){using T=std::decay_t<decltype(data)>;
        if constexpr(std::is_same_v<T,Undefined>)out<<"{\"$\":\"undefined\"}";
        else if constexpr(std::is_same_v<T,Null>)out<<"null";
        else if constexpr(std::is_same_v<T,bool>)out<<(data?"true":"false");
        else if constexpr(std::is_same_v<T,double>)number(out,data);
        else if constexpr(std::is_same_v<T,std::u16string>)quote(out,property_host::debugText(data));
        else if constexpr(std::is_same_v<T,ObjectRef>){
            const auto kind=context.kind(data);
            if(kind==ObjectKind::array){const auto length=context.arrayLength(data);out<<"{\"$\":\"array\",\"length\":"<<length<<",\"slots\":[";for(std::uint32_t i=0;i<length;++i){if(i)out.put(',');const auto key=Key(property_host::text(std::to_string(i)));if(context.hasOwn(Value(data),key))value(out,context,context.get(Value(data),key));else out<<"{\"$\":\"hole\"}";}out<<"]}";}
            else if(kind==ObjectKind::function)out<<"{\"$\":\"function\"}";
            else {out<<"{\"$\":\"object\",\"entries\":[";auto keys=context.enumerableStringKeys(Value(data));for(std::size_t i=0;i<keys.size();++i){if(i)out.put(',');out.put('[');quote(out,property_host::debugText(keys[i]));out.put(',');value(out,context,context.get(Value(data),Key(keys[i])));out.put(']');}out<<"]}";}
        }else if constexpr(std::is_same_v<T,SymbolRef>)throw std::runtime_error("Symbol serialization is not part of this candidate profile");
    },item.data);
}
std::vector<std::u16string> stringKeys(Context& context,const Value& object){
    if(std::holds_alternative<Undefined>(object.data)||std::holds_alternative<Null>(object.data))return {};
    return context.enumerableStringKeys(object);
}
void strings(std::ostream& out,const std::vector<std::u16string>& items){out.put('[');for(std::size_t i=0;i<items.size();++i){if(i)out.put(',');quote(out,property_host::debugText(items[i]));}out.put(']');}
void objectMap(std::ostream& out,Context& context,const Value& object){
    const auto objectKeys=stringKeys(context,object);out.put('{');
    for(std::size_t i=0;i<objectKeys.size();++i){if(i)out.put(',');quote(out,property_host::debugText(objectKeys[i]));out.put(':');value(out,context,context.get(object,Key(objectKeys[i])));}
    out.put('}');
}
template<class Read>
void selectedObjectMap(std::ostream& out,Context& context,const std::vector<std::u16string>& names,Read read){
    out.put('{');
    for(std::size_t i=0;i<names.size();++i){if(i)out.put(',');quote(out,property_host::debugText(names[i]));out.put(':');read(names[i]);}
    out.put('}');
}
double parameter(const FontParameters& p,std::string_view key){
    using Pair=std::pair<std::string_view,double FontParameters::*>;
    static constexpr Pair fields[]={
        {"x_height",&FontParameters::x_height},{"quad",&FontParameters::quad},{"num1",&FontParameters::num1},
        {"num2",&FontParameters::num2},{"num3",&FontParameters::num3},{"denom1",&FontParameters::denom1},
        {"denom2",&FontParameters::denom2},{"sup1",&FontParameters::sup1},{"sup2",&FontParameters::sup2},
        {"sup3",&FontParameters::sup3},{"sub1",&FontParameters::sub1},{"sub2",&FontParameters::sub2},
        {"sup_drop",&FontParameters::sup_drop},{"sub_drop",&FontParameters::sub_drop},{"delim1",&FontParameters::delim1},
        {"delim2",&FontParameters::delim2},{"axis_height",&FontParameters::axis_height},{"rule_thickness",&FontParameters::rule_thickness},
        {"big_op_spacing1",&FontParameters::big_op_spacing1},{"big_op_spacing2",&FontParameters::big_op_spacing2},
        {"big_op_spacing3",&FontParameters::big_op_spacing3},{"big_op_spacing4",&FontParameters::big_op_spacing4},
        {"big_op_spacing5",&FontParameters::big_op_spacing5},{"surd_height",&FontParameters::surd_height},
        {"scriptspace",&FontParameters::scriptspace},{"nulldelimiterspace",&FontParameters::nulldelimiterspace},
        {"delimiterfactor",&FontParameters::delimiterfactor},{"delimitershortfall",&FontParameters::delimitershortfall},
        {"rule_factor",&FontParameters::rule_factor},{"min_rule_thickness",&FontParameters::min_rule_thickness},
        {"separation_factor",&FontParameters::separation_factor},{"extra_ic",&FontParameters::extra_ic},
        {"extender_factor",&FontParameters::extender_factor}};
    for(const auto& [name,member]:fields)if(name==key)return p.*member;
    if(const auto found=p.extra.find(std::string(key));found!=p.extra.end()){
        if(const auto* number=std::get_if<double>(&found->second.value))return *number;
        throw std::runtime_error("unsupported native FontParameters extra value");
    }
    throw std::runtime_error("FontParameters field is not represented by the native owner");
}
std::vector<std::u16string> addedVariants(Context& context,const ObjectRef& extension){
    std::vector<std::u16string> result;
    const auto patch=context.get(Value(extension),Key(u"variants"));
    const auto additions=context.get(patch,Key(u"[+]"));
    if(std::holds_alternative<Undefined>(additions.data)||std::holds_alternative<Null>(additions.data))return result;
    context.forEachIterable(additions,[&](Value pair){
        const auto name=context.get(pair,Key(u"0"));
        const auto* text=std::get_if<std::u16string>(&name.data);
        if(!text)throw std::runtime_error("extension variant name is not a string");
        result.push_back(*text);
    });
    return result;
}
Value callMember(Context& context,const ObjectRef& receiver,std::u16string_view name,std::span<const Value> args){
    const auto method=context.get(Value(receiver),Key(std::u16string(name)));
    const auto* function=std::get_if<ObjectRef>(&method.data);
    if(!function||context.kind(*function)!=ObjectKind::function)throw Exception("TypeError",u"member is not callable");
    return context.call(*function,Value(receiver),args);
}
void appendRow(std::ostream& out,NativeRenderSession& session,const ObjectRef& jax,
    const ObjectRef& fontConstructor,std::string_view name,std::string& stage){
    stage="read Jax font";
    auto& context=session.context();auto& services=session.services();
    const auto fontValue=context.get(Value(jax),Key(u"font"));
    const auto* fontObject=std::get_if<ObjectRef>(&fontValue.data);
    if(!fontObject)throw Exception("TypeError",u"Jax font is not an object");
    auto fontOwner=services.fonts().resolve(*fontObject);
    auto classOwner=std::static_pointer_cast<fonts::NewcmFontDataClass>(services.fonts().resolveClass(fontConstructor));

    stage="resolve generated extension payload";
    const auto extension=name=="bboldx"?NewcmFontExtension::bboldx:name=="bbm"?NewcmFontExtension::bbm:NewcmFontExtension::dsfont;
    const auto data=services.extensionObject(extension);
    const std::array<Value,1> args{Value(data)};
    stage="call Jax.addExtension";
    const auto result=callMember(context,jax,u"addExtension",args);
    const auto resultObject=std::get_if<ObjectRef>(&result.data);
    if(!resultObject)throw std::runtime_error("Jax.addExtension did not return its Source array");
    const auto resultLength=context.arrayLength(*resultObject);

    stage="read extension name and dynamic map";
    const auto extensionNameValue=context.get(Value(data),Key(u"name"));
    const auto* extensionName=std::get_if<std::u16string>(&extensionNameValue.data);
    if(!extensionName)throw std::runtime_error("extension name is not a string");
    if(!classOwner->sourceIdentity)throw std::runtime_error("NewCM class has no Source constructor identity");
    const auto dynamicMapValue=context.get(Value(*classOwner->sourceIdentity),Key(u"dynamicExtensions"));
    const auto* mapObject=std::get_if<ObjectRef>(&dynamicMapValue.data);
    if(!mapObject)throw std::runtime_error("dynamicExtensions is not a Map object");
    const std::array<Value,1> mapArgs{Value(*extensionName)};
    const auto dynamicValue=callMember(context,*mapObject,u"get",mapArgs);
    const auto* dynamicObject=std::get_if<ObjectRef>(&dynamicValue.data);
    if(!dynamicObject)throw std::runtime_error("extension dynamic record was not found");
    const Value dynamic(*dynamicObject);

    out<<"{\"extensionName\":";quote(out,name);out<<",\"dynamicKeys\":[";
    auto mapStorage=services.mapBindings().resolve(*mapObject);bool first=true;
    for(const auto& entry:mapStorage->entries())if(entry.live){if(!first)out.put(',');first=false;value(out,context,entry.key);}
    out<<"],\"dynamicRecord\":{";
    const std::array<std::u16string_view,4> fields{u"name",u"prefix",u"sizeN",u"stretchN"};
    for(std::size_t i=0;i<fields.size();++i){if(i)out.put(',');quote(out,property_host::debugText(fields[i]));out.put(':');value(out,context,context.get(dynamic,Key(std::u16string(fields[i]))));}
    out<<",\"fileKeys\":";strings(out,stringKeys(context,context.get(dynamic,Key(u"files"))));
    out<<"},\"sizeVariants\":";property(out,Property(fontOwner->sizeVariants()));out<<",\"stretchVariants\":";property(out,Property(fontOwner->stretchVariants()));

    stage="read merged variants";
    const auto variantNames=addedVariants(context,data);
    out<<",\"addedVariantNames\":";strings(out,variantNames);out<<",\"addedVariants\":{";
    for(std::size_t i=0;i<variantNames.size();++i){if(i)out.put(',');const auto& variant=variantNames[i];quote(out,property_host::debugText(variant));out<<":{";
        const auto found=fontOwner->variants().find(property_host::debugText(variant));
        out<<"\"exists\":"<<(found==fontOwner->variants().end()?"false":"true")<<",\"cacheID\":";
        if(found!=fontOwner->variants().end()&&found->second.cacheID)quote(out,*found->second.cacheID);else out<<"{\"$\":\"undefined\"}";
        out<<'}';}
    stage="read merged VariantSmp";
    const auto smpNames=stringKeys(context,context.get(Value(data),Key(u"variantSmp")));
    out<<"},\"addedSmp\":{";
    for(std::size_t i=0;i<smpNames.size();++i){
        if(i)out.put(',');const auto& name=smpNames[i];quote(out,property_host::debugText(name));out.put(':');
        const auto found=classOwner->VariantSmp.find(property_host::debugText(name));
        if(found==classOwner->VariantSmp.end())throw std::runtime_error("merged Source VariantSmp entry is absent");
        std::visit([&](const auto& item){using T=std::decay_t<decltype(item)>;
            if constexpr(std::is_same_v<T,PropertyArray>)property(out,Property(item));
            else quote(out,item);
        },found->second);
    }
    // Source records are ordinary objects in the observer schema. Serialize
    // their real enumerable key/value pairs directly instead of exposing the
    // PropertyList implementation tag.
    out<<"},\"addedGlyphs\":{";
    stage="read merged glyphs";
    const auto chars=context.get(Value(data),Key(u"chars"));
    const auto charNames=stringKeys(context,chars);
    for(std::size_t i=0;i<charNames.size();++i){
        if(i)out.put(',');const auto& variant=charNames[i];quote(out,property_host::debugText(variant));out<<":{";
        const auto variantChars=context.get(chars,Key(variant));const auto codes=stringKeys(context,variantChars);
        for(std::size_t j=0;j<codes.size();++j){
            if(j)out.put(',');const auto& code=codes[j];quote(out,property_host::debugText(code));out.put(':');
            const auto numeric=std::stod(property_host::debugText(code));
            const auto glyph=fontOwner->getChar(property_host::debugText(variant),numeric);
            if(glyph)property(out,*glyph);else out<<"{\"$\":\"undefined\"}";
        }
        out.put('}');
    }
    out<<"},\"addedDelimiters\":{";
    stage="read merged delimiters";
    const auto delimiterNames=stringKeys(context,context.get(Value(data),Key(u"delimiters")));
    for(std::size_t i=0;i<delimiterNames.size();++i){
        if(i)out.put(',');const auto& code=delimiterNames[i];quote(out,property_host::debugText(code));out.put(':');
        const auto numeric=std::stod(property_host::debugText(code));
        const auto delimiter=fontOwner->getDelimiter(numeric);
        if(!delimiter)out<<"{\"$\":\"undefined\"}";
        else if(const auto* dataValue=std::get_if<DelimiterDataRef>(&*delimiter);dataValue&&*dataValue){
            out<<"{\"$\":\"object\",\"entries\":[[\"dir\",";quote(out,(**dataValue).dir);out<<"]]}";
        }else if(std::holds_alternative<std::monostate>(*delimiter))out<<"null";
        else throw std::runtime_error("extension delimiter remains a dynamic placeholder");
    }
    out<<'}';
    const auto cssNames=stringKeys(context,context.get(Value(data),Key(u"cssFonts")));
    out<<",\"addedCssFonts\":";selectedObjectMap(out,context,cssNames,[&](const auto& key){
        const auto found=fontOwner->cssFonts().find(property_host::debugText(key));
        if(found==fontOwner->cssFonts().end())throw std::runtime_error("merged CSS font is absent");
        property(out,Property(found->second));
    });
    const auto optionNames=stringKeys(context,context.get(Value(data),Key(u"options")));
    out<<",\"addedOptions\":";selectedObjectMap(out,context,optionNames,[&](const auto& key){
        value(out,context,context.get(fontOwner->options(),Key(key)));
    });
    const auto parameterNames=stringKeys(context,context.get(Value(data),Key(u"parameters")));
    out<<",\"addedParameters\":";selectedObjectMap(out,context,parameterNames,[&](const auto& key){
        number(out,parameter(fontOwner->params(),property_host::debugText(key)));
    });
    out<<",\"delimiterNames\":";strings(out,stringKeys(context,context.get(Value(data),Key(u"delimiters"))));
    out<<",\"charNames\":";strings(out,charNames);
    out<<",\"result\":";value(out,context,result);
    out<<",\"resultLength\":"<<resultLength<<'}';
}
}

int main(){
    std::string stage="construct NativeRenderSession";
    try{
        FontLoadHost host;host.asyncIsSynchronous=[] {return false;};host.warn=[](const std::any&){};
        NativeRenderSession session(std::move(host),{});
        stage="construct Jax instance";
        auto jax=session.construct();auto& services=session.services();
        stage="read NewCM constructor and dynamic map";
        const auto fontConstructor=services.newcmFontConstructor();
        auto classOwner=services.fonts().resolveClass(fontConstructor);
        if(!classOwner->sourceIdentity)throw std::runtime_error("NewCM class has no Source constructor identity");
        const auto mapValue=session.context().get(Value(*classOwner->sourceIdentity),Key(u"dynamicExtensions"));
        const auto* initialMap=std::get_if<ObjectRef>(&mapValue.data);
        if(!initialMap)throw std::runtime_error("dynamicExtensions is not a Map object");
        const auto initialStorage=services.mapBindings().resolve(*initialMap);
        std::cout<<"{\"source\":\"native FontData instance extension profile\",\"initialClassKeys\":[";
        bool firstKey=true;
        for(const auto& entry:initialStorage->entries())if(entry.live){
            if(!firstKey)std::cout.put(',');firstKey=false;value(std::cout,session.context(),entry.key);
        }
        std::cout<<"],\"nativeEquivalent\":[";
        const std::array<std::pair<std::string_view,NewcmFontExtension>,3> extensions{{
            {"bboldx",NewcmFontExtension::bboldx},{"bbm",NewcmFontExtension::bbm},{"dsfont",NewcmFontExtension::dsfont}}};
        for(std::size_t i=0;i<extensions.size();++i){
            if(i)std::cout.put(',');
            appendRow(std::cout,session,jax->instanceIdentity(),fontConstructor,extensions[i].first,stage);
        }
        std::cout<<"]}\n";return 0;
    }catch(const Exception& error){
        std::cerr<<"font instance extension observer at "<<stage<<": "<<error.name<<": "
            <<property_host::debugText(error.message)<<'\n';
        return 1;
    }catch(const std::exception& error){std::cerr<<"font instance extension observer at "<<stage<<": "<<error.what()<<'\n';return 1;}
}
