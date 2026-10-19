module folia.math.adaptors.lite.source_text_content;
import folia.math.util.property_host;
import std;

namespace folia::math::adaptors::lite {
namespace {
using namespace options;
[[noreturn]] void typeError(std::u16string message) {
    throw Exception("TypeError",std::move(message));
}

}

Value sourceTextContent(Context& context,Value receiver,Value node) {
    if(std::holds_alternative<Undefined>(node.data))
        typeError(u"Cannot read properties of undefined (reading 'children')");
    if(std::holds_alternative<Null>(node.data))
        typeError(u"Cannot read properties of null (reading 'children')");
    const Value children=context.get(node,Key(u"children"));
    if(std::holds_alternative<Undefined>(children.data))
        typeError(u"Cannot read properties of undefined (reading 'reduce')");
    if(std::holds_alternative<Null>(children.data))
        typeError(u"Cannot read properties of null (reading 'reduce')");
    const Value reduce=context.get(children,Key(u"reduce"));
    const ObjectRef callback=context.function(u"",
        [receiver](Context& cx,Value,std::span<const Value> args)->Value {
            const Value accumulator=args.empty()?Value(Undefined{}):args[0];
            const Value child=args.size()>1?args[1]:Value(Undefined{});
            if(std::holds_alternative<Undefined>(child.data))
                typeError(u"Cannot read properties of undefined (reading 'kind')");
            if(std::holds_alternative<Null>(child.data))
                typeError(u"Cannot read properties of null (reading 'kind')");
            const Value kind=cx.get(child,Key(u"kind"));
            if(kind==Value(u"#text")) {
                const Value value=cx.get(child,Key(u"value"));
                return property_host::add(cx,accumulator,value);
            }
            if(kind==Value(u"#comment"))return accumulator;
            const Value method=cx.get(receiver,Key(u"textContent"));
            const std::array<Value,1> recursiveArgs{child};
            const Value nested=cx.callMember(method,receiver,recursiveArgs,
                u"LiteAdaptor.textContent");
            return property_host::add(cx,accumulator,nested);
        });
    context.define(callback,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    const std::array<Value,2> reduceArgs{Value(callback),Value(u"")};
    return context.callMember(reduce,children,reduceArgs,u"LiteAdaptor.textContent children.reduce");
}
}
