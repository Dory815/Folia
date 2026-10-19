/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/TeXAtom.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :tex_atom;
import :wrapper;
import std;

namespace folia::math::output::common {
void CommonTeXAtom::computeBBox(BBox& bbox, bool recompute) {
    CommonWrapper::computeBBox(bbox, recompute);
    const auto children = childNodes;
    if (!children)
        throw options::Exception("TypeError", u"Cannot read properties of undefined (reading '0')");
    if (!*children)
        throw options::Exception("TypeError", u"Cannot read properties of null (reading '0')");
    if ((*children)->empty()) return;
    const auto first = (*children)->get(0);
    if (!first.isNode()) return;
    auto* child = dynamic_cast<CommonWrapper*>(first.nodeOrNull());
    if (!child) return;
    if (child->bbox && child->bbox->ic) bbox.ic = child->bbox->ic;
}
}
