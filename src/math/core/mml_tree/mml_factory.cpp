module folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.node_defaults;
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

AttributeSet installDefaults(NodeKind kind, const AttributeSet& supplied) {
    AttributeSet result = nodeDefaultAttributes(kind);

    // Preserve caller-provided data in every layer, with caller entries taking
    // precedence over the installed Source defaults in their respective layer.
    for (const auto& entry : supplied.globalDefaultEntries()) {
        result.setGlobalDefault(entry.first, entry.second);
    }
    for (const auto& entry : supplied.defaultEntries()) {
        result.setDefault(entry.first, entry.second);
    }
    for (const auto& entry : supplied.inheritedEntries()) {
        result.setInherited(entry.first, entry.second);
    }
    for (const auto& entry : supplied.ownEntries()) {
        result.set(entry.first, entry.second);
    }
    return result;
}
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
    auto installedAttributes = installDefaults(kind, attributes);
    auto& node = document_.make(kind);
    node.attributes() = std::move(installedAttributes);
    initializeNodeProperties(node.kind(), node.properties());
    node.state() = initialIntrinsicState(node.kind());

    if (arity == NodeArityKind::inferred_row) {
        document_.reserveChildren(node, 1);
        auto& inferredRow = make(NodeKind::inferred_mrow, {}, children);
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

        auto& explicitRow = make(NodeKind::mrow, child->attributes());
        copyNodeProperties(child->properties(), explicitRow.properties());
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
