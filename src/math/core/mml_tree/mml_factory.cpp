module folia.math.core.mml_tree.mml_factory;
import std;

namespace folia::math::mml {
namespace {
void validateChildren(
    const MmlDocument& document,
    std::span<MmlNode* const> children) {
    for (std::size_t i = 0; i < children.size(); ++i) {
        auto* const child = children[i];
        if (!child) {
            throw std::invalid_argument("MML factory child cannot be null");
        }
        if (&child->document() != &document) {
            throw std::invalid_argument("MML factory child belongs to another document");
        }
        for (std::size_t j = 0; j < i; ++j) {
            auto* const previous = children[j];
            if (previous == child) {
                throw std::invalid_argument("MML factory children cannot contain duplicates");
            }
            for (auto* ancestor = previous; ancestor; ancestor = ancestor->parent()) {
                if (ancestor == child) {
                    throw std::invalid_argument("MML factory children cannot overlap by ancestry");
                }
            }
            for (auto* ancestor = child; ancestor; ancestor = ancestor->parent()) {
                if (ancestor == previous) {
                    throw std::invalid_argument("MML factory children cannot overlap by ancestry");
                }
            }
        }
    }
}

void collectAnyArityChildren(
    std::span<MmlNode* const> children,
    std::vector<MmlNode*>& result) {
    for (auto* child : children) {
        if (child->kind() == NodeKind::inferred_mrow) {
            for (auto* grandchild : child->children()) {
                result.push_back(grandchild);
            }
        } else {
            result.push_back(child);
        }
    }
}

struct ExplicitRowPlan final {
    MmlNode* row;
    std::vector<MmlNode*> children;
};
}

MmlNode& MmlFactory::make(
    NodeKind kind,
    AttributeSet attributes,
    std::span<MmlNode* const> children) const {
    if (nodeKindName(kind).empty()) {
        throw std::invalid_argument("MML factory requires a registered node kind");
    }
    validateChildren(document_, children);

    const auto arity = nodeTraits(kind).arityKind;
    auto& node = document_.make(kind);
    node.attributes() = std::move(attributes);

    if (arity == NodeArityKind::inferred_row) {
        auto& inferredRow = document_.make(NodeKind::inferred_mrow);
        std::vector<MmlNode*> rowChildren;
        rowChildren.reserve(children.size());
        collectAnyArityChildren(children, rowChildren);
        validateChildren(document_, rowChildren);

        document_.reserveChildren(node, 1);
        document_.reserveChildren(inferredRow, rowChildren.size());
        for (auto* child : rowChildren) {
            document_.append(inferredRow, *child);
        }
        document_.append(node, inferredRow);
        return node;
    }

    if (arity == NodeArityKind::any) {
        std::vector<MmlNode*> flattenedChildren;
        flattenedChildren.reserve(children.size());
        collectAnyArityChildren(children, flattenedChildren);
        validateChildren(document_, flattenedChildren);

        document_.reserveChildren(node, flattenedChildren.size());
        for (auto* child : flattenedChildren) {
            document_.append(node, *child);
        }
        return node;
    }

    std::vector<MmlNode*> preparedChildren;
    std::vector<ExplicitRowPlan> explicitRows;
    preparedChildren.reserve(children.size());
    explicitRows.reserve(children.size());
    for (auto* child : children) {
        if (child->kind() != NodeKind::inferred_mrow) {
            preparedChildren.push_back(child);
            continue;
        }

        auto& explicitRow = document_.make(NodeKind::mrow);
        explicitRow.attributes() = child->attributes();
        ExplicitRowPlan plan{&explicitRow, {}};
        plan.children.reserve(child->children().size());
        collectAnyArityChildren(child->children(), plan.children);
        validateChildren(document_, plan.children);
        preparedChildren.push_back(&explicitRow);
        explicitRows.push_back(std::move(plan));
    }
    validateChildren(document_, preparedChildren);

    // Reserve every destination before moving any caller-owned child.
    document_.reserveChildren(node, preparedChildren.size());
    for (const auto& plan : explicitRows) {
        document_.reserveChildren(*plan.row, plan.children.size());
    }
    for (const auto& plan : explicitRows) {
        for (auto* child : plan.children) {
            document_.append(*plan.row, *child);
        }
    }
    for (auto* child : preparedChildren) {
        document_.append(node, *child);
    }
    return node;
}
}
