// Focused lifetime witness for the exact CommonRuntimeBindings token. The
// factory registry and Context remain alive after the runtime value dies;
// invoking its retained Source method must fail before dereferencing runtime.
import std;
import folia.math.output.common;
import folia.math.util.function_list_bindings;

int main(){
    using namespace folia::math;
    using namespace folia::math::options;

    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    FunctionListContextBindings filters(context,functions);
    util::RuntimeModules modules(context);
    output::common::FontDataContextBindings fonts(context);
    output::common::CommonFactoryContextBindings factories(context);

    std::weak_ptr<void> runtimeLifetime;
    std::shared_ptr<void> retainedFactoryState=factories.lifetimeToken();
    ObjectRef method;
    {
        output::common::CommonRuntimeBindings runtime(modules,fonts,factories,properties);
        runtimeLifetime=runtime.lifetimeToken();
        output::common::CommonJaxClassBindings bindings(functions,filters,runtime);
        const auto actualClass=context.constructor(u"ActualCommon");
        (void)bindings.bindClassDefaults(actualClass);
        const auto prototype=std::get<ObjectRef>(
            context.get(Value(actualClass),Key(u"prototype")).data);
        method=std::get<ObjectRef>(context.get(Value(prototype),Key(u"getBBox")).data);
        if(runtimeLifetime.expired())
            throw Exception("AssertionError",u"CommonRuntimeBindings token started expired");
    }

    if(!runtimeLifetime.expired()||!retainedFactoryState)
        throw Exception("AssertionError",u"Runtime token was not independent of factory state");
    try{
        (void)context.call(method,Value(context.object()));
    }catch(const Exception& error){
        if(error.name=="Unimplemented"&&
           error.message==u"CommonOutputJax source methods have been released")return 0;
        throw;
    }
    throw Exception("AssertionError",u"Expired getBBox callback was not rejected");
}
