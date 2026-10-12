// Native ordinary-array forEach primitive needed by actual FindTeX options.
module folia.math.util.option_types;
import std;
namespace folia::math::options {
ObjectRef Context::arrayMap(ObjectRef object,Value callback,Value thisArgument){
    if(kind(object)!=ObjectKind::array)
        throw Exception("TypeError",u"Array method called on incompatible receiver");

    const std::uint32_t length=arrayLength(object);
    const auto* callbackObject=std::get_if<ObjectRef>(&callback.data);
    if(!callbackObject||kind(*callbackObject)!=ObjectKind::function)
        throw Exception("TypeError",u"Array map callback must be callable");

    // ArraySpeciesCreate first observes the receiver's current constructor,
    // even for empty arrays. Only the intrinsic Array and undefined cases are
    // part of this host profile; custom species need their own constructor host.
    const Value constructorValue=get(Value(object),Key(u"constructor"));
    ObjectRef intrinsicArray=intrinsics().arrayConstructor;
    if(!std::holds_alternative<Undefined>(constructorValue.data)){
        if(const auto* constructorObject=std::get_if<ObjectRef>(&constructorValue.data)){
            if(*constructorObject!=intrinsicArray)
                throw Exception("Unimplemented",u"Custom Array species requires a native constructor binding");
        }else{
            throw Exception("TypeError",u"Array constructor is not a constructor");
        }
    }

    const ObjectRef result=array(length);
    const Value receiver(object);
    for(std::uint64_t index=0;index<length;++index){
        const auto ascii=std::to_string(index);
        const Key key(std::u16string(ascii.begin(),ascii.end()));
        if(!has(receiver,key))continue;
        const Value value=get(receiver,key);
        const std::array<Value,3> arguments{
            value,Value(static_cast<double>(index)),receiver
        };
        const Value mapped=call(*callbackObject,thisArgument,arguments);
        define(result,key,Descriptor::data(mapped));
    }
    return result;
}
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
