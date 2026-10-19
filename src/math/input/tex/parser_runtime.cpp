// Loaded-module lifetime and class identity metadata for the real TeX parser graph.
module folia.math.input.tex.parser;

import folia.math.util.options;
import :column_parser;
import :tex_parser;
import :text_parser;
import :parse_util;
import :parse_options;
import :tex_jax;
import :base_items;
import :stack_item_factory_bindings;
import folia.math.input.tex.token_map_context_bindings;
import folia.math.input.tex.handler_context_bindings;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key runtimeKey(std::u16string_view name) { return Key(std::u16string(name)); }

Value arg(std::span<const Value> arguments, std::size_t i) {
    return i < arguments.size() ? arguments[i] : Value(Undefined{});
}

void setFunctionLength(Context& context, ObjectRef function, double length, bool namedConstructor=false) {
    context.define(function, runtimeKey(u"length"), Descriptor::data(Value(length), false, false, true));
    if (!namedConstructor)
        context.define(function, runtimeKey(u"name"), Descriptor::data(Value(u""), false, false, true));
}

void requireRuntimeActive(const std::weak_ptr<std::atomic_bool>& lifetime){
    const auto active=lifetime.lock();
    if(!active||!active->load(std::memory_order_acquire))
        throw Exception("TypeError",u"Parser runtime has been released");
}

class RuntimeRegExpContext final:public RegExpContext{
    regexp_host::RegExpBindings& bindings_;
    options::Value identity_;
    std::weak_ptr<std::atomic_bool> executionLifetime_;
public:
    RuntimeRegExpContext(regexp_host::RegExpBindings& bindings,options::Value identity,
                         std::weak_ptr<std::atomic_bool> executionLifetime)
        :bindings_(bindings),identity_(std::move(identity)),
         executionLifetime_(std::move(executionLifetime)){}
    bool test(const Property& value)override{
        requireRuntimeActive(executionLifetime_);
        const Property primitive=property_host::primitive(value,false);
        // Keep the actual Context-bound RegExp identity as the source of
        // lastIndex. This is the same binding path exposed to Source calls.
        return bindings_.exec(identity_,property_host::string(primitive)).has_value();
    }
    const options::Value& identity()const{return identity_;}
};

ParseMethod parseMethodFromValue(ParserRuntime& runtime,options::Value value){
    if(const auto* object=std::get_if<options::ObjectRef>(&value.data)){
        if(runtime.context().kind(*object)==options::ObjectKind::function){
            if(const auto function=runtime.functions().find(*object))return *function;
        }
    }
    return runtime.properties.from(std::move(value));
}

options::Value parseMethodToValue(ParserRuntime& runtime,const ParseMethod& method){
    if(const auto* function=std::get_if<FunctionRef>(&method)){
        if(const auto object=runtime.functions().find(*function))return options::Value(*object);
        throw options::Exception("Unimplemented",u"ParseMethod has no canonical Context function identity");
    }
    return runtime.properties.to(std::get<Property>(method));
}

TokenMapContextHooks tokenMapHooks(ParserRuntime& runtime){
    TokenMapContextHooks hooks;
    hooks.constructMap=[&runtime](TokenMapClass type,options::Context& context,
                                  std::span<const options::Value> arguments)->TokenMapRef{
        if(&context!=&runtime.context())throw options::Exception("TypeError",u"TokenMap constructor belongs to another Context");
        const auto property=[&](std::size_t index){return runtime.properties.from(arg(arguments,index));};
        const auto parser=[&](std::size_t index){return parseMethodFromValue(runtime,arg(arguments,index));};
        switch(type){
        case TokenMapClass::AbstractTokenMap:
            return makeAbstractTokenMap(runtime.mapHandler,property(0),parser(1));
        case TokenMapClass::RegExpMap:
            return TokenMapHost::create<RegExpMap>(runtime.mapHandler,property(0),parser(1),runtime.makeRegExpContext(arg(arguments,2)));
        case TokenMapClass::AbstractParseMap:
            return TokenMapHost::create<AbstractParseMap>(runtime.mapHandler,property(0),parser(1));
        case TokenMapClass::CharacterMap:
            return TokenMapHost::create<CharacterMap>(runtime.mapHandler,property(0),parser(1),property(2));
        case TokenMapClass::DelimiterMap:
            return TokenMapHost::create<DelimiterMap>(runtime.mapHandler,property(0),parser(1),property(2));
        case TokenMapClass::MacroMap:
            return TokenMapHost::create<MacroMap>(runtime.mapHandler,property(0),property(1),property(2));
        case TokenMapClass::CommandMap:
            return TokenMapHost::create<CommandMap>(runtime.mapHandler,property(0),property(1),property(2));
        case TokenMapClass::EnvironmentMap:
            return TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,property(0),parser(1),property(2),property(3));
        }
        throw options::Exception("TypeError",u"Unknown TokenMap constructor class");
    };
    hooks.constructToken=[&runtime](options::Context& context,std::span<const options::Value> arguments){
        if(&context!=&runtime.context())throw options::Exception("TypeError",u"Token constructor belongs to another Context");
        return std::make_shared<Token>(runtime.properties.from(arg(arguments,0)),
            runtime.properties.from(arg(arguments,1)),runtime.properties.from(arg(arguments,2)));
    };
    hooks.constructMacro=[&runtime](options::Context& context,std::span<const options::Value> arguments){
        if(&context!=&runtime.context())throw options::Exception("TypeError",u"Macro constructor belongs to another Context");
        return std::make_shared<Macro>(runtime.properties.from(arg(arguments,0)),
            parseMethodFromValue(runtime,arg(arguments,1)),runtime.properties.from(arg(arguments,2)));
    };
    hooks.parseMethodFromValue=[&runtime](options::Context& context,options::Value value){
        if(&context!=&runtime.context())throw options::Exception("TypeError",u"ParseMethod belongs to another Context");
        return parseMethodFromValue(runtime,std::move(value));
    };
    hooks.parseMethodToValue=[&runtime](options::Context& context,const ParseMethod& method){
        if(&context!=&runtime.context())throw options::Exception("TypeError",u"ParseMethod belongs to another Context");
        return parseMethodToValue(runtime,method);
    };
    hooks.regexpFromValue=[&runtime](options::Context& context,options::Value value){
        if(&context!=&runtime.context())throw options::Exception("TypeError",u"RegExp belongs to another Context");
        return runtime.makeRegExpContext(std::move(value));
    };
    hooks.regexpToValue=[&runtime](options::Context& context,const RegExpRef& regexp){
        if(&context!=&runtime.context())throw options::Exception("TypeError",u"RegExp belongs to another Context");
        return runtime.regexpContextValue(regexp);
    };
    hooks.parseInputFromValue=[&runtime](options::Context& context,options::Value value){
        if(&context!=&runtime.context())throw options::Exception("TypeError",u"ParseInput belongs to another Context");
        const auto input=context.readIterable(std::move(value),2);
        const auto parserValue=context.get(input,runtimeKey(u"0"));
        const auto* parserObject=std::get_if<options::ObjectRef>(&parserValue.data);
        if(!parserObject)throw options::Exception("TypeError",u"ParseInput[0] must be a TexParser");
        const auto parser=runtime.parserIdentities.resolve(*parserObject);
        const auto token=context.get(input,runtimeKey(u"1"));
        return ParseInput{*parser,runtime.properties.from(token)};
    };
    hooks.parseResultToValue=[&runtime](options::Context& context,ParseResult result)->options::Value{
        if(&context!=&runtime.context())throw options::Exception("TypeError",u"ParseResult belongs to another Context");
        return std::visit([&runtime](const auto& value)->options::Value{
            using T=std::decay_t<decltype(value)>;
            if constexpr(std::same_as<T,Property>)return runtime.properties.to(value);
            else if constexpr(std::same_as<T,StackItemRef>){
                const auto identity=runtime.stackItemIdentities.find(value);
                if(!identity)throw options::Exception("TypeError",u"ParseResult StackItem has no canonical Context identity");
                return options::Value(*identity);
            }else return options::Value(runtime.bindParseSymbol(value));
        },result);
    };
    return hooks;
}

