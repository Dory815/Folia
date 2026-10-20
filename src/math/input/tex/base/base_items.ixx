// Declarations for the 26 MathJax 4.1.3 base TeX StackItem subclasses.
// Source-owned frame data is concrete class state.
export module folia.math.input.tex.parser:base_items;
export import :stack_item;
export import :stack_item_factory;
export import :types;
import std;

export namespace folia::math::tex {

// SubsupItem's base is the three-slot msubsup/munderover built by the command.
// Two-slot msup/mover expose their upper position through the MML family API.
enum class ScriptPosition : std::size_t { sub = 1, sup = 2 };

struct ArrayFrameRule {
    enum class Edge { top, bottom, left, right } edge{};
    std::optional<std::u16string> style;
};

struct ArrayColumnAlignment {
    std::u16string vertical_align;
    std::u16string width;
    std::u16string align;
};

struct ArrayBreakAlignment {
    std::u16string cell;
    std::u16string row;
    std::u16string table;
};

// Source getEntry() returns [prefix, entry, terminator, found]. A named record
// keeps that exact order and meaning without a JS array or sparse-value model.
struct ArrayEntryScan {
    std::u16string prefix;
    std::u16string entry;
    std::u16string terminator;
    bool found{};
};

class StartItem : public BaseItem {
  public:
    StartItem(StackItemFactory &, StackGlobal &);
    StackKind kind() const override;
    bool isOpen() const noexcept override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class StopItem : public BaseItem {
  public:
    explicit StopItem(StackItemFactory &);
    StackKind kind() const override;
    bool isClose() const noexcept override;
};

class OpenItem : public BaseItem {
  protected:
    std::optional<StackErrorDefinition> getErrors(StackErrorKey) const override;

