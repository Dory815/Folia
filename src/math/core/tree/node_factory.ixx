/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium
 * Licensed under the Apache License, Version 2.0.
 * The upstream MathJax source is Apache-2.0 licensed.
 * Port of MathJax ts/core/Tree/NodeFactory.ts (4.1.3).
 *************************************************************/
export module folia.math.core.tree.node_factory;
export import folia.math.core.tree.factory;
import std;

export namespace folia::math::tree {
template<class Product>
class AbstractNodeFactory : public AbstractFactory<Product> {
    using Base = AbstractFactory<Product>;
public:
    using Base::Base;

    Product* create(std::string_view kind, FactoryArgs args) override {
        std::array<std::any, 2> normalized;
        normalized[0] = args.size() > 0 && args[0].has_value()
            ? args[0] : std::any(PropertyList{});
        normalized[1] = args.size() > 1 && args[1].has_value()
            ? args[1]
            : std::any(FactoryChildren<Product>(
                  std::make_shared<ChildArray<Product>>()));
        return this->createDirect(kind, FactoryArgs(normalized));
    }

    Product* create(std::string_view kind, PropertyList properties = {},
                    FactoryChildren<Product> children = {}) {
        if (!children) {
            children = std::make_shared<ChildArray<Product>>();
        }
        const std::array<std::any, 2> args{
            std::any(std::move(properties)), std::any(std::move(children))};
        return this->createDirect(kind, FactoryArgs(args));
    }
};
}
