/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 input/tex/base/BaseConfiguration.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.input.tex.base_configuration;

import folia.math.input.tex.parser;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.base_parse_methods;
import folia.math.input.tex.base_mappings;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.node_util;
import folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.tex_error;
import folia.math.core.mml_tree.operator_dictionary;
import folia.math.core.input_jax;
import folia.math.util.property_host;
import folia.math.util.regexp_binding;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view text){return Key(std::u16string(text));}
Value arg(std::span<const Value> args,std::size_t i){return i<args.size()?args[i]:Value(Undefined{});}

ObjectRef object(Context& context){return context.object();}
ObjectRef array(Context& context,std::span<const Value> values){
    const ObjectRef result=context.array(values.size());
    for(std::size_t i=0;i<values.size();++i)context.set(result,key(property_host::text(std::to_string(i))),values[i]);
    return result;
}

std::u16string string(ParserRuntime& runtime,const Value& value){return property_host::string(runtime.properties.from(value));}

std::u16string numberPattern(){
    return u"^(?:[0-9]+(?:\\{,\\}[0-9]{3})*(?:\\.[0-9]*)?|\\.[0-9]+)";
}

ObjectRef makeRegexp(ParserRuntime& runtime,std::u16string pattern,std::u16string flags={}){
    return runtime.regexp().create(std::move(pattern),std::move(flags));
}

FunctionRef nonscriptFilter(ParserRuntime& runtime){
    auto function=std::make_shared<Function>();
    function->sourceMetadata=FunctionSourceMetadata{u"filterNonscript",1};
    function->call=[&runtime](const FunctionReceiver&,FunctionArgs args)->std::any{
        ParseOptionsRef parseOptions;
        if(!args.empty()){
            if(const auto* data=std::any_cast<core::InputJaxFilterDataRef>(&args[0]);data&&*data){
                const auto current=(*data)->currentData();
                if(const auto* parser=std::any_cast<ParseOptionsRef>(&current))parseOptions=*parser;
            }else if(const auto* parser=std::any_cast<ParseOptionsRef>(&args[0]))parseOptions=*parser;
        }
        if(!parseOptions)throw Exception("TypeError",u"filterNonscript data is not ParseOptions");
        const mml::Children candidates=parseOptions->getList(u"nonscript");
        const std::size_t candidateLength=candidates->size();
        for(std::size_t index=0;index<candidateLength;++index){
            const mml::NodeValue value=candidates->get(index);mml::MmlNode& node=*value.requireNode("attributes");
            const auto attributes=node.attributes();if(!attributes)throw Exception("TypeError",u"Cannot read properties of null (reading 'get')");
            const double level=runtime.context().toNumber(runtime.properties.to(attributes->get("scriptlevel")));
            if(level>0){
                const mml::NodeValue parentValue=node.Parent();mml::MmlNode& parent=*parentValue.requireNode("childIndex");
                // In `parent.childNodes.splice(parent.childIndex(mml), 1)`,
                // JavaScript resolves the splice receiver before evaluating
                // the childIndex argument. Keep this array handle even if the
                // virtual childIndex callback rebinds parent.childNodes.
                const mml::Children parentChildren=parent.childNodes;
                if(!parentChildren)throw Exception("TypeError",u"Cannot read properties of null (reading 'splice')");
                const auto childIndex=parent.childIndex(value);
                (void)parentChildren->splice(childIndex?static_cast<std::int64_t>(*childIndex):std::int64_t(-1),1);
                const std::array<mml::NodeValue,1> removed{value};const auto removedArray=std::make_shared<tree::ChildArray<mml::MmlNode>>();removedArray->push_back(removed[0]);
                const std::string kind=node.kind();parseOptions->removeFromList(std::u16string(kind.begin(),kind.end()),removedArray);
                if(node.isKind("mrow")){
                    if(!node.childNodes)throw Exception("TypeError",u"Cannot read properties of null (reading '0')");
                    const mml::NodeValue style=node.childNodes->get(0);const std::array<mml::NodeValue,1> styleItem{style};
                    const auto styleArray=std::make_shared<tree::ChildArray<mml::MmlNode>>();styleArray->push_back(styleItem[0]);parseOptions->removeFromList(u"mstyle",styleArray);
                    mml::MmlNode& styleNode=*style.requireNode("childNodes");if(!styleNode.childNodes)throw Exception("TypeError",u"Cannot read properties of null (reading '0')");
                    const mml::NodeValue space=styleNode.childNodes->get(0);
                    mml::MmlNode& spaceNode=*space.requireNode("childNodes");parseOptions->removeFromList(u"mspace",spaceNode.childNodes);
                }
            }else if(node.isKind("mrow")){
                const mml::NodeValue parentValue=node.Parent();mml::MmlNode& parent=*parentValue.requireNode("replaceChild");
                const mml::Children rowChildren=node.childNodes;
                if(!rowChildren)throw Exception("TypeError",u"Cannot read properties of null (reading '0')");
                const mml::NodeValue child=rowChildren->get(0);(void)parent.replaceChild(child,value);
                const auto removedArray=std::make_shared<tree::ChildArray<mml::MmlNode>>();removedArray->push_back(value);parseOptions->removeFromList(u"mrow",removedArray);
            }
        }
        return options::Undefined{};
    };
    return function;
}

