// Native ordinary-array forEach primitive needed by actual FindTeX options.
module folia.math.util.option_types;
import std;
namespace folia::math::options {
void Context::arrayForEach(ObjectRef,Value,Value){
    throw Exception("Unimplemented",u"Native ordinary Array.forEach");
}
}
