// Root-owned node-array and borrowed service host operations.
export module folia.math.input.tex.stack_item_host;
export import folia.math.input.tex.stack_item;
export import folia.math.core.mml_tree.mml_node_host;
import std;
export namespace folia::math::tex::stack_item_host {
inline mml::Children array(mml::Children nodes,std::string_view member){
    if(!nodes)mml::host::typeError("Cannot read properties of null (reading '"+std::string(member)+"')");
    return nodes;
}
inline mml::Children writeArray(mml::Children nodes,std::string_view member){
    if(!nodes)mml::host::typeError("Cannot set properties of null (setting '"+std::string(member)+"')");
    return nodes;
}
inline std::string key(double index){return mml::host::debugText(mml::host::string(Property(index)));}
template<class Product,class... Alternatives>
Product& require(const std::variant<Alternatives...>& ref,std::string_view member){
    if(auto* p=std::get_if<Product*>(&ref);p&&*p)return **p;
    const bool missing=std::holds_alternative<Missing>(ref);
    if constexpr ((std::is_same_v<options::Value,Alternatives> || ...)) {
        if(const auto* raw=std::get_if<options::Value>(&ref)){
            if(std::holds_alternative<options::Undefined>(raw->data))
                mml::host::typeError("Cannot read properties of undefined (reading '"+std::string(member)+"')");
            if(std::holds_alternative<options::Null>(raw->data))
                mml::host::typeError("Cannot read properties of null (reading '"+std::string(member)+"')");
            mml::host::typeError("Stack source value has no native owner for '"+std::string(member)+"'");
        }
    }
    mml::host::typeError(std::string("Cannot read properties of ")+(missing?"undefined":"null")+" (reading '"+std::string(member)+"')");
}
}
