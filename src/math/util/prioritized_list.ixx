// MathJax 4.1.3 util/PrioritizedList.ts; Apache-2.0.
export module folia.math.util.prioritized_list;
import std;

export namespace folia::math {
template<class T> struct PrioritizedListItem {
    double priority;
    T item;
};

template<class T> class PrioritizedList {
protected:
    std::vector<PrioritizedListItem<T>> items_;
public:
    static constexpr double DEFAULTPRIORITY = 5;
    using Item = PrioritizedListItem<T>;
    // The indexed view sees live insertions/removals without retaining a vector
    // iterator across a callback. Like other native views it borrows its owner.
    class iterator {
        PrioritizedList const* owner_{};
        std::size_t index_{};
    public:
        using value_type = Item;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        iterator() = default;
        iterator(PrioritizedList const& owner, std::size_t index) noexcept
            : owner_(&owner), index_(index) {}
        Item operator*() const { return owner_->items_.at(index_); }
        iterator& operator++() noexcept { ++index_; return *this; }
        iterator operator++(int) noexcept { auto old = *this; ++*this; return old; }
        bool operator==(std::default_sentinel_t) const noexcept {
            return !owner_ || index_ >= owner_->items_.size();
        }
    };
    virtual ~PrioritizedList() = default;
    iterator begin() const noexcept { return iterator(*this, 0); }
    std::default_sentinel_t end() const noexcept { return {}; }
    std::span<Item const> entries() const noexcept { return items_; }
    T add(T item, double priority = DEFAULTPRIORITY) {
        auto index = items_.size();
        while (index && priority < items_[index - 1].priority) --index;
        items_.insert(items_.begin() + static_cast<std::ptrdiff_t>(index), Item{priority, item});
        return item;
    }
    PrioritizedList& remove(T const& item) {
        for (auto index = items_.size(); index; --index) {
            if (items_[index - 1].item == item) {
                items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(index - 1));
                break;
            }
        }
        return *this;
    }
};
}
