// One native function identity/body for typed input and utility invocation.
export module folia.math.util.callable_types;
export import folia.math.util.object_types;
export import folia.math.core.tree.property;
import std;
export namespace folia::math {
using FunctionReceiver=std::variant<Missing,NativeObjectRef>;
using FunctionArgs=std::span<const std::any>;
struct Function {
    std::function<std::any(const FunctionReceiver&,FunctionArgs)> call;
};
using FunctionRef=std::shared_ptr<Function>;
}
