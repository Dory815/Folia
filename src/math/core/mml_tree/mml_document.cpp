module folia.math.core.mml_tree.mml_document;
import std;

namespace folia::math::mml {
MmlDocument::MmlDocument() : factory_(new MmlFactory(*this)) {}

MmlDocument::~MmlDocument() = default;

MmlFactory &MmlDocument::factory() noexcept {
    return *factory_;
}

MmlFactory const &MmlDocument::factory() const noexcept {
    return *factory_;
}

MmlNode &MmlDocument::adopt(std::unique_ptr<MmlNode> node) {
    if (!node)
        throw std::invalid_argument("cannot adopt an absent MML node");
    requireOwned(*node);
    auto &result = *node;
    arena_.push_back(std::move(node));
    return result;
}

void MmlDocument::requireOwned(const MmlNode &node) const {
    if (&node.document() != this) {
        throw std::invalid_argument("MML node belongs to another document");
    }
}

void MmlDocument::requireAcyclic(const MmlNode &parent, const MmlNode &child) const {
    for (auto *ancestor = &parent; ancestor; ancestor = ancestor->parent_) {
        if (ancestor == &child) {
            throw std::invalid_argument("MML tree cycle");
        }
    }
}

void MmlDocument::detach(MmlNode &node) noexcept {
    if (node.parent_) {
        auto &siblings = node.parent_->children_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), &node), siblings.end());
        node.parent_ = nullptr;
    }
    if (root_ == &node) {
        root_ = nullptr;
    }
}

MmlNode &MmlDocument::make(NodeKind kind) {
    return factory_->allocateRaw(kind);
}

void MmlDocument::setRoot(MmlNode &node) {
    requireOwned(node);
    if (node.parent_) {
        detach(node);
    }
    if (root_ && root_ != &node) {
        root_->parent_ = nullptr;
    }
    root_ = &node;
}

void MmlDocument::reserveChildren(MmlNode &parent, std::size_t count) {
    requireOwned(parent);
    if (count > parent.children_.max_size()) {
        throw std::length_error("MML child capacity exceeds the vector limit");
    }
    parent.children_.reserve(count);
}

void MmlDocument::append(MmlNode &parent, MmlNode &child) {
    insert(parent, parent.children_.size(), child);
}

void MmlDocument::setChildren(MmlNode &parent, std::span<MmlNode *const> children) {
    requireOwned(parent);
    // Snapshot first: callers may pass parent.children() itself.
    std::vector<MmlNode *> prepared(children.begin(), children.end());
    for (std::size_t i = 0; i < prepared.size(); ++i) {
        auto *child = prepared[i];
        if (!child)
            throw std::invalid_argument("MML child cannot be null");
        requireOwned(*child);
        requireAcyclic(parent, *child);
        if (std::find(prepared.begin(), prepared.begin() + i, child) != prepared.begin() + i)
            throw std::invalid_argument("MML children cannot contain duplicates");
        for (std::size_t j = 0; j < i; ++j) {
            // Moving both an ancestor and descendant as siblings is ambiguous.
            for (auto *p = prepared[j]->parent(); p; p = p->parent())
                if (p == child)
                    throw std::invalid_argument("MML children overlap by ancestry");
            for (auto *p = child->parent(); p; p = p->parent())
                if (p == prepared[j])
                    throw std::invalid_argument("MML children overlap by ancestry");
        }
    }
    parent.children_.reserve(prepared.size());
    for (auto *child : parent.children_)
        child->parent_ = nullptr;
    parent.children_.clear();
    for (auto *child : prepared) {
        detach(*child);
        parent.children_.push_back(child);
        child->parent_ = &parent;
    }
}

void MmlDocument::insert(MmlNode &parent, std::size_t index, MmlNode &child) {
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
    const auto current = std::find(parent.children_.begin(), parent.children_.end(), &child);
    const bool existingChild = child.parent_ == &parent && current != parent.children_.end();
    if (!existingChild) {
        parent.children_.reserve(parent.children_.size() + 1);
    }

    if (existingChild) {
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

void MmlDocument::replace(MmlNode &parent, MmlNode &oldChild, MmlNode &replacement) {
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

void MmlDocument::remove(MmlNode &node) {
    requireOwned(node);
    detach(node);
}

void MmlDocument::attachAuxiliary(MmlNode &parent, MmlNode &child) {
    requireOwned(parent);
    requireOwned(child);
    requireAcyclic(parent, child);
    if (child.parent_ || root_ == &child)
        throw std::invalid_argument("MML auxiliary node must be detached");
    child.parent_ = &parent;
}

MmlNode &MmlDocument::clone(MmlNode const &node, bool keepIds) {
    requireOwned(node);
    return node.copy(keepIds);
}

bool MmlDocument::nodeIsKind(MmlNode const &node, NodeKind kind) const {
    requireOwned(node);
    return factory_->nodeIsKind(node, kind);
}

std::string_view MmlDocument::kindName(NodeKind kind) const {
    return nodeKindName(kind);
}

MmlNode &MmlDocument::cloneSubtree(const MmlNode &source) {
    // A clone copies values into this arena and never copies links or ownership.
    auto &clone = make(source.kind());
    clone.text_ = source.text_;
    clone.attributes_ = source.attributes_;
    clone.properties_ = source.properties_;
    clone.state_ = source.state_;
    for (auto *child : source.children_) {
        append(clone, cloneSubtree(*child));
    }
    return clone;
}
} // namespace folia::math::mml
