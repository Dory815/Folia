// Native observations paired with parser-projection-original-oracle.cjs and
// parser-options-projection-original-oracle.cjs. Uses the current owner graph.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.textmacros.configuration;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.math_render_actions;
import folia.math.core.math_item;
import folia.math.core.input_jax_host;
import folia.math.core.find_math_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.option_functions;
import folia.math.util.function_list;
import folia.math.util.map_bindings;
import folia.math.util.property_host;
import folia.math.util.native_identities;

namespace {
namespace tex=folia::math::tex;
namespace opts=folia::math::options;
using namespace folia::math;

std::string quote(std::string_view input){
    std::string out="\"";constexpr char hex[]="0123456789abcdef";
    for(const unsigned char c:input){
        switch(c){case '\\':out+="\\\\";break;case '"':out+="\\\"";break;
        case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;
        default:if(c<32){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}else out+=static_cast<char>(c);}
    }
    return out+'"';
}
std::string jsonString(std::u16string_view value){return quote(property_host::debugText(value));}
std::string jsonBool(bool value){return value?"true":"false";}
std::string ownKeys(opts::Context& cx,opts::ObjectRef object){
    std::string result="[";bool first=true;
    for(const auto& name:cx.enumerableStringKeys(opts::Value(object))){
        if(!first)result+=',';first=false;result+=jsonString(name);
    }
    return result+"]";
}
std::string functionInfo(opts::Context& cx,opts::ObjectRef object){
    const auto name=cx.get(object,opts::Key(u"name"));
    const auto length=cx.get(object,opts::Key(u"length"));
    const auto* n=std::get_if<std::u16string>(&name.data);const auto* l=std::get_if<double>(&length.data);
    return "{\"name\":"+jsonString(n?*n:u"")+",\"length\":"+(l?std::to_string(static_cast<int>(*l)):"0")+"}";
}
std::string dataDescriptor(opts::Context& cx,opts::ObjectRef object,std::u16string_view name){
    const auto descriptor=cx.descriptor(opts::Value(object),opts::Key(std::u16string(name)));
    if(!descriptor)return "null";
    const auto* function=std::get_if<opts::ObjectRef>(&descriptor->value.data);
    const bool isFunction=function&&cx.kind(*function)==opts::ObjectKind::function;
    const auto nameValue=isFunction?cx.get(*function,opts::Key(u"name")):opts::Value(opts::Null{});
    const auto lengthValue=isFunction?cx.get(*function,opts::Key(u"length")):opts::Value(opts::Null{});
    const auto* functionName=std::get_if<std::u16string>(&nameValue.data);
    const auto* functionLength=std::get_if<double>(&lengthValue.data);
    return "{\"enumerable\":"+jsonBool(descriptor->enumerable)+",\"writable\":"+
        jsonBool(descriptor->writable)+",\"configurable\":"+jsonBool(descriptor->configurable)+
        ",\"functionName\":"+(isFunction?jsonString(functionName?*functionName:u""):"null")+
        ",\"functionLength\":"+(functionLength?std::to_string(static_cast<int>(*functionLength)):"null")+"}";
}
std::string descriptorTable(opts::Context& cx,opts::ObjectRef object,
        std::span<const std::u16string_view> names){
    std::string result="{";bool first=true;
    for(const auto name:names){if(!first)result+=',';first=false;
        result+=jsonString(name)+":"+dataDescriptor(cx,object,name);}
    return result+"}";
}
struct Fixture{
    opts::Context cx;
    OptionFunctionBindings functions{cx};
    regexp_host::RegExpBindings regex{cx};
    Unicode16Normalization unicode;
    core::InputJaxModule input{cx};
    core::FindMathModule findMath{cx};
    tex::FindTeXModule findTeX{cx,findMath,regex};
    tex::ParserRuntime runtime{cx,functions,regex,unicode,input,findTeX};
    std::shared_ptr<mml::MmlFactory> mmlFactory=std::make_shared<mml::MmlFactory>();
    tex::ParserConfigurationRef configuration;
    tex::ParseOptionsRef parseOptions;
    opts::ObjectRef parseOptionsIdentity;
    bool nodeFactoryConfigurationNativeValueOwnerRetainsReplacement=false;
    Fixture(){
        runtime.initializeParserRuntime();
        configuration=std::make_shared<tex::ParserConfiguration>(runtime);
        const auto* ctor=std::get_if<opts::ObjectRef>(&runtime.parserConfigurationClass.data);
        if(!ctor)throw std::runtime_error("ParserConfiguration constructor is missing");
        (void)runtime.parserConfigurationIdentities.instance(configuration,*ctor);
        configuration->initializeParserConfiguration(cx.array(),opts::Undefined{});
        parseOptions=tex::ParseOptions::make(runtime,*configuration,cx.array());
        const auto identity=runtime.parseOptionsIdentities.find(parseOptions);
        if(!identity)throw std::runtime_error("ParseOptions identity is missing");
        parseOptionsIdentity=*identity;
        (void)runtime.bindMmlFactory(mmlFactory);
        parseOptions->nodeFactoryOwner().setMmlFactory(mmlFactory);
    }
    opts::Value field(std::u16string_view name){return cx.get(parseOptionsIdentity,opts::Key(std::u16string(name)));}
};

struct TextParserListsFixture{
    opts::Context cx;
    OptionFunctionBindings functions{cx};
    regexp_host::RegExpBindings regex{cx};
    Unicode16Normalization unicode;
    core::InputJaxModule input{cx};
    core::FindMathModule findMath{cx};
    tex::FindTeXModule findTeX{cx,findMath,regex};
    tex::ParserRuntime runtime{cx,functions,regex,unicode,input,findTeX};
    std::shared_ptr<mml::MmlFactory> mmlFactory=std::make_shared<mml::MmlFactory>();
    tex::TeXRef jax;
    tex::ParseOptionsRef outer;
    tex::ParseOptionsRef inner;
    tex::TexParserRef outerParser;
    std::shared_ptr<tex::TextParser> textParser;
    opts::ObjectRef outerOptionsIdentity;
    opts::ObjectRef innerIdentity;
    opts::ObjectRef nodeLists;
    TextParserListsFixture(){
        runtime.initializeParserRuntime();
        tex::BasePackage::initializeModule(runtime);
        tex::TextMacrosConfiguration::initializeModule(runtime);
        const auto supplied=cx.object();
        const auto packages=cx.array(2);
        cx.set(packages,opts::Key(u"0"),opts::Value(u"base"));
        cx.set(packages,opts::Key(u"1"),opts::Value(u"textmacros"));
        cx.set(supplied,opts::Key(u"packages"),opts::Value(packages));
        jax=tex::TeX::make(runtime,opts::Value(supplied));
        (void)runtime.bindMmlFactory(mmlFactory);
        jax->setMmlFactory(mmlFactory);
        const auto outerBinding=jax->parseOptions();
        const auto* outerOptions=std::get_if<tex::ParseOptionsRef>(&outerBinding);
        if(!outerOptions||!*outerOptions)throw std::runtime_error("outer TeX ParseOptions are missing");
        outer=*outerOptions;
        outerParser=tex::TexParser::make(u"",PropertyList{},*outer);
        const auto outerIdentity=runtime.parseOptionsIdentities.find(outer);
        if(!outerIdentity)throw std::runtime_error("outer ParseOptions identity is missing");
        outerOptionsIdentity=*outerIdentity;
        const auto packageDataValue=cx.get(*outerIdentity,opts::Key(u"packageData"));
        const auto* packageData=std::get_if<opts::ObjectRef>(&packageDataValue.data);
        if(!packageData)throw std::runtime_error("outer packageData Map identity is missing");
        const auto recordValue=runtime.maps.get(*packageData,opts::Value(u"textmacros"));
        const auto* record=std::get_if<opts::ObjectRef>(&recordValue.data);
        if(!record)throw std::runtime_error("textmacros packageData record is missing");
        const auto innerValue=cx.get(*record,opts::Key(u"parseOptions"));
        const auto* innerFieldIdentity=std::get_if<opts::ObjectRef>(&innerValue.data);
        if(!innerFieldIdentity)throw std::runtime_error("textmacros ParseOptions identity is missing");
        inner=runtime.parseOptionsIdentities.resolve(*innerFieldIdentity);
        const auto identity=runtime.parseOptionsIdentities.find(inner);
        if(!identity)throw std::runtime_error("inner ParseOptions identity is missing");
        innerIdentity=*identity;
        cx.set(*record,opts::Key(u"texParser"),opts::Value(outerParser->instanceIdentity()));
        inner->clear();
        inner->nodeFactoryOwner().setMmlFactory(mmlFactory);
        textParser=tex::TextParser::make(runtime,u"",PropertyList{},*inner);
        const auto lists=cx.get(innerIdentity,opts::Key(u"nodeLists"));
        const auto* object=std::get_if<opts::ObjectRef>(&lists.data);
        if(!object)throw std::runtime_error("inner nodeLists identity is missing");
        nodeLists=*object;
    }
};

void require(bool condition,std::string_view message){if(!condition)throw std::runtime_error(std::string(message));}

opts::Value iteratorResult(opts::Context& cx,bool done,opts::Value value=opts::Undefined{}){
    const auto result=cx.object();
    cx.set(result,opts::Key(u"done"),opts::Value(done));
    if(!done)cx.set(result,opts::Key(u"value"),std::move(value));
    return opts::Value(result);
}

void textParserCopyListsChecks(){
    {
        TextParserListsFixture state;auto& cx=state.cx;auto& runtime=state.runtime;
        const auto node=state.mmlFactory->create("mi",std::span<const std::any>{});
        const auto nodeIdentity=runtime.mmlNodeIdentities.find(node);
        if(!nodeIdentity)throw std::runtime_error("plain-array MML node identity is missing");
        const auto list=cx.array();
        cx.set(list,opts::Key(u"0"),opts::Value(*nodeIdentity));
        cx.set(state.nodeLists,opts::Key(u"mi"),opts::Value(list));
        state.textParser->copyLists();
        const auto outerList=std::get<opts::ObjectRef>(cx.get(state.outerOptionsIdentity,opts::Key(u"nodeLists")).data);
        const auto copied=std::get<opts::ObjectRef>(cx.get(outerList,opts::Key(u"mi")).data);
        const bool copiedActualArray=cx.arrayLength(copied)==1&&
            cx.get(copied,opts::Key(u"0"))==opts::Value(*nodeIdentity);
        const auto after=cx.get(state.innerIdentity,opts::Key(u"nodeLists"));
        require(copiedActualArray&&after!=opts::Value(state.nodeLists),
            "TextParser.copyLists must iterate ordinary node-list arrays and clear only after success");
    }
    {
        TextParserListsFixture state;auto& cx=state.cx;auto& runtime=state.runtime;
        const auto first=state.mmlFactory->create("mi",std::span<const std::any>{});
        const auto second=state.mmlFactory->create("mn",std::span<const std::any>{});
        const auto firstIdentity=runtime.mmlNodeIdentities.find(first);
        const auto secondIdentity=runtime.mmlNodeIdentities.find(second);
        if(!firstIdentity||!secondIdentity)throw std::runtime_error("iterable MML node identity is missing");
        const auto list=cx.array();const auto iterator=cx.object();
        bool replacementNextUsed=false;
        const auto replacementNext=cx.function(u"next",[&](opts::Context& context,opts::Value,std::span<const opts::Value>){
            if(!replacementNextUsed){replacementNextUsed=true;return iteratorResult(context,false,opts::Value(*secondIdentity));}
            return iteratorResult(context,true);
        });
        const auto initialNext=cx.function(u"next",[&](opts::Context& context,opts::Value,std::span<const opts::Value>){
            context.set(iterator,opts::Key(u"next"),opts::Value(replacementNext));
            return iteratorResult(context,false,opts::Value(*firstIdentity));
        });
        cx.set(iterator,opts::Key(u"next"),opts::Value(initialNext));
        const auto iteratorMethod=cx.function(u"iterator",[iterator](opts::Context&,opts::Value,std::span<const opts::Value>){return opts::Value(iterator);});
        cx.set(list,opts::Key(cx.intrinsics().symbolIterator),opts::Value(iteratorMethod));
        cx.set(state.nodeLists,opts::Key(u"mi"),opts::Value(list));
        state.textParser->copyLists();
        const auto outerList=std::get<opts::ObjectRef>(cx.get(state.outerOptionsIdentity,opts::Key(u"nodeLists")).data);
        const auto copied=std::get<opts::ObjectRef>(cx.get(outerList,opts::Key(u"mi")).data);
        require(replacementNextUsed&&cx.arrayLength(copied)==2&&
            cx.get(copied,opts::Key(u"0"))==opts::Value(*firstIdentity)&&
            cx.get(copied,opts::Key(u"1"))==opts::Value(*secondIdentity),
            "TextParser.copyLists must read the iterator next method live while consuming its list");
    }
    {
        TextParserListsFixture state;auto& cx=state.cx;auto& runtime=state.runtime;
        const auto node=state.mmlFactory->create("mi",std::span<const std::any>{});
        const auto nodeIdentity=runtime.mmlNodeIdentities.find(node);
        if(!nodeIdentity)throw std::runtime_error("throwing-iterator MML node identity is missing");
        const auto list=cx.array();const auto iterator=cx.object();
        const auto throwingNext=cx.function(u"next",[](opts::Context&,opts::Value,std::span<const opts::Value>)->opts::Value{
            throw opts::Exception("RangeError",u"node-list iterator failed");
        });
        bool yielded=false;
        const auto initialNext=cx.function(u"next",[&](opts::Context& context,opts::Value,std::span<const opts::Value>){
            if(yielded)return iteratorResult(context,true);
            yielded=true;
            context.set(iterator,opts::Key(u"next"),opts::Value(throwingNext));
            return iteratorResult(context,false,opts::Value(*nodeIdentity));
        });
        cx.set(iterator,opts::Key(u"next"),opts::Value(initialNext));
        const auto iteratorMethod=cx.function(u"iterator",[iterator](opts::Context&,opts::Value,std::span<const opts::Value>){return opts::Value(iterator);});
        cx.set(list,opts::Key(cx.intrinsics().symbolIterator),opts::Value(iteratorMethod));
        cx.set(state.nodeLists,opts::Key(u"mi"),opts::Value(list));
        bool originalException=false;
        try{state.textParser->copyLists();}
        catch(const opts::Exception& error){originalException=error.name=="RangeError"&&error.message==u"node-list iterator failed";}
        const auto outerLists=std::get<opts::ObjectRef>(cx.get(state.outerOptionsIdentity,opts::Key(u"nodeLists")).data);
        const auto copied=std::get<opts::ObjectRef>(cx.get(outerLists,opts::Key(u"mi")).data);
        require(originalException&&cx.arrayLength(copied)==1&&
            cx.get(copied,opts::Key(u"0"))==opts::Value(*nodeIdentity)&&
            cx.get(state.innerIdentity,opts::Key(u"nodeLists"))==opts::Value(state.nodeLists),
            "TextParser.copyLists must preserve earlier additions, propagate iterator exceptions and retain source lists on failure");
    }
}

std::string nodeFactoryObservation(Fixture& f){
    auto& cx=f.cx;auto& runtime=f.runtime;
    const auto ctor=std::get<opts::ObjectRef>(runtime.nodeFactoryClass.data);
    const auto prototype=std::get<opts::ObjectRef>(cx.get(ctor,opts::Key(u"prototype")).data);
    const auto factory=std::get<opts::ObjectRef>(f.field(u"nodeFactory").data);
    const auto bare= cx.construct(ctor,{});
    const auto bareKeys=ownKeys(cx,bare);
    const auto bareHasConfig=cx.hasOwn(opts::Value(bare),opts::Key(u"configuration"));
    const auto staticKeys=ownKeys(cx,ctor);
    const auto prototypeKeys=ownKeys(cx,prototype);
    const bool prototypeConstructorIsClass=cx.get(prototype,opts::Key(u"constructor"))==opts::Value(ctor);
    const auto staticMethod=[&](std::u16string_view name){return std::get<opts::ObjectRef>(cx.get(ctor,opts::Key(std::u16string(name))).data);};
    const auto prototypeMethod=[&](std::u16string_view name){return std::get<opts::ObjectRef>(cx.get(prototype,opts::Key(std::u16string(name))).data);};
    const auto factoryTable=std::get<opts::ObjectRef>(cx.get(bare,opts::Key(u"factory")).data);
    const auto defaultNode=std::get<opts::ObjectRef>(cx.get(factoryTable,opts::Key(u"node")).data);
    const auto defaultToken=std::get<opts::ObjectRef>(cx.get(factoryTable,opts::Key(u"token")).data);
    const auto second=cx.construct(ctor,{});
    const auto secondFactory=std::get<opts::ObjectRef>(cx.get(second,opts::Key(u"factory")).data);
    const auto secondDefaultNode=std::get<opts::ObjectRef>(cx.get(secondFactory,opts::Key(u"node")).data);
    const auto custom=cx.function(u"replacement",[](opts::Context&,opts::Value,std::span<const opts::Value>){return opts::Value(opts::Null{});});
    const auto set=prototypeMethod(u"set"),get=prototypeMethod(u"get");
    const std::array<opts::Value,2> setArgs{opts::Value(u"custom"),opts::Value(custom)};
    (void)cx.call(set,opts::Value(bare),setArgs);
    const std::array<opts::Value,1> getArgs{opts::Value(u"custom")};
    const auto firstCustom=std::get<opts::ObjectRef>(cx.call(get,opts::Value(bare),getArgs).data);
    const std::array<opts::Value,1> getTextArgs{opts::Value(u"text")};
    const auto secondDefaultText=std::get<opts::ObjectRef>(cx.call(get,opts::Value(second),getTextArgs).data);
    const std::array<opts::Value,2> restoreArgs{opts::Value(u"custom"),opts::Value(secondDefaultText)};
    (void)cx.call(set,opts::Value(bare),restoreArgs);
    const auto restored=std::get<opts::ObjectRef>(cx.call(get,opts::Value(bare),getArgs).data);
    const bool replacementRestored=restored==secondDefaultText;
    const auto firstConfiguration=cx.object(),secondConfiguration=cx.object();
    cx.set(bare,opts::Key(u"configuration"),opts::Value(firstConfiguration));
    const bool configurationAssignmentRetainsIdentity=cx.get(bare,opts::Key(u"configuration"))==opts::Value(firstConfiguration);
    const auto configurationDescriptor=cx.descriptor(opts::Value(bare),opts::Key(u"configuration"));
    cx.set(bare,opts::Key(u"configuration"),opts::Value(secondConfiguration));
    const bool configurationReplacementRetainsIdentity=cx.get(bare,opts::Key(u"configuration"))==opts::Value(secondConfiguration);
    const auto bareKeysAfterConfiguration=ownKeys(cx,bare);
    const auto bareOwner=runtime.nodeFactoryIdentities.resolve(bare);
    const auto* configurationValueOwner=std::get_if<opts::Value>(&bareOwner->configuration);
    f.nodeFactoryConfigurationNativeValueOwnerRetainsReplacement=
        configurationValueOwner&&*configurationValueOwner==opts::Value(secondConfiguration);
    const auto assignedConfig=cx.get(factory,opts::Key(u"configuration"));
    std::string error="none";
    try{
        const std::array<opts::Value,2> createArgs{opts::Value(u"text"),opts::Value(u"x")};
        (void)cx.call(prototypeMethod(u"create"),opts::Value(bare),createArgs);
    }catch(const opts::Exception& exception){error=exception.name+"|"+property_host::debugText(exception.message);}
    require(assignedConfig==opts::Value(f.parseOptionsIdentity)&&configurationAssignmentRetainsIdentity&&
        configurationReplacementRetainsIdentity,
        "NodeFactory Context field must retain Source assignment and replacement identities");
    require(!bareHasConfig&&bareKeys=="[\"mmlFactory\",\"factory\"]"&&prototypeConstructorIsClass,
        "bare NodeFactory fields and prototype constructor must match Source");
    require(firstCustom==custom&&replacementRestored,"NodeFactory replacement must remain live and restore the second instance's text default");
    constexpr std::array<std::u16string_view,4> staticNames{u"createNode",u"createToken",u"createText",u"createError"};
    constexpr std::array<std::u16string_view,5> prototypeNames{u"setMmlFactory",u"set",u"setCreators",u"create",u"get"};
    const bool instanceOf=cx.prototype(bare)==std::optional<opts::ObjectRef>(prototype);
    const auto constructorInfo=functionInfo(cx,ctor);
    return "{\"constructor\":{"+constructorInfo.substr(1,constructorInfo.size()-2)+
        ",\"ownKeys\":"+staticKeys+",\"static\":"+descriptorTable(cx,ctor,staticNames)+
        ",\"prototypeKeys\":"+prototypeKeys+",\"prototype\":"+descriptorTable(cx,prototype,prototypeNames)+
        ",\"prototypeConstructorIsClass\":"+jsonBool(prototypeConstructorIsClass)+"},\"instance\":{\"instanceof\":"+
        jsonBool(instanceOf)+",\"ownKeys\":"+bareKeysAfterConfiguration+
        ",\"configurationPresentInitially\":"+jsonBool(bareHasConfig)+
        ",\"configurationAssignmentRetainsIdentity\":"+jsonBool(configurationAssignmentRetainsIdentity)+
        ",\"configurationReplacementRetainsIdentity\":"+jsonBool(configurationReplacementRetainsIdentity)+
        ",\"configurationDescriptor\":{\"enumerable\":"+jsonBool(configurationDescriptor&&configurationDescriptor->enumerable)+
        ",\"writable\":"+jsonBool(configurationDescriptor&&configurationDescriptor->writable)+
        ",\"configurable\":"+jsonBool(configurationDescriptor&&configurationDescriptor->configurable)+"}"
        ",\"fieldDescriptors\":{\"mmlFactory\":"+dataDescriptor(cx,bare,u"mmlFactory")+
        ",\"factory\":"+dataDescriptor(cx,bare,u"factory")+"},\"factoryKeys\":"+ownKeys(cx,factoryTable)+
        ",\"defaultsAreDistinct\":"+jsonBool(defaultNode!=defaultToken)+
        ",\"distinctInstancesHaveDistinctDefaults\":"+jsonBool(defaultNode!=secondDefaultNode)+
        ",\"staticDefaultIdentityShared\":"+jsonBool(defaultNode==staticMethod(u"createNode")&&secondDefaultNode==staticMethod(u"createNode"))+
        ",\"replacementCallableIdentity\":"+jsonBool(firstCustom==custom)+",\"replacementRestored\":"+jsonBool(replacementRestored)+
        ",\"noFactoryError\":"+quote(error)+"}}";
}

std::string parseOptionsObservation(Fixture& f){
    auto& cx=f.cx;auto& runtime=f.runtime;
    const auto initialKeys=ownKeys(cx,f.parseOptionsIdentity);
    const auto noMathItem=cx.hasOwn(opts::Value(f.parseOptionsIdentity),opts::Key(u"mathItem"));
    const auto noTags=cx.hasOwn(opts::Value(f.parseOptionsIdentity),opts::Key(u"tags"));
    const auto parsers=std::get<opts::ObjectRef>(f.field(u"parsers").data);
    const auto parser=cx.object();cx.set(parser,opts::Key(u"marker"),opts::Value(u"parser"));
    const auto push=std::get<opts::ObjectRef>(cx.get(f.parseOptionsIdentity,opts::Key(u"pushParser")).data);
    const std::array<opts::Value,1> pushArgs{opts::Value(parser)};
    (void)cx.call(push,opts::Value(f.parseOptionsIdentity),pushArgs);
    const bool parserIdentity=cx.get(f.parseOptionsIdentity,opts::Key(u"parser"))==opts::Value(parser);
    const auto pop=std::get<opts::ObjectRef>(cx.get(f.parseOptionsIdentity,opts::Key(u"popParser")).data);
    (void)cx.call(pop,opts::Value(f.parseOptionsIdentity),{});
    const bool parserArrayIdentity=f.field(u"parsers")==opts::Value(parsers);
    const bool parserEmptyAfterPop=cx.arrayLength(parsers)==0;
    const auto originalColumn=f.field(u"columnParser");
    const auto replacementColumn=cx.object();cx.set(replacementColumn,opts::Key(u"marker"),opts::Value(u"column parser replacement"));
    cx.set(f.parseOptionsIdentity,opts::Key(u"columnParser"),opts::Value(replacementColumn));
    const bool columnReplacement=f.field(u"columnParser")==opts::Value(replacementColumn);
    const auto packageMap=std::get<opts::ObjectRef>(f.field(u"packageData").data);
    const auto packageRecord=cx.object();f.runtime.maps.set(packageMap,opts::Value(u"parser-projection"),opts::Value(packageRecord));
    const bool mapValueIdentity=f.runtime.maps.get(packageMap,opts::Value(u"parser-projection"))==opts::Value(packageRecord);
    const auto replacementMap=f.runtime.maps.create();cx.set(f.parseOptionsIdentity,opts::Key(u"packageData"),opts::Value(replacementMap));
    const auto typedMap=std::get<std::shared_ptr<SourceStringMap<opts::Value>>>(f.parseOptions->packageData);
    const bool mapReplacementIdentity=typedMap->storage()==runtime.maps.resolve(replacementMap);
    cx.set(f.parseOptionsIdentity,opts::Key(u"packageData"),opts::Value(packageMap));
    const auto mathItemIdentity=cx.object();
    cx.set(mathItemIdentity,opts::Key(u"marker"),opts::Value(u"math item"));
    cx.set(f.parseOptionsIdentity,opts::Key(u"mathItem"),opts::Value(mathItemIdentity));
    const bool mathItemAssignmentIdentity=f.field(u"mathItem")==opts::Value(mathItemIdentity);
    const auto configuredNodeFactory=std::get<opts::ObjectRef>(f.field(u"nodeFactory").data);
    const bool nodeFactoryConfigurationIsParseOptions=
        cx.get(configuredNodeFactory,opts::Key(u"configuration"))==opts::Value(f.parseOptionsIdentity);
    const auto root=f.mmlFactory->create("mrow",std::span<const std::any>{});
    const auto live=f.mmlFactory->create("mi",std::span<const std::any>{});
    const auto detached=f.mmlFactory->create("mi",std::span<const std::any>{});
    root->appendChild(mml::NodeValue(live));
    const auto rootIdentity=runtime.mmlNodeIdentities.find(root);
    if(!rootIdentity)throw std::runtime_error("root MML identity missing");
    cx.set(f.parseOptionsIdentity,opts::Key(u"root"),opts::Value(*rootIdentity));
    f.parseOptions->addNode(Property("mi"),mml::NodeValue(live));
    f.parseOptions->addNode(Property("mi"),mml::NodeValue(detached));
    const auto result=f.parseOptions->getList(u"mi");
    const auto resultIdentity=runtime.nodeArrayIdentities.find(result);
    if(!resultIdentity)throw std::runtime_error("getList did not bind its original ChildArray");
    const auto list=*resultIdentity;
    const auto liveIdentity=runtime.mmlNodeIdentities.find(live);
    const auto detachedIdentity=runtime.mmlNodeIdentities.find(detached);
    if(!liveIdentity||!detachedIdentity)throw std::runtime_error("MML child identity missing");
    const bool liveTreeFiltering=result->size()==1&&result->get(0).nodeOrNull()==live;
    cx.set(list,opts::Key(u"1"),opts::Value(*detachedIdentity));
    const auto nodeLists=std::get<opts::ObjectRef>(f.field(u"nodeLists").data);
    const bool indexedWriteVisible=result->get(1).nodeOrNull()==detached&&
        cx.get(nodeLists,opts::Key(u"mi"))==opts::Value(list);
    auto removal=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    removal->set(0,tree::ChildValue<mml::MmlNode>(detached));
    f.parseOptions->removeFromList(u"mi",removal);
    const bool removalUsesCurrentList=result->size()==1&&result->get(0).nodeOrNull()==live;
    cx.erase(list,opts::Key(u"0"));
    const bool deleteCreatesHole=!cx.hasOwn(list,opts::Key(u"0"))&&cx.arrayLength(list)==1;
    cx.set(list,opts::Key(u"length"),opts::Value(2));
    const bool lengthWriteVisible=result->size()==2&&!cx.hasOwn(list,opts::Key(u"1"));
    const bool columnHadOriginalValue=!std::holds_alternative<opts::Undefined>(originalColumn.data)&&
        !std::holds_alternative<opts::Null>(originalColumn.data);
    require(parserIdentity&&parserArrayIdentity&&parserEmptyAfterPop&&columnReplacement&&mapValueIdentity&&mapReplacementIdentity&&mathItemAssignmentIdentity,
        "live parser, column parser or Map owner projection failed");
    require(liveTreeFiltering&&indexedWriteVisible&&removalUsesCurrentList&&deleteCreatesHole&&lengthWriteVisible,
        "nodeLists must retain its indexed ChildArray owner and array mutations");
    return "{\"constructorKeys\":"+initialKeys+
        ",\"freshFields\":{\"mathItemWasAbsent\":"+jsonBool(!noMathItem)+",\"tagsWereAbsent\":"+jsonBool(!noTags)+
        ",\"nodeFactoryConfigurationIsParseOptions\":"+jsonBool(nodeFactoryConfigurationIsParseOptions)+"},"+
        "\"parsers\":{\"arrayIdentityRetained\":"+jsonBool(parserArrayIdentity)+",\"currentParserIdentity\":"+jsonBool(parserIdentity)+
        ",\"emptyAfterPop\":"+jsonBool(parserEmptyAfterPop)+"},"+
        "\"columnParser\":{\"replacementIdentity\":"+jsonBool(columnReplacement)+
        ",\"hadOriginalValue\":"+jsonBool(columnHadOriginalValue)+"},"+
        "\"packageData\":{\"mapValueIdentity\":"+jsonBool(mapValueIdentity)+
        ",\"mapReplacementIdentity\":"+jsonBool(mapReplacementIdentity)+"},"+
        "\"mathItemAssignmentIdentity\":"+jsonBool(mathItemAssignmentIdentity)+","+
        "\"nodeLists\":{\"liveTreeFiltering\":"+jsonBool(liveTreeFiltering)+
        ",\"indexedWriteVisible\":"+jsonBool(indexedWriteVisible)+
        ",\"deleteCreatesHole\":"+jsonBool(deleteCreatesHole)+",\"lengthWriteVisible\":"+jsonBool(lengthWriteVisible)+
        ",\"removalUsesCurrentList\":"+jsonBool(removalUsesCurrentList)+"}}";
}

void contextNodeFactoryExecutionChecks(){
    Fixture f;auto& cx=f.cx;auto& runtime=f.runtime;
    const auto factory=std::get<opts::ObjectRef>(f.field(u"nodeFactory").data);
    const auto prototype=std::get<opts::ObjectRef>(cx.get(runtime.nodeFactoryClass,opts::Key(u"prototype")).data);
    const auto set=std::get<opts::ObjectRef>(cx.get(prototype,opts::Key(u"set")).data);
    const auto create=std::get<opts::ObjectRef>(cx.get(prototype,opts::Key(u"create")).data);
    const auto get=std::get<opts::ObjectRef>(cx.get(prototype,opts::Key(u"get")).data);
    auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    const auto childrenIdentity=tex::bindNodeArray(runtime,children);
    const auto definition=cx.object();cx.set(definition,opts::Key(u"marker"),opts::Value(u"definition"));
    bool exactCall=false,liveReplacementObserved=false;
    const auto replacement=cx.function(u"replacement",[&](opts::Context&,opts::Value,std::span<const opts::Value>){
        liveReplacementObserved=true;
        const auto node=runtime.mmlNodeIdentities.find(f.mmlFactory->create("mi",std::span<const std::any>{}));
        require(node.has_value(),"replacement MML node must have a canonical Context identity");
        return opts::Value(*node);
    });
    const auto custom=cx.function(u"custom",[&](opts::Context& context,opts::Value receiver,std::span<const opts::Value> arguments){
        const auto* factoryArgument=arguments.size()>0?std::get_if<opts::ObjectRef>(&arguments[0].data):nullptr;
        const auto* kindArgument=arguments.size()>1?std::get_if<std::u16string>(&arguments[1].data):nullptr;
        const auto* childrenArgument=arguments.size()>2?std::get_if<opts::ObjectRef>(&arguments[2].data):nullptr;
        const auto* definitionArgument=arguments.size()>3?std::get_if<opts::ObjectRef>(&arguments[3].data):nullptr;
        exactCall=std::holds_alternative<opts::Undefined>(receiver.data)&&arguments.size()==5&&
            factoryArgument&&*factoryArgument==factory&&kindArgument&&*kindArgument==u"mi"&&
            childrenArgument&&*childrenArgument==childrenIdentity&&definitionArgument&&*definitionArgument==definition&&
            std::holds_alternative<opts::Undefined>(arguments[4].data);
        const auto node=f.mmlFactory->create("mi",std::span<const std::any>{});
        const auto nodeIdentity=runtime.mmlNodeIdentities.find(node);
        require(nodeIdentity.has_value(),"custom NodeFactory result must have a canonical MML identity");
        const auto factoryTable=std::get<opts::ObjectRef>(context.get(factory,opts::Key(u"factory")).data);
        context.set(factoryTable,opts::Key(u"node"),opts::Value(replacement));
        return opts::Value(*nodeIdentity);
    });
    const std::array<opts::Value,2> setCreator{opts::Value(u"node"),opts::Value(custom)};
    (void)cx.call(set,opts::Value(factory),setCreator);
    const auto sourceProperty=runtime.properties.from(opts::Value(custom));
    require(std::get<ContextFunctionIdentity>(sourceProperty.value).object==custom&&
        runtime.properties.to(sourceProperty)==opts::Value(custom),
        "Property must round-trip the original Context callable identity");
    const std::array<opts::Value,5> createArguments{opts::Value(u"node"),opts::Value(u"mi"),
        opts::Value(childrenIdentity),opts::Value(definition),opts::Value(opts::Undefined{})};
    const auto result=cx.call(create,opts::Value(factory),createArguments);
    const auto resultObject=std::get<opts::ObjectRef>(result.data);
    const auto getNodeArguments=std::array<opts::Value,1>{opts::Value(u"node")};
    const auto current=std::get<opts::ObjectRef>(cx.call(get,opts::Value(factory),getNodeArguments).data);
    const auto parseNodeLists=std::get<opts::ObjectRef>(cx.get(f.parseOptionsIdentity,opts::Key(u"nodeLists")).data);
    const auto miNodes=std::get<opts::ObjectRef>(cx.get(parseNodeLists,opts::Key(u"mi")).data);
    require(exactCall&&cx.arrayLength(miNodes)==1&&
        cx.get(miNodes,opts::Key(u"0"))==opts::Value(resultObject)&&
        current==replacement,"NodeFactory must preserve call shape, result identity, addNode, and live replacement");
    const std::array<opts::Value,2> secondCall{opts::Value(u"node"),opts::Value(u"mi")};
    (void)cx.call(create,opts::Value(factory),secondCall);
    require(liveReplacementObserved,"NodeFactory must read the replacement function on its next call");
    const auto throwing=cx.function(u"throwing",[](opts::Context&,opts::Value,std::span<const opts::Value>)->opts::Value{
        throw opts::Exception("RangeError",u"raw creator failure");
    });
    const std::array<opts::Value,2> setThrowing{opts::Value(u"node"),opts::Value(throwing)};
    (void)cx.call(set,opts::Value(factory),setThrowing);
    bool preservedException=false;
    try{(void)cx.call(create,opts::Value(factory),secondCall);}
    catch(const opts::Exception& error){preservedException=error.name=="RangeError"&&error.message==u"raw creator failure";}
    require(preservedException,"NodeFactory must propagate the original Context exception");
    Fixture foreign;
    bool rejectedForeignContext=false;
    try{(void)foreign.runtime.properties.to(sourceProperty);}
    catch(const opts::Exception& error){rejectedForeignContext=error.name=="TypeError";}
    require(rejectedForeignContext,"Context callable identities must reject conversion through a foreign Context");
}

void releasedNodeFactoryLeaseCheck(){
    opts::Context cx;OptionFunctionBindings functions(cx);regexp_host::RegExpBindings regex(cx);
    Unicode16Normalization unicode;core::InputJaxModule input(cx);core::FindMathModule findMath(cx);
    tex::FindTeXModule findTeX(cx,findMath,regex);
    auto runtime=std::make_unique<tex::ParserRuntime>(cx,functions,regex,unicode,input,findTeX);
    runtime->initializeParserRuntime();
    auto configuration=std::make_shared<tex::ParserConfiguration>(*runtime);
    const auto configurationConstructor=std::get<opts::ObjectRef>(runtime->parserConfigurationClass.data);
    (void)runtime->parserConfigurationIdentities.instance(configuration,configurationConstructor);
    configuration->initializeParserConfiguration(cx.array(),opts::Undefined{});
    auto parseOptions=tex::ParseOptions::make(*runtime,*configuration,cx.array());
    const auto parseOptionsIdentity=*runtime->parseOptionsIdentities.find(parseOptions);
    const auto factory=std::get<opts::ObjectRef>(cx.get(opts::Value(parseOptionsIdentity),opts::Key(u"nodeFactory")).data);
    const auto prototype=std::get<opts::ObjectRef>(cx.get(runtime->nodeFactoryClass,opts::Key(u"prototype")).data);
    const auto create=std::get<opts::ObjectRef>(cx.get(prototype,opts::Key(u"create")).data);
    runtime.reset();
    bool rejectedReleasedRuntime=false;
    try{const std::array<opts::Value,1> arguments{opts::Value(u"text")};(void)cx.call(create,opts::Value(factory),arguments);}
    catch(const opts::Exception& error){rejectedReleasedRuntime=error.name=="TypeError"&&error.message==u"Parser runtime has been released";}
    require(rejectedReleasedRuntime,"retained NodeFactory methods must check the runtime lease before accessing borrowed services");
}
}

