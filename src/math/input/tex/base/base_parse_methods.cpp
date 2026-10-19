/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 input/tex/ParseMethods.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.input.tex.base_parse_methods;

import folia.math.input.tex.parser;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.node_util;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.token_map;
import folia.math.input.tex.stack_item_factory;
import folia.math.util.argument_expansion;
import folia.math.util.property_host;
import folia.math.util.regexp_binding;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
using Body=ParseBody;

TexParser& parserFrom(ParserContext& context){
    if(auto* parser=dynamic_cast<TexParser*>(&context))return *parser;
    property_host::typeError("ParseMethod requires a TexParser");
}
ParseOptions& configuration(TexParser& parser){
    if(auto* value=std::get_if<ParseOptions*>(&parser.configuration);value&&*value)return **value;
    property_host::typeError("Cannot read properties of null or undefined (reading 'options')");
}
Stack& stack(TexParser& parser){
    if(auto* value=std::get_if<std::shared_ptr<Stack>>(&parser.stack);value&&*value)return **value;
    property_host::typeError("Cannot read properties of null or undefined (reading 'env')");
}
StackItemFactory& itemFactory(TexParser& parser){
    const auto factory=parser.itemFactory();
    if(auto* value=std::get_if<std::shared_ptr<StackItemFactory>>(&factory);value&&*value)return **value;
    property_host::typeError("Cannot read properties of null or undefined (reading 'create')");
}
Property restAt(ParseRest rest,std::size_t i){
    if(i>=rest.size())return Missing{};
    if(const auto* value=std::any_cast<Property>(&rest[i]))return *value;
    if(const auto* value=std::any_cast<FunctionRef>(&rest[i]))return Property(*value);
    if(const auto* value=std::any_cast<std::u16string>(&rest[i]))return property_host::debugText(*value);
    if(const auto* value=std::any_cast<double>(&rest[i]))return *value;
    if(const auto* value=std::any_cast<bool>(&rest[i]))return *value;
    return Missing{};
}
std::u16string atomString(const ParseAtom& atom){
    if(const auto* value=std::get_if<Property>(&atom))return property_host::string(*value);
    if(const auto* value=std::get_if<TokenRef>(&atom);value&&*value)return property_host::string((*value)->character());
    property_host::typeError("ParseMethod token is not a string");
}
std::optional<TokenRef> tokenRef(const ParseAtom& atom){
    if(const auto* token=std::get_if<TokenRef>(&atom))return *token;
    return std::nullopt;
}
mml::NodeValue createToken(TexParser& parser,std::u16string_view kind,Property def,Property text){
    const std::array<std::any,3> args{std::any(Property(property_host::debugText(kind))),std::any(std::move(def)),std::any(std::move(text))};
    return parser.create(u"token",args);
}
ParseResult push(TexParser& parser,mml::NodeValue node){parser.Push(std::move(node));return Property(true);}
Property variant(std::string_view name){return property_host::get(property_host::get(TexConstant,"Variant"),name);}
Property mathStyle(TexParser& parser,std::u16string_view character,bool upper=false){
    ParseOptions& config=configuration(parser);
    const Value callback=config.mathStyle;
    const auto* function=std::get_if<ObjectRef>(&callback.data);
    if(!function)return Missing{};
    ParserRuntime& runtime=configuration(parser).runtime();
    const auto owner=runtime.parseOptionsIdentities.find(config.shared_from_this());
    if(!owner)return Missing{};
    std::vector<Value> args{Value(std::u16string(character))};
    if(upper)args.emplace_back(true);
    return runtime.properties.from(runtime.context().call(*function,Value(*owner),args));
}
FunctionRef make(Body body){return makeParseCallable(std::move(body));}
}

