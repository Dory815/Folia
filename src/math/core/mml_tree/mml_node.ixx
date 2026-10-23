export module folia.math.core.mml_tree.mml_document:node;
export import folia.math.core.tree.node;
export import folia.math.core.mml_tree.node_kind;
export import folia.math.core.mml_tree.attributes;
export import folia.math.core.mml_tree.node_properties;
import folia.math.util.instance_count;
import std;

export namespace folia::math::mml {
class MmlDocument;
class MmlFactory;
class MmlClone;

struct XmlObject {
    virtual ~XmlObject() = default;
};

using XmlRef = std::shared_ptr<XmlObject const>;

class XmlAdaptor {
  public:
    virtual ~XmlAdaptor() = default;
    virtual XmlRef clone(XmlRef) const = 0;
    virtual std::string serializeXML(XmlRef) const = 0;
};

struct InheritedAttribute {
    std::u16string name;
    NodeKind origin;
    AttributeValue value;
};

using AttributeList = std::vector<InheritedAttribute>;
using AttributeAdditions = std::vector<AttributeSet::Entry>;

inline constexpr std::array<std::string_view, 8> TEXCLASSNAMES{"ORD",  "OP",    "BIN",   "REL",
                                                               "OPEN", "CLOSE", "PUNCT", "INNER"};
inline constexpr std::array<std::string_view, 18> MATHVARIANTS{"normal",
                                                               "bold",
                                                               "italic",
                                                               "bold-italic",
                                                               "double-struck",
                                                               "fraktur",
                                                               "bold-fraktur",
                                                               "script",
                                                               "bold-script",
                                                               "sans-serif",
                                                               "bold-sans-serif",
                                                               "sans-serif-italic",
                                                               "sans-serif-bold-italic",
                                                               "monospace",
                                                               "inital",
                                                               "tailed",
                                                               "looped",
                                                               "stretched"};
inline constexpr std::array<std::string_view, 4> indentAttributes{
    "indentalign", "indentalignfirst", "indentshift", "indentshiftfirst"};

struct VerifyOptions {
    bool checkArity{true};
    bool checkAttributes{};
    bool checkMathvariants{true};
    bool fullErrors{};
    bool fixMmultiscripts{true};
    bool fixMtables{true};
};

struct InheritanceExclusion {
    NodeKind from;
    NodeKind to;
    std::u16string_view attribute;
};

struct InheritanceStop {
    NodeKind kind;
    std::u16string_view attribute;
};

class MmlNode : public tree::Node<MmlNode, NodeKind> {
    friend class MmlDocument;
    friend class MmlClone;
    [[no_unique_address]] util::InstanceCount<MmlNode> instance_count_;
    MmlDocument *owner_;
    NodeKind kind_;
    std::u16string text_;
    AttributeSet attributes_;
    NodeProperties properties_;
    MmlIntrinsicState state_;

  protected:
    MmlNode(MmlDocument &, NodeKind);
    tree::NodeOwner<MmlNode, NodeKind> &treeOwner() const noexcept override;
    virtual void copyNativeStateTo(MmlNode &destination, MmlClone &) const;

  public:
    MmlNode(MmlNode const &) = delete;
    MmlNode &operator=(MmlNode const &) = delete;
    MmlNode(MmlNode &&) = delete;
    MmlNode &operator=(MmlNode &&) = delete;
    virtual ~MmlNode() = default;

    virtual NodeKind kind() const noexcept;
    bool isKind(NodeKind) const override;
    MmlDocument &document() const noexcept;
    std::u16string_view text() const noexcept;
    void setText(std::u16string);
    AttributeSet &attributes() noexcept;
    AttributeSet const &attributes() const noexcept;
    NodeProperties &properties() noexcept;
    NodeProperties const &properties() const noexcept;
    MmlIntrinsicState &state() noexcept;
    MmlIntrinsicState const &state() const noexcept;

