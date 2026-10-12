// Native ordinary-array forEach primitive needed by actual FindTeX options.
module folia.math.util.option_types;
import std;
namespace folia::math::options {
void Context::arrayForEach(ObjectRef object,Value callback,Value thisArgument){
    // This is Array.prototype.forEach's ordinary-array profile: length is
    // captured once, while HasProperty/Get and callback effects remain live.
    const ObjectRef array = object;
    if (kind(array) != ObjectKind::array) {
        throw Exception("TypeError",u"Array method called on incompatible receiver");
    }
    const std::uint32_t length = arrayLength(array);

    const auto* callback_object = std::get_if<ObjectRef>(&callback.data);
    if (!callback_object || kind(*callback_object) != ObjectKind::function) {
        throw Exception("TypeError",u"Array forEach callback must be callable");
    }

    const Value receiver(array);
    for (std::uint64_t index = 0; index < length; ++index) {
        const auto ascii = std::to_string(index);
        const Key key(std::u16string(ascii.begin(),ascii.end()));
        if (!has(receiver,key)) continue;

        const Value value = get(receiver,key);
        const std::array<Value,3> arguments{
            value, Value(static_cast<double>(index)), receiver
        };
        (void)call(*callback_object,thisArgument,arguments);
    }
}
}
