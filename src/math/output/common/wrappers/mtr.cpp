/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtr.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :mtr;
import :mtd;
import :mtable;
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

double CommonMtr::placeSceneCell(scene::Builder &builder, CommonMtd &cell,
                                 std::span<SceneNode const> emitted, TableRowGeometry const &row,
                                 TableCellPlacement const &placement) {
    const double scale = 1 / getBBox().rscale;
    const double h = row.h * scale;
    const double d = row.d * scale;
    const double topSpace = row.t_space * scale;
    const double bottomSpace = row.b_space * scale;
    const auto [dx, dy] = cell.placeSceneCell(builder, emitted, placement.x + placement.l_space,
                                              placement.y, placement.width, h, d);
    const double width = placement.l_space + placement.width + placement.r_space;
    const double x = -(dx + placement.l_space + placement.l_line / 2);
    const double y = -(d + bottomSpace + dy);
    const double boundsWidth = width + (placement.l_line + placement.r_line) / 2;
    const double boundsHeight = h + d + topSpace + bottomSpace;
    cell.placeSceneColor(builder, emitted, {x, y, boundsWidth, boundsHeight});
    return width + placement.r_line;
}

TableRowScene CommonMtr::emitTableScene(scene::Builder &builder,
                                        std::span<scene::NodeId const> parents,
                                        TableRowGeometry const &geometry,
                                        std::optional<scene::NodeId>) {
    auto *table = dynamic_cast<CommonMtable *>(parent);
    if (!table)
        throw std::logic_error("CommonMtr scene emission requires an mtable parent");

    TableRowScene result;
    auto containers = standardSceneNodes(builder, parents);
    result.nodes = containers.nodes;

    const double scale = 1 / getBBox().rscale;
    const TableRowGeometry row{geometry.h * scale,       geometry.d * scale,
                               geometry.t_space * scale, geometry.b_space * scale,
                               geometry.t_line * scale,  geometry.b_line * scale};
    const auto columnSpaces = table->getColumnHalfSpacing();
    const auto widths = table->getComputedWidths();
    if (numCells() && (columnSpaces.size() < numCells() + 1 || widths.size() < numCells() ||
                       table->cLines.size() + 2 < numCells() + 1))
        throw std::logic_error(
            "CommonMtr table layout is missing column spacing, width, or line data");
    std::vector<double> columnLines;
    columnLines.reserve(table->cLines.size() + 2);
    columnLines.push_back(table->fLine);
    columnLines.insert(columnLines.end(), table->cLines.begin(), table->cLines.end());
    columnLines.push_back(table->fLine);

    // Source initializes x from cLines[0] before the scaled placeCell terms.
    double x = columnLines.front();
    for (std::size_t i = 0; i < numCells(); ++i) {
        auto *cell = dynamic_cast<CommonMtd *>(getChild(i));
        if (!cell)
            throw std::logic_error("CommonMtr requires an mtd cell for scene emission");
        auto emitted = cell->emitScene(builder, containers.content);
        const TableCellPlacement placement{x,
                                           0,
                                           columnSpaces[i] * scale,
                                           widths[i] * scale,
                                           columnSpaces[i + 1] * scale,
                                           columnLines[i] * scale,
                                           columnLines[i + 1] * scale};
        x += placeSceneCell(builder, *cell, emitted, geometry, placement);
    }

    // Source placeColor adjusts only the row's first background rectangle,
    // after every cell has been emitted and placed.
    if (!result.nodes.empty() && result.nodes.front()) {
        if (const auto bounds = builder.background_bounds(*result.nodes.front())) {
            builder.set_background_bounds(
                *result.nodes.front(),
                {bounds->x, -(row.d + row.b_space + row.b_line / 2), table->getWidth() * scale,
                 row.t_line / 2 + row.t_space + row.h + row.d + row.b_space + row.b_line / 2});
        }
    }
    return result;
}

TableRowScene CommonMlabeledtr::emitTableScene(scene::Builder &builder,
                                               std::span<scene::NodeId const> parents,
                                               TableRowGeometry const &geometry,
                                               std::optional<scene::NodeId> label_parent) {
    auto result = CommonMtr::emitTableScene(builder, parents, geometry);
    if (!label_parent || childNodes.empty())
        return result;
    auto *label = childNodes.front();
    if (!label)
        throw std::logic_error("CommonMlabeledtr has a null label wrapper");
    const std::array<scene::NodeId, 1> labelParents{*label_parent};
    result.label_nodes = label->emitScene(builder, labelParents);
    return result;
}
} // namespace folia::math::output::common
