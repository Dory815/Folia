module folia.math.input.tex.token_map_context_bindings;
import folia.math.util.native_identities;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
using namespace options;
namespace {
using namespace options;

class AbstractTokenMapCarrier final:public AbstractTokenMap {
    std::function<bool(const Property&)> contains_;
    std::function<TokenLookup(const Property&)> lookup_;
public:
    explicit AbstractTokenMapCarrier(TokenMapAllocation tag):AbstractTokenMap(tag){}
    bool contains(const Property& token)override{
        if(!contains_)throw Exception("TypeError",u"AbstractTokenMap.contains is not implemented");
        return contains_(token);
    }
    TokenLookup lookup(const Property& token)override{
        if(!lookup_)throw Exception("TypeError",u"AbstractTokenMap.lookup is not implemented");
        return lookup_(token);
    }
    void sourceDispatch(std::function<bool(const Property&)> contains,
                        std::function<TokenLookup(const Property&)> lookup){
        contains_=std::move(contains);lookup_=std::move(lookup);
    }
};

Value argument(std::span<const Value> values,std::size_t index){
    return index<values.size()?values[index]:Value(Undefined{});
}
void typeError(std::u16string message){throw Exception("TypeError",std::move(message));}

class ContextTokenLookupStorage final:public OrderedMapAccess<Property,TokenLookup>{
    options::MapStorageRef backing_;
    std::function<Value(const Property&)> keyToValue_;
    std::function<Property(const Value&)> valueToKey_;
    std::function<Value(const TokenLookup&)> lookupToValue_;
    std::function<TokenLookup(const Value&)> valueToLookup_;
public:
    ContextTokenLookupStorage(options::MapStorageRef backing,
        std::function<Value(const Property&)> keyToValue,
        std::function<Property(const Value&)> valueToKey,
        std::function<Value(const TokenLookup&)> lookupToValue,
        std::function<TokenLookup(const Value&)> valueToLookup)
        :backing_(std::move(backing)),keyToValue_(std::move(keyToValue)),
         valueToKey_(std::move(valueToKey)),lookupToValue_(std::move(lookupToValue)),
         valueToLookup_(std::move(valueToLookup)){
        if(!backing_||!keyToValue_||!valueToKey_||!lookupToValue_||!valueToLookup_)
            throw Exception("TypeError",u"Context-backed TokenMap storage requires a Map and converters");
    }
    const void* identity()const noexcept override{return backing_->identity();}
    std::size_t size()const noexcept override{return backing_->size();}
    std::size_t slotCount()const noexcept override{return backing_->slotCount();}
    std::optional<Entry> entryAt(std::size_t slot)const override{
        const auto entry=backing_->entryAt(slot);if(!entry)return std::nullopt;
        return Entry{valueToKey_(entry->first),valueToLookup_(entry->second)};
    }
    std::optional<TokenLookup> get(const Property& key)const override{
        const auto contextKey=keyToValue_(key);
        if(!backing_->has(contextKey))return std::nullopt;
        const auto value=backing_->get(contextKey);
        return valueToLookup_(value);
    }
    bool has(const Property& key)const override{return backing_->has(keyToValue_(key));}
    void set(Property key,TokenLookup value)override{
        backing_->set(keyToValue_(key),lookupToValue_(value));
    }
    bool erase(const Property& key)override{return backing_->erase(keyToValue_(key));}
    void clear()noexcept override{backing_->clear();}
    const options::MapStorageRef& backing()const noexcept{return backing_;}
};

constexpr std::array<TokenMapClass,8> mapClasses{
    TokenMapClass::AbstractTokenMap,TokenMapClass::RegExpMap,
    TokenMapClass::AbstractParseMap,TokenMapClass::CharacterMap,
    TokenMapClass::DelimiterMap,TokenMapClass::MacroMap,
    TokenMapClass::CommandMap,TokenMapClass::EnvironmentMap};
constexpr std::size_t index(TokenMapClass type){return static_cast<std::size_t>(type);}

TokenMapClass classOf(const TokenMapRef& map){
    if(dynamic_cast<EnvironmentMap*>(map.get()))return TokenMapClass::EnvironmentMap;
    if(dynamic_cast<CommandMap*>(map.get()))return TokenMapClass::CommandMap;
    if(dynamic_cast<DelimiterMap*>(map.get()))return TokenMapClass::DelimiterMap;
    if(dynamic_cast<CharacterMap*>(map.get()))return TokenMapClass::CharacterMap;
    if(dynamic_cast<MacroMap*>(map.get()))return TokenMapClass::MacroMap;
    if(dynamic_cast<AbstractParseMap*>(map.get()))return TokenMapClass::AbstractParseMap;
    if(dynamic_cast<RegExpMap*>(map.get()))return TokenMapClass::RegExpMap;
    if(dynamic_cast<AbstractTokenMap*>(map.get()))return TokenMapClass::AbstractTokenMap;
    throw Exception("TypeError",u"Unsupported native TokenMap class");
}

} // namespace

