/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native C++ port of the generic Node behavior from
 * .reference/MathJax-src/ts/core/Tree/Node.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.core.tree.node;
export import folia.math.core.tree.node_types;
import std;

export namespace folia::math::tree {
template<class Product, class Base = Node<Product>>
class AbstractNode : public Base {
public:
    explicit AbstractNode(FactoryServices<Product>& factory) : Base(factory) {}
    AbstractNode(const AbstractNode&) = delete;
    AbstractNode& operator=(const AbstractNode&) = delete;
    AbstractNode(AbstractNode&&) = delete;
    AbstractNode& operator=(AbstractNode&&) = delete;
    virtual ~AbstractNode() = default;

    void initializeNode(FactoryArgs args = {}) {
        PropertyList defaultProperties;
        const PropertyList* inputProperties = &defaultProperties;
        if (!args.empty() && args[0].has_value()) {
            inputProperties = std::any_cast<PropertyList>(&args[0]);
            if (!inputProperties) type_error("Node properties must be a PropertyList");
        }
        for (const auto& name : inputProperties->keys()) {
            this->setProperty(name, inputProperties->getOwn(name));
        }

        FactoryChildren<Product> defaultChildren = std::make_shared<ChildArray<Product>>();
        FactoryChildren<Product> inputChildren = std::move(defaultChildren);
        if (args.size() > 1 && args[1].has_value()) {
            const auto* children = std::any_cast<FactoryChildren<Product>>(&args[1]);
            if (!children) type_error("Node children must be a child-array handle");
            inputChildren = *children;
        }
        if (!inputChildren) type_error("Cannot read length of null children");
        if (!inputChildren->empty()) this->setChildren(std::move(inputChildren));
    }

    std::string kind() const override { return "unknown"; }

    void setProperty(std::string_view name, Property value) override {
        properties.set(name, std::move(value));
    }

    Property getProperty(std::string_view name) const override {
        return properties.get(name);
    }

    std::vector<std::string> getPropertyNames() const override {
        return properties.keys();
    }

    PropertyList getAllProperties() const override {
        return properties;
    }

    void removeProperty(std::span<const std::string> names) override {
        for (const auto& name : names) properties.erase(name);
    }

    bool isKind(std::string_view name) const override {
        return this->factory.nodeIsKind(static_cast<const Product*>(this), name);
    }

    void setChildren(FactoryChildren<Product> children) override {
        // P23c frozen stub: void setChildren.
        throw FactoryError("Unimplemented",u"Root-frozen sparse Node operation");
    }

    ChildValue<Product> appendChild(ChildValue<Product> child) override {
        // P23c frozen stub: ChildValue<Product> appendChild.
        throw FactoryError("Unimplemented",u"Root-frozen sparse Node operation");
    }

    ChildValue<Product> replaceChild(ChildValue<Product> newChild, ChildValue<Product> oldChild) override {
        // P23c frozen stub: ChildValue<Product> replaceChild.
        throw FactoryError("Unimplemented",u"Root-frozen sparse Node operation");
    }

    ChildValue<Product> removeChild(ChildValue<Product> child) override {
        // P23c frozen stub: ChildValue<Product> removeChild.
        throw FactoryError("Unimplemented",u"Root-frozen sparse Node operation");
    }

    std::optional<std::size_t> childIndex(ChildValue<Product> child) const override {
        // P23c frozen stub: std::optional<std::size_t> childIndex.
        throw FactoryError("Unimplemented",u"Root-frozen sparse Node operation");
    }

    Product* copy(bool keepIds = false) override {
        // P23c frozen stub: Product* copy.
        throw FactoryError("Unimplemented",u"Root-frozen sparse Node operation");
    }

    std::vector<Product*> findNodes(std::string_view name) override {
        std::vector<Product*> result;
        walkTree([&](Product* node, const std::any&) {
            if (node->isKind(name)) result.push_back(node);
        });
        return result;
    }

    std::any walkTree(const TreeVisitor<Product>& visit, const std::any& data = {}) override {
        // P23c frozen stub: std::any walkTree.
        throw FactoryError("Unimplemented",u"Root-frozen sparse Node operation");
    }

    std::string toString() const override {
        // P23c frozen stub: std::string toString.
        throw FactoryError("Unimplemented",u"Root-frozen sparse Node operation");
    }

protected:
    PropertyList properties;

    [[noreturn]] static void type_error(std::string_view message) {
        throw FactoryError("TypeError", std::u16string(message.begin(), message.end()));
    }
};

template<class Product, class Base = Node<Product>>
class AbstractEmptyNode : public AbstractNode<Product, Base> {
    using Parent = AbstractNode<Product, Base>;
public:
    using Parent::Parent;
    AbstractEmptyNode(const AbstractEmptyNode&) = delete;
    AbstractEmptyNode& operator=(const AbstractEmptyNode&) = delete;
    AbstractEmptyNode(AbstractEmptyNode&&) = delete;
    AbstractEmptyNode& operator=(AbstractEmptyNode&&) = delete;

    void setChildren(FactoryChildren<Product>) override {}
    ChildValue<Product> appendChild(ChildValue<Product> child) override { return child; }
    ChildValue<Product> replaceChild(ChildValue<Product>, ChildValue<Product> oldChild) override { return oldChild; }
    std::optional<std::size_t> childIndex(ChildValue<Product>) const override { return std::nullopt; }

    std::any walkTree(const TreeVisitor<Product>& visit, const std::any& data = {}) override {
        TreeCallGuard<Product> guard;
        visit(static_cast<Product*>(this), data);
        return data;
    }

    std::string toString() const override {
        TreeCallGuard<Product> guard;
        return this->kind();
    }
};
}
