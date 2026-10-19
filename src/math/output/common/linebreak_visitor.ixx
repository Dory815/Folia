/*************************************************************
 * Candidate native port of MathJax 4.1.3 output/common/LinebreakVisitor.ts.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:linebreak_visitor;
export import :wrapper;
export import :wrapper_factory;
export import folia.math.core.tree.visitor;
import std;

export namespace folia::math::output::common {
class CommonMo;
class CommonMspace;
class CommonMsub;
class CommonMsup;
class CommonMsubsup;
class CommonMmultiscripts;

inline constexpr double NOBREAK = 1000000;
using IndexData = std::optional<std::array<std::size_t, 2>>;

struct BreakData {
    // Each push creates a new Source identity, including equal wrapper/index values.
    std::uint64_t identity{};
    std::pair<CommonWrapper *, IndexData> wrapperAndIndex{};
    double penalty{};
    double beforeWidth{};
    double afterWidth{};
    double widthToNext{};
};

struct BreakEntry {
    std::uint64_t identity{};
    std::pair<CommonWrapper *, IndexData> wrapperAndIndex{};
};

struct LinebreakState {
    std::vector<BreakEntry> breaks;
    std::vector<BreakData> potential;
    double width{};
    double w{};
    double prevWidth{};
    std::optional<BreakData> prevBreak;
    // Source arithmetic permits negative depth after an unmatched close fence.
    std::int64_t depth{};
    double mathWidth{};
    double mathLeft{};
    std::uint64_t nextIdentity{};
};

// Source's base service is deliberately abstract: a consumer must supply the
// actual linebreak algorithm rather than receiving a production no-op.
class Linebreaks {
  protected:
    CommonWrapperFactory &factory_;

  public:
    explicit Linebreaks(CommonWrapperFactory &factory) noexcept : factory_(factory) {
    }

    virtual ~Linebreaks() = default;

    CommonWrapperFactory &factory() const noexcept {
        return factory_;
    }

    virtual void breakToWidth(CommonWrapper &, double) = 0;
};

using LinebreakVisitorBase = tree::AbstractVisitor<CommonWrapper, mml::NodeKind, std::size_t, void>;

class LinebreakVisitor : public Linebreaks, public LinebreakVisitorBase {
    LinebreakState state_;

    static LinebreakVisitorBase::HandlerTable handlers(LinebreakVisitor *);
    static tree::VisitorChildren<CommonWrapper> liveChildren();
    void addWidth(BBox const &, std::optional<double> width = {});
    void processBreak();
    void pushBreak(CommonWrapper &, double penalty, double width, IndexData);
    void breakLineToWidth(CommonWrapper &, std::size_t);
    LinebreakState createState(CommonWrapper &);
    std::pair<double, double> borderLR(CommonWrapper const &) const;
    double moPenalty(CommonMo &);
    CommonWrapper *getPrevious(CommonMo &) const;
    double mspacePenalty(CommonMspace &);
    double mtextPenalty() const;

  public:
    explicit LinebreakVisitor(CommonWrapperFactory &);
    void visitNode(CommonWrapper &, std::size_t const &) override;
    void breakToWidth(CommonWrapper &, double) override;
    void visitDefault(CommonWrapper &, std::size_t const &) override;
    void visitEmbellishedOperator(CommonWrapper &, std::size_t const &);
    void visitMoNode(CommonWrapper &, std::size_t const &);
    void visitMspaceNode(CommonWrapper &, std::size_t const &);
    void visitMtextNode(CommonWrapper &, std::size_t const &);
    void visitMrowNode(CommonWrapper &, std::size_t const &);
    void visitInferredMrowNode(CommonWrapper &, std::size_t const &);
    void visitMfracNode(CommonWrapper &, std::size_t const &);
    void visitMsqrtNode(CommonWrapper &, std::size_t const &);
    void visitMrootNode(CommonWrapper &, std::size_t const &);
    void visitMsubNode(CommonWrapper &, std::size_t const &);
    void visitMsupNode(CommonWrapper &, std::size_t const &);
    void visitMsubsupNode(CommonWrapper &, std::size_t const &);
    void visitMmultiscriptsNode(CommonWrapper &, std::size_t const &);
    void visitMfencedNode(CommonWrapper &, std::size_t const &);
    void visitMactionNode(CommonWrapper &, std::size_t const &);
};
} // namespace folia::math::output::common
