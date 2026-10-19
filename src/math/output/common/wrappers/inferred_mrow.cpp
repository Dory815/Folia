/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mrow.ts.
 * CommonInferredMrowMixin Source specialization.
 *************************************************************/
module folia.math.output.common;
import :inferred_mrow;
import folia.math.util.option_types;
import std;

namespace folia::math::output::common {
void CommonInferredMrow::getScale(){
    if(!parent)
        throw options::Exception("TypeError",u"Cannot read properties of null (reading 'bbox')");
    if(!parent->bbox)
        throw options::Exception("TypeError",u"Cannot read properties of null (reading 'scale')");
    const double parentScale=parent->bbox->scale;
    if(!bbox)
        throw options::Exception("TypeError",u"Cannot set properties of null (setting 'scale')");
    bbox->scale=parentScale;
    bbox->rscale=1;
}
}
