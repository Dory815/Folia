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
        // The replacement happens before the source starts iterating its input.
        this->childNodes = std::make_shared<ChildArray<Product>>();
        if (!children) {
            type_error("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        }
        // A JS for-of loop captures the array object, but reads its live length
        // and indexed values as it advances. Get turns holes into Undefined.
        const auto input = std::move(children);
        for (std::size_t index = 0; index < input->size(); ++index) {
            this->appendChild(input->get(index));
        }
    }

    ChildValue<Product> appendChild(ChildValue<Product> child) override {
        const auto target = this->childNodes;
        if (!target) type_error("Cannot read properties of null (reading 'push')");
        target->push_back(child);
        // The array write precedes the JS assignment receiver check.
        Product* const receiver = child.requireNodeForWrite("parent");
        receiver->setParent(ChildValue<Product>(static_cast<Product*>(this)));
        return child;
    }

    ChildValue<Product> replaceChild(ChildValue<Product> newChild, ChildValue<Product> oldChild) override {
        // This virtual call may mutate or replace this->childNodes. The write
        // below must therefore resolve the receiver's current array afterward.
        const auto index = this->childIndex(oldChild);
        if (index) {
            const auto target = this->childNodes;
            if (!target) {
                const std::string key = std::to_string(*index);
                type_error(std::string("Cannot set properties of null (setting '") + key + "')");
            }
            target->set(*index, newChild);

            Product* const newReceiver = newChild.requireNodeForWrite("parent");
            newReceiver->setParent(ChildValue<Product>(static_cast<Product*>(this)));

            // Do not inspect oldChild until after the replacement and new
            // parent write have completed; a nullish old value throws here.
            Product* const oldReceiver = oldChild.requireNode("parent");
            if (oldReceiver->getParent() == ChildValue<Product>(static_cast<Product*>(this))) {
                oldReceiver->setParent(ChildValue<Product>(PropertyNull{}));
            }
        }
        return newChild;
    }

    ChildValue<Product> removeChild(ChildValue<Product> child) override {
        const auto index = this->childIndex(child);
        if (index) {
            const auto target = this->childNodes;
            if (!target) type_error("Cannot read properties of null (reading 'splice')");
            target->splice(static_cast<std::int64_t>(*index), std::size_t{1});

            // Splice remains visible if the source value cannot receive the
            // following parent assignment.
            Product* const receiver = child.requireNodeForWrite("parent");
            receiver->setParent(ChildValue<Product>(PropertyNull{}));
        }
        return child;
    }

    std::optional<std::size_t> childIndex(ChildValue<Product> child) const override {
        const auto children = this->childNodes;
        if (!children) type_error("Cannot read properties of null (reading 'indexOf')");
        return children->indexOf(child);
    }

    Product* copy(bool keepIds = false) override {
        TreeCallGuard<Product> guard;
        (void)keepIds; // Generic Node.copy ignores this extra argument.

        const FactoryArgs noArguments{};
        Product* const node = this->factory.create(this->kind(), noArguments);
        if (!node) type_error("Cannot set properties of null (setting 'properties')");

        // The pinned CJS spread is Object.assign({}, source): make a fresh
        // record, enumerate own keys, and retain nested Property identities.
        PropertyList copiedProperties;
        copiedProperties.assign(this->properties);
        auto* const copiedBase = dynamic_cast<AbstractNode<Product, Base>*>(node);
        if (!copiedBase) {
            type_error("Factory product does not implement this Node property storage");
        }
        copiedBase->properties = std::move(copiedProperties);

        // Capture after factory construction and property copying, as Source
        // does. Iterate this handle with live length and Has/Get hole behavior.
        const auto sourceChildren = this->childNodes;
        if (sourceChildren) {
            for (std::size_t index = 0; index < sourceChildren->size(); ++index) {
                const ChildValue<Product> child = sourceChildren->get(index);
                if (child.isNode()) {
                    Product* const copiedChild = child.requireNode("copy")->copy();
                    node->appendChild(ChildValue<Product>(copiedChild));
                }
            }
        }
        return node;
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

        // Match for-of: the receiver may rebind childNodes in the callback,
        // so capture the array only after visiting this node. Its length and
        // indexed Get remain live during traversal.
        const auto children = this->childNodes;
        if (!children) {
            type_error("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        }
        for (std::size_t index = 0; index < children->size(); ++index) {
            const ChildValue<Product> child = children->get(index);
            if (child.isNode()) child.requireNode("walkTree")->walkTree(visit, data);
        }
        return data;
    }

    std::string toString() const override {
        TreeCallGuard<Product> callGuard;
        const std::string nodeKind = this->kind();
        const auto children = this->childNodes;
        if (!children) {
            type_error("Cannot read properties of null (reading 'join')");
        }

        // Array.join suppresses recursion by array identity, not by node.
        TreeJoinGuard<Product> joinGuard(children.get());
        if (!joinGuard.entered()) return nodeKind + "()";

        const std::size_t length = children->size();
        std::string result = nodeKind;
        result.push_back('(');
        for (std::size_t index = 0; index < length; ++index) {
            if (index != 0) result.push_back(',');
            const ChildValue<Product> child = children->get(index);
            if (child.isNode()) result += child.requireNode("toString")->toString();
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
