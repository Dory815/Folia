export module folia.math.input.tex.node_factory;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::tex {
class NodeKindRegistration {
public:
    virtual ~NodeKindRegistration() = default;
    virtual void addNode(mml::NodeKind kind, mml::MmlNode &node) = 0;
};

class NodeFactory final {
    mml::MmlFactory& mmlFactory_;
    NodeKindRegistration* registration_;

    static bool isTokenKind(mml::NodeKind) noexcept;
    void notifyCreated(mml::NodeKind, mml::MmlNode&) const;

public:
    explicit NodeFactory(mml::MmlFactory& factory,
                         NodeKindRegistration* registration = nullptr) noexcept
        : mmlFactory_(factory), registration_(registration) {}
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
