/**
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native C++ port of MathJax 4.1.3 core/Tree/Wrapper.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 */
export module folia.math.core.tree.wrapper;
export import folia.math.core.tree.wrapper_types;
import std;

export namespace folia::math::tree {
template<class Product, class Source>
class AbstractWrapper : public Wrapper<Product, Source> {
    using Base = Wrapper<Product, Source>;
public:
    using Base::Base;

    typename Base::Kind kind() const override { return this->node->kind(); }

    Product& wrap(Source& source, Product* parent = nullptr) override {
        return this->factory_.wrap(source, parent);
    }

    void walkTree(const std::function<void(Product&)>& visitor) override {
        visitor(static_cast<Product&>(*this));
        // Visit before reading the vector, then honor live appends made by
        // this or descendant visitors during the walk.
        for (std::size_t index = 0; index < this->childNodes.size(); ++index) {
            if (auto* child = this->childNodes[index]) child->walkTree(visitor);
        }
    }
};
}
