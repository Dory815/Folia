#pragma once
// Root-owned private representation shared by two units of option_types.
// Include after `module folia.math.util.option_types; import std;`.
namespace folia::math::options {
struct Record {
    ObjectKind kind = ObjectKind::object;
    std::optional<ObjectRef> prototype;
    std::vector<std::pair<Key,Descriptor>> properties;
    NativeCall callable;
    std::any payload;
    std::u16string label;
};
struct ContextState {
    std::uint64_t identity = 0;
    Intrinsics intrinsics;
    std::vector<std::unique_ptr<Record>> objects;
    std::vector<std::optional<std::u16string>> symbols;
};
}