  public:
    explicit OpenItem(StackItemFactory &);
    StackKind kind() const override;
    bool isOpen() const noexcept override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class CloseItem : public BaseItem {
  public:
    bool braket_bar{};
    explicit CloseItem(StackItemFactory &);
    StackKind kind() const override;
    bool isClose() const noexcept override;
};

class NullItem : public BaseItem {
  public:
    explicit NullItem(StackItemFactory &);
    StackKind kind() const override;
};

class PrimeItem : public BaseItem {
  public:
    PrimeItem(StackItemFactory &, mml::MmlNode &base, mml::MmlNode &prime);
    StackKind kind() const override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class SubsupItem : public BaseItem {
  public:
    std::optional<ScriptPosition> position;
    mml::MmlNode *primes{}; // borrowed from the parse MmlDocument
    std::optional<bool> movesupsub;

  protected:
    std::optional<StackErrorDefinition> getErrors(StackErrorKey) const override;

  public:
    SubsupItem(StackItemFactory &, mml::MmlNode &base);
    StackKind kind() const override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class OverItem : public BaseItem {
  public:
    std::u16string name{u"\\over"};
    mml::MmlNode *numerator{}; // borrowed from the parse MmlDocument
    std::optional<std::variant<std::u16string, double>> thickness;
    std::optional<std::u16string> leftDelimiter;
    std::optional<std::u16string> rightDelimiter;
    explicit OverItem(StackItemFactory &);
    StackKind kind() const override;
    bool isClose() const noexcept override;
    std::optional<std::u16string_view> getName() const noexcept override;
    CheckResult checkItem(FrameHandle const &incoming) override;
    std::string toString() const override;
};

class LeftItem : public BaseItem {
  public:
    std::u16string delimiter;

  protected:
    std::optional<StackErrorDefinition> getErrors(StackErrorKey) const override;

  public:
    LeftItem(StackItemFactory &, std::u16string delimiter);
    StackKind kind() const override;
    bool isOpen() const noexcept override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class Middle : public BaseItem {
  public:
    std::u16string delimiter;
    std::optional<std::u16string> color;
    Middle(StackItemFactory &, std::u16string delimiter, std::optional<std::u16string> color);
    StackKind kind() const override;
    bool isClose() const noexcept override;
};

class RightItem : public BaseItem {
  public:
    std::u16string delimiter;
    std::optional<std::u16string> color;
    RightItem(StackItemFactory &, std::u16string delimiter, std::optional<std::u16string> color);
    StackKind kind() const override;
    bool isClose() const noexcept override;
};

class BreakItem : public BaseItem {
  public:
    std::u16string linebreak;
    bool insert{};
    BreakItem(StackItemFactory &, std::u16string linebreak, bool insert);
    StackKind kind() const override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class BeginItem : public BaseItem {
  public:
    std::optional<std::u16string> name;
    std::optional<std::u16string> spread; // Mathtools state on a begin frame
    bool nestStart{};                     // Mathtools nested spread-lines environment marker
    explicit BeginItem(StackItemFactory &);
    StackKind kind() const override;
    bool isOpen() const noexcept override;
    std::optional<std::u16string_view> getName() const noexcept override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class EndItem : public BaseItem {
  public:
    std::optional<std::u16string> name;
    explicit EndItem(StackItemFactory &);
    StackKind kind() const override;
    bool isClose() const noexcept override;
    std::optional<std::u16string_view> getName() const noexcept override;
};

class StyleItem : public BaseItem {
  public:
    mml::AttributeSet styles;
    explicit StyleItem(StackItemFactory &);
    StackKind kind() const override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

enum class PositionMove { vertical, horizontal };

class PositionItem : public BaseItem {
  public:
    std::optional<std::u16string> name;
    std::optional<PositionMove> move;
    std::optional<std::u16string> dh, dd; // signed Source length spellings
    mml::MmlNode *left{};                 // borrowed MathML mspace node
    mml::MmlNode *right{};                // borrowed MathML mspace node
    explicit PositionItem(StackItemFactory &);
    StackKind kind() const override;
    std::optional<std::u16string_view> getName() const noexcept override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class CellItem : public BaseItem {
  public:
    bool isEntry{};
    bool isCR{};
    std::optional<std::u16string> name;
    std::optional<bool> linebreak;
    explicit CellItem(StackItemFactory &);
    StackKind kind() const override;
    bool isClose() const noexcept override;
    std::optional<std::u16string_view> getName() const noexcept override;
};

class MmlItem : public BaseItem {
  public:
    MmlItem(StackItemFactory &, mml::MmlNode &node);
    StackKind kind() const override;
    bool isFinal() const noexcept override;
};

class FnItem : public BaseItem {
  public:
    FnItem(StackItemFactory &, mml::MmlNode &function);
    StackKind kind() const override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class NotItem : public BaseItem {
  public:
    std::shared_ptr<TokenMap> remap; // actual map retained while this frame uses it
    explicit NotItem(StackItemFactory &);
    StackKind kind() const override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class NonscriptItem : public BaseItem {
  public:
    explicit NonscriptItem(StackItemFactory &);
    StackKind kind() const override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class DotsItem : public BaseItem {
  public:
    mml::MmlNode *ldots{}; // borrowed from the parse MmlDocument
    mml::MmlNode *cdots{}; // borrowed from the parse MmlDocument
    explicit DotsItem(StackItemFactory &);
    StackKind kind() const override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class ArrayItem : public BaseItem {
  protected:
    virtual mml::MmlNode &handleFrame(mml::MmlNode &);
    virtual ArrayEntryScan getEntry();

  public:
    std::vector<mml::MmlNode *> table; // borrowed; MmlDocument owns every node
    std::vector<mml::MmlNode *> row;
    std::vector<ArrayFrameRule> frame;
    std::vector<std::size_t> hfill; // Source records StackItem::Size() positions
    mml::AttributeSet arraydef;     // attributes installed on the created mtable
    std::vector<std::u16string> cstart, cend;
    std::optional<std::u16string> open, close;
    std::vector<bool> cextra;
    bool atEnd{};
    std::vector<ArrayColumnAlignment> ralign;
    ArrayBreakAlignment breakAlign;
    std::size_t templateSubs{};
    TexParser *parser{}; // borrowed active parser; never owns its input/services
    bool requireClose{};
    std::optional<std::u16string> arrayPadding;
    bool isCases{}, isNumbered{}, isLabeled{};
    std::optional<double> rowspacing; // numeric baseline set by addRowSpacing
    bool numCases{};
    std::optional<std::u16string> casesEnv;
    bool nestable{};
    std::optional<std::size_t> flushspaceabove;
    std::optional<std::u16string> shove;
    // CD captures the environment's arrow dimensions when this array opens.
    std::optional<std::u16string> cd_min_width, cd_min_height;

    explicit ArrayItem(StackItemFactory &);
    StackKind kind() const override;
    bool isOpen() const noexcept override;
    bool copyEnv() const noexcept override;
    CheckResult checkItem(FrameHandle const &incoming) override;
    mml::MmlNode &createMml();
    void StartEntry();
    virtual void EndEntry();
    virtual void EndRow();
    virtual void EndTable();
    void checkLines();
    void addRowSpacing(std::u16string_view spacing);
};

class EqnArrayItem : public ArrayItem {
  protected:
    std::u16string name;
    bool numbered{};
    bool taggable{};
    std::size_t maxrow{};
    void extendArray(std::u16string_view name, std::ptrdiff_t max);
    void addIndentshift();

  public:
    EqnArrayItem(StackItemFactory &, std::u16string name, bool numbered, bool taggable);
    StackKind kind() const override;
    void EndEntry() override;
    void EndRow() override;
    void EndTable() override;
};

class MstyleItem : public BeginItem {
  public:
    mml::AttributeSet attributes;
    MstyleItem(StackItemFactory &, mml::AttributeSet attributes, std::u16string name);
    StackKind kind() const override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

class EquationItem : public BaseItem {
  public:
    bool numbered{};
    std::optional<std::u16string> name;
    EquationItem(StackItemFactory &, bool numbered);
    StackKind kind() const override;
    std::optional<std::u16string_view> getName() const noexcept override;
    bool isOpen() const noexcept override;
    CheckResult checkItem(FrameHandle const &incoming) override;
};

StackItemCatalog baseItemClasses();

} // namespace folia::math::tex
