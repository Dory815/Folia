// MathJax 4.1.3 input/tex/textmacros/TextParser.ts semantic methods.
module folia.math.input.tex.parser;
import :runtime;
import :parse_options;
import :tex_parser;
import :base_items;
import :text_parser;
import :parse_util;
import folia.math.input.tex.node_util;
import folia.math.util.property_host;
import folia.math.util.options;
import folia.math.input.tex.tex_parser_context_fields;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
Value argument(std::span<const Value> args,std::size_t index){return index<args.size()?args[index]:Value(Undefined{});}
ParseOptions& parserConfiguration(const TexParser& parser){
    if(const auto* owner=std::get_if<ParseOptions*>(&parser.configuration);owner&&*owner)return **owner;
    throw Exception("TypeError",u"Cannot read properties of undefined (reading 'configuration')");
}
Stack& parserStack(TexParser& parser){
    if(const auto* owner=std::get_if<std::shared_ptr<Stack>>(&parser.stack);owner&&*owner)return **owner;
    throw Exception("TypeError",u"Cannot read properties of undefined (reading 'env')");
}
std::u16string propertyText(const Property& value){return property_host::string(value);}
std::shared_ptr<Function> nativeBody(std::function<std::any(const NativeObjectRef&,FunctionArgs)> invoke){
    auto body=std::make_shared<Function>();
    body->call=[invoke=std::move(invoke)](const FunctionReceiver& receiver,FunctionArgs args)->std::any{
        const auto* owner=std::get_if<NativeObjectRef>(&receiver);
        if(!owner||!*owner)throw Exception("TypeError",u"TextParser method receiver is incompatible");
        return invoke(*owner,args);
    };
    return body;
}
}

std::shared_ptr<TextParser> TextParser::make(ParserRuntime& runtime,std::u16string source,
        Property env,ParseOptions& config,Property level){
    if(std::holds_alternative<Undefined>(runtime.textParserClass.data))
        throw Exception("Unimplemented",u"TextParser constructor is not registered for this loaded module");
    auto parser=std::make_shared<TextParser>();
    const ObjectRef constructor=std::get<ObjectRef>(runtime.textParserClass.data);
    const ObjectRef identity=runtime.parserIdentities.instance(std::static_pointer_cast<TexParser>(parser),constructor,parser);
    parser->bindIdentity(identity);
    // Source's super constructor runs Parse before the derived constructor
    // assigns level; initializeTextParser makes that virtual call in place.
    parser->initializeTexParser(std::move(source),std::move(env),config);
    parser->level_=std::move(level);
    TexParserContextFields fields(runtime.context(),identity,std::static_pointer_cast<void>(parser),runtime.executionLifetime());
    fields.assign(u"level",runtime.properties.to(parser->level_),[parser,&runtime]{return runtime.properties.to(parser->level_);},
        [parser,&runtime](Value value){parser->level_=runtime.properties.from(std::move(value));});
    return parser;
}

Value TextParser::texParserValue()const{
    const auto& config=parserConfiguration(*this);
    ParserRuntime& runtime=config.runtime();
    const auto package=config.packageData;
    if(!std::holds_alternative<std::shared_ptr<SourceStringMap<Value>>>(package)||
       !std::get<std::shared_ptr<SourceStringMap<Value>>>(package))
        throw Exception("TypeError",u"Cannot read properties of undefined (reading 'get')");
    const auto record=std::get<std::shared_ptr<SourceStringMap<Value>>>(package)->get(u"textmacros");
    if(!record||std::holds_alternative<Undefined>(record->data))
        throw Exception("TypeError",u"Cannot read properties of undefined (reading 'texParser')");
    if(std::holds_alternative<Null>(record->data))
        throw Exception("TypeError",u"Cannot read properties of null (reading 'texParser')");
    return runtime.context().get(*record,key(u"texParser"));
}

