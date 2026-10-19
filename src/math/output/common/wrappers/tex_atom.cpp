/*************************************************************
 * Native CommonTeXAtom methods mirroring MathJax 4.1.3.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :tex_atom;
import :wrapper;
import std;

namespace folia::math::output::common {
void CommonTeXAtom::computeBBox(BBox &box, bool recompute) {
    CommonWrapper::computeBBox(box, recompute);
    if (childNodes.empty() || !childNodes.front())
        return;
    const double italicCorrection = childNodes.front()->bbox.ic;
    if (italicCorrection)
        box.ic = italicCorrection;
}
} // namespace folia::math::output::common
