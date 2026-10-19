export module folia.math.input.tex.node_factory;
export import folia.math.core.mml_tree.mml_factory;
import std;

export namespace folia::math::tex {
class NodeFactory final {
    mml::MmlFactory& mmlFactory_;

    static bool isTokenKind(mml::NodeKind) noexcept;

public:
    explicit NodeFactory(mml::MmlFactory& factory) noexcept : mmlFactory_(factory) {}
    NodeFactory(NodeFactory const&) = delete;
    NodeFactory& operator=(NodeFactory const&) = delete;
    NodeFactory(NodeFactory&&) = delete;
    NodeFactory& operator=(NodeFactory&&) = delete;


    mml::MmlFactory& mmlFactory() const noexcept { return mmlFactory_; }
    mml::MmlNode& makeNode(
        mml::NodeKind kind,
        std::span<mml::MmlNode* const> children = {},
        mml::AttributeSet attributes = {}) const;
    mml::MmlNode& makeToken(
        mml::NodeKind kind,
        std::u16string_view text,
        mml::AttributeSet attributes = {}) const;
    mml::MmlNode& makeText(std::u16string_view text) const;
    mml::MmlNode& makeError(std::u16string_view message) const;
};
}
