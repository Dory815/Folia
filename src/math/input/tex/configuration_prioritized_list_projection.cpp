// Draft replacement for src/math/input/tex/configuration_prioritized_list_projection.cpp.
// Depends on the shared PrioritizedListItemsAccess API from the FunctionList packet.
module folia.math.input.tex.parser;
import :runtime;
import folia.math.util.options;
import folia.math.util.prioritized_list_bindings;
import folia.math.util.prioritized_list_context_array;
import folia.math.util.js_number;
import folia.math.util.prioritized_list_source_operations;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
using ConfigurationList = PrioritizedList<ConfigurationRef>;
using ConfigurationItems = PrioritizedListItemsAccess<ConfigurationRef>;
using ConfigurationHandle = PrioritizedListItemHandle<ConfigurationRef>;
using ConfigurationSlot = ConfigurationHandle::ContextSlot;
using ConfigurationRecord = PrioritizedListItem<ConfigurationRef>;
using ConfigurationCell = std::shared_ptr<ConfigurationRecord>;
using ConfigurationStorage = ConfigurationList::StorageRef;
using ConfigurationArrayView = PrioritizedListContextArray<ConfigurationRef>;

[[noreturn]] void configurationListTypeError(std::u16string message) {
    throw Exception("TypeError", std::move(message));
}
Value configurationArgument(std::span<const Value> arguments, std::size_t index) {
    return index < arguments.size() ? arguments[index] : Value(Undefined{});
}
std::u16string configurationIndex(double index) {
    const auto digits = js_number::toString(index);
    return {digits.begin(), digits.end()};
}
std::u16string configurationIndex(std::size_t index) {
    return configurationIndex(static_cast<double>(index));
}
bool strictEqual(const Value& left, const Value& right) {
    if (left.data.index() != right.data.index()) return false;
    if (const auto* a = std::get_if<double>(&left.data)) {
        const double b = std::get<double>(right.data);
        return !std::isnan(*a) && !std::isnan(b) && *a == b;
    }
    return left == right;
}