std::u16string requireText(const Value& value, std::u16string_view parameter) {
    if (const auto* text = std::get_if<std::u16string>(&value.data)) return *text;
    throw Exception("TypeError", std::u16string(parameter) + u" must be a string");
}

ObjectRef configurationObject(ParserRuntime& runtime, ConfigurationRef value) {
    const auto object = value ? runtime.configurationIdentities.find(value) : std::optional<ObjectRef>{};
    if (!object) throw Exception("TypeError", u"Configuration has no bound Context identity");
    return *object;
}

StackContextHooks stackContextHooks(ParserRuntime& runtime){
    StackContextHooks hooks;
    hooks.construct=[](StackFactoryReference factory,Property env,Property inner){
        return std::make_shared<Stack>(std::move(factory),std::move(env),std::move(inner));
    };
    hooks.factory=[](const StackRef& stack){return stack->sourceFactory();};
    hooks.setFactory=[](const StackRef& stack,StackFactoryReference factory){stack->setSourceFactory(std::move(factory));};
    hooks.environment=[](const StackRef& stack){return stack->env();};
    hooks.setEnvironment=[](const StackRef& stack,Property env){stack->setEnv(std::move(env));};
    hooks.global=[](const StackRef& stack){return stack->global;};
    hooks.setGlobal=[](const StackRef& stack,Property value){stack->global=std::move(value);};
    hooks.factoryToValue=[&runtime](const StackFactoryReference& ref)->Value{
        if(const auto* pointer=std::get_if<StackItemFactoryService*>(&ref);pointer&&*pointer){
            const auto identity=runtime.stackItemFactoryIdentities.find(static_cast<StackItemFactory*>(*pointer));
            if(!identity)throw Exception("TypeError",u"Stack factory has no canonical Context identity");
            return Value(*identity);
        }
        if(const auto* raw=std::get_if<Value>(&ref))return *raw;
        if(std::holds_alternative<PropertyNull>(ref))return Value(Null{});
        return Value(Undefined{});
    };
    hooks.factoryFromValue=[&runtime](const Value& value)->StackFactoryReference{
        if(std::holds_alternative<Undefined>(value.data))return StackFactoryReference(value);
        if(std::holds_alternative<Null>(value.data))return StackFactoryReference(value);
        if(const auto* object=std::get_if<ObjectRef>(&value.data)){
            const auto& payload=std::as_const(runtime.context()).payload(*object);
            if(const auto* bound=std::any_cast<NativeBindingPayload<StackItemFactory*>>(&payload))
                if(runtime.stackItemFactoryIdentities.find(bound->identity))return static_cast<StackItemFactoryService*>(bound->identity);
        }
        return StackFactoryReference(value);
    };
    hooks.propertyToValue=[&runtime](const Property& p){return runtime.properties.to(p);};
    hooks.propertyFromValue=[&runtime](const Value& v){return runtime.properties.from(v);};
    hooks.array=[](const StackRef& stack){return stack->stackArray();};
    hooks.replaceArray=[](const StackRef& stack,StackArrayAccessRef array){stack->replaceStackArray(std::move(array));};
    hooks.arrayFromValue=[&runtime](const Value& value){
        auto toValue=[&runtime](const StackArrayStorageValue& entry)->Value{
            if(const auto* raw=std::get_if<Value>(&entry))return *raw;
            const auto& typed=std::get<StackEntry>(entry);
            return std::visit([&runtime](const auto& item)->Value{
                using T=std::decay_t<decltype(item)>;
                if constexpr(std::same_as<T,Property>)return runtime.properties.to(item);
                else if constexpr(std::same_as<T,options::Value>)return item;
                else if constexpr(std::same_as<T,StackItemRef>){auto id=runtime.stackItemIdentities.find(item);if(!id)throw Exception("TypeError",u"Stack item has no canonical Context identity");return Value(*id);}
                else {auto id=runtime.mmlNodeIdentities.find(item.nodeOrNull());if(!id)throw Exception("TypeError",u"MML node has no canonical Context identity");return Value(*id);}
            },typed);
        };
        auto fromValue=[&runtime](const Value& v)->StackEntry{
            if(const auto* object=std::get_if<ObjectRef>(&v.data)){
                const auto& payload=std::as_const(runtime.context()).payload(*object);
                if(const auto* bound=std::any_cast<NativeBindingPayload<StackItemRef>>(&payload))
                    if(runtime.stackItemIdentities.find(bound->identity))return StackEntry(bound->identity);
                if(const auto* bound=std::any_cast<NativeBindingPayload<mml::MmlNode*>>(&payload))
                    if(runtime.mmlNodeIdentities.find(bound->identity))return StackEntry(mml::NodeValue(bound->identity));
            }
            return StackEntry(v);
        };
        return StackArrayAccessRef(std::make_shared<ContextArrayStackAccess>(runtime.context(),value,toValue,fromValue,
            [&runtime](const Property& p){return runtime.properties.to(p);},runtime.executionLifetime()));
    };
    hooks.entryFromValue=[&runtime](const Value& value)->StackEntry{
        if(const auto* object=std::get_if<ObjectRef>(&value.data)){
            const auto& payload=std::as_const(runtime.context()).payload(*object);
            if(const auto* bound=std::any_cast<NativeBindingPayload<StackItemRef>>(&payload))
                if(runtime.stackItemIdentities.find(bound->identity))return bound->identity;
            if(const auto* bound=std::any_cast<NativeBindingPayload<mml::MmlNode*>>(&payload))
                if(runtime.mmlNodeIdentities.find(bound->identity))return mml::NodeValue(bound->identity);
        }
        return StackEntry(value);
    };
    hooks.entryToValue=[&runtime](const StackEntry& entry)->Value{
        return std::visit([&runtime](const auto& item)->Value{
            using T=std::decay_t<decltype(item)>;
            if constexpr(std::same_as<T,Property>)return runtime.properties.to(item);
            else if constexpr(std::same_as<T,options::Value>)return item;
            else if constexpr(std::same_as<T,StackItemRef>){auto id=runtime.stackItemIdentities.find(item);if(!id)throw Exception("TypeError",u"Stack item has no canonical Context identity");return Value(*id);}
            else {auto id=runtime.mmlNodeIdentities.find(item.nodeOrNull());if(!id)throw Exception("TypeError",u"MML node has no canonical Context identity");return Value(*id);}
        },entry);
    };
    hooks.stringToValue=[](std::string text){return Value(property_host::text(text));};
    return hooks;
}
}

