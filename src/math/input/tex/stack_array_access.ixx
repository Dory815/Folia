// One operational Stack array with native initial storage or an exact Context
// owner. This module owns no parser or proof-tree algorithm.
export module folia.math.input.tex.stack_array_access;
export import folia.math.input.tex.stack_item;
export import folia.math.util.options;
import std;

export namespace folia::math::tex {
using StackArrayStorageValue = std::variant<StackEntry, options::Value>;
using StackArrayScalar = std::variant<Property, options::Value>;

class StackArrayStorage {
public:
    struct Slot {
        StackArrayStorageValue value=StackEntry(Property(Missing{}));
        bool writable=true, enumerable=true, configurable=true, accessor=false;
        std::optional<options::ObjectRef> getter, setter;
    };
private:
    std::vector<std::optional<Slot>> entries_;
    bool lengthWritable_=true;
public:
    std::uint32_t length() const;
    bool lengthWritable()const noexcept{return lengthWritable_;}
    bool has(std::uint32_t index) const noexcept;
    StackArrayStorageValue rawAt(std::uint32_t index) const;
    std::optional<Slot> slotAt(std::uint32_t index)const;
    void rawSet(std::uint32_t index, StackArrayStorageValue value);
    StackArrayStorageValue popRaw();
    void pushRaw(StackArrayStorageValue value);
    void setLength(std::uint32_t length,bool writable=true);
    void defineIndex(std::uint32_t index,options::Descriptor descriptor);
    void deleteIndex(std::uint32_t index) noexcept;
};
using StackArrayStorageRef = std::shared_ptr<StackArrayStorage>;

enum class StackArrayMethodKind { contextCallable, nativePush, nativePop, nativeJoin };
struct StackArrayMethod {
    StackArrayMethodKind kind=StackArrayMethodKind::contextCallable;
    std::u16string name;
    options::Value receiver;
    options::Value callable;
};

// Operations correspond to the actual Stack.ts member expressions. Values
// remain raw across length/index/method calls; Native and Context owners share
// this single algorithm boundary while preserving their own storage semantics.
class StackArrayAccess {
public:
    virtual ~StackArrayAccess() = default;
    virtual std::optional<options::ObjectRef> identity() const noexcept = 0;
    virtual options::Value sourceValue() const = 0;
    virtual options::Value readLength() = 0;
    virtual StackEntry getIndex(const StackArrayScalar& keyExpression) = 0;
    virtual void setIndex(const StackArrayScalar& keyExpression,StackArrayStorageValue value) = 0;
    virtual bool lessThan(const options::Value& left,const StackArrayScalar& right) = 0;
    virtual StackArrayScalar subtract(const options::Value& left,const StackArrayScalar& right) = 0;
    virtual StackArrayMethod getMethod(std::u16string_view name) = 0;
    virtual StackArrayStorageValue callMethod(const StackArrayMethod& method,
        std::span<const StackArrayStorageValue> arguments) = 0;
    virtual std::u16string toStringValue(const StackArrayStorageValue& value) = 0;
    // Native parser convenience operations use this same storage owner.
    virtual void pushEntry(const StackEntry& value) = 0;
    virtual StackEntry popEntry() = 0;
    virtual StackEntry materialize(const StackEntry& value) = 0;
    virtual double number(const StackArrayScalar& value) = 0;
};
using StackArrayAccessRef = std::shared_ptr<StackArrayAccess>;

class NativeStackArrayAccess final : public StackArrayAccess {
    struct State;
    std::shared_ptr<State> state_;
public:
    NativeStackArrayAccess(StackArrayStorageRef,
        std::function<StackEntry(const options::Value&)>,
        std::function<std::string(const StackArrayStorageValue&)>);
    ~NativeStackArrayAccess() override;
    std::optional<options::ObjectRef> identity() const noexcept override;
    options::Value sourceValue() const override;
    options::Value readLength() override;
    StackEntry getIndex(const StackArrayScalar& keyExpression) override;
    void setIndex(const StackArrayScalar& keyExpression,StackArrayStorageValue value) override;
    bool lessThan(const options::Value& left,const StackArrayScalar& right) override;
    StackArrayScalar subtract(const options::Value& left,const StackArrayScalar& right) override;
    StackArrayMethod getMethod(std::u16string_view name) override;
    StackArrayStorageValue callMethod(const StackArrayMethod& method,
        std::span<const StackArrayStorageValue> arguments) override;
    std::u16string toStringValue(const StackArrayStorageValue& value) override;
    void pushEntry(const StackEntry& value) override;
    StackEntry popEntry() override;
    StackEntry materialize(const StackEntry& value) override;
    double number(const StackArrayScalar& value) override;
    StackEntry readEntry(std::uint32_t index);
    const StackArrayStorageRef& storage() const noexcept;
};

class StackArrayIndexedView final : public options::NativeIndexedView {
    struct State;
    std::shared_ptr<State> state_;
public:
    StackArrayIndexedView(StackArrayStorageRef,
        std::function<options::Value(const StackArrayStorageValue&)>);
    ~StackArrayIndexedView() override;
    std::uint32_t length() const override;
    options::Descriptor lengthDescriptor() const override;
    std::optional<options::Descriptor> indexDescriptor(std::uint32_t index) const override;
    std::vector<std::uint32_t> ownIndexKeys() const override;
    void defineIndex(std::uint32_t index,options::Descriptor descriptor) override;
    void defineLength(options::Descriptor descriptor) override;
    void deleteIndex(std::uint32_t index) override;
    const StackArrayStorageRef& storage() const noexcept;
};

// An assigned Context value is retained exactly. It is never validated at the
// field setter; ordinary Context property operations determine when it fails.
class ContextArrayStackAccess final : public StackArrayAccess {
    struct State;
    std::shared_ptr<State> state_;
public:
    ContextArrayStackAccess(options::Context&,options::Value,
        std::function<options::Value(const StackArrayStorageValue&)>,
        std::function<StackEntry(const options::Value&)>,
        std::function<options::Value(const Property&)>,
        std::weak_ptr<std::atomic_bool> executionLease);
    ~ContextArrayStackAccess() override;
    std::optional<options::ObjectRef> identity() const noexcept override;
    options::Value sourceValue() const override;
    options::Value readLength() override;
    StackEntry getIndex(const StackArrayScalar& keyExpression) override;
    void setIndex(const StackArrayScalar& keyExpression,StackArrayStorageValue value) override;
    bool lessThan(const options::Value& left,const StackArrayScalar& right) override;
    StackArrayScalar subtract(const options::Value& left,const StackArrayScalar& right) override;
    StackArrayMethod getMethod(std::u16string_view name) override;
    StackArrayStorageValue callMethod(const StackArrayMethod& method,
        std::span<const StackArrayStorageValue> arguments) override;
    std::u16string toStringValue(const StackArrayStorageValue& value) override;
    void pushEntry(const StackEntry& value) override;
    StackEntry popEntry() override;
    StackEntry materialize(const StackEntry& value) override;
    double number(const StackArrayScalar& value) override;
    StackEntry readEntry(std::uint32_t index);
};
}
