// Root-owned native receiver identity/lifetime marker, no object evaluation.
export module folia.math.util.object_types;
import std;
export namespace folia::math {
// Shared undefined marker; callables and property values use this one identity.
struct Missing { bool operator==(const Missing&) const = default; };
struct NativeObject {virtual ~NativeObject()=default;};
using NativeObjectRef=std::shared_ptr<NativeObject>;
}
