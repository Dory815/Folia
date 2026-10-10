// Root-owned scalar/native Property adapter. No source evaluation or VM.
export module folia.math.core.mml_tree.mml_node_host;
export import folia.math.core.tree.property;
import folia.math.core.tree.factory_types;
import folia.math.util.js_number;
import folia.math.util.js_numeric_input;
import folia.math.util.utf16;
import std;
export namespace folia::math::mml::host {
inline void inheritanceDefaults(Property& display,Property& level,Property& prime){
    if(display.missing())display=false;
    if(level.missing())level=0;
    if(prime.missing())prime=false;
}
[[noreturn]] inline void typeError(std::string_view message){throw tree::FactoryError("TypeError",std::u16string(message.begin(),message.end()));}
inline bool truthy(const Property& p){
    if(p.missing()||std::holds_alternative<PropertyNull>(p.value))return false;
    if(auto b=std::get_if<bool>(&p.value))return *b;
    if(auto n=std::get_if<double>(&p.value))return *n!=0&&!std::isnan(*n);
    if(auto s=std::get_if<std::string>(&p.value))return !s->empty();
    return true;
}
inline bool nullish(const Property& p){return p.missing()||std::holds_alternative<PropertyNull>(p.value);}
// Property strings and generic debug strings use lossless WTF-8 internally.
// External UTF-8 encode/decode retains the separate strict/replacement contract.
inline std::string debugText(std::u16string_view text){
    std::string out;
    for(auto cp:utf16::codePoints(text)){
        if(cp<=0x7f)out.push_back(static_cast<char>(cp));
        else if(cp<=0x7ff){out.push_back(static_cast<char>(0xc0|(cp>>6)));out.push_back(static_cast<char>(0x80|(cp&63)));}
        else if(cp<=0xffff){out.push_back(static_cast<char>(0xe0|(cp>>12)));out.push_back(static_cast<char>(0x80|((cp>>6)&63)));out.push_back(static_cast<char>(0x80|(cp&63)));}
        else{out.push_back(static_cast<char>(0xf0|(cp>>18)));out.push_back(static_cast<char>(0x80|((cp>>12)&63)));out.push_back(static_cast<char>(0x80|((cp>>6)&63)));out.push_back(static_cast<char>(0x80|(cp&63)));}
    }return out;
}
inline std::u16string text(std::string_view bytes){
    // Decode each sequence strictly, except the explicitly permitted WTF-8
    // surrogate sequence. This is not a permissive external input decoder.
    std::u16string out;
    for(std::size_t i=0;i<bytes.size();){
        auto lead=static_cast<unsigned char>(bytes[i]);
        std::size_t n=lead<0x80?1:lead<0xe0?2:lead<0xf0?3:4;
        if(n>bytes.size()-i)typeError("Invalid native string encoding");
        auto b=[&](std::size_t j){return static_cast<unsigned char>(bytes[i+j]);};
        if(n==3&&lead==0xed&&b(1)>=0xa0&&b(1)<=0xbf&&b(2)>=0x80&&b(2)<=0xbf){
            out.push_back(static_cast<char16_t>(((lead&15)<<12)|((b(1)&63)<<6)|(b(2)&63)));
        }else{
            try{out+=utf16::decode(bytes.substr(i,n)).text;}catch(const utf16::DecodeError&){typeError("Invalid native string encoding");}
        }i+=n;
    }return out;
}
inline std::u16string string(const Property&);
inline double number(const Property&);
inline Property get(const Property&,std::string_view);
std::vector<Property>& activeArrays();
inline std::u16string arrayString(Property receiver){
    auto& active=activeArrays();
    if(std::find(active.begin(),active.end(),receiver)!=active.end())return {};
    struct Reset{std::vector<Property>& active;std::size_t size;~Reset(){active.resize(size);}} reset{active,active.size()};
    auto length=[](const Property& p){auto n=number(get(p,"length"));return std::isnan(n)||n<=0?std::size_t(0):static_cast<std::size_t>(std::min(std::floor(n),9007199254740991.0));};
    struct Frame{Property receiver;std::size_t length,index=0;};
    std::vector<Frame> frames{{receiver,length(receiver)}};active.push_back(receiver);std::u16string out;
    while(!frames.empty()){
        auto& f=frames.back();if(f.index==f.length){frames.pop_back();active.pop_back();continue;}
        const auto i=f.index++;if(i)out.push_back(u',');auto value=get(f.receiver,std::to_string(i));
        if(value.missing()||std::holds_alternative<PropertyNull>(value.value))continue;
        if(auto nested=PropertyArray::array(value);nested&&get(value,"toString")==Property(BuiltinProperty::arrayToString)&&get(value,"join")==Property(BuiltinProperty::arrayJoin)){
            if(std::find(active.begin(),active.end(),value)!=active.end())continue;
            frames.push_back({value,nested->size()});active.push_back(value);
        }else out+=string(value);
    }return out;
}
inline Property primitive(const Property& p,bool stringHint){
    if(p==Property(BuiltinProperty::arrayPrototype))return "";
    if(p==Property(BuiltinProperty::numberPrototype))return 0;
    auto object=PropertyList::record(p);if(!object)return p;
    for(auto key:stringHint?std::array{"toString","valueOf"}:std::array{"valueOf","toString"}){
        auto method=object->get(key);auto fn=std::get_if<BuiltinProperty>(&method.value);if(!fn)continue;
        switch(*fn){
        case BuiltinProperty::toString:return PropertyArray::array(p)?"[object Array]":"[object Object]";
        case BuiltinProperty::arrayToString:{
            auto join=object->get("join");
            if(join==Property(BuiltinProperty::arrayJoin))return debugText(arrayString(p));
            if(join==Property(BuiltinProperty::toString)||!std::holds_alternative<BuiltinProperty>(join.value))return PropertyArray::array(p)?"[object Array]":"[object Object]";
            typeError("Unbound native array join callback");
        }
        case BuiltinProperty::arrayJoin:return debugText(arrayString(p));
        case BuiltinProperty::hasOwnProperty:return object->hasOwn("undefined");
        case BuiltinProperty::propertyIsEnumerable:return object->hasOwn("undefined");
        case BuiltinProperty::isPrototypeOf:return false;
        case BuiltinProperty::lookupGetter:case BuiltinProperty::lookupSetter:return Missing{};
        case BuiltinProperty::defineGetter:case BuiltinProperty::defineSetter:typeError("Invalid accessor callback");
        case BuiltinProperty::toLocaleString:{auto inner=object->get("toString");if(inner==Property(BuiltinProperty::toString))return "[object Object]";typeError("Unbound toLocaleString method");}
        default:break; // Object()/valueOf return objects; not primitive results.
        }
    }typeError("Cannot convert native object to primitive value");
}
inline std::u16string string(const Property& input){
    auto p=primitive(input,true);
    if(p.missing())return u"undefined";
    if(std::holds_alternative<PropertyNull>(p.value))return u"null";
    if(auto b=std::get_if<bool>(&p.value))return *b?u"true":u"false";
    if(auto n=std::get_if<double>(&p.value))return text(js_number::toString(*n));
    if(auto s=std::get_if<std::string>(&p.value))return text(*s);
    if(auto b=std::get_if<BuiltinProperty>(&p.value)){
        if(*b==BuiltinProperty::prototype)return u"[object Object]";
        if(*b==BuiltinProperty::arrayPrototype)return u"";
        if(*b==BuiltinProperty::arrayConstructor)return u"function Array() { [native code] }";
        if(*b==BuiltinProperty::arrayToString)return u"function toString() { [native code] }";
        if(*b==BuiltinProperty::arrayJoin)return u"function join() { [native code] }";
        if(*b==BuiltinProperty::numberConstructor)return u"function Number() { [native code] }";
        if(*b==BuiltinProperty::numberToString)return u"function toString() { [native code] }";
        if(*b==BuiltinProperty::numberValueOf)return u"function valueOf() { [native code] }";
        if(*b==BuiltinProperty::numberPrototype)return u"0";
        if(*b==BuiltinProperty::functionPrototype)return u"function () { [native code] }";
        if(*b==BuiltinProperty::functionConstructor)return u"function Function() { [native code] }";
        if(*b==BuiltinProperty::functionCall)return u"function call() { [native code] }";
        if(*b==BuiltinProperty::functionApply)return u"function apply() { [native code] }";
        if(*b==BuiltinProperty::functionBind)return u"function bind() { [native code] }";
        if(*b==BuiltinProperty::functionToString)return u"function toString() { [native code] }";
        constexpr std::array<std::string_view,11> names={"Object","__defineGetter__","__defineSetter__","hasOwnProperty","__lookupGetter__","__lookupSetter__","isPrototypeOf","propertyIsEnumerable","toString","valueOf","toLocaleString"};
        if(static_cast<std::size_t>(*b)>=names.size())typeError("Unbound intrinsic string metadata");
        return text("function "+std::string(names[static_cast<std::size_t>(*b)])+"() { [native code] }");
    }typeError("Unbound native string conversion");
}
// Property access used by source indexed operator/range and ordinary tables.
// Typed arrays expose standard numeric slots/length and bound intrinsics;
// arbitrary descriptors, evaluated functions and iterator overrides are not bindings.
inline Property get(const Property& value,std::string_view key){
    if(value.missing()||std::holds_alternative<PropertyNull>(value.value))typeError("Cannot read property of null or undefined");
    if(auto object=PropertyList::record(value))return object->get(key);
    std::uint32_t index=0;auto [end,error]=std::from_chars(key.data(),key.data()+key.size(),index);
    bool numeric=!key.empty()&&(key.size()==1||key.front()!='0')&&error==std::errc{}&&end==key.data()+key.size()&&index!=0xFFFFFFFFU;
    if(auto array=PropertyArray::array(value)){
        if(numeric)return array->get(index);
        if(key=="length")return static_cast<double>(array->size());
        if(key=="constructor")return BuiltinProperty::arrayConstructor;
        if(key=="toString")return BuiltinProperty::arrayToString;
        if(key=="join")return BuiltinProperty::arrayJoin;
        if(key=="__proto__")return BuiltinProperty::arrayPrototype;
    }else if(auto textValue=std::get_if<std::string>(&value.value)){
        auto units=text(*textValue);
        if(key=="length")return static_cast<double>(units.size());
        if(numeric)return index<units.size()?Property(debugText(std::u16string(1,units[index]))):Property(Missing{});
    }else if(std::holds_alternative<double>(value.value)){
        if(numeric)return Missing{};
        if(key=="__proto__")return BuiltinProperty::numberPrototype;
        if(key=="constructor")return BuiltinProperty::numberConstructor;
        if(key=="toString")return BuiltinProperty::numberToString;
        if(key=="valueOf")return BuiltinProperty::numberValueOf;
    }else if(auto builtin=std::get_if<BuiltinProperty>(&value.value)){
        if(numeric)return Missing{};
        if(*builtin==BuiltinProperty::prototype)return key=="__proto__"?Property(PropertyNull{}):PropertyList{}.get(key);
        if(*builtin==BuiltinProperty::arrayPrototype)return key=="__proto__"?Property(BuiltinProperty::prototype):get(Property(PropertyArray{}),key);
        if(*builtin==BuiltinProperty::numberPrototype){
            if(key=="__proto__")return BuiltinProperty::prototype;
            if(key=="constructor")return BuiltinProperty::numberConstructor;
            if(key=="toString")return BuiltinProperty::numberToString;
            if(key=="valueOf")return BuiltinProperty::numberValueOf;
            return PropertyList{}.get(key);
        }
        // All remaining identities bind pristine native functions. Their
        // string-key prototype members are observable through malformed Mo
        // form names; they are metadata identities, never executable JS.
        if(key=="caller"||key=="arguments")typeError("Restricted function property");
        if(key=="__proto__")return *builtin==BuiltinProperty::functionPrototype
            ?BuiltinProperty::prototype:BuiltinProperty::functionPrototype;
        if(key=="constructor")return BuiltinProperty::functionConstructor;
        if(key=="call")return BuiltinProperty::functionCall;
        if(key=="apply")return BuiltinProperty::functionApply;
        if(key=="bind")return BuiltinProperty::functionBind;
        if(key=="toString")return BuiltinProperty::functionToString;
        if(key=="prototype"&&*builtin==BuiltinProperty::constructor)return BuiltinProperty::prototype;
        if(key=="prototype"&&*builtin==BuiltinProperty::arrayConstructor)return BuiltinProperty::arrayPrototype;
        if(key=="prototype"&&*builtin==BuiltinProperty::numberConstructor)return BuiltinProperty::numberPrototype;
        if(key=="prototype"&&*builtin==BuiltinProperty::functionConstructor)return BuiltinProperty::functionPrototype;
        if(key=="length"){
            switch(*builtin){
            case BuiltinProperty::defineGetter:case BuiltinProperty::defineSetter:return 2;
            case BuiltinProperty::functionApply:return 2;
            case BuiltinProperty::constructor:case BuiltinProperty::arrayConstructor:case BuiltinProperty::arrayJoin:
            case BuiltinProperty::numberConstructor:case BuiltinProperty::numberToString:
            case BuiltinProperty::functionConstructor:case BuiltinProperty::functionCall:case BuiltinProperty::functionBind:
            case BuiltinProperty::hasOwnProperty:case BuiltinProperty::lookupGetter:case BuiltinProperty::lookupSetter:
            case BuiltinProperty::isPrototypeOf:case BuiltinProperty::propertyIsEnumerable:return 1;
            default:return 0;
            }
        }
        if(key=="name"){
            auto name=string(value);auto start=name.find(u' ')+1;auto stop=name.find(u'(',start);return debugText(name.substr(start,stop-start));
        }
    }else if(numeric)return Missing{};
    if(numeric)return Missing{};
    return PropertyList{}.get(key);
}
inline double number(const Property& input){
    auto p=primitive(input,false);
    if(p.missing())return std::numeric_limits<double>::quiet_NaN();
    if(std::holds_alternative<PropertyNull>(p.value))return 0;
    if(auto b=std::get_if<bool>(&p.value))return *b?1:0;
    if(auto n=std::get_if<double>(&p.value))return *n;
    return js_input::number(string(p));
}
inline bool greaterThanZero(const Property& p){return number(p)>0;}
inline bool sameValueZero(const Property& a,const Property& b){
    if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return *x==*y||(std::isnan(*x)&&std::isnan(*y));
    return a==b;
}
}