Value filterFunction(ParserRuntime& runtime){
    OptionCallBinding binding;
    binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
    binding.arguments=[&runtime](Context& context,std::span<const Value> args){
        if(args.empty())throw Exception("TypeError",u"Cannot destructure property 'data' of 'undefined' as it is undefined");
        const Value data=context.get(args[0],key(u"data"));const auto* identity=std::get_if<ObjectRef>(&data.data);
        if(!identity)throw Exception("TypeError",u"filterNonscript data is not a ParseOptions object");
        const ParseOptionsRef parser=runtime.parseOptionsIdentities.resolve(*identity);
        return std::vector<std::any>{std::any(parser)};
    };
    binding.result=[](Context&,const std::any&){return Value(Undefined{});};
    const Value function=runtime.functions().bind(nonscriptFilter(runtime),std::move(binding),u"filterNonscript");
    const auto* identity=std::get_if<ObjectRef>(&function.data);
    if(!identity)throw Exception("TypeError",u"Base postprocessor has no Context identity");
    runtime.context().define(*identity,key(u"length"),Descriptor::data(Value(1),false,false,true));
    return function;
}

// The BaseTags constructor is a real Source constructor object. Its allocation
// callback retains the same AbstractTags implementation and identity bucket.
TagsClassRef makeBaseTagsClass(ParserRuntime& runtime){
    const auto* abstractConstructor=std::get_if<ObjectRef>(&runtime.abstractTagsClass.data);
    if(!abstractConstructor)throw Exception("TypeError",u"AbstractTags constructor is unavailable");
    auto info=TagsFactory::registerSubclass(runtime,u"BaseTags",
        [&runtime]()->TagsRef{return std::make_shared<BaseTags>(runtime);},
        *abstractConstructor);
    runtime.baseTagsClass=Value(info->constructor);
    return info;
}

} // namespace

