// Native graph view at the Context/typed Property boundary; no TeX algorithm.
export module folia.math.util.context_property_bindings;
export import folia.math.util.option_functions;
export import folia.math.core.tree.property;
import std;
export namespace folia::math {
class ContextPropertyBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    using PropertyToObject = std::function<std::optional<options::ObjectRef>(const Property&)>;
    using ObjectToProperty = std::function<std::optional<Property>(options::ObjectRef)>;
    ContextPropertyBindings(options::Context&,OptionFunctionBindings&);
    ~ContextPropertyBindings();
    ContextPropertyBindings(const ContextPropertyBindings&)=delete;
    ContextPropertyBindings& operator=(const ContextPropertyBindings&)=delete;
    options::Context& context() const;
    Property from(options::Value);
    options::Value to(const Property&)const;
    // Diagnostic inventory of projection registry entries and still-live storage owners.
    std::array<std::size_t,2> projectionCountsForDiagnostics()const noexcept;
    // Retire only the Context record created as a projection for this exact native storage.
    // Source-origin wrappers and identities returned by registered projections are retained.
    void retireNativeProjectionForStorage(const std::shared_ptr<PropertyStorage>&);
    // Register a canonical projection for one existing native Property graph.
    // The callbacks must return the actual Context object / original Property
    // handle; this facility never copies or fabricates a wrapper object.
    void registerProjection(PropertyToObject, ObjectToProperty);
    // Implements the explicit Source Object.assign({}, source) boundary as a
    // real new Context-owned record, retaining bound nested identities.
    Property copyOwnEnumerableProperties(const Property& source);
};
}
