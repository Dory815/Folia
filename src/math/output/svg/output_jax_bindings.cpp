module folia.math.output.svg.output_jax_bindings;
import folia.math.output.svg.wrapper_classes;
import folia.math.output.svg.wrapper_constructors;
import folia.math.output.svg.menclose_surface;
import folia.math.util.option_types;
import folia.math.util.property_host;
import std;

namespace folia::math::output::svg {
using namespace options;

struct SvgOutputJaxContextBindings::State
    : std::enable_shared_from_this<SvgOutputJaxContextBindings::State> {
    common::CommonJaxClassBindings& commonClasses;
    common::CommonRuntimeBindings& runtime;
    SvgWrapperFactoryContextBindings& factories;
    OptionFunctionBindings& functions;
    FunctionListContextBindings& filters;
    Context& context;
    MapContextBindings& maps;
    math::tex::ParserRuntime* parser=nullptr;
    std::optional<ObjectRef> constructor;
    ObjectRef factoryConstructor;
    ObjectRef defaultFont;
    std::optional<ObjectRef> defaultHtmlMethod;
    std::shared_ptr<SvgMencloseSourceSurface> mencloseSurface;
    std::shared_ptr<SvgMactionSourceSurface> mactionSurface;
    std::shared_ptr<SceneContextNodes> sceneContextNodes;
    std::shared_ptr<adaptors::lite::LiteModule> liteModule;

    State(common::CommonJaxClassBindings& common,common::CommonRuntimeBindings& rt,
          SvgWrapperFactoryContextBindings& factoryBindings,
          OptionFunctionBindings& optionBindings,FunctionListContextBindings& filterBindings,
          MapContextBindings& mapBindings)
        :commonClasses(common),runtime(rt),factories(factoryBindings),functions(optionBindings),
         filters(filterBindings),context(optionBindings.context()),
         maps(mapBindings){
        if(&context!=&runtime.context()||&context!=&factories.context()||&context!=&optionBindings.context())
            throw Exception("TypeError",u"SVG output bindings must share one actual Context");
        if(&context!=&mapBindings.context())
            throw Exception("TypeError",u"SVG output and Map bindings must share one actual Context");
    }

    void copyEnumerable(Value source,ObjectRef target){
        if(std::holds_alternative<Undefined>(source.data)||std::holds_alternative<Null>(source.data))return;
        (void)context.kind(std::get<ObjectRef>(source.data));
        const auto keys=context.enumerableStringKeys(source);
        for(const auto& key:keys)
            context.set(Value(target),Key(key),context.get(source,Key(key)));
    }

    void addOptions(ObjectRef jaxClass){
        // The Source class literal spreads CommonOutputJax.OPTIONS first, then
        // adds these SVG-specific defaults to a fresh ordinary object.
        const auto base=context.get(Value(jaxClass),Key(u"OPTIONS"));
        const ObjectRef options=context.object();
        copyEnumerable(base,options);
        context.set(Value(options),Key(u"blacker"),Value(3));
        context.set(Value(options),Key(u"fontCache"),Value(u"local"));
        context.set(Value(options),Key(u"localID"),Null{});
        context.set(Value(options),Key(u"useXlink"),Value(true));
        context.define(jaxClass,Key(u"OPTIONS"),Descriptor::data(Value(options),true,true,true));

        // commonStyles uses the same Source object-spread semantics and remains
        // a class-owned record read by CommonOutputJax.styleSheet at call time.
        const auto baseStyles=context.get(Value(jaxClass),Key(u"commonStyles"));
        const ObjectRef styles=context.object();
        copyEnumerable(baseStyles,styles);
        auto rule=[&](std::u16string_view selector,
            std::initializer_list<std::pair<std::u16string_view,Value>> entries){
            const auto value=context.object();
            for(const auto& [key,entry]:entries)context.set(Value(value),Key(std::u16string(key)),entry);
            context.set(Value(styles),Key(std::u16string(selector)),Value(value));
        };
        rule(u"mjx-container[jax=\"SVG\"]",{{u"direction",Value(u"ltr")},{u"white-space",Value(u"nowrap")}});
        rule(u"mjx-container[jax=\"SVG\"] > svg",{{u"overflow",Value(u"visible")},
            {u"min-height",Value(u"1px")},{u"min-width",Value(u"1px")}});
        rule(u"mjx-container[jax=\"SVG\"] > svg a",{{u"fill",Value(u"blue")},{u"stroke",Value(u"blue")}});
        rule(u"rect[data-sre-highlighter-added]:has(+ .mjx-selected), rect[data-sre-highlighter-bbox].mjx-selected",
            {{u"stroke",Value(u"black")},{u"stroke-width",Value(u"80px")}});
        const auto darkHighlighter=context.object();
        context.set(Value(darkHighlighter),Key(u"stroke"),Value(u"#C8C8C8"));
        const auto darkRules=context.object();
        context.set(Value(darkRules),Key(u"rect[data-sre-highlighter-added]:has(+ .mjx-selected), rect[data-sre-highlighter-bbox].mjx-selected"),Value(darkHighlighter));
        context.set(Value(styles),Key(u"@media (prefers-color-scheme: dark)"),Value(darkRules));
        context.define(jaxClass,Key(u"commonStyles"),Descriptor::data(Value(styles),true,true,true));
        context.define(jaxClass,Key(u"NAME"),Descriptor::data(Value(u"SVG"),true,true,true));
        context.define(jaxClass,Key(u"FONTCACHEID"),Descriptor::data(Value(u"MJX-SVG-global-cache"),true,true,true));
        context.define(jaxClass,Key(u"STYLESHEETID"),Descriptor::data(Value(u"MJX-SVG-styles"),true,true,true));
    }

    std::shared_ptr<SvgOutputJax> resolve(ObjectRef object)const{
        context.kind(object);
        const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(object));
        if(!payload||!*payload)
            throw Exception("TypeError",u"Context object is not an SVG output Jax");
        auto owner=std::dynamic_pointer_cast<SvgOutputJax>(*payload);
        if(!owner||owner->instanceIdentity()!=object)
            throw Exception("TypeError",u"Context object is not an SVG output Jax");
        return owner;
    }
    ObjectRef create(ObjectRef actualClass,Value supplied){
        const auto identity=context.instance(actualClass);
        auto owner=std::make_shared<SvgOutputJax>(functions,identity,filters);
        context.payload(identity)=std::static_pointer_cast<NativeObject>(owner);
        owner->bindOutputJaxFields(std::static_pointer_cast<void>(owner));
        owner->bindMencloseSurface(mencloseSurface);
        owner->bindMactionSurface(mactionSurface);
        owner->bindSceneContextNodes(sceneContextNodes,liteModule);
        if(defaultHtmlMethod)owner->setDefaultHtmlMethod(*defaultHtmlMethod);
        owner->initializeSvg(std::move(supplied),factoryConstructor,defaultFont,
                             runtime,std::static_pointer_cast<void>(owner));
        if(!parser)throw Exception("TypeError",u"SVG wrapper nodeMap requires the actual ParserRuntime");
        auto self=shared_from_this();
        owner->bindNodeMap(
            [self]{
                auto nodeIdentity=[self](mml::MmlNode& node){
                    const auto object=self->parser->mmlNodeIdentities.find(&node);
                    if(!object)throw Exception("TypeError",u"MML node has no canonical Context identity");
                    return *object;
                };
                auto wrapperIdentity=[](common::CommonWrapper& wrapper){
                    return wrapper.factory().objectFor(wrapper);
                };
                auto wrapperResolver=[self](ObjectRef object)->common::CommonWrapper&{
                    return self->runtime.factories.resolveWrapper(object);
                };
                return std::make_shared<common::WrapperNodeMap>(
                    self->context,self->maps,std::move(nodeIdentity),
                    std::move(wrapperIdentity),std::move(wrapperResolver));
            },
            [self,identity](Value value){
                self->context.set(Value(identity),Key(u"nodeMap"),std::move(value));
            });
        return identity;
    }
};