TokenMapRef makeAbstractTokenMap(MapHandler& registry,Property name,ParseMethod parser){
    return TokenMapHost::create<AbstractTokenMapCarrier>(registry,std::move(name),std::move(parser));
}

struct TokenMapContextBindings::State:std::enable_shared_from_this<State>{
    Context& context;
    OptionFunctionBindings& functions;
    ContextPropertyBindings& properties;
    MapContextBindings& maps;
    TokenMapContextHooks hooks;
    std::weak_ptr<std::atomic_bool> executionLease;
    NativeIdentityBindings<TokenMapRef> mapIds;
    NativeIdentityBindings<TokenRef> tokenIds;
    NativeIdentityBindings<MacroRef> macroIds;
    std::array<ObjectRef,8> ctors{},protos{};
    ObjectRef tokenCtor{},tokenProto{},macroCtor{},macroProto{};

    struct RawOwnerEdges final:NativeOwnerTrace{
        std::weak_ptr<State> state;
        std::function<bool(const ContextEdgeVisitor&)> visit;
        RawOwnerEdges(std::weak_ptr<State> owner,std::function<bool(const ContextEdgeVisitor&)> body)
            :state(std::move(owner)),visit(std::move(body)){}
        bool visitContextEdges(const ContextEdgeVisitor& visitor)const override{
            const auto owner=state.lock();return owner&&visit&&visit(visitor);
        }
    };

    struct ParseMapEdgeTrace final:NativeOwnerTrace{
        std::weak_ptr<State> state;
        AbstractParseMap* map=nullptr;
        ParseMapEdgeTrace(std::weak_ptr<State> owner,AbstractParseMap* value)
            :state(std::move(owner)),map(value){}
        bool visitContextEdges(const ContextEdgeVisitor& visitor)const override{
            const auto owner=state.lock();
            return owner&&owner->visitParseMapEdges(map,visitor);
        }
    };

    State(Context& cx,OptionFunctionBindings& funcs,ContextPropertyBindings& props,
          MapContextBindings& maps,TokenMapContextHooks hooks,std::weak_ptr<std::atomic_bool> lease)
        :context(cx),functions(funcs),properties(props),maps(maps),hooks(std::move(hooks)),
         executionLease(std::move(lease)),mapIds(cx),tokenIds(cx),macroIds(cx){
        if(&functions.context()!=&context||&properties.context()!=&context||&maps.context()!=&context)
            typeError(u"TokenMap bindings require one shared Context");
        if(!this->hooks.constructMap||!this->hooks.constructToken||!this->hooks.constructMacro||
           !this->hooks.parseMethodFromValue||!this->hooks.parseMethodToValue||
           !this->hooks.regexpFromValue||!this->hooks.regexpToValue||
           !this->hooks.parseInputFromValue||!this->hooks.parseResultToValue)
            throw Exception("Unimplemented",u"TokenMap bindings require complete runtime factories and typed conversion hooks");
        const auto live=executionLease.lock();
        if(!live||!live->load())throw Exception("Unimplemented",u"TokenMap bindings require a live parser execution lease");
    }

    void requireExecution()const{
        const auto live=executionLease.lock();
        if(!live||!live->load())throw Exception("TypeError",u"TokenMap runtime services have expired");
    }

    Value propertyValue(const Property& value){requireExecution();return properties.to(value);}
    Property property(Value value){requireExecution();return properties.from(std::move(value));}
    Value parseMethodValue(const ParseMethod& value){requireExecution();return hooks.parseMethodToValue(context,value);}
    ParseMethod parseMethod(Value value){requireExecution();return hooks.parseMethodFromValue(context,std::move(value));}
    ParseInput parseInput(Value value){requireExecution();return hooks.parseInputFromValue(context,std::move(value));}
    Value parseResult(ParseResult value){requireExecution();return hooks.parseResultToValue(context,std::move(value));}