    virtual bool isToken() const = 0;
    virtual bool isEmbellished() const = 0;
    virtual bool isSpacelike() const = 0;
    virtual bool linebreakContainer() const = 0;
    virtual std::string linebreakAlign() const = 0;
    virtual bool isEmpty() const = 0;
    virtual double arity() const = 0;
    virtual bool isInferred() const = 0;
    virtual bool notParent() const = 0;
    virtual MmlNode *Parent() const noexcept = 0;
    virtual std::optional<TexClass> texClass() = 0;
    virtual void setTexClass(std::optional<TexClass>) = 0;
    virtual std::optional<TexClass> prevClass() const noexcept = 0;
    virtual void setPrevClass(std::optional<TexClass>) = 0;
    virtual std::optional<double> prevLevel() const noexcept = 0;
    virtual void setPrevLevel(std::optional<double>) = 0;
    virtual MmlNode *core() = 0;
    virtual MmlNode *coreMO() = 0;
    virtual std::size_t coreIndex() = 0;
    virtual std::optional<std::size_t> childPosition() = 0;
    virtual MmlNode *setTeXclass(MmlNode *) = 0;
    virtual std::string texSpacing() = 0;
    virtual bool hasSpacingAttributes() = 0;
    virtual void initializeMml(AttributeSet attributes, std::span<MmlNode *const> children) = 0;
    virtual void setInheritedAttributes(AttributeList const &attributes = {}, bool display = false,
                                        double level = 0, bool prime = false) = 0;
    virtual void inheritAttributesFrom(MmlNode &) = 0;
    virtual void verifyTree(std::optional<VerifyOptions> const &options = std::nullopt) = 0;
    virtual MmlNode *mError(std::u16string message, VerifyOptions const &,
                            bool shortError = false) = 0;

  protected:
    virtual void inheritAttributes(AttributeList const &, bool, double, bool) = 0;
};

class AbstractMmlNode : public tree::AbstractNode<MmlNode, NodeKind, MmlNode> {
  protected:
    explicit AbstractMmlNode(MmlDocument &, NodeKind);
    void inheritAttributes(AttributeList const &, bool, double, bool) override;
    virtual void setChildInheritedAttributes(AttributeList const &, bool, double, bool);
    virtual void updateTeXclass(MmlNode *);
    virtual void getPrevClass(MmlNode *);
    virtual void verifyAttributes(VerifyOptions const &);
    virtual void verifyChildren(VerifyOptions const &);
    virtual AttributeList addInheritedAttributes(AttributeList const &current,
                                                 AttributeAdditions const &additions) const;

  public:
    static AttributeSet defaults();
    static std::span<InheritanceExclusion const> noInherit();
    static std::span<InheritanceStop const> stopInherit();
    static std::span<std::u16string_view const> alwaysInherit();
    static VerifyOptions verifyDefaults();
    void initializeMml(AttributeSet, std::span<MmlNode *const>) override;
    void setChildren(std::span<MmlNode *const>) override;
    MmlNode &appendChild(MmlNode &) override;
    MmlNode &replaceChild(MmlNode &newChild, MmlNode &oldChild) override;
    MmlNode *setTeXclass(MmlNode *) override;
    bool isToken() const override;
    bool isEmbellished() const override;
    bool isSpacelike() const override;
    bool linebreakContainer() const override;
    std::string linebreakAlign() const override;
    bool isEmpty() const override;
    double arity() const override;
    MmlNode *core() override;
    MmlNode *coreMO() override;
    bool isInferred() const override;
    bool notParent() const override;
    MmlNode *Parent() const noexcept override;
    std::optional<TexClass> texClass() override;
    void setTexClass(std::optional<TexClass>) override;
    std::optional<TexClass> prevClass() const noexcept override;
    void setPrevClass(std::optional<TexClass>) override;
    std::optional<double> prevLevel() const noexcept override;
    void setPrevLevel(std::optional<double>) override;
    std::size_t coreIndex() override;
    std::optional<std::size_t> childPosition() override;
    std::string texSpacing() override;
    bool hasSpacingAttributes() override;
    void setInheritedAttributes(AttributeList const &attributes = {}, bool display = false,
                                double level = 0, bool prime = false) override;
    void inheritAttributesFrom(MmlNode &) override;
    void verifyTree(std::optional<VerifyOptions> const &options = std::nullopt) override;
    MmlNode *mError(std::u16string, VerifyOptions const &, bool shortError = false) override;
    MmlNode &copy(bool keepIds = false) const override;
};

class AbstractMmlTokenNode : public AbstractMmlNode {
  public:
    using AbstractMmlNode::AbstractMmlNode;
    static AttributeSet defaults();
    bool isToken() const override;
    bool isEmpty() const override;
    virtual std::u16string getText() const;
    void walkTree(std::function<void(MmlNode &)> const &) override;

