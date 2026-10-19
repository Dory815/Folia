/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mrow.ts.
 * CommonInferredMrowMixin Source specialization.
 *************************************************************/
export module folia.math.output.common:inferred_mrow;
export import :mrow;

export namespace folia::math::output::common {
class CommonInferredMrow : public CommonMrow {
public:
    using CommonMrow::CommonMrow;
protected:
    void getScale() override;
};
}
