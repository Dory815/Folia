// Source static methods shared by MathJax SVG wrapper constructors.
module folia.math.output.svg.wrapper_source_methods;
import folia.math.output.common;
import folia.math.core.dom_node;
import folia.math.util.native_identities;
import folia.math.util.property_host;
import folia.math.input.tex.parser;
import std;

namespace folia::math::output::svg {
using namespace options;
using common::CommonWrapper;
using common::LineBBox;

struct SvgWrapperSourceMethods::State : std::enable_shared_from_this<State> {
    Context& context;
    ContextPropertyBindings& properties;
    common::CommonFactoryContextBindings& factories;
    math::tex::ParserRuntime& parser;
    common::BBoxContextBindings& bboxes;
    NativeIdentityBindings<tree::FactoryChildren<CommonWrapper>> childArrays;
    std::optional<ObjectRef> commonWrapper;
    std::optional<ObjectRef> textNode;

    State(Context& cx,ContextPropertyBindings& bindings,
          common::CommonFactoryContextBindings& factoryBindings,
          common::BBoxContextBindings& bboxBindings,
          math::tex::ParserRuntime& runtime)
        :context(cx),properties(bindings),factories(factoryBindings),parser(runtime),bboxes(bboxBindings),
         childArrays(cx){
        if(&cx!=&bindings.context()||&cx!=&factoryBindings.context()||&cx!=&runtime.context())
            throw Exception("TypeError",u"SVG wrapper methods must share one Context");
        if(&cx!=&bboxBindings.context())
            throw Exception("TypeError",u"SVG wrapper methods and BBox bindings must share one Context");
    }

