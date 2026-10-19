module folia.math.core.math_location;
import folia.math.util.property_host;
import std;

namespace folia::math::core {
namespace {
[[noreturn]] void nullishLocationError(const Property& location,std::string_view key){
    const auto kind=location.missing()?"undefined":"null";
    property_host::typeError("Cannot read properties of "+std::string(kind)+
                             " (reading '"+std::string(key)+"')");
}
[[noreturn]] void nullishLocationWriteError(const Property& location,std::string_view key){
    property_host::typeError(std::string("Cannot set properties of ")+
        (location.missing()?"undefined":"null")+" (setting '"+std::string(key)+"')");
}
}

Property readMathLocation(const MathLocation& location,std::string_view key,
                          MathLocationService* service){
    if(const auto* record=std::get_if<Property>(&location)){
        if(record->missing()||std::holds_alternative<PropertyNull>(record->value))
            nullishLocationError(*record,key);
        if(auto object=PropertyList::record(*record))return object->get(key);
        if(std::holds_alternative<FunctionRef>(record->value))
            throw options::Exception("Unimplemented",u"Function-valued MathLocation requires its canonical Context binding");
        if(std::holds_alternative<ContextFunctionIdentity>(record->value))
            throw options::Exception("Unimplemented",u"Context function-valued MathLocation requires its live Context accessor");
        // Reuse the shared primitive-property rules: in particular strings
        // expose UTF-16 length and indexed code units, while ordinary absent
        // named properties still produce Missing.
        return property_host::get(*record,key);
    }
    const auto& native=std::get<NativeObjectRef>(location);
    if(!native)property_host::typeError("Cannot read properties of null (reading '"+
                                        std::string(key)+"')");
    if(!service)
        throw options::Exception("Unimplemented",u"Native MathLocation has no live property accessor");
    return service->get(native,key);
}

void writeMathLocation(const MathLocation& location,std::string_view key,Property value,
                       MathLocationService* service){
    if(const auto* record=std::get_if<Property>(&location)){
        if(record->missing()||std::holds_alternative<PropertyNull>(record->value))
            nullishLocationWriteError(*record,key);
        if(auto object=PropertyList::record(*record)){object->set(key,std::move(value));return;}
        // In the pinned strict-mode CJS module, assignment to a primitive
        // receiver throws. The property name is dynamic in Source.
        const char* type="primitive";
        if(std::holds_alternative<std::string>(record->value))type="string";
        else if(std::holds_alternative<bool>(record->value))type="boolean";
        else if(std::holds_alternative<double>(record->value))type="number";
        else if(std::holds_alternative<options::SymbolRef>(record->value))type="symbol";
        property_host::typeError("Cannot create property '"+std::string(key)+"' on "+
            std::string(type)+
            " '"+property_host::debugText(property_host::string(*record))+"'");
    }
    const auto& native=std::get<NativeObjectRef>(location);
    if(!native)property_host::typeError("Cannot set properties of null (setting '"+
                                        std::string(key)+"')");
    if(!service)
        throw options::Exception("Unimplemented",u"Native MathLocation has no live property accessor");
    service->set(native,key,std::move(value));
}
}