// This access object owns exactly one live Source array identity. When it wraps
// the initial native array, nativeStorage_ is the list's original StorageRef.
class ConfigurationArrayAccess final : public ConfigurationItems,
    public std::enable_shared_from_this<ConfigurationArrayAccess> {
    Context& context_;
    NativeIdentityBindings<ConfigurationRef>& configurations_;
    std::weak_ptr<std::atomic_bool> lease_;
    Value receiver_;
    Value identity_;
    ConfigurationStorage nativeStorage_;
    std::function<ObjectRef(const ConfigurationCell&)> recordObject_;

    void ensureActive() const {
        const auto lease = lease_.lock();
        if (!lease || !lease->load(std::memory_order_acquire))
            throw Exception("TypeError", u"ParserConfiguration runtime has expired");
    }
    Value get(Value object, std::u16string_view key) const {
        ensureActive();
        return context_.get(std::move(object), Key(std::u16string(key)));
    }
    Value callArrayMethod(std::u16string_view method, std::span<const Value> arguments) {
        ensureActive();
        const Value callable = context_.get(identity_, Key(std::u16string(method)));
        return context_.callMember(callable, identity_, arguments,
            u"this.items." + std::u16string(method));
    }
    ObjectRef makeRecord(Value item, Value priority) {
        ensureActive();
        const ObjectRef record = context_.object();
        context_.define(record, Key(u"item"), Descriptor::data(std::move(item)));
        context_.define(record, Key(u"priority"), Descriptor::data(std::move(priority)));
        return record;
    }
    std::optional<Value> identityOf(const ConfigurationRef& item) const {
        return configurations_.find(item);
    }
    ConfigurationSlot slot(Value record, bool present) {
        const std::weak_ptr<std::atomic_bool> lease = lease_;
        Context* context = &context_;
        NativeIdentityBindings<ConfigurationRef>* identities = &configurations_;
        ConfigurationSlot result;
        result.context = context;
        result.record = std::move(record);
        result.lease = lease;
        result.present = present;
        const Value captured = result.record;
        result.readItem = [context, captured]() -> ConfigurationHandle::ItemValue {
            return context->get(captured, Key(u"item"));
        };
        result.readPriority = [context, captured] {
            return context->get(captured, Key(u"priority"));
        };
        result.writeItem = [context, identities, captured](ConfigurationRef item) {
            const auto identity = identities->find(item);
            if (!identity) configurationListTypeError(u"Configuration has no bound Context identity");
            context->set(captured, Key(u"item"), Value(*identity));
        };
        result.writePriority = [context, captured](Value priority) {
            context->set(captured, Key(u"priority"), std::move(priority));
        };
        return result;
    }

public:
    ConfigurationArrayAccess(Context& context,
        NativeIdentityBindings<ConfigurationRef>& configurations,
        std::weak_ptr<std::atomic_bool> lease, Value receiver, Value array, ConfigurationStorage nativeStorage,
        std::function<ObjectRef(const ConfigurationCell&)> recordObject)
        : context_(context), configurations_(configurations), lease_(std::move(lease)), receiver_(std::move(receiver)),
          identity_(std::move(array)), nativeStorage_(std::move(nativeStorage)),
          recordObject_(std::move(recordObject)) {}

    Value identity() const override { ensureActive(); return identity_; }
    ConfigurationItems::StorageRef* nativeStorageSlot() override {
        return nativeStorage_ ? &nativeStorage_ : nullptr;
    }
    std::size_t length() override {
        ensureActive();
        if (nativeStorage_) return nativeStorage_->size();
        const double raw = context_.toNumber(context_.get(identity_, Key(u"length")));
        if (!std::isfinite(raw) || raw <= 0) return 0;
        if (raw > 9007199254740991.0)
            throw Exception("RangeError", u"Invalid array-like length");
        return static_cast<std::size_t>(std::ceil(raw));
    }
    bool doneAfterRead(std::size_t nextIndex) override {
        ensureActive();
        const Value liveLength = context_.get(identity_, Key(u"length"));
        return static_cast<double>(nextIndex) > context_.toNumber(liveLength);
    }
    bool indexBeforeLength(double index) override {
        ensureActive();
        const Value liveLength = context_.get(identity_, Key(u"length"));
        return index < context_.toNumber(liveLength);
    }
    ConfigurationHandle read(std::size_t index) override {
        ensureActive();
        if (nativeStorage_) {
            if (index >= nativeStorage_->size() || !(*nativeStorage_)[index])
                return ConfigurationHandle(slot(Value(Undefined{}), false));
            const auto& cell = (*nativeStorage_)[index];
            if (cell->rawArraySlot) return ConfigurationHandle(slot(cell->rawSlotValue, true));
            (void)recordObject_(cell);
            return ConfigurationHandle(cell);
        }
        const Value record = context_.get(identity_, Key(configurationIndex(index)));
        return ConfigurationHandle(slot(record, context_.hasOwn(identity_, Key(configurationIndex(index)))));
    }
    ConfigurationRef add(ConfigurationRef item, double priority) override {
        ensureActive();
        const auto identity = identityOf(item);
        if (!identity) configurationListTypeError(u"Configuration has no bound Context identity");
        addRaw(*identity, Value(priority));
        return item;
    }
    void remove(const ConfigurationRef& item) override {
        ensureActive();
        const auto identity = identityOf(item);
        if (!identity) configurationListTypeError(u"Configuration has no bound Context identity");
        removeRaw(*identity);
    }
    void addRaw(Value item, Value priority) override {
        ensureActive();
        const auto keepAlive = shared_from_this();
        (void)PrioritizedListSourceOperations::add(context_, receiver_, std::move(item), std::move(priority),
            [keepAlive](Value, Value sourceItem, Value sourcePriority) -> Value {
                const ObjectRef record = keepAlive->makeRecord(std::move(sourceItem), std::move(sourcePriority));
                return Value(record);
            });
    }
    void removeRaw(Value item) override {
        ensureActive();
        [[maybe_unused]] const auto keepAlive = shared_from_this();
        (void)PrioritizedListSourceOperations::remove(context_, receiver_, std::move(item));
    }
    bool less(const Value& left, const Value& right) override {
        ensureActive();
        Value lhs = context_.toPrimitive(left, PrimitiveHint::numberHint);
        Value rhs = context_.toPrimitive(right, PrimitiveHint::numberHint);
        const auto* leftString = std::get_if<std::u16string>(&lhs.data);
        const auto* rightString = std::get_if<std::u16string>(&rhs.data);
        if (leftString && rightString) return *leftString < *rightString;
        const double a = context_.toNumber(lhs);
        const double b = context_.toNumber(rhs);
        return !std::isnan(a) && !std::isnan(b) && a < b;
    }
    void setSlot(std::size_t index, Value value) override {
        ensureActive();
        context_.set(identity_, Key(configurationIndex(index)), std::move(value));
    }
    bool deleteSlot(std::size_t index) override {
        ensureActive();
        const Key key(configurationIndex(index));
        const bool existed = context_.hasOwn(identity_, key);
        context_.erase(std::get<ObjectRef>(identity_.data), key);
        return existed;
    }
    void setLength(Value value) override {
        ensureActive();
        context_.set(identity_, Key(u"length"), std::move(value));
    }
};

