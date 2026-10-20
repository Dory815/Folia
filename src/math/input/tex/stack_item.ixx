export module folia.math.input.tex.parser:stack_item;
export import :types;
import std;

export namespace folia::math::tex {
class MmlStack {
    std::vector<mml::MmlNode *> nodes_;

  protected:
    explicit MmlStack(std::vector<mml::MmlNode *> nodes = {});
    virtual StackItemFactory &factory() const = 0;
    std::vector<mml::MmlNode *> &nodes() noexcept;
    std::vector<mml::MmlNode *> const &nodes() const noexcept;

  public:
    // An unset owner represents Source's initial empty provenance string only.
    SourceSnapshot startStr;
    std::size_t startI{}, stopI{};
    virtual ~MmlStack() = default;
    MmlStack(MmlStack const &) = delete;
    MmlStack &operator=(MmlStack const &) = delete;

    std::u16string_view sourceAtCreation() const noexcept;
    virtual void Push(std::span<mml::MmlNode *const>);
    virtual mml::MmlNode *Pop() noexcept;
    virtual mml::MmlNode *First() const noexcept;
    virtual void setFirst(mml::MmlNode &);
    virtual mml::MmlNode *Last() const noexcept;
    virtual void setLast(mml::MmlNode &);
    virtual std::vector<mml::MmlNode *> Peek(std::size_t count = 1) const;
    virtual std::size_t Size() const noexcept;
    virtual void Clear() noexcept;
    virtual mml::MmlNode &toMml(bool inferred = true, bool forceRow = false);
    virtual mml::MmlNode &create(mml::NodeKind, std::span<mml::MmlNode *const> = {},
                                 mml::AttributeSet = {});
    virtual mml::MmlNode &createToken(mml::NodeKind, std::u16string_view, mml::AttributeSet = {});
};

struct StackErrorDefinition {
    std::string id;
    std::u16string message;
};
enum class StackErrorKey { end, close, right, middle, stop, sub, sup };

class BaseItem : public MmlStack, public tree::FactoryNode<StackKind> {
    StackItemFactory &factory_;
    EnvironmentScope env_;
    StackGlobal *global_{};

  protected:
    // Explicit because virtual isOpen cannot dispatch to a C++ derived class
    // from the base constructor. Open subclasses pass true, including arrays.
    BaseItem(StackItemFactory &, bool open, std::vector<mml::MmlNode *> nodes = {});
    StackItemFactory &factory() const override;
    virtual std::optional<StackErrorDefinition> getErrors(StackErrorKey) const;

  public:
    ~BaseItem() override = default;
    StackKind kind() const override;
    void setGlobal(StackGlobal &) noexcept;
    StackGlobal &global() const;
    EnvironmentScope environmentScope() const noexcept;
    void setEnv(EnvironmentScope);
    TexEnvironment &env();
    TexEnvironment const &env() const;
    void clearEnv();
    virtual bool copyEnv() const noexcept;
    virtual bool isOpen() const noexcept;
    virtual bool isClose() const noexcept;
    virtual bool isFinal() const noexcept;
    virtual bool isKind(StackKind) const;
    virtual std::optional<std::u16string_view> getName() const noexcept;
    virtual CheckResult checkItem(FrameHandle const &incoming);
    void addLatexItem(mml::MmlNode &, std::u16string_view prefix = {});
    virtual std::string toString() const;
};
} // namespace folia::math::tex