SvgOutputJaxContextBindings::SvgOutputJaxContextBindings(
    common::CommonJaxClassBindings& commonClasses,common::CommonRuntimeBindings& runtime,
    SvgWrapperFactoryContextBindings& factories,OptionFunctionBindings& functions,
    FunctionListContextBindings& filters,MapContextBindings& maps)
    :state_(std::make_shared<State>(commonClasses,runtime,factories,functions,filters,maps)){}
SvgOutputJaxContextBindings::~SvgOutputJaxContextBindings()=default;
Context& SvgOutputJaxContextBindings::context()const{return state_->context;}
MapContextBindings& SvgOutputJaxContextBindings::mapBindings()const{return state_->maps;}
void SvgOutputJaxContextBindings::bindMencloseSurface(
    std::shared_ptr<SvgMencloseSourceSurface> surface){
    if(!surface)throw Exception("TypeError",u"Cannot bind an empty SVG menclose surface");
    if(&surface->context()!=&state_->context)
        throw Exception("TypeError",u"SvgMenclose surface belongs to another Context");
    if(state_->constructor)
        throw Exception("TypeError",u"SvgMenclose surface must be bound before Jax construction");
    state_->mencloseSurface=std::move(surface);
}
void SvgOutputJaxContextBindings::bindMactionSurface(
    std::shared_ptr<SvgMactionSourceSurface> surface){
    if(!surface)throw Exception("TypeError",u"Cannot bind an empty SVG Maction surface");
    if(&surface->context()!=&state_->context)
        throw Exception("TypeError",u"SvgMaction surface belongs to another Context");
    if(state_->constructor)
        throw Exception("TypeError",u"SvgMaction surface must be bound before Jax construction");
    state_->mactionSurface=std::move(surface);
}