TexParser& TextParser::texParser()const{
    const Value parser=texParserValue();
    const auto* identity=std::get_if<ObjectRef>(&parser.data);
    if(!identity){
        if(std::holds_alternative<Null>(parser.data))throw Exception("TypeError",u"Cannot read properties of null (reading 'configuration')");
        throw Exception("TypeError",u"Cannot read properties of undefined (reading 'configuration')");
    }
    return *parserConfiguration(*this).runtime().parserIdentities.resolve(*identity);
}

ParserShared<Tags> TextParser::tags()const{
    const Value parser=texParserValue();
    if(std::holds_alternative<Null>(parser.data))throw Exception("TypeError",u"Cannot read properties of null (reading 'tags')");
    if(std::holds_alternative<Undefined>(parser.data))throw Exception("TypeError",u"Cannot read properties of undefined (reading 'tags')");
    const auto* identity=std::get_if<ObjectRef>(&parser.data);
    if(!identity)return Missing{};
    return parserConfiguration(*this).runtime().parserIdentities.resolve(*identity)->tags();
}

mml::NodeValue TextParser::mml(){
    copyLists();
    parserConfiguration(*this).popParser();
    if(!level_.missing()&&!std::holds_alternative<PropertyNull>(level_.value)){
        const std::array<std::any,3> args{std::any(Property("mstyle")),std::any(nodes_),
            std::any(Property(PropertyList{{"displaystyle",false},{"scriptlevel",level_}}))};
        return create(u"node",args);
    }
    if(nodes_->size()==1)return nodes_->get(0);
    const std::array<std::any,2> args{std::any(Property("mrow")),std::any(nodes_)};
    return create(u"node",args);
}

void TextParser::copyLists(){
    TexParser& original=texParser();
    ParseOptions& target=parserConfiguration(original);
    ParseOptions& source=parserConfiguration(*this);
    Context& cx=source.runtime().context();
    std::vector<std::pair<std::u16string,Value>> entries;
    for(const auto& name:cx.enumerableStringKeys(source.nodeLists))
        entries.emplace_back(name,cx.get(source.nodeLists,key(name)));
    for(const auto& [name,list]:entries){
        cx.forEachIterable(list,[&](Value value){
            mml::NodeValue node;
            if(std::holds_alternative<Undefined>(value.data))node=Missing{};
            else if(std::holds_alternative<Null>(value.data))node=PropertyNull{};
            else if(const auto* object=std::get_if<ObjectRef>(&value.data))
                node=mml::NodeValue(&source.runtime().resolveMmlNode(*object));
            else throw Exception("TypeError",u"TextParser node list value is not an MML node");
            target.addNode(Property(property_host::debugText(name)),std::move(node));
        });
    }
    source.nodeLists=cx.object();
}

void TextParser::Parse(){
    ParserRuntime& runtime=parserConfiguration(*this).runtime();
    Context& cx=runtime.context();
    const ObjectRef identity=instanceIdentity();
    const auto owner=std::dynamic_pointer_cast<TextParser>(runtime.parserIdentities.resolve(identity));
    if(!owner)throw Exception("TypeError",u"TextParser identity resolved to a different parser owner");
    TexParserContextFields fields(cx,identity,std::static_pointer_cast<void>(owner),runtime.executionLifetime());
    text_.clear();
    fields.assign(u"text",Value(text_),[this]{return Value(text_);},[this](Value value){
        const auto* text=std::get_if<std::u16string>(&value.data);
        if(!text)throw Exception("TypeError",u"TextParser.text must be a string");
        text_=*text;
    });
    nodes_=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    const ObjectRef nodesIdentity=bindNodeArray(runtime,nodes_);
    fields.assign(u"nodes",Value(nodesIdentity),[this,&runtime](){
        if(!nodes_)return Value(Undefined{});
        const auto identity=bindNodeArray(runtime,nodes_);return Value(identity);
    },[this,&runtime](Value value){
        if(std::holds_alternative<Undefined>(value.data)||std::holds_alternative<Null>(value.data))
            throw Exception("Unimplemented",u"TextParser.nodes cannot represent a nullish replacement in its typed ChildArray owner");
        const auto* object=std::get_if<ObjectRef>(&value.data);
        if(!object)throw Exception("TypeError",u"TextParser.nodes must be an Array");
        nodes_=runtime.nodeArrayIdentities.resolve(*object);
    });
    const ObjectRef envStackIdentity=resetEnvStack(cx,runtime.properties,
        std::static_pointer_cast<void>(owner),runtime.executionLifetime());
    fields.assign(u"envStack",Value(envStackIdentity),[this]{return envStackContextValue();},
        [this](Value value){replaceEnvStack(std::move(value));});
    TexParser::Parse();
}

