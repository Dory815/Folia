// Ordinary iterator protocol and TypeScript __values/__read helper semantics.
module folia.math.util.option_types;
import std;

namespace folia::math::options {
namespace {
struct ArrayLikeIteratorState {
    Value source;
    std::uint64_t index=0;
    bool exhausted=false;
};

Value read_iterator_method(Context& context,const Value& iterable) {
    if(std::holds_alternative<Null>(iterable.data))
        throw Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    if(std::holds_alternative<Undefined>(iterable.data))
        throw Exception("TypeError",u"undefined is not iterable (cannot read property Symbol(Symbol.iterator))");
    return context.get(iterable,Key(context.intrinsics().symbolIterator));
}
Key index_key(std::uint64_t index) {
    const auto ascii=std::to_string(index);
    return Key(std::u16string(ascii.begin(),ascii.end()));
}
ObjectRef callable(Context& context,const Value& value,std::u16string_view expression) {
    const auto* function=std::get_if<ObjectRef>(&value.data);
    if(!function||context.kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",std::u16string(expression)+u" is not a function");
    return *function;
}
}

Value Context::callMember(Value function,Value receiver,std::span<const Value> arguments,
                          std::u16string_view expression) {
    const Value callableProperty=get(function,Key(u"call"));
    const ObjectRef callFunction=callable(*this,callableProperty,expression);
    std::vector<Value> callArguments;
    callArguments.reserve(arguments.size()+1);
    callArguments.push_back(receiver);
    callArguments.insert(callArguments.end(),arguments.begin(),arguments.end());
    return call(callFunction,function,callArguments);
}

ObjectRef Context::nativeIterator(std::function<Value(Context&)> next) {
    if(!next) throw Exception("TypeError",u"Native iterator requires a next operation");
    const ObjectRef iterator=objectWithPrototype(intrinsics().mapIteratorPrototype);
    payload(iterator)=std::move(next);
    return iterator;
}

Value Context::getIterator(Value iterable) {
    // Mirrors TypeScript's emitted __values helper: read @@iterator first,
    // then use the length fallback only when the method is falsy.
    const Value method=read_iterator_method(*this,iterable);
    if(truthy(method)) {
        return callMember(method,iterable,{},u"m.call");
    }
    if(!truthy(iterable))
        throw Exception("TypeError",u"Object is not iterable.");
    const Value length=get(iterable,Key(u"length"));
    if(!std::holds_alternative<double>(length.data))
        throw Exception("TypeError",u"Object is not iterable.");

    auto state=std::make_shared<ArrayLikeIteratorState>(ArrayLikeIteratorState{iterable,0,false});
    // Transpiled __values returns a plain object for its array-like fallback;
    // unlike native Array/String iterators it has no inherited @@iterator.
    const ObjectRef iterator=object();
    const ObjectRef next=function(u"next",[state](Context& context,Value,std::span<const Value>)->Value {
        Value source=state->source;
        if(!state->exhausted&&truthy(source)) {
            const double currentLength=context.toNumber(context.get(source,Key(u"length")));
            if(static_cast<double>(state->index)>=currentLength) {
                state->source=Undefined{};
                state->exhausted=true;
                source=Undefined{};
            }
        }
        Value value=Undefined{};
        const bool done=!truthy(source);
        if(!done) {
            value=context.get(source,index_key(state->index));
            ++state->index;
        }
        const ObjectRef result=context.object();
        context.define(result,Key(u"value"),Descriptor::data(value));
        context.define(result,Key(u"done"),Descriptor::data(Value(done)));
        return Value(result);
    });
    define(iterator,Key(u"next"),Descriptor::data(Value(next)));
    return Value(iterator);
}

Value Context::iteratorNext(Value iterator) {
    const Value next=get(iterator,Key(u"next"));
    const ObjectRef function=callable(*this,next,u"iterator.next");
    return call(function,iterator);
}

void Context::forEachIterable(Value iterable,const std::function<void(Value)>& body) {
    const Value iterator=getIterator(iterable);
    Value step=Undefined{};
    std::exception_ptr error;
    try {
        step=iteratorNext(iterator);
        while(!truthy(get(step,Key(u"done")))) {
            const Value value=get(step,Key(u"value"));
            body(value);
            step=iteratorNext(iterator);
        }
    } catch(...) {
        error=std::current_exception();
    }
    try {
        if(truthy(step)&&!truthy(get(step,Key(u"done")))) {
            const Value close=get(iterator,Key(u"return"));
            if(truthy(close)) {
                (void)callMember(close,iterator,{},u"_a.call");
            }
        }
    } catch(...) {
        if(!error) error=std::current_exception();
    }
    if(error) std::rethrow_exception(error);
}

Value Context::readIterable(Value iterable,std::optional<std::uint32_t> count) {
    // This is the emitted __read helper, including its unusual no-iterator
    // return-original behavior and IteratorClose precedence.
    const Value method=read_iterator_method(*this,iterable);
    if(!truthy(method)) return iterable;
    const Value iterator=callMember(method,iterable,{},u"m.call");
    const ObjectRef result=array();
    Value last=Undefined{};
    std::exception_ptr error;
    try {
        std::uint32_t read=0;
        while(!count||read<*count) {
            last=iteratorNext(iterator);
            if(truthy(get(last,Key(u"done")))) break;
            const Value value=get(last,Key(u"value"));
            const Value push=get(Value(result),Key(u"push"));
            const ObjectRef pushMethod=callable(*this,push,u"Array.prototype.push");
            const std::array<Value,1> args{value};
            (void)call(pushMethod,Value(result),args);
            ++read;
        }
    } catch(...) {
        error=std::current_exception();
    }
    try {
        if(truthy(last)&&!truthy(get(last,Key(u"done")))) {
            const Value close=get(iterator,Key(u"return"));
            if(truthy(close)) {
                (void)callMember(close,iterator,{},u"m.call");
            }
        }
    } catch(...) {
        if(!error) error=std::current_exception();
    }
    if(error) std::rethrow_exception(error);
    return Value(result);
}
}