class ConfigurationListProjection final : public std::enable_shared_from_this<ConfigurationListProjection> {
    Context& context_;
    PrioritizedListContextBindings& lists_;
    NativeIdentityBindings<ConfigurationRef>& configurations_;
    std::shared_ptr<ConfigurationList> owner_;
    std::weak_ptr<std::atomic_bool> lease_;
    ObjectRef list_{};
    struct RecordEntry { std::weak_ptr<ConfigurationRecord> cell; ObjectRef object{}; };
    struct ArrayEntry {
        ConfigurationStorage storage;
        ObjectRef object{};
        std::weak_ptr<ConfigurationArrayView> view;
    };
    std::vector<RecordEntry> records_;
    std::vector<ArrayEntry> arrays_;

    ConfigurationListProjection(Context& context, PrioritizedListContextBindings& lists,
        NativeIdentityBindings<ConfigurationRef>& configurations,
        std::shared_ptr<ConfigurationList> owner,
        std::weak_ptr<std::atomic_bool> lease)
        : context_(context), lists_(lists), configurations_(configurations),
          owner_(std::move(owner)), lease_(std::move(lease)) {}
    void ensureActive() const {
        const auto lease = lease_.lock();
        if (!lease || !lease->load(std::memory_order_acquire))
            throw Exception("TypeError", u"ParserConfiguration runtime has expired");
    }
    ObjectRef recordObject(const ConfigurationCell& cell) {
        ensureActive();
        if (!cell) configurationListTypeError(u"Configuration priority record is null");
        if (cell->contextBinding) {
            if (const auto* object = std::get_if<ObjectRef>(&cell->contextBinding->record.data)) return *object;
        }
        for (auto& entry : records_) if (auto known = entry.cell.lock(); known == cell) return entry.object;

        const ObjectRef object = context_.object();
        context_.setPayload(object, std::static_pointer_cast<NativeObject>(cell));
        const std::weak_ptr<ConfigurationListProjection> weak = shared_from_this();
        auto itemField = std::make_shared<NativeDataField>(cell,
            [weak, cell] {
                auto self = weak.lock();
                if (!self) configurationListTypeError(u"Configuration record Context has expired");
                self->ensureActive();
                if (cell->hasRawItem) return cell->rawItem;
                const auto identity = self->configurations_.find(cell->item);
                if (!identity) configurationListTypeError(u"Configuration has no bound Context identity");
                return Value(*identity);
            },
            [weak, cell](Value value) {
                auto self = weak.lock();
                if (!self) configurationListTypeError(u"Configuration record Context has expired");
                self->ensureActive();
                cell->rawItem = std::move(value);
                cell->hasRawItem = true;
            });
        auto priorityField = std::make_shared<NativeDataField>(cell,
            [weak, cell] {
                auto self = weak.lock();
                if (!self) configurationListTypeError(u"Configuration record Context has expired");
                self->ensureActive();
                return cell->hasRawPriority ? cell->rawPriority : Value(cell->priority);
            },
            [weak, cell](Value value) {
                auto self = weak.lock();
                if (!self) configurationListTypeError(u"Configuration record Context has expired");
                self->ensureActive();
                cell->rawPriority = std::move(value);
                cell->hasRawPriority = true;
            });
        context_.bindData(object, Key(u"item"), std::move(itemField));
        context_.bindData(object, Key(u"priority"), std::move(priorityField));
        auto binding = std::make_shared<PrioritizedListItemContextBinding<ConfigurationRef>>();
        binding->context = &context_;
        binding->record = Value(object);
        binding->lease = lease_;
        binding->readItem = [weak, cell] {
            auto self = weak.lock();
            if (!self) configurationListTypeError(u"Configuration record Context has expired");
            self->ensureActive();
            if (cell->hasRawItem) return cell->rawItem;
            const auto identity = self->configurations_.find(cell->item);
            if (!identity) configurationListTypeError(u"Configuration has no bound Context identity");
            return Value(*identity);
        };
        binding->readPriority = [weak, cell] {
            auto self = weak.lock();
            if (!self) configurationListTypeError(u"Configuration record Context has expired");
            self->ensureActive();
            return cell->hasRawPriority ? cell->rawPriority : Value(cell->priority);
        };
        binding->writeItem = [weak, cell](ConfigurationRef item) {
            auto self = weak.lock();
            if (!self) configurationListTypeError(u"Configuration record Context has expired");
            self->ensureActive();
            cell->item = std::move(item);
            cell->hasRawItem = false;
        };
        binding->writePriority = [weak, cell](Value value) {
            auto self = weak.lock();
            if (!self) configurationListTypeError(u"Configuration record Context has expired");
            self->ensureActive();
            cell->rawPriority = std::move(value);
            cell->hasRawPriority = true;
        };
        cell->contextBinding = std::move(binding);
        records_.push_back({cell, object});
        return object;
    }
    std::shared_ptr<ConfigurationArrayAccess> arrayAccess(Value identity, ConfigurationStorage nativeStorage = {}) {
        const std::weak_ptr<ConfigurationListProjection> weak = shared_from_this();
        return std::make_shared<ConfigurationArrayAccess>(context_, configurations_, lease_, Value(list_), std::move(identity),
            std::move(nativeStorage), [weak](const ConfigurationCell& cell) {
                auto self = weak.lock();
                if (!self) configurationListTypeError(u"Configuration array Context has expired");
                return self->recordObject(cell);
            });
    }
    ObjectRef arrayObject(const ConfigurationStorage& storage) {
        ensureActive();
        for (auto& entry : arrays_) if (entry.storage == storage) {
            if (auto view = entry.view.lock()) return entry.object;
        }
        const std::weak_ptr<ConfigurationListProjection> weak = shared_from_this();
        ConfigurationArrayView::Callbacks callbacks;
        callbacks.ensureActive = [weak] {
            auto self = weak.lock();
            if (!self) configurationListTypeError(u"Configuration array Context has expired");
            self->ensureActive();
        };
        callbacks.recordObject = [weak](const ConfigurationCell& cell) {
            auto self = weak.lock();
            if (!self) configurationListTypeError(u"Configuration array Context has expired");
            return self->recordObject(cell);
        };
        callbacks.cellFromValue = [weak, storage](const Value& value) -> ConfigurationCell {
            auto self = weak.lock();
            if (!self) configurationListTypeError(u"Configuration array Context has expired");
            self->ensureActive();
            for (const auto& cell : *storage) if (cell) {
                if (cell->rawArraySlot && cell->rawSlotValue == value) return cell;
                if (cell->contextBinding && cell->contextBinding->record == value) return cell;
            }
            return {};
        };
        callbacks.rawCell = [](Value value) {
            auto cell = std::make_shared<ConfigurationRecord>();
            cell->rawArraySlot = true;
            cell->rawSlotValue = std::move(value);
            return cell;
        };
        auto [array, view] = makePrioritizedListContextArray<ConfigurationRef>(context_, storage, std::move(callbacks));
        arrays_.push_back({storage, array, view});
        return array;
    }
    ConfigurationStorage storageForArray(Value value) const {
        const auto* object = std::get_if<ObjectRef>(&value.data);
        if (!object) return {};
        for (const auto& entry : arrays_) if (entry.object == *object) return entry.storage;
        return {};
    }
    void bindArray(const std::shared_ptr<ConfigurationList>& owner, Value value) {
        if (auto active = owner->itemsAccess(); active && active->identity() == value) return;
        owner->bindItemsAccess(arrayAccess(value, storageForArray(value)));
    }
public:
    static ObjectRef bind(Context& context, PrioritizedListContextBindings& lists,
        NativeIdentityBindings<ConfigurationRef>& configurations,
        const std::shared_ptr<ConfigurationList>& owner,
        std::weak_ptr<std::atomic_bool> lease) {
        if (!owner) configurationListTypeError(u"ParserConfiguration configurations owner is null");
        if (const auto existing = lists.findOwner(std::static_pointer_cast<void>(owner))) return *existing;
        auto state = std::shared_ptr<ConfigurationListProjection>(
            new ConfigurationListProjection(context, lists, configurations, owner, std::move(lease)));
        state->ensureActive();
        state->list_ = context.instance(lists.constructor());
        context.payload(state->list_) = owner;

        // Project the same native vector and record cells. Pre-bind StorageRefs,
        // records, and iterators remain attached to these exact cells.
        const auto oldStorage = owner->storageRef();
        if (!oldStorage) configurationListTypeError(u"Configuration list is already Context-backed");
        const Value initialArray(state->arrayObject(oldStorage));
        owner->bindItemsAccess(state->arrayAccess(initialArray, oldStorage));

        auto field = std::make_shared<NativeDataField>(owner,
            [state, owner] {
                state->ensureActive();
                const auto active = owner->itemsAccess();
                if (active && active->nativeStorageSlot()) return Value(state->arrayObject(owner->storageRef()));
                return active ? active->identity() : Value(Undefined{});
            },
            [state, owner](Value next) {
                state->ensureActive();
                state->bindArray(owner, std::move(next));
            });
        context.bindData(state->list_, Key(u"items"), std::move(field));

        PrioritizedListContextBindings::Operations operations;
        operations.add = [state, owner](std::span<const Value> arguments) {
            state->ensureActive();
            const Value item = configurationArgument(arguments, 0);
            Value priority = configurationArgument(arguments, 1);
            if (std::holds_alternative<Undefined>(priority.data)) priority = Value(state->lists_.defaultPriority());
            owner->addRaw(item, priority);
            return item;
        };
        operations.remove = [state, owner, list = state->list_](std::span<const Value> arguments) {
            state->ensureActive();
            owner->removeRaw(configurationArgument(arguments, 0));
            return Value(list);
        };
        operations.items = [state, owner] {
            state->ensureActive();
            const auto active = owner->itemsAccess();
            if (active && active->nativeStorageSlot()) return Value(state->arrayObject(owner->storageRef()));
            return active ? active->identity() : Value(Undefined{});
        };
        operations.setItems = [state, owner](Value next) {
            state->ensureActive();
            state->bindArray(owner, std::move(next));
        };
        lists.registerOwner(state->list_, owner, std::move(operations));
        return state->list_;
    }
};
}

options::ObjectRef bindConfigurationPriorityList(ParserRuntime& runtime,
    const std::shared_ptr<PrioritizedList<ConfigurationRef>>& owner) {
    return ConfigurationListProjection::bind(runtime.context(), runtime.functionLists.prioritizedLists(),
        runtime.configurationIdentities, owner, runtime.executionLifetime());
}
}
