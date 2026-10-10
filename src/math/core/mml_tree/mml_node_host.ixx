// Root-owned scalar/native Property adapter. No source evaluation or VM.
export module folia.math.core.mml_tree.mml_node_host;
export import folia.math.core.tree.property;
import folia.math.core.tree.factory_types;
import folia.math.util.js_number;
import folia.math.util.js_numeric_input;
import folia.math.util.utf16;
import std;
export namespace folia::math::mml::host {
[[noreturn]] inline void typeError(std::string_view message){throw tree::FactoryError("TypeError",std::u16string(message.begin(),message.end()));}
inline bool truthy(const Property& p){
    if(p.missing()||std::holds_alternative<PropertyNull>(p.value))return false;
    if(auto b=std::get_if<bool>(&p.value))return *b;
    if(auto n=std::get_if<double>(&p.value))return *n!=0&&!std::isnan(*n);
    if(auto s=std::get_if<std::string>(&p.value))return !s->empty();
    return true;
}
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
inline Property primitive(const Property& p,bool stringHint){
    auto object=PropertyList::object(p);if(!object)return p;
    for(auto key:stringHint?std::array{"toString","valueOf"}:std::array{"valueOf","toString"}){
        auto method=object->get(key);auto fn=std::get_if<BuiltinProperty>(&method.value);if(!fn)continue;
        switch(*fn){
        case BuiltinProperty::toString:return "[object Object]";
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
        constexpr std::array<std::string_view,11> names={"Object","__defineGetter__","__defineSetter__","hasOwnProperty","__lookupGetter__","__lookupSetter__","isPrototypeOf","propertyIsEnumerable","toString","valueOf","toLocaleString"};
        return text("function "+std::string(names[static_cast<std::size_t>(*b)])+"() { [native code] }");
    }typeError("Unbound native string conversion");
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
