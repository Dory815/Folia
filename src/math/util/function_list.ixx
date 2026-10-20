// MathJax 4.1.3 util/FunctionList.ts, synchronous mathematical pipeline.
// Native callback signatures remain fixed at compile time. Apache-2.0.
export module folia.math.util.function_list;
export import folia.math.util.prioritized_list;
import std;

export namespace folia::math {
template<class... Args> class FunctionList : public PrioritizedList<
    std::variant<void (*)(Args...), bool (*)(Args...)>> {
public:
    using VoidCallback = void (*)(Args...);
    using BoolCallback = bool (*)(Args...);
    using Callback = std::variant<VoidCallback, BoolCallback>;
    using Base = PrioritizedList<Callback>;
    using Item = typename Base::Item;
    using Base::remove;
    FunctionList() = default;
    explicit FunctionList(std::span<Item const> entries) { addList(entries); }
    Callback add(Callback callback, double priority = Base::DEFAULTPRIORITY) {
        if (std::visit([](auto function) { return function == nullptr; }, callback))
            throw std::invalid_argument("A native processor needs its compiled callback");
        return Base::add(callback, priority);
    }
    void addList(std::span<Item const> entries) {
        for (auto const& entry : entries) add(entry.item, entry.priority);
    }
    bool execute(Args... arguments) {
        // Copy only the function pointer before invoking. A callback may grow
        // or shrink this same list, including removal of its current entry.
        for (std::size_t index = 0; index < this->items_.size(); ++index) {
            const auto callback = this->items_[index].item;
            const bool continued = std::visit([&](auto function) {
                using Result = std::invoke_result_t<decltype(function), Args...>;
                if constexpr (std::is_void_v<Result>) {
                    function(arguments...);
                    return true;
                } else return function(arguments...);
            }, callback);
            if (!continued) return false;
        }
        return true;
    }
};
}