void TextParser::saveText(){
    if(text_.empty())return;
    const Property environment=parserStack(*this).env();
    const Property mathvariant=property_host::get(environment,"mathvariant");
    Property definition=PropertyList{};
    if(property_host::truthy(mathvariant))definition=Property(PropertyList{{"mathvariant",mathvariant}});
    const mml::NodeValue saved=ParseUtil::internalText(*this,text_,std::move(definition));
    text_.clear();
    Push(StackEntry(saved));
}

void TextParser::Push(StackEntry entry){
    if(!text_.empty())saveText();
    if(const auto* item=std::get_if<StackItemRef>(&entry);item&&*item){
        if(dynamic_cast<StopItem*>(item->get())){TexParser::Push(std::move(entry));return;}
        if(dynamic_cast<StyleItem*>(item->get())){
            const Property receiver=parserStack(*this).env();
            const Property color=property_host::get(parserStack(*this).env(),"color");
            property_host::set(receiver,"mathcolor",color);
            return;
        }
        return;
    }
    if(const auto* value=std::get_if<mml::NodeValue>(&entry);value&&value->isNode()){
        auto* node=dynamic_cast<mml::AbstractMmlNode*>(value->nodeOrNull());
        if(node){addAttributes(*value);nodes_->push_back(*value);}
    }
}

void TextParser::PushMath(mml::NodeValue node){
    const Property environment=parserStack(*this).env();
    for(const std::string_view name:{"mathsize","mathcolor"}){
        const Property value=property_host::get(environment,name);
        if(property_host::truthy(value)&&!node.requireNode("attributes")->attributes()->hasExplicit(name)){
            if(!node.requireNode("isToken")->isToken()&&!NodeUtil::instance().isType(node,"mstyle")){
                const auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(node);
                const std::array<std::any,2> args{std::any(Property("mstyle")),std::any(children)};
                node=create(u"node",args);
            }
        NodeUtil::instance().setAttribute(node,name,property_host::get(environment,name));
        }
    }
    if(NodeUtil::instance().isInferred(node)){
        const mml::Children children=NodeUtil::instance().getChildren(node);
        const std::array<std::any,2> args{std::any(Property("mrow")),std::any(children)};
        node=create(u"node",args);
    }
    if(!NodeUtil::instance().isType(node,"TeXAtom")){
        const auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(node);
        const std::array<std::any,2> args{std::any(Property("TeXAtom")),std::any(children)};
        node=create(u"node",args);
    }
    nodes_->push_back(node);
}

void TextParser::addAttributes(mml::NodeValue node){
    const Property environment=parserStack(*this).env();
    if(!node.requireNode("isToken")->isToken())return;
    for(const std::string_view name:{"mathsize","mathcolor","mathvariant"}){
        const Property value=property_host::get(environment,name);
        if(property_host::truthy(value)&&!node.requireNode("attributes")->attributes()->hasExplicit(name))
            NodeUtil::instance().setAttribute(node,name,property_host::get(environment,name));
    }
}

mml::NodeValue TextParser::ParseTextArg(std::u16string name,Property env){
    const Property argument=GetArgument(std::move(name));
    PropertyList merged=property_host::objectAssign(parserStack(*this).env());
    const PropertyList supplied=property_host::objectAssign(env);
    for(const auto& key:supplied.keys())merged.set(key,supplied.getOwn(key));
    auto parser=make(parserConfiguration(*this).runtime(),propertyText(argument),Property(std::move(merged)),parserConfiguration(*this));
    return parser->mml();
}

mml::NodeValue TextParser::ParseArg(std::u16string name){
    const Property argument=GetArgument(std::move(name));
    auto parser=make(parserConfiguration(*this).runtime(),propertyText(argument),parserStack(*this).env(),parserConfiguration(*this));
    return parser->mml();
}

