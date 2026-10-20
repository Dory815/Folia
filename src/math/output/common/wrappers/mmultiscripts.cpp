/*************************************************************
 * Candidate typed port of MathJax 4.1.3 output/common/Wrappers/mmultiscripts.ts.
 *************************************************************/
module folia.math.output.common;
import :mmultiscripts;
import :msubsup;
import :scriptbase;
import :wrapper;
import folia.math.core.mml_tree.node_kind;
import std;

namespace folia::math::output::common {
namespace {
double parameter(CommonWrapper const &wrapper, std::string_view name) {
    return wrapper.jax().face().parameters.required(name);
}
} // namespace

CommonMmultiscripts::CommonMmultiscripts(CommonWrapperFactory &factory, mml::MmlNode &node,
                                         CommonWrapper *parent)
    : CommonMsubsup(factory, node, parent) {
}

void CommonMmultiscripts::initializeSource() {
    CommonMsubsup::initializeSource();
    getScriptData();
}

BBox CommonMmultiscripts::combinePrePost(const BBox &pre, const BBox &post) const {
    BBox result = pre;
    result.combine(post);
    return result;
}

void CommonMmultiscripts::getScriptData() {
    scriptData_ = {};
    scriptData_.sub.empty();
    scriptData_.sup.empty();
    scriptData_.psub.empty();
    scriptData_.psup.empty();
    auto lists = getScriptBBoxLists();
    combineBBoxLists(scriptData_.sub, scriptData_.sup, lists.subList, lists.supList);
    combineBBoxLists(scriptData_.psub, scriptData_.psup, lists.psubList, lists.psupList);
    if (!lists.base.empty())
        scriptData_.base = lists.base.front();
    scriptData_.numPrescripts = lists.psubList.size();
    scriptData_.numScripts = lists.subList.size();
}

ScriptLists CommonMmultiscripts::getScriptBBoxLists() {
    ScriptLists lists;
    enum class Side { base, sub, sup, psub, psup };
    Side side = Side::base;
    for (auto *child : childNodes) {
        if (child->node->kind() == mml::NodeKind::mprescripts) {
            side = Side::psub;
            continue;
        }
        const auto box = child->getOuterBBox();
        switch (side) {
        case Side::base:
            lists.base.push_back(box);
            side = Side::sub;
            break;
        case Side::sub:
            lists.subList.push_back(box);
            side = Side::sup;
            break;
        case Side::sup:
            lists.supList.push_back(box);
            side = Side::sub;
            break;
        case Side::psub:
            lists.psubList.push_back(box);
            side = Side::psup;
            break;
        case Side::psup:
            lists.psupList.push_back(box);
            side = Side::psub;
            break;
        }
    }
    firstPrescript_ = lists.subList.size() + lists.supList.size() + 2;
    padLists(lists.subList, lists.supList);
    padLists(lists.psubList, lists.psupList);
    return lists;
}

void CommonMmultiscripts::padLists(std::vector<BBox> &first, std::vector<BBox> &second) const {
    if (first.size() > second.size())
        second.push_back(BBox::makeEmpty());
}

void CommonMmultiscripts::combineBBoxLists(BBox &sub, BBox &sup, const std::vector<BBox> &subs,
                                           const std::vector<BBox> &sups) const {
    for (std::size_t i = 0; i < subs.size(); ++i) {
        if (i >= sups.size())
            throw std::logic_error("multiscript side lists have different lengths");
        const auto a = getScaledWHD(subs[i]);
        const auto b = getScaledWHD(sups[i]);
        const double width = std::max(a[0], b[0]);
        sub.w += width;
        sup.w += width;
        sub.h = std::max(sub.h, a[1]);
        sub.d = std::max(sub.d, a[2]);
        sup.h = std::max(sup.h, b[1]);
        sup.d = std::max(sup.d, b[2]);
    }
}

std::array<double, 3> CommonMmultiscripts::getScaledWHD(const BBox &box) const {
    return {box.w * box.rscale, box.h * box.rscale, box.d * box.rscale};
}

std::array<double, 3> CommonMmultiscripts::getCombinedUV() {
    const auto sub = combinePrePost(scriptData_.sub, scriptData_.psub);
    const auto sup = combinePrePost(scriptData_.sup, scriptData_.psup);
    return getUVQ(&sub, &sup);
}

BBox CommonMmultiscripts::addPrescripts(BBox &box, double u, double v) const {
    if (scriptData_.numPrescripts) {
        const double space = parameter(*this, "scriptspace");
        box.combine(scriptData_.psup, space, u);
        box.combine(scriptData_.psub, space, v);
    }
    return box;
}

BBox CommonMmultiscripts::addPostscripts(BBox &box, double u, double v) const {
    if (scriptData_.numScripts) {
        const double x = box.w;
        box.combine(scriptData_.sup, x, u);
        box.combine(scriptData_.sub, x, v);
        box.w += parameter(*this, "scriptspace");
    }
    return box;
}

std::array<double, 3> CommonMmultiscripts::getUVQ(const BBox *subValue, const BBox *supValue) {
    if (UVQ)
        return *UVQ;
    if (!subValue || !supValue)
        throw std::invalid_argument("multiscripts UVQ requires combined script boxes");
    const BBox& sub = *subValue;
    const BBox& sup = *supValue;
    if (sub.w == 0)
        UVQ = std::array<double, 3>{getU(), 0, 0};
    else if (sup.w == 0)
        UVQ = std::array<double, 3>{-getV(), 0, 0};
    else
        UVQ = CommonMsubsup::getUVQ(&sub, &sup);
    return *UVQ;
}

BBox CommonMmultiscripts::appendScripts(BBox &box) const {
    box.empty();
    const auto uv = const_cast<CommonMmultiscripts *>(this)->getCombinedUV();
    addPrescripts(box, uv[0], uv[1]);
    if (!scriptData_.base)
        throw std::logic_error("multiscripts wrapper has no base box");
    box.append(*scriptData_.base);
    addPostscripts(box, uv[0], uv[1]);
    box.clean();
    return box;
}

std::shared_ptr<LineBBox> CommonMmultiscripts::computeLineBBox(std::size_t index) {
    auto *base = baseChild();
    if (!base)
        throw std::logic_error("multiscripts wrapper has no base child");
    const auto count = base->breakCount();
    auto childBox = base->getLineBBox(index)->copy();
    const auto uv = getCombinedUV();
    LineBBox box = childBox;
    if (index == 0) {
        BBox pre = BBox::zero();
        auto prescripts = addPrescripts(pre, uv[0], uv[1]);
        box = LineBBox::from(prescripts, linebreakOptions().lineleading);
        box.append(childBox);
        addLeftBorders(box);
        box.L = bbox.L;
    } else if (index == count) {
        addPostscripts(box, uv[0], uv[1]);
        addRightBorders(box);
        box.R = bbox.R;
    }
    addMiddleBorders(box);
    return std::make_shared<LineBBox>(std::move(box));
}

SceneNodes CommonMmultiscripts::emitScene(scene::Builder &builder,
                                          std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;

    auto containers = standardSceneNodes(builder, parents);
    auto &nodes = containers.nodes;
    const auto &content = containers.content;
    if (nodes.empty())
        return nodes;

    const auto uv = getCombinedUV();
    std::string_view preAlign = "right";
    std::string_view postAlign = "left";
    if (const auto alignment = node->properties().tex.script_alignment) {
        switch (*alignment) {
        case mml::HorizontalAlignment::left:
            preAlign = postAlign = "left";
            break;
        case mml::HorizontalAlignment::center:
            preAlign = postAlign = "center";
            break;
        case mml::HorizontalAlignment::right:
            preAlign = postAlign = "right";
            break;
        }
    }

    auto addScripts = [&](scene::NodeId owner, double x, std::size_t first, std::size_t count,
                          std::string_view alignment) {
        const auto supRow =
            builder.add_group(owner, {}, {}, {}, {}, {}, scene::ElementRole::anonymous_group);
        const auto subRow =
            builder.add_group(owner, {}, {}, {}, {}, {}, scene::ElementRole::anonymous_group);
        const std::array<scene::NodeId, 1> supParent{supRow};
        const std::array<scene::NodeId, 1> subParent{subRow};
        placeScene(builder, nodes, x, uv[0], supRow);
        placeScene(builder, nodes, x, uv[1], subRow);

        double dx = 0;
        const auto alignScript = [alignment](double scriptWidth, double pairWidth) {
            if (alignment == "left")
                return 0.0;
            if (alignment == "center")
                return (pairWidth - scriptWidth) / 2;
            if (alignment == "right")
                return pairWidth - scriptWidth;
            return 0.0;
        };
        for (std::size_t pair = 0; pair < count; ++pair) {
            const auto subIndex = first + pair * 2;
            const auto supIndex = subIndex + 1;
            if (subIndex >= childNodes.size() || supIndex >= childNodes.size() ||
                !childNodes[subIndex] || !childNodes[supIndex])
                throw std::logic_error("multiscripts wrapper has an incomplete script pair");
            auto *sub = childNodes[subIndex];
            auto *sup = childNodes[supIndex];
            const BBox subbox = sub->getOuterBBox();
            const BBox supbox = sup->getOuterBBox();
            const double subWidth = subbox.w * subbox.rscale;
            const double supWidth = supbox.w * supbox.rscale;
            const double width = std::max(subWidth, supWidth);
            const auto subNodes = sub->emitScene(builder, subParent);
            const auto supNodes = sup->emitScene(builder, supParent);
            sub->placeScene(builder, subNodes, dx + alignScript(subWidth, width), 0);
            sup->placeScene(builder, supNodes, dx + alignScript(supWidth, width), 0);
            dx += width;
        }
        return x + dx;
    };

    double x = 0;
    if (scriptData_.numPrescripts) {
        if (!nodes.front())
            throw std::logic_error("multiscripts pre-script container segment is absent");
        x = addScripts(*nodes.front(), parameter(*this, "scriptspace"), firstPrescript_,
                       scriptData_.numPrescripts, preAlign);
    }

    auto *base = baseChild();
    if (!base)
        throw std::logic_error("multiscripts wrapper has no base child");
    const auto baseNodes = base->emitScene(builder, content);
    base->placeScene(builder, baseNodes, x, 0);
    if (base->breakCount())
        x = 0;
    x += base->getLineBBox(base->breakCount())->w;

    if (scriptData_.numScripts) {
        if (!nodes.back())
            throw std::logic_error("multiscripts post-script container segment is absent");
        addScripts(*nodes.back(), x, 1, scriptData_.numScripts, postAlign);
    }
    return nodes;
}
} // namespace folia::math::output::common
