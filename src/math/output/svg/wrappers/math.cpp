module folia.math.output.svg.wrappers.math;
import std;

namespace folia::math::output::svg {
bool SvgMath::setChildPWidths(bool recompute,std::optional<double> width,bool) {
    return CommonMath::setChildPWidths(recompute,
        parent?width:std::optional<double>(metrics().containerWidth/jax().pxPerEm),false);
}
}
