export module folia.math.core.mml_tree.mml_factory;
export import folia.math.core.mml_tree.mml_document;
export import folia.math.core.mml_tree.node_traits;
import std;

export namespace folia::math::mml {
class MmlFactory final {
    MmlDocument& document_;

public:
    explicit MmlFactory(MmlDocument& document) noexcept : document_(document) {}
    MmlFactory(MmlFactory const&) = delete;
    MmlFactory& operator=(MmlFactory const&) = delete;
    MmlFactory(MmlFactory&&) = delete;
    MmlFactory& operator=(MmlFactory&&) = delete;


    MmlDocument& document() const noexcept { return document_; }
    MmlNode& make(
        NodeKind kind,
        AttributeSet attributes = {},
        std::span<MmlNode* const> children = {}) const;
};
}
