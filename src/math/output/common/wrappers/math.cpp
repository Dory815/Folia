/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/math.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :math;
import :wrapper;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
double CommonMath::getWrapWidth(std::size_t) {
    if (parent) return getBBox()->w;
    const auto& metrics = this->metrics();
    return metrics.containerWidth / jax().pxPerEm;
}

void CommonMath::computeBBox(BBox& bbox, bool recompute) {
    CommonWrapper::computeBBox(bbox, recompute);
    const auto attributes = node->attributes();
    const Property overflow = attributes->get("overflow");
    if (!parent && jax().math && property_host::truthy(jax().math->display) &&
        overflow == Property("linebreak")) {
        const double width = containerWidth();
        if (bbox.w > width) {
            if (!childNodes)
                throw options::Exception("TypeError", u"Cannot read properties of undefined (reading '0')");
            if (!*childNodes)
                throw options::Exception("TypeError", u"Cannot read properties of null (reading '0')");
            const auto first = (*childNodes)->get(0);
            auto* child = dynamic_cast<CommonWrapper*>(first.requireNode("breakToWidth"));
            child->breakToWidth(width);
        }
        if (!childNodes)
            throw options::Exception("TypeError", u"Cannot read properties of undefined (reading '0')");
        if (!*childNodes)
            throw options::Exception("TypeError", u"Cannot read properties of null (reading '0')");
        auto* child = dynamic_cast<CommonWrapper*>((*childNodes)->get(0).requireNode("getBBox"));
        bbox.updateFrom(*child->getBBox());
    }
}
}
