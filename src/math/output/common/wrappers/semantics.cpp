/*************************************************************
 * Native CommonSemantics methods mirroring MathJax 4.1.3.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :semantics;
import :wrapper;
import :mo;
import std;

namespace folia::math::output::common {
void CommonSemantics::computeBBox(BBox &box, bool) {
    if (childNodes.empty())
        return;
    if (!childNodes.front())
        throw std::logic_error("semantics has a null presentation wrapper");
    const auto &childBox = childNodes.front()->getBBox();
    box.w = childBox.w;
    box.h = childBox.h;
    box.d = childBox.d;
}

std::size_t CommonSemantics::breakCount() {
    if (node->isEmbellished()) {
        auto *core = dynamic_cast<CommonMo *>(coreMO());
        if (!core)
            throw std::logic_error("embellished semantics has no CommonMo core");
        return core->embellishedBreakCount();
    }
    if (childNodes.empty() || !childNodes.front())
        throw std::out_of_range("semantics has no presentation wrapper for breakCount");
    return childNodes.front()->breakCount();
}

SceneNodes CommonSemantics::emitScene(scene::Builder &builder,
                                      std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;

    auto containers = standardSceneNodes(builder, parents);
    if (!childNodes.empty()) {
        if (!childNodes.front())
            throw std::logic_error("semantics has a null presentation wrapper");
        (void)childNodes.front()->emitScene(builder, containers.content);
    }
    return containers.nodes;
}
} // namespace folia::math::output::common
