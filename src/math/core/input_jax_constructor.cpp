// MathJax 4.1.3 core/InputJax.ts constructor; Apache-2.0.
module folia.math.core.input_jax;
import std;
namespace folia::math::core {
void AbstractInputJax::initializeInputJax(options::Value supplied){
    auto& context=bindings_.context();
    // Source's default parameter runs before the constructor body.
    if(std::holds_alternative<options::Undefined>(supplied.data))
        supplied=options::Value(context.object());

    // Capture the Source `CLASS` once. Later callbacks may replace the live
    // instance.constructor, but the options merge uses this original value.
    const options::Value klass=context.get(options::Value(identity_),options::Key(u"constructor"));

    // defaultOptions creates a fresh destination; userOptions mutates that
    // same graph. Do not publish it to this instance until both calls succeed.
    const options::Value destination(context.object());
    const options::Value classOptions=context.get(klass,options::Key(u"OPTIONS"));
    const std::array<options::Value,1> defaults{classOptions};
    const options::Value defaulted=options::defaultOptions(context,destination,defaults);
    const std::array<options::Value,1> userDefinitions{supplied};
    const options::Value initialized=options::userOptions(context,defaulted,userDefinitions);
    options=initialized;

    // Source reads each property only when constructing its corresponding
    // FunctionList. A pre-filter getter/add callback may change postFilters.
    const options::Value pre=context.get(options,options::Key(u"preFilters"));
    preFilters=makeOptionFunctionList(bindings_,pre);
    const options::Value post=context.get(options,options::Key(u"postFilters"));
    postFilters=makeOptionFunctionList(bindings_,post);
}
}
