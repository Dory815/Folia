/**
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native C++ storage roles ported from MathJax 4.1.3 core/Tree/Wrapper.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 */
export module folia.math.core.tree.wrapper_types;
export import folia.math.core.tree.factory_types;
import std;

export namespace folia::math::tree {
template<class Product, class Source>
using WrapperKind = std::remove_cvref_t<decltype(std::declval<Source&>().kind())>;

template<class Product, class Source>
class Wrapper : public FactoryNode<WrapperKind<Product, Source>> {
public:
    using Kind = WrapperKind<Product, Source>;
    Wrapper(WrapperServices<Product, Source>& factory, Source& node)
        : node(&node), factory_(factory) {}
    Wrapper(const Wrapper&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;
    Wrapper(Wrapper&&) = delete;
    Wrapper& operator=(Wrapper&&) = delete;

    virtual Product& wrap(Source&, Product* parent = nullptr) = 0;
    virtual void walkTree(const std::function<void(Product&)>& visitor) = 0;

    Source* const node;
    std::vector<Product*> childNodes;
protected:
    WrapperServices<Product, Source>& factory_;
};
}
