// Context identity bindings for the existing FunctionList/StorageRef graph.
export module folia.math.util.function_list_bindings;
export import folia.math.util.option_functions;
export import folia.math.util.context_property_bindings;
export import folia.math.util.prioritized_list_bindings;
export import folia.math.util.prioritized_list_context_array;
export import folia.math.util.promise_context_bindings;
import std;
export namespace folia::math {
class FunctionListContextBindings {
    struct State;
    std::shared_ptr<std::atomic_bool> ownedExecutionLease_;
    std::shared_ptr<State> state_;
public:
    FunctionListContextBindings(options::Context&,OptionFunctionBindings&);
    FunctionListContextBindings(options::Context&,OptionFunctionBindings&,std::weak_ptr<std::atomic_bool> executionLease);
    FunctionListContextBindings(options::Context&,OptionFunctionBindings&,std::weak_ptr<std::atomic_bool> executionLease,async::QueueRef queue);
    ~FunctionListContextBindings();
    FunctionListContextBindings(const FunctionListContextBindings&)=delete;
    FunctionListContextBindings& operator=(const FunctionListContextBindings&)=delete;
    options::ObjectRef constructor()const;
    options::ObjectRef prototype()const;
    options::ObjectRef bind(const std::shared_ptr<FunctionList>& list);
    std::shared_ptr<FunctionList> resolve(options::ObjectRef object)const;
    std::shared_ptr<FunctionList> make(options::Value definitions=options::Undefined{});
    options::ObjectRef view(const FunctionList::StorageRef& storage);
    using NativeArgumentEncoder = std::function<std::optional<options::Value>(const std::any&)>;
    void addNativeArgumentEncoder(NativeArgumentEncoder encoder);
    PrioritizedListContextBindings& prioritizedLists();
    OptionFunctionBindings& functions()const;
    // Share the same ContextPropertyBindings identity cache used by native
    // callable and item-record adapters at Source boundary crossings.
    options::Value toValue(const Property&)const;
    Property fromValue(options::Value);
    std::function<options::Value(const Property&)> toValueConverter()const;
    std::function<Property(options::Value)> fromValueConverter()const;
    std::function<options::ObjectRef(const std::shared_ptr<FunctionList>&)> bindConverter()const;
    std::function<std::shared_ptr<FunctionList>(options::ObjectRef)> resolveConverter()const;
    // Exposes the existing runtime lease to record projections owned by this
    // binding. It does not extend the runtime lifetime.
    std::weak_ptr<std::atomic_bool> executionLifetime()const;
};
}
