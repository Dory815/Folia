// Owner-backed enumerable own fields shared by the native TexParser and
// TextParser projections. This module holds Context binding mechanics only;
// each field's value remains in its existing C++ owner.
export module folia.math.input.tex.tex_parser_context_fields;
export import folia.math.util.options;
export import folia.math.util.context_property_bindings;
import std;

export namespace folia::math::tex {
class TexParserContextFields {
    options::Context* context_;
    options::ObjectRef identity_;
    std::shared_ptr<void> owner_;
    std::weak_ptr<std::atomic_bool> lease_;
public:
    TexParserContextFields(options::Context&, options::ObjectRef,
                           std::shared_ptr<void> actualOwner,
                           std::weak_ptr<std::atomic_bool> executionLease);
    void assign(std::u16string_view name, options::Value initial,
                std::function<options::Value()> read,
                std::function<void(options::Value)> write);
};

// Live Context Array view over TextParser's existing std::vector<Property>.
// The vector owns the values; the adapter owns no copy of the list.
class TextParserEnvStackView final : public options::NativeIndexedView {
    ContextPropertyBindings* properties_;
    std::shared_ptr<void> owner_;
    std::shared_ptr<std::vector<Property>> valuesOwner_;
    std::vector<Property>* values_;
    std::weak_ptr<std::atomic_bool> lease_;
public:
    TextParserEnvStackView(ContextPropertyBindings&,
                           std::shared_ptr<void> actualOwner,
                           std::shared_ptr<std::vector<Property>> actualValues,
                           std::weak_ptr<std::atomic_bool> executionLease);
    std::uint32_t length()const override;
    options::Descriptor lengthDescriptor()const override;
    std::optional<options::Descriptor> indexDescriptor(std::uint32_t)const override;
    std::vector<std::uint32_t> ownIndexKeys()const override;
    void defineIndex(std::uint32_t,options::Descriptor)override;
    void defineLength(options::Descriptor)override;
    void deleteIndex(std::uint32_t)override;
};

// One replaceable envStack authority. Initially it publishes a Context Array
// over the parser's native vector; after assignment it owns the exact assigned
// Context Array identity. Old arrays keep their original view/store alive.
class TextParserEnvStackOwner {
    struct VectorArray {
        std::shared_ptr<std::vector<Property>> values;
        options::ObjectRef identity;
    };
    struct ContextArray { options::ObjectRef identity; };
    options::Context* context_;
    ContextPropertyBindings* properties_;
    std::shared_ptr<void> parserOwner_;
    std::weak_ptr<std::atomic_bool> lease_;
    std::variant<VectorArray,ContextArray> active_;
public:
    TextParserEnvStackOwner(options::Context&,ContextPropertyBindings&,
        std::shared_ptr<void> parserOwner,std::weak_ptr<std::atomic_bool> executionLease);
    options::ObjectRef reset();
    options::ObjectRef identity()const;
    void replace(options::Value);
    void push(Property);
    bool empty()const;
    Property back()const;
    Property pop();
};
}
