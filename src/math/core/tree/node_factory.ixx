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

    // Native Context method adapter. The callback runs only after the actual
    // Source creator has been resolved, and its returned argument vector is
    // consumed by that captured creator without a second registry lookup.
    template<class BuildArguments>
    Product* createWithArguments(std::string_view kind,BuildArguments&& buildArguments) {
        return this->createDirectWithArguments(kind,std::forward<BuildArguments>(buildArguments));
    }

    // Use when raw Source argument values are already available and native
    // conversion must wait until the captured target is known callable.
    template<class EvaluateRawArguments,class ConvertArguments>
    Product* createWithConvertedArguments(std::string_view kind,
        EvaluateRawArguments&& evaluateRawArguments,ConvertArguments&& convertArguments) {
        return this->createDirectWithArgumentPhases(kind,
            std::forward<EvaluateRawArguments>(evaluateRawArguments),
            std::forward<ConvertArguments>(convertArguments));
    }

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
