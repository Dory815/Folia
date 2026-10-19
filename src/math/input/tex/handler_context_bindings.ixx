// Live Context identities for the MapHandler.ts SubHandlers/SubHandler graph.
export module folia.math.input.tex.handler_context_bindings;
export import folia.math.input.tex.map_handler;
export import folia.math.input.tex.token_map_context_bindings;
export import folia.math.util.map_bindings;
export import folia.math.util.context_property_bindings;
export import folia.math.util.function_list_bindings;
import std;

export namespace folia::math::tex {
using SubHandlersRef = std::shared_ptr<SubHandlers>;

class SubHandlerContextBindings {
    struct State;
    std::shared_ptr<std::atomic_bool> ownedExecutionLease_;
    std::shared_ptr<State> state_;
public:
    SubHandlerContextBindings(options::Context&, ContextPropertyBindings&,
        options::MapContextBindings&, TokenMapContextBindings&,
        FunctionListContextBindings&, OptionFunctionBindings&, MapHandler&);
    SubHandlerContextBindings(options::Context&, ContextPropertyBindings&,
        options::MapContextBindings&, TokenMapContextBindings&,
        FunctionListContextBindings&, OptionFunctionBindings&, MapHandler&,
        std::weak_ptr<std::atomic_bool> executionLease);
    ~SubHandlerContextBindings();
    SubHandlerContextBindings(const SubHandlerContextBindings&)=delete;
    SubHandlerContextBindings& operator=(const SubHandlerContextBindings&)=delete;

    // The supplied SubHandlers identity is already owned by ParserRuntime.
    options::ObjectRef bind(const SubHandlersRef&, options::ObjectRef existingIdentity);
    options::ObjectRef bind(const SubHandlerRef&);
    options::ObjectRef subHandlersConstructor()const;
    options::ObjectRef subHandlersPrototype()const;
    options::ObjectRef subHandlerConstructor()const;
    options::ObjectRef subHandlerPrototype()const;
    SubHandlersRef resolveSubHandlers(options::ObjectRef)const;
    SubHandlerRef resolveSubHandler(options::ObjectRef)const;
    options::Context& context()const;
};
}
