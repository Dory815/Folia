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
    ContextPropertyBindings(options::Context&,OptionFunctionBindings&);
    ~ContextPropertyBindings();
    ContextPropertyBindings(const ContextPropertyBindings&)=delete;
    ContextPropertyBindings& operator=(const ContextPropertyBindings&)=delete;
    Property from(options::Value);
    options::Value to(const Property&)const;
};
}
