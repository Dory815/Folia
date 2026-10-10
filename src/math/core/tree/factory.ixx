/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native C++ port of the generic Factory behavior from
 * .reference/MathJax-src/ts/core/Tree/Factory.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.core.tree.factory;
export import folia.math.core.tree.factory_types;
import std;

export namespace folia::math::tree {
template<class Product>
class AbstractFactory : public FactoryServices<Product> {
public:
    inline static FactoryClassMap<Product> defaultNodes{};
    std::string defaultKind = "unknown";

    explicit AbstractFactory(
        std::optional<FactoryClassMap<Product>> nodes = std::nullopt,
        const FactoryClassMap<Product>* classDefaults = nullptr)
        : node_(std::make_shared<FactoryCreatorObject<Product>>()) {
        FactoryClassMap<Product> initial;
        if (nodes) {
            initial = std::move(*nodes);
        } else if (classDefaults) {
            initial = *classDefaults;
        } else {
            initial = defaultNodes;
        }
        orderInitialMap(initial);
        for (auto& [kind, type] : initial) setNodeClass(std::move(kind), std::move(type));
    }

    AbstractFactory(const AbstractFactory&) = delete;
    AbstractFactory& operator=(const AbstractFactory&) = delete;
    AbstractFactory(AbstractFactory&&) = delete;
    AbstractFactory& operator=(AbstractFactory&&) = delete;

    Product* create(std::string_view kind, FactoryArgs args = {}) override {
        const auto requested = creator(kind);
        if (requested.kind == CreatorLookupKind::creator) return invoke(requested.ref, args);
        if (requested.kind == CreatorLookupKind::non_callable_inherited) {
            type_error("Factory creator is not a native product constructor");
        }
        const auto fallback = creator(defaultKind);
        if (fallback.kind == CreatorLookupKind::creator) return invoke(fallback.ref, args);
        type_error("Factory creator is not callable");
    }

    void setNodeClass(std::string kind, FactoryClassRef<Product> type) override {
        auto mapEntry = findClass(kind);
        if (mapEntry == nodeMap_.end()) nodeMap_.emplace_back(kind, type);
        else mapEntry->second = type;

        auto captured = std::make_shared<FactoryCreator<Product>>();
        captured->call = [this, type = std::move(type)](FactoryArgs args) -> Product* {
            if (!type || !type->construct) type_error("Invalid factory class");
            Product* const product = type->construct(*this, args);
            if (!product) type_error("Factory constructor returned no product");
            return product;
        };

        const auto ownCreator = findCreator(kind);
        if (kind == "__proto__" && ownCreator == node_->entries.end()) {
            node_->prototypeCreator = std::move(captured);
            node_->prototypeAssigned = true;
        } else if (ownCreator == node_->entries.end()) {
            node_->entries.emplace_back(std::move(kind), std::move(captured));
        } else {
            ownCreator->second = std::move(captured);
        }
    }

    FactoryClassRef<Product> getNodeClass(std::string_view kind) const override {
        const auto found = findClass(kind);
        return found == nodeMap_.end() ? FactoryClassRef<Product>{} : found->second;
    }

    void deleteNodeClass(std::string_view kind) override {
        const auto found = findClass(kind);
        if (found != nodeMap_.end()) nodeMap_.erase(found);
        const auto creatorEntry = findCreator(kind);
        if (creatorEntry != node_->entries.end()) node_->entries.erase(creatorEntry);
    }

    bool nodeIsKind(const Product* node, std::string_view kind) const override {
        const auto type = getNodeClass(kind);
        if (!type || !type->isInstance) type_error("Invalid factory class");
        if (!node) return false;
        return type->isInstance(node);
    }

    std::vector<std::string> getKinds() const override {
        std::vector<std::string> kinds;
        kinds.reserve(nodeMap_.size());
        for (const auto& entry : nodeMap_) kinds.push_back(entry.first);
        return kinds;
    }

    Product* adopt(std::unique_ptr<Product> product) override {
        if (!product) type_error("Cannot adopt a null factory product");
        Product* const result = product.get();
        products_.push_back(std::move(product));
        return result;
    }

    FactoryCreatorView<Product> creatorView() const { return node_; }

protected:
    Product* createDirect(std::string_view kind, FactoryArgs args) {
        const auto requested = creator(kind);
        if (requested.kind != CreatorLookupKind::creator) type_error("Factory creator is not callable");
        return invoke(requested.ref, args);
    }

