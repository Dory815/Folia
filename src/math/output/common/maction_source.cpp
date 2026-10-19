/*************************************************************
 * Source call adapters for CommonMaction.
 *************************************************************/
module folia.math.output.common.maction_source;
import std;

namespace folia::math::output::common {
using namespace options;

void initializeCommonMactionSource(CommonMaction& wrapper){
    auto& context=wrapper.jax().optionsContext();
    const auto identity=wrapper.factory().objectFor(wrapper);
    using options::Key;
    using options::ObjectKind;
    using options::ObjectRef;
    using options::Value;

    const auto constructor=context.get(Value(identity),Key(u"constructor"));
    const auto actions=context.get(constructor,Key(u"actions"));
    const auto mapGet=context.get(actions,Key(u"get"));
    const auto* getFunction=std::get_if<ObjectRef>(&mapGet.data);
    if(!getFunction||context.kind(*getFunction)!=ObjectKind::function)
        throw Exception("TypeError",u"this.constructor.actions.get is not a function");

    const auto sourceNode=context.get(Value(identity),Key(u"node"));
    const auto attributes=context.get(sourceNode,Key(u"attributes"));
    const auto attributeGet=context.get(attributes,Key(u"get"));
    const auto* attributeFunction=std::get_if<ObjectRef>(&attributeGet.data);
    if(!attributeFunction||context.kind(*attributeFunction)!=ObjectKind::function)
        throw Exception("TypeError",u"this.node.attributes.get is not a function");
    const std::array<Value,1> actionTypeArgs={Value(u"actiontype")};
    const auto actionType=context.call(*attributeFunction,attributes,actionTypeArgs);
    const std::array<Value,1> lookupArgs={actionType};
    auto pair=context.call(*getFunction,actions,lookupArgs);
    if(!options::truthy(pair)){
        pair=Value(context.array(2));
        const auto noop=context.function(u"",[](Context&,Value,std::span<const Value>){
            return Value(Undefined{});
        });
        context.define(noop,Key(u"length"),Descriptor::data(Value(2),false,false,true));
        context.set(pair,Key(u"0"),Value(noop));
        context.set(pair,Key(u"1"),Value(context.object()));
    }
    const auto entries=context.readIterable(pair,2);
    const auto handler=context.get(entries,Key(u"0"));
    const auto data=context.get(entries,Key(u"1"));
    context.set(Value(identity),Key(u"action"),handler);
    context.set(Value(identity),Key(u"data"),data);

    const auto getParameters=context.get(Value(identity),Key(u"getParameters"));
    const auto* parameters=std::get_if<ObjectRef>(&getParameters.data);
    if(!parameters||context.kind(*parameters)!=ObjectKind::function)
        throw Exception("TypeError",u"this.getParameters is not a function");
    (void)context.call(*parameters,Value(identity));
}

Value invokeCommonMactionAction(CommonMaction& wrapper){
    auto& context=wrapper.jax().optionsContext();
    const auto identity=wrapper.factory().objectFor(wrapper);
    const auto action=context.get(Value(identity),Key(u"action"));
    const auto* function=std::get_if<ObjectRef>(&action.data);
    if(!function||context.kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",u"this.action is not a function");
    const std::array<Value,2> args={Value(identity),context.get(Value(identity),Key(u"data"))};
    return context.call(*function,Value(identity),args);
}
}
