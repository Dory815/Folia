/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/munderover.ts.
 *************************************************************/
export module folia.math.output.common:munderover;
export import :msubsup;
import std;

export namespace folia::math::output::common {
class CommonMunder : public CommonMsub {
public:
    CommonMunder(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    CommonWrapper* scriptChild() const override;
    void computeBBox(BBox&,bool recompute=false) override;
};
class CommonMover : public CommonMsup {
public:
    CommonMover(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    CommonWrapper* scriptChild() const override;
    void computeBBox(BBox&,bool recompute=false) override;
};
class CommonMunderover : public CommonMsubsup {
public:
    CommonMunderover(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    CommonWrapper* underChild() const;
    CommonWrapper* overChild() const;
    CommonWrapper* subChild() const override;
    CommonWrapper* supChild() const override;
    void computeBBox(BBox&,bool recompute=false) override;
};
}
