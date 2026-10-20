/*************************************************************
 *
 *  Copyright (c) 2017-2026 The MathJax Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *************************************************************/
/**
 * @file Native typed Node algorithms based on MathJax core/Tree/Node.ts.
 */
export module folia.math.core.tree.node;
import std;
export import folia.math.core.tree.node_types;

export namespace folia::math::tree {
template<class Product, class Kind, class Base = Node<Product, Kind>>
class AbstractNode : public Base {
public:
    using Base::Base;
    virtual ~AbstractNode() = default;

    bool isKind(Kind value) const override {
        return this->treeOwner().nodeIsKind(static_cast<Product const&>(*this), value);
    }
    void setChildren(std::span<Product* const> values) override {
        this->treeOwner().setChildren(static_cast<Product&>(*this), values);
    }
    Product& appendChild(Product& child) override {
        this->treeOwner().append(static_cast<Product&>(*this), child);
        return child;
    }
    Product& replaceChild(Product& replacement, Product& oldChild) override {
        if (!childIndex(oldChild)) return replacement;
        this->treeOwner().replace(static_cast<Product&>(*this), oldChild, replacement);
        return replacement;
    }
    Product& removeChild(Product& child) override {
        if (childIndex(child)) this->treeOwner().remove(child);
        return child;
    }
    std::optional<std::size_t> childIndex(Product const& child) const override {
        const auto values = this->children();
        const auto found = std::find(values.begin(), values.end(), &child);
        if (found == values.end()) return std::nullopt;
        return static_cast<std::size_t>(found - values.begin());
    }
    Product& copy(bool keepIds = false) const override {
        return this->treeOwner().clone(static_cast<Product const&>(*this), keepIds);
    }
    std::vector<Product*> findNodes(Kind value) override {
        std::vector<Product*> found;
        walkTree([&](Product& node) {
            if (node.isKind(value)) found.push_back(&node);
        });
        return found;
    }
    void walkTree(std::function<void(Product&)> const& visit) override {
        visit(static_cast<Product&>(*this));
        for (std::size_t i = 0; i < this->children().size(); ++i)
            if (auto child = this->children()[i]) child->walkTree(visit);
    }
    std::string toString() const override {
        std::string result{this->treeOwner().kindName(this->kind())};
        result += '(';
        bool first = true;
        for (auto* child : this->children()) {
            if (!first) result += ',';
            first = false;
            if (child) result += child->toString();
        }
        result += ')';
        return result;
    }
};

template<class Product, class Kind, class Base = Node<Product, Kind>>
class AbstractEmptyNode : public AbstractNode<Product, Kind, Base> {
public:
    using AbstractNode<Product, Kind, Base>::AbstractNode;
    void setChildren(std::span<Product* const>) override {}
    Product& appendChild(Product& child) override { return child; }
    Product& replaceChild(Product&, Product& oldChild) override { return oldChild; }
    std::optional<std::size_t> childIndex(Product const&) const override { return std::nullopt; }
    void walkTree(std::function<void(Product&)> const& visit) override {
        visit(static_cast<Product&>(*this));
    }
    std::string toString() const override {
        return std::string{this->treeOwner().kindName(this->kind())};
    }
};
}
