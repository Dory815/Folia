// Focused candidate for the actual CommonOutputJax.getBBox Context bridge.
// Builds a real HTML MathDocument and MathItem through NativeRenderSession;
// it uses no replacement item, output Jax, BBox, or layout implementation.
import std;
import folia.math.core.math_item;
import folia.math.output.svg.native_document;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
using namespace folia::math::options;
namespace svg=folia::math::output::svg;
namespace common=folia::math::output::common;

common::FontLoadHost host(){
    common::FontLoadHost value;
    value.asyncIsSynchronous=[] {return true;};
    value.warn=[](const std::any&){};
    return value;
}
Value member(Context& context,Value receiver,std::u16string_view name){
    return context.get(receiver,Key(std::u16string(name)));
}
std::string quotedAscii(std::u16string_view text){
    std::string result="\"";
    for(const char16_t value:text){
        if(value>0x7f)throw Exception("TypeError",u"Unexpected non-ASCII Source label");
        if(value==u'"'||value==u'\\')result.push_back('\\');
        result.push_back(static_cast<char>(value));
    }
    result.push_back('"');
    return result;
}
std::string quotedAscii(std::string_view text){
    return std::string("\"")+std::string(text)+"\"";
}
}

int main(){
 try{
    svg::NativeRenderSession session(host());
    auto& context=session.context();
    const auto jax=session.construct();
    (void)jax;
    auto document=svg::createNativeSvgDocument(session);
    auto& source=document->sourceContext();
    const auto documentIdentity=document->sourceIdentity();
    const Value documentValue(documentIdentity);
    const auto options=member(source,documentValue,u"options");
    const auto itemConstructor=member(source,options,u"MathItem");
    const auto inputs=member(source,documentValue,u"inputJax");
    const auto input=source.get(inputs,Key(u"0"));
    const std::array<Value,3> itemArgs{Value(u"$x+1$"),input,Value(true)};
    const auto itemIdentity=source.construct(std::get<ObjectRef>(itemConstructor.data),itemArgs);
    const Value itemValue(itemIdentity);
    // Match the Source order. The real typed setter is used because the current
    // HTMLMathDocument Context facade does not expose Source setMetrics.
    const auto* itemPayload=std::any_cast<std::shared_ptr<NativeObject>>(
        &source.payload(itemIdentity));
    const auto nativeItem=itemPayload?
        std::dynamic_pointer_cast<core::MathItem>(*itemPayload):nullptr;
    if(!nativeItem)throw Exception("TypeError",u"Actual MathItem payload is unavailable");
    nativeItem->setMetrics(Property(16),Property(8),Property(640),Property(1));
    const auto compile=member(source,itemValue,u"compile");
    const std::array<Value,1> compileArgs{documentValue};
    (void)source.call(std::get<ObjectRef>(compile.data),itemValue,compileArgs);
    const auto root=member(source,itemValue,u"root");
    if(!std::holds_alternative<ObjectRef>(root.data))
        throw Exception("TypeError",u"Actual MathItem.compile did not create an MML root");

    const auto outputValue=member(source,documentValue,u"outputJax");
    const auto getBBox=member(source,outputValue,u"getBBox");
    auto outputPrototype=source.prototype(std::get<ObjectRef>(outputValue.data));
    std::optional<Descriptor> descriptor;
    while(outputPrototype){
        descriptor=source.descriptor(*outputPrototype,Key(u"getBBox"));
        if(descriptor)break;
        outputPrototype=source.prototype(*outputPrototype);
    }
    const auto* method=std::get_if<ObjectRef>(&getBBox.data);
    if(!method||source.get(*method,Key(u"name"))!=Value(u"")||
       source.get(*method,Key(u"length"))!=Value(2)||!descriptor||
       !descriptor->writable||!descriptor->enumerable||!descriptor->configurable)
        throw Exception("TypeError",u"CommonOutputJax.getBBox Source descriptor mismatch");

    const std::array<Value,2> args{itemValue,documentValue};
    const auto firstValue=source.call(*method,outputValue,args);
    const auto secondValue=source.call(*method,outputValue,args);
    const auto* first=std::get_if<ObjectRef>(&firstValue.data);
    const auto* second=std::get_if<ObjectRef>(&secondValue.data);
    if(!first||!second)throw Exception("TypeError",u"getBBox did not return a Context BBox");
    auto firstOwner=session.services().commonRuntime().bboxes.resolve(*first);
    auto secondOwner=session.services().commonRuntime().bboxes.resolve(*second);
    if(session.services().commonRuntime().bboxes.bind(firstOwner)!=*first||
       session.services().commonRuntime().bboxes.bind(secondOwner)!=*second)
        throw Exception("TypeError",u"BBox binding did not preserve canonical owner identity");
    const auto firstW=source.get(firstValue,Key(u"w"));
    const auto firstH=source.get(firstValue,Key(u"h"));
    const auto firstD=source.get(firstValue,Key(u"d"));
    const auto firstL=source.get(firstValue,Key(u"L"));
    const auto firstR=source.get(firstValue,Key(u"R"));
    const auto firstScale=source.get(firstValue,Key(u"scale"));
    const auto secondW=source.get(secondValue,Key(u"w"));
    const auto secondH=source.get(secondValue,Key(u"h"));
    const auto secondD=source.get(secondValue,Key(u"d"));
    if(firstW!=Value(firstOwner->w)||firstH!=Value(firstOwner->h)||
       firstD!=Value(firstOwner->d))
        throw Exception("TypeError",u"BBox fields do not project the returned owner");
    std::vector<Value> replacementMeasurements;
    bool replacementReceiver=false;
    bool replacementMath=false;
    bool replacementDocument=false;
    const auto replacement=source.function(u"replacement",
        [&](Context&,Value receiver,std::span<const Value> forwarded)->Value{
            replacementReceiver=receiver==outputValue;
            replacementMath=forwarded.size()==2&&forwarded[0]==itemValue;
            replacementDocument=forwarded.size()==2&&forwarded[1]==documentValue;
            if(!replacementReceiver||!replacementMath||!replacementDocument)
                throw Exception("TypeError",u"getBBox replacement received different Source arguments");
            const auto result=source.call(*method,receiver,forwarded);
            replacementMeasurements.push_back(result);
            return result;
        });
    source.set(outputValue,Key(u"getBBox"),Value(replacement));
    const auto liveMethod=std::get<ObjectRef>(source.get(outputValue,Key(u"getBBox")).data);
    const auto thirdValue=source.call(liveMethod,outputValue,args);
    const bool liveReplacementUsed=replacementMeasurements.size()==1&&
        replacementMeasurements.front()==thirdValue;
    if(!liveReplacementUsed)
        throw Exception("TypeError",u"Live outputJax.getBBox replacement was not used");
    const auto thirdW=source.get(thirdValue,Key(u"w"));
    const auto thirdH=source.get(thirdValue,Key(u"h"));
    const auto thirdD=source.get(thirdValue,Key(u"d"));

    const auto throwing=source.function(u"throwing",
        [](Context&,Value,std::span<const Value>)->Value{
            throw Exception("ProbeError",u"getBBox exception sentinel");
        });
    source.set(outputValue,Key(u"getBBox"),Value(throwing));
    bool exceptionPropagated=false;
    try{
        const auto liveThrowingMethod=std::get<ObjectRef>(
            source.get(outputValue,Key(u"getBBox")).data);
        (void)source.call(liveThrowingMethod,outputValue,args);
    }catch(const Exception& error){
        exceptionPropagated=error.name=="ProbeError"&&error.message==u"getBBox exception sentinel";
        if(!exceptionPropagated)throw;
    }
    if(!exceptionPropagated)throw Exception("TypeError",u"getBBox exception was not propagated");

    const auto firstWBefore=firstOwner->w;
    firstOwner->w+=.25;
    const bool liveOwnerField=source.get(firstValue,Key(u"w"))==Value(firstOwner->w);
    if(!liveOwnerField)
        throw Exception("TypeError",u"BBox.w is not a live owner field");

    const auto number=[](const Value& value)->double{
        const auto* result=std::get_if<double>(&value.data);
        if(!result)throw Exception("TypeError",u"Expected numeric BBox field");
        return *result;
    };
    auto& boxes=session.services().commonRuntime().bboxes;
    const bool firstIdentity=boxes.bind(firstOwner)==*first;
    const bool secondIdentity=boxes.bind(secondOwner)==*second;
    const auto constructorName=source.get(itemConstructor,Key(u"name"));
    const auto* constructorNameText=std::get_if<std::u16string>(&constructorName.data);
    const auto* rootNode=nativeItem->root.nodeOrNull();
    if(!constructorNameText||!rootNode)
        throw Exception("TypeError",u"MathItem constructor/root observation is unavailable");
    const auto boolean=[](bool value){return value?"true":"false";};
    const auto writeNumber=[&](const Value& value){std::cout<<std::setprecision(std::numeric_limits<double>::max_digits10)<<number(value);};
    std::cout<<R"({"source":"ts/output/common.ts","sourceSha256":"3686a8e803e26ae59d03c096ac2e6e5b1ab1332a5c4bcaa083cb9a02c2c61096","method":{"name":"","length":2,"writable":)"
             <<boolean(descriptor->writable)<<R"(,"enumerable":)"<<boolean(descriptor->enumerable)
             <<R"(,"configurable":)"<<boolean(descriptor->configurable)
             <<R"(},"item":{"constructor":)"<<quotedAscii(*constructorNameText)
             <<R"(,"root":)"<<quotedAscii(rootNode->kind())<<R"(},"measurements":[{"w":)";
    writeNumber(firstW);std::cout<<R"(,"h":)";writeNumber(firstH);std::cout<<R"(,"d":)";writeNumber(firstD);
    std::cout<<R"(,"L":)";writeNumber(firstL);std::cout<<R"(,"R":)";writeNumber(firstR);
    std::cout<<R"(,"scale":)";writeNumber(firstScale);std::cout<<R"(},{"w":)";
    writeNumber(secondW);std::cout<<R"(,"h":)";writeNumber(secondH);std::cout<<R"(,"d":)";writeNumber(secondD);
    std::cout<<R"(},{"w":)";writeNumber(thirdW);std::cout<<R"(,"h":)";writeNumber(thirdH);
    std::cout<<R"(,"d":)";writeNumber(thirdD);std::cout<<R"(}],"replacementCalls":[{"receiver":)"
             <<boolean(replacementReceiver)<<R"(,"math":)"<<boolean(replacementMath)
             <<R"(,"html":)"<<boolean(replacementDocument)
             <<R"(}],"exceptionPropagated":)"<<boolean(exceptionPropagated)
             <<R"(,"nativeChecks":{"sourceSetMetricsExposed":false,"nativeMathItemSetMetricsUsed":true,"firstCanonical":)"
             <<boolean(firstIdentity)<<R"(,"secondCanonical":)"<<boolean(secondIdentity)
             <<R"(,"firstWidthBefore":)"<<firstWBefore<<R"(,"firstWidthAfter":)"<<firstOwner->w
             <<R"(,"liveField":)"<<boolean(liveOwnerField)<<R"(,"replacementForwarded":)"
             <<boolean(liveReplacementUsed)<<"}}\n";
    return 0;
 }catch(const Exception& error){
    std::cerr<<"getBBox candidate "<<error.name<<": ";
    for(const char16_t ch:error.message)std::cerr<<(ch<0x80?static_cast<char>(ch):'?');
    std::cerr<<'\n';
    return 2;
 }catch(const std::exception& error){
    std::cerr<<"getBBox candidate NativeException: "<<error.what()<<'\n';
    return 2;
 }catch(...){
    std::cerr<<"getBBox candidate unclassified exception\n";
    return 2;
 }
}
