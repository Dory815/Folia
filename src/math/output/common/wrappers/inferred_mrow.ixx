/*************************************************************
 * Candidate native port of MathJax 4.1.3 inferred mrow specialization.
 * See src/math/LICENSE-MathJax.txt.
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
} // namespace folia::math::output::common
