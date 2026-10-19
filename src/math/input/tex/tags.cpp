/*************************************************************
 * MathJax 4.1.3 input/tex/Tags.ts; Apache-2.0.
 * Source method port. See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.input.tex.parser;
import :parse_options;
import :tex_parser;
import folia.math.input.tex.node_factory;
import folia.math.core.math_item;
import folia.math.core.mml_tree.mml_node;
import folia.math.util.js_case;
import folia.math.util.js_number;
import folia.math.util.js_numeric_input;
import folia.math.util.property_host;
import folia.math.util.regexp_host;
import std;

namespace folia::math::tex {
Label::Label(options::Context& context,options::ObjectRef identity)
    :context_(&context),identity_(identity){context.kind(identity);}
options::Value Label::field(std::u16string_view name)const{
    if(!context_)throw options::Exception("TypeError",u"Label has no Context");
    return context_->get(options::Value(identity_),options::Key(std::u16string(name)));
}
void Label::setField(std::u16string_view name,options::Value value){
    if(!context_)throw options::Exception("TypeError",u"Label has no Context");
    context_->set(identity_,options::Key(std::u16string(name)),std::move(value));
}
void Label::initialize(options::Context& context,options::ObjectRef identity,
                       std::span<const options::Value> arguments){
    auto arg=[&](std::size_t index){return index<arguments.size()?arguments[index]:options::Value(options::Undefined{});};
    auto tag=arg(0),id=arg(1);
    if(std::holds_alternative<options::Undefined>(tag.data))tag=options::Value(u"???");
    if(std::holds_alternative<options::Undefined>(id.data))id=options::Value(u"");
    context.set(identity,options::Key(u"tag"),std::move(tag));
    context.set(identity,options::Key(u"id"),std::move(id));
}

TagInfo::TagInfo(options::Context& context,options::ObjectRef identity)
    :context_(&context),identity_(identity){context.kind(identity);}
options::Value TagInfo::field(std::u16string_view name)const{
    if(!context_)throw options::Exception("TypeError",u"TagInfo has no Context");
    return context_->get(options::Value(identity_),options::Key(std::u16string(name)));
}
void TagInfo::setField(std::u16string_view name,options::Value value){
    if(!context_)throw options::Exception("TypeError",u"TagInfo has no Context");
    context_->set(identity_,options::Key(std::u16string(name)),std::move(value));
}
void TagInfo::initialize(options::Context& context,options::ObjectRef identity,
                         std::span<const options::Value> arguments){
    auto arg=[&](std::size_t index){return index<arguments.size()?arguments[index]:options::Value(options::Undefined{});};
    auto env=arg(0),taggable=arg(1),defaults=arg(2),tag=arg(3),tagId=arg(4),format=arg(5),noTag=arg(6),labelId=arg(7);
    if(std::holds_alternative<options::Undefined>(env.data))env=options::Value(u"");
    if(std::holds_alternative<options::Undefined>(taggable.data))taggable=options::Value(false);
    if(std::holds_alternative<options::Undefined>(defaults.data))defaults=options::Value(false);
    if(std::holds_alternative<options::Undefined>(tag.data))tag=options::Value(options::Null{});
    if(std::holds_alternative<options::Undefined>(tagId.data))tagId=options::Value(u"");
    if(std::holds_alternative<options::Undefined>(format.data))format=options::Value(u"");
    if(std::holds_alternative<options::Undefined>(noTag.data))noTag=options::Value(false);
    if(std::holds_alternative<options::Undefined>(labelId.data))labelId=options::Value(u"");
    context.set(identity,options::Key(u"env"),std::move(env));
    context.set(identity,options::Key(u"taggable"),std::move(taggable));
    context.set(identity,options::Key(u"defaultTags"),std::move(defaults));
    context.set(identity,options::Key(u"tag"),std::move(tag));
    context.set(identity,options::Key(u"tagId"),std::move(tagId));
    context.set(identity,options::Key(u"tagFormat"),std::move(format));
    context.set(identity,options::Key(u"noTag"),std::move(noTag));
    context.set(identity,options::Key(u"labelId"),std::move(labelId));
}

namespace {

using OptionsValue=options::Value;
using OptionsContext=options::Context;

OptionsContext& context(ParserRuntime& runtime){return runtime.context();}
bool nullish(const OptionsValue& value);

[[noreturn]] void type_error(std::u16string message){
    throw options::Exception("TypeError",std::move(message));
}

std::u16string js_text(const OptionsValue& value){
    if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
    if(std::holds_alternative<options::Undefined>(value.data))return u"undefined";
    if(std::holds_alternative<options::Null>(value.data))return u"null";
    if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?u"true":u"false";
    if(const auto* number=std::get_if<double>(&value.data))return property_host::text(js_number::toString(*number));
    if(std::holds_alternative<options::SymbolRef>(value.data))type_error(u"Cannot convert a Symbol value to a string");
    type_error(u"Object ToPrimitive requires the owning Options Context");
}

std::u16string js_text(OptionsContext& ctx,const OptionsValue& value){
    const auto* object=std::get_if<options::ObjectRef>(&value.data);
    if(!object)return js_text(value);
    const std::array<std::u16string_view,2> methods{u"toString",u"valueOf"};
    for(const auto name:methods){
        const auto method=ctx.get(value,options::Key(std::u16string(name)));
        const auto* callable=std::get_if<options::ObjectRef>(&method.data);
        if(!callable||ctx.kind(*callable)!=options::ObjectKind::function)continue;
        const auto primitive=ctx.call(*callable,value,{});
        if(!std::holds_alternative<options::ObjectRef>(primitive.data))return js_text(primitive);
    }
    type_error(u"Cannot convert object to primitive value");
}

bool nullish(const OptionsValue& value){
    return std::holds_alternative<options::Undefined>(value.data)||
           std::holds_alternative<options::Null>(value.data);
}
Property scalar_property(const OptionsValue& value){
    if(std::holds_alternative<options::Undefined>(value.data))return Missing{};
    if(std::holds_alternative<options::Null>(value.data))return PropertyNull{};
    if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean;
    if(const auto* number=std::get_if<double>(&value.data))return *number;
    if(const auto* text=std::get_if<std::u16string>(&value.data))return property_host::debugText(*text);
    throw options::Exception("Unimplemented",u"Context object/symbol counter cannot cross into MathItem Property inputData without the canonical graph bridge");
}
OptionsValue call_method(OptionsContext& ctx,const OptionsValue& receiver,std::u16string_view name,
                         std::span<const OptionsValue> arguments={}){
    const auto method=ctx.get(receiver,options::Key(std::u16string(name)));
    const auto* function=std::get_if<options::ObjectRef>(&method.data);
    if(!function||ctx.kind(*function)!=options::ObjectKind::function)
        type_error(u"method is not a function");
    return ctx.call(*function,receiver,arguments);
}

AbstractTags& abstract(Tags& tags){
    auto* result=dynamic_cast<AbstractTags*>(&tags);
    if(!result)type_error(u"Tags operation requires an AbstractTags instance");
    return *result;
}
const AbstractTags& abstract(const Tags& tags){
    auto* result=dynamic_cast<const AbstractTags*>(&tags);
    if(!result)type_error(u"Tags operation requires an AbstractTags instance");
    return *result;
}
OptionsValue instance_field(Tags& tags,std::u16string_view name){return abstract(tags).field(name);}
OptionsValue instance_field(const Tags& tags,std::u16string_view name){return abstract(tags).field(name);}
void set_instance_field(Tags& tags,std::u16string_view name,OptionsValue value){abstract(tags).setField(name,std::move(value));}
OptionsValue current_info(Tags& tags){return instance_field(tags,u"currentTag");}
OptionsValue source_call(Tags& tags,std::u16string_view method,
                         std::span<const OptionsValue> arguments={}){
    auto& owner=abstract(tags);
    auto& ctx=owner.runtime().context();
    return call_method(ctx,OptionsValue(tags.sourceIdentity()),method,arguments);
}
mml::NodeValue source_call_node(Tags& tags,std::u16string_view method,
                                std::span<const OptionsValue> arguments={}){
    auto& owner=abstract(tags);
    auto& runtime=owner.runtime();
    auto& ctx=runtime.context();
    const OptionsValue receiver(tags.sourceIdentity());
    const OptionsValue methodValue=ctx.get(receiver,options::Key(std::u16string(method)));
    const auto* callable=std::get_if<options::ObjectRef>(&methodValue.data);
    if(!callable||ctx.kind(*callable)!=options::ObjectKind::function)
        type_error(std::u16string(u"this.")+std::u16string(method)+u" is not a function");
    if(const auto* bound=std::any_cast<FunctionRef>(&std::as_const(ctx).payload(*callable));bound&&*bound&&(*bound)->call){
        std::vector<std::any> nativeArgs;nativeArgs.reserve(arguments.size());
        for(const auto& argument:arguments)nativeArgs.emplace_back(argument);
        const auto nativeReceiver=std::static_pointer_cast<NativeObject>(tags.shared_from_this());
        const std::any result=(*bound)->call(FunctionReceiver{nativeReceiver},nativeArgs);
        if(const auto* node=std::any_cast<mml::NodeValue>(&result))return *node;
        if(const auto* value=std::any_cast<OptionsValue>(&result)){
            if(std::holds_alternative<options::Null>(value->data))return mml::NodeValue(PropertyNull{});
            if(std::holds_alternative<options::Undefined>(value->data))return mml::NodeValue(Missing{});
        }
        type_error(std::u16string(u"this.")+std::u16string(method)+u" did not return an MML node");
    }
    const OptionsValue result=ctx.call(*callable,receiver,arguments);
    if(std::holds_alternative<options::Null>(result.data))return mml::NodeValue(PropertyNull{});
    if(std::holds_alternative<options::Undefined>(result.data))return mml::NodeValue(Missing{});
    throw options::Exception("Unimplemented",u"Context MML node result has no canonical NodeValue identity adapter");
}
OptionsValue current_info_field(Tags& tags,std::u16string_view name){
    auto& owner=abstract(tags);
    return owner.runtime().context().get(current_info(tags),options::Key(std::u16string(name)));
}
void set_current_info_field(Tags& tags,std::u16string_view name,OptionsValue value){
    auto& owner=abstract(tags);
    owner.runtime().context().set(current_info(tags),options::Key(std::u16string(name)),std::move(value));
}

TagInfoRef make_tag_info(ParserRuntime& runtime,std::span<const OptionsValue> arguments){
    const auto* constructor=std::get_if<options::ObjectRef>(&runtime.tagInfoClass.data);
    if(!constructor)type_error(u"TagInfo constructor is not initialized");
    return runtime.tagInfoIdentities.resolve(context(runtime).construct(*constructor,arguments));
}

OptionsValue make_label(ParserRuntime& runtime,OptionsValue tag,OptionsValue id){
    const auto* constructor=std::get_if<options::ObjectRef>(&runtime.labelClass.data);
    if(!constructor)type_error(u"Label constructor is not initialized");
    const std::array<OptionsValue,2> arguments{std::move(tag),std::move(id)};
    return context(runtime).construct(*constructor,arguments);
}

ParseOptions& require_configuration(Tags& tags){
    auto& owner=abstract(tags);
    const auto configuration=owner.runtime().context().get(options::Value(tags.sourceIdentity()),options::Key(u"configuration"));
    if(const auto* object=std::get_if<options::ObjectRef>(&configuration.data))
        return *owner.runtime().parseOptionsIdentities.resolve(*object);
    const bool isNull=std::holds_alternative<options::Null>(configuration.data);
    std::string message="Cannot read properties of ";
    message+=isNull?"null":"undefined";
    message+=" (reading 'nodeFactory')";
    throw options::Exception("TypeError",std::u16string(message.begin(),message.end()));
}

NodeFactory& require_node_factory(ParseOptions& configuration){
    return configuration.nodeFactoryOwner();
}

std::u16string trim_tag_id(std::u16string_view id){
    std::u16string result;
    result.reserve(id.size());
    for(char16_t unit:id)result.push_back(js_input::isWhitespace(unit)?u'_':unit);
    return result;
}

std::u16string encode_component(std::u16string_view input){
    std::string bytes;
    bytes.reserve(input.size()*3);
    const auto emit=[&](std::uint32_t cp){
        if(cp<=0x7f)bytes.push_back(static_cast<char>(cp));
        else if(cp<=0x7ff){bytes.push_back(static_cast<char>(0xc0|(cp>>6)));bytes.push_back(static_cast<char>(0x80|(cp&63)));}
        else if(cp<=0xffff){bytes.push_back(static_cast<char>(0xe0|(cp>>12)));bytes.push_back(static_cast<char>(0x80|((cp>>6)&63)));bytes.push_back(static_cast<char>(0x80|(cp&63)));}
        else{bytes.push_back(static_cast<char>(0xf0|(cp>>18)));bytes.push_back(static_cast<char>(0x80|((cp>>12)&63)));bytes.push_back(static_cast<char>(0x80|((cp>>6)&63)));bytes.push_back(static_cast<char>(0x80|(cp&63)));}
    };
    for(std::size_t i=0;i<input.size();++i){
        const auto first=static_cast<std::uint16_t>(input[i]);
        std::uint32_t cp=first;
        if(first>=0xd800&&first<=0xdbff){
            if(i+1>=input.size())throw options::Exception("URIError",u"URI malformed");
            const auto second=static_cast<std::uint16_t>(input[i+1]);
            if(second<0xdc00||second>0xdfff)throw options::Exception("URIError",u"URI malformed");
            cp=0x10000+((first-0xd800)<<10)+(second-0xdc00);++i;
        }else if(first>=0xdc00&&first<=0xdfff){
            throw options::Exception("URIError",u"URI malformed");
        }
        emit(cp);
    }
    constexpr char hex[]= "0123456789ABCDEF";
    std::u16string encoded;
    encoded.reserve(bytes.size()*3);
    for(unsigned char byte:bytes){
        const bool safe=(byte>='A'&&byte<='Z')||(byte>='a'&&byte<='z')||
            (byte>='0'&&byte<='9')||byte=='-'||byte=='_'||byte=='.'||byte=='!'||
            byte=='~'||byte=='*'||byte=='\''||byte=='('||byte==')';
        if(safe)encoded.push_back(static_cast<char16_t>(byte));
        else{encoded.push_back(u'%');encoded.push_back(static_cast<char16_t>(hex[byte>>4]));encoded.push_back(static_cast<char16_t>(hex[byte&15]));}
    }
    return encoded;
}

mml::NodeValue create_node(NodeFactory& factory,std::u16string kind,
                           mml::Children children,PropertyList definition=PropertyList{}){
    const std::array<std::any,3> args{std::any(Property(property_host::debugText(kind))),
                                     std::any(std::move(children)),std::any(Property(std::move(definition)))};
    return factory.create("node",args);
}

mml::Children child_array(std::initializer_list<mml::NodeValue> nodes){
    auto result=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    for(const auto& node:nodes)result->push_back(node);
    return result;
}

options::ObjectRef as_object(const OptionsValue& value,std::u16string_view member){
    if(const auto* object=std::get_if<options::ObjectRef>(&value.data))return *object;
    std::u16string message=u"Cannot read properties of undefined (reading '";
    message+=member;message+=u"')";type_error(std::move(message));
}
TagsRef receiver_tags(ParserRuntime& runtime,OptionsValue receiver){
    const auto object=as_object(receiver,u"start");
    return runtime.tagIdentities.resolve(object);
}
AbstractTags& receiver_abstract(ParserRuntime& runtime,OptionsValue receiver){
    const auto tags=receiver_tags(runtime,std::move(receiver));
    auto* result=dynamic_cast<AbstractTags*>(tags.get());
    if(!result)type_error(u"Tags receiver is not an AbstractTags instance");
    return *result;
}
OptionsValue method_argument(std::span<const OptionsValue> arguments,std::size_t index){
    return index<arguments.size()?arguments[index]:OptionsValue(options::Undefined{});
}
void define_method(ParserRuntime& runtime,OptionsContext& context,options::ObjectRef prototype,
                   std::u16string name,double length,options::NativeCall call){
    auto body=std::make_shared<options::NativeCall>(std::move(call));
    auto function=std::make_shared<Function>();
    function->call=[ctx=&context,body](const FunctionReceiver& receiver,FunctionArgs raw)->std::any{
        const auto* native=std::get_if<NativeObjectRef>(&receiver);
        const auto tags=native&&*native?std::dynamic_pointer_cast<Tags>(*native):TagsRef{};
        if(!tags)type_error(u"Tags method called on incompatible receiver");
        std::vector<OptionsValue> arguments;arguments.reserve(raw.size());
        for(const auto& item:raw){
            const auto* value=std::any_cast<OptionsValue>(&item);
            if(!value)type_error(u"Tags method argument has no Context value");
            arguments.push_back(*value);
        }
        return (*body)(*ctx,OptionsValue(tags->sourceIdentity()),std::span<const OptionsValue>(arguments));
    };
    OptionCallBinding binding;
    binding.receiver=[&runtime](OptionsContext&,OptionsValue receiver)->FunctionReceiver{
        const auto* object=std::get_if<options::ObjectRef>(&receiver.data);
        if(!object)type_error(u"Tags method called on incompatible receiver");
        return std::static_pointer_cast<NativeObject>(runtime.tagIdentities.resolve(*object));
    };
    binding.arguments=[](OptionsContext&,std::span<const OptionsValue> args){
        std::vector<std::any> result;result.reserve(args.size());
        for(const auto& argument:args)result.emplace_back(argument);
        return result;
    };
    binding.result=[](OptionsContext&,const std::any& result)->OptionsValue{
        if(const auto* value=std::any_cast<OptionsValue>(&result))return *value;
        throw options::Exception("Unimplemented",u"Tags method result has no Context value adapter");
    };
    const auto value=runtime.functions().bind(std::move(function),std::move(binding),u"");
    const auto* functionObject=std::get_if<options::ObjectRef>(&value.data);
    if(!functionObject)type_error(u"Tags method callable binding is not a Context function");
    context.define(*functionObject,options::Key(u"length"),options::Descriptor::data(length,false,false,true));
    context.define(prototype,options::Key(std::move(name)),options::Descriptor::data(value,true,true,true));
}
OptionsValue node_to_option_value(mml::NodeValue value){
    if(value.kind()==tree::ChildKind::Undefined)return OptionsValue(options::Undefined{});
    if(value.kind()==tree::ChildKind::Null)return OptionsValue(options::Null{});
    throw options::Exception("Unimplemented",u"MML NodeValue has no canonical Options Context identity binding");
}
void define_node_method(ParserRuntime& runtime,OptionsContext& context,options::ObjectRef prototype,
                        std::u16string name,double length,
                        std::function<mml::NodeValue(AbstractTags&,std::span<const OptionsValue>)> body){
    auto function=std::make_shared<Function>();
    function->call=[body=std::move(body)](const FunctionReceiver& receiver,FunctionArgs raw)->std::any{
        const auto* native=std::get_if<NativeObjectRef>(&receiver);
        const auto tags=native&&*native?std::dynamic_pointer_cast<Tags>(*native):TagsRef{};
        if(!tags)type_error(u"Tags method called on incompatible receiver");
        auto* abstractTags=dynamic_cast<AbstractTags*>(tags.get());
        if(!abstractTags)type_error(u"Tags receiver is not an AbstractTags instance");
        std::vector<OptionsValue> arguments;arguments.reserve(raw.size());
        for(const auto& item:raw){
            const auto* value=std::any_cast<OptionsValue>(&item);
            if(!value)type_error(u"Tags method argument has no Context value");
            arguments.push_back(*value);
        }
        return body(*abstractTags,std::span<const OptionsValue>(arguments));
    };
    OptionCallBinding binding;
    binding.receiver=[&runtime](OptionsContext&,OptionsValue receiver)->FunctionReceiver{
        const auto* object=std::get_if<options::ObjectRef>(&receiver.data);
        if(!object)type_error(u"Tags method called on incompatible receiver");
        return std::static_pointer_cast<NativeObject>(runtime.tagIdentities.resolve(*object));
    };
    binding.arguments=[](OptionsContext&,std::span<const OptionsValue> args){
        std::vector<std::any> result;result.reserve(args.size());
        for(const auto& argument:args)result.emplace_back(argument);
        return result;
    };
    binding.result=[](OptionsContext&,const std::any& result)->OptionsValue{
        const auto* node=std::any_cast<mml::NodeValue>(&result);
        if(!node)type_error(u"Tags node method did not return an MML NodeValue");
        return node_to_option_value(*node);
    };
    const auto value=runtime.functions().bind(std::move(function),std::move(binding),u"");
    const auto* functionObject=std::get_if<options::ObjectRef>(&value.data);
    if(!functionObject)type_error(u"Tags method callable binding is not a Context function");
    context.define(*functionObject,options::Key(u"length"),options::Descriptor::data(length,false,false,true));
    context.define(prototype,options::Key(std::move(name)),options::Descriptor::data(value,true,true,true));
}
void define_native_method(ParserRuntime& runtime,OptionsContext& context,options::ObjectRef prototype,
                          std::u16string name,double length,
                          std::function<std::any(Tags&,FunctionArgs)> body,
                          std::function<OptionsValue(OptionsContext&,const std::any&)> resultAdapter){
    auto function=std::make_shared<Function>();
    function->call=[body=std::move(body)](const FunctionReceiver& receiver,FunctionArgs args)->std::any{
        const auto* native=std::get_if<NativeObjectRef>(&receiver);
        const auto tags=native&&*native?std::dynamic_pointer_cast<Tags>(*native):TagsRef{};
        if(!tags)type_error(u"Tags method called on incompatible receiver");
        return body(*tags,args);
    };
    OptionCallBinding binding;
    binding.receiver=[&runtime](OptionsContext&,OptionsValue receiver)->FunctionReceiver{
        const auto* object=std::get_if<options::ObjectRef>(&receiver.data);
        if(!object)type_error(u"Tags method called on incompatible receiver");
        return std::static_pointer_cast<NativeObject>(runtime.tagIdentities.resolve(*object));
    };
    binding.arguments=[](OptionsContext&,std::span<const OptionsValue> args){
        std::vector<std::any> result;result.reserve(args.size());
        for(const auto& argument:args)result.emplace_back(argument);
        return result;
    };
    binding.result=std::move(resultAdapter);
    const auto value=runtime.functions().bind(std::move(function),std::move(binding),u"");
    const auto* functionObject=std::get_if<options::ObjectRef>(&value.data);
    if(!functionObject)type_error(u"Tags method callable binding is not a Context function");
    context.define(*functionObject,options::Key(u"length"),options::Descriptor::data(length,false,false,true));
    context.define(prototype,options::Key(std::move(name)),options::Descriptor::data(value,true,true,true));
}

}

void AbstractTags::initializeAbstractTags(){
    auto& ctx=context(runtime_);
    const auto canonical=runtime_.tagIdentities.find(shared_from_this());
    if(!canonical)type_error(u"Tags instance must be identity-bound before initialization");
    identity_=*canonical;
    ctx.set(identity_,options::Key(u"counter"),0);
    ctx.set(identity_,options::Key(u"allCounter"),0);
    ctx.set(identity_,options::Key(u"configuration"),options::Null{});
    ctx.set(identity_,options::Key(u"ids"),ctx.object());
    ctx.set(identity_,options::Key(u"allIds"),ctx.object());
    ctx.set(identity_,options::Key(u"labels"),ctx.object());
    ctx.set(identity_,options::Key(u"allLabels"),ctx.object());
    ctx.set(identity_,options::Key(u"redo"),false);
    ctx.set(identity_,options::Key(u"refUpdate"),false);
    const auto initialTag=make_tag_info(runtime_,{});
    const auto initialTagObject=runtime_.tagInfoIdentities.find(initialTag);
    if(!initialTagObject)type_error(u"TagInfo constructor returned an unbound instance");
    ctx.set(identity_,options::Key(u"currentTag"),*initialTagObject);
    ctx.set(identity_,options::Key(u"history"),ctx.array());
    ctx.set(identity_,options::Key(u"stack"),ctx.array());

    // Source declares an own function-valued field per instance. The callable
    // uses its invocation receiver, as the original non-arrow function does.
    auto function=std::make_shared<Function>();
    function->call=[runtime=&runtime_](const FunctionReceiver& receiver,FunctionArgs args)->std::any{
        const auto* object=std::get_if<NativeObjectRef>(&receiver);
        if(!object||!*object)type_error(u"Cannot read properties of undefined (reading 'configuration')");
        auto tags=std::dynamic_pointer_cast<Tags>(*object);
        if(!tags)type_error(u"enTag receiver is not a Tags instance");
        if(args.size()<2)type_error(u"Cannot read properties of undefined (reading 'requireNode')");
        const auto* node=std::any_cast<mml::NodeValue>(&args[0]);
        const auto* tag=std::any_cast<mml::NodeValue>(&args[1]);
        if(!node)type_error(u"enTag node argument is not a native MML node value");
        if(!tag)type_error(u"enTag tag argument is not a native MML node value");
        auto& configuration=require_configuration(*tags);
        auto& factory=require_node_factory(configuration);
        const auto cell=create_node(factory,u"mtd",child_array({*node}));
        const auto row=create_node(factory,u"mlabeledtr",child_array({*tag,cell}));
        const auto side=runtime->context().get(configuration.options,options::Key(u"tagSide"));
        const auto spacing=runtime->context().get(configuration.options,options::Key(u"tagIndent"));
        const PropertyList definition{{"side",property_host::debugText(js_text(side))},
                                       {"minlabelspacing",property_host::debugText(js_text(spacing))},
                                       {"displaystyle",true}};
        return create_node(factory,u"mtable",child_array({row}),definition);
    };
    OptionCallBinding binding;
    binding.receiver=[runtime=&runtime_](OptionsContext&,OptionsValue receiver)->FunctionReceiver{
        const auto* object=std::get_if<options::ObjectRef>(&receiver.data);
        if(!object)type_error(u"Cannot read properties of undefined (reading 'configuration')");
        const auto tags=runtime->tagIdentities.resolve(*object);
        return std::static_pointer_cast<NativeObject>(tags);
    };
    binding.arguments=[](OptionsContext&,std::span<const OptionsValue> arguments){
        std::vector<std::any> result;result.reserve(arguments.size());
        for(const auto& argument:arguments)result.emplace_back(argument);
        return result;
    };
    binding.result=[](OptionsContext&,const std::any&)->OptionsValue{
        throw options::Exception("Unimplemented",u"MML value has no canonical option Context identity adapter");
    };
    const auto functionValue=runtime_.functions().bind(std::move(function),std::move(binding),u"");
    const auto* enTagObject=std::get_if<options::ObjectRef>(&functionValue.data);
    if(!enTagObject)type_error(u"Tags enTag field is not a Context function");
    ctx.define(*enTagObject,options::Key(u"length"),options::Descriptor::data(2,false,false,true));
    ctx.set(identity_,options::Key(u"enTag"),functionValue);
}

ParserRuntime& AbstractTags::runtime()const{return runtime_;}
AbstractTags::AbstractTags(ParserRuntime& runtime):runtime_(runtime){}
OptionsValue AbstractTags::field(std::u16string_view name)const{
    return context(runtime_).get(OptionsValue(identity_),options::Key(std::u16string(name)));
}
void AbstractTags::setField(std::u16string_view name,OptionsValue value){
    context(runtime_).set(identity_,options::Key(std::u16string(name)),std::move(value));
}

void AbstractTags::initializeModule(ParserRuntime& runtime){
    auto& ctx=runtime.context();
    const auto ctor=[&](const OptionsValue& value){
        const auto* object=std::get_if<options::ObjectRef>(&value.data);
        if(!object)type_error(u"Tags class constructor is not initialized");
        return *object;
    };
    const auto prototype=[&](const OptionsValue& value){
        return as_object(ctx.get(OptionsValue(ctor(value)),options::Key(u"prototype")),u"prototype");
    };
    const auto base=prototype(runtime.abstractTagsClass);
    const auto none=prototype(runtime.noTagsClass);
    const auto all=prototype(runtime.allTagsClass);

    define_method(runtime,ctx,base,u"start",3,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue> args){
        receiver_abstract(runtime,std::move(receiver)).start(method_argument(args,0),method_argument(args,1),method_argument(args,2));
        return OptionsValue(options::Undefined{});
    });
    define_method(runtime,ctx,base,u"end",0,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue>){
        receiver_abstract(runtime,std::move(receiver)).end();return OptionsValue(options::Undefined{});
    });
    define_method(runtime,ctx,base,u"tag",2,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue> args){
        receiver_abstract(runtime,std::move(receiver)).tag(method_argument(args,0),method_argument(args,1));
        return OptionsValue(options::Undefined{});
    });
    define_method(runtime,ctx,base,u"notag",0,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue>){
        receiver_abstract(runtime,std::move(receiver)).notag();return OptionsValue(options::Undefined{});
    });
    define_method(runtime,ctx,base,u"formatUrl",2,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue> args){
        return OptionsValue(receiver_abstract(runtime,std::move(receiver)).formatUrl(method_argument(args,0),method_argument(args,1)));
    });
    define_method(runtime,ctx,base,u"formatTag",1,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue> args){
        return receiver_abstract(runtime,std::move(receiver)).formatTag(method_argument(args,0));
    });
    define_method(runtime,ctx,base,u"formatRef",1,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue> args){
        return receiver_abstract(runtime,std::move(receiver)).formatRef(method_argument(args,0));
    });
    define_method(runtime,ctx,base,u"autoTag",0,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue>){
        receiver_tags(runtime,std::move(receiver))->autoTag();return OptionsValue(options::Undefined{});
    });
    define_method(runtime,ctx,base,u"clearTag",0,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue>){
        receiver_tags(runtime,std::move(receiver))->clearTag();return OptionsValue(options::Undefined{});
    });
    define_node_method(runtime,ctx,base,u"getTag",1,[](AbstractTags& self,std::span<const OptionsValue> args){
        OptionsValue force=method_argument(args,0);
        if(std::holds_alternative<options::Undefined>(force.data))force=OptionsValue(false);
        return self.getTag(std::move(force));
    });
    define_node_method(runtime,ctx,base,u"makeTag",0,[](AbstractTags& self,std::span<const OptionsValue>){return self.makeTag();});
    define_method(runtime,ctx,base,u"makeId",0,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue>){
        receiver_abstract(runtime,std::move(receiver)).makeId();return OptionsValue(options::Undefined{});
    });
    define_method(runtime,ctx,base,u"formatId",1,[&runtime](OptionsContext& inner,OptionsValue receiver,std::span<const OptionsValue> args){
        auto& owner=receiver_abstract(runtime,std::move(receiver));
        (void)inner;
        return owner.formatId(method_argument(args,0));
    });
    define_method(runtime,ctx,base,u"formatNumber",1,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue> args){
        auto& owner=receiver_abstract(runtime,std::move(receiver));
        return owner.formatNumber(method_argument(args,0));
    });
    define_method(runtime,ctx,base,u"resetTag",0,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue>){
        receiver_tags(runtime,std::move(receiver))->resetTag();return OptionsValue(options::Undefined{});
    });
    define_method(runtime,ctx,base,u"reset",1,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue> args){
        auto offset=method_argument(args,0);if(std::holds_alternative<options::Undefined>(offset.data))offset=OptionsValue(0);
        receiver_tags(runtime,std::move(receiver))->reset(std::move(offset));return OptionsValue(options::Undefined{});
    });
    const auto mathItemArgument=[&runtime](const std::any& raw)->core::MathItem&{
        if(const auto* item=std::any_cast<core::MathItem*>(&raw);item&&*item)return **item;
        if(const auto* native=std::any_cast<NativeObjectRef>(&raw);native&&*native){
            auto item=std::dynamic_pointer_cast<core::MathItem>(*native);
            if(item)return *item;
        }
        if(const auto* value=std::any_cast<OptionsValue>(&raw)){
            if(const auto* object=std::get_if<options::ObjectRef>(&value->data)){
                const auto& payload=std::as_const(runtime.context()).payload(*object);
                if(const auto* item=std::any_cast<core::MathItem*>(&payload);item&&*item)return **item;
                if(const auto* native=std::any_cast<NativeObjectRef>(&payload);native&&*native){
                    auto item=std::dynamic_pointer_cast<core::MathItem>(*native);
                    if(item)return *item;
                }
            }
        }
        throw options::Exception("Unimplemented",u"MathItem Context identity adapter is not available for Tags equation methods");
    };
    define_native_method(runtime,ctx,base,u"startEquation",1,
        [mathItemArgument](Tags& tags,FunctionArgs args)->std::any{
            if(args.empty())type_error(u"Cannot read properties of undefined (reading 'inputData')");
            tags.startEquation(mathItemArgument(args[0]));return OptionsValue(options::Undefined{});
        },[](OptionsContext&,const std::any& result){
            if(const auto* value=std::any_cast<OptionsValue>(&result))return *value;
            throw options::Exception("Unimplemented",u"Tags.startEquation result has no Context value adapter");
        });
    define_native_method(runtime,ctx,base,u"finishEquation",1,
        [mathItemArgument](Tags& tags,FunctionArgs args)->std::any{
            if(args.empty())type_error(u"Cannot read properties of undefined (reading 'redo')");
            tags.finishEquation(mathItemArgument(args[0]));return OptionsValue(options::Undefined{});
        },[](OptionsContext&,const std::any& result){
            if(const auto* value=std::any_cast<OptionsValue>(&result))return *value;
            throw options::Exception("Unimplemented",u"Tags.finishEquation result has no Context value adapter");
        });
    const auto nodeArgument=[&runtime](const std::any& raw)->mml::NodeValue{
        if(const auto* node=std::any_cast<mml::NodeValue>(&raw))return *node;
        if(const auto* value=std::any_cast<OptionsValue>(&raw)){
            if(std::holds_alternative<options::Undefined>(value->data))return mml::NodeValue(Missing{});
            if(std::holds_alternative<options::Null>(value->data))return mml::NodeValue(PropertyNull{});
            if(const auto* object=std::get_if<options::ObjectRef>(&value->data)){
                const auto& payload=std::as_const(runtime.context()).payload(*object);
                if(const auto* node=std::any_cast<mml::NodeValue>(&payload))return *node;
            }
        }
        throw options::Exception("Unimplemented",u"MML NodeValue Context identity adapter is not available for Tags.finalize");
    };
    const auto environmentArgument=[&runtime](const std::any& raw)->Property{
        if(const auto* environment=std::any_cast<Property>(&raw))return *environment;
        if(const auto* value=std::any_cast<OptionsValue>(&raw)){
            if(std::holds_alternative<options::Undefined>(value->data))return Missing{};
            if(std::holds_alternative<options::Null>(value->data))return PropertyNull{};
            if(const auto* boolean=std::get_if<bool>(&value->data))return *boolean;
            if(const auto* number=std::get_if<double>(&value->data))return *number;
            if(const auto* text=std::get_if<std::u16string>(&value->data))return property_host::debugText(*text);
            if(const auto* object=std::get_if<options::ObjectRef>(&value->data)){
                const auto& payload=std::as_const(runtime.context()).payload(*object);
                if(const auto* environment=std::any_cast<Property>(&payload))return *environment;
            }
        }
        throw options::Exception("Unimplemented",u"EnvList Context identity adapter is not available for Tags.finalize");
    };
    define_native_method(runtime,ctx,base,u"finalize",2,
        [nodeArgument,environmentArgument](Tags& tags,FunctionArgs args)->std::any{
            if(args.size()<2)type_error(u"Cannot read properties of undefined (reading 'display')");
            const auto node=nodeArgument(args[0]);
            const auto environment=environmentArgument(args[1]);
            return tags.finalize(node,environment);
        },[](OptionsContext&,const std::any& result){
            const auto* node=std::any_cast<mml::NodeValue>(&result);
            if(!node)type_error(u"Tags.finalize did not return an MML NodeValue");
            return node_to_option_value(*node);
        });

    const auto envGetter=ctx.function(u"get",[](OptionsContext& inner,OptionsValue receiver,std::span<const OptionsValue>){
        const auto current=inner.get(receiver,options::Key(u"currentTag"));
        return inner.get(current,options::Key(u"env"));
    });
    ctx.define(envGetter,options::Key(u"length"),options::Descriptor::data(0,false,false,true));
    ctx.define(base,options::Key(u"env"),options::Descriptor::access(envGetter,std::nullopt,false,true));
    const auto labelGetter=ctx.function(u"get",[](OptionsContext& inner,OptionsValue receiver,std::span<const OptionsValue>){
        const auto current=inner.get(receiver,options::Key(u"currentTag"));
        return inner.get(current,options::Key(u"labelId"));
    });
    const auto labelSetter=ctx.function(u"set",[](OptionsContext& inner,OptionsValue receiver,std::span<const OptionsValue> args){
        const auto current=inner.get(receiver,options::Key(u"currentTag"));
        inner.set(current,options::Key(u"labelId"),method_argument(args,0));
        return OptionsValue(options::Undefined{});
    });
    ctx.define(labelGetter,options::Key(u"length"),options::Descriptor::data(0,false,false,true));
    ctx.define(labelSetter,options::Key(u"length"),options::Descriptor::data(1,false,false,true));
    ctx.define(base,options::Key(u"label"),options::Descriptor::access(labelGetter,labelSetter,false,true));
    const auto noTagGetter=ctx.function(u"get",[](OptionsContext& inner,OptionsValue receiver,std::span<const OptionsValue>){
        const auto current=inner.get(receiver,options::Key(u"currentTag"));
        return inner.get(current,options::Key(u"noTag"));
    });
    ctx.define(noTagGetter,options::Key(u"length"),options::Descriptor::data(0,false,false,true));
    ctx.define(base,options::Key(u"noTag"),options::Descriptor::access(noTagGetter,std::nullopt,false,true));

    define_method(runtime,ctx,none,u"autoTag",0,[&runtime](OptionsContext&,OptionsValue receiver,std::span<const OptionsValue>){
        auto tags=receiver_tags(runtime,std::move(receiver));tags->autoTag();return OptionsValue(options::Undefined{});
    });
    define_node_method(runtime,ctx,none,u"getTag",0,[](AbstractTags& self,std::span<const OptionsValue>){
        auto* noTags=dynamic_cast<NoTags*>(&self);
        if(!noTags)type_error(u"NoTags receiver is incompatible");
        return noTags->getTag(options::Undefined{});
    });
    define_native_method(runtime,ctx,all,u"finalize",2,
        [nodeArgument,environmentArgument](Tags& tags,FunctionArgs args)->std::any{
            if(args.size()<2)type_error(u"Cannot read properties of undefined (reading 'display')");
            const auto node=nodeArgument(args[0]);
            const auto environment=environmentArgument(args[1]);
            auto* allTags=dynamic_cast<AllTags*>(&tags);
            if(!allTags)type_error(u"AllTags receiver is incompatible");
            return allTags->finalize(node,environment);
        },[](OptionsContext&,const std::any& result){
            const auto* node=std::any_cast<mml::NodeValue>(&result);
            if(!node)type_error(u"AllTags.finalize did not return an MML NodeValue");
            return node_to_option_value(*node);
        });
}

bool AbstractTags::noTag()const{
    return options::truthy(current_info_field(const_cast<AbstractTags&>(*this),u"noTag"));
}

OptionsValue AbstractTags::env()const{
    return current_info_field(const_cast<AbstractTags&>(*this),u"env");
}
OptionsValue AbstractTags::label()const{
    return current_info_field(const_cast<AbstractTags&>(*this),u"labelId");
}
void AbstractTags::setLabel(OptionsValue value){set_current_info_field(*this,u"labelId",std::move(value));}

TagFormat AbstractTags::formatTag(OptionsValue tag){
    auto& ctx=context(runtime_);
    const auto array=ctx.array(3);
    ctx.set(OptionsValue(array),options::Key(u"0"),u"(");
    ctx.set(OptionsValue(array),options::Key(u"1"),std::move(tag));
    ctx.set(OptionsValue(array),options::Key(u"2"),u")");
    return array;
}
TagFormat AbstractTags::formatRef(OptionsValue tag){
    const std::array<OptionsValue,1> arguments{std::move(tag)};
    return source_call(*this,u"formatTag",arguments);
}
OptionsValue AbstractTags::formatUrl(OptionsValue id,OptionsValue base){
    const auto baseText=js_text(context(runtime_),base);
    const auto idText=js_text(context(runtime_),id);
    return OptionsValue(baseText+u"#"+encode_component(idText));
}

void AbstractTags::start(OptionsValue environment,OptionsValue taggable,OptionsValue defaultTags){
    auto& ctx=context(runtime_);
    const auto testCurrent=field(u"currentTag");
    if(options::truthy(testCurrent)){
        const auto stackValue=field(u"stack");
        const auto push=ctx.get(stackValue,options::Key(u"push"));
        const auto* pushFunction=std::get_if<options::ObjectRef>(&push.data);
        if(!pushFunction||ctx.kind(*pushFunction)!=options::ObjectKind::function)type_error(u"this.stack.push is not a function");
        const auto current=field(u"currentTag");
        const std::array<OptionsValue,1> args{current};
        ctx.call(*pushFunction,stackValue,args);
    }
    const OptionsValue savedLabel=ctx.get(OptionsValue(identity_),options::Key(u"label"));
    const std::array<OptionsValue,3> constructorArgs{std::move(environment),std::move(taggable),std::move(defaultTags)};
    const auto next=make_tag_info(runtime_,constructorArgs);
    const auto object=runtime_.tagInfoIdentities.find(next);
    if(!object)type_error(u"TagInfo constructor returned an unbound instance");
    setField(u"currentTag",*object);
    ctx.set(OptionsValue(identity_),options::Key(u"label"),savedLabel);
}
void AbstractTags::end(){
    auto& ctx=context(runtime_);
    const auto historyValue=field(u"history");
    const auto push=ctx.get(historyValue,options::Key(u"push"));
    const auto* pushFunction=std::get_if<options::ObjectRef>(&push.data);
    if(!pushFunction||ctx.kind(*pushFunction)!=options::ObjectKind::function)type_error(u"this.history.push is not a function");
    const auto current=field(u"currentTag");
    const std::array<OptionsValue,1> args{current};
    ctx.call(*pushFunction,historyValue,args);
    const OptionsValue savedLabel=ctx.get(OptionsValue(identity_),options::Key(u"label"));
    const auto stackValue=field(u"stack");
    const auto pop=ctx.get(stackValue,options::Key(u"pop"));
    const auto* popFunction=std::get_if<options::ObjectRef>(&pop.data);
    if(!popFunction||ctx.kind(*popFunction)!=options::ObjectKind::function)type_error(u"this.stack.pop is not a function");
    setField(u"currentTag",ctx.call(*popFunction,stackValue,{}));
    if(options::truthy(savedLabel)&&!options::truthy(ctx.get(OptionsValue(identity_),options::Key(u"label"))))
        ctx.set(OptionsValue(identity_),options::Key(u"label"),savedLabel);
}
void AbstractTags::tag(OptionsValue value,OptionsValue noFormat){
    set_current_info_field(*this,u"tag",value);
    OptionsValue format=std::move(value);
    if(!options::truthy(noFormat)){
        const std::array<OptionsValue,1> arguments{format};
        format=source_call(*this,u"formatTag",arguments);
    }
    set_current_info_field(*this,u"tagFormat",std::move(format));
    set_current_info_field(*this,u"noTag",false);
}
void AbstractTags::notag(){
    const std::array<OptionsValue,2> arguments{OptionsValue(std::u16string{}),OptionsValue(true)};
    (void)source_call(*this,u"tag",arguments);
    set_current_info_field(*this,u"noTag",true);
}

OptionsValue AbstractTags::formatId(OptionsValue id){
    const auto* text=std::get_if<std::u16string>(&id.data);
    if(!text)type_error(u"id.replace is not a function");
    return OptionsValue(u"mjx-eqn:"+trim_tag_id(*text));
}
OptionsValue AbstractTags::formatNumber(OptionsValue number){
    auto& ctx=context(runtime_);
    const OptionsValue primitive=ctx.toPrimitive(std::move(number),options::PrimitiveHint::stringHint);
    return OptionsValue(js_text(primitive));
}

void AbstractTags::autoTag(){
    const OptionsValue tag=current_info_field(*this,u"tag");
    if(nullish(tag)){
        const double counter=context(runtime_).toNumber(field(u"counter"))+1;
        setField(u"counter",counter);
        const std::array<OptionsValue,1> formatArguments{OptionsValue(counter)};
        const auto formatted=source_call(*this,u"formatNumber",formatArguments);
        const std::array<OptionsValue,2> tagArguments{formatted,OptionsValue(false)};
        (void)source_call(*this,u"tag",tagArguments);
    }
}
void AbstractTags::clearTag(){
    const std::array<OptionsValue,2> arguments{OptionsValue(options::Null{}),OptionsValue(true)};
    (void)source_call(*this,u"tag",arguments);
    set_current_info_field(*this,u"tagId",u"");
}
mml::NodeValue AbstractTags::getTag(OptionsValue force){
    if(options::truthy(force)){
        (void)source_call(*this,u"autoTag");
        return source_call_node(*this,u"makeTag");
    }
    auto& ctx=context(runtime_);
    const auto captured=field(u"currentTag");
    if(options::truthy(ctx.get(captured,options::Key(u"taggable")))&&
       !options::truthy(ctx.get(captured,options::Key(u"noTag")))){
        if(options::truthy(ctx.get(captured,options::Key(u"defaultTags"))))(void)source_call(*this,u"autoTag");
        if(options::truthy(ctx.get(captured,options::Key(u"tag"))))return source_call_node(*this,u"makeTag");
    }
    return mml::NodeValue(PropertyNull{});
}
void AbstractTags::resetTag(){
    setField(u"history",context(runtime_).array());
    setField(u"redo",false);
    setField(u"refUpdate",false);
    (void)source_call(*this,u"clearTag");
}
void AbstractTags::reset(OptionsValue offset){
    (void)source_call(*this,u"resetTag");
    setField(u"allCounter",offset);
    setField(u"counter",std::move(offset));
    auto& ctx=context(runtime_);
    setField(u"allLabels",ctx.object());
    setField(u"allIds",ctx.object());
    ctx.set(OptionsValue(identity_),options::Key(u"label"),OptionsValue(u""));
}
void AbstractTags::startEquation(core::MathItem& math){
    setField(u"history",context(runtime_).array());
    setField(u"stack",context(runtime_).array());
    (void)source_call(*this,u"clearTag");
    const std::array<OptionsValue,3> constructorArgs{OptionsValue(u""),OptionsValue(options::Undefined{}),OptionsValue(options::Undefined{})};
    const auto current=make_tag_info(runtime_,constructorArgs);
    const auto object=runtime_.tagInfoIdentities.find(current);
    if(!object)type_error(u"TagInfo constructor returned an unbound instance");
    setField(u"currentTag",*object);
    auto& ctx=context(runtime_);
    setField(u"labels",ctx.object());
    setField(u"ids",ctx.object());
    setField(u"counter",field(u"allCounter"));
    setField(u"redo",false);
    const Property inputData=math.inputData;
    const Property recompile=property_host::get(inputData,"recompile");
    if(property_host::truthy(recompile)){
        setField(u"refUpdate",true);
        setField(u"counter",property_host::number(property_host::get(recompile,"counter")));
    }
}
void AbstractTags::finishEquation(core::MathItem& math){
    if(options::truthy(field(u"redo"))){
        const Property inputData=math.inputData;
        const Property state=math.state();
        const auto savedCounter=field(u"allCounter");
        property_host::set(inputData,"recompile",PropertyList{{"state",state},{"counter",scalar_property(savedCounter)}});
    }
    if(!options::truthy(field(u"refUpdate")))setField(u"allCounter",field(u"counter"));
    auto& ctx=context(runtime_);
    const OptionsValue idsTarget=field(u"allIds");
    const OptionsValue idsSource=field(u"ids");
    if(std::holds_alternative<options::ObjectRef>(idsTarget.data)&&
       std::holds_alternative<options::ObjectRef>(idsSource.data)){
        const auto keys=ctx.enumerableStringKeys(idsSource);
        for(const auto& key:keys){const auto value=ctx.get(idsSource,options::Key(key));ctx.set(idsTarget,options::Key(key),value);}
    }else type_error(u"Object.assign target is not an object");
    const OptionsValue labelsTarget=field(u"allLabels");
    const OptionsValue labelsSource=field(u"labels");
    if(std::holds_alternative<options::ObjectRef>(labelsTarget.data)&&
       std::holds_alternative<options::ObjectRef>(labelsSource.data)){
        const auto keys=ctx.enumerableStringKeys(labelsSource);
        for(const auto& key:keys){const auto value=ctx.get(labelsSource,options::Key(key));ctx.set(labelsTarget,options::Key(key),value);}
    }else type_error(u"Object.assign target is not an object");
}
mml::NodeValue AbstractTags::finalize(mml::NodeValue node,Property environment){
    if(!property_host::truthy(property_host::get(environment,"display"))||
       options::truthy(current_info_field(*this,u"env"))||
       nullish(current_info_field(*this,u"tag")))return node;
    const mml::NodeValue tagNode=source_call_node(*this,u"makeTag");
    auto& ctx=context(runtime_);
    const auto callable=ctx.get(OptionsValue(identity_),options::Key(u"enTag"));
    FunctionValue enTagValue;
    try{enTagValue=runtime_.functions().functionValue(callable);}
    catch(...){throw options::Exception("Unimplemented",u"Context enTag callable has no MML NodeValue argument adapter");}
    const auto* enTagFunction=std::get_if<FunctionRef>(&enTagValue);
    if(!enTagFunction||!*enTagFunction||!(*enTagFunction)->call)type_error(u"this.enTag is not a function");
    const FunctionRef enTag=*enTagFunction;
    const auto self=shared_from_this();
    const auto receiver=std::static_pointer_cast<NativeObject>(self);
    const std::array<std::any,2> args{std::any(node),std::any(tagNode)};
    const std::any result=enTag->call(FunctionReceiver{receiver},args);
    if(const auto* value=std::any_cast<mml::NodeValue>(&result))return *value;
    type_error(u"enTag did not return an MML node value");
}

mml::NodeValue AbstractTags::makeTag(){
    (void)source_call(*this,u"makeId");
    auto& ctx=context(runtime_);
    const OptionsValue labelValue=ctx.get(OptionsValue(identity_),options::Key(u"label"));
    if(options::truthy(labelValue)){
        const auto labels=field(u"labels");
        const std::u16string labelKey=js_text(ctx,labelValue);
        const OptionsValue tagValue=current_info_field(*this,u"tag");
        const OptionsValue tagId=current_info_field(*this,u"tagId");
        const auto label=make_label(runtime_,tagValue,tagId);
        ctx.set(labels,options::Key(labelKey),label);
        ctx.set(OptionsValue(identity_),options::Key(u"label"),OptionsValue(u""));
    }
    const OptionsValue format=current_info_field(*this,u"tagFormat");
    OptionsValue parts;
    if(const auto* array=std::get_if<options::ObjectRef>(&format.data);
       array&&ctx.kind(*array)==options::ObjectKind::array){
        parts=format;
    }else{
        const auto* text=std::get_if<std::u16string>(&format.data);
        if(!text)type_error(u"format.match is not a function");
        regexp_host::RegExp matcher(u"^(\\(|\\[|\\{)(.*)(\\}|\\]|\\))$");
        const auto match=matcher.exec(*text);
        if(match){
            const auto fullMatch=ctx.array();
            for(std::size_t i=0;i<4;++i){
                const auto capture=match->at(i);
                const auto digits=std::to_string(i);
                ctx.set(OptionsValue(fullMatch),options::Key(std::u16string(digits.begin(),digits.end())),
                        capture?OptionsValue(*capture):OptionsValue(options::Undefined{}));
            }
            const OptionsValue slice=ctx.get(OptionsValue(fullMatch),options::Key(u"slice"));
            const auto* sliceFunction=std::get_if<options::ObjectRef>(&slice.data);
            if(!sliceFunction||ctx.kind(*sliceFunction)!=options::ObjectKind::function)
                type_error(u"match.slice is not a function");
            const std::array<OptionsValue,1> sliceArguments{OptionsValue(1)};
            parts=ctx.call(*sliceFunction,OptionsValue(fullMatch),sliceArguments);
        }else{
            const auto result=ctx.array(1);
            ctx.set(OptionsValue(result),options::Key(u"0"),format);
            parts=result;
        }
    }
    const auto callback=ctx.function(u"Tags.formatPart",[&ctx](OptionsContext&,OptionsValue,std::span<const OptionsValue> args){
        const OptionsValue part=args.empty()?OptionsValue(options::Undefined{}):args.front();
        if(!options::truthy(part))return OptionsValue(std::u16string{});
        return OptionsValue(std::u16string(u"\\text{")+js_text(ctx,part)+u"}");
    });
    ctx.define(callback,options::Key(u"length"),options::Descriptor::data(1,false,false,true));
    const OptionsValue map=ctx.get(parts,options::Key(u"map"));
    const auto* mapFunction=std::get_if<options::ObjectRef>(&map.data);
    if(!mapFunction||ctx.kind(*mapFunction)!=options::ObjectKind::function)
        type_error(u"tag.map is not a function");
    const std::array<OptionsValue,1> mapArguments{OptionsValue(callback)};
    const OptionsValue mapped=ctx.call(*mapFunction,parts,mapArguments);
    const OptionsValue join=ctx.get(mapped,options::Key(u"join"));
    const auto* joinFunction=std::get_if<options::ObjectRef>(&join.data);
    if(!joinFunction||ctx.kind(*joinFunction)!=options::ObjectKind::function)
        type_error(u"tag.join is not a function");
    const std::array<OptionsValue,1> joinArguments{OptionsValue(std::u16string{})};
    const std::u16string source=js_text(ctx,ctx.call(*joinFunction,mapped,joinArguments));
    auto& configuration=require_configuration(*this);
    const TexParserRef parser=TexParser::make(std::move(source),PropertyList{},configuration);
    const mml::NodeValue mml=parser->mml();
    auto& factory=require_node_factory(configuration);
    const auto tagId=current_info_field(*this,u"tagId");
    const OptionsValue rowAlign=context(runtime_).get(configuration.options,options::Key(u"tagAlign"));
    const std::u16string align=js_text(context(runtime_),rowAlign);
    const PropertyList definition{{"id",property_host::debugText(js_text(context(runtime_),tagId))},
                                  {"rowalign",property_host::debugText(align)}};
    return create_node(factory,u"mtd",child_array({mml}),definition);
}

void AbstractTags::makeId(){
    auto& ctx=context(runtime_);
    const auto target=field(u"currentTag");
    const OptionsValue useLabel=ctx.get(require_configuration(*this).options,options::Key(u"useLabelIds"));
    OptionsValue id;
    if(options::truthy(useLabel)){
        const OptionsValue labelValue=ctx.get(OptionsValue(identity_),options::Key(u"label"));
        id=options::truthy(labelValue)?labelValue:current_info_field(*this,u"tag");
    }else id=current_info_field(*this,u"tag");
    const auto* text=std::get_if<std::u16string>(&id.data);
    if(!text)type_error(u"id.replace is not a function");
    const std::array<OptionsValue,1> arguments{id};
    const OptionsValue formatted=source_call(*this,u"formatId",arguments);
    ctx.set(target,options::Key(u"tagId"),formatted);
}

void NoTags::autoTag(){}
mml::NodeValue NoTags::getTag(OptionsValue){
    if(!options::truthy(current_info_field(*this,u"tag")))return mml::NodeValue(PropertyNull{});
    return AbstractTags::getTag(false);
}
mml::NodeValue AllTags::finalize(mml::NodeValue node,Property environment){
    if(!property_host::truthy(property_host::get(environment,"display")))return node;
    auto& ctx=context(runtime());
    const auto history=field(u"history");
    const auto callback=ctx.function(u"Tags.history.find",[](OptionsContext& inner,OptionsValue,std::span<const OptionsValue> args){
        const OptionsValue item=args.empty()?OptionsValue(options::Undefined{}):args.front();
        return OptionsValue(options::truthy(inner.get(item,options::Key(u"taggable"))));
    });
    ctx.define(callback,options::Key(u"length"),options::Descriptor::data(1,false,false,true));
    const std::array<OptionsValue,1> findArguments{OptionsValue(callback)};
    const auto found=call_method(ctx,history,u"find",findArguments);
    if(options::truthy(found))return node;
    const std::array<OptionsValue,1> getTagArguments{OptionsValue(true)};
    const mml::NodeValue tagNode=source_call_node(*this,u"getTag",getTagArguments);
    const auto callable=ctx.get(OptionsValue(sourceIdentity()),options::Key(u"enTag"));
    FunctionValue enTagValue;
    try{enTagValue=runtime().functions().functionValue(callable);}
    catch(...){throw options::Exception("Unimplemented",u"Context enTag callable has no MML NodeValue argument adapter");}
    const auto* enTagFunction=std::get_if<FunctionRef>(&enTagValue);
    if(!enTagFunction||!*enTagFunction||!(*enTagFunction)->call)type_error(u"this.enTag is not a function");
    const FunctionRef enTag=*enTagFunction;
    const auto receiver=std::static_pointer_cast<NativeObject>(shared_from_this());
    const std::array<std::any,2> args{std::any(node),std::any(tagNode)};
    const std::any result=enTag->call(FunctionReceiver{receiver},args);
    if(const auto* value=std::any_cast<mml::NodeValue>(&result))return *value;
    type_error(u"enTag did not return an MML node value");
}

namespace {
TagsRef instantiate_tags(ParserRuntime& runtime,const TagsClassRef& descriptor){
    if(!descriptor||!descriptor->allocate)
        throw options::Exception("TypeError",u"Tags constructor has no allocation binding");
    TagsRef tags=descriptor->allocate();
    if(!tags)throw options::Exception("TypeError",u"Tags allocator returned no actual instance");
    (void)runtime.tagIdentities.instance(tags,descriptor->constructor);
    if(descriptor->initialize)descriptor->initialize(*tags);
    return tags;
}
TagsClassRef parent_tags_class(ParserRuntime& runtime,options::ObjectRef constructor){
    auto descriptor=runtime.tagClassIdentities.resolve(constructor);
    if(!descriptor)throw options::Exception("TypeError",u"Tags parent constructor is unavailable");
    return descriptor;
}
}

TagsClassRef TagsFactory::registerSubclass(ParserRuntime& runtime,std::u16string className,
        std::function<TagsRef()> allocate,std::optional<options::ObjectRef> parentConstructor,
        std::function<void(Tags&)> initializeDerived){
    if(!allocate)throw options::Exception("TypeError",u"Tags subclass requires its actual allocator");
    auto info=std::make_shared<TagsClassInfo>();
    const std::weak_ptr<TagsClassInfo> weakInfo(info);
    const auto constructor=runtime.context().nativeConstructor(std::move(className),
        [&runtime,weakInfo](options::Context&,std::span<const options::Value>)->options::ObjectRef{
            const auto descriptor=weakInfo.lock();
            if(!descriptor)throw options::Exception("TypeError",u"Tags class metadata is no longer live");
            return runtime.tagIdentities.instance(instantiate_tags(runtime,descriptor),descriptor->constructor);
        });
    info->constructor=constructor;
    info->allocate=std::move(allocate);
    TagsClassRef parentInfo;
    if(parentConstructor){
        parentInfo=parent_tags_class(runtime,*parentConstructor);
        auto& cx=runtime.context();
        const auto prototype=cx.get(constructor,options::Key(u"prototype"));
        const auto parentPrototype=cx.get(*parentConstructor,options::Key(u"prototype"));
        const auto* actual=std::get_if<options::ObjectRef>(&prototype.data);
        const auto* parent=std::get_if<options::ObjectRef>(&parentPrototype.data);
        if(!actual||!parent)throw options::Exception("TypeError",u"Tags subclass requires actual constructor prototypes");
        cx.setPrototype(constructor,*parentConstructor);
        cx.setPrototype(*actual,*parent);
        cx.define(*actual,options::Key(u"constructor"),
                  options::Descriptor::data(options::Value(constructor),true,true,true));
    }
    info->initialize=[parentInfo=std::move(parentInfo),initializeDerived=std::move(initializeDerived)](Tags& tags){
        if(parentInfo&&parentInfo->initialize)parentInfo->initialize(tags);
        else if(auto* abstract=dynamic_cast<AbstractTags*>(&tags))abstract->initializeAbstractTags();
        if(initializeDerived)initializeDerived(tags);
    };
    runtime.tagClassIdentities.bind(info,constructor);
    return info;
}
TagsClassRef TagsFactory::registerSourceSubclass(ParserRuntime& runtime,std::u16string className,
        options::ObjectRef parentConstructor,std::function<void(Tags&)> initializeDerived,
        std::function<void(options::ObjectRef)> installPrototypeMethods){
    const TagsClassRef parent=parent_tags_class(runtime,parentConstructor);
    if(!parent->allocate)throw options::Exception("TypeError",u"Tags parent has no native allocator");
    auto info=std::make_shared<TagsClassInfo>();
    const std::weak_ptr<TagsClassInfo> weakInfo(info);
    const auto constructor=runtime.context().nativeConstructor(std::move(className),
        [&runtime,weakInfo](options::Context&,std::span<const options::Value>)->options::ObjectRef{
            const auto descriptor=weakInfo.lock();
            if(!descriptor)throw options::Exception("TypeError",u"Tags class metadata is no longer live");
            return runtime.tagIdentities.instance(instantiate_tags(runtime,descriptor),descriptor->constructor);
        });
    info->constructor=constructor;
    info->allocate=parent->allocate;
    info->initialize=[parent,initializeDerived=std::move(initializeDerived)](Tags& tags){
        if(parent->initialize)parent->initialize(tags);
        if(initializeDerived)initializeDerived(tags);
    };
    auto& cx=runtime.context();
    const auto prototypeValue=cx.get(constructor,options::Key(u"prototype"));
    const auto* prototype=std::get_if<options::ObjectRef>(&prototypeValue.data);
    const auto parentPrototypeValue=cx.get(parentConstructor,options::Key(u"prototype"));
    const auto* parentPrototype=std::get_if<options::ObjectRef>(&parentPrototypeValue.data);
    if(!prototype||!parentPrototype)
        throw options::Exception("TypeError",u"Tags subclass requires actual constructor prototypes");
    cx.setPrototype(constructor,parentConstructor);
    cx.setPrototype(*prototype,*parentPrototype);
    cx.define(*prototype,options::Key(u"constructor"),
              options::Descriptor::data(options::Value(constructor),true,true,true));
    if(installPrototypeMethods)installPrototypeMethods(*prototype);
    runtime.tagClassIdentities.bind(info,constructor);
    return info;
}
TagsFactory::TagsFactory(ParserRuntime& runtime):runtime_(runtime){}
void TagsFactory::initializeModule(TagsClassRef noneClass,TagsClassRef allClass){
    if(!noneClass||!allClass)throw options::Exception("TypeError",u"TagsFactory requires the loaded none/all class descriptors");
    auto& ctx=context(runtime_);
    defaultTags_=u"none";
    const auto options=ctx.object();
    ctx.set(OptionsValue(options),options::Key(u"tags"),defaultTags_);
    ctx.set(OptionsValue(options),options::Key(u"tagSide"),u"right");
    ctx.set(OptionsValue(options),options::Key(u"tagIndent"),u"0.8em");
    ctx.set(OptionsValue(options),options::Key(u"useLabelIds"),true);
    ctx.set(OptionsValue(options),options::Key(u"ignoreDuplicateLabels"),false);
    ctx.set(OptionsValue(options),options::Key(u"tagAlign"),u"baseline");
    OPTIONS=options;
    this->add(u"none",std::move(noneClass));
    this->add(u"all",std::move(allClass));
}
void TagsFactory::add(std::u16string name,TagsClassRef constructor){tagsMapping_.set(std::move(name),std::move(constructor));}
void TagsFactory::addTags(TagsConfigValue tags){
    auto& ctx=context(runtime_);
    const auto keys=ctx.enumerableStringKeys(tags);
    for(const auto& key:keys){
        const auto value=ctx.get(tags,options::Key(key));
        const auto* object=std::get_if<options::ObjectRef>(&value.data);
        if(!object)throw options::Exception("TypeError",u"Tags class must be a constructor object");
        auto descriptor=runtime_.tagClassIdentities.resolve(*object);
        this->add(key,std::move(descriptor));
    }
}
TagsRef TagsFactory::create(std::u16string_view name){
    auto constructor=tagsMapping_.get(name);
    if(!constructor||!*constructor)constructor=tagsMapping_.get(defaultTags_);
    if(!constructor||!*constructor)throw options::Exception("Error",u"Unknown tags class");
    const auto descriptor=*constructor;
    return instantiate_tags(runtime_,descriptor);
}
void TagsFactory::setDefault(std::u16string name){defaultTags_=std::move(name);}
TagsRef TagsFactory::getDefault(){return create(defaultTags_);}

}
