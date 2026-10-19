/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/semantics.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :semantics;
import :wrapper;
import std;

namespace folia::math::output::common {
void CommonSemantics::computeBBox(BBox& bbox, bool) {
    const auto children = childNodes;
    if (!children)
        throw options::Exception("TypeError", u"Cannot read properties of undefined (reading 'length')");
    if (!*children)
        throw options::Exception("TypeError", u"Cannot read properties of null (reading 'length')");
    if ((*children)->size()) {
        auto* child = dynamic_cast<CommonWrapper*>((*children)->get(0).requireNode("getBBox"));
        const auto box = child->getBBox();
        bbox.w = box->w;
        bbox.h = box->h;
        bbox.d = box->d;
    }
}

std::size_t CommonSemantics::breakCount() {
    if (node->isEmbellished()) {
        auto* child = coreMO();
        if (!child)
            throw options::Exception("TypeError", u"Cannot read properties of null (reading 'embellishedBreakCount')");
        return child->embellishedBreakCount();
    }
    if (!childNodes)
        throw options::Exception("TypeError", u"Cannot read properties of undefined (reading '0')");
    if (!*childNodes)
        throw options::Exception("TypeError", u"Cannot read properties of null (reading '0')");
    const auto first = (*childNodes)->get(0);
    auto* child = dynamic_cast<CommonWrapper*>(first.requireNode("breakCount"));
    return child->breakCount();
}
}
