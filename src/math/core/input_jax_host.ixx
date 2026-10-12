// Root Source class registration and allocation bindings; no InputJax algorithms.
export module folia.math.core.input_jax_host;
export import folia.math.core.input_jax;
import std;
export namespace folia::math::core {
// One loaded module owner per native runtime/Context. Source static properties
// live only on this class object, shared by all instances of that Source class.
class InputJaxModule {
    options::Context& context_;
    options::ObjectRef abstractClass_;
public:
    explicit InputJaxModule(options::Context& context):context_(context),
        abstractClass_(context.constructor(u"AbstractInputJax")){
        auto defaults=context.object();
        context.set(defaults,options::Key(u"preFilters"),context.array());
        context.set(defaults,options::Key(u"postFilters"),context.array());
        context.set(abstractClass_,options::Key(u"NAME"),u"generic");
        context.set(abstractClass_,options::Key(u"OPTIONS"),defaults);
    }
    InputJaxModule(const InputJaxModule&)=delete;
    InputJaxModule& operator=(const InputJaxModule&)=delete;
    options::ObjectRef abstractClass()const{return abstractClass_;}
};
template<class T>std::shared_ptr<T> makeInputJax(OptionFunctionBindings& bindings,
    options::ObjectRef actualConstructor,options::Value supplied=options::Undefined{}){
    static_assert(std::is_base_of_v<AbstractInputJax,T>);
    auto& context=bindings.context();
    const auto identity=context.instance(actualConstructor);
    auto result=std::make_shared<T>(bindings,identity);
    context.payload(identity)=std::static_pointer_cast<NativeObject>(result);
    result->initializeInputJax(std::move(supplied));
    return result;
}
}
