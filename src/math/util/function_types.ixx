// Root-frozen typed native callable/argument seam for complete FunctionList.ts.
export module folia.math.util.function_types;
export import folia.math.util.prioritized_list;
export import folia.math.util.promise;
export import folia.math.core.tree.property;
import folia.math.core.tree.factory_types;
import std;
export namespace folia::math {
struct Function;
using FunctionRef=std::shared_ptr<Function>;
using FunctionValue=std::variant<Property,FunctionRef>;
using FunctionListItem=PrioritizedListItem<FunctionValue>;
using FunctionArgs=std::span<const std::any>;
struct Function {std::function<std::any(FunctionListItem&,FunctionArgs)> call;};
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
[[noreturn]] inline void typeError(std::u16string message){throw tree::FactoryError("TypeError",std::move(message));}
inline std::any invoke(FunctionListItem& receiver,FunctionArgs args){
    const auto* function=std::get_if<FunctionRef>(&receiver.item);
    if(!function||!*function||!(*function)->call)
        throw tree::FactoryError("TypeError",u"FunctionList item is not callable");
    // Capture callable before a callback mutates the receiver's item field.
    const FunctionRef captured=*function;
    return captured->call(receiver,args);
}
inline bool isFalse(const std::any& result){
    if(const auto* b=std::any_cast<bool>(&result))return !*b;
    if(const auto* p=std::any_cast<Property>(&result))return *p==Property(false);
    return false;
}
inline async::PromiseRef promise(const std::any& result,const async::QueueRef&queue){
    if(const auto* p=std::any_cast<async::PromiseRef>(&result);p&&*p&&(*p)->belongsTo(queue))return *p;
    return {};
}
}
}
