/**
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native typed Factory and Wrapper services based on MathJax 4.1.3
 * core/Tree/Factory.ts and core/Tree/WrapperFactory.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 */
export module folia.math.core.tree.factory_types;
import std;

export namespace folia::math::tree {
template<class Product, class Arguments> struct FactoryClass {
    std::function<std::unique_ptr<Product>(const Arguments&)> construct;
    std::function<bool(const Product&)> isInstance;
};

template<class Product, class Kind, class Arguments>
using FactoryClassMap = std::vector<std::pair<Kind, FactoryClass<Product, Arguments>>>;

template<class Product, class Kind, class Arguments, class Result = Product&> class FactoryServices {
public:
    virtual ~FactoryServices() = default;
    virtual Result create(const Kind&, const Arguments&) = 0;
    virtual void setNodeClass(Kind, FactoryClass<Product, Arguments>) = 0;
    virtual std::optional<FactoryClass<Product, Arguments>> getNodeClass(const Kind&) const = 0;
    virtual void deleteNodeClass(const Kind&) = 0;
    virtual bool nodeIsKind(const Product&, const Kind&) const = 0;
    virtual std::vector<Kind> getKinds() const = 0;
    virtual Result adopt(std::unique_ptr<Product>) = 0;
};

template<class Product, class Source> class WrapperServices {
public:
    virtual ~WrapperServices() = default;
    virtual Product& wrap(Source&, Product* parent = nullptr) = 0;
};

template<class Kind> class FactoryNode {
public:
    virtual ~FactoryNode() = default;
    virtual Kind kind() const = 0;
};
}