ConfigurationRef BaseConfiguration::initializeModule(ParserRuntime& runtime,const Property& methods,const Property& parseMethods){
    Context& cx=runtime.context();
    const auto* module=std::get_if<ObjectRef>(&runtime.baseItemsModule.data);
    if(!module)throw Exception("TypeError",u"BaseItems module exports are not initialized");
    BaseMappings::initializeModule(runtime,methods,parseMethods);

    // BaseConfiguration.ts owns this additional map (BaseMappings does not).
    PropertyList remap;
    remap.setOwn("-",Property(property_host::debugText(u"\u2212")));
    remap.setOwn("*",Property(property_host::debugText(u"\u2217")));
    remap.setOwn("`",Property(property_host::debugText(u"\u2018")));
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("remap"),ParseMethod{Property(PropertyNull{})},Property(remap));

    const ObjectRef config=cx.object();
    // Source Configuration normalizes each handler list in a fresh ordinary
    // array, then assigns the package's own handler arrays over those fields.
    const ObjectRef handlers=cx.object();
    const std::array<std::pair<std::u16string_view,std::vector<std::u16string_view>>,4> handlerData{{
        {u"character",{u"command",u"special"}},
        {u"delimiter",{u"delimiter"}},
        {u"macro",{u"delimiter",u"macros",u"lcGreek",u"ucGreek",u"mathchar0mi",u"mathchar0mo",u"mathchar7"}},
        {u"environment",{u"environment"}}}};
    for(const auto& [name,names]:handlerData){
        std::vector<Value> values;values.reserve(names.size());
        for(const auto item:names)values.emplace_back(std::u16string(item));
        cx.define(handlers,key(name),Descriptor::data(Value(array(cx,values))));
    }

    const ObjectRef items=cx.object();
    constexpr std::array<std::u16string_view,26> itemNames{
        u"StartItem",u"StopItem",u"OpenItem",u"CloseItem",u"NullItem",u"PrimeItem",u"SubsupItem",
        u"OverItem",u"LeftItem",u"Middle",u"RightItem",u"BreakItem",u"BeginItem",u"EndItem",
        u"StyleItem",u"PositionItem",u"CellItem",u"MmlItem",u"FnItem",u"NotItem",u"NonscriptItem",
        u"DotsItem",u"ArrayItem",u"EqnArrayItem",u"EquationItem",u"MstyleItem"};
    for(const auto name:itemNames){
        const Value constructor=cx.get(*module,key(name));
        const Value prototype=cx.get(constructor,key(u"prototype"));
        const Value kind=cx.get(prototype,key(u"kind"));
        cx.set(items,key(string(runtime,kind)),constructor);
    }

    const ObjectRef tags= cx.object();
    if(std::holds_alternative<Undefined>(runtime.baseTagsClass.data))
        (void)makeBaseTagsClass(runtime);
    cx.set(tags,key(u"base"),runtime.baseTagsClass);

    const ObjectRef options=cx.object();
    cx.set(options,key(u"maxMacros"),Value(1000));
    cx.set(options,key(u"digits"),Value(u""));
    cx.set(options,key(u"numberPattern"),Value(makeRegexp(runtime,numberPattern())));
    cx.set(options,key(u"initialDigit"),Value(makeRegexp(runtime,u"[0-9.,]")));
    cx.set(options,key(u"identifierPattern"),Value(makeRegexp(runtime,u"^[a-zA-Z]+")));
    cx.set(options,key(u"initialLetter"),Value(makeRegexp(runtime,u"[a-zA-Z]")));
    // The pinned Node profile has no browser document/base element.
    cx.set(options,key(u"baseURL"),Value(u""));

    auto configBody=std::make_shared<Function>();
    configBody->sourceMetadata=FunctionSourceMetadata{u"",2};
    configBody->call=[&runtime,parseMethods](const FunctionReceiver&,FunctionArgs args)->std::any{
        const ParserConfigurationRef configuration=args.size()>0&&args[0].type()==typeid(ParserConfigurationRef)
            ?std::any_cast<ParserConfigurationRef>(args[0]):ParserConfigurationRef{};
        TeX* jax=args.size()>1&&args[1].type()==typeid(TeX*)?std::any_cast<TeX*>(args[1]):nullptr;
        if(!configuration||!jax)throw Exception("TypeError",u"Base configuration requires ParserConfiguration and TeX instances");
        Context& context=runtime.context();
        const ParserShared<ParseOptions> sharedOptions=jax->parseOptions();
        const auto* optionsPointer=std::get_if<std::shared_ptr<ParseOptions>>(&sharedOptions);
        if(!optionsPointer||!*optionsPointer)
            throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'options')");
        const Value options=(*optionsPointer)->options;
        const Value digits=context.get(options,key(u"digits"));
        if(truthy(digits))context.set(options,key(u"numberPattern"),digits);
        const Value number=context.get(options,key(u"initialDigit"));
        const Value identifier=context.get(options,key(u"initialLetter"));
        const ParseMethod digit=functions::fromProperty(property_host::get(parseMethods,"digit"));
        const ParseMethod variable=functions::fromProperty(property_host::get(parseMethods,"variable"));
        (void)TokenMapHost::create<RegExpMap>(runtime.mapHandler,Property("digit"),digit,BaseMappings::regexpContext(runtime,number));
        (void)TokenMapHost::create<RegExpMap>(runtime.mapHandler,Property("letter"),variable,BaseMappings::regexpContext(runtime,identifier));

        // The Source callback receives (ParserConfiguration, TeX). Its handler
        // collection is a native payload carrier in this host, so resolve the
        // live field from that same Source object and dispatch through its real
        // SubHandlers/SubHandler virtual methods.
        const Value handlersValue=configuration->sourceField(u"handlers");
        const auto* handlersObject=std::get_if<ObjectRef>(&handlersValue.data);
        if(!handlersObject)throw Exception("TypeError",u"Cannot read properties of null (reading 'get')");
        const auto& payload=context.payload(*handlersObject);
        const auto* handlers=std::any_cast<std::shared_ptr<SubHandlers>>(&payload);
        if(!handlers||!*handlers)throw Exception("TypeError",u"config.handlers.get is not a function");
        const auto handler=(*handlers)->get("character");
        if(!handler)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'add')");
        const ObjectRef maps=context.array(2);
        context.define(maps,key(u"0"),Descriptor::data(Value(u"letter")));
        context.define(maps,key(u"1"),Descriptor::data(Value(u"digit")));
        handler->add(context,Value(maps),functions::fromProperty(Property(PropertyNull{})),4);
        return options::Undefined{};
    };
    OptionCallBinding configBinding;
    configBinding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
    configBinding.arguments=[&runtime](Context&,std::span<const Value> args){
        if(args.size()<2)throw Exception("TypeError",u"Base configuration requires (config, jax)");
        const auto* configIdentity=std::get_if<ObjectRef>(&args[0].data);
        const auto* jaxIdentity=std::get_if<ObjectRef>(&args[1].data);
        if(!configIdentity||!jaxIdentity)throw Exception("TypeError",u"Base configuration requires ParserConfiguration and TeX instances");
        const ParserConfigurationRef configuration=runtime.parserConfigurationIdentities.resolve(*configIdentity);
        const TeXRef jax=runtime.texIdentities.resolve(*jaxIdentity);
        return std::vector<std::any>{std::any(configuration),std::any(jax.get())};
    };
    configBinding.result=[](Context&,const std::any&){return Value(Undefined{});};
    const Value configMethod=runtime.functions().bind(configBody,std::move(configBinding),u"");
    const ObjectRef fallback=cx.object();
    const FunctionRef otherBody=makeParseCallable([&runtime](const FunctionReceiver&,ParserContext& context,
            const ParseAtom& atom,ParseRest)->ParseResult{
        auto* parser=dynamic_cast<TexParser*>(&context);
        if(!parser)throw Exception("TypeError",u"Base Other requires a TexParser");
        std::u16string character;
        if(const auto* property=std::get_if<Property>(&atom))character=property_host::string(*property);
        else if(const auto* token=std::get_if<TokenRef>(&atom);token&&*token)character=property_host::string((*token)->character());
        else throw Exception("TypeError",u"Base Other requires a character");
        const auto* stack=std::get_if<std::shared_ptr<Stack>>(&parser->stack);
        if(!stack||!*stack)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'env')");
        const Property font=property_host::get((*stack)->env(),"font");
        const Property italicFont=property_host::get((*stack)->env(),"italicFont");
        Property def=property_host::truthy(font)?Property(PropertyList{{"mathvariant",font}}):Property(PropertyList{});
        Property output(property_host::debugText(character));
        const auto remap=runtime.mapHandler.getMap(Property("remap"));
        if(remap){
            const auto entry=remap->lookup(Property(property_host::debugText(character)));
            if(const auto* token=std::get_if<TokenRef>(&entry);token&&*token)output=(*token)->character();
        }
        const Property range=mml::operator_dict::getRange(character);
        const Property type=property_host::get(range,"3");
        const std::array<std::any,3> args{std::any(type),std::any(def),std::any(output)};
        const mml::NodeValue mo=parser->create(u"token",args);
        Property style;
        if(ParseUtil::isLatinOrGreekChar(runtime,character)){
            const auto* config=std::get_if<ParseOptions*>(&parser->configuration);
            if(!config||!*config)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'mathStyle')");
            const auto* function=std::get_if<ObjectRef>(&(**config).mathStyle.data);
            if(function){
                const auto owner=runtime.parseOptionsIdentities.find((**config).shared_from_this());
                if(!owner)throw Exception("TypeError",u"ParseOptions has no Context identity");
                const std::array<Value,2> styleArgs{Value(character),Value(true)};
                style=runtime.properties.from(runtime.context().call(*function,Value(*owner),styleArgs));
            }
            if(!property_host::truthy(style))style=italicFont;
        }else style=Property("");
        const Property explicitVariant=property_host::get(range,"4");
        Property chosen=explicitVariant;
        if(!property_host::truthy(chosen)){
            const Property normal=property_host::get(property_host::get(TexConstant,"Variant"),"NORMAL");
            chosen=(property_host::truthy(font)&&style==normal)?Property(""):style;
        }
        if(property_host::truthy(chosen))NodeUtil::instance().setAttribute(mo,"mathvariant",chosen);
        if(type==Property("mo")){
            NodeUtil::instance().setProperty(mo,"fixStretchy",Property(true));
            const auto* config=std::get_if<ParseOptions*>(&parser->configuration);
            if(!config||!*config)throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'addNode')");
            (**config).addNode(Property("fixStretchy"),mo);
        }
        parser->Push(mo);return Property(true);
    });
    const Value other=BaseMethods::bind(runtime,otherBody,u"Other");
    cx.set(fallback,key(u"character"),other);
    const FunctionRef undefinedControl=makeParseCallable([](const FunctionReceiver&,ParserContext&,
            const ParseAtom& atom,ParseRest)->ParseResult{
        std::u16string name;
        if(const auto* value=std::get_if<Property>(&atom))name=property_host::string(*value);
        else throw Exception("TypeError",u"Undefined-control-sequence name is not a string");
        const std::array<Property,1> args{Property(property_host::debugText(u"\\"+name))};
        throw TexError(u"UndefinedControlSequence",u"Undefined control sequence %1",args);
    });
    const FunctionRef undefinedEnvironment=makeParseCallable([](const FunctionReceiver&,ParserContext&,
            const ParseAtom& atom,ParseRest)->ParseResult{
        std::u16string name;
        if(const auto* value=std::get_if<Property>(&atom))name=property_host::string(*value);
        else throw Exception("TypeError",u"Undefined-environment name is not a string");
        const std::array<Property,1> args{Property(property_host::debugText(name))};
        throw TexError(u"UnknownEnv",u"Unknown environment '%1'",args);
    });
    cx.set(fallback,key(u"macro"),BaseMethods::bind(runtime,undefinedControl,u"csUndefined"));
    cx.set(fallback,key(u"environment"),BaseMethods::bind(runtime,undefinedEnvironment,u"envUndefined"));
    cx.define(config,key(u"name"),Descriptor::data(Value(u"base")));
    cx.define(config,key(u"config"),Descriptor::data(Value(configMethod)));
    cx.define(config,key(u"handler"),Descriptor::data(Value(handlers)));
    cx.define(config,key(u"items"),Descriptor::data(Value(items)));
    cx.define(config,key(u"tags"),Descriptor::data(Value(tags)));
    cx.define(config,key(u"options"),Descriptor::data(Value(options)));
    cx.define(config,key(u"fallback"),Descriptor::data(Value(fallback)));
    const ObjectRef postprocessors=cx.array(1);const ObjectRef postprocessor=cx.array(2);
    cx.set(postprocessor,key(u"0"),filterFunction(runtime));cx.set(postprocessor,key(u"1"),Value(-4));
    cx.set(postprocessors,key(u"0"),Value(postprocessor));
    cx.define(config,key(u"postprocessors"),Descriptor::data(Value(postprocessors)));
    const auto result=Configuration::create(runtime,u"base",Value(config));
    return result;
}

ConfigurationRef BasePackage::initializeModule(ParserRuntime& runtime){
    const Property baseMethods=BaseMethods::initializeModule(runtime);
    const Property parseMethods=ParseMethods::initializeModule(runtime);
    return BaseConfiguration::initializeModule(runtime,baseMethods,parseMethods);
}

} // namespace folia::math::tex