ParserRuntime::ParserRuntime(Context& context, OptionFunctionBindings& functions,
        regexp_host::RegExpBindings& regexp, UnicodeNormalization& unicode,
        core::InputJaxModule& inputJaxModule, FindTeXModule& findTeXModule,
        util::RuntimeModules* sharedModules)
    : context_(context), functions_(functions), regexp_(regexp), unicode_(unicode),
      inputJaxModule_(inputJaxModule), findTeXModule_(findTeXModule),
      ownedRuntimeModules_(context),
      sharedRuntimeModulesOwner_(sharedModules?std::optional<util::RuntimeModules>(*sharedModules):std::nullopt),
      runtimeModules_(sharedRuntimeModulesOwner_?&*sharedRuntimeModulesOwner_:&ownedRuntimeModules_),
      mmlNodeContextModuleOwner_(runtimeModules_->getOrCreate<mml::MmlNodeContextModule>(u"core/MmlTree/MmlNode.js",
          [&]{return std::make_shared<mml::MmlNodeContextModule>(context);})),
      mapsOwner_(runtimeModules_->getOrCreate<MapContextBindings>(u"ECMAScript/Map",
          [&]{return std::make_shared<MapContextBindings>(context);})),
      mmlContextBindingsOwner_(runtimeModules_->getOrCreate<mml::MmlContextBindings>(
          u"core/MmlTree/ContextBindings",[&]{return mml::MmlContextBindings::create(
              context,*runtimeModules_,mmlNodeContextModuleOwner_,mapsOwner_);})),
      properties(context,functions),
      functionLists(context,functions,executionLifetime(),queue_),
      maps(*mapsOwner_),
      factoryContextBaseModule(runtimeModules_->getOrCreate<tree::FactoryContextBaseModule>(
          u"core/Tree/Factory",[&]{return std::make_shared<tree::FactoryContextBaseModule>(*runtimeModules_);})),
      attributeBindings(mmlNodeContextModuleOwner_->attributes()),
      tokenMaps(context,functions,properties,maps,tokenMapHooks(*this),executionLifetime()),
      handlerBindings(context,properties,maps,tokenMaps,functionLists,functions,mapHandler,executionLifetime()),
      configurationIdentities(context), parserConfigurationIdentities(context),
      parseOptionsIdentities(context), nodeFactoryIdentities(context), parserIdentities(context), tagIdentities(context),
      tagClassIdentities(context), texIdentities(context), keyValueIdentities(context),
      labelIdentities(context), tagInfoIdentities(context), nodeArrayIdentities(mmlContextBindingsOwner_->arrays()),
      globalDefinitionScopeIdentities(context),
      itemClassIdentities(context), stackItemFactoryIdentities(context), stackItemIdentities(context), stackPropertyIdentities(context), mmlNodeIdentities(mmlContextBindingsOwner_->nodes()),
      treeNodeBindings(mmlContextBindingsOwner_->treeNodes()),
      treeNodeClasses(mmlNodeContextModuleOwner_->treeClasses()),
      stackIdentities(context), stackBindings(context,stackIdentities,stackContextHooks(*this),executionLifetime()),
      mmlFactoryIdentities(mmlContextBindingsOwner_->factories()),
      mmlFactoryContextMethods(mmlContextBindingsOwner_->factoryMethods()),
      mmlFactoryContextFields(mmlContextBindingsOwner_->factoryFields()),
      mmlNodeContextFields(mmlContextBindingsOwner_->nodeFields()),
      mmlNodeContextMethods(mmlContextBindingsOwner_->nodeMethods()),
      columnStateIdentities(context), columnParserIdentities(context), tagsFactory(*this) {
    try{
        const auto prototype=context_.intrinsics().objectPrototype;
        bool snapshotComplete=true;
        for(const auto& key:context_.ownKeys(Value(prototype))){
            const auto descriptor=context_.descriptor(Value(prototype),key);
            if(!descriptor){snapshotComplete=false;break;}
            objectPrototypeRootSnapshot_.emplace_back(key,*descriptor);
        }
        objectPrototypeParentSnapshot_=context_.prototype(prototype);
        objectPrototypeRootSnapshotValid_=snapshotComplete;
        if(!snapshotComplete)objectPrototypeRootSnapshot_.clear();
    }catch(...){objectPrototypeRootSnapshot_.clear();}
    if (&functions.context() != &context)
        throw Exception("TypeError", u"ParserRuntime callable bindings belong to a different Context");
    if (&regexp.context() != &context)
        throw Exception("TypeError", u"ParserRuntime RegExp bindings belong to a different Context");
    const std::weak_ptr<std::atomic_bool> runtimeLease=executionLifetime();
    functionLists.addNativeArgumentEncoder([this,runtimeLease](const std::any& argument)->std::optional<options::Value>{
        const auto active=runtimeLease.lock();
        if(!active||!active->load(std::memory_order_acquire))
            throw Exception("TypeError",u"Parser runtime has been released");
        if(const auto* configuration=std::any_cast<ParserConfigurationRef>(&argument)){
            if(!*configuration)return options::Value(options::Null{});
            const auto identity=parserConfigurationIdentities.find(*configuration);
            if(!identity)throw Exception("TypeError",u"ParserConfiguration argument has no canonical Context identity");
            return options::Value(*identity);
        }
        if(const auto* jax=std::any_cast<TeX*>(&argument)){
            if(!*jax)return options::Value(options::Null{});
            const auto identity=texIdentities.findPointee(*jax);
            if(!identity)throw Exception("TypeError",u"TeX argument has no canonical Context identity");
            return options::Value(*identity);
        }
        if(const auto* jax=std::any_cast<TeXRef>(&argument)){
            if(!*jax)return options::Value(options::Null{});
            const auto identity=texIdentities.find(*jax);
            if(!identity)throw Exception("TypeError",u"TeX reference has no canonical Context identity");
            return options::Value(*identity);
        }
        return std::nullopt;
    });
}

