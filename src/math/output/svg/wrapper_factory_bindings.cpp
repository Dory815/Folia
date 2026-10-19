/*************************************************************
 * Source-order SvgWrapperFactory defaultNodes extraction.
 *************************************************************/
module folia.math.output.svg.wrapper_factory_bindings;
import folia.math.util.property_host;
import std;

namespace folia::math::output::svg {
common::CommonWrapperClassMap readDefaultNodes(
    options::Context& context, options::ObjectRef actualConstructor,
    const WrapperClassResolver& resolveClass) {
    context.kind(actualConstructor);
    if (!resolveClass)
        throw options::Exception("TypeError", u"SvgWrapperFactory has no canonical wrapper-class resolver");

    // Evaluate the static property once, as the Source expression does, then
    // snapshot only its own enumerable string keys. Values stay live until
    // their individual key is reached, so getters may mutate later entries.
    const auto nodes = context.get(options::Value(actualConstructor),
                                   options::Key(u"defaultNodes"));
    return readDefaultNodes(context,nodes,resolveClass);
}

common::CommonWrapperClassMap readDefaultNodes(
    options::Context& context,options::Value nodes,
    const WrapperClassResolver& resolveClass){
    if(!resolveClass)
        throw options::Exception("TypeError",u"SvgWrapperFactory has no canonical wrapper-class resolver");
    const auto keys = context.enumerableStringKeys(nodes);
    common::CommonWrapperClassMap result;
    result.reserve(keys.size());
    for (const auto& key : keys) {
        const auto value = context.get(nodes, options::Key(key));
        const auto* constructor = std::get_if<options::ObjectRef>(&value.data);
        if (!constructor)
            throw options::Exception("TypeError", u"SvgWrapperFactory.defaultNodes entry is not a constructor object");
        auto descriptor = resolveClass(*constructor);
        if (!descriptor)
            throw options::Exception("TypeError", u"SvgWrapperFactory.defaultNodes constructor has no native wrapper class");
        result.emplace_back(property_host::debugText(key), std::move(descriptor));
    }
    return result;
}

struct SvgWrapperFactoryContextBindings::State
    : std::enable_shared_from_this<SvgWrapperFactoryContextBindings::State> {
    options::Context& context;
    common::CommonFactoryContextBindings::FactoryResolver factoryResolver;
    common::CommonFactoryContextBindings::WrapperClassResolver wrapperClassResolver;
    std::shared_ptr<void> factoryLifetime;
    std::optional<options::ObjectRef> constructor;

    State(options::Context& cx,common::CommonFactoryContextBindings& bindings)
        :context(cx),factoryResolver(bindings.factoryResolver()),
         wrapperClassResolver(bindings.wrapperClassResolver()),
         factoryLifetime(bindings.lifetimeToken()){
        if(&context!=&bindings.context())
            throw options::Exception("TypeError",u"SVG and Common factory bindings must share one Context");
    }
    std::shared_ptr<SvgWrapperFactory> resolve(options::ObjectRef object)const{
        auto base=factoryResolver(object);
        auto result=std::dynamic_pointer_cast<SvgWrapperFactory>(std::move(base));
        if(!result)throw options::Exception("TypeError",u"Factory instance is not an SvgWrapperFactory");
        return result;
    }
};

SvgWrapperFactoryContextBindings::SvgWrapperFactoryContextBindings(
    options::Context& context,common::CommonFactoryContextBindings& factories)
    :state_(std::make_shared<State>(context,factories)){}
SvgWrapperFactoryContextBindings::~SvgWrapperFactoryContextBindings()=default;
options::Context& SvgWrapperFactoryContextBindings::context()const{return state_->context;}

options::ObjectRef SvgWrapperFactoryContextBindings::registerConstructor(
    options::ObjectRef actualSvgWrappers,
    common::CommonFactoryContextBindings& commonBindings,
    std::optional<options::ObjectRef> parentConstructor){
    auto state=state_;
    if(state->constructor)return *state->constructor;
    state->context.kind(actualSvgWrappers);
    if(&commonBindings.context()!=&state->context)
        throw options::Exception("TypeError",u"SVG and Common factory bindings must share one Context");
    if(!parentConstructor)parentConstructor=commonBindings.constructor();
    const std::weak_ptr<State> weak=state;
    const auto constructor=commonBindings.registerConstructor(
        u"SvgWrapperFactory",parentConstructor,
        [weak](options::Context& cx,options::ObjectRef actualConstructor,
                         std::span<const options::Value> args)
            ->std::shared_ptr<common::CommonWrapperFactory>{
            const auto bindings=weak.lock();
            if(!bindings)throw options::Exception("Unimplemented",u"SVG wrapper factory module has been released");
            // Source uses the supplied map when non-nullish, otherwise it
            // reads this.constructor.defaultNodes at construction time.
            const auto nodes=args.empty()||std::holds_alternative<options::Undefined>(args.front().data)||
                std::holds_alternative<options::Null>(args.front().data)
                ?cx.get(options::Value(actualConstructor),options::Key(u"defaultNodes"))
                :args.front();
            auto classes=readDefaultNodes(cx,nodes,
                bindings->wrapperClassResolver);
            return std::make_shared<SvgWrapperFactory>(std::move(classes));
        });
    state->context.define(constructor,options::Key(u"defaultNodes"),
        options::Descriptor::data(options::Value(actualSvgWrappers),true,true,true));
    state->constructor=constructor;
    return constructor;
}

std::shared_ptr<SvgWrapperFactory> SvgWrapperFactoryContextBindings::construct(
    std::span<const options::Value> args){
    if(!state_->constructor)
        throw options::Exception("TypeError",u"SvgWrapperFactory constructor is not registered");
    const auto instance=state_->context.construct(*state_->constructor,args);
    return resolve(instance);
}
std::shared_ptr<SvgWrapperFactory> SvgWrapperFactoryContextBindings::resolve(
    options::ObjectRef object)const{return state_->resolve(object);}
}
