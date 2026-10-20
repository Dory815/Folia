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

SceneNodes CommonTeXAtom::emitScene(scene::Builder &builder,
                                    std::span<scene::NodeId const> parents) {
    auto nodes = CommonWrapper::emitScene(builder, parents);
    if (nodes.empty() || !nodes.front())
        return nodes;
    const auto texClass = node->texClass();
    const auto index = texClass ? static_cast<int>(*texClass) : -1;
    const std::string value =
        index >= 0 && static_cast<std::size_t>(index) < mml::TEXCLASSNAMES.size()
            ? std::string(mml::TEXCLASSNAMES[static_cast<std::size_t>(index)])
            : "undefined";
    auto metadata = builder.metadata(*nodes.front());
    const auto existing =
        std::find_if(metadata.ordered_attributes.begin(), metadata.ordered_attributes.end(),
                     [](auto const &attribute) { return attribute.name == "data-mjx-texclass"; });
    if (existing == metadata.ordered_attributes.end())
        metadata.ordered_attributes.push_back({"data-mjx-texclass", value});
    else
        existing->value = value;
    builder.set_metadata(*nodes.front(), std::move(metadata));
    return nodes;
}
} // namespace folia::math::output::common