[[noreturn]] void TextParser::Error(std::u16string id,std::u16string message,std::span<const Property> args){
    throw TexError(std::move(id),message,args);
}

Value TextParser::internalMathFunction(ParserRuntime& runtime){
    auto body=std::make_shared<Function>();
    body->call=[&runtime](const FunctionReceiver&,FunctionArgs args)->std::any{
        if(args.size()<2)throw Exception("TypeError",u"internalMath requires parser and text arguments");
        TexParserRef parser;
        if(const auto* native=std::any_cast<TexParserRef>(&args[0]))parser=*native;
        else if(const auto* value=std::any_cast<Value>(&args[0])){
            const auto* identity=std::get_if<ObjectRef>(&value->data);
            if(!identity)throw Exception("TypeError",u"internalMath parser is not a TexParser");
            parser=runtime.parserIdentities.resolve(*identity);
        }else throw Exception("TypeError",u"internalMath parser is not a TexParser");
        if(!parser)throw Exception("TypeError",u"internalMath parser is null");
        std::u16string source;
        if(const auto* value=std::any_cast<std::u16string>(&args[1]))source=*value;
        else if(const auto* value=std::any_cast<Value>(&args[1])){
            if(const auto* text=std::get_if<std::u16string>(&value->data))source=*text;
            else throw Exception("TypeError",u"internalMath text must be a string");
        }else throw Exception("TypeError",u"internalMath text must be a string");
        const auto propertyArg=[&](std::size_t index){
            if(index>=args.size())return Property(Missing{});
            if(const auto* value=std::any_cast<Property>(&args[index]))return *value;
            if(const auto* value=std::any_cast<Value>(&args[index]))return runtime.properties.from(*value);
            return Property(Missing{});
        };
        const Property level=propertyArg(2),font=propertyArg(3);
        ParseOptions& main=parserConfiguration(*parser);
        const auto packageData=main.packageData;
        if(!std::holds_alternative<std::shared_ptr<SourceStringMap<Value>>>(packageData)||
           !std::get<std::shared_ptr<SourceStringMap<Value>>>(packageData))
            throw Exception("TypeError",u"Cannot read properties of undefined (reading 'get')");
        const auto record=std::get<std::shared_ptr<SourceStringMap<Value>>>(packageData)->get(u"textmacros");
        if(!record||std::holds_alternative<Undefined>(record->data)||std::holds_alternative<Null>(record->data))
            throw Exception("TypeError",u"Cannot read properties of undefined (reading 'texParser')");
        Context& context=runtime.context();
        if(dynamic_cast<TextParser*>(parser.get())==nullptr)
            context.set(*record,key(u"texParser"),Value(parser->instanceIdentity()));
        const Value textOptions=context.get(*record,key(u"parseOptions"));
        const auto* textOptionsObject=std::get_if<ObjectRef>(&textOptions.data);
        if(!textOptionsObject)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'clear')");
        const ParseOptionsRef inner=runtime.parseOptionsIdentities.resolve(*textOptionsObject);
        inner->clear();
        Property environment=PropertyList{};
        if(property_host::truthy(font))environment=Property(PropertyList{{"mathvariant",font}});
        auto textParser=TextParser::make(runtime,std::move(source),std::move(environment),*inner,level);
        const mml::NodeValue result=textParser->mml();
        auto nodes=std::make_shared<tree::ChildArray<mml::MmlNode>>();
        nodes->push_back(result);
        return std::any(std::move(nodes));
    };
    OptionCallBinding binding;
    binding.receiver=[](Context&,Value receiver)->FunctionReceiver{return FunctionReceiver{std::move(receiver)};};
    binding.arguments=[&runtime](Context&,std::span<const Value> values){
        std::vector<std::any> result;result.reserve(4);
        if(!values.empty()){
            const auto* identity=std::get_if<ObjectRef>(&values[0].data);
            if(!identity)throw Exception("TypeError",u"internalMath parser is not a TexParser");
            result.emplace_back(runtime.parserIdentities.resolve(*identity));
        }else result.emplace_back(TexParserRef{});
        if(values.size()>1){const auto* value=std::get_if<std::u16string>(&values[1].data);if(!value)throw Exception("TypeError",u"internalMath text must be a string");result.emplace_back(*value);}
        else result.emplace_back(std::u16string{});
        result.emplace_back(values.size()>2?runtime.properties.from(values[2]):Property(Missing{}));
        result.emplace_back(values.size()>3?runtime.properties.from(values[3]):Property(Missing{}));
        return result;
    };
    binding.result=[&runtime](Context&,const std::any& result){
        const auto* children=std::any_cast<mml::Children>(&result);
        if(!children||!*children)throw Exception("TypeError",u"internalMath did not return a node array");
        return Value(bindNodeArray(runtime,*children));
    };
    Value result=runtime.functions().bind(body,std::move(binding),u"internalMath");
    if(const auto* function=std::get_if<ObjectRef>(&result.data))
        runtime.context().define(*function,key(u"length"),Descriptor::data(Value(4),false,false,true));
    return result;
}

