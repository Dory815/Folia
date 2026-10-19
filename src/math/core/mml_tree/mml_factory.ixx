export module folia.math.core.mml_tree.mml_document:factory;
export import :node;
export import folia.math.core.mml_tree.node_traits;
export import folia.math.core.tree.factory;
import std;

export namespace folia::math::mml {
struct MmlCreationArguments {
    MmlDocument &document;
};

class MmlFactory final : public tree::AbstractFactory<MmlNode, NodeKind, MmlCreationArguments> {
    friend class MmlDocument;
    explicit MmlFactory(MmlDocument &);
    MmlDocument &document_;
    MmlNode &adopt(std::unique_ptr<MmlNode>) override;

  public:
    MmlDocument &document() const noexcept {
        return document_;
    }

    MmlNode &allocateRaw(NodeKind);
    MmlNode &make(NodeKind, AttributeSet attributes = {}, std::span<MmlNode *const> children = {});
};
} // namespace folia::math::mml