    FactoryClassMap<Product> nodeMap_;
    FactoryCreatorView<Product> node_;

private:
    enum class CreatorLookupKind { absent, null_value, creator, non_callable_inherited };
    struct CreatorLookup {
        CreatorLookupKind kind;
        FactoryCreatorRef<Product> ref;
    };

    std::vector<std::unique_ptr<Product>> products_;

    [[noreturn]] static void type_error(std::string_view message) {
        throw FactoryError("TypeError", std::u16string(message.begin(), message.end()));
    }

    static std::optional<std::uint32_t> array_index(std::string_view key) {
        if (key.empty() || (key.size() > 1 && key.front() == '0')) return std::nullopt;
        std::uint64_t value = 0;
        const auto [end, error] = std::from_chars(key.data(), key.data() + key.size(), value);
        if (error != std::errc{} || end != key.data() + key.size() || value >= std::numeric_limits<std::uint32_t>::max()) return std::nullopt;
        return static_cast<std::uint32_t>(value);
    }

    static void orderInitialMap(FactoryClassMap<Product>& entries) {
        FactoryClassMap<Product> unique;
        unique.reserve(entries.size());
        for (auto& [kind, type] : entries) {
            const auto found = std::find_if(unique.begin(), unique.end(), [&](const auto& entry) { return entry.first == kind; });
            if (found == unique.end()) unique.emplace_back(std::move(kind), std::move(type));
            else found->second = std::move(type);
        }
        std::stable_sort(unique.begin(), unique.end(), [](const auto& left, const auto& right) {
            const auto leftIndex = array_index(left.first);
            const auto rightIndex = array_index(right.first);
            if (leftIndex && rightIndex) return *leftIndex < *rightIndex;
            return leftIndex.has_value() && !rightIndex.has_value();
        });
        entries = std::move(unique);
    }

    static bool isObjectPrototypeMember(std::string_view kind) {
        return kind == "constructor" || kind == "__defineGetter__" || kind == "__defineSetter__" ||
               kind == "hasOwnProperty" || kind == "__lookupGetter__" || kind == "__lookupSetter__" ||
               kind == "isPrototypeOf" || kind == "propertyIsEnumerable" || kind == "toString" ||
               kind == "valueOf" || kind == "toLocaleString";
    }

    auto findClass(std::string_view kind) {
        return std::find_if(nodeMap_.begin(), nodeMap_.end(), [&](const auto& entry) { return entry.first == kind; });
    }
    auto findClass(std::string_view kind) const {
        return std::find_if(nodeMap_.begin(), nodeMap_.end(), [&](const auto& entry) { return entry.first == kind; });
    }
    auto findCreator(std::string_view kind) {
        return std::find_if(node_->entries.begin(), node_->entries.end(), [&](const auto& entry) { return entry.first == kind; });
    }
    auto findCreator(std::string_view kind) const {
        return std::find_if(node_->entries.begin(), node_->entries.end(), [&](const auto& entry) { return entry.first == kind; });
    }

    CreatorLookup creator(std::string_view kind) const {
        const auto own = findCreator(kind);
        if (own != node_->entries.end()) {
            if (!own->second) return {CreatorLookupKind::null_value, {}};
            return {CreatorLookupKind::creator, own->second};
        }
        if (kind == "__proto__") {
            if (node_->prototypeAssigned) {
                if (!node_->prototypeCreator) return {CreatorLookupKind::null_value, {}};
                return {CreatorLookupKind::creator, node_->prototypeCreator};
            }
            return {CreatorLookupKind::non_callable_inherited, {}};
        }
        if (node_->prototypeAssigned && isFunctionPrototypeMember(kind)) {
            return {CreatorLookupKind::non_callable_inherited, {}};
        }
        if (isObjectPrototypeMember(kind)) return {CreatorLookupKind::non_callable_inherited, {}};
        return {CreatorLookupKind::absent, {}};
    }

    static bool isFunctionPrototypeMember(std::string_view kind) {
        return kind == "call" || kind == "apply" || kind == "bind" ||
               kind == "arguments" || kind == "caller";
    }

    static Product* invoke(const FactoryCreatorRef<Product>& creatorRef, FactoryArgs args) {
        if (!creatorRef || !creatorRef->call) type_error("Factory creator is not callable");
        Product* const product = creatorRef->call(args);
        if (!product) type_error("Factory constructor returned no product");
        return product;
    }
};
}