options::ObjectRef ParserRuntime::bindMmlFactory(std::shared_ptr<mml::MmlFactory> factory){
    return mmlContextBindingsOwner_->bindFactory(std::move(factory));
}

ParserRuntime::~ParserRuntime(){
    executionLifetime_->store(false,std::memory_order_release);
}

std::shared_ptr<mml::MmlFactory> ParserRuntime::resolveMmlFactory(options::ObjectRef object)const{
    return mmlContextBindingsOwner_->resolveFactory(object);
}
options::ObjectRef ParserRuntime::bindMmlChildren(mml::Children children){
    return mmlContextBindingsOwner_->bindChildren(std::move(children));
}

std::shared_ptr<RegExpContext> ParserRuntime::makeRegExpContext(options::Value identity){
    (void)regexp_.resolve(identity);
    return std::make_shared<RuntimeRegExpContext>(regexp_,std::move(identity),executionLifetime());
}

options::Value ParserRuntime::regexpContextValue(const std::shared_ptr<RegExpContext>& regexp)const{
    const auto* contextRegexp=dynamic_cast<const RuntimeRegExpContext*>(regexp.get());
    if(!contextRegexp)throw options::Exception("TypeError",u"RegExpMap has no canonical Context RegExp identity");
    const auto lease=executionLifetime_.get();
    if(!lease||!lease->load(std::memory_order_acquire))
        throw options::Exception("TypeError",u"Parser runtime has been released");
    (void)context_.kind(std::get<options::ObjectRef>(contextRegexp->identity().data));
    return contextRegexp->identity();
}

std::shared_ptr<StackItemFactory> ParserRuntime::resolveStackItemFactory(ObjectRef object)const{
    auto* identity=stackItemFactoryIdentities.resolve(object);
    const auto* binding=std::any_cast<NativeBindingPayload<StackItemFactory*>>(&std::as_const(context_).payload(object));
    if(!binding||binding->identity!=identity||!binding->lifetime)
        throw options::Exception("TypeError",u"Context object is not bound to an actual StackItemFactory owner");
    return std::shared_ptr<StackItemFactory>(binding->lifetime,identity);
}
ObjectRef ParserRuntime::bindStack(const StackRef& owner){return stackBindings.bind(owner);}
StackRef ParserRuntime::resolveStack(ObjectRef identity)const{return stackBindings.resolve(identity);}

mml::MmlNode& ParserRuntime::resolveMmlNode(options::ObjectRef object)const{
    return mmlContextBindingsOwner_->resolveNode(object);
}

