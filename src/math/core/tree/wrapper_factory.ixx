/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium
 * Licensed under the Apache License, Version 2.0.
 * The upstream MathJax source is Apache-2.0 licensed.
 * Port of MathJax ts/core/Tree/WrapperFactory.ts (4.1.3).
 *************************************************************/
export module folia.math.core.tree.wrapper_factory;
export import folia.math.core.tree.factory;
import std;

export namespace folia::math::tree {
template<class Product, class Node>
class AbstractWrapperFactory : public AbstractFactory<Product>, public WrapperServices<Product,Node> {
    using Base = AbstractFactory<Product>;
public:
    using Base::Base;

    Product* wrap(Node* node, FactoryArgs args = {}) override {
        if (!node) {
            throw options::Exception(
                "TypeError", u"Cannot read properties of null (reading 'kind')");
        }
        const std::string kind = node->kind();
        std::vector<std::any> forwarded;
        forwarded.reserve(args.size() + 1);
        forwarded.emplace_back(node);
        forwarded.insert(forwarded.end(), args.begin(), args.end());
        return this->create(kind, FactoryArgs(forwarded));
    }
};
}
