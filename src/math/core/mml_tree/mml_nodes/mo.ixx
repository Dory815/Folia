// Native typed MmlMo declaration. Pinned behavior: MathJax 4.1.3 mo.ts.
export module folia.math.core.mml_tree.mml_nodes.mo;
export import folia.math.core.mml_tree.mml_document;
export import folia.math.core.mml_tree.mo_types;
export import folia.math.core.mml_tree.operator_dictionary;
export import folia.math.core.mml_tree.mo_patterns;
import std;

export namespace folia::math::mml {
class MmlMo : public AbstractMmlTokenNode {
    MoTexClassMode tex_class_mode_{MoTexClassMode::unset};

  protected:
    void copyNativeStateTo(MmlNode &destination, MmlClone &) const override;
    virtual operator_dict::OperatorDef getOperatorDef(std::u16string_view);
    virtual void checkOperatorTable(std::u16string_view);
    virtual std::vector<std::u16string>
    handleExplicitForm(std::span<std::u16string_view const> forms);
    virtual void checkPseudoScripts(std::u16string_view);
    virtual void checkPrimes(std::u16string_view);
    virtual void checkMathAccent(std::u16string_view);
    void inheritAttributes(AttributeList const &, bool display, double level, bool prime) override;

  public:
    explicit MmlMo(MmlDocument &document) : AbstractMmlTokenNode(document, NodeKind::mo) {}

    static AttributeSet defaults();
    static std::span<operator_dict::OperatorEntry const> operatorTable() noexcept;
    static std::span<std::array<double, 2> const> mathSpacing() noexcept;
    static std::vector<std::pair<char32_t, char32_t>> const &remapPrimes();

    static constexpr MoPattern pseudoScripts() noexcept {
        return MoPattern::pseudoScripts;
    }

    static constexpr MoPattern primes() noexcept {
        return MoPattern::primes;
    }

    static constexpr MoPattern opPattern() noexcept {
        return MoPattern::opPattern;
    }

    static constexpr MoPattern mathaccents() noexcept {
        return MoPattern::mathaccents;
    }

    static constexpr MoPattern mathaccentsWithWidth() noexcept {
        return MoPattern::mathaccentsWithWidth;
    }

    void initializeMml(AttributeSet, std::span<MmlNode *const>) override;
    NodeKind kind() const noexcept override;
    bool isEmbellished() const override;
    std::optional<TexClass> texClass() override;
    void setTexClass(std::optional<TexClass>) override;
    std::optional<double> lspace;
    std::optional<double> rspace;

    MoTexClassMode texClassMode() const noexcept {
        return tex_class_mode_;
    }

    virtual MmlNode *coreParent();
    virtual std::u16string coreText(MmlNode *parent);
    bool hasSpacingAttributes() override;
    virtual bool isAccent();
    MmlNode *setTeXclass(MmlNode *) override;
    virtual MmlNode *adjustTeXclass(MmlNode *);
    virtual std::vector<std::u16string> getForms();
    virtual bool isMathAccent(std::optional<std::u16string_view> text = std::nullopt);
    virtual bool isMathAccentWithWidth(std::optional<std::u16string_view> text = std::nullopt);
};
} // namespace folia::math::mml
