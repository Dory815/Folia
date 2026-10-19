#pragma once
// Root-owned private representation shared by two units of option_types.
// Include after `module folia.math.util.option_types; import std;`.
namespace folia::math::options {
struct Record {
    ObjectKind kind = ObjectKind::object;
    std::optional<ObjectRef> prototype;
    std::vector<std::pair<Key,Descriptor>> properties;
    std::vector<std::pair<Key,NativeDataFieldRef>> nativeFields;
    std::vector<std::pair<Key,NativeDataFieldRef>> declaredNativeFields;
    NativeIndexedViewRef nativeIndexedView;
    NativeRecordViewRef nativeRecordView;
    NativeCall callable;
    NativeConstruct construct;
    std::any payload;
    std::vector<std::pair<NativeOwnerPart,NativeOwnerTraceRef>> ownerTraces;
    NativeAttachedOwnerRef attachedOwner;
    bool payloadMutableAliasEscaped = false;
    // A retired record keeps its slot so stale ObjectRefs can never alias a
    // later object. Context::record rejects it before exposing any field.
    bool retired = false;
    std::u16string label;
};
struct ContextState {
    std::uint64_t identity = 0;
    Intrinsics intrinsics;
    std::vector<std::unique_ptr<Record>> objects;
    std::vector<std::optional<std::u16string>> symbols;
    struct ServiceEntry {
        std::type_index type{typeid(void)};
        std::weak_ptr<void> owner;
    };
    // Weak Context-scoped service lookup lets independent facades find one
    // live owner without making Context its lifetime root or creating a cycle.
    std::unordered_map<std::u16string,ServiceEntry> services;
};
}