    bool visitPropertyEdges(const Property& propertyValue,const ContextEdgeVisitor& visitor,
                            std::unordered_set<const PropertyStorage*>& seen)const{
        if(const auto* identity=std::get_if<ContextFunctionIdentity>(&propertyValue.value)){
            visitor(Value(identity->object));return true;
        }
        if(std::holds_alternative<options::SymbolRef>(propertyValue.value)){
            visitor(Value(std::get<options::SymbolRef>(propertyValue.value)));return true;
        }
        if(const auto* function=std::get_if<FunctionRef>(&propertyValue.value)){
            return !*function;
        }
        const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&propertyValue.value);
        if(!storage||!*storage)return true;
        return visitPropertyStorageEdges(*storage,visitor,seen);
    }

    bool visitFunctionValueEdges(const FunctionValue& value,const ContextEdgeVisitor& visitor,
                                 std::unordered_set<const PropertyStorage*>& seen)const{
        if(const auto* propertyValue=std::get_if<Property>(&value))
            return visitPropertyEdges(*propertyValue,visitor,seen);
        return false; // FunctionRef captures are opaque and are never invoked.
    }

    bool visitPropertyStorageEdges(const std::shared_ptr<PropertyStorage>& storage,
            const ContextEdgeVisitor& visitor,std::unordered_set<const PropertyStorage*>& seen)const{
        if(!storage)return true;
        if(!seen.insert(storage.get()).second)return true;
        bool complete=!storage->binding;
        if(storage->arrayPresent.size()!=storage->arrayValues.size())complete=false;
        for(const auto& [key,value]:storage->entries){
            (void)key;
            complete=visitPropertyEdges(value,visitor,seen)&&complete;
        }
        for(std::size_t index=0;index<storage->arrayValues.size();++index){
            if(index<storage->arrayPresent.size()&&storage->arrayPresent[index])
                complete=visitPropertyEdges(storage->arrayValues[index],visitor,seen)&&complete;
        }
        complete=visitPropertyStorageEdges(storage->prototype,visitor,seen)&&complete;
        return complete;
    }

    bool visitParseMapEdges(AbstractParseMap* map,const ContextEdgeVisitor& visitor)const{
        if(!map)return false;
        const auto storage=map->mapStorage();
        if(!storage)return true;
        if(const auto contextStorage=std::dynamic_pointer_cast<ContextTokenLookupStorage>(storage))
            return contextStorage->backing()->visitContextEdges(visitor);
        bool complete=true;
        std::unordered_set<const PropertyStorage*> seenProperties;
        const auto rawComplete=storage->visitRawEntries([&](const Property& key,const TokenLookup& lookup){
            complete=visitPropertyEdges(key,visitor,seenProperties)&&complete;
            std::visit([&](const auto& entry){
                using Entry=std::decay_t<decltype(entry)>;
                if constexpr(std::same_as<Entry,Property>)
                    complete=visitPropertyEdges(entry,visitor,seenProperties)&&complete;
                else if constexpr(std::same_as<Entry,TokenRef>){
                    if(const auto identity=tokenIds.find(entry))visitor(Value(*identity));
                    else complete=false;
                }else if constexpr(std::same_as<Entry,MacroRef>){
                    if(const auto identity=macroIds.find(entry))visitor(Value(*identity));
                    else complete=false;
                }
            },lookup);
        });
        return rawComplete&&complete;
    }

    template<class Owner,class Read,class Write>
    void field(ObjectRef object,const std::u16string& name,const std::shared_ptr<Owner>& owner,
               Read read,Write write,NativeOwnerTraceRef edgeTrace={}){
        auto field=std::make_shared<NativeDataField>(owner,
            [self=shared_from_this(),read=std::move(read)]{self->requireExecution();return read();},
            [self=shared_from_this(),write=std::move(write)](Value value){self->requireExecution();write(std::move(value));},
            std::function<void()>{},std::move(edgeTrace));
        context.bindData(object,Key(name),std::move(field));
    }

    template<class Owner,class Getter>
    void getter(ObjectRef proto,const std::u16string& name,Getter body){
        const auto fn=context.function(u"get",std::move(body));
        context.define(proto,Key(name),Descriptor::access(fn,std::nullopt,false,true));
    }
    template<class Body>
    ObjectRef method(ObjectRef proto,std::u16string name,std::uint32_t arity,Body body){
        const auto self=shared_from_this();
        const auto function=context.function(name,[self,body=std::move(body)](Context& cx,Value receiver,std::span<const Value> args)mutable{
            self->requireExecution();return body(cx,std::move(receiver),args);
        });
        context.define(function,Key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
        context.define(proto,Key(std::move(name)),Descriptor::data(Value(function),true,false,true));
        return function;
    }
    void setConstructorPrototype(ObjectRef ctor,ObjectRef proto){
        context.define(ctor,Key(u"prototype"),Descriptor::data(Value(proto),false,false,false));
        context.define(proto,Key(u"constructor"),Descriptor::data(Value(ctor),true,false,true));
    }
    ObjectRef makeCtor(std::u16string name,std::uint32_t arity,NativeConstruct construct){
        const auto ctor=context.nativeConstructor(std::move(name),std::move(construct));
        context.define(ctor,Key(u"length"),Descriptor::data(Value(static_cast<double>(arity)),false,false,true));
        return ctor;
    }

    ObjectRef bindMap(const TokenMapRef& map){
        requireExecution();if(!map)typeError(u"Native TokenMap reference is null");
        if(auto known=mapIds.find(map))return *known;
        const auto cls=classOf(map);
        const ObjectRef object=mapIds.instance(map,ctors[index(cls)],std::static_pointer_cast<void>(map));
        const auto self=shared_from_this();
        auto own=[&](std::u16string name,auto read,auto write,NativeOwnerTraceRef edgeTrace={}){
            field(object,name,map,std::move(read),std::move(write),std::move(edgeTrace));
        };
        auto* abstract=dynamic_cast<AbstractTokenMap*>(map.get());
        own(u"_name",[self,abstract]{return self->propertyValue(abstract->sourceNameField());},
            [self,abstract](Value value){abstract->sourceNameField(self->property(std::move(value)));},
            std::make_shared<RawOwnerEdges>(self,[self,abstract](const ContextEdgeVisitor& visitor){
                std::unordered_set<const PropertyStorage*> seen;
                return self->visitPropertyEdges(abstract->sourceNameField(),visitor,seen);
            }));
        own(u"_parser",[self,abstract]{return self->parseMethodValue(abstract->sourceParserField());},
            [self,abstract](Value value){abstract->sourceParserField(self->parseMethod(std::move(value)));},
            std::make_shared<RawOwnerEdges>(self,[self,abstract](const ContextEdgeVisitor& visitor){
                std::unordered_set<const PropertyStorage*> seen;
                return self->visitFunctionValueEdges(abstract->sourceParserField(),visitor,seen);
            }));
        if(auto* carrier=dynamic_cast<AbstractTokenMapCarrier*>(map.get())){
            const std::weak_ptr<State> weak=self;
            const Value receiver(object);
            carrier->sourceDispatch(
                [weak,receiver](const Property& token){
                    const auto state=weak.lock();if(!state)throw Exception("TypeError",u"TokenMap runtime services have expired");
                    state->requireExecution();
                    const Value callback=state->context.get(receiver,Key(u"contains"));
                    const std::array args{state->propertyValue(token)};
                    return options::truthy(state->context.callMember(callback,receiver,args,u"AbstractTokenMap.contains"));
                },
                [weak,receiver](const Property& token)->TokenLookup{
                    const auto state=weak.lock();if(!state)throw Exception("TypeError",u"TokenMap runtime services have expired");
                    state->requireExecution();
                    const Value callback=state->context.get(receiver,Key(u"lookup"));
                    const std::array args{state->propertyValue(token)};
                    return state->tokenLookupFromValue(state->context.callMember(callback,receiver,args,u"AbstractTokenMap.lookup"));
                });
        }
        if(auto* regexp=dynamic_cast<RegExpMap*>(map.get())){
            own(u"_regExp",[self,regexp]{self->requireExecution();return self->hooks.regexpToValue(self->context,regexp->sourceRegExpField());},
                [self,regexp](Value value){self->requireExecution();regexp->sourceRegExpField(self->hooks.regexpFromValue(self->context,std::move(value)));});
        }
        if(auto* parseMap=dynamic_cast<AbstractParseMap*>(map.get())){
            auto project=[self,parseMap]{
                self->requireExecution();
                auto projected=std::make_shared<ProjectedMapStorage<Property,TokenLookup,TokenPropertyEqual>>(
                    parseMap->mapStorage(),
                    [self](const Value& value){return self->property(value);},
                    [self](const Property& value){return self->propertyValue(value);},
                    [self](const Value& value){return self->tokenLookupFromValue(value);},
                    [self](const TokenLookup& value){return self->tokenLookupToValue(value);},
                    [](Property key){if(const auto* number=std::get_if<double>(&key.value);number&&*number==0)key=Property(0.0);return key;});
                return std::static_pointer_cast<MapStorage>(std::move(projected));
            };
            own(u"map",[self,project,parseMap]{
                    self->requireExecution();
                    const auto storage=parseMap->mapStorage();
                    if(const auto contextStorage=std::dynamic_pointer_cast<ContextTokenLookupStorage>(storage))
                        return Value(self->maps.bind(contextStorage->backing()));
                    return Value(self->maps.bind(project()));
                },
                [self,parseMap](Value value){
                    const auto* object=std::get_if<ObjectRef>(&value.data);
                    if(!object)typeError(u"TokenMap.map must be assigned a Map object");
                    const auto replacement=self->maps.resolve(*object);
                    if(!replacement)typeError(u"TokenMap.map must be assigned a Map from this Context");
                    const std::weak_ptr<State> weak=self;
                    auto keyToValue=[weak](const Property& key){auto state=weak.lock();if(!state)throw Exception("TypeError",u"TokenMap runtime services have expired");state->requireExecution();return state->propertyValue(key);};
                    auto valueToKey=[weak](const Value& key){auto state=weak.lock();if(!state)throw Exception("TypeError",u"TokenMap runtime services have expired");state->requireExecution();return state->property(key);};
                    auto lookupToValue=[weak](const TokenLookup& lookup){auto state=weak.lock();if(!state)throw Exception("TypeError",u"TokenMap runtime services have expired");state->requireExecution();return state->tokenLookupToValue(lookup);};
                    auto valueToLookup=[weak](const Value& lookup){auto state=weak.lock();if(!state)throw Exception("TypeError",u"TokenMap runtime services have expired");state->requireExecution();return state->tokenLookupFromValue(lookup);};
                    parseMap->rebindMapStorage(std::make_shared<ContextTokenLookupStorage>(
                        replacement,std::move(keyToValue),std::move(valueToKey),
                        std::move(lookupToValue),std::move(valueToLookup)));
                },std::make_shared<ParseMapEdgeTrace>(self,parseMap));
        }
        return object;
    }

    ObjectRef bindToken(const TokenRef& token){
        requireExecution();if(!token)typeError(u"Native Token reference is null");
        if(auto known=tokenIds.find(token))return *known;
        const auto object=tokenIds.instance(token,tokenCtor,std::static_pointer_cast<void>(token));
        const auto self=shared_from_this();
        field(object,u"_token",token,[self,token]{return self->propertyValue(token->sourceTokenField());},
              [self,token](Value value){token->sourceTokenField(self->property(std::move(value)));},
              std::make_shared<RawOwnerEdges>(self,[self,token](const ContextEdgeVisitor& visitor){
                  std::unordered_set<const PropertyStorage*> seen;
                  return self->visitPropertyEdges(token->sourceTokenField(),visitor,seen);
              }));
        field(object,u"_char",token,[self,token]{return self->propertyValue(token->sourceCharField());},
              [self,token](Value value){token->sourceCharField(self->property(std::move(value)));},
              std::make_shared<RawOwnerEdges>(self,[self,token](const ContextEdgeVisitor& visitor){
                  std::unordered_set<const PropertyStorage*> seen;
                  return self->visitPropertyEdges(token->sourceCharField(),visitor,seen);
              }));
        field(object,u"_attributes",token,[self,token]{return self->propertyValue(token->sourceAttributesField());},
              [self,token](Value value){token->sourceAttributesField(self->property(std::move(value)));},
              std::make_shared<RawOwnerEdges>(self,[self,token](const ContextEdgeVisitor& visitor){
                  std::unordered_set<const PropertyStorage*> seen;
                  return self->visitPropertyEdges(token->sourceAttributesField(),visitor,seen);
              }));
        return object;
    }

    ObjectRef bindMacro(const MacroRef& macro){
        requireExecution();if(!macro)typeError(u"Native Macro reference is null");
        if(auto known=macroIds.find(macro))return *known;
        const auto object=macroIds.instance(macro,macroCtor,std::static_pointer_cast<void>(macro));
        const auto self=shared_from_this();
        field(object,u"_token",macro,[self,macro]{return self->propertyValue(macro->sourceTokenField());},
              [self,macro](Value value){macro->sourceTokenField(self->property(std::move(value)));},
              std::make_shared<RawOwnerEdges>(self,[self,macro](const ContextEdgeVisitor& visitor){
                  std::unordered_set<const PropertyStorage*> seen;
                  return self->visitPropertyEdges(macro->sourceTokenField(),visitor,seen);
              }));
        field(object,u"_func",macro,[self,macro]{return self->parseMethodValue(macro->sourceFuncField());},
              [self,macro](Value value){macro->sourceFuncField(self->parseMethod(std::move(value)));},
              std::make_shared<RawOwnerEdges>(self,[self,macro](const ContextEdgeVisitor& visitor){
                  std::unordered_set<const PropertyStorage*> seen;
                  return self->visitFunctionValueEdges(macro->sourceFuncField(),visitor,seen);
              }));
        field(object,u"_args",macro,[self,macro]{return self->propertyValue(macro->sourceArgsField());},
              [self,macro](Value value){macro->sourceArgsField(self->property(std::move(value)));},
              std::make_shared<RawOwnerEdges>(self,[self,macro](const ContextEdgeVisitor& visitor){
                  std::unordered_set<const PropertyStorage*> seen;
                  return self->visitPropertyEdges(macro->sourceArgsField(),visitor,seen);
              }));
        return object;
    }

    TokenLookup tokenLookupFromValue(Value value){
        requireExecution();
        if(const auto* object=std::get_if<ObjectRef>(&value.data)){
            try{if(auto token=tokenIds.resolve(*object);token)return token;}catch(const NativeIdentityResolutionFailure&){}
            try{if(auto macro=macroIds.resolve(*object);macro)return macro;}catch(const NativeIdentityResolutionFailure&){}
        }
        return property(std::move(value));
    }
    Value tokenLookupToValue(const TokenLookup& value){
        requireExecution();
        return std::visit([&](const auto& entry)->Value{
            using T=std::decay_t<decltype(entry)>;
            if constexpr(std::same_as<T,Property>)return propertyValue(entry);
            else if constexpr(std::same_as<T,TokenRef>)return Value(bindToken(entry));
            else return Value(bindMacro(entry));
        },value);
    }

    TokenMapRef resolveMap(ObjectRef object)const{return mapIds.resolve(object);}
    TokenRef resolveTokenOwner(ObjectRef object)const{return tokenIds.resolve(object);}
    MacroRef resolveMacroOwner(ObjectRef object)const{return macroIds.resolve(object);}

    void initialize(){
        const auto self=shared_from_this();
        for(auto cls:mapClasses){
            const auto i=index(cls);
            static const std::array<std::u16string,8> labels{u"AbstractTokenMap",u"RegExpMap",u"AbstractParseMap",u"CharacterMap",u"DelimiterMap",u"MacroMap",u"CommandMap",u"EnvironmentMap"};
            static constexpr std::array<std::uint32_t,8> arities{2,3,0,3,0,2,0,3};
            ctors[i]=makeCtor(labels[i],arities[i],[self,cls](Context& cx,std::span<const Value> args){
                self->requireExecution();
                const auto owner=self->hooks.constructMap(cls,cx,args);
                if(!owner)throw Exception("TypeError",u"TokenMap constructor factory returned null");
                if(classOf(owner)!=cls)throw Exception("TypeError",u"TokenMap constructor factory returned the wrong native class");
                return self->bindMap(owner);
            });
            protos[i]=context.object();
            if(i>0){
                std::size_t parent=i;
                switch(cls){
                case TokenMapClass::RegExpMap: parent=index(TokenMapClass::AbstractTokenMap);break;
                case TokenMapClass::AbstractParseMap: parent=index(TokenMapClass::AbstractTokenMap);break;
                case TokenMapClass::CharacterMap: parent=index(TokenMapClass::AbstractParseMap);break;
                case TokenMapClass::DelimiterMap: parent=index(TokenMapClass::CharacterMap);break;
                case TokenMapClass::MacroMap: parent=index(TokenMapClass::AbstractParseMap);break;
                case TokenMapClass::CommandMap: parent=index(TokenMapClass::MacroMap);break;
                case TokenMapClass::EnvironmentMap: parent=index(TokenMapClass::MacroMap);break;
                default: break;
                }
                context.setPrototype(protos[i],protos[parent]);
                context.setPrototype(ctors[i],ctors[parent]);
            }
            setConstructorPrototype(ctors[i],protos[i]);
        }
        tokenCtor=makeCtor(u"Token",3,[self](Context& cx,std::span<const Value> args){
            self->requireExecution();
            auto owner=self->hooks.constructToken(cx,args);if(!owner)throw Exception("TypeError",u"Token constructor factory returned null");
            return self->bindToken(owner);
        });
        tokenProto=context.object();setConstructorPrototype(tokenCtor,tokenProto);
        macroCtor=makeCtor(u"Macro",2,[self](Context& cx,std::span<const Value> args){
            self->requireExecution();
            auto owner=self->hooks.constructMacro(cx,args);if(!owner)throw Exception("TypeError",u"Macro constructor factory returned null");
            return self->bindMacro(owner);
        });
        macroProto=context.object();setConstructorPrototype(macroCtor,macroProto);

        auto accessor=[&](ObjectRef proto,std::u16string propertyName,std::u16string label,
                          NativeCall read,std::optional<NativeCall> write={}){
            const auto getterFn=context.function(label+u" "+propertyName,std::move(read));
            std::optional<ObjectRef> setterFn;
            if(write)setterFn=context.function(u"set "+propertyName,std::move(*write));
            context.define(proto,Key(std::move(propertyName)),Descriptor::access(getterFn,setterFn,false,true));
        };
        auto mapGetter=[&](std::u16string name,auto read,auto write){
            accessor(protos[index(TokenMapClass::AbstractTokenMap)],std::move(name),u"get",
                [self,read](Context&,Value receiver,std::span<const Value>)->Value{
                    self->requireExecution();auto owner=self->resolveMap(std::get<ObjectRef>(receiver.data));return read(*owner);
                },NativeCall([self,write](Context&,Value receiver,std::span<const Value> args)->Value{
                    self->requireExecution();auto owner=self->resolveMap(std::get<ObjectRef>(receiver.data));write(*owner,argument(args,0));return Undefined{};
                }));
        };
        mapGetter(u"name",[self](TokenMap& map){return self->propertyValue(map.name());},
            [self](TokenMap& map,Value value){dynamic_cast<AbstractTokenMap&>(map).sourceNameField(self->property(std::move(value)));});
        mapGetter(u"parser",[self](TokenMap& map){return self->parseMethodValue(map.parser());},
            [self](TokenMap& map,Value value){map.parser(self->parseMethod(std::move(value)));});

        auto containsBody=[self](Context&,Value receiver,std::span<const Value> args){
            return Value(self->resolveMap(std::get<ObjectRef>(receiver.data))->contains(self->property(argument(args,0))));};
        method(protos[index(TokenMapClass::AbstractTokenMap)],u"parserFor",1,[self](Context&,Value receiver,std::span<const Value> args){
            return self->parseMethodValue(self->resolveMap(std::get<ObjectRef>(receiver.data))->parserFor(self->property(argument(args,0))));});
        method(protos[index(TokenMapClass::AbstractTokenMap)],u"parse",1,[self](Context&,Value receiver,std::span<const Value> args){
            auto map=self->resolveMap(std::get<ObjectRef>(receiver.data));
            return self->parseResult(map->parse(self->parseInput(argument(args,0))));});
        method(protos[index(TokenMapClass::RegExpMap)],u"contains",1,containsBody);
        method(protos[index(TokenMapClass::RegExpMap)],u"lookup",1,[self](Context&,Value receiver,std::span<const Value> args){
            return self->tokenLookupToValue(self->resolveMap(std::get<ObjectRef>(receiver.data))->lookup(self->property(argument(args,0))));});
        method(protos[index(TokenMapClass::AbstractParseMap)],u"contains",1,containsBody);
        method(protos[index(TokenMapClass::AbstractParseMap)],u"lookup",1,[self](Context&,Value receiver,std::span<const Value> args){
            return self->tokenLookupToValue(self->resolveMap(std::get<ObjectRef>(receiver.data))->lookup(self->property(argument(args,0))));});
        method(protos[index(TokenMapClass::AbstractParseMap)],u"add",2,[self](Context&,Value receiver,std::span<const Value> args){
            auto map=std::dynamic_pointer_cast<AbstractParseMap>(self->resolveMap(std::get<ObjectRef>(receiver.data)));
            map->add(self->property(argument(args,0)),self->tokenLookupFromValue(argument(args,1)));return Value(Undefined{});});
        method(protos[index(TokenMapClass::AbstractParseMap)],u"remove",1,[self](Context&,Value receiver,std::span<const Value> args){
            auto map=std::dynamic_pointer_cast<AbstractParseMap>(self->resolveMap(std::get<ObjectRef>(receiver.data)));
            map->remove(self->property(argument(args,0)));return Value(Undefined{});});
        method(protos[index(TokenMapClass::DelimiterMap)],u"parse",1,[self](Context&,Value receiver,std::span<const Value> args){
            auto map=self->resolveMap(std::get<ObjectRef>(receiver.data));
            return self->parseResult(map->parse(self->parseInput(argument(args,0))));});
        method(protos[index(TokenMapClass::MacroMap)],u"parserFor",1,[self](Context&,Value receiver,std::span<const Value> args){
            return self->parseMethodValue(self->resolveMap(std::get<ObjectRef>(receiver.data))->parserFor(self->property(argument(args,0))));});
        method(protos[index(TokenMapClass::MacroMap)],u"parse",1,[self](Context&,Value receiver,std::span<const Value> args){
            auto map=self->resolveMap(std::get<ObjectRef>(receiver.data));
            return self->parseResult(map->parse(self->parseInput(argument(args,0))));});
        method(protos[index(TokenMapClass::CommandMap)],u"parse",1,[self](Context&,Value receiver,std::span<const Value> args){
            auto map=self->resolveMap(std::get<ObjectRef>(receiver.data));
            return self->parseResult(map->parse(self->parseInput(argument(args,0))));});
        method(protos[index(TokenMapClass::EnvironmentMap)],u"parse",1,[self](Context&,Value receiver,std::span<const Value> args){
            auto map=self->resolveMap(std::get<ObjectRef>(receiver.data));
            return self->parseResult(map->parse(self->parseInput(argument(args,0))));});

        auto tokenAccessor=[&](std::u16string key,std::u16string fieldName,auto read){
            accessor(tokenProto,std::move(key),u"get",[self,read](Context&,Value receiver,std::span<const Value>)->Value{
                const auto token=self->resolveTokenOwner(std::get<ObjectRef>(receiver.data));return self->propertyValue(read(*token));});
            (void)fieldName;
        };
        tokenAccessor(u"token",u"_token",[](Token& token){return token.token();});
        tokenAccessor(u"char",u"_char",[](Token& token){return token.character();});
        tokenAccessor(u"attributes",u"_attributes",[](Token& token){return token.attributes();});
        auto macroAccessor=[&](std::u16string key,auto read){
            accessor(macroProto,std::move(key),u"get",[self,read](Context&,Value receiver,std::span<const Value>)->Value{
                const auto macro=self->resolveMacroOwner(std::get<ObjectRef>(receiver.data));return read(*macro);});
        };
        macroAccessor(u"token",[self](Macro& macro){return self->propertyValue(macro.token());});
        macroAccessor(u"func",[self](Macro& macro){return self->parseMethodValue(macro.func());});
        macroAccessor(u"args",[self](Macro& macro){return self->propertyValue(macro.args());});
    }
};

