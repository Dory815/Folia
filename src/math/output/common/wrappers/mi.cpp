module folia.math.output.common;
import :mi;
namespace folia::math::output::common {
void CommonMi::computeBBox(BBox& bbox,bool recompute){
    (void)recompute;
    CommonWrapper::computeBBox(bbox,false);
    copySkewIC(bbox);
}
}