std::size_t ParserRuntime::retireTagInfoOwnersOutsidePrivateRendererRoots(){
    using namespace options;
    // Tags.ts keeps the current TagInfo and may keep nested TagInfo entries in
    // its per-equation history/stack arrays. Its labels copy string values and
    // do not retain TagInfo objects. Read only the raw data descriptors: this
    // must never execute a Source getter or callback during request cleanup.
    std::vector<ObjectRef> retained;
    const auto retainIfTagInfo=[&](const Value& value){
        const auto* object=std::get_if<ObjectRef>(&value.data);
        if(object&&tagInfoIdentities.ownsHandle(*object))retained.push_back(*object);
    };
    const auto rawOwnData=[&](ObjectRef owner,const Key& key)->std::optional<Value>{
        if(!context_.hasOwn(Value(owner),key))return std::nullopt;
        const auto descriptor=context_.descriptor(Value(owner),key);
        if(!descriptor||descriptor->accessor)return std::nullopt;
        return descriptor->value;
    };
    const auto retainArray=[&](const Value& value)->bool{
        const auto* array=std::get_if<ObjectRef>(&value.data);
        if(!array||context_.kind(*array)!=ObjectKind::array)return false;
        const auto length=context_.arrayLength(*array);
        for(std::uint32_t index=0;index<length;++index){
            const auto digits=std::to_string(index);
            const Key indexKey(std::u16string(digits.begin(),digits.end()));
            std::vector<ObjectRef> visited;
            std::optional<ObjectRef> cursor=*array;
            while(cursor){
                if(std::find(visited.begin(),visited.end(),*cursor)!=visited.end())return false;
                visited.push_back(*cursor);
                if(context_.hasOwn(Value(*cursor),indexKey)){
                    const auto descriptor=context_.descriptor(Value(*cursor),indexKey);
                    if(!descriptor||descriptor->accessor)return false;
                    retainIfTagInfo(descriptor->value);
                    break;
                }
                cursor=context_.prototype(*cursor);
            }
        }
        return true;
    };
    bool rootsReadable=true;
    try{
        tagIdentities.visitCanonicalHandles([&](const TagsRef& owner,ObjectRef identity){
            if(!rootsReadable)return;
            if(!owner||owner->sourceIdentity()!=identity){rootsReadable=false;return;}
            const auto current=rawOwnData(identity,Key(u"currentTag"));
            const auto history=rawOwnData(identity,Key(u"history"));
            const auto stack=rawOwnData(identity,Key(u"stack"));
            if(!current||!history||!stack){rootsReadable=false;return;}
            retainIfTagInfo(*current);
            if(!retainArray(*history)||!retainArray(*stack))rootsReadable=false;
        });
    }catch(const Exception&){rootsReadable=false;}
    if(!rootsReadable)return 0;

    std::vector<TagInfoRef> stale;
    tagInfoIdentities.visitCanonicalHandles([&](const TagInfoRef& owner,ObjectRef identity){
        if(std::find(retained.begin(),retained.end(),identity)==retained.end())stale.push_back(owner);
    });
    for(const auto& owner:stale)
        if(!tagInfoIdentities.retire(owner))
            throw options::Exception("TypeError",u"TagInfo owner changed during private Renderer retirement");
    return stale.size();
}

std::size_t ParserRuntime::retireLabelOwnersOutsidePrivateRendererRoots(){
    using namespace options;
    // Tags.ts owns the live labels and allLabels dictionaries. The former
    // contains labels for the current equation; finishEquation copies them into
    // the latter. A Label is durable only while it is reachable from one of
    // those raw data-property graphs. Do not call Context.get/enumeration here:
    // accessors and callbacks are not part of this ownership check.
    std::vector<ObjectRef> retained;
    const auto retainIfLabel=[&](const Value& value){
        const auto* object=std::get_if<ObjectRef>(&value.data);
        if(object&&labelIdentities.ownsHandle(*object))retained.push_back(*object);
    };
    const auto rawOwnData=[&](ObjectRef owner,const Key& key)->std::optional<Value>{
        if(!context_.hasOwn(Value(owner),key))return std::nullopt;
        const auto descriptor=context_.descriptor(Value(owner),key);
        if(!descriptor||descriptor->accessor)return std::nullopt;
        return descriptor->value;
    };
    bool rootsReadable=objectPrototypeRootSnapshotValid_;
    try{
        if(rootsReadable){
            const auto prototype=context_.intrinsics().objectPrototype;
            if(context_.prototype(prototype)!=objectPrototypeParentSnapshot_)
                rootsReadable=false;
            const auto keys=context_.ownKeys(Value(prototype));
            if(keys.size()!=objectPrototypeRootSnapshot_.size())rootsReadable=false;
            for(std::size_t i=0;rootsReadable&&i<keys.size();++i){
                if(!(keys[i]==objectPrototypeRootSnapshot_[i].first)){rootsReadable=false;break;}
                const auto descriptor=context_.descriptor(Value(prototype),keys[i]);
                const auto& expected=objectPrototypeRootSnapshot_[i].second;
                if(!descriptor||descriptor->value!=expected.value||
                   descriptor->writable!=expected.writable||descriptor->enumerable!=expected.enumerable||
                   descriptor->configurable!=expected.configurable||descriptor->accessor!=expected.accessor||
                   descriptor->getter!=expected.getter||descriptor->setter!=expected.setter)
                    rootsReadable=false;
            }
        }
        tagIdentities.visitCanonicalHandles([&](const TagsRef& owner,ObjectRef identity){
            if(!rootsReadable)return;
            if(!owner||owner->sourceIdentity()!=identity){rootsReadable=false;return;}
            for(const Key field:{Key(u"labels"),Key(u"allLabels")}){
                const auto mapValue=rawOwnData(identity,field);
                if(!mapValue){rootsReadable=false;return;}
                const auto* map=std::get_if<ObjectRef>(&mapValue->data);
                if(!map){rootsReadable=false;return;}
                std::vector<ObjectRef> visited;
                std::optional<ObjectRef> cursor=*map;
                while(cursor){
                    if(std::find(visited.begin(),visited.end(),*cursor)!=visited.end()){
                        rootsReadable=false;return;
                    }
                    visited.push_back(*cursor);
                    // An Object.create(Label) map or a Label used directly as
                    // the labels map is itself a live path to that Label.
                    retainIfLabel(Value(*cursor));
                    for(const auto& key:context_.ownKeys(Value(*cursor))){
                        const auto descriptor=context_.descriptor(Value(*cursor),key);
                        if(!descriptor||descriptor->accessor){rootsReadable=false;return;}
                        retainIfLabel(descriptor->value);
                    }
                    const auto parent=context_.prototype(*cursor);
                    if(!parent||*parent==context_.intrinsics().objectPrototype)break;
                    cursor=parent;
                }
            }
        });
    }catch(...){rootsReadable=false;}
    if(!rootsReadable)return 0;

    std::vector<LabelRef> stale;
    labelIdentities.visitCanonicalHandles([&](const LabelRef& owner,ObjectRef identity){
        if(std::find(retained.begin(),retained.end(),identity)==retained.end())stale.push_back(owner);
    });
    for(const auto& owner:stale)
        if(!labelIdentities.retire(owner))
            throw options::Exception("TypeError",u"Label owner changed during private Renderer retirement");
    return stale.size();
}

