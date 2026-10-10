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

        FactoryChildren<Product> defaultChildren = std::make_shared<std::vector<Product*>>();
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
        this->childNodes = std::make_shared<std::vector<Product*>>();
        if (!children) type_error("Cannot iterate null children");
        for (std::size_t index = 0; index < children->size(); ++index) {
            this->appendChild((*children)[index]);
        }
    }

    Product* appendChild(Product* child) override {
        if (!this->childNodes) type_error("Cannot append to null child list");
        this->childNodes->push_back(child);
        if (!child) type_error("Cannot set parent on null child");
        child->parent = static_cast<Product*>(this);
        return child;
    }

    Product* replaceChild(Product* newChild, Product* oldChild) override {
        const auto index = childIndex(oldChild);
        if (index) {
            (*this->childNodes)[*index] = newChild;
            if (!newChild) type_error("Cannot set parent on null child");
            newChild->parent = static_cast<Product*>(this);
            if (!oldChild) type_error("Cannot read parent of null child");
            if (oldChild->parent == static_cast<Product*>(this)) oldChild->parent = nullptr;
        }
        return newChild;
    }

    Product* removeChild(Product* child) override {
        const auto index = childIndex(child);
        if (index) {
            this->childNodes->erase(this->childNodes->begin() + static_cast<std::ptrdiff_t>(*index));
            if (!child) type_error("Cannot clear parent of null child");
            child->parent = nullptr;
        }
        return child;
    }

    std::optional<std::size_t> childIndex(Product* child) const override {
        if (!this->childNodes) type_error("Cannot read child list");
        const auto found = std::find(this->childNodes->begin(), this->childNodes->end(), child);
        if (found == this->childNodes->end()) return std::nullopt;
        return static_cast<std::size_t>(found - this->childNodes->begin());
    }

    Product* copy(bool keepIds = false) override {
        TreeCallGuard<Product> guard;
        (void)keepIds;
        Product* const created = this->factory.create(this->kind(), {});
        auto* const result = dynamic_cast<AbstractNode<Product, Base>*>(created);
        if (!result) type_error("Factory product is not bound to this AbstractNode family");

        result->properties = PropertyList{};
        result->properties.assign(properties);

        const auto children = this->childNodes;
        if (children) {
            for (std::size_t index = 0; index < children->size(); ++index) {
                Product* const child = (*children)[index];
                if (child) result->appendChild(child->copy());
            }
        }
        return created;
    }

    std::vector<Product*> findNodes(std::string_view name) override {
        std::vector<Product*> result;
        walkTree([&](Product* node, const std::any&) {
            if (node->isKind(name)) result.push_back(node);
        });
        return result;
    }

    std::any walkTree(const TreeVisitor<Product>& visit, const std::any& data = {}) override {
        TreeCallGuard<Product> guard;
        visit(static_cast<Product*>(this), data);
        const auto children = this->childNodes;
        if (!children) type_error("Cannot iterate null child list");
        for (std::size_t index = 0; index < children->size(); ++index) {
            Product* const child = (*children)[index];
            if (child) child->walkTree(visit, data);
        }
        return data;
    }

    std::string toString() const override {
        TreeCallGuard<Product> guard;
        const std::string kindName = this->kind();
        const auto children = this->childNodes;
        if (!children) type_error("Cannot join null child list");
        TreeJoinGuard<Product> joinGuard(children.get());
        if (!joinGuard.entered()) return kindName + "()";
        const auto count = children->size();
        std::string result = kindName + "(";
        for (std::size_t index = 0; index < count; ++index) {
            if (index) result.push_back(',');
            if (index < children->size()) {
                if (Product* const child = (*children)[index]) result += child->toString();
            }
        }
        result.push_back(')');
        return result;
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
    Product* appendChild(Product* child) override { return child; }
    Product* replaceChild(Product*, Product* oldChild) override { return oldChild; }
    std::optional<std::size_t> childIndex(Product*) const override { return std::nullopt; }

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
