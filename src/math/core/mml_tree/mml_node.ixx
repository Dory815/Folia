export module folia.math.core.mml_tree.mml_document:node;
export import folia.math.core.mml_tree.node_kind;
export import folia.math.core.mml_tree.attributes;
import std;

export namespace folia::math::mml {
class MmlDocument;

class MmlNode final {
    friend class MmlDocument;

    MmlDocument* owner_;
    NodeKind kind_;
    std::u16string text_;
    AttributeSet attributes_;
    MmlNode* parent_ = nullptr;
    std::vector<MmlNode*> children_;

    MmlNode(MmlDocument&, NodeKind);

public:
    MmlNode(MmlNode const&) = delete;
    MmlNode& operator=(MmlNode const&) = delete;
    MmlNode(MmlNode&&) = delete;
    MmlNode& operator=(MmlNode&&) = delete;

    NodeKind kind() const noexcept { return kind_; }
    std::u16string_view text() const noexcept { return text_; }
    void setText(std::u16string value) { text_ = std::move(value); }
    AttributeSet& attributes() noexcept { return attributes_; }
    const AttributeSet& attributes() const noexcept { return attributes_; }
    MmlNode* parent() const noexcept { return parent_; }
    std::span<MmlNode* const> children() const noexcept { return children_; }
    MmlDocument& document() const noexcept { return *owner_; }
};
}
