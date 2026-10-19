/**
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native C++ port of MathJax 4.1.3 core/Tree/WrapperFactory.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 */
export module folia.math.core.tree.wrapper_factory;
export import folia.math.core.tree.factory;
export import folia.math.core.tree.wrapper_types;
import std;

export namespace folia::math::tree {
template<class Product, class Source, class Arguments>
class AbstractWrapperFactory
    : public AbstractFactory<Product, WrapperKind<Product, Source>, Arguments>,
      public WrapperServices<Product, Source> {
    using Kind = WrapperKind<Product, Source>;
    using Base = AbstractFactory<Product, Kind, Arguments>;
public:
    using Base::Base;

    Product& wrap(Source& source, Product* parent = nullptr) override {
        const Kind requested = source.kind();
        return this->create(requested, creationArguments(source, parent));
    }

protected:
    virtual Arguments creationArguments(Source&, Product*) = 0;
};
}
