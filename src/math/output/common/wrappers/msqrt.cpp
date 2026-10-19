/*************************************************************
 * Native CommonMsqrt algorithms from MathJax 4.1.3.
 *************************************************************/
module folia.math.output.common;
import :msqrt;
import :wrapper;
import :mo;
import :output_jax;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.util.bbox;
import std;

namespace folia::math::output::common {
namespace {
using mml::AttributeNull;
using mml::AttributeValue;

bool sourceTruthy(AttributeValue const *value) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return false;
    if (auto const *boolean = std::get_if<bool>(value))
        return *boolean;
    if (auto const *number = std::get_if<double>(value))
        return *number != 0 && !std::isnan(*number);
    return !std::get<std::u16string>(*value).empty();
}

FontFaceData const &face(CommonWrapper const &wrapper) {
    auto const &jax = wrapper.jax();
    return jax.fonts().face(jax.face().name);
}

double sourceMax(double left, double right) {
    return std::isnan(left) || std::isnan(right) ? std::numeric_limits<double>::quiet_NaN()
                                                 : std::max(left, right);
}

CommonWrapper &requiredChild(CommonWrapper const &wrapper, std::size_t index) {
    if (index >= wrapper.childNodes.size() || !wrapper.childNodes[index])
        throw std::logic_error("radical wrapper is missing a required child");
    return *wrapper.childNodes[index];
}
} // namespace

CommonMsqrt::CommonMsqrt(CommonWrapperFactory &factory, mml::MmlNode &node, CommonWrapper *parent)
    : CommonWrapper(factory, node, parent) {
}

void CommonMsqrt::initializeSource() {
    CommonWrapper::initializeSource();
    surd = &createMo(u"\u221A");
    (void)surd->canStretch(StretchDirection::vertical);
    getStretchedSurd();
}

void CommonMsqrt::combineRootBBox(BBox &, BBox const &, double) const {
}

std::array<double, 2> CommonMsqrt::getPQ(BBox const &surdBox) const {
    const auto &parameters = face(*this).parameters;
    const double thickness = parameters.required("rule_thickness");
    const double surdHeight = parameters.required("surd_height");
    const double p = sourceTruthy(node->attributes().lookup(u"displaystyle"))
                         ? parameters.required("x_height")
                         : thickness;
    const double q = surdBox.h + surdBox.d > surdH
                         ? (surdBox.h + surdBox.d - (surdH - thickness - surdHeight - p / 2)) / 2
                         : surdHeight + p / 4;
    return {p, q};
}

std::array<double, 3> CommonMsqrt::getRootDimens(BBox const &, double) const {
    return {0, 0, 0};
}

double CommonMsqrt::rootWidth() const {
    return 1.25;
}

void CommonMsqrt::getStretchedSurd() {
    if (!surd)
        throw std::logic_error("radical operator wrapper has not been initialized");
    const auto &parameters = face(*this).parameters;
    const double thickness = parameters.required("rule_thickness");
    const double height = parameters.required("surd_height");
    const double p = sourceTruthy(node->attributes().lookup(u"displaystyle"))
                         ? parameters.required("x_height")
                         : thickness;
    const auto baseBox = requiredChild(*this, base()).getOuterBBox();
    surdH = baseBox.h + baseBox.d + thickness + height + p / 4;
    const std::array<double, 2> requested{surdH - baseBox.d, baseBox.d};
    surd->getStretchedVariant(requested, true);
}

void CommonMsqrt::computeBBox(BBox &target, bool recompute) {
    if (!surd)
        throw std::logic_error("radical operator wrapper has not been initialized");
    target.empty();
    const BBox &surdBox = surd->getBBox();
    const auto childBase = requiredChild(*this, base()).getOuterBBox();
    const BBox baseBox(BBoxData{.w = childBase.w, .h = childBase.h, .d = childBase.d});
    const double q = getPQ(surdBox)[1];
    const auto &parameters = face(*this).parameters;
    const double thickness = parameters.required("rule_thickness");
    const double height = parameters.required("surd_height");
    const double wholeHeight = baseBox.h + q + thickness;
    const auto dimensions = getRootDimens(surdBox, wholeHeight);
    const double x = dimensions[0];
    target.h = wholeHeight + height;
    combineRootBBox(target, surdBox, wholeHeight);
    target.combine(surdBox, x, wholeHeight - surdBox.h);
    target.combine(baseBox, x + surdBox.w, 0);
    target.clean();
    (void)setChildPWidths(recompute);
}

void CommonMsqrt::invalidateBBox(bool bubble) {
    CommonWrapper::invalidateBBox(bubble);
    if (!surd)
        throw std::logic_error("radical operator wrapper has not been initialized");
    (void)requiredChild(*surd, 0).invalidateBBox();
}
} // namespace folia::math::output::common
