// Root-owned native receiver identity/lifetime marker, no object evaluation.
export module folia.math.util.object_types;
import std;
export namespace folia::math {
struct NativeObject {virtual ~NativeObject()=default;};
using NativeObjectRef=std::shared_ptr<NativeObject>;
}
