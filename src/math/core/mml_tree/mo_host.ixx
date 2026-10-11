// Root native call/iterator bindings for mo.ts, no node-specific algorithms.
export module folia.math.core.mml_tree.mo_host;
export import folia.math.core.mml_tree.leaf_host;
import folia.math.core.mml_tree.mo_types;
import folia.math.util.string;
import folia.math.util.utf16;
import std;
export namespace folia::math::mml::host {
template<class T>T moStatic(const std::function<T()>& f){if(!f)typeError("Missing native constructor static getter");return f();}
inline std::shared_ptr<folia::math::Attributes> moAttributes(MmlNode& n){auto p=n.attributes();if(!p)typeError("Cannot read null MML attributes");return p;}
inline NodeValue moChild(NodeValue,std::size_t){throw Error("Unimplemented",u"Root-frozen MML operator indexed Get");}
inline MmlNode& moRequired(NodeValue,std::string_view){throw Error("Unimplemented",u"Root-frozen MML operator member receiver");}
inline bool moInstance(MmlNode* n,const tree::FactoryClassRef<MmlNode>& c){if(!c||!c->isInstance)typeError("Invalid math constructor for instanceof");return c->isInstance(n);}
inline bool moMatch(const MoPatternHandle& p,std::u16string_view s){if(!p||!*p)typeError("Missing native constructor pattern");return (*p)(s);}
inline std::vector<std::string> moKeys(const Property& p){
    if(auto r=PropertyList::record(p))return r->keys();
    if(auto s=std::get_if<std::string>(&p.value)){std::vector<std::string> keys;for(std::size_t i=0;i<text(*s).size();++i)keys.push_back(std::to_string(i));return keys;}
    if(nullish(p))typeError("Cannot enumerate null property value");return {};
}
// Source destructuring accepts an array iterator or a primitive string iterator.
// Array slots are read live; string iteration uses source code points.
inline std::vector<Property> moHead(const Property& p,std::size_t count){
    std::vector<Property> result;
    if(auto a=PropertyArray::array(p)){for(std::size_t i=0;i<count;++i)result.push_back(i<a->size()?a->get(i):Property(Missing{}));return result;}
    if(auto s=std::get_if<std::string>(&p.value)){const auto points=utf16::codePoints(text(*s));for(std::size_t i=0;i<count;++i){if(i>=points.size())result.push_back(Missing{});else {const std::array<double,1> point={static_cast<double>(points[i])};result.push_back(debugText(string_util::unicodeString(point)));}}return result;}
    typeError("Operator spacing value is not iterable");
}
}
