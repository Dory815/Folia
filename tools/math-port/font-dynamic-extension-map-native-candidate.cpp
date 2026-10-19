// Observe the real NewCM constructor Map field after replacing it, retaining
// the previous Map alias, and invoking the actual Jax instance extension path.
import std;
import folia.math.output.svg.render_session;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
using namespace folia::math::options;
using namespace folia::math::output::svg;

void quote(std::ostream& out,std::string_view text){
    out.put('"');
    for(unsigned char c:text){
        switch(c){case '"':out<<"\\\"";break;case '\\':out<<"\\\\";break;
        case '\n':out<<"\\n";break;case '\r':out<<"\\r";break;case '\t':out<<"\\t";break;
        default:if(c<0x20){constexpr char hex[]="0123456789abcdef";out<<"\\u00"<<hex[c>>4]<<hex[c&15];}else out.put(static_cast<char>(c));}
    }
    out.put('"');
}
void keys(std::ostream& out,MapContextBindings& maps,ObjectRef object){
    out.put('[');const auto storage=maps.resolve(object);bool first=true;
    for(const auto& entry:storage->entries())if(entry.live){
        if(!first)out.put(',');first=false;
        const auto* name=std::get_if<std::u16string>(&entry.key.data);
        if(!name)throw std::runtime_error("test Map key is not a string");
        quote(out,property_host::debugText(*name));
    }
    out.put(']');
}
Value callMember(Context& context,ObjectRef receiver,std::u16string_view name,
    std::span<const Value> args={}){
    const auto value=context.get(Value(receiver),Key(std::u16string(name)));
    const auto* function=std::get_if<ObjectRef>(&value.data);
    if(!function)throw Exception("TypeError",u"member is not callable");
    return context.call(*function,Value(receiver),args);
}
void value(std::ostream& out,Context& context,const Value& item){
    std::visit([&](const auto& data){using T=std::decay_t<decltype(data)>;
        if constexpr(std::is_same_v<T,Undefined>)out<<"{\"$\":\"undefined\"}";
        else if constexpr(std::is_same_v<T,Null>)out<<"null";
        else if constexpr(std::is_same_v<T,bool>)out<<(data?"true":"false");
        else if constexpr(std::is_same_v<T,double>)out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<data;
        else if constexpr(std::is_same_v<T,std::u16string>)quote(out,property_host::debugText(data));
        else if constexpr(std::is_same_v<T,ObjectRef>){
            if(context.kind(data)!=ObjectKind::array)throw std::runtime_error("unexpected object-valued extension field");
            const auto length=context.arrayLength(data);out<<"{\"$\":\"array\",\"length\":"<<length<<",\"slots\":[";
            for(std::uint32_t i=0;i<length;++i){if(i)out.put(',');const auto key=Key(property_host::text(std::to_string(i)));
                if(context.hasOwn(Value(data),key))value(out,context,context.get(Value(data),key));else out<<"{\"$\":\"hole\"}";}
            out<<"]}";
        }else if constexpr(std::is_same_v<T,SymbolRef>)throw std::runtime_error("unexpected Symbol-valued extension field");
    },item.data);
}
}