int main(){
    try{
        Fixture fixture;
        const auto variantMap=std::get<opts::ObjectRef>(fixture.cx.get(fixture.runtime.parseOptionsClass,opts::Key(u"getVariant")).data);
        require(fixture.runtime.maps.resolve(variantMap)==fixture.runtime.variantMap.storage(),"ParseOptions.getVariant must use its canonical MapStorage");
        auto functionList=std::make_shared<FunctionList>();
        const auto functionListIdentity=fixture.runtime.functionLists.bind(functionList);
        require(fixture.runtime.functionLists.resolve(functionListIdentity)==functionList,"FunctionList binding must retain the actual owner");
        Fixture secondContext;
        contextNodeFactoryExecutionChecks();
        releasedNodeFactoryLeaseCheck();
        textParserCopyListsChecks();
        const auto firstBodies=tex::NodeFactory::defaultFunctions();
        const auto secondBodies=tex::NodeFactory::defaultFunctions();
        bool sharedNativeBodies=true;
        for(std::size_t index=0;index<firstBodies.size();++index)
            sharedNativeBodies=sharedNativeBodies&&firstBodies[index]==secondBodies[index];
        const auto firstContextFunction=std::get<opts::ObjectRef>(fixture.cx.get(
            std::get<opts::ObjectRef>(fixture.runtime.nodeFactoryClass.data),opts::Key(u"createNode")).data);
        const auto secondContextFunction=std::get<opts::ObjectRef>(secondContext.cx.get(
            std::get<opts::ObjectRef>(secondContext.runtime.nodeFactoryClass.data),opts::Key(u"createNode")).data);
        const bool contextFunctionIdentityIsDistinct=firstContextFunction!=secondContextFunction;
        fixture.cx.set(firstContextFunction,opts::Key(u"projectionContextMarker"),opts::Value(true));
        const bool contextFunctionPropertiesAreIndependent=
            !secondContext.cx.hasOwn(opts::Value(secondContextFunction),opts::Key(u"projectionContextMarker"));
        require(sharedNativeBodies&&contextFunctionIdentityIsDistinct&&contextFunctionPropertiesAreIndependent,
            "NodeFactory callable bodies must be shared while Context functions/properties remain isolated");
        std::cout<<"{\"nodeFactory\":"<<nodeFactoryObservation(fixture)
            <<",\"parseOptions\":"<<parseOptionsObservation(fixture)
            <<",\"runtimeOwners\":{\"variantMapStorageIdentity\":true,\"functionListOwnerIdentity\":true,\"nodeFactoryConfigurationValueOwnerRetainsReplacement\":"
            <<jsonBool(fixture.nodeFactoryConfigurationNativeValueOwnerRetainsReplacement)
            <<",\"nodeFactoryNativeCallableBodiesShared\":"<<jsonBool(sharedNativeBodies)
            <<",\"nodeFactoryContextFunctionIdentityIsDistinct\":"<<jsonBool(contextFunctionIdentityIsDistinct)
            <<",\"nodeFactoryContextFunctionPropertiesAreIndependent\":"<<jsonBool(contextFunctionPropertiesAreIndependent)<<"}}\n";
        return 0;
    }catch(const NativeIdentityResolutionFailure& error){
        std::cerr<<error.name<<": "<<property_host::debugText(error.message)
            <<"; requested native type="<<error.requestedType<<"; actual payload type="<<error.payloadType<<'\n';return 1;
    }catch(const opts::Exception& error){
        std::cerr<<error.name<<": "<<property_host::debugText(error.message)<<'\n';return 1;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
