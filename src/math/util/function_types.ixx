// Root-frozen typed native callable/argument seam for complete FunctionList.ts.
export module folia.math.util.function_types;
export import folia.math.util.callable_types;
export import folia.math.util.prioritized_list;
export import folia.math.util.promise;
export import folia.math.core.tree.property;
import folia.math.core.tree.factory_types;
import std;
export namespace folia::math {
using FunctionValue=std::variant<Property,FunctionRef>;
// Typed boundaries and raw configuration slots retain the same function owner.
bool operator==(const FunctionValue&,const FunctionValue&);
using FunctionListItem=PrioritizedListItem<FunctionValue>;
struct FunctionTuple {
    FunctionValue item=Property(Missing{});
    std::optional<double> priority;
    std::vector<std::any> extra;
};
using FunctionTupleRef=std::shared_ptr<FunctionTuple>;
using FunctionDefinition=std::variant<FunctionValue,FunctionTupleRef>;
using FunctionDefinitions=std::vector<FunctionDefinition>;
using FunctionDefinitionsRef=std::shared_ptr<FunctionDefinitions>;
namespace functions {
FunctionRef reference(const FunctionValue&);
FunctionValue fromProperty(const Property&);
Property toProperty(const FunctionValue&);
[[noreturn]] inline void typeError(std::u16string message){throw tree::FactoryError("TypeError",std::move(message));}
inline std::any invoke(const std::shared_ptr<FunctionListItem>& receiver,FunctionArgs args){
    if(!receiver)typeError(u"FunctionList item record is null");
    const auto function=reference(receiver->item);
    if(!function||!function->call)
        throw tree::FactoryError("TypeError",u"FunctionList item is not callable");
    // Capture callable before a callback mutates the receiver's item field.
    const FunctionRef captured=function;
    const auto body=captured->call;
    return body(FunctionReceiver{std::static_pointer_cast<NativeObject>(receiver)},args);
}
inline bool isFalse(const std::any& result){
    if(const auto* b=std::any_cast<bool>(&result))return !*b;
    if(const auto* p=std::any_cast<Property>(&result))return *p==Property(false);
    if(const auto* value=std::any_cast<options::Value>(&result))if(const auto* b=std::get_if<bool>(&value->data))return !*b;
    return false;
}
inline async::PromiseRef promise(const std::any& result,const async::QueueRef&queue){
    if(const auto* p=std::any_cast<async::PromiseRef>(&result);p&&*p&&(*p)->belongsTo(queue))return *p;
    return {};
}
}
}