Property ParseMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.parseMethods.missing())return runtime.parseMethods;
    PropertyList result;
    auto add=[&](std::u16string name,Body body){
        const FunctionRef callback=make(std::move(body));
        result.setOwn(property_host::debugText(name),Property(callback));
        (void)BaseMethods::bind(runtime,callback,name);
    };
    add(u"variable",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& parser=parserFrom(context);std::u16string text=atomString(atom);
        Property def=ParseUtil::getFontDef(parser);
        const auto env=property_host::get(stack(parser).env(),"multiLetterIdentifiers");
        const auto font=property_host::get(stack(parser).env(),"font");
        if(property_host::truthy(env)&&font!=Property("")){
            const Value pattern=configuration(parser).runtime().properties.to(env);
            const auto match=configuration(parser).runtime().regexp().exec(pattern,parser.string().substr(static_cast<std::size_t>(std::max(0.0,parser.i-1))));
            if(match&&match->at(0)&&!match->at(0)->empty()){
                text=*match->at(0);parser.i+=static_cast<double>(text.size())-1;
                if(property_host::get(def,"mathvariant")==variant("NORMAL")&&
                   property_host::truthy(property_host::get(stack(parser).env(),"noAutoOP"))&&text.size()>1)
                    property_host::set(def,"autoOP",Property(false));
            }
        }
        if(!property_host::truthy(property_host::get(def,"mathvariant"))&&ParseUtil::isLatinOrGreekChar(configuration(parser).runtime(),text)){
            const Property style=mathStyle(parser,text);
            if(property_host::truthy(style))property_host::set(def,"mathvariant",style);
        }
        return push(parser,createToken(parser,u"mi",def,Property(property_host::debugText(text))));
    });
    add(u"digit",[](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        auto& parser=parserFrom(context);const auto rest=parser.string().substr(static_cast<std::size_t>(std::max(0.0,parser.i-1)));
        auto& runtime=configuration(parser).runtime();
        const Value pattern=runtime.context().get(configuration(parser).options,Key(u"numberPattern"));
        const auto match=runtime.regexp().exec(pattern,rest);
        if(!match||!match->at(0))return Property(false);
        const Property def=ParseUtil::getFontDef(parser);std::u16string value=*match->at(0);
        value.erase(std::remove_if(value.begin(),value.end(),[](char16_t c){return c==u'{'||c==u'}';}),value.end());
        const auto node=createToken(parser,u"mn",def,Property(property_host::debugText(value)));
        parser.i+=static_cast<double>(match->at(0)->size())-1;return push(parser,node);
    });
    add(u"controlSequence",[](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        auto& parser=parserFrom(context);const auto name=parser.GetCS();
        (void)parser.parse(Property("macro"),ParseInput{std::ref(static_cast<ParserContext&>(parser)),Property(property_host::debugText(name))});
        return Property(true);
    });
    auto greek=[&](std::u16string name,bool upper){
        add(std::move(name),[upper](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
            auto& parser=parserFrom(context);const auto token=tokenRef(atom);const std::u16string character=atomString(atom);
            Property def;
            if(upper){
                const Property font=property_host::get(stack(parser).env(),"font");
                const Property style=mathStyle(parser,character,true);
                const Property selected=property_host::truthy(font)?font:property_host::truthy(style)?style:variant("NORMAL");
                def=Property(PropertyList{{"mathvariant",selected}});
            }else{
                const Property style=mathStyle(parser,character);
                const Property selected=property_host::truthy(style)?style:variant("ITALIC");
                def=Property(PropertyList{{"mathvariant",selected}});
            }
            (void)token;return push(parser,createToken(parser,u"mi",def,Property(property_host::debugText(character))));
        });
    };
    greek(u"lcGreek",false);greek(u"ucGreek",true);
    for(const auto name:{u"mathchar0mi",u"mathchar0mo",u"mathchar7",u"delimiter"}){
        add(std::u16string(name),[name=std::u16string(name)](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
            auto& parser=parserFrom(context);const auto token=tokenRef(atom);
            if(!token)property_host::typeError("Character map did not supply a Token");
            Property def=(*token)->attributes();
            if(name==u"mathchar0mi"&&!property_host::truthy(def))def=Property(PropertyList{{"mathvariant",variant("ITALIC")}});
            else if(name==u"mathchar7"){
                if(!property_host::truthy(def))def=Property(PropertyList{{"mathvariant",variant("NORMAL")}});
                const Property font=property_host::get(stack(parser).env(),"font");if(property_host::truthy(font))property_host::set(def,"mathvariant",font);
            }else if(name==u"mathchar0mo"){
                if(!property_host::truthy(def))def=Property(PropertyList{});property_host::set(def,"stretchy",Property(false));
            }else if(name==u"delimiter"){
                PropertyList defaults{{"fence",false},{"stretchy",false}};Property merged(defaults);
                if(property_host::truthy(def))for(const auto& key:property_host::ownKeys(def))property_host::set(merged,key,property_host::get(def,key));
                def=std::move(merged);
            }
            const std::u16string character=property_host::string((*token)->character());
            const std::u16string kind=(name==u"mathchar0mo"||name==u"delimiter")?u"mo":u"mi";
            const auto node=createToken(parser,kind,def,(*token)->character());
            if(name==u"mathchar0mo"){
                NodeUtil::instance().setProperty(node,"fixStretchy",Property(true));configuration(parser).addNode(Property("fixStretchy"),node);
            }else if(name==u"delimiter"&&character==u"|")NodeUtil::instance().setProperty(node,"keep-attrs",Property("stretchy"));
            return push(parser,node);
        });
    }
    add(u"environment",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& parser=parserFrom(context);const Property env=Property(property_host::debugText(atomString(atom)));
        BaseItem* raw=itemFactory(parser).create("begin",{});
        const StackItemRef item=itemFactory(parser).reference(raw);
        if(auto* begin=dynamic_cast<BaseItem*>(item.get()))begin->setProperty("name",StackProperty(env));
        else property_host::typeError("Environment begin item is not a BaseItem");
        if(rest.size()<2)property_host::typeError("Cannot read properties of undefined (reading 'slice')");
        const Property callable=restAt(rest,0);
        const ParseMethod method=functions::fromProperty(callable);
        const Property supplied=restAt(rest,1);
        const auto list=PropertyArray::array(supplied);
        if(!list)property_host::typeError("args.slice is not a function");
        const Property trailing((*list).slice(1));
        const std::vector<Property> expanded=arguments::expand(trailing);
        std::vector<std::any> callRest;callRest.reserve(expanded.size());
        for(const auto& value:expanded)callRest.emplace_back(value);
        const ParseResult parsed=invokeParseMethod(method,FunctionReceiver{Missing{}},
            static_cast<ParserContext&>(parser),ParseAtom(item),callRest);
        if(const auto* entry=std::get_if<StackItemRef>(&parsed))parser.Push(*entry);
        else if(const auto* value=std::get_if<Property>(&parsed))parser.Push(*value);
        else property_host::typeError("Environment parser did not return a stack item");
        return Property(Missing{});
    });
    runtime.parseMethods=Property(result);
    return runtime.parseMethods;
}
}
