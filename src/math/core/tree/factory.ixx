/**
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native C++ port of generic Factory behavior from
 * .reference/MathJax-src/ts/core/Tree/Factory.ts (MathJax 4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 */
export module folia.math.core.tree.factory;
export import folia.math.core.tree.factory_types;
import std;

export namespace folia::math::tree {
template<class Product, class Kind, class Arguments>
class AbstractFactory : public FactoryServices<Product, Kind, Arguments> {
public:
    explicit AbstractFactory(FactoryClassMap<Product, Kind, Arguments> nodes = {},
                             std::optional<Kind> defaultKind = std::nullopt)
        : defaultKind(std::move(defaultKind)) {
        for (auto& [kind, factoryClass] : nodes)
            setNodeClass(std::move(kind), std::move(factoryClass));
    }
    AbstractFactory(const AbstractFactory&) = delete;
    AbstractFactory& operator=(const AbstractFactory&) = delete;
    AbstractFactory(AbstractFactory&&) = delete;
    AbstractFactory& operator=(AbstractFactory&&) = delete;

    Product& create(const Kind& requested, const Arguments& arguments) override {
        auto record = getNodeClass(requested);
        if (!record) {
            if (!defaultKind) throw std::out_of_range("Factory kind is not registered and no default kind is configured");
            record = getNodeClass(*defaultKind);
        }
        if (!record) throw std::out_of_range("Factory default kind is not registered");
        if (!record->construct) throw std::invalid_argument("Factory class has no constructor");
        auto product = record->construct(arguments);
        if (!product) throw std::runtime_error("Factory constructor returned no product");
        return adopt(std::move(product));
    }

    void setNodeClass(Kind kind, FactoryClass<Product, Arguments> value) override {
        auto found = find(kind);
        if (found == classes_.end()) classes_.emplace_back(std::move(kind), std::move(value));
        else found->second = std::move(value);
    }

    std::optional<FactoryClass<Product, Arguments>> getNodeClass(const Kind& kind) const override {
        const auto found = find(kind);
        if (found == classes_.end()) return std::nullopt;
        return found->second;
    }

    void deleteNodeClass(const Kind& kind) override {
        const auto found = find(kind);
        if (found != classes_.end()) classes_.erase(found);
    }

    bool nodeIsKind(const Product& product, const Kind& kind) const override {
        const auto record = getNodeClass(kind);
        if (!record || !record->isInstance) throw std::invalid_argument("Factory class has no instance predicate");
        return record->isInstance(product);
    }

    std::vector<Kind> getKinds() const override {
        std::vector<Kind> result;
        result.reserve(classes_.size());
        for (const auto& [kind, _] : classes_) result.push_back(kind);
        return result;
    }

    Product& adopt(std::unique_ptr<Product>) override = 0;

    std::optional<Kind> defaultKind;

private:
    using Registry = FactoryClassMap<Product, Kind, Arguments>;
    auto find(const Kind& kind) {
        return std::find_if(classes_.begin(), classes_.end(), [&](const auto& item) { return item.first == kind; });
    }
    auto find(const Kind& kind) const {
        return std::find_if(classes_.cbegin(), classes_.cend(), [&](const auto& item) { return item.first == kind; });
    }
    Registry classes_;
};
}
