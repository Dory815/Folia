// Shared NativeIndexedView over the actual PrioritizedList StorageRef.
// This is a candidate module intended for src/math/util/prioritized_list_context_array.ixx.
export module folia.math.util.prioritized_list_context_array;
export import folia.math.util.prioritized_list;
import std;

export namespace folia::math {
template<class T>
class PrioritizedListContextArray final : public options::NativeIndexedView {
public:
    using Cell = std::shared_ptr<PrioritizedListItem<T>>;
    using StorageRef = typename PrioritizedList<T>::StorageRef;
    struct Callbacks {
        std::function<void()> ensureActive;
        std::function<options::ObjectRef(const Cell&)> recordObject;
        std::function<Cell(const options::Value&)> cellFromValue;
        std::function<Cell(options::Value)> rawCell;
    };

    PrioritizedListContextArray(StorageRef storage, Callbacks callbacks)
        : storage_(std::move(storage)), callbacks_(std::move(callbacks)) {
        if (!storage_) throw std::invalid_argument("PrioritizedList Context array has no storage owner");
        if (!callbacks_.ensureActive || !callbacks_.recordObject || !callbacks_.cellFromValue || !callbacks_.rawCell)
            throw std::invalid_argument("PrioritizedList Context array callbacks are incomplete");
    }

    const StorageRef& storage() const { ensureActive(); return storage_; }

    std::uint32_t length() const override {
        ensureActive();
        return checkedLength();
    }
    options::Descriptor lengthDescriptor() const override {
        ensureActive();
        return options::Descriptor::data(options::Value(static_cast<double>(checkedLength())), lengthWritable_, false, false);
    }
    std::optional<options::Descriptor> indexDescriptor(std::uint32_t index) const override {
        ensureActive();
        if (index >= checkedLength() || !(*storage_)[index]) return {};
        const auto& cell = (*storage_)[index];
        if (cell->rawArraySlot) return options::Descriptor::data(cell->rawSlotValue);
        return options::Descriptor::data(options::Value(callbacks_.recordObject(cell)));
    }
    std::vector<std::uint32_t> ownIndexKeys() const override {
        ensureActive();
        const auto size=checkedLength();
        std::vector<std::uint32_t> keys;
        keys.reserve(size);
        for (std::uint32_t index = 0; index < size; ++index)
            if ((*storage_)[index]) keys.push_back(index);
        return keys;
    }
    void defineIndex(std::uint32_t index, options::Descriptor descriptor) override {
        ensureActive();
        if (descriptor.accessor || !descriptor.writable || !descriptor.enumerable || !descriptor.configurable)
            throw options::Exception("Unimplemented", u"PrioritizedList arrays do not support custom indexed descriptors");
        if (index >= checkedLength()) {
            if (!lengthWritable_) throw options::Exception("TypeError", u"Array length is not writable");
            storage_->resize(static_cast<std::size_t>(index) + 1);
        }
        auto cell = callbacks_.cellFromValue(descriptor.value);
        if (!cell) cell = callbacks_.rawCell(std::move(descriptor.value));
        (*storage_)[index] = std::move(cell);
    }
    void defineLength(options::Descriptor descriptor) override {
        ensureActive();
        if (descriptor.accessor || descriptor.enumerable || descriptor.configurable)
            throw options::Exception("TypeError", u"Invalid array length descriptor");
        if (!lengthWritable_ && descriptor.writable)
            throw options::Exception("TypeError", u"Cannot redefine non-writable property");
        const auto* number = std::get_if<double>(&descriptor.value.data);
        if (!number || !std::isfinite(*number) || *number < 0 ||
            *number > static_cast<double>(std::numeric_limits<std::uint32_t>::max()) || std::trunc(*number) != *number)
            throw options::Exception("RangeError", u"Invalid array length");
        storage_->resize(static_cast<std::size_t>(*number));
        lengthWritable_ = descriptor.writable;
    }
    void deleteIndex(std::uint32_t index) override {
        ensureActive();
        if (index < checkedLength()) (*storage_)[index].reset();
    }

private:
    void ensureActive() const { callbacks_.ensureActive(); }
    std::uint32_t checkedLength() const {
        if (storage_->size() > std::numeric_limits<std::uint32_t>::max())
            throw options::Exception("RangeError", u"PrioritizedList array length exceeds the native array limit");
        return static_cast<std::uint32_t>(storage_->size());
    }
    StorageRef storage_;
    Callbacks callbacks_;
    bool lengthWritable_ = true;
};

template<class T>
std::pair<options::ObjectRef, std::shared_ptr<PrioritizedListContextArray<T>>>
makePrioritizedListContextArray(options::Context& context,
    const typename PrioritizedList<T>::StorageRef& storage,
    typename PrioritizedListContextArray<T>::Callbacks callbacks) {
    if (!callbacks.ensureActive)
        throw std::invalid_argument("PrioritizedList Context array callbacks are incomplete");
    callbacks.ensureActive();
    auto view = std::make_shared<PrioritizedListContextArray<T>>(storage, std::move(callbacks));
    return {context.nativeArray(view), std::move(view)};
}
}
