/*************************************************************
 * MathJax 4.1.3 output/common/LinebreakVisitor.ts.
 * Common line-break traversal foundation; visitor method dispatch uses the
 * actual generic core Tree Visitor module.
 *************************************************************/
export module folia.math.output.common:linebreak_visitor;
export import :wrapper;
export import folia.math.core.tree.visitor;
import std;

export namespace folia::math::output::common {
class CommonWrapper;
class CommonWrapperFactory;
class CommonMo;
class CommonMspace;
class CommonMsub;
class CommonMsup;
class CommonMsubsup;
class CommonMmultiscripts;
inline constexpr double NOBREAK = 1000000;
using IndexData = std::optional<std::array<std::size_t,2>>;

struct BreakData {
    // Source keeps the inner [wrapper,index] tuple as an identity-bearing
    // array in its Set. Equal values from separate pushes remain distinct.
    std::shared_ptr<const std::byte> identity=std::make_shared<const std::byte>();
    std::pair<CommonWrapper*,IndexData> wrapperAndIndex{};
    double penalty=0;
    double beforeWidth=0;
    double afterWidth=0;
    double widthToNext=0;
};

struct BreakEntry {
    std::shared_ptr<const std::byte> identity;
    std::pair<CommonWrapper*,IndexData> wrapperAndIndex;
};

struct LinebreakState {
    std::vector<BreakEntry> breaks;
    std::vector<BreakData> potential;
    double width=0,w=0,prevWidth=0;
    std::optional<BreakData> prevBreak;
    std::size_t depth=0;
    double mathWidth=0,mathLeft=0;
};

class Linebreaks : public tree::AbstractVisitor<CommonWrapper> {
protected:
    CommonWrapperFactory* factory_;
public:
    using MethodLookup=tree::AbstractVisitor<CommonWrapper>::MethodLookup;
    explicit Linebreaks(CommonWrapperFactory&,MethodLookup={});
    CommonWrapperFactory& factory()const{return *factory_;}
    virtual void breakToWidth(CommonWrapper&,double) {}
};

class LinebreakVisitor : public Linebreaks {
    LinebreakState state_;
    static MethodLookup methods();
    static std::size_t lineArgument(std::span<const std::any>);
    void addWidth(const BBox&,std::optional<double> width={});
    void processBreak();
    void pushBreak(CommonWrapper&,double penalty,double width,IndexData);
    void breakLineToWidth(CommonWrapper&,std::size_t);
    LinebreakState createState(CommonWrapper&);
    std::pair<double,double> borderLR(const CommonWrapper&)const;
    double moPenalty(CommonMo&);
    CommonMo* getPrevious(CommonMo&) const;
    double mspacePenalty(CommonMspace&);
    double mtextPenalty() const;
public:
    explicit LinebreakVisitor(CommonWrapperFactory&);
    std::any visitNode(CommonWrapper&,std::span<const std::any> args={}) override;
    void breakToWidth(CommonWrapper&,double)override;
    std::any visitDefault(CommonWrapper&,std::span<const std::any> args={})override;
    std::any visitEmbellishedOperator(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMoNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMspaceNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMtextNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMrowNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitInferredMrowNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMfracNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMsqrtNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMrootNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMsubNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMsupNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMsubsupNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMmultiscriptsNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMfencedNode(CommonWrapper&,std::span<const std::any> args={});
    std::any visitMactionNode(CommonWrapper&,std::span<const std::any> args={});
};
}
