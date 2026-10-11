// One native function identity/body for typed input and utility invocation.
export module folia.math.util.callable_types;
export import folia.math.util.object_types;
import std;
export namespace folia::math {
using FunctionReceiver=std::variant<Missing,NativeObjectRef>;
using FunctionArgs=std::span<const std::any>;
class FunctionBody {
    using Callback=std::function<std::any(const FunctionReceiver&,FunctionArgs)>;
    std::shared_ptr<Callback> callback_;
public:
    FunctionBody()=default;
    template<class F> requires (!std::same_as<std::remove_cvref_t<F>,FunctionBody> && std::constructible_from<Callback,F>)
    FunctionBody(F&& callback):callback_(std::make_shared<Callback>(std::forward<F>(callback))) {}
    template<class F> requires (!std::same_as<std::remove_cvref_t<F>,FunctionBody> && std::constructible_from<Callback,F>)
    FunctionBody& operator=(F&& callback){callback_=std::make_shared<Callback>(std::forward<F>(callback));return *this;}
    explicit operator bool()const noexcept{return callback_&&bool(*callback_);}
    std::any operator()(const FunctionReceiver& receiver,FunctionArgs args)const{
        const auto active=callback_;
        if(!active||!*active)throw std::bad_function_call();
        return (*active)(receiver,args);
    }
};
struct Function {FunctionBody call;};
using FunctionRef=std::shared_ptr<Function>;
}