options::SymbolRef ParserRuntime::bindParseSymbol(ParseSymbolRef symbol) {
    if (!symbol) throw options::Exception("TypeError",u"Native ParseSymbol reference is null");
    for (const auto& [native, bound] : parseSymbols_) {
        if (native == symbol) {
            (void)context_.symbolString(bound);
            return bound;
        }
    }
    const auto bound = context_.symbol(symbol->description);
    parseSymbols_.emplace_back(std::move(symbol), bound);
    return bound;
}

options::ObjectRef ParserRuntime::bindSubHandlers(const std::shared_ptr<SubHandlers>& owner,ObjectRef identity){
    if(!owner)throw options::Exception("TypeError",u"SubHandlers owner is null");
    (void)context_.kind(identity);
    const auto* payload=std::any_cast<std::shared_ptr<SubHandlers>>(&std::as_const(context_).payload(identity));
    if(!payload||payload->get()!=owner.get())throw options::Exception("TypeError",u"SubHandlers identity does not retain the supplied owner");
    const auto [it,inserted]=subHandlersIdentities_.emplace(owner.get(),identity);
    if(!inserted&&it->second!=identity)throw options::Exception("TypeError",u"SubHandlers owner already has another canonical identity");
    return handlerBindings.bind(owner,identity);
}

std::optional<options::ObjectRef> ParserRuntime::findSubHandlers(const SubHandlers* owner)const{
    const auto found=subHandlersIdentities_.find(const_cast<SubHandlers*>(owner));
    if(found==subHandlersIdentities_.end())return std::nullopt;
    return found->second;
}

std::shared_ptr<SubHandlers> ParserRuntime::resolveSubHandlers(ObjectRef identity)const{
    const auto* payload=std::any_cast<std::shared_ptr<SubHandlers>>(&std::as_const(context_).payload(identity));
    if(!payload||!*payload)throw options::Exception("TypeError",u"Context object is not a SubHandlers instance");
    const auto found=findSubHandlers(payload->get());
    if(!found||*found!=identity)throw options::Exception("TypeError",u"Context object is not the canonical SubHandlers identity");
    return *payload;
}

