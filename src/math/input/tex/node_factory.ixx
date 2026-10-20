export module folia.math.input.tex.node_factory;
export import folia.math.input.tex.node_util;
export import folia.math.input.tex.environment;
import std;

export namespace folia::math::tex {
class NodeKindRegistration {
  public:
    virtual ~NodeKindRegistration() = default;
    virtual void addNode(mml::NodeKind kind, mml::MmlNode &node) = 0;
};

// The original factory.configuration collaboration, narrowed to the native
// data actually consumed by configured node creators. The owner is ParseOptions.
class NodeFactoryConfiguration : public NodeKindRegistration {
  public:
    virtual TexEnvironment const &environment() const = 0;
    virtual void addNode(std::u16string_view property, mml::MmlNode &) = 0;
};
class NodeFactory;
using NodeCreator = mml::MmlNode &(*)(NodeFactory &, mml::NodeKind, std::span<mml::MmlNode *const>,
                                      NodeDefinition, mml::MmlNode *text);
using TokenCreator = mml::MmlNode &(*)(NodeFactory &, mml::NodeKind, NodeDefinition,
                                       std::optional<std::u16string_view>);
using TextCreator = mml::MmlNode *(*)(NodeFactory &, std::optional<std::u16string_view>);
using ErrorCreator = mml::MmlNode &(*)(NodeFactory &, std::u16string_view);
struct NodeCreators {
    std::optional<NodeCreator> node;
    std::optional<TokenCreator> token;
    std::optional<TextCreator> text;
    std::optional<ErrorCreator> error;
    void merge(NodeCreators const &);
};

class NodeFactory final {
    mml::MmlFactory *mmlFactory_;
    NodeKindRegistration *registration_;
    NodeFactoryConfiguration *configuration_{};
    NodeCreators creators_;

    static bool isTokenKind(mml::NodeKind) noexcept;
    void notifyCreated(mml::NodeKind, mml::MmlNode &) const;

  public:
    explicit NodeFactory(mml::MmlFactory &factory,
                         NodeKindRegistration *registration = nullptr) noexcept;
    NodeFactory(NodeFactory const &) = delete;
    NodeFactory &operator=(NodeFactory const &) = delete;
    NodeFactory(NodeFactory &&) = delete;
    NodeFactory &operator=(NodeFactory &&) = delete;

    mml::MmlFactory &mmlFactory() const noexcept {
        return *mmlFactory_;
    }
    void setMmlFactory(mml::MmlFactory &factory) noexcept {
        mmlFactory_ = &factory;
    }
    void setConfiguration(NodeFactoryConfiguration &owner) noexcept;
    NodeFactoryConfiguration &configuration() const;
    void setCreators(NodeCreators const &);
    NodeCreators const &creators() const noexcept {
        return creators_;
    }

    static mml::MmlNode &createNode(NodeFactory &, mml::NodeKind, std::span<mml::MmlNode *const>,
                                    NodeDefinition, mml::MmlNode *text);
    static mml::MmlNode &createToken(NodeFactory &, mml::NodeKind, NodeDefinition,
                                     std::optional<std::u16string_view>);
    static mml::MmlNode *createText(NodeFactory &, std::optional<std::u16string_view>);
    static mml::MmlNode &createError(NodeFactory &, std::u16string_view);
    mml::MmlNode &makeNode(mml::NodeKind kind, std::span<mml::MmlNode *const> children = {},
                           NodeDefinition definition = {}, mml::MmlNode *text = nullptr);
    mml::MmlNode &makeToken(mml::NodeKind kind, std::optional<std::u16string_view> text = std::u16string_view{},
                            NodeDefinition definition = {});
    mml::MmlNode *makeText(std::optional<std::u16string_view>);
    mml::MmlNode &makeError(std::u16string_view message);
};
} // namespace folia::math::tex