  protected:
    void setChildInheritedAttributes(AttributeList const &, bool, double, bool) override;
};

class AbstractMmlLayoutNode : public AbstractMmlNode {
  public:
    using AbstractMmlNode::AbstractMmlNode;
    static AttributeSet defaults();
    bool isSpacelike() const override;
    bool isEmbellished() const override;
    double arity() const override;
    MmlNode *core() override;
    MmlNode *coreMO() override;
    MmlNode *setTeXclass(MmlNode *) override;
};

class AbstractMmlBaseNode : public AbstractMmlNode {
  public:
    using AbstractMmlNode::AbstractMmlNode;
    static AttributeSet defaults();
    bool isEmbellished() const override;
    MmlNode *core() override;
    MmlNode *coreMO() override;
    MmlNode *setTeXclass(MmlNode *) override;
};

class AbstractMmlEmptyNode : public tree::AbstractEmptyNode<MmlNode, NodeKind, MmlNode> {
  protected:
    explicit AbstractMmlEmptyNode(MmlDocument &, NodeKind);
    void inheritAttributes(AttributeList const &, bool, double, bool) override;

  public:
    void initializeMml(AttributeSet, std::span<MmlNode *const>) override;
    bool isToken() const override;
    bool isEmbellished() const override;
    bool isSpacelike() const override;
    bool linebreakContainer() const override;
    std::string linebreakAlign() const override;
    bool isEmpty() const override;
    double arity() const override;
    bool isInferred() const override;
    bool notParent() const override;
    MmlNode *Parent() const noexcept override;
    std::optional<TexClass> texClass() override;
    void setTexClass(std::optional<TexClass>) override;
    std::optional<TexClass> prevClass() const noexcept override;
    void setPrevClass(std::optional<TexClass>) override;
    std::optional<double> prevLevel() const noexcept override;
    void setPrevLevel(std::optional<double>) override;
    MmlNode *core() override;
    MmlNode *coreMO() override;
    std::size_t coreIndex() override;
    std::optional<std::size_t> childPosition() override;
    MmlNode *setTeXclass(MmlNode *) override;
    std::string texSpacing() override;
    bool hasSpacingAttributes() override;
    void setInheritedAttributes(AttributeList const &attributes = {}, bool display = false,
                                double level = 0, bool prime = false) override;
    void inheritAttributesFrom(MmlNode &) override;
    void verifyTree(std::optional<VerifyOptions> const &options = std::nullopt) override;
    MmlNode *mError(std::u16string, VerifyOptions const &, bool shortError = false) override;
    MmlNode &copy(bool keepIds = false) const override;
};

class TextNode : public AbstractMmlEmptyNode {
  public:
    explicit TextNode(MmlDocument &);
    NodeKind kind() const noexcept override;
    virtual std::u16string getText() const;
    virtual TextNode &setText(std::u16string);
    MmlNode &copy(bool keepIds = false) const override;
    std::string toString() const override;
};

class XMLNode : public AbstractMmlEmptyNode {
  protected:
    XmlRef xml_{};
    std::shared_ptr<XmlAdaptor const> adaptor_{};
    void copyNativeStateTo(MmlNode &destination, MmlClone &) const override;

  public:
    explicit XMLNode(MmlDocument &);
    NodeKind kind() const noexcept override; // NodeKind::xml; nodeKindName supplies "XML"
    virtual XmlRef getXML() const;
    std::shared_ptr<XmlAdaptor const> xmlAdaptor() const noexcept;
    virtual XMLNode *setXML(XmlRef, std::shared_ptr<XmlAdaptor const> adaptor = {});
    virtual std::string getSerializedXML() const;
    MmlNode &copy(bool keepIds = false) const override;
    std::string toString() const override;
};
} // namespace folia::math::mml