    using Getter=std::function<Value(State&,ObjectRef)>;
    using Setter=std::function<void(State&,ObjectRef,Value)>;
    void installGetter(ObjectRef prototype,std::u16string name,Getter body,Setter setter={}){
        const auto weak=std::weak_ptr<State>(shared_from_this());
        const auto function=context.function(u"get "+name,
            [weak,body=std::move(body)](Context&,Value receiver,
                std::span<const Value>)->Value{
                auto state=weak.lock();
                if(!state)throw Exception("Unimplemented",u"SVG wrapper source methods have been released");
                const auto* object=std::get_if<ObjectRef>(&receiver.data);
                if(!object)throw Exception("TypeError",u"SVG wrapper getter called on a non-object");
                return body(*state,*object);
            });
        context.define(function,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        std::optional<ObjectRef> setFunction;
        if(setter){
            setFunction=context.function(u"set "+name,
                [weak,setter=std::move(setter)](Context&,Value receiver,
                    std::span<const Value> args)->Value{
                    auto state=weak.lock();
                    if(!state)throw Exception("Unimplemented",u"SVG wrapper source methods have been released");
                    const auto* object=std::get_if<ObjectRef>(&receiver.data);
                    if(!object)throw Exception("TypeError",u"SVG wrapper setter called on a non-object");
                    setter(*state,*object,args.empty()?Value(Undefined{}):args.front());
                    return Undefined{};
                });
            context.define(*setFunction,Key(u"length"),
                Descriptor::data(Value(1),false,false,true));
        }
        context.define(prototype,Key(std::move(name)),
            Descriptor::access(function,setFunction,false,true));
    }

    using Method=std::function<Value(State&,CommonWrapper&,std::span<const Value>)>;
    void installMethod(ObjectRef prototype,std::u16string name,std::uint32_t arity,Method body){
        const auto weak=std::weak_ptr<State>(shared_from_this());
        const auto function=context.function(name,
            [weak,body=std::move(body)](Context&,Value receiver,
                std::span<const Value> args)->Value{
                auto state=weak.lock();
                if(!state)throw Exception("Unimplemented",u"SVG wrapper source methods have been released");
                const auto* object=std::get_if<ObjectRef>(&receiver.data);
                if(!object)throw Exception("TypeError",u"SVG wrapper method called on a non-object");
                auto& wrapper=state->factories.resolveWrapper(*object);
                if(wrapper.factory().objectFor(wrapper)!=*object)
                    throw Exception("TypeError",u"SVG wrapper receiver is not its canonical Context identity");
                return body(*state,wrapper,args);
            });
        context.define(function,Key(u"length"),Descriptor::data(
            Value(static_cast<double>(arity)),false,false,true));
        context.set(Value(prototype),Key(std::move(name)),Value(function));
    }

    static ObjectRef constructor(const SvgWrapperConstructorSet& graph,
                                 std::u16string_view name){
        const auto found=std::find_if(graph.constructors.begin(),graph.constructors.end(),
            [&](const auto& entry){return entry.first==name;});
        if(found==graph.constructors.end())
            throw Exception("TypeError",u"SVG wrapper constructor graph is missing a required class");
        return found->second;
    }

    void callSheetAddStyles(Value target,Value styles){
        const auto* object=std::get_if<ObjectRef>(&target.data);
        if(!object)throw Exception("TypeError",u"StyleJsonSheet target is not an object");
        const auto method=context.get(target,Key(u"addStyles"));
        const auto* function=std::get_if<ObjectRef>(&method.data);
        if(!function)throw Exception("TypeError",u"StyleJsonSheet.addStyles is not callable");
        const std::array<Value,1> args{std::move(styles)};
        (void)context.call(*function,std::move(target),args);
    }

    ObjectRef stringArray(std::initializer_list<std::u16string_view> values){
        const auto array=context.array(static_cast<std::uint32_t>(values.size()));
        std::uint32_t index=0;
        for(const auto value:values){
            const auto key=std::u16string(1,static_cast<char16_t>(u'0'+index++));
            context.set(Value(array),Key(key),Value(std::u16string(value)));
        }
        return array;
    }

    ObjectRef valueArray(std::initializer_list<Value> values){
        const auto array=context.array(static_cast<std::uint32_t>(values.size()));
        std::uint32_t index=0;
        for(const auto& value:values){
            const auto key=std::u16string(1,static_cast<char16_t>(u'0'+index++));
            context.set(Value(array),Key(key),value);
        }
        return array;
    }

    ObjectRef mmlIdentity(mml::MmlNode& node){
        const auto identity=parser.mmlNodeIdentities.find(&node);
        if(!identity)
            throw Exception("Unimplemented",u"MML factory product has no canonical Context identity");
        return *identity;
    }

    static std::u16string arrayIndex(std::size_t index){
        std::u16string key;
        do{
            key.insert(key.begin(),static_cast<char16_t>(u'0'+index%10));
            index/=10;
        }while(index);
        return key;
    }

    struct ChildrenView final:NativeIndexedView {
        std::weak_ptr<State> state;
        tree::FactoryChildren<CommonWrapper> children;
        std::shared_ptr<void> wrapperLifetime;
        bool lengthWritable=true;
        ChildrenView(std::shared_ptr<State> owner,tree::FactoryChildren<CommonWrapper> source,
                     std::shared_ptr<void> lifetime)
            :state(owner),children(std::move(source)),wrapperLifetime(std::move(lifetime)){}
        std::uint32_t length()const override{
            if(children->size()>std::numeric_limits<std::uint32_t>::max())
                throw Exception("RangeError",u"Wrapper child array length exceeds the Context array limit");
            return static_cast<std::uint32_t>(children->size());
        }
        Descriptor lengthDescriptor()const override{
            return Descriptor::data(Value(static_cast<double>(length())),lengthWritable,false,false);
        }
        std::optional<Descriptor> indexDescriptor(std::uint32_t index)const override{
            if(index>=length()||!children->hasOwn(index))return {};
            const auto child=children->get(index);
            if(child.kind()==tree::ChildKind::Undefined)return Descriptor::data(Value(Undefined{}));
            if(child.kind()==tree::ChildKind::Null)return Descriptor::data(Value(Null{}));
            auto* wrapper=child.nodeOrNull();
            return Descriptor::data(Value(wrapper->factory().objectFor(*wrapper)));
        }
        std::vector<std::uint32_t> ownIndexKeys()const override{
            std::vector<std::uint32_t> result;
            result.reserve(children->size());
            for(std::uint32_t index=0;index<length();++index)
                if(children->hasOwn(index))result.push_back(index);
            return result;
        }
        void defineIndex(std::uint32_t index,Descriptor descriptor)override{
            if(descriptor.accessor||!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
                throw Exception("Unimplemented",u"Wrapper child arrays do not store per-index descriptor flags");
            if(std::holds_alternative<Undefined>(descriptor.value.data)){
                children->set(index,tree::ChildValue<CommonWrapper>(Missing{}));return;
            }
            if(std::holds_alternative<Null>(descriptor.value.data)){
                children->set(index,tree::ChildValue<CommonWrapper>(PropertyNull{}));return;
            }
            const auto* identity=std::get_if<ObjectRef>(&descriptor.value.data);
            if(!identity)throw Exception("TypeError",u"Wrapper child must be a canonical wrapper, null or undefined");
            const auto owner=state.lock();
            if(!owner)throw Exception("Unimplemented",u"SVG wrapper source methods have been released");
            children->set(index,&owner->factories.resolveWrapper(*identity));
        }
        void defineLength(Descriptor descriptor)override{
            if(descriptor.accessor||descriptor.enumerable||descriptor.configurable)
                throw Exception("TypeError",u"Invalid array length descriptor");
            if(!lengthWritable&&descriptor.writable)
                throw Exception("TypeError",u"Cannot redefine non-writable property");
            const auto* number=std::get_if<double>(&descriptor.value.data);
            if(!number||!std::isfinite(*number)||*number<0||*number>4294967295.0||std::trunc(*number)!=*number)
                throw Exception("RangeError",u"Invalid array length");
            children->resize(static_cast<std::size_t>(*number));
            lengthWritable=descriptor.writable;
        }
        void deleteIndex(std::uint32_t index)override{children->erase(index);}
    };
    ObjectRef bindChildren(ObjectRef wrapperIdentity,
                           tree::FactoryChildren<CommonWrapper> children){
        if(const auto existing=childArrays.find(children))return *existing;
        const auto* binding=std::any_cast<NativeBindingPayload<CommonWrapper*>>(
            &context.payload(wrapperIdentity));
        if(!binding||!binding->lifetime)
            throw Exception("TypeError",u"Wrapper child array has no owning layout lifetime");
        const auto identity=context.nativeArray(std::make_shared<ChildrenView>(
            shared_from_this(),children,binding->lifetime));
        return childArrays.bind(children,identity,children);
    }

    ObjectRef variantTable(bool bold){
        const auto table=context.object();
        const auto add=[&](std::u16string_view from,
                           std::initializer_list<std::pair<std::u16string_view,std::u16string_view>> pairs){
            const auto row=context.object();
            for(const auto& [key,value]:pairs)
                context.set(Value(row),Key(std::u16string(key)),Value(std::u16string(value)));
            context.set(Value(table),Key(std::u16string(from)),Value(row));
        };
        if(bold){
            add(u"bold",{{u"normal",u"bold"},{u"italic",u"bold-italic"},
                {u"fraktur",u"bold-fraktur"},{u"script",u"bold-script"},
                {u"sans-serif",u"bold-sans-serif"},{u"sans-serif-italic",u"sans-serif-bold-italic"}});
            add(u"normal",{{u"bold",u"normal"},{u"bold-italic",u"italic"},
                {u"bold-fraktur",u"fraktur"},{u"bold-script",u"script"},
                {u"bold-sans-serif",u"sans-serif"},{u"sans-serif-bold-italic",u"sans-serif-italic"}});
        }else{
            add(u"italic",{{u"normal",u"italic"},{u"bold",u"bold-italic"},
                {u"sans-serif",u"sans-serif-italic"},{u"bold-sans-serif",u"sans-serif-bold-italic"}});
            add(u"normal",{{u"italic",u"normal"},{u"bold-italic",u"bold"},
                {u"sans-serif-italic",u"sans-serif"},{u"sans-serif-bold-italic",u"bold-sans-serif"}});
        }
        return table;
    }

    void install(const SvgWrapperConstructorSet& graph){
        if(commonWrapper)return;
        const auto base=constructor(graph,u"CommonWrapper");
        const auto text=constructor(graph,u"SvgTextNode");
        context.set(Value(base),Key(u"kind"),Value(u"unknown"));
        const auto prototypeValue=context.get(Value(base),Key(u"prototype"));
        const auto* prototype=std::get_if<ObjectRef>(&prototypeValue.data);
        if(!prototype)throw Exception("TypeError",u"CommonWrapper has no prototype");
        const auto styles=context.object();
        context.set(Value(base),Key(u"styles"),Value(styles));
        context.set(Value(base),Key(u"removeStyles"),Value(stringArray({
            u"fontSize",u"fontFamily",u"fontWeight",u"fontStyle",u"fontVariant",u"font"})));
        const auto skip=context.object();
        for(const auto key:{u"fontfamily",u"fontsize",u"fontweight",u"fontstyle",u"color",
                            u"background",u"class",u"href",u"style",u"xmlns"})
            context.set(Value(skip),Key(key),Value(true));
        context.set(Value(base),Key(u"skipAttributes"),Value(skip));
        context.set(Value(base),Key(u"BOLDVARIANTS"),Value(variantTable(true)));
        context.set(Value(base),Key(u"ITALICVARIANTS"),Value(variantTable(false)));
        auto addStyles=context.function(u"addStyles",
            [weak=std::weak_ptr<State>(shared_from_this())](Context&,Value receiver,
                std::span<const Value> args)->Value{
                auto owner=weak.lock();
                if(!owner)throw Exception("Unimplemented",u"SVG wrapper source methods have been released");
                if(args.empty())throw Exception("TypeError",u"StyleJsonSheet argument is missing");
                const auto styles=owner->context.get(receiver,Key(u"styles"));
                owner->callSheetAddStyles(args.front(),styles);
                return Undefined{};
            });
        context.define(addStyles,Key(u"length"),Descriptor::data(Value(2),false,false,true));
        context.set(Value(base),Key(u"addStyles"),Value(addStyles));

        installGetter(*prototype,u"jax",[](State& state,ObjectRef receiver){
            return Value(state.factories.resolveWrapper(receiver).jax().instanceIdentity());
        });
        installGetter(*prototype,u"factory",[](State& state,ObjectRef receiver){
            auto& wrapper=state.factories.resolveWrapper(receiver);
            return Value(state.factories.bind(wrapper.jax().factory));
        });
        installGetter(*prototype,u"node",[](State& state,ObjectRef receiver){
            auto& wrapper=state.factories.resolveWrapper(receiver);
            return Value(state.mmlIdentity(*wrapper.node));
        });
        installGetter(*prototype,u"childNodes",[](State& state,ObjectRef receiver){
            auto& wrapper=state.factories.resolveWrapper(receiver);
            if(!wrapper.childNodes)return Value(Undefined{});
            const auto children=*wrapper.childNodes;
            if(!children)return Value(Null{});
            return Value(state.bindChildren(receiver,children));
        },[](State& state,ObjectRef receiver,Value value){
            auto& wrapper=state.factories.resolveWrapper(receiver);
            if(std::holds_alternative<Undefined>(value.data)){
                wrapper.childNodes.reset();return;
            }
            if(std::holds_alternative<Null>(value.data)){
                wrapper.childNodes=tree::FactoryChildren<CommonWrapper>{};return;
            }
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object||state.context.kind(*object)!=ObjectKind::array)
                throw Exception("TypeError",u"Wrapper childNodes must be an array, null or undefined");
            if(const auto* binding=std::any_cast<NativeBindingPayload<
                    tree::FactoryChildren<CommonWrapper>>>(&state.context.payload(*object));
               binding&&binding->identity){
                wrapper.childNodes=state.childArrays.resolve(*object);
                return;
            }
            const auto size=state.context.toNumber(state.context.get(value,Key(u"length")));
            if(!std::isfinite(size)||size<0||size>4294967295.0||std::trunc(size)!=size)
                throw Exception("RangeError",u"Invalid array length");
            auto children=std::make_shared<tree::ChildArray<CommonWrapper>>();
            children->resize(static_cast<std::size_t>(size));
            for(std::size_t index=0;index<static_cast<std::size_t>(size);++index){
                const auto key=Key(arrayIndex(index));
                if(!state.context.hasOwn(value,key))continue;
                const auto entry=state.context.get(value,key);
                if(std::holds_alternative<Undefined>(entry.data))
                    children->set(index,tree::ChildValue<CommonWrapper>(Missing{}));
                else if(std::holds_alternative<Null>(entry.data))
                    children->set(index,tree::ChildValue<CommonWrapper>(PropertyNull{}));
                else if(const auto* child=std::get_if<ObjectRef>(&entry.data))
                    children->set(index,&state.factories.resolveWrapper(*child));
                else throw Exception("TypeError",u"Wrapper child array values must be canonical wrappers");
            }
            wrapper.childNodes=std::move(children);
        });
        const auto svgBase=constructor(graph,u"SvgWrapper");
        const auto svgBasePrototypeValue=context.get(Value(svgBase),Key(u"prototype"));
        const auto* svgBasePrototype=std::get_if<ObjectRef>(&svgBasePrototypeValue.data);
        if(!svgBasePrototype)throw Exception("TypeError",u"SvgWrapper has no prototype object");
        installMethod(*svgBasePrototype,u"fixed",2,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args)->Value{
            const double number=args.empty()?std::numeric_limits<double>::quiet_NaN():
                state.context.toNumber(args[0]);
            const double digits=args.size()<2||std::holds_alternative<Undefined>(args[1].data)
                ?1.0:state.context.toNumber(args[1]);
            const auto wrapperIdentity=wrapper.factory().objectFor(wrapper);
            const auto jax=state.context.get(Value(wrapperIdentity),Key(u"jax"));
            const auto fixed=state.context.get(jax,Key(u"fixed"));
            const auto* function=std::get_if<ObjectRef>(&fixed.data);
            if(!function||state.context.kind(*function)!=ObjectKind::function)
                throw Exception("TypeError",u"this.jax.fixed is not a function");
            const std::array<Value,2> forwarded={Value(number*1000),Value(digits)};
            return state.context.callMember(Value(*function),jax,forwarded,u"SvgWrapper.fixed");
        });
        // SvgWrapper.svg is a live two-hop Source dispatch: wrapper.svg gets
        // this.jax.svg at call time and preserves the Jax as receiver. The
        // Jax method owns the scoped typed Scene target; ordinary calls still
        // reach its actual html adaptor path.
        installMethod(*prototype,u"svg",3,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args)->Value{
            const auto wrapperIdentity=wrapper.factory().objectFor(wrapper);
            const auto jax=state.context.get(Value(wrapperIdentity),Key(u"jax"));
            const auto method=state.context.get(jax,Key(u"svg"));
            const auto* function=std::get_if<ObjectRef>(&method.data);
            if(!function||state.context.kind(*function)!=ObjectKind::function)
                throw Exception("TypeError",u"this.jax.svg is not a function");
            const auto type=args.empty()?Value(Undefined{}):args[0];
            const auto def=args.size()<2||std::holds_alternative<Undefined>(args[1].data)
                ?Value(state.context.object()):args[1];
            const auto content=args.size()<3||std::holds_alternative<Undefined>(args[2].data)
                ?Value(state.context.array()):args[2];
            const std::array<Value,3> forwarded{type,def,content};
            return state.context.callMember(Value(*function),jax,forwarded,u"SvgWrapper.svg");
        });
        installMethod(*prototype,u"text",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args)->Value{
            const auto value=args.empty()?Value(Undefined{}):args.front();
            const auto text=property_host::string(state.properties.from(value));
            return state.properties.to(nodeProperty(wrapper.jax().text(text)));
        });
        installGetter(*prototype,u"adaptor",[](State& state,ObjectRef receiver){
            auto& wrapper=state.factories.resolveWrapper(receiver);
            return state.context.get(Value(wrapper.jax().instanceIdentity()),Key(u"adaptor"));
        });
        installGetter(*prototype,u"metrics",[](State& state,ObjectRef receiver){
            auto& wrapper=state.factories.resolveWrapper(receiver);
            const auto jax=Value(wrapper.jax().instanceIdentity());
            const auto math=state.context.get(jax,Key(u"math"));
            return state.context.get(math,Key(u"metrics"));
        });
        installGetter(*prototype,u"containerWidth",[](State& state,ObjectRef receiver){
            auto& wrapper=state.factories.resolveWrapper(receiver);
            if(wrapper.parent){
                const auto parent=wrapper.factory().objectFor(*wrapper.parent);
                return state.context.get(Value(parent),Key(u"containerWidth"));
            }
            return state.context.get(Value(wrapper.jax().instanceIdentity()),Key(u"containerWidth"));
        });
        installGetter(*prototype,u"linebreaks",[](State& state,ObjectRef receiver){
            auto& wrapper=state.factories.resolveWrapper(receiver);
            return state.context.get(Value(wrapper.jax().instanceIdentity()),Key(u"linebreaks"));
        });
        installGetter(*prototype,u"linebreakOptions",[](State& state,ObjectRef receiver){
            return state.factories.resolveWrapper(receiver).linebreakOptions();
        });
        installGetter(*prototype,u"fixesPWidth",[](State& state,ObjectRef receiver){
            auto& wrapper=state.factories.resolveWrapper(receiver);
            return Value(!property_host::truthy(wrapper.node->notParent())&&!wrapper.node->isToken());
        });
        installGetter(*prototype,u"breakCount",[](State& state,ObjectRef receiver){
            return Value(static_cast<double>(state.factories.resolveWrapper(receiver).breakCount()));
        });
        installMethod(*prototype,u"core",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){
            auto* core=wrapper.core();
            return core?Value(core->factory().objectFor(*core)):Value(Null{});
        });
        installMethod(*prototype,u"coreMO",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){
            auto* core=wrapper.coreMO();
            return core?Value(core->factory().objectFor(*core)):Value(Null{});
        });
        installMethod(*prototype,u"coreRScale",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){return Value(wrapper.coreRScale());});
        installMethod(*prototype,u"getRScale",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){return Value(wrapper.getRScale());});
        installMethod(*prototype,u"getText",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){return Value(wrapper.getText());});
        installMethod(*prototype,u"getBBox",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const bool save=args.empty()||std::holds_alternative<Undefined>(args.front().data)
                ?true:property_host::truthy(state.properties.from(args.front()));
            return Value(state.bboxes.bind(wrapper.getBBox(save)));
        });
        installMethod(*prototype,u"getOuterBBox",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const bool save=args.empty()||std::holds_alternative<Undefined>(args.front().data)
                ?true:property_host::truthy(state.properties.from(args.front()));
            return Value(state.bboxes.bind(wrapper.getOuterBBox(save)));
        });
        installMethod(*prototype,u"computeBBox",2,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'w')");
            const auto* object=std::get_if<ObjectRef>(&args.front().data);
            if(!object)throw Exception("TypeError",u"BBox argument is not an object");
            auto box=state.bboxes.resolve(*object);
            const bool recompute=args.size()>1&&property_host::truthy(state.properties.from(args[1]));
            wrapper.sourceComputeBBox(*box,recompute);
            return Undefined{};
        });
        installMethod(*prototype,u"getLineBBox",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto raw=args.empty()?0.0:property_host::number(state.properties.from(args.front()));
            if(!std::isfinite(raw)||raw<0||std::trunc(raw)!=raw||raw>static_cast<double>(std::numeric_limits<std::size_t>::max()))
                throw Exception("Unimplemented",u"CommonWrapper.getLineBBox requires a non-negative integer index");
            return Value(state.bboxes.bind(wrapper.getLineBBox(static_cast<std::size_t>(raw))));
        });
        installMethod(*prototype,u"embellishedBBox",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto raw=args.empty()?0.0:property_host::number(state.properties.from(args.front()));
            if(!std::isfinite(raw)||raw<0||std::trunc(raw)!=raw||
               raw>static_cast<double>(std::numeric_limits<std::size_t>::max()))
                throw Exception("Unimplemented",u"CommonWrapper.embellishedBBox requires a non-negative integer index");
            return Value(state.bboxes.bind(wrapper.sourceEmbellishedBBox(static_cast<std::size_t>(raw))));
        });
        installMethod(*prototype,u"computeLineBBox",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto raw=args.empty()?0.0:property_host::number(state.properties.from(args.front()));
            if(!std::isfinite(raw)||raw<0||std::trunc(raw)!=raw||
               raw>static_cast<double>(std::numeric_limits<std::size_t>::max()))
                throw Exception("Unimplemented",u"CommonWrapper.computeLineBBox requires a non-negative integer index");
            return Value(state.bboxes.bind(wrapper.sourceComputeLineBBox(static_cast<std::size_t>(raw))));
        });
        installMethod(*prototype,u"breakTop",2,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            auto resolve=[&](std::size_t index)->CommonWrapper*{
                if(index>=args.size()||std::holds_alternative<Null>(args[index].data)||
                   std::holds_alternative<Undefined>(args[index].data))return nullptr;
                const auto* object=std::get_if<ObjectRef>(&args[index].data);
                if(!object)throw Exception("TypeError",u"breakTop argument is not a wrapper");
                return &state.factories.resolveWrapper(*object);
            };
            auto* result=wrapper.sourceBreakTop(resolve(0),resolve(1));
            return result?Value(result->factory().objectFor(*result)):Value(Null{});
        });
        installMethod(*prototype,u"getBreakNode",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'start')");
            const auto* object=std::get_if<ObjectRef>(&args.front().data);
            if(!object)throw Exception("TypeError",u"LineBBox argument is not an object");
            auto line=state.bboxes.resolveLine(*object);
            const auto result=wrapper.getBreakNode(*line);
            auto handle=[&](CommonWrapper* value)->Value{
                return value?Value(value->factory().objectFor(*value)):Value(Null{});
            };
            return Value(state.valueArray({handle(result[0]),handle(result[1])}));
        });
        installMethod(*prototype,u"getChildLineBBox",2,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.size()<2)throw Exception("TypeError",u"getChildLineBBox arguments are missing");
            const auto* childObject=std::get_if<ObjectRef>(&args[0].data);
            if(!childObject)throw Exception("TypeError",u"getChildLineBBox child is not a wrapper");
            auto& child=state.factories.resolveWrapper(*childObject);
            const double raw=property_host::number(state.properties.from(args[1]));
            if(!std::isfinite(raw)||raw<0||std::trunc(raw)!=raw)
                throw Exception("Unimplemented",u"getChildLineBBox requires a non-negative integer index");
            return Value(state.bboxes.bind(wrapper.sourceGetChildLineBBox(child,static_cast<std::size_t>(raw))));
        });
        installMethod(*prototype,u"sideStyleSize",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.empty())throw Exception("TypeError",u"sideStyleSize index is missing");
            const double raw=property_host::number(state.properties.from(args.front()));
            if(!std::isfinite(raw)||raw<0||std::trunc(raw)!=raw)
                throw Exception("Unimplemented",u"sideStyleSize requires a non-negative integer index");
            return Value(wrapper.sourceSideStyleSize(static_cast<std::size_t>(raw)));
        });
        auto mutateBox=[&](std::u16string name,std::uint32_t arity,
            std::function<void(CommonWrapper&,BBox&)> body){
            installMethod(*prototype,std::move(name),arity,[body=std::move(body)]
                (State& bindings,CommonWrapper& wrapper,std::span<const Value> args){
                if(args.empty())throw Exception("TypeError",u"BBox argument is missing");
                const auto* object=std::get_if<ObjectRef>(&args.front().data);
                if(!object)throw Exception("TypeError",u"BBox argument is not an object");
                auto box=bindings.bboxes.resolve(*object);body(wrapper,*box);return Undefined{};
            });
        };
        mutateBox(u"addLeftBorders",1,[](CommonWrapper& w,BBox& b){w.sourceAddLeftBorders(b);});
        mutateBox(u"addMiddleBorders",1,[](CommonWrapper& w,BBox& b){w.sourceAddMiddleBorders(b);});
        mutateBox(u"addRightBorders",1,[](CommonWrapper& w,BBox& b){w.sourceAddRightBorders(b);});
        mutateBox(u"copySkewIC",1,[](CommonWrapper& w,BBox& b){w.sourceCopySkewIC(b);});
        const Method setChildPWidths=[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const bool recompute=!args.empty()&&property_host::truthy(state.properties.from(args[0]));
            std::optional<double> width;
            if(args.size()>1&&!std::holds_alternative<Undefined>(args[1].data)&&
               !std::holds_alternative<Null>(args[1].data))
                width=property_host::number(state.properties.from(args[1]));
            const bool clear=args.size()<3||std::holds_alternative<Undefined>(args[2].data)
                ?true:property_host::truthy(state.properties.from(args[2]));
            return Value(wrapper.setChildPWidths(recompute,width,clear));
        };
        installMethod(*prototype,u"setChildPWidths",1,setChildPWidths);
        const auto mathPrototypeValue=context.get(
            Value(constructor(graph,u"SvgMath")),Key(u"prototype"));
        const auto* mathPrototype=std::get_if<ObjectRef>(&mathPrototypeValue.data);
        if(!mathPrototype)throw Exception("TypeError",u"SvgMath constructor has no prototype");
        installMethod(*mathPrototype,u"setChildPWidths",1,setChildPWidths);
        installMethod(*prototype,u"getStyles",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){wrapper.sourceGetStyles();return Undefined{};});
        installMethod(*prototype,u"getStyleData",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){wrapper.sourceGetStyleData();return Undefined{};});
        installMethod(*prototype,u"getVariant",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){wrapper.sourceGetVariant();return Undefined{};});
        installMethod(*prototype,u"getScale",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){wrapper.sourceGetScale();return Undefined{};});
        installMethod(*prototype,u"getSpace",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){wrapper.sourceGetSpace();return Undefined{};});
        installMethod(*prototype,u"getMathMLSpacing",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){wrapper.sourceGetMathMLSpacing();return Undefined{};});
        installMethod(*prototype,u"getTeXSpacing",2,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const bool isTop=!args.empty()&&property_host::truthy(state.properties.from(args[0]));
            const bool hasSpacing=args.size()>1&&property_host::truthy(state.properties.from(args[1]));
            wrapper.sourceGetTeXSpacing(isTop,hasSpacing);return Undefined{};
        });
        installMethod(*prototype,u"isTopEmbellished",0,[](State&,CommonWrapper& wrapper,
            std::span<const Value>){return Value(wrapper.sourceIsTopEmbellished());});
        installMethod(*prototype,u"explicitVariant",3,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            auto text=[&](std::size_t i){return i<args.size()?property_host::debugText(
                property_host::string(state.properties.from(args[i]))):std::string("undefined");};
            return state.properties.to(Property(wrapper.sourceExplicitVariant(text(0),text(1),text(2))));
        });
        installMethod(*prototype,u"getAlignX",3,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.size()<3)throw Exception("TypeError",u"getAlignX arguments are missing");
            const auto* object=std::get_if<ObjectRef>(&args[1].data);
            if(!object)throw Exception("TypeError",u"getAlignX BBox is not an object");
            auto box=state.bboxes.resolve(*object);
            return Value(wrapper.sourceGetAlignX(property_host::number(state.properties.from(args[0])),
                *box,property_host::debugText(property_host::string(state.properties.from(args[2])))));
        });
        installMethod(*prototype,u"getAlignY",5,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.size()<5)throw Exception("TypeError",u"getAlignY arguments are missing");
            auto n=[&](std::size_t i){return property_host::number(state.properties.from(args[i]));};
            const auto align=property_host::debugText(property_host::string(state.properties.from(args[4])));
            return Value(wrapper.sourceGetAlignY(n(0),n(1),n(2),n(3),align));
        });
        installMethod(*prototype,u"getUnbrokenHD",0,[](State& state,CommonWrapper& wrapper,
            std::span<const Value>){
            const auto dimensions=wrapper.getUnbrokenHD();
            return Value(state.valueArray({Value(dimensions[0]),Value(dimensions[1])}));
        });
        installMethod(*prototype,u"getAlignShift",0,[](State& state,CommonWrapper& wrapper,
            std::span<const Value>){
            const auto [alignment,shift]=wrapper.sourceGetAlignShift();
            return Value(state.valueArray({state.properties.to(Property(alignment)),Value(shift)}));
        });
        installMethod(*prototype,u"unicodeChars",2,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto text=args.empty()?std::u16string{}:
                property_host::string(state.properties.from(args[0]));
            const auto variant=args.size()<2||std::holds_alternative<Undefined>(args[1].data)
                ?std::string{}:property_host::debugText(property_host::string(state.properties.from(args[1])));
            const auto chars=wrapper.sourceUnicodeChars(text,variant);
            const auto result=state.context.array(static_cast<std::uint32_t>(chars->size()));
            for(std::size_t i=0;i<chars->size();++i)
                state.context.set(Value(result),Key(arrayIndex(i)),Value(static_cast<double>((*chars)[i])));
            return Value(result);
        });
        installMethod(*prototype,u"remapChars",1,[](State&,CommonWrapper&,
            std::span<const Value> args){
            if(args.empty())return Value(Undefined{});
            // Source remapChars returns its input array unchanged.
            return args.front();
        });
        installMethod(*prototype,u"mmlText",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto text=args.empty()?std::u16string{}:
                property_host::string(state.properties.from(args.front()));
            auto* result=wrapper.sourceMmlText(text);
            return Value(state.mmlIdentity(*result));
        });
        installMethod(*prototype,u"mmlNode",3,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto kind=args.empty()?std::string{}:
                property_host::debugText(property_host::string(state.properties.from(args[0])));
            PropertyList properties;
            if(args.size()>1&&!std::holds_alternative<Undefined>(args[1].data)&&
               !std::holds_alternative<Null>(args[1].data)){
                const auto value=state.properties.from(args[1]);
                auto record=PropertyList::object(value);
                if(!record)throw Exception("TypeError",u"MML node properties are not an object record");
                properties=*record;
            }
            std::vector<mml::MmlNode*> children;
            if(args.size()>2&&!std::holds_alternative<Undefined>(args[2].data)&&
               !std::holds_alternative<Null>(args[2].data)){
                const auto* array=std::get_if<ObjectRef>(&args[2].data);
                if(!array)throw Exception("TypeError",u"MML node children are not an array object");
                const auto lengthValue=state.context.get(Value(*array),Key(u"length"));
                const auto length=property_host::number(state.properties.from(lengthValue));
                if(!std::isfinite(length)||length<0||std::trunc(length)!=length||
                   length>static_cast<double>(std::numeric_limits<std::size_t>::max()))
                    throw Exception("RangeError",u"MML children length is invalid");
                children.reserve(static_cast<std::size_t>(length));
                for(std::size_t i=0;i<static_cast<std::size_t>(length);++i){
                    const auto child=state.context.get(Value(*array),Key(arrayIndex(i)));
                    if(std::holds_alternative<Undefined>(child.data)||std::holds_alternative<Null>(child.data)){
                        children.push_back(nullptr);continue;
                    }
                    const auto* childObject=std::get_if<ObjectRef>(&child.data);
                    if(!childObject)throw Exception("TypeError",u"MML child is not an object");
                    children.push_back(&state.parser.resolveMmlNode(*childObject));
                }
            }
            auto* result=wrapper.sourceMmlNode(kind,std::move(properties),std::move(children));
            return Value(state.mmlIdentity(*result));
        });
        installMethod(*prototype,u"createMo",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto text=args.empty()?std::u16string{}:
                property_host::string(state.properties.from(args.front()));
            auto& result=wrapper.sourceCreateMo(text);
            return Value(result.factory().objectFor(result));
        });
        installMethod(*prototype,u"getVariantChar",2,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.size()<2)throw Exception("TypeError",u"getVariantChar arguments are missing");
            const auto variant=property_host::debugText(property_host::string(state.properties.from(args[0])));
            const auto raw=property_host::number(state.properties.from(args[1]));
            if(!std::isfinite(raw)||raw<0||raw>0x10FFFF||std::trunc(raw)!=raw)
                throw Exception("RangeError",u"getVariantChar code point is invalid");
            const auto chars=wrapper.sourceGetVariantChar(variant,static_cast<std::uint32_t>(raw));
            return state.properties.to(Property(chars));
        });
        installMethod(*prototype,u"html",3,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.empty())throw Exception("TypeError",u"html node type is missing");
            const auto type=property_host::string(state.properties.from(args[0]));
            PropertyList definition;
            if(args.size()>1&&!std::holds_alternative<Undefined>(args[1].data)&&
               !std::holds_alternative<Null>(args[1].data)){
                const auto value=state.properties.from(args[1]);
                auto record=PropertyList::object(value);
                if(!record)throw Exception("TypeError",u"HTML node attributes are not an object record");
                definition=*record;
            }
            PropertyArray content;
            if(args.size()>2&&!std::holds_alternative<Undefined>(args[2].data)&&
               !std::holds_alternative<Null>(args[2].data)){
                const auto value=state.properties.from(args[2]);
                const auto array=PropertyArray::array(value);
                if(!array)throw Exception("TypeError",u"HTML node content is not an array");
                content=*array;
            }
            const auto node=wrapper.sourceHtml(type,std::move(definition),std::move(content));
            return state.properties.to(nodeProperty(node));
        });
        installMethod(*prototype,u"processIndent",5,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            auto stringArg=[&](std::size_t index){
                const auto value=index<args.size()?state.properties.from(args[index]):Property(Missing{});
                return property_host::debugText(property_host::string(value));
            };
            const auto align=stringArg(0),shift=stringArg(1);
            const auto defaultAlign=args.size()<3||std::holds_alternative<Undefined>(args[2].data)
                ?std::string{}:stringArg(2);
            const auto defaultShift=args.size()<4||std::holds_alternative<Undefined>(args[3].data)
                ?std::string{}:stringArg(3);
            std::optional<double> width;
            if(args.size()>4&&!std::holds_alternative<Undefined>(args[4].data))
                width=property_host::number(state.properties.from(args[4]));
            const auto [alignment,indent]=wrapper.processIndent(align,shift,defaultAlign,defaultShift,width);
            return Value(state.valueArray({state.properties.to(Property(alignment)),Value(indent)}));
        });
        installMethod(*prototype,u"wrap",2,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.empty()||std::holds_alternative<Undefined>(args.front().data))
                throw Exception("TypeError",u"Cannot read properties of undefined (reading 'kind')");
            if(std::holds_alternative<Null>(args.front().data))
                throw Exception("TypeError",u"Cannot read properties of null (reading 'kind')");
            const auto* node=std::get_if<ObjectRef>(&args.front().data);
            if(!node)throw Exception("TypeError",u"Wrapper node argument is not an MmlNode object");
            auto& source=state.parser.resolveMmlNode(*node);
            CommonWrapper* parent=&wrapper;
            if(args.size()>1&&property_host::truthy(state.properties.from(args[1]))){
                const auto* parentObject=std::get_if<ObjectRef>(&args[1].data);
                if(!parentObject)throw Exception("TypeError",u"Wrapper parent is not an object");
                parent=&state.factories.resolveWrapper(*parentObject);
            }
            auto* child=wrapper.wrapNode(source,parent);
            return Value(child->factory().objectFor(*child));
        });
        installMethod(*prototype,u"canStretch",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto direction=args.empty()?Property(Missing{}):state.properties.from(args.front());
            return Value(wrapper.canStretch(property_host::debugText(property_host::string(direction))));
        });
        installMethod(*prototype,u"getChildAlign",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value>){
            return state.properties.to(Property(wrapper.getChildAlign(0)));
        });
        installMethod(*prototype,u"getWrapWidth",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.empty()||std::holds_alternative<Undefined>(args.front().data))
                throw Exception("TypeError",u"Cannot read properties of undefined (reading 'getBBox')");
            const auto index=property_host::number(state.properties.from(args.front()));
            if(!std::isfinite(index)||index<0||std::trunc(index)!=index)
                throw Exception("TypeError",u"Cannot read properties of undefined (reading 'getBBox')");
            return Value(wrapper.getWrapWidth(static_cast<std::size_t>(index)));
        });
        installMethod(*prototype,u"percent",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto value=args.empty()?Property(Missing{}):state.properties.from(args.front());
            return state.properties.to(Property(wrapper.sourcePercent(property_host::number(value))));
        });
        installMethod(*prototype,u"em",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto value=args.empty()?Property(Missing{}):state.properties.from(args.front());
            return state.properties.to(Property(wrapper.sourceEm(property_host::number(value))));
        });
        installMethod(*prototype,u"px",2,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto value=args.empty()?Property(Missing{}):state.properties.from(args[0]);
            const double minimum=args.size()<2||std::holds_alternative<Undefined>(args[1].data)
                ?-BIGDIMEN:property_host::number(state.properties.from(args[1]));
            return state.properties.to(Property(wrapper.sourcePx(property_host::number(value),minimum)));
        });
        installMethod(*prototype,u"length2em",3,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto length=args.empty()?Property(Missing{}):state.properties.from(args[0]);
            const double size=args.size()<2||std::holds_alternative<Undefined>(args[1].data)
                ?1.0:property_host::number(state.properties.from(args[1]));
            std::optional<double> scale;
            if(args.size()>2&&!std::holds_alternative<Undefined>(args[2].data)&&
               !std::holds_alternative<Null>(args[2].data))
                scale=property_host::number(state.properties.from(args[2]));
            return Value(wrapper.sourceLength2em(length,size,scale));
        });
        installMethod(*prototype,u"breakToWidth",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            if(args.empty())throw Exception("TypeError",u"breakToWidth requires a width");
            wrapper.breakToWidth(property_host::number(state.properties.from(args.front())));
            return Value(Undefined{});
        });
        installMethod(*prototype,u"invalidateBBox",1,[](State& state,CommonWrapper& wrapper,
            std::span<const Value> args){
            const auto bubble=args.empty()||std::holds_alternative<Undefined>(args.front().data)||
                property_host::truthy(state.properties.from(args.front()));
            wrapper.invalidateBBox(bubble);
            return Value(Undefined{});
        });

        // CommonMenclose's methods live on the exact SVG constructor prototype
        // selected for this node.  Keep these callbacks in the same canonical
        // wrapper-instance bridge as the other Common methods so they resolve
        // the already adopted wrapper instead of creating a shadow object.
        const auto mencloseConstructor=constructor(graph,u"CommonMencloseMixin");
        const auto menclosePrototypeValue=context.get(Value(mencloseConstructor),Key(u"prototype"));
        const auto* menclosePrototype=std::get_if<ObjectRef>(&menclosePrototypeValue.data);
        if(!menclosePrototype)throw Exception("TypeError",u"CommonMencloseMixin has no prototype");
        auto menclose=[&](CommonWrapper& wrapper)->common::CommonMenclose&{
            auto* result=dynamic_cast<common::CommonMenclose*>(&wrapper);
            if(!result)throw Exception("TypeError",u"SvgMenclose receiver has the wrong native wrapper type");
            return *result;
        };
        auto numberArg=[](State& state,std::span<const Value> args,std::size_t index){
            const Value value=index<args.size()?args[index]:Value(Undefined{});
            return state.context.toNumber(value);
        };
        auto array4=[&](const std::array<double,4>& values){
            return Value(valueArray({Value(values[0]),Value(values[1]),Value(values[2]),Value(values[3])}));
        };
        auto array2=[&](const std::array<double,2>& values){
            return Value(valueArray({Value(values[0]),Value(values[1])}));
        };
        for(const std::u16string_view name:{u"getParameters",u"getNotations",u"removeRedundantNotations",
                                            u"initializeNotations"}){
            installMethod(*menclosePrototype,std::u16string(name),0,[name,menclose](State&,CommonWrapper& wrapper,
                std::span<const Value>){
                auto& value=menclose(wrapper);
                if(name==u"getParameters")value.getParameters();
                else if(name==u"getNotations")value.getNotations();
                else if(name==u"removeRedundantNotations")value.removeRedundantNotations();
                else value.initializeNotations();
                return Value(Undefined{});
            });
        }
        installMethod(*menclosePrototype,u"getBBoxExtenders",0,[menclose,array4](State&,
            CommonWrapper& wrapper,std::span<const Value>){return array4(menclose(wrapper).getBBoxExtenders());});
        installMethod(*menclosePrototype,u"getPadding",0,[menclose,array4](State&,
            CommonWrapper& wrapper,std::span<const Value>){return array4(menclose(wrapper).getPadding());});
        installMethod(*menclosePrototype,u"maximizeEntries",2,[](State& state,
            CommonWrapper& wrapper,std::span<const Value> args){
            if(args.size()<2)throw Exception("TypeError",u"maximizeEntries requires two arrays");
            const auto* first=std::get_if<ObjectRef>(&args[0].data);
            const auto* second=std::get_if<ObjectRef>(&args[1].data);
            if(!first||!second)throw Exception("TypeError",u"maximizeEntries arguments must be arrays");
            std::size_t index=0;
            for(;;++index){
                // The Source loop re-reads X.length on every condition and
                // performs X[i], Y[i], comparison, and possible Set in order.
                const auto length=state.context.toNumber(
                    state.context.get(Value(*first),Key(u"length")));
                if(!(static_cast<double>(index)<length))break;
                const auto digits=std::to_string(index);
                std::u16string keyText;keyText.reserve(digits.size());
                for(const char digit:digits)keyText.push_back(static_cast<char16_t>(digit));
                const Key key(std::move(keyText));
                const auto x=state.context.get(Value(*first),key);
                const auto y=state.context.get(Value(*second),key);
                const auto left=state.context.toPrimitive(x,options::PrimitiveHint::numberHint);
                const auto right=state.context.toPrimitive(y,options::PrimitiveHint::numberHint);
                bool less=false;
                const auto* leftString=std::get_if<std::u16string>(&left.data);
                const auto* rightString=std::get_if<std::u16string>(&right.data);
                if(leftString&&rightString)less=*leftString<*rightString;
                else less=state.context.toNumber(left)<state.context.toNumber(right);
                if(less)state.context.set(Value(*first),key,y);
            }
            return Value(Undefined{});
        });
        installMethod(*menclosePrototype,u"getOffset",1,[menclose](State& state,
            CommonWrapper& wrapper,std::span<const Value> args){
            const auto direction=args.empty()?std::u16string{}:
                property_host::string(state.properties.from(args.front()));
            return Value(menclose(wrapper).getOffset(property_host::debugText(direction)));
        });
        installMethod(*menclosePrototype,u"getArgMod",2,[menclose,array2,numberArg](State& state,
            CommonWrapper& wrapper,std::span<const Value> args){
            return array2(menclose(wrapper).getArgMod(numberArg(state,args,0),numberArg(state,args,1)));
        });
        installMethod(*menclosePrototype,u"arrow",5,[menclose](State& state,
            CommonWrapper& wrapper,std::span<const Value> args){
            const auto offset=args.size()<4||std::holds_alternative<Undefined>(args[3].data)
                ?std::string{}:property_host::debugText(property_host::string(state.properties.from(args[3])));
            const double distance=args.size()<5||std::holds_alternative<Undefined>(args[4].data)
                ?0.0:property_host::number(state.properties.from(args[4]));
            const bool doubleHeaded=args.size()>2&&property_host::truthy(state.properties.from(args[2]));
            return menclose(wrapper).arrow(
                args.empty()?std::numeric_limits<double>::quiet_NaN():property_host::number(state.properties.from(args[0])),
                args.size()<2?std::numeric_limits<double>::quiet_NaN():property_host::number(state.properties.from(args[1])),
                doubleHeaded,offset,distance);
        });
        installMethod(*menclosePrototype,u"arrowData",0,[menclose](State& state,
            CommonWrapper& wrapper,std::span<const Value>){
            const auto data=menclose(wrapper).arrowData();
            const auto result=state.context.object();
            state.context.set(Value(result),Key(u"a"),Value(data.angle));
            state.context.set(Value(result),Key(u"W"),Value(data.width));
            state.context.set(Value(result),Key(u"x"),Value(data.x));
            state.context.set(Value(result),Key(u"y"),Value(data.y));
            return Value(result);
        });
        installMethod(*menclosePrototype,u"arrowAW",0,[menclose,array2](State&,
            CommonWrapper& wrapper,std::span<const Value>){return array2(menclose(wrapper).arrowAW());});
        installMethod(*menclosePrototype,u"createMsqrt",1,[menclose](State& state,
            CommonWrapper& wrapper,std::span<const Value> args){
            if(args.empty())throw Exception("TypeError",u"createMsqrt child is missing");
            const auto* object=std::get_if<ObjectRef>(&args.front().data);
            if(!object)throw Exception("TypeError",u"createMsqrt child is not an object");
            auto& child=state.factories.resolveWrapper(*object);
            auto& square=menclose(wrapper).createMsqrt(child);
            return Value(square.factory().objectFor(square));
        });
        installMethod(*menclosePrototype,u"sqrtTRBL",0,[menclose,array4](State&,
            CommonWrapper& wrapper,std::span<const Value>){return array4(menclose(wrapper).sqrtTRBL());});

        auto textAddStyles=context.function(u"addStyles",
            [weak=std::weak_ptr<State>(shared_from_this())](Context&,Value,
                std::span<const Value> args)->Value{
                auto owner=weak.lock();
                if(!owner)throw Exception("Unimplemented",u"SVG wrapper source methods have been released");
                if(args.size()<2)throw Exception("TypeError",u"SvgTextNode.addStyles arguments are missing");
                const auto* jax=std::get_if<ObjectRef>(&args[1].data);
                if(!jax)throw Exception("TypeError",u"SvgTextNode.addStyles Jax is not an object");
                const auto options=owner->context.get(Value(*jax),Key(u"options"));
                const auto blacker=owner->context.get(options,Key(u"blacker"));
                const auto rule=owner->context.object();
                owner->context.set(Value(rule),Key(u"stroke-width"),blacker);
                const auto style=owner->context.object();
                owner->context.set(Value(style),Key(
                    u"mjx-container[jax=\"SVG\"] path[data-c], mjx-container[jax=\"SVG\"] use[data-c]"),
                    Value(rule));
                owner->callSheetAddStyles(args[0],Value(style));
                return Undefined{};
            });
        context.define(textAddStyles,Key(u"length"),Descriptor::data(Value(2),false,false,true));
        context.set(Value(text),Key(u"addStyles"),Value(textAddStyles));
        textNode=text;
        commonWrapper=base;
    }
};

SvgWrapperSourceMethods::SvgWrapperSourceMethods(Context& context,
    ContextPropertyBindings& properties,common::CommonFactoryContextBindings& factories,
    common::BBoxContextBindings& bboxes,math::tex::ParserRuntime& parser)
    :state_(std::make_shared<State>(context,properties,factories,bboxes,parser)){}
SvgWrapperSourceMethods::~SvgWrapperSourceMethods()=default;
Context& SvgWrapperSourceMethods::context()const{return state_->context;}
void SvgWrapperSourceMethods::install(const SvgWrapperConstructorSet& graph){state_->install(graph);}
}
