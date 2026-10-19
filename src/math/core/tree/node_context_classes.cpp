// Non-template Context value primitives for the shared Tree class binder.
module folia.math.core.tree.node_context_classes;
import folia.math.util.property_host;
import std;

namespace folia::math::tree {
options::Key treeContextPropertyKey(options::Context& context,
                                    ContextPropertyBindings& properties,options::Value raw) {
    const auto primitive=context.toPrimitive(std::move(raw),options::PrimitiveHint::stringHint);
    if(const auto* symbol=std::get_if<options::SymbolRef>(&primitive.data))return options::Key(*symbol);
    const auto text=property_host::string(properties.from(primitive));
    return options::Key(text);
}

Property treeContextCopyOwnEnumerable(options::Context& context,
                                      ContextPropertyBindings& properties,options::Value source) {
    // Pinned CJS __assign selects native Object.assign. Capture [[OwnPropertyKeys]]
    // before invoking a getter; test each descriptor and Get in that order.
    // Context::set on the fresh ordinary target preserves its inherited
    // __proto__ setter and all enumerable Symbol keys.
    const auto target=context.object();
    if(std::holds_alternative<options::Null>(source.data)||
       std::holds_alternative<options::Undefined>(source.data))
        return properties.from(options::Value(target));
    const auto keys=context.ownKeys(source);
    for(const auto& key:keys){
        if(!context.ownPropertyEnumerable(source,key))continue;
        const auto value=context.get(source,key);
        context.set(options::Value(target),key,value);
    }
    return properties.from(options::Value(target));
}

std::string treeContextString(options::Context& context,
                              ContextPropertyBindings& properties,options::Value value) {
    const auto primitive=context.toPrimitive(std::move(value),options::PrimitiveHint::defaultHint);
    if(std::holds_alternative<options::SymbolRef>(primitive.data))
        throw options::Exception("TypeError",u"Cannot convert a Symbol value to a string");
    return property_host::debugText(property_host::string(properties.from(primitive)));
}
}
