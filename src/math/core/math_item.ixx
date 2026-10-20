// Native MathItem expression state for the shared mathematical output pipeline.
// MathJax 4.1.3 core/MathItem.ts; Apache-2.0.
export module folia.math.core.math_item;
export import folia.math.core.mml_tree.mml_document;
export import folia.math.core.dom_adaptor;
export import folia.math.output.scene;
export import folia.math.util.bbox;
import std;

export namespace folia::math::core {
enum class MathState : std::int32_t {
    unprocessed = 0, find_math = 10, compiled = 20, convert = 100,
    metrics = 110, rerender = 125, typeset = 150, inserted = 200, last = 10000
};
// Stock codes retain their Source values. A numeric predecessor such as 124
// is representable; no runtime string -> state registry is needed by scope38.
struct MathStates {
    static constexpr auto UNPROCESSED = MathState::unprocessed;
    static constexpr auto FINDMATH = MathState::find_math;
    static constexpr auto COMPILED = MathState::compiled;
    static constexpr auto CONVERT = MathState::convert;
    static constexpr auto METRICS = MathState::metrics;
    static constexpr auto RERENDER = MathState::rerender;
    static constexpr auto TYPESET = MathState::typeset;
    static constexpr auto INSERTED = MathState::inserted;
    static constexpr auto LAST = MathState::last;
};
inline constexpr MathStates STATE;

enum class MathDisplay : std::uint8_t { inline_math, display_math, escaped };
// Source protoItem() results before HTMLDocument maps string offsets to DOM nodes.
struct ProtoItemLocation {
    std::size_t n{};
};
struct ProtoItem {
    std::u16string math;
    ProtoItemLocation start, end;
    std::u16string open, close;
    std::size_t n{};
    MathDisplay display{MathDisplay::escaped};
};
ProtoItem protoItem(std::u16string open, std::u16string math, std::u16string close,
                   std::size_t n, std::size_t start, std::size_t end,
                   MathDisplay display = MathDisplay::escaped);
struct MathMetrics {
    double em;
    double ex;
    double containerWidth;
    double scale;
};
struct MathRecompile {
    MathState state;
    std::int64_t counter;
    friend bool operator==(MathRecompile const&, MathRecompile const&) = default;
};
struct MathInputData {
    std::optional<MathRecompile> recompile;
};
struct MathOutputData {
    std::optional<std::u16string> mtext_family;
    std::optional<std::u16string> merror_family;
};

class MathItem;
class MathOutputJaxService;
struct CompileServices {
    MathOutputJaxService* output{};
    AbstractDOMAdaptor* adaptor{};
};
// Retained existing service roles, narrowed to the native expression pipeline.
// No service owns a MathItem or a second MmlDocument.
class MathInputJaxService {
public:
    virtual ~MathInputJaxService() = default;
    virtual mml::MmlNode& compile(MathItem&, mml::MmlDocument&,
                                 CompileServices = {}) = 0;
};
class MathOutputJaxService {
public:
    virtual ~MathOutputJaxService() = default;
    virtual output::scene::Scene typeset(MathItem&, mml::MmlDocument&) = 0;
    virtual output::scene::Scene escaped(MathItem&, mml::MmlDocument&) = 0;
    // Source CommonOutputJax.getBBox; needed by configured bussproofs.
    // Returns a value, never a reference into a temporary wrapper arena.
    virtual BBox getBBox(MathItem&, mml::MmlDocument&) = 0;
};
class MathRenderActionsService {
public:
    virtual ~MathRenderActionsService() = default;
    virtual void renderMath(MathItem&, mml::MmlDocument&,
                            MathState start = STATE.UNPROCESSED) = 0;
    virtual void renderConvert(MathItem&, mml::MmlDocument&,
                               MathState end = STATE.LAST) = 0;
};

class MathItem {
public:
    std::u16string math;
    MathInputJaxService* inputJax{}; // nullable Source input: measurement-only item
    MathDisplay display{MathDisplay::display_math};
    mml::MmlNode* root{}; // sole expression-root borrow, owned by supplied document
    std::optional<output::scene::Scene> typesetRoot; // actual owned native output
    std::optional<MathMetrics> metrics; // Source starts with {}; no metric defaults here
    MathInputData inputData;
    MathOutputData outputData;

    virtual ~MathItem() = default;
    virtual bool isEscaped() const noexcept = 0;
    bool isDisplay() const noexcept;
    void setDisplay(bool) noexcept; // BaseMethods.Equation updates this one field
    virtual void render(MathRenderActionsService&, mml::MmlDocument&) = 0;
    virtual void rerender(MathRenderActionsService&, mml::MmlDocument&,
                          MathState start = STATE.RERENDER) = 0;
    virtual void convert(MathRenderActionsService&, mml::MmlDocument&,
                         MathState end = STATE.LAST) = 0;
    virtual void compile(mml::MmlDocument&, CompileServices = {}) = 0;
    virtual void typeset(MathOutputJaxService&, mml::MmlDocument&) = 0;
    // Core Source defaults only; no HTML insertion shell is implemented.
    virtual void updateDocument() = 0;
    virtual void removeFromDocument(bool restore = false) = 0;
    virtual void setMetrics(double em, double ex, double cwidth, double scale) = 0;
    virtual MathState state() const noexcept = 0;
    virtual MathState state(MathState, bool restore = false) = 0;
    virtual void reset(bool restore = false) = 0;
    virtual void clear() = 0;
};

// The actual Source implementation family, not a second job/request facade.
// No native subclass is needed solely to duplicate these default algorithms.
class AbstractMathItem : public MathItem {
protected:
    MathState state_{STATE.UNPROCESSED};
public:
    AbstractMathItem(std::u16string math, MathInputJaxService*,
                     MathDisplay display = MathDisplay::display_math);
    AbstractMathItem(AbstractMathItem const&) = delete;
    AbstractMathItem& operator=(AbstractMathItem const&) = delete;
    AbstractMathItem(AbstractMathItem&&) = delete;
    AbstractMathItem& operator=(AbstractMathItem&&) = delete;
    bool isEscaped() const noexcept override;
    void render(MathRenderActionsService&, mml::MmlDocument&) override;
    void rerender(MathRenderActionsService&, mml::MmlDocument&,
                  MathState start = STATE.RERENDER) override;
    void convert(MathRenderActionsService&, mml::MmlDocument&,
                 MathState end = STATE.LAST) override;
    void compile(mml::MmlDocument&, CompileServices = {}) override;
    void typeset(MathOutputJaxService&, mml::MmlDocument&) override;
    void updateDocument() override;
    void removeFromDocument(bool restore = false) override;
    void setMetrics(double em, double ex, double cwidth, double scale) override;
    MathState state() const noexcept override;
    MathState state(MathState, bool restore = false) override;
    void reset(bool restore = false) override;
    void clear() override;
};
}
