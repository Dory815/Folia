module folia.math.core.mml_tree.mml_document;
import std;

namespace folia::math::mml {
void MmlDocument::requireOwned(const MmlNode& node) const {
    if (&node.document() != this) {
        throw std::invalid_argument("MML node belongs to another document");
    }
}

void MmlDocument::requireAcyclic(const MmlNode& parent, const MmlNode& child) const {
    for (auto* ancestor = &parent; ancestor; ancestor = ancestor->parent_) {
        if (ancestor == &child) {
            throw std::invalid_argument("MML tree cycle");
        }
    }
}

void MmlDocument::detach(MmlNode& node) noexcept {
    if (node.parent_) {
        auto& siblings = node.parent_->children_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), &node), siblings.end());
        node.parent_ = nullptr;
    }
    if (root_ == &node) {
        root_ = nullptr;
    }
}

MmlNode& MmlDocument::make(NodeKind kind) {
    auto node = std::unique_ptr<MmlNode>(new MmlNode(*this, kind));
    auto& result = *node;
    arena_.push_back(std::move(node));
    return result;
}

void MmlDocument::setRoot(MmlNode& node) {
    requireOwned(node);
    if (node.parent_) {
        detach(node);
    }
    if (root_ && root_ != &node) {
        root_->parent_ = nullptr;
    }
    root_ = &node;
}

void MmlDocument::reserveChildren(MmlNode& parent, std::size_t count) {
    requireOwned(parent);
    if (count > parent.children_.max_size()) {
        throw std::length_error("MML child capacity exceeds the vector limit");
    }
    parent.children_.reserve(count);
}

void MmlDocument::append(MmlNode& parent, MmlNode& child) {
    insert(parent, parent.children_.size(), child);
}

void MmlDocument::insert(MmlNode& parent, std::size_t index, MmlNode& child) {
    requireOwned(parent);
    requireOwned(child);
    if (index > parent.children_.size()) {
        throw std::out_of_range("MML child index");
    }
    if (&parent == &child) {
        throw std::invalid_argument("MML node cannot parent itself");
    }
    requireAcyclic(parent, child);

    // Allocate before detaching so allocation failure leaves the old tree intact.
    if (child.parent_ != &parent) {
        parent.children_.reserve(parent.children_.size() + 1);
    }

    if (child.parent_ == &parent) {
        const auto current = std::find(parent.children_.begin(), parent.children_.end(), &child);
        const auto oldIndex = static_cast<std::size_t>(current - parent.children_.begin());
        parent.children_.erase(current);
        if (oldIndex < index) {
            --index;
        }
    } else {
        detach(child);
    }

    parent.children_.insert(parent.children_.begin() + static_cast<std::ptrdiff_t>(index), &child);
    child.parent_ = &parent;
}

void MmlDocument::replace(MmlNode& parent, MmlNode& oldChild, MmlNode& replacement) {
    requireOwned(parent);
    requireOwned(oldChild);
    requireOwned(replacement);
    auto oldPosition = std::find(parent.children_.begin(), parent.children_.end(), &oldChild);
    if (oldPosition == parent.children_.end()) {
        throw std::invalid_argument("old node is not a child");
    }
    if (&oldChild == &replacement) {
        return;
    }
    requireAcyclic(parent, replacement);

    detach(replacement);
    oldPosition = std::find(parent.children_.begin(), parent.children_.end(), &oldChild);
    *oldPosition = &replacement;
    oldChild.parent_ = nullptr;
    replacement.parent_ = &parent;
}

void MmlDocument::remove(MmlNode& node) {
    requireOwned(node);
    detach(node);
}

MmlNode& MmlDocument::cloneSubtree(const MmlNode& source) {
    // A clone copies values into this arena and never copies links or ownership.
    auto& clone = make(source.kind_);
    clone.text_ = source.text_;
    clone.attributes_ = source.attributes_;
    for (auto* child : source.children_) {
        append(clone, cloneSubtree(*child));
    }
    return clone;
}
}
