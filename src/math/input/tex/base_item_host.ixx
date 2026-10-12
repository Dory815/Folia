// Root-owned typed host operations for Source BaseItem; no reducer algorithms.
export module folia.math.input.tex.base_item_host;
export import folia.math.input.tex.stack_item_host;
export import folia.math.input.tex.tex_error;
export import folia.math.input.tex.tex_constants;
import std;
export namespace folia::math::tex::base_item_host {
inline StackPropertyList properties(const StackProperty& value){
    if(auto record=StackPropertyList::record(value))return *record;
    property_host::typeError("Native stack property record is not initialized");
}
inline BaseItem& item(BaseItem* value,std::string_view member){
    if(!value)property_host::typeError("Cannot read properties of null (reading '"+std::string(member)+"')");
    return *value;
}
inline Property get(const Property& value,std::string_view key){
    if(property_host::nullish(value))property_host::typeError(std::string("Cannot read properties of ")+
        (value.missing()?"undefined":"null")+" (reading '"+std::string(key)+"')");
    return property_host::get(value,key);
}
inline void deleteProperty(const Property& value,std::string_view key){
    if(property_host::nullish(value))property_host::typeError("Cannot convert undefined or null to object");
    if(auto record=PropertyList::record(value)){record->erase(key);return;}
    if(auto* p=std::get_if<std::string>(&value.value)){
        const auto units=property_host::text(*p);
        for(std::size_t i=0;i<units.size();++i)if(key==std::to_string(i))
            property_host::typeError("Cannot delete property '"+std::string(key)+"' of [object String]");
    }
    // Delete absent properties on boxed scalar primitives succeeds.
}
inline std::size_t sliceIndex(double index,std::size_t length){
    const auto integer=std::isnan(index)?0:std::trunc(index);
    const auto offset=integer<0?std::max(static_cast<double>(length)+integer,0.0):std::min(integer,static_cast<double>(length));
    return static_cast<std::size_t>(offset);
}
inline std::u16string slice(std::u16string_view source,double start,double end){
    const auto first=sliceIndex(start,source.size()),last=sliceIndex(end,source.size());
    return std::u16string(source.substr(first,last>first?last-first:0));
}
inline std::string join(mml::Children nodes,std::string_view separator){
    const auto array=stack_item_host::array(std::move(nodes),"join");
    const auto length=array->size();std::string out;
    for(std::size_t i=0;i<length;++i){
        if(i)out+=separator;
        const auto value=array->get(i);
        if(value.isNode())out+=value.nodeOrNull()->toString();
    }
    return out;
}
inline std::shared_ptr<folia::math::Attributes> attributes(mml::NodeValue node){
    const auto attributes=node.requireNode("attributes")->attributes();
    if(!attributes)property_host::typeError("Cannot read properties of null (reading 'set')");
    return attributes;
}
inline Property errorField(const Property& value,std::size_t index){
    // Captured class tables contain ordinary string[] tuples (live PropertyArray).
    if(value.missing())property_host::typeError("undefined is not iterable (cannot read property Symbol(Symbol.iterator))");
    if(std::holds_alternative<PropertyNull>(value.value))property_host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    if(!PropertyArray::array(value))property_host::typeError("Native stack error tuple is not an array");
    return get(value,std::to_string(index));
}
[[noreturn]] inline void texError(Property id,Property message,StackProperty name){
    const auto* text=std::get_if<std::string>(&message.value);
    if(!text){
        if(property_host::nullish(message))property_host::typeError(std::string("Cannot read properties of ")+
            (message.missing()?"undefined":"null")+" (reading 'split')");
        property_host::typeError("message.split is not a function");
    }
    const std::array<Property,1> args{stack_property_host::scalar(name)};
    throw TexError(property_host::string(id),property_host::text(*text),args);
}
}
