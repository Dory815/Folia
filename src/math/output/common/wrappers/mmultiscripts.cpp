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

std::array<double, 2> CommonMmultiscripts::getCombinedUV() {
    const auto sub = combinePrePost(scriptData_.sub, scriptData_.psub);
    const auto sup = combinePrePost(scriptData_.sup, scriptData_.psup);
    const auto uvq = getUVQ(&sub, &sup);
    return {uvq[0], uvq[1]};
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
    const BBox sub = subValue ? *subValue : subChild()->getOuterBBox();
    const BBox sup = supValue ? *supValue : supChild()->getOuterBBox();
    if (UVQ)
        return *UVQ;
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
} // namespace folia::math::output::common
