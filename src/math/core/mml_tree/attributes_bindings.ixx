// Context surface for the canonical MathJax MmlTree Attributes owner.
export module folia.math.core.mml_tree.attributes_bindings;
export import folia.math.core.mml_tree.attributes;
export import folia.math.util.context_property_bindings;
export import folia.math.util.native_identities;
import std;

export namespace folia::math {
class AttributesContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    AttributesContextBindings(options::Context&, OptionFunctionBindings&,
                              ContextPropertyBindings&,
                              std::weak_ptr<std::atomic_bool> executionLifetime);
    ~AttributesContextBindings();
    AttributesContextBindings(const AttributesContextBindings&)=delete;
    AttributesContextBindings& operator=(const AttributesContextBindings&)=delete;
    options::Context& context() const;
    options::ObjectRef constructor() const;
    options::ObjectRef prototype() const;
    options::ObjectRef bind(const std::shared_ptr<Attributes>&);
    std::shared_ptr<Attributes> resolve(options::ObjectRef) const;
    options::Value toValue(const Property&) const;
    Property fromValue(options::Value);
    std::function<options::Value(const Property&)> toValueConverter() const;
    std::function<Property(options::Value)> fromValueConverter() const;
};
}