int main(){
    try{
        FontLoadHost host;host.asyncIsSynchronous=[] {return false;};
        host.warn=[](const std::any&){};
        NativeRenderSession session(std::move(host),{});
        auto jax=session.construct();auto& context=session.context();auto& services=session.services();
        const auto constructor=services.newcmFontConstructor();
        const auto beforeValue=context.get(Value(constructor),Key(u"dynamicExtensions"));
        const auto* before=std::get_if<ObjectRef>(&beforeValue.data);
        if(!before)throw std::runtime_error("NewCM dynamicExtensions is not a Map object");
        const auto oldMap=*before;
        const auto replacement=services.mapBindings().create();
        const auto marker=context.object();
        context.set(Value(marker),Key(u"kind"),Value(u"replacement-marker"));
        services.mapBindings().set(replacement,Value(u"replacement-marker"),Value(marker));
        context.set(Value(constructor),Key(u"dynamicExtensions"),Value(replacement));
        const auto assigned=context.get(Value(constructor),Key(u"dynamicExtensions"));
        const auto* assignedObject=std::get_if<ObjectRef>(&assigned.data);
        if(!assignedObject)throw std::runtime_error("replacement Map field is not an object");

        const auto oldMarker=context.object();
        context.set(Value(oldMarker),Key(u"kind"),Value(u"old-marker"));
        services.mapBindings().set(oldMap,Value(u"old-marker"),Value(oldMarker));
        const auto extension=services.extensionObject(NewcmFontExtension::bboldx);
        const std::array<Value,1> args{Value(extension)};
        const auto result=callMember(context,jax->instanceIdentity(),u"addExtension",args);
        const auto* resultArray=std::get_if<ObjectRef>(&result.data);
        if(!resultArray||context.kind(*resultArray)!=ObjectKind::array)
            throw std::runtime_error("Jax.addExtension did not return an actual array");

        const auto nameValue=context.get(Value(extension),Key(u"name"));
        const auto* extensionName=std::get_if<std::u16string>(&nameValue.data);
        if(!extensionName)throw std::runtime_error("extension name is not a string");
        const auto entry=services.mapBindings().get(replacement,Value(*extensionName));
        const auto* entryObject=std::get_if<ObjectRef>(&entry.data);
        if(!entryObject)throw std::runtime_error("instance extension did not update replacement Map");
        const auto oldEntry=services.mapBindings().get(oldMap,Value(*extensionName));
        const auto oldMarkerValue=services.mapBindings().get(oldMap,Value(u"old-marker"));
        const auto* oldMarkerObject=std::get_if<ObjectRef>(&oldMarkerValue.data);
        const auto lateOldMarker=context.object();
        context.set(Value(lateOldMarker),Key(u"kind"),Value(u"late-old-marker"));
        services.mapBindings().set(oldMap,Value(u"late-old-marker"),Value(lateOldMarker));
        const auto lateOldValue=services.mapBindings().get(oldMap,Value(u"late-old-marker"));
        const auto* lateOldObject=std::get_if<ObjectRef>(&lateOldValue.data);

        std::cout<<"{\"replacement\":{";
        const auto descriptor=context.descriptor(Value(constructor),Key(u"dynamicExtensions"));
        if(!descriptor)throw std::runtime_error("dynamicExtensions descriptor is missing");
        std::cout<<"\"assignedSameObject\":"<<(*assignedObject==replacement?"true":"false")
            <<",\"replacementDiffersFromPrevious\":"<<(oldMap!=replacement?"true":"false")
            <<",\"descriptor\":{\"writable\":"<<(descriptor->writable?"true":"false")
            <<",\"enumerable\":"<<(descriptor->enumerable?"true":"false")
            <<",\"configurable\":"<<(descriptor->configurable?"true":"false")<<"},\"oldAliasRetainsRecord\":"
            <<(oldMarkerObject&&*oldMarkerObject==oldMarker?"true":"false")
            <<",\"oldAliasRetainsLaterMutation\":"<<(lateOldObject&&*lateOldObject==lateOldMarker?"true":"false")
            <<",\"oldAliasHasExtension\":"<<(std::holds_alternative<Undefined>(oldEntry.data)?"false":"true")
            <<",\"replacementKeys\":";
        keys(std::cout,services.mapBindings(),replacement);
        std::cout<<",\"addExtensionResult\":{\"isArray\":true,\"length\":"
            <<context.arrayLength(*resultArray)<<"},\"insertedRecord\":{\"identity\":true,\"name\":";
        quote(std::cout,property_host::debugText(*extensionName));
        std::cout<<",\"prefix\":";
        const auto prefix=context.get(Value(*entryObject),Key(u"prefix"));
        const auto* prefixText=std::get_if<std::u16string>(&prefix.data);
        if(!prefixText)throw std::runtime_error("inserted dynamic record prefix is not a string");
        quote(std::cout,property_host::debugText(*prefixText));
        std::cout<<",\"sizeN\":";value(std::cout,context,context.get(Value(*entryObject),Key(u"sizeN")));
        std::cout<<",\"stretchN\":";value(std::cout,context,context.get(Value(*entryObject),Key(u"stretchN")));
        std::cout<<",\"keys\":[";
        const auto recordKeys=context.enumerableStringKeys(Value(*entryObject));
        for(std::size_t i=0;i<recordKeys.size();++i){if(i)std::cout.put(',');quote(std::cout,property_host::debugText(recordKeys[i]));}
        std::cout<<"],\"fileKeys\":";
        const auto files=context.get(Value(*entryObject),Key(u"files"));
        const auto* filesObject=std::get_if<ObjectRef>(&files.data);
        if(!filesObject)throw std::runtime_error("inserted dynamic record files is not an object");
        const auto fileKeys=context.enumerableStringKeys(Value(*filesObject));
        std::cout<<"[";for(std::size_t i=0;i<fileKeys.size();++i){if(i)std::cout.put(',');quote(std::cout,property_host::debugText(fileKeys[i]));}std::cout<<"]}}}\n";
        return 0;
    }catch(const Exception& error){
        std::cerr<<"font dynamicExtensions observer: "<<error.name<<": "
            <<property_host::debugText(error.message)<<'\n';return 1;
    }catch(const std::exception& error){std::cerr<<"font dynamicExtensions observer: "<<error.what()<<'\n';return 1;}
}
