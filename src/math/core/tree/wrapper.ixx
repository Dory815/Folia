/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium
 * Licensed under the Apache License, Version 2.0.
 * The upstream MathJax source is Apache-2.0 licensed.
 * Port of MathJax ts/core/Tree/Wrapper.ts (4.1.3).
 *************************************************************/
export module folia.math.core.tree.wrapper;
export import folia.math.core.tree.wrapper_types;
import std;

export namespace folia::math::tree {
template<class Product, class Source,
         class Base = Wrapper<Product, Source>>
class AbstractWrapper : public Base {
public:
    explicit AbstractWrapper(WrapperServices<Product, Source>& service,
                             Source* source)
        : Base(service, source) {}

    std::string kind() const override {
        if (!this->node) {
            throw FactoryError("TypeError",
                               u"Cannot read properties of null (reading 'kind')");
        }
        return this->node->kind();
    }

    Product* wrap(Source* source, FactoryArgs = {}) override {
        return this->factory.wrap(source, FactoryArgs{});
    }

    std::any walkTree(const TreeVisitor<Product>& visit,
                      const std::any& data = {}) override {
        TreeCallGuard<Product> guard;
        visit(static_cast<Product*>(this), data);
        if (!this->childNodes) return data;

        const FactoryChildren<Product> children = *this->childNodes;
        if (!children) {
            throw FactoryError("TypeError",
                               u"this.childNodes is not iterable");
        }
        for (std::size_t i = 0; i < children->size(); ++i) {
            Product* const child = (*children)[i];
            if (child) child->walkTree(visit, data);
        }
        return data;
    }
};
}