TokenMapContextBindings::TokenMapContextBindings(Context& context,OptionFunctionBindings& functions,
    ContextPropertyBindings& properties,MapContextBindings& maps,TokenMapContextHooks hooks,
    std::weak_ptr<std::atomic_bool> executionLease){
    state_=std::make_shared<State>(context,functions,properties,maps,std::move(hooks),std::move(executionLease));
    state_->initialize();
}
TokenMapContextBindings::~TokenMapContextBindings()=default;
ObjectRef TokenMapContextBindings::constructor(TokenMapClass type)const{return state_->ctors[index(type)];}
ObjectRef TokenMapContextBindings::prototype(TokenMapClass type)const{return state_->protos[index(type)];}
ObjectRef TokenMapContextBindings::tokenConstructor()const{return state_->tokenCtor;}
ObjectRef TokenMapContextBindings::tokenPrototype()const{return state_->tokenProto;}
ObjectRef TokenMapContextBindings::macroConstructor()const{return state_->macroCtor;}
ObjectRef TokenMapContextBindings::macroPrototype()const{return state_->macroProto;}
ObjectRef TokenMapContextBindings::bind(const TokenMapRef& value){return state_->bindMap(value);}
ObjectRef TokenMapContextBindings::bind(const TokenRef& value){return state_->bindToken(value);}
ObjectRef TokenMapContextBindings::bind(const MacroRef& value){return state_->bindMacro(value);}
TokenMapRef TokenMapContextBindings::resolveTokenMap(ObjectRef object)const{return state_->resolveMap(object);}
TokenMapRef TokenMapContextBindings::resolve(ObjectRef object)const{return state_->resolveMap(object);}
TokenRef TokenMapContextBindings::resolveToken(ObjectRef object)const{return state_->resolveTokenOwner(object);}
MacroRef TokenMapContextBindings::resolveMacro(ObjectRef object)const{return state_->resolveMacroOwner(object);}
TokenLookup TokenMapContextBindings::tokenLookupFromValue(Value value){state_->requireExecution();return state_->tokenLookupFromValue(std::move(value));}
Value TokenMapContextBindings::tokenLookupToValue(const TokenLookup& value){state_->requireExecution();return state_->tokenLookupToValue(value);}
Property TokenMapContextBindings::fromValue(Value value)const{return state_->property(std::move(value));}
Value TokenMapContextBindings::toValue(const Property& value)const{return state_->propertyValue(value);}
ParseMethod TokenMapContextBindings::parseMethodFromValue(Value value)const{return state_->parseMethod(std::move(value));}
Value TokenMapContextBindings::parseMethodToValue(const ParseMethod& value)const{return state_->parseMethodValue(value);}
ParseInput TokenMapContextBindings::parseInputFromValue(Value value)const{return state_->parseInput(std::move(value));}
Value TokenMapContextBindings::parseResultToValue(ParseResult value)const{return state_->parseResult(std::move(value));}
Context& TokenMapContextBindings::context()const{return state_->context;}

} // namespace folia::math::tex
