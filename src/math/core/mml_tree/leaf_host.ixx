// Root-frozen primitive operations for the concrete MML source patterns.
export module folia.math.core.mml_tree.leaf_host;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.core.mml_tree.mml_node_host;
import folia.math.util.js_numeric_input;
import std;
export namespace folia::math::mml {
using Pattern=std::function<bool(std::u16string_view)>;
namespace host {
inline bool nullish(const Property& p){return p.missing()||std::holds_alternative<PropertyNull>(p.value);}
inline bool isSpace(char16_t c){
    switch(c){case 0x9:case 0xa:case 0xb:case 0xc:case 0xd:case 0x20:case 0xa0:case 0x1680:
    case 0x2000:case 0x2001:case 0x2002:case 0x2003:case 0x2004:case 0x2005:case 0x2006:case 0x2007:case 0x2008:case 0x2009:case 0x200a:
    case 0x2028:case 0x2029:case 0x202f:case 0x205f:case 0x3000:case 0xfeff:return true;default:return false;}
}
inline std::u16string_view trimStart(std::u16string_view s){while(!s.empty()&&isSpace(s.front()))s.remove_prefix(1);return s;}
inline std::u16string_view trim(std::u16string_view s){s=trimStart(s);while(!s.empty()&&isSpace(s.back()))s.remove_suffix(1);return s;}
inline bool onlySpaces(std::u16string_view s){return std::all_of(s.begin(),s.end(),isSpace);}
inline bool relativeLevel(std::u16string_view s){s=trimStart(s);return !s.empty()&&(s.front()==u'+'||s.front()==u'-');}
inline double integer(std::u16string_view s){return js_input::parseInt(s);}
inline Property add(const Property& a,const Property& b){
    auto x=primitive(a,false),y=primitive(b,false);
    if(std::holds_alternative<std::string>(x.value)||std::holds_alternative<std::string>(y.value))return debugText(string(x)+string(y));
    return number(x)+number(y);
}
// Direct .toString(), followed by a source String.match/parseInt operation.
// Unlike String(value), a non-string method result cannot proceed to match().
inline std::u16string methodString(const Property& p){
    tree::TreeCallGuard<MmlNode> guard;
    if(nullish(p))typeError("Cannot call toString on null or undefined");
    if(auto object=PropertyList::object(p)){
        auto method=object->get("toString");auto builtin=std::get_if<BuiltinProperty>(&method.value);
        if(!builtin)typeError("toString is not callable");
        if(*builtin==BuiltinProperty::toString)return u"[object Object]";
        if(*builtin==BuiltinProperty::toLocaleString)return methodString(p);
        // Other bound Object prototype methods return non-strings or throw;
        // source then fails at scriptlevel.match, before touching child state.
        typeError("toString result has no string match method");
    }
    return string(p);
}
// Exactly /^[a-z][a-z0-9]*$/i in the source's non-Unicode regex mode.
inline bool operatorNamePattern(std::u16string_view s){
    auto alpha=[](char16_t c){return(c>=u'a'&&c<=u'z')||(c>=u'A'&&c<=u'Z');};
    if(s.empty()||!alpha(s.front()))return false;
    return std::all_of(s.begin()+1,s.end(),[&](char16_t c){return alpha(c)||(c>=u'0'&&c<=u'9');});
}
// Exactly /^[\uD800-\uDBFF]?.[\u0300-\u036F\u1AB0-\u1ABE\u1DC0-\u1DFF\u20D0-\u20EF]*$/.
// Dot consumes one UTF-16 unit, not a Unicode scalar; no newline is accepted.
inline bool singleCharacterPattern(std::u16string_view s){
    auto marks=[](char16_t c){return(c>=0x300&&c<=0x36f)||(c>=0x1ab0&&c<=0x1abe)||(c>=0x1dc0&&c<=0x1dff)||(c>=0x20d0&&c<=0x20ef);};
    auto match=[&](std::size_t prefix){
        if(prefix>=s.size())return false;auto c=s[prefix];
        if(c==u'\n'||c==u'\r'||c==0x2028||c==0x2029)return false;
        return std::all_of(s.begin()+static_cast<std::ptrdiff_t>(prefix+1),s.end(),marks);
    };
    return match(0)||(!s.empty()&&s.front()>=0xd800&&s.front()<=0xdbff&&match(1));
}
}
}