void TextParser::initializeModule(ParserRuntime& runtime){
    Context& cx=runtime.context();
    const ObjectRef constructor=cx.nativeConstructor(u"TextParser",[&runtime](Context&,std::span<const Value> args)->ObjectRef{
        const Value source=argument(args,0),env=argument(args,1),configurationValue=argument(args,2),level=argument(args,3);
        const auto* sourceText=std::get_if<std::u16string>(&source.data);
        const auto* envObject=std::get_if<ObjectRef>(&env.data);
        const auto* configObject=std::get_if<ObjectRef>(&configurationValue.data);
        if(!sourceText)throw Exception("TypeError",u"TextParser text must be a string");
        if(!configObject)throw Exception("TypeError",u"TextParser requires its ParseOptions identity");
        if(!envObject&& !std::holds_alternative<Undefined>(env.data) && !std::holds_alternative<Null>(env.data))
            throw Exception("TypeError",u"TextParser environment must be an object");
        const Property nativeEnv=runtime.properties.from(env);
        const Property nativeLevel=runtime.properties.from(level);
        const auto parser=TextParser::make(runtime,*sourceText,nativeEnv,*runtime.parseOptionsIdentities.resolve(*configObject),nativeLevel);
        return parser->instanceIdentity();
    });
    cx.define(constructor,key(u"length"),Descriptor::data(Value(4),false,false,true));
    runtime.textParserClass=Value(constructor);
    const auto baseConstructor=std::get<ObjectRef>(runtime.texParserClass.data);
    cx.setPrototype(constructor,baseConstructor);
    const auto prototype=std::get<ObjectRef>(cx.get(Value(constructor),key(u"prototype")).data);
    const auto basePrototype=std::get<ObjectRef>(cx.get(Value(baseConstructor),key(u"prototype")).data);
    cx.setPrototype(prototype,basePrototype);
    cx.define(prototype,key(u"constructor"),Descriptor::data(Value(constructor),true,true,true));

    const auto receiver=[](const FunctionReceiver& value)->std::shared_ptr<TextParser>{
        const auto* object=std::get_if<NativeObjectRef>(&value);
        auto parser=object?std::dynamic_pointer_cast<TexParser>(*object):TexParserRef{};
        auto result=parser?std::dynamic_pointer_cast<TextParser>(parser):std::shared_ptr<TextParser>{};
        if(!result)throw Exception("TypeError",u"TextParser method receiver is incompatible");
        return result;
    };
    const auto resultValue=[&runtime](const std::any& result)->Value{
        if(std::any_cast<std::monostate>(&result))return Value(Undefined{});
        if(const auto* value=std::any_cast<bool>(&result))return Value(*value);
        if(const auto* value=std::any_cast<double>(&result))return Value(*value);
        if(const auto* value=std::any_cast<std::u16string>(&result))return Value(*value);
        if(const auto* value=std::any_cast<Value>(&result))return *value;
        if(const auto* value=std::any_cast<Property>(&result)){
            if(value->missing())return Value(Undefined{});
            if(std::holds_alternative<PropertyNull>(value->value))return Value(Null{});
            if(const auto* item=std::get_if<bool>(&value->value))return Value(*item);
            if(const auto* item=std::get_if<double>(&value->value))return Value(*item);
            if(const auto* item=std::get_if<std::string>(&value->value))return property_host::text(*item);
            throw Exception("Unimplemented",u"TextParser Property result requires a Context identity adapter");
        }
        if(const auto* value=std::any_cast<ParserShared<Tags>>(&result)){
            if(std::holds_alternative<Missing>(*value))return Value(Undefined{});
            if(std::holds_alternative<PropertyNull>(*value))return Value(Null{});
            const auto& tags=std::get<std::shared_ptr<Tags>>(*value);
            const auto identity=runtime.tagIdentities.find(tags);
            if(!identity)throw Exception("Unimplemented",u"TextParser tags has no canonical Context identity");
            return Value(*identity);
        }
        if(std::any_cast<mml::NodeValue>(&result))
            throw Exception("Unimplemented",u"TextParser MML result requires a canonical MML node identity binding");
        throw Exception("Unimplemented",u"TextParser result has no canonical Context conversion");
    };
    const auto addMethod=[&](std::u16string_view name,double length,
            std::function<std::any(TextParser&,FunctionArgs)> invoke){
        auto body=std::make_shared<Function>();
        body->call=[invoke,receiver](const FunctionReceiver& target,FunctionArgs args)->std::any{return invoke(*receiver(target),args);};
        OptionCallBinding binding;
        binding.receiver=[&runtime](Context&,Value target)->FunctionReceiver{
            const auto* object=std::get_if<ObjectRef>(&target.data);
            if(!object)throw Exception("TypeError",u"TextParser method receiver is not an object");
            return std::static_pointer_cast<NativeObject>(runtime.parserIdentities.resolve(*object));
        };
        binding.arguments=[](Context&,std::span<const Value> args){std::vector<std::any> result;result.reserve(args.size());for(const auto& arg:args)result.emplace_back(arg);return result;};
        binding.result=[resultValue](Context&,const std::any& result){return resultValue(result);};
        const Value bound=runtime.functions().bind(body,std::move(binding),u"");
        const auto* function=std::get_if<ObjectRef>(&bound.data);
        if(!function)throw Exception("TypeError",u"TextParser method binding is not callable");
        cx.define(*function,key(u"name"),Descriptor::data(Value(u""),false,false,true));
        cx.define(*function,key(u"length"),Descriptor::data(Value(length),false,false,true));
        cx.define(prototype,key(name),Descriptor::data(bound,true,true,true));
    };
    const auto addGetter=[&](std::u16string_view name,std::function<std::any(TextParser&)> invoke){
        auto body=std::make_shared<Function>();
        body->call=[invoke,receiver](const FunctionReceiver& target,FunctionArgs)->std::any{return invoke(*receiver(target));};
        OptionCallBinding binding;
        binding.receiver=[&runtime](Context&,Value target)->FunctionReceiver{
            const auto* object=std::get_if<ObjectRef>(&target.data);
            if(!object)throw Exception("TypeError",u"TextParser getter receiver is not an object");
            return std::static_pointer_cast<NativeObject>(runtime.parserIdentities.resolve(*object));
        };
        binding.arguments=[](Context&,std::span<const Value>){return std::vector<std::any>{};};
        binding.result=[resultValue](Context&,const std::any& result){return resultValue(result);};
        const Value bound=runtime.functions().bind(body,std::move(binding),u"get");
        const auto* function=std::get_if<ObjectRef>(&bound.data);
        if(!function)throw Exception("TypeError",u"TextParser getter binding is not callable");
        cx.define(*function,key(u"name"),Descriptor::data(Value(u"get"),false,false,true));
        cx.define(*function,key(u"length"),Descriptor::data(Value(0),false,false,true));
        cx.define(prototype,key(name),Descriptor::access(*function,std::nullopt,false,true));
    };
    addGetter(u"texParser",[](TextParser& parser)->std::any{return std::any(parser.texParserValue());});
    addGetter(u"tags",[](TextParser& parser)->std::any{return std::any(parser.tags());});
    addMethod(u"mml",0,[](TextParser& parser,FunctionArgs)->std::any{return std::any(parser.mml());});
    addMethod(u"copyLists",0,[](TextParser& parser,FunctionArgs)->std::any{parser.copyLists();return std::any(std::monostate{});});
    addMethod(u"Parse",0,[](TextParser& parser,FunctionArgs)->std::any{parser.Parse();return std::any(std::monostate{});});
    addMethod(u"saveText",0,[](TextParser& parser,FunctionArgs)->std::any{parser.saveText();return std::any(std::monostate{});});
    addMethod(u"Push",1,[&runtime](TextParser& parser,FunctionArgs args)->std::any{
        if(args.empty())throw Exception("TypeError",u"TextParser.Push requires a node or stack item");
        const auto* value=std::any_cast<Value>(&args[0]);const auto* object=value?std::get_if<ObjectRef>(&value->data):nullptr;
        if(!object)throw Exception("Unimplemented",u"TextParser.Push requires a canonical MML node or StackItem identity");
        try{parser.Push(StackEntry(runtime.stackItemIdentities.resolve(*object)));}
        catch(const Exception&){throw Exception("Unimplemented",u"TextParser.Push MML node identity adapter is not installed");}
        return std::any(std::monostate{});
    });
    addMethod(u"PushMath",1,[](TextParser&,FunctionArgs)->std::any{throw Exception("Unimplemented",u"TextParser.PushMath requires a canonical MML node identity adapter");});
    addMethod(u"addAttributes",1,[](TextParser&,FunctionArgs)->std::any{throw Exception("Unimplemented",u"TextParser.addAttributes requires a canonical MML node identity adapter");});
    addMethod(u"ParseTextArg",2,[&runtime,resultValue](TextParser& parser,FunctionArgs args)->std::any{
        const auto* name=args.empty()?nullptr:std::any_cast<Value>(&args[0]);
        if(!name||!std::holds_alternative<std::u16string>(name->data))throw Exception("TypeError",u"TextParser.ParseTextArg name must be a string");
        const Value env=args.size()>1?std::any_cast<Value>(&args[1])?*std::any_cast<Value>(&args[1]):Value(Undefined{}):Value(Undefined{});
        return std::any(parser.ParseTextArg(std::get<std::u16string>(name->data),runtime.properties.from(env)));
    });
    addMethod(u"ParseArg",1,[](TextParser& parser,FunctionArgs args)->std::any{
        if(args.empty())throw Exception("TypeError",u"TextParser.ParseArg name is missing");
        const auto* name=std::any_cast<Value>(&args[0]);
        if(!name||!std::holds_alternative<std::u16string>(name->data))throw Exception("TypeError",u"TextParser.ParseArg name must be a string");
        return std::any(parser.ParseArg(std::get<std::u16string>(name->data)));
    });
    addMethod(u"Error",2,[](TextParser& parser,FunctionArgs args)->std::any{
        if(args.size()<2)throw Exception("TypeError",u"TextParser.Error requires id and message");
        const auto* id=std::any_cast<Value>(&args[0]);const auto* message=std::any_cast<Value>(&args[1]);
        if(!id||!message||!std::holds_alternative<std::u16string>(id->data)||!std::holds_alternative<std::u16string>(message->data))
            throw Exception("TypeError",u"TextParser.Error id and message must be strings");
        std::vector<Property> substitutions;for(std::size_t i=2;i<args.size();++i){const auto* arg=std::any_cast<Value>(&args[i]);if(!arg||!std::holds_alternative<std::u16string>(arg->data))throw Exception("TypeError",u"TextParser.Error substitutions must be strings");substitutions.emplace_back(property_host::debugText(std::get<std::u16string>(arg->data)));}
        parser.Error(std::get<std::u16string>(id->data),std::get<std::u16string>(message->data),substitutions);return std::any(std::monostate{});
    });
}

}