void ParserRuntime::initializeParserRuntime() {
    if (moduleInitialized) throw Exception("TypeError", u"ParserRuntime module metadata already initialized");
    Context& context = context_;

    // Native allocations and ordinary emitted-function calls are distinct.
    // Calling a constructor on an arbitrary receiver requires its Source body;
    // it must never silently allocate a different instance.
    const ObjectRef configurationConstructor = context.nativeConstructor(u"Configuration",
        [this](Context&, std::span<const Value> arguments) -> ObjectRef {
            const ConfigurationRef value = Configuration::construct(*this, arguments);
            return configurationObject(*this, value);
        });
    setFunctionLength(context, configurationConstructor, 13, true);
    configurationClass = Value(configurationConstructor);
    const ObjectRef configurationPrototype = std::get<ObjectRef>(context.get(
        configurationClass, runtimeKey(u"prototype")).data);
    const ObjectRef initGetter = context.function(u"get", [](Context& cx, Value receiver,
            std::span<const Value>) {
        const Value tuple = cx.get(receiver, runtimeKey(u"initMethod"));
        if (!truthy(tuple)) return Value(Null{});
        return cx.get(tuple, runtimeKey(u"0"));
    });
    const ObjectRef configGetter = context.function(u"get", [](Context& cx, Value receiver,
            std::span<const Value>) {
        const Value tuple = cx.get(receiver, runtimeKey(u"configMethod"));
        if (!truthy(tuple)) return Value(Null{});
        return cx.get(tuple, runtimeKey(u"0"));
    });
    context.define(configurationPrototype, runtimeKey(u"init"), Descriptor::access(initGetter, std::nullopt, false, true));
    context.define(configurationPrototype, runtimeKey(u"config"), Descriptor::access(configGetter, std::nullopt, false, true));

    const ObjectRef makeProcessor = context.function(u"makeProcessor", [this](Context&, Value,
            std::span<const Value> arguments) {
        return Configuration::makeProcessor(*this, arg(arguments, 0), arg(arguments, 1));
    });
    setFunctionLength(context, makeProcessor, 2);
    context.define(configurationConstructor, runtimeKey(u"makeProcessor"),
                   Descriptor::data(Value(makeProcessor), true, true, true));
    const ObjectRef createConfiguration = context.function(u"_create", [this](Context&, Value receiver,
            std::span<const Value> arguments) {
        const ConfigurationRef value = Configuration::_create(*this, receiver, arg(arguments, 0), arg(arguments, 1));
        return Value(configurationObject(*this, value));
    });
    setFunctionLength(context, createConfiguration, 2);
    context.define(configurationConstructor, runtimeKey(u"_create"),
                   Descriptor::data(Value(createConfiguration), true, true, true));

    const ObjectRef create = context.function(u"create", [this](Context& cx, Value,
            std::span<const Value> arguments) {
        const Value method = cx.get(configurationClass, runtimeKey(u"_create"));
        const auto* function = std::get_if<ObjectRef>(&method.data);
        if (!function || cx.kind(*function) != ObjectKind::function)
            throw Exception("TypeError", u"Configuration._create is not a function");
        const Value nameValue = arg(arguments, 0);
        const std::array<Value, 2> callArguments{nameValue, arg(arguments, 1)};
        const Value result = cx.call(*function, configurationClass, callArguments);
        const auto setMethod = cx.get(configurationHandlerObject, runtimeKey(u"set"));
        const auto* setFunction = std::get_if<ObjectRef>(&setMethod.data);
        if (!setFunction || cx.kind(*setFunction) != ObjectKind::function)
            throw Exception("TypeError", u"ConfigurationHandler.set is not a function");
        const std::array<Value, 2> setArguments{nameValue, result};
        (void)cx.call(*setFunction, configurationHandlerObject, setArguments);
        return result;
    });
    setFunctionLength(context, create, 1);
    const ObjectRef local = context.function(u"local", [this](Context& cx, Value,
            std::span<const Value> arguments) {
        const Value method = cx.get(configurationClass, runtimeKey(u"_create"));
        const auto* function = std::get_if<ObjectRef>(&method.data);
        if (!function || cx.kind(*function) != ObjectKind::function)
            throw Exception("TypeError", u"Configuration._create is not a function");
        const std::array<Value, 2> callArguments{Value(u""), arg(arguments, 0)};
        return cx.call(*function, configurationClass, callArguments);
    });
    setFunctionLength(context, local, 0);
    context.define(configurationConstructor, runtimeKey(u"create"), Descriptor::data(Value(create), true, true, true));
    context.define(configurationConstructor, runtimeKey(u"local"), Descriptor::data(Value(local), true, true, true));
    configurationHandler.initializeModule(*this);
    stackClass=Value(stackBindings.constructor());

    const ObjectRef parserConstructor = context.nativeConstructor(u"ParserConfiguration",
        [this](Context&, std::span<const Value> arguments) -> ObjectRef {
            auto parser = std::make_shared<ParserConfiguration>(*this);
            const ObjectRef object = parserConfigurationIdentities.instance(parser,
                std::get<ObjectRef>(parserConfigurationClass.data));
            parser->initializeParserConfiguration(arg(arguments, 0), arg(arguments, 1));
            return object;
        });
    setFunctionLength(context, parserConstructor, 1, true);
    parserConfigurationClass = Value(parserConstructor);
    const ObjectRef parserPrototype = std::get<ObjectRef>(context.get(
        parserConfigurationClass, runtimeKey(u"prototype")).data);
    const ObjectRef parserInit = context.function(
        u"init", [this](Context&, Value receiver, std::span<const Value>) {
            const auto* object = std::get_if<ObjectRef>(&receiver.data);
            if (!object) throw Exception("TypeError", u"ParserConfiguration.init receiver is invalid");
            parserConfigurationIdentities.resolve(*object)->init();
            return Value(Undefined{});
        });
    setFunctionLength(context, parserInit, 0);
    context.define(parserPrototype, runtimeKey(u"init"), Descriptor::data(Value(parserInit), true, true, true));
    const ObjectRef parserConfig = context.function(
        u"config", [this](Context&, Value receiver, std::span<const Value> arguments) {
            const auto* object = std::get_if<ObjectRef>(&receiver.data);
            if (!object) throw Exception("TypeError", u"ParserConfiguration.config receiver is invalid");
            const Value jaxValue = arg(arguments, 0);
            const auto* jaxObject = std::get_if<ObjectRef>(&jaxValue.data);
            if (!jaxObject) throw Exception("TypeError", u"ParserConfiguration.config requires a TeX instance");
            const ParserConfigurationRef parser = parserConfigurationIdentities.resolve(*object);
            const TeXRef jax = texIdentities.resolve(*jaxObject);
            parser->config(*jax);
            return Value(Undefined{});
        });
    setFunctionLength(context, parserConfig, 1);
    context.define(parserPrototype, runtimeKey(u"config"), Descriptor::data(Value(parserConfig), true, true, true));
    const ObjectRef parserAddPackage = context.function(
        u"addPackage", [this](Context&, Value receiver, std::span<const Value> arguments) {
            const auto* object = std::get_if<ObjectRef>(&receiver.data);
            if (!object) throw Exception("TypeError", u"ParserConfiguration.addPackage receiver is invalid");
            parserConfigurationIdentities.resolve(*object)->addPackage(arg(arguments, 0));
            return Value(Undefined{});
        });
    setFunctionLength(context, parserAddPackage, 1);
    context.define(parserPrototype, runtimeKey(u"addPackage"), Descriptor::data(Value(parserAddPackage), true, true, true));
    const ObjectRef parserAdd = context.function(
        u"add", [this](Context& cx, Value receiver, std::span<const Value> arguments) {
            const auto* object = std::get_if<ObjectRef>(&receiver.data);
            if (!object) throw Exception("TypeError", u"ParserConfiguration.add receiver is invalid");
            const std::u16string name = requireText(arg(arguments, 0), u"name");
            const Value jaxValue = arg(arguments, 1);
            const auto* jaxObject = std::get_if<ObjectRef>(&jaxValue.data);
            if (!jaxObject) throw Exception("TypeError", u"ParserConfiguration.add requires a TeX instance");
            const ParserConfigurationRef parser = parserConfigurationIdentities.resolve(*object);
            const TeXRef jax = texIdentities.resolve(*jaxObject);
            Value supplied = arg(arguments, 2);
            if (std::holds_alternative<Undefined>(supplied.data)) supplied = Value(cx.object());
            parser->add(name, *jax, std::move(supplied));
            return Value(Undefined{});
        });
    setFunctionLength(context, parserAdd, 2);
    context.define(parserPrototype, runtimeKey(u"add"), Descriptor::data(Value(parserAdd), true, true, true));

    const ObjectRef labelConstructor = context.nativeConstructor(u"Label",
        [this](Context& cx, std::span<const Value> arguments) -> ObjectRef {
            const auto object = cx.instance(std::get<ObjectRef>(labelClass.data));
            auto value = std::make_shared<Label>(cx, object);
            labelIdentities.bind(value, object);
            Label::initialize(cx, object, arguments);
            return object;
        }, [](Context& cx, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* object = std::get_if<ObjectRef>(&receiver.data);
            if (!object) throw Exception("Unimplemented", u"Label call requires an object receiver");
            Label::initialize(cx, *object, arguments);
            return Value(Undefined{});
        });
    setFunctionLength(context, labelConstructor, 2, true);
    labelClass = Value(labelConstructor);

    const ObjectRef tagInfoConstructor = context.nativeConstructor(u"TagInfo",
        [this](Context& cx, std::span<const Value> arguments) -> ObjectRef {
            const auto object = cx.instance(std::get<ObjectRef>(tagInfoClass.data));
            auto value = std::make_shared<TagInfo>(cx, object);
            tagInfoIdentities.bind(value, object);
            TagInfo::initialize(cx, object, arguments);
            return object;
        }, [](Context& cx, Value receiver, std::span<const Value> arguments) -> Value {
            const auto* object = std::get_if<ObjectRef>(&receiver.data);
            if (!object) throw Exception("Unimplemented", u"TagInfo call requires an object receiver");
            TagInfo::initialize(cx, *object, arguments);
            return Value(Undefined{});
        });
    setFunctionLength(context, tagInfoConstructor, 8, true);
    tagInfoClass = Value(tagInfoConstructor);

    const ObjectRef columnParserConstructor = context.nativeConstructor(u"ColumnParser",
        [this](Context&, std::span<const Value>) -> ObjectRef {
            auto owner = std::make_shared<ColumnParser>(*this);
            const ObjectRef constructor = std::get<ObjectRef>(columnParserClass.data);
            const ObjectRef object = columnParserIdentities.instance(owner.get(), constructor, owner);
            owner->initializeColumnParser(owner);
            return object;
        });
    setFunctionLength(context, columnParserConstructor, 0, true);
    columnParserClass = Value(columnParserConstructor);
    ColumnParser::initializeModule(*this);

    const TagsClassRef abstractTags = TagsFactory::registerSubclass(*this,u"AbstractTags",
        [this]()->TagsRef{return std::make_shared<AbstractTags>(*this);});
    const TagsClassRef noTags = TagsFactory::registerSubclass(*this,u"NoTags",
        [this]()->TagsRef{return std::make_shared<NoTags>(*this);},abstractTags->constructor);
    const TagsClassRef allTags = TagsFactory::registerSubclass(*this,u"AllTags",
        [this]()->TagsRef{return std::make_shared<AllTags>(*this);},abstractTags->constructor);
    abstractTagsClass = Value(abstractTags->constructor);
    noTagsClass = Value(noTags->constructor);
    allTagsClass = Value(allTags->constructor);
    AbstractTags::initializeModule(*this);
    tagsFactory.initializeModule(noTags, allTags);
    StackItemFactoryBindings::initializeModule(*this);
    BaseItems::initializeModule(*this);
    ParseOptions::initializeModule(*this);
    TexParser::initializeModule(*this);
    ParseUtil::initializeModule(*this);
    TextParser::initializeModule(*this);
    TeX::initializeModule(*this);
    missingClassMetadata.clear();
    if (std::holds_alternative<Undefined>(texClass.data)) missingClassMetadata.push_back(u"TeX");
    if (std::holds_alternative<Undefined>(parseOptionsClass.data)) missingClassMetadata.push_back(u"ParseOptions");
    if (std::holds_alternative<Undefined>(texParserClass.data)) missingClassMetadata.push_back(u"TexParser");
    if (std::holds_alternative<Undefined>(columnParserClass.data)) missingClassMetadata.push_back(u"ColumnParser");
    if (std::holds_alternative<Undefined>(keyValueClass.data)) missingClassMetadata.push_back(u"KeyValueDef");
    if (std::holds_alternative<Undefined>(KeyValueTypes.data)) missingClassMetadata.push_back(u"KeyValueTypes");
    if (std::holds_alternative<Undefined>(labelClass.data)) missingClassMetadata.push_back(u"Label");
    if (std::holds_alternative<Undefined>(tagInfoClass.data)) missingClassMetadata.push_back(u"TagInfo");
    moduleMetadataComplete = missingClassMetadata.empty();
    moduleInitialized = true;
}

Context& ParserRuntime::context() const { return context_; }
OptionFunctionBindings& ParserRuntime::functions() const { return functions_; }
regexp_host::RegExpBindings& ParserRuntime::regexp() const { return regexp_; }
UnicodeNormalization& ParserRuntime::unicode() const { return unicode_; }
core::InputJaxModule& ParserRuntime::inputJaxModule() const { return inputJaxModule_; }
FindTeXModule& ParserRuntime::findTeXModule() const { return findTeXModule_; }
const async::QueueRef& ParserRuntime::queue() const { return queue_; }

} // namespace folia::math::tex