void SvgOutputJaxContextBindings::bindSceneContextNodes(
    std::shared_ptr<SceneContextNodes> nodes,
    std::shared_ptr<adaptors::lite::LiteModule> lite){
    if(!nodes||!lite||&nodes->context()!=&state_->context||&lite->context()!=&state_->context)
        throw Exception("TypeError",u"SVG scene providers belong to another Context");
    if(state_->sceneContextNodes&&state_->sceneContextNodes!=nodes)
        throw Exception("TypeError",u"SVG scene providers are already bound");
    if(state_->liteModule&&state_->liteModule!=lite)
        throw Exception("TypeError",u"SVG scene providers are already bound");
    state_->sceneContextNodes=std::move(nodes);
    state_->liteModule=std::move(lite);
}

ObjectRef SvgOutputJaxContextBindings::registerClass(ObjectRef commonConstructor,
    ObjectRef actualSvgWrappers,ObjectRef defaultFont){
    auto state=state_;
    state->context.kind(commonConstructor);
    state->context.kind(defaultFont);
    if(state->constructor)return *state->constructor;
    state->commonClasses.bindClassDefaults(commonConstructor);
    if(!state->parser)
        throw Exception("TypeError",u"SVG wrapper constructors require the actual ParserRuntime");
    (void)registerImplementedWrapperClasses(state->context,actualSvgWrappers,
                                             state->runtime.factories,state->parser->properties);
    const auto commonFactoryConstructor=state->runtime.factories.registerConstructor();
    state->factoryConstructor=state->factories.registerConstructor(
        actualSvgWrappers,state->runtime.factories,commonFactoryConstructor);
    state->defaultFont=defaultFont;
    auto ctor=state->context.nativeConstructor(u"SVG",
        [weak=std::weak_ptr<State>(state)](Context&,std::span<const Value> args)->ObjectRef{
            auto bindings=weak.lock();
            if(!bindings)throw Exception("Unimplemented",u"SVG output module has been released");
            const Value supplied=args.empty()?Value(Undefined{}):args.front();
            if(!bindings->constructor)throw Exception("Unimplemented",u"SVG output constructor is not registered");
            return bindings->create(*bindings->constructor,supplied);
        });
    state->context.define(ctor,Key(u"name"),Descriptor::data(Value(u"SVG"),false,false,true));
    state->context.define(ctor,Key(u"length"),Descriptor::data(Value(1),false,false,true));
    const auto prototype=std::get<ObjectRef>(state->context.get(Value(ctor),Key(u"prototype")).data);
    state->context.setPrototype(ctor,commonConstructor);
    const auto parentPrototype=state->context.get(Value(commonConstructor),Key(u"prototype"));
    const auto* parent=std::get_if<ObjectRef>(&parentPrototype.data);
    if(!parent)throw Exception("TypeError",u"CommonOutputJax constructor has no prototype");
    state->context.setPrototype(prototype,*parent);
    state->addOptions(ctor);
    const auto svgPrototype=std::get<ObjectRef>(
        state->context.get(Value(ctor),Key(u"prototype")).data);
    const auto htmlMethod=state->context.get(Value(svgPrototype),Key(u"html"));
    const auto* htmlFunction=std::get_if<ObjectRef>(&htmlMethod.data);
    if(!htmlFunction||state->context.kind(*htmlFunction)!=ObjectKind::function)
        throw Exception("TypeError",u"AbstractOutputJax.html is not callable");
    state->defaultHtmlMethod=*htmlFunction;
    const auto fixedMethod=state->context.function(u"fixed",
        [weak=std::weak_ptr<State>(state)](Context& cx,Value receiver,
            std::span<const Value> args)->Value{
            auto owner=weak.lock();
            if(!owner)throw Exception("Unimplemented",u"SVG output methods have been released");
            const auto* identity=std::get_if<ObjectRef>(&receiver.data);
            if(!identity)throw Exception("TypeError",u"OutputJax.fixed receiver is not an object");
            auto jax=owner->resolve(*identity);
            const double number=args.empty()?std::numeric_limits<double>::quiet_NaN():cx.toNumber(args[0]);
            const double requested=args.size()<2||std::holds_alternative<Undefined>(args[1].data)
                ?3.0:cx.toNumber(args[1]);
            if((!std::isfinite(requested)&&!std::isnan(requested))||requested<0||requested>100)
                throw Exception("RangeError",u"toFixed() digits argument must be between 0 and 100");
            return Value(property_host::text(jax->fixed(number,
                static_cast<std::uint32_t>(std::isnan(requested)?0:std::trunc(requested)))));
        });
    state->context.define(fixedMethod,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    state->context.set(Value(svgPrototype),Key(u"fixed"),Value(fixedMethod));
    const auto svgMethod=state->context.function(u"svg",
        [weak=std::weak_ptr<State>(state)](Context&,Value receiver,
            std::span<const Value> args)->Value{
            auto owner=weak.lock();
            if(!owner)throw Exception("Unimplemented",u"SVG output methods have been released");
            const auto* identity=std::get_if<ObjectRef>(&receiver.data);
            if(!identity)throw Exception("TypeError",u"SVG.svg receiver is not an object");
            auto jax=owner->resolve(*identity);
            return jax->dispatchSvg(args);
        });
    state->context.define(svgMethod,Key(u"length"),Descriptor::data(Value(3),false,false,true));
    state->context.set(Value(svgPrototype),Key(u"svg"),Value(svgMethod));
    const auto processMathMethod=state->context.function(u"",
        [weak=std::weak_ptr<State>(state)](Context&,Value receiver,
            std::span<const Value> args)->Value{
            auto owner=weak.lock();
            if(!owner)throw Exception("Unimplemented",u"SVG output methods have been released");
            const auto* identity=std::get_if<ObjectRef>(&receiver.data);
            if(!identity)throw Exception("TypeError",u"SVG.processMath receiver is not an object");
            if(args.size()<2)throw Exception("TypeError",u"SVG.processMath requires a wrapper and parent node");
            const auto* wrapperIdentity=std::get_if<ObjectRef>(&args[0].data);
            if(!wrapperIdentity)throw Exception("TypeError",u"SVG.processMath wrapper is not an object");
            const auto* parentIdentity=std::get_if<ObjectRef>(&args[1].data);
            if(!parentIdentity)throw Exception("TypeError",u"SVG.processMath parent is not an object");
            auto jax=owner->resolve(*identity);
            auto& wrapper=owner->runtime.factories.resolveWrapper(*wrapperIdentity);
            const auto parentValue=propertyNode(owner->runtime.properties.from(Value(*parentIdentity)));
            const auto* parent=std::get_if<DOMNodeRef>(&parentValue);
            if(!parent||!*parent)
                throw Exception("TypeError",u"SVG.processMath parent is not a Lite DOM node");
            jax->processMathObserved(wrapper,*parent);
            return Value(Undefined{});
        });
    state->context.define(processMathMethod,Key(u"length"),
        Descriptor::data(Value(2),false,false,true));
    // The locked CJS source assigns `SVG.prototype.processMath = function (...)`.
    state->context.define(svgPrototype,Key(u"processMath"),
        Descriptor::data(Value(processMathMethod),true,true,true));
    state->constructor=ctor;
    return ctor;
}

ObjectRef SvgOutputJaxContextBindings::registerClass(ObjectRef commonConstructor,
    math::tex::ParserRuntime& parser,ObjectRef defaultFont){
    if(&parser.context()!=&state_->context)
        throw Exception("TypeError",u"SVG parser runtime belongs to a different Context");
    state_->runtime.factories.bindFactoryMethods(state_->runtime.modules,state_->runtime.properties,state_->maps,
        [&parser](ObjectRef identity)->mml::MmlNode&{
            return parser.resolveMmlNode(identity);
        },
        [&parser](mml::MmlNode& node)->ObjectRef{
            const auto identity=parser.mmlNodeIdentities.find(&node);
            if(!identity)throw Exception("TypeError",u"MML node has no canonical Context identity");
            return *identity;
        },parser.executionLifetime());
    auto constructors=loadSvgWrapperConstructors(state_->runtime.modules,parser,
                                                   state_->runtime.factories);
    state_->parser=&parser;
    return registerClass(commonConstructor,constructors.wrappers,defaultFont);
}

ObjectRef SvgOutputJaxContextBindings::constructor()const{
    if(!state_->constructor)throw Exception("TypeError",u"SVG output constructor is not registered");
    return *state_->constructor;
}
std::shared_ptr<SvgOutputJax> SvgOutputJaxContextBindings::construct(Value supplied){
    const std::array<Value,1> args{std::move(supplied)};
    const auto instance=state_->context.construct(constructor(),args);
    return resolve(instance);
}
std::shared_ptr<SvgOutputJax> SvgOutputJaxContextBindings::resolve(ObjectRef instance)const{
    return state_->resolve(instance);
}
}
