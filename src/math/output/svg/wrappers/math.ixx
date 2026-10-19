// MathJax output/svg/Wrappers/math.ts: SVG-specific percentage-width policy.
export module folia.math.output.svg.wrappers.math;
export import folia.math.output.common;
import std;

export namespace folia::math::output::svg {
class SvgMath final : public common::CommonMath {
public:
    using CommonMath::CommonMath;
    bool setChildPWidths(bool recompute,std::optional<double> width={},bool clear=true) override;
};
}
