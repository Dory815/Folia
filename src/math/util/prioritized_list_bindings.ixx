// One Context constructor/prototype for every live native PrioritizedList<T>.
export module folia.math.util.prioritized_list_bindings;
export import folia.math.util.prioritized_list;
export import folia.math.util.option_types;
import std;

export namespace folia::math {
class PrioritizedListContextBindings {
public:
    struct Operations {
        std::function<options::Value(std::span<const options::Value>)> add;
        std::function<options::Value(std::span<const options::Value>)> remove;
        std::function<options::Value()> items;
        std::function<void(options::Value)> setItems;
    };
private:
    struct State;
    std::shared_ptr<std::atomic_bool> ownedExecutionLease_;
    std::shared_ptr<State> state_;
public:
    PrioritizedListContextBindings(options::Context& context);
    PrioritizedListContextBindings(options::Context& context,std::weak_ptr<std::atomic_bool> executionLease);
    ~PrioritizedListContextBindings();
    PrioritizedListContextBindings(const PrioritizedListContextBindings&)=delete;
    PrioritizedListContextBindings& operator=(const PrioritizedListContextBindings&)=delete;
    options::Context& context()const;
    options::ObjectRef constructor()const;
    options::ObjectRef prototype()const;
    double defaultPriority()const;
    void defaultPriority(double value);
    void registerOwner(options::ObjectRef object,std::weak_ptr<void> lifetime,Operations operations);
    bool owns(options::ObjectRef object)const;
    std::optional<options::ObjectRef> findOwner(const std::shared_ptr<void>& owner)const;
    options::Value add(options::Value receiver,std::span<const options::Value> args)const;
    options::Value remove(options::Value receiver,std::span<const options::Value> args)const;
};
}
