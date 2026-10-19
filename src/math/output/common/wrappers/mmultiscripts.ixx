/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mmultiscripts.ts.
 *************************************************************/
export module folia.math.output.common:mmultiscripts;
export import :msubsup;
import std;

export namespace folia::math::output::common {
struct ScriptData {
    std::optional<BBox> base;
    BBox sub,sup,psub,psup;
    std::size_t numPrescripts=0,numScripts=0;
};
struct ScriptLists {
    std::vector<BBox> base,subList,supList,psubList,psupList;
};
class CommonMmultiscripts : public CommonMsubsup {
    ScriptData scriptData_;
    std::size_t firstPrescript_=0;
public:
    CommonMmultiscripts(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    BBox combinePrePost(const BBox&,const BBox&) const;
    void getScriptData();
    ScriptLists getScriptBBoxLists();
    void padLists(std::vector<BBox>&,std::vector<BBox>&) const;
    void combineBBoxLists(BBox&,BBox&,const std::vector<BBox>&,const std::vector<BBox>&) const;
    std::array<double,3> getScaledWHD(const BBox&) const;
    std::array<double,2> getCombinedUV();
    BBox addPrescripts(BBox&,double u,double v) const;
    BBox addPostscripts(BBox&,double u,double v) const;
    std::shared_ptr<std::array<double,3>> getUVQ(const BBox* subbox=nullptr,const BBox* supbox=nullptr) override;
    BBox appendScripts(BBox&) const override;
    std::shared_ptr<LineBBox> computeLineBBox(std::size_t) override;
    const ScriptData& scriptsData() const { return scriptData_; }
    std::size_t firstPrescript() const { return firstPrescript_; }
};
}
