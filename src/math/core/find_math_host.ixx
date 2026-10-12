// Root class registration/allocation binding, no FindMath algorithms.
export module folia.math.core.find_math_host;
export import folia.math.core.find_math;
import std;
export namespace folia::math::core {
class FindMathModule {
    options::ObjectRef abstractClass_;
public:
    explicit FindMathModule(options::Context& context)
        :abstractClass_(context.constructor(u"AbstractFindMath")){
        context.set(abstractClass_,options::Key(u"OPTIONS"),context.object());
    }
    FindMathModule(const FindMathModule&)=delete;
    FindMathModule& operator=(const FindMathModule&)=delete;
    options::ObjectRef abstractClass()const{return abstractClass_;}
};
template<class T,class... Bindings>std::shared_ptr<T> makeFindMath(options::Context& context,
    options::ObjectRef actualConstructor,options::Value supplied,Bindings&&... bindings){
    static_assert(std::is_base_of_v<AbstractFindMath,T>);
    const auto identity=context.instance(actualConstructor);
    auto result=std::make_shared<T>(context,identity,std::forward<Bindings>(bindings)...);
    context.payload(identity)=std::static_pointer_cast<NativeObject>(result);
    result->initializeFindMath(std::move(supplied));
    return result;
}
}
