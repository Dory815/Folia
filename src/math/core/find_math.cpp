// MathJax 4.1.3 core/FindMath.ts constructor; Apache-2.0.
module folia.math.core.find_math;
import std;
namespace folia::math::core {
void AbstractFindMath::initializeFindMath(options::Value supplied){
    // Source captures `this.constructor` once before reading the class defaults.
    const options::Value klass=context_.get(options::Value(identity_),options::Key(u"constructor"));

    // Evaluate the fresh default destination before the captured CLASS.OPTIONS
    // property access, as in defaultOptions({}, CLASS.OPTIONS).
    const options::Value destination(context_.object());
    const options::Value classOptions=context_.get(klass,options::Key(u"OPTIONS"));
    const std::array<options::Value,1> defaults{classOptions};
    const options::Value defaulted=options::defaultOptions(context_,destination,defaults);
    const std::array<options::Value,1> userDefinitions{supplied};
    const options::Value initialized=options::userOptions(context_,defaulted,userDefinitions);

    // Publish only after both Source merges have completed successfully.
    options_=initialized;
}
}
