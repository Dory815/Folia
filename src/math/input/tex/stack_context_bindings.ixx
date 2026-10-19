// Canonical Context surface for the actual Source Stack owner.
export module folia.math.input.tex.stack_context_bindings;
export import folia.math.input.tex.stack_array_access;
export import folia.math.util.native_identities;
export import folia.math.input.tex.stack;
export import folia.math.util.options;
import std;

export namespace folia::math::tex {
using StackRef = std::shared_ptr<Stack>;

// ParserRuntime supplies conversions through its existing owner registries.
// Prototype callbacks dispatch to the one Stack algorithm; stack arguments
// remain raw Value alternatives until that algorithm reads Source members.
struct StackContextHooks {
    std::function<StackRef(StackFactoryReference, Property, Property)> construct;
    std::function<StackFactoryReference(const StackRef&)> factory;
    std::function<void(const StackRef&, StackFactoryReference)> setFactory;
    std::function<Property(const StackRef&)> environment;
    std::function<void(const StackRef&, Property)> setEnvironment;
    std::function<Property(const StackRef&)> global;
    std::function<void(const StackRef&, Property)> setGlobal;
    std::function<options::Value(const StackFactoryReference&)> factoryToValue;
    std::function<StackFactoryReference(const options::Value&)> factoryFromValue;
    std::function<options::Value(const Property&)> propertyToValue;
    std::function<Property(const options::Value&)> propertyFromValue;
    std::function<StackArrayAccessRef(const StackRef&)> array;
    std::function<void(const StackRef&, StackArrayAccessRef)> replaceArray;
    std::function<StackArrayAccessRef(const options::Value&)> arrayFromValue;
    std::function<StackEntry(const options::Value&)> entryFromValue;
    std::function<options::Value(const StackEntry&)> entryToValue;
    std::function<options::Value(std::string)> stringToValue;
};

enum class StackMemberKind { constructor, method, getter, setter };
struct StackMemberSurface {
    std::u16string_view name;
    std::u16string_view functionName;
    StackMemberKind kind;
    std::uint32_t length;
    bool enumerable;
    bool configurable;
    bool writable;
};
std::span<const StackMemberSurface> sourceStackMemberSurface();
std::span<const std::u16string_view> sourceStackOwnFieldOrder();

class StackContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    StackContextBindings(options::Context&, NativeIdentityBindings<StackRef>&,
                         StackContextHooks, std::weak_ptr<std::atomic_bool> executionLease);
    ~StackContextBindings();
    StackContextBindings(const StackContextBindings&) = delete;
    StackContextBindings& operator=(const StackContextBindings&) = delete;
    options::ObjectRef constructor() const;
    options::ObjectRef prototype() const;
    options::ObjectRef bind(const StackRef& actualOwner);
    StackRef resolve(options::ObjectRef canonicalIdentity) const;
    options::Context& context() const;
};
}
