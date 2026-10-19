/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtr.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :mtr;
import :mo;
import std;

namespace folia::math::output::common {
std::size_t CommonMtr::numCells() const {
    return childNodes.size();
}

std::span<CommonWrapper *const> CommonMtr::tableCells() const {
    return childNodes;
}

CommonWrapper *CommonMtr::getChild(std::size_t index) const {
    return index < childNodes.size() ? childNodes[index] : nullptr;
}

std::vector<BBox> CommonMtr::getChildBBoxes() const {
    std::vector<BBox> result;
    result.reserve(childNodes.size());
    for (auto *cell : childNodes)
        result.push_back(cell->getBBox());
    return result;
}

void CommonMtr::stretchChildren(std::optional<std::array<double, 2>> HD) {
    const auto cells = tableCells();
    std::vector<CommonWrapper *> stretchy;
    for (auto *cell : cells) {
        if (cell->childNodes.empty())
            throw std::logic_error("table cell has no first child wrapper for stretch lookup");
        auto *child = cell->childNodes.front();
        if (child->canStretch(StretchDirection::vertical))
            stretchy.push_back(child);
    }
    if (stretchy.size() && childNodes.size() > 1 && !HD) {
        double H = 0, D = 0;
        const bool all = stretchy.size() > 1 && stretchy.size() == childNodes.size();
        for (auto *cell : cells) {
            if (cell->childNodes.empty())
                throw std::logic_error("table cell has no first child wrapper for height lookup");
            auto *child = cell->childNodes.front();
            const bool noStretch = std::holds_alternative<NoStretch>(child->stretch);
            if (all || noStretch) {
                const auto box = child->getBBox(noStretch);
                H = std::max(H, box.h);
                D = std::max(D, box.d);
            }
        }
        HD = std::array<double, 2>{H, D};
    }
    if (!HD)
        return;
    for (auto *child : stretchy) {
        auto *mo = dynamic_cast<CommonMo *>(child->coreMO());
        if (!mo)
            throw std::logic_error("stretchable table cell has no operator core");
        const double scale = child->coreRScale();
        mo->getStretchedVariant(std::array<double, 2>{(*HD)[0] * scale, (*HD)[1] * scale});
    }
}

std::size_t CommonMlabeledtr::numCells() const {
    return childNodes.empty() ? 0 : childNodes.size() - 1;
}

std::span<CommonWrapper *const> CommonMlabeledtr::tableCells() const {
    return childNodes.empty() ? std::span<CommonWrapper *const>{}
                              : std::span<CommonWrapper *const>{childNodes}.subspan(1);
}

CommonWrapper *CommonMlabeledtr::getChild(std::size_t index) const {
    return index + 1 < childNodes.size() ? childNodes[index + 1] : nullptr;
}

std::vector<BBox> CommonMlabeledtr::getChildBBoxes() const {
    std::vector<BBox> result;
    result.reserve(numCells());
    for (auto *cell : tableCells())
        result.push_back(cell->getBBox());
    return result;
}
} // namespace folia::math::output::common
