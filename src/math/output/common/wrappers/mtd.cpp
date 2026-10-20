/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtd.ts.
 *************************************************************/
module folia.math.output.common;
import :mtd;
import :mtr;
import :mtable;
import folia.math.core.mml_tree.attributes;
import folia.math.core.mml_tree.mml_nodes.mtd;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
SceneNodes CommonMtd::emitScene(scene::Builder &builder, std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;
    auto containers = standardSceneNodes(builder, parents);
    addSceneChildren(builder, containers.content);
    return containers.nodes;
}

std::array<double, 2> CommonMtd::placeSceneCell(scene::Builder &builder,
                                                std::span<SceneNode const> nodes, double x,
                                                double y, double width, double height,
                                                double depth) {
    const auto &box = getBBox();
    const double h = std::max(box.h, 0.75);
    const double d = std::max(box.d, 0.25);
    const double dx = getAlignX(width, box, getChildAlign(0));
    std::string rowAlign;
    if (const auto *value = node->attributes().lookup(u"rowalign"))
        if (const auto *text = std::get_if<std::u16string>(value))
            rowAlign = utf16::encode(*text);
    const double dy = getAlignY(height, depth, h, d, rowAlign);
    placeScene(builder, nodes, x + dx, y + dy);
    return {dx, dy};
}

void CommonMtd::placeSceneColor(scene::Builder &builder, std::span<SceneNode const> nodes,
                                scene::Rect bounds) {
    if (!nodes.empty() && nodes.front())
        builder.set_background_bounds(*nodes.front(), bounds);
}

void CommonMtd::invalidateBBox(bool) {
    bboxComputed = false;
    lineBBox.clear();
}

double CommonMtd::getWrapWidth(std::size_t) {
    auto *row = dynamic_cast<CommonMtr *>(parent);
    auto *table = row ? dynamic_cast<CommonMtable *>(row->parent) : nullptr;
    if (!table)
        throw std::logic_error("CommonMtd requires an mtr parent in an mtable");
    const auto position = node->childPosition();
    if (!position)
        throw std::logic_error("CommonMtd is not linked into its mtr node");
    const auto index = *position - (row->labeled() ? 1u : 0u);
    if (index < table->cWidths.size())
        if (const auto *width = std::get_if<double>(&table->cWidths[index]))
            return *width;
    const auto data = table->getTableData();
    if (index >= data.W.size())
        throw std::out_of_range("CommonMtd column is outside TableData");
    return data.W[index];
}

std::string CommonMtd::getChildAlign(std::size_t) const {
    const auto *value = node->attributes().lookup(u"columnalign");
    if (!value || std::holds_alternative<mml::AttributeNull>(*value))
        return {};
    if (const auto *text = std::get_if<std::u16string>(value))
        return utf16::encode(*text);
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean ? "true" : "false";
    return std::to_string(std::get<double>(*value));
}
} // namespace folia::math::output::common
