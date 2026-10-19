/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/munderover.ts.
 *************************************************************/
export module folia.math.output.common:munderover;
export import :scriptbase;
import std;

export namespace folia::math::output::common {
class CommonMunder : public CommonScriptbase {
  public:
    CommonMunder(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
    void initializeSource() override;
    CommonWrapper *scriptChild() const override;
    void computeBBox(BBox &, bool recompute = false) override;
};

class CommonMover : public CommonScriptbase {
  public:
    CommonMover(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
    void initializeSource() override;
    CommonWrapper *scriptChild() const override;
    void computeBBox(BBox &, bool recompute = false) override;
};

class CommonMunderover : public CommonScriptbase {
  public:
    CommonMunderover(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
    void initializeSource() override;
    CommonWrapper *scriptChild() const override;
    CommonWrapper *underChild() const;
    CommonWrapper *overChild() const;
    void computeBBox(BBox &, bool recompute = false) override;
};
} // namespace folia::math::output::common
