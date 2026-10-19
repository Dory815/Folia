import folia.math.output.common.font_data;
import folia.math.output.common.font_data_surface;
import folia.math.util.options;
import folia.math.util.context_property_bindings;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import folia.math.util.regexp_host;
import folia.math.util.promise;
import std;

namespace {
using namespace folia::math;
using namespace folia::math::options;
namespace common=folia::math::output::common;

void quote(std::ostream& out,std::string_view text){
    out.put('"');
    for(const unsigned char c:text){
        switch(c){
        case '"':out<<"\\\"";break;
        case '\\':out<<"\\\\";break;
        case '\n':out<<"\\n";break;
        case '\r':out<<"\\r";break;
        case '\t':out<<"\\t";break;
        default:if(c<0x20){constexpr char hex[]="0123456789abcdef";out<<"\\u00"<<hex[c>>4]<<hex[c&15];}
            else out.put(static_cast<char>(c));
        }
    }
    out.put('"');
}

int run(){
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    common::FontDataContextBindings identities(context);
    MapContextBindings maps(context);
    regexp_host::RegExpBindings regexp(context);
    common::FontDataSourceSurface surface(context,identities,properties,maps,&regexp);
    auto queue=std::make_shared<async::JobQueue>();
    common::FontLoadHost host;host.queue=queue;host.regexp=&regexp;
    host.asyncIsSynchronous=[] {return false;};
    auto hostAnchor=std::make_shared<int>(1);
    auto fontClass=std::make_shared<common::FontDataClass>(context);
    const auto constructor=surface.registerBase(fontClass,host,hostAnchor,
        [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
           Context& cx,Value supplied){
            return std::make_shared<common::FontData>(owner,loadHost,cx,std::move(supplied));
        });
    const auto instance=context.construct(constructor);
    auto font=identities.resolve(instance);

    common::FontExtensionData extension;
    extension.name="second-key";
    extension.ranges=std::vector<common::DynamicFileDef>{{"relative.js",{}, {}}};
    (void)font->addExtension(extension,"./second");
    const auto classMapValue=context.get(Value(*fontClass->sourceIdentity),Key(u"dynamicExtensions"));
    const auto classMap=std::get<ObjectRef>(classMapValue.data);
    const auto extensionValue=maps.get(classMap,Value(u"second-key"));
    const auto extensionObject=std::get<ObjectRef>(extensionValue.data);
    const auto files=context.get(Value(extensionObject),Key(u"files"));
    const auto fileValue=context.get(files,Key(u"relative.js"));
    const auto dynamicObject=std::get<ObjectRef>(fileValue.data);
    const auto dynamic=std::any_cast<common::DynamicFileRef>(&context.payload(dynamicObject));
    if(!dynamic||!*dynamic)throw std::runtime_error("missing actual generated DynamicFile owner");

    std::vector<std::string> firstOrder;
    const auto oldExtension=context.object();
    context.define(oldExtension,Key(u"prefix"),Descriptor::data(Value(u"./old"),true,true,true));
    (void)maps.set(classMap,Value(u"extension"),Value(oldExtension));
    const auto originalMapGet=context.function(u"Map.get",
        [&maps,&firstOrder](Context&,Value receiver,std::span<const Value> args)->Value{
            const auto* text=args.empty()?nullptr:std::get_if<std::u16string>(&args.front().data);
            firstOrder.push_back("map.invoke:"+(text?property_host::debugText(*text):"undefined"));
            return maps.get(std::get<ObjectRef>(receiver.data),
                args.empty()?Value(Undefined{}):args.front());
        });
    const auto originalMapCallGetter=context.function(u"get function.call",
        [](Context&,Value,std::span<const Value>)->Value{
            throw Exception("Error",u"unexpected function.call property read");
        });
    context.define(originalMapGet,Key(u"call"),Descriptor::access(originalMapCallGetter,{},true,true));
    const auto originalMapGetGetter=context.function(u"get Map.get",
        [&firstOrder,originalMapGet](Context&,Value,std::span<const Value>)->Value{
            firstOrder.push_back("map.get");return Value(originalMapGet);
        });
    context.define(classMap,Key(u"get"),Descriptor::access(originalMapGetGetter,{},true,true));

    std::size_t firstExtensionReads=0,firstFileReads=0;
    const auto customFile=context.object();
    const auto matchGetter=context.function(u"get match",
        [&context,&firstOrder](Context&,Value,std::span<const Value>)->Value{
            firstOrder.push_back("file.match.get");
            return Value(context.function(u"match",
                [&firstOrder](Context&,Value,std::span<const Value>)->Value{
                    firstOrder.push_back("file.match.invoke");return Value(false);
                }));
        });
    const auto replaceGetter=context.function(u"get replace",
        [&context,&firstOrder](Context&,Value,std::span<const Value>)->Value{
            firstOrder.push_back("file.replace.get");
            return Value(context.function(u"replace",
                [&firstOrder](Context&,Value,std::span<const Value>)->Value{
                    firstOrder.push_back("file.replace.invoke");return Value(u"custom.js");
                }));
        });
    context.define(customFile,Key(u"match"),Descriptor::access(matchGetter,{},true,true));
    context.define(customFile,Key(u"replace"),Descriptor::access(replaceGetter,{},true,true));
    const auto firstExtensionGetter=context.function(u"get extension",
        [&firstOrder,&firstExtensionReads](Context&,Value,std::span<const Value>)->Value{
            firstOrder.push_back("extension.get:"+std::to_string(++firstExtensionReads));
            return Value(u"extension");
        });
    const auto firstFileGetter=context.function(u"get file",
        [&firstOrder,&firstFileReads,customFile](Context&,Value,std::span<const Value>)->Value{
            firstOrder.push_back("file.get:"+std::to_string(++firstFileReads));return Value(customFile);
        });
    context.define(dynamicObject,Key(u"extension"),Descriptor::access(firstExtensionGetter,{},true,true));
    context.define(dynamicObject,Key(u"file"),Descriptor::access(firstFileGetter,{},true,true));
    std::string firstResult,firstErrorName,firstErrorMessage;
    try{
        const auto value=font->dynamicFileName(Value(*(*dynamic)->sourceIdentity));
        firstResult=property_host::debugText(property_host::string(properties.from(value)));
    }catch(const Exception& error){
        firstErrorName=error.name;firstErrorMessage=property_host::debugText(error.message);
    }
    const auto firstOrderSnapshot=firstOrder;

    // Replace the Source class field with a new Map object. Keep the old Map
    // reference so the output checks that replacing the field did not mutate
    // an alias to the former object.
    const auto oldMap=classMap;
    const auto replacementMap=maps.create();
    const auto newExtension=context.object();
    context.define(newExtension,Key(u"prefix"),Descriptor::data(Value(u"./new"),true,true,true));
    (void)maps.set(replacementMap,Value(u"extension"),Value(newExtension));
    context.set(Value(*fontClass->sourceIdentity),Key(u"dynamicExtensions"),Value(replacementMap));

    const auto replacementFile=context.object();
    const auto replacementMatch=context.function(u"dynamic.file.match",
        [](Context&,Value,std::span<const Value>)->Value{return Value(false);});
    const auto replacementReplace=context.function(u"dynamic.file.replace",
        [](Context&,Value,std::span<const Value>)->Value{return Value(u"new.js");});
    context.define(replacementFile,Key(u"match"),
        Descriptor::data(Value(replacementMatch),true,true,true));
    context.define(replacementFile,Key(u"replace"),
        Descriptor::data(Value(replacementReplace),true,true,true));
    context.define(dynamicObject,Key(u"extension"),
        Descriptor::data(Value(u"extension"),true,true,true));
    context.define(dynamicObject,Key(u"file"),
        Descriptor::data(Value(replacementFile),true,true,true));
    std::string mapReplacementResult,mapReplacementErrorName,mapReplacementErrorMessage;
    try{
        const auto value=font->dynamicFileName(Value(*(*dynamic)->sourceIdentity));
        mapReplacementResult=property_host::debugText(property_host::string(properties.from(value)));
    }
    catch(const Exception& error){mapReplacementErrorName=error.name;mapReplacementErrorMessage=property_host::debugText(error.message);}
    const auto actualMap=context.get(Value(*fontClass->sourceIdentity),Key(u"dynamicExtensions"));
    const bool mapWasReplaced=std::get_if<ObjectRef>(&actualMap.data)&&
        *std::get_if<ObjectRef>(&actualMap.data)==replacementMap;
    const auto oldGetValue=context.get(Value(oldMap),Key(u"get"));
    const auto* oldGet=std::get_if<ObjectRef>(&oldGetValue.data);
    if(!oldGet||context.kind(*oldGet)!=ObjectKind::function)
        throw std::runtime_error("old Map.get is not callable");
    const std::array<Value,1> oldGetArgs{Value(u"extension")};
    const auto retainedOldExtension=context.call(*oldGet,Value(oldMap),oldGetArgs);
    const bool oldMapKeptValue=retainedOldExtension==Value(oldExtension)&&
        context.get(retainedOldExtension,Key(u"prefix")).data==Value(u"./old").data;

    const auto changingExtensionRecord=context.object();
    context.define(changingExtensionRecord,Key(u"prefix"),
        Descriptor::data(Value(u"./second"),true,true,true));
    (void)maps.set(replacementMap,Value(u"second-key"),Value(changingExtensionRecord));
    context.define(dynamicObject,Key(u"file"),
        Descriptor::data(Value(u"relative.js"),true,true,true));
    std::vector<std::string> order;
    std::size_t extensionReads=0;
    const auto extensionGetter=context.function(u"get extension",
        [&order,&extensionReads](Context&,Value,std::span<const Value>)->Value{
            ++extensionReads;order.push_back("extension.get:"+std::to_string(extensionReads));
            return Value(extensionReads==1?u"first-key":u"second-key");
        });
    context.define(dynamicObject,Key(u"extension"),
        Descriptor::access(extensionGetter,{},true,true));

    const auto map=replacementMap;
    const auto mapGetGetter=context.function(u"get Map.get",
        [&order,&maps](Context& cx,Value receiver,std::span<const Value>)->Value{
            order.push_back("map.get");
            const auto callable=cx.function(u"Map.get",
                [&order,&maps](Context&,Value actualReceiver,std::span<const Value> args)->Value{
                    const auto key=args.empty()?Value(Undefined{}):args[0];
                    if(const auto* text=std::get_if<std::u16string>(&key.data))
                        order.push_back("map.invoke:"+property_host::debugText(*text));
                    else order.push_back("map.invoke:<non-string>");
                    return maps.get(std::get<ObjectRef>(actualReceiver.data),key);
                });
            const auto callGetter=cx.function(u"get function.call",
                [&order](Context&,Value,std::span<const Value>)->Value{
                    order.push_back("function.call.get");
                    throw Exception("Error",u"unexpected function.call property read");
                });
            cx.define(callable,Key(u"call"),Descriptor::access(callGetter,{},true,true));
            return Value(callable);
        });
    context.define(map,Key(u"get"),Descriptor::access(mapGetGetter,{},true,true));

    std::string result,errorName,errorMessage;
    try{
        const auto value=font->dynamicFileName(Value(*(*dynamic)->sourceIdentity));
        result=property_host::debugText(property_host::string(properties.from(value)));
    }
    catch(const Exception& error){errorName=error.name;errorMessage=property_host::debugText(error.message);}
    const auto changingOrderSnapshot=order;
    std::cout<<"{\"first\":{\"result\":";
    if(firstErrorName.empty())quote(std::cout,firstResult);else std::cout<<"null";
    std::cout<<",\"error\":";
    if(firstErrorName.empty())std::cout<<"null";else{
        std::cout<<"{\"name\":";quote(std::cout,firstErrorName);std::cout<<",\"message\":";
        quote(std::cout,firstErrorMessage);std::cout<<'}';}
    std::cout<<",\"extensionReads\":"<<firstExtensionReads<<",\"fileReads\":"<<firstFileReads<<",\"order\":[";
    for(std::size_t i=0;i<firstOrderSnapshot.size();++i){if(i)std::cout.put(',');quote(std::cout,firstOrderSnapshot[i]);}
    std::cout<<"]},\"mapReplacement\":{\"replaced\":"<<(mapWasReplaced?"true":"false")
        <<",\"oldMapStillHasOriginal\":"<<(oldMapKeptValue?"true":"false")<<",\"result\":";
    if(mapReplacementErrorName.empty())quote(std::cout,mapReplacementResult);else std::cout<<"null";
    std::cout<<",\"error\":";
    if(mapReplacementErrorName.empty())std::cout<<"null";else{
        std::cout<<"{\"name\":";quote(std::cout,mapReplacementErrorName);std::cout<<",\"message\":";
        quote(std::cout,mapReplacementErrorMessage);std::cout<<'}';}
    std::cout<<"},\"changingExtension\":{\"result\":";
    if(errorName.empty())quote(std::cout,result);else std::cout<<"null";
    std::cout<<",\"error\":";if(errorName.empty())std::cout<<"null";else{
        std::cout<<"{\"name\":";quote(std::cout,errorName);std::cout<<",\"message\":";
        quote(std::cout,errorMessage);std::cout<<'}';}
    std::cout<<",\"reads\":"<<extensionReads<<",\"order\":[";
    for(std::size_t i=0;i<changingOrderSnapshot.size();++i){if(i)std::cout.put(',');quote(std::cout,changingOrderSnapshot[i]);}
    std::cout<<"]},\"rawReturnIdentity\":{\"resultIsInput\":";
    std::vector<std::string> rawReturnOrder;
    std::size_t rawReturnFileReads=0;
    const auto matchingFile=context.object();
    const auto rawMatchGetter=context.function(u"get match",
        [&context,&rawReturnOrder](Context&,Value,std::span<const Value>)->Value{
            rawReturnOrder.push_back("match.get");
            return Value(context.function(u"match",
                [&rawReturnOrder](Context&,Value,std::span<const Value>)->Value{
                    rawReturnOrder.push_back("match.call");return Value(true);
                }));
        });
    const auto rawReplaceGetter=context.function(u"get replace",
        [&rawReturnOrder](Context&,Value,std::span<const Value>)->Value{
            rawReturnOrder.push_back("replace.get");
            throw Exception("Error",u"replace must not be read");
        });
    context.define(matchingFile,Key(u"match"),Descriptor::access(rawMatchGetter,{},true,true));
    context.define(matchingFile,Key(u"replace"),Descriptor::access(rawReplaceGetter,{},true,true));
    context.define(dynamicObject,Key(u"extension"),Descriptor::data(Value(u"second-key"),true,true,true));
    const auto rawFileGetter=context.function(u"get file",
        [&rawReturnOrder,&rawReturnFileReads,matchingFile](Context&,Value,std::span<const Value>)->Value{
            rawReturnOrder.push_back("file.get:"+std::to_string(++rawReturnFileReads));return Value(matchingFile);
        });
    context.define(dynamicObject,Key(u"file"),Descriptor::access(rawFileGetter,{},true,true));
    bool rawReturnIsInput=false;
    std::string rawReturnErrorName,rawReturnErrorMessage;
    try{
        const auto value=font->dynamicFileName(Value(*(*dynamic)->sourceIdentity));
        rawReturnIsInput=value==Value(matchingFile);
    }catch(const Exception& error){rawReturnErrorName=error.name;rawReturnErrorMessage=property_host::debugText(error.message);}
    std::cout<<(rawReturnIsInput?"true":"false")<<",\"error\":";
    if(rawReturnErrorName.empty())std::cout<<"null";else{
        std::cout<<"{\"name\":";quote(std::cout,rawReturnErrorName);std::cout<<",\"message\":";
        quote(std::cout,rawReturnErrorMessage);std::cout<<'}';}
    std::cout<<",\"fileReads\":"<<rawReturnFileReads<<",\"order\":[";
    for(std::size_t i=0;i<rawReturnOrder.size();++i){if(i)std::cout.put(',');quote(std::cout,rawReturnOrder[i]);}
    std::cout<<"]},\"descriptorReplacement\":";
    auto descriptorDynamic=std::make_shared<common::DynamicFile>();
    descriptorDynamic->extension=Value(u"descriptor");
    descriptorDynamic->file=Value(u"descriptor.js");
    const auto descriptorFile=common::bindDynamicFileSourceObject(*fontClass,descriptorDynamic);
    std::vector<std::string> descriptorOrder;
    const auto descriptorGetter=context.function(u"get DynamicFile.file",
        [&descriptorOrder](Context&,Value,std::span<const Value>)->Value{
            descriptorOrder.push_back("file.get");return Value(u"accessor.js");
        });
    context.define(descriptorFile,Key(u"file"),Descriptor::access(descriptorGetter,{},true,true));
    const auto accessorValue=context.get(Value(descriptorFile),Key(u"file"));
    context.define(descriptorFile,Key(u"file"),Descriptor::data(Value(u"replaced.js"),true,true,true));
    const auto afterDataReplacement=context.get(Value(descriptorFile),Key(u"file"));
    context.erase(descriptorFile,Key(u"file"));
    const bool absentAfterDelete=!context.hasOwn(Value(descriptorFile),Key(u"file"));
    context.set(Value(descriptorFile),Key(u"file"),Value(u"assigned.js"));
    const auto afterAssignment=context.get(Value(descriptorFile),Key(u"file"));
    const auto finalDescriptor=context.descriptor(Value(descriptorFile),Key(u"file"));
    auto emitString=[&](const Value& value){
        const auto* text=std::get_if<std::u16string>(&value.data);
        if(text)quote(std::cout,property_host::debugText(*text));else std::cout<<"null";
    };
    std::cout<<"{\"accessorValue\":";emitString(accessorValue);
    std::cout<<",\"afterDataReplacement\":";emitString(afterDataReplacement);
    std::cout<<",\"deleted\":true,\"absentAfterDelete\":"<<(absentAfterDelete?"true":"false")
        <<",\"afterAssignment\":";emitString(afterAssignment);
    std::cout<<",\"descriptor\":{\"value\":";emitString(finalDescriptor?finalDescriptor->value:Value(Undefined{}));
    std::cout<<",\"writable\":"<<(finalDescriptor&&finalDescriptor->writable?"true":"false")
        <<",\"enumerable\":"<<(finalDescriptor&&finalDescriptor->enumerable?"true":"false")
        <<",\"configurable\":"<<(finalDescriptor&&finalDescriptor->configurable?"true":"false")
        <<"},\"order\":[";
    for(std::size_t i=0;i<descriptorOrder.size();++i){if(i)std::cout.put(',');quote(std::cout,descriptorOrder[i]);}
    std::cout<<"]},\"throwingGetter\":";
    std::vector<std::string> throwingOrder;
    const auto throwingRecord=context.object();
    const auto throwingExtensionGetter=context.function(u"get DynamicFile.extension",
        [&throwingOrder](Context&,Value,std::span<const Value>)->Value{
            throwingOrder.push_back("extension.get");return Value(u"second-key");
        });
    context.define(throwingRecord,Key(u"extension"),Descriptor::access(throwingExtensionGetter,{},true,true));
    const auto throwingFileGetter=context.function(u"get DynamicFile.file",
        [&throwingOrder](Context&,Value,std::span<const Value>)->Value{
            throwingOrder.push_back("file.get");throw Exception("Error",u"file getter failed");
        });
    context.define(throwingRecord,Key(u"file"),Descriptor::access(throwingFileGetter,{},true,true));
    std::string throwingErrorName,throwingErrorMessage;
    try{(void)font->dynamicFileName(Value(throwingRecord));}
    catch(const Exception& error){throwingErrorName=error.name;throwingErrorMessage=property_host::debugText(error.message);}
    std::cout<<"{\"error\":";
    if(throwingErrorName.empty())std::cout<<"null";else{
        std::cout<<"{\"name\":";quote(std::cout,throwingErrorName);std::cout<<",\"message\":";
        quote(std::cout,throwingErrorMessage);std::cout<<'}';}
    std::cout<<",\"order\":[";
    for(std::size_t i=0;i<throwingOrder.size();++i){if(i)std::cout.put(',');quote(std::cout,throwingOrder[i]);}
    std::cout<<"]}}\n";
    return 0;
}
}

int main(){
    try{return run();}
    catch(const options::Exception& error){
        std::cerr<<error.name<<": "<<property_host::debugText(error.message)<<'\n';return 1;
    }
    catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
