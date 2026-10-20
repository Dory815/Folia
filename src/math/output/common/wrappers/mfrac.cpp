// Native method bodies for MathJax 4.1.3 output/common/Wrappers/mfrac.ts.
module folia.math.output.common;
import :mfrac;
import :output_jax;
import folia.math.core.mml_tree.attributes;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
namespace {
using mml::AttributeNull;
using mml::AttributeValue;

bool truthy(AttributeValue const *value) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return false;
    if (auto const *boolean = std::get_if<bool>(value))
        return *boolean;
    if (auto const *number = std::get_if<double>(value))
        return *number != 0 && !std::isnan(*number);
    return !std::get<std::u16string>(*value).empty();
}

double sourceMax(double left, double right) {
    if (std::isnan(left) || std::isnan(right))
        return std::numeric_limits<double>::quiet_NaN();
    return std::max(left, right);
}

LengthInput lengthValue(AttributeValue const *value) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return std::u16string(u"medium");
    if (auto const *numeric = std::get_if<double>(value))
        return *numeric;
    if (auto const *text = std::get_if<std::u16string>(value))
        return *text;
    return std::u16string(std::get<bool>(*value) ? u"1" : u"0");
}

CommonWrapper &childAt(CommonMfrac const &fraction, std::size_t index) {
    if (index >= fraction.childNodes.size() || !fraction.childNodes[index])
        throw std::out_of_range("CommonMfrac requires numerator and denominator wrappers");
    return *fraction.childNodes[index];
}

BBox childBox(CommonMfrac const &fraction, std::size_t index) {
    return childAt(fraction, index).getOuterBBox();
}
} // namespace

CommonMfrac::CommonMfrac(CommonWrapperFactory &factory, mml::MmlNode &source,
                         CommonWrapper *parentValue)
    : CommonWrapper(factory, source, parentValue) {
}

void CommonMfrac::initializeSource() {
    CommonWrapper::initializeSource();
    if (!dynamic_cast<mml::MmlMfrac *>(node))
        throw std::invalid_argument("CommonMfrac requires an MmlMfrac node");
    const auto &properties = node->properties().tex;
    pad = properties.with_delimiters.value_or(false)
              ? 0.0
              : jax().face().parameters.required("nulldelimiterspace");
    if (truthy(node->attributes().lookup(u"bevelled"))) {
        const std::array<double, 1> height{getBevelData(isDisplay()).H};
        bevel = &createMo(u"/");
        bevel->node->attributes().set(u"symmetric", true);
        (void)bevel->canStretch(StretchDirection::vertical);
        bevel->getStretchedVariant(height, true);
    }
}

void CommonMfrac::getFractionBBox(BBox &box, bool display, double thickness) {
    const BBox numerator = childBox(*this, 0);
    const BBox denominator = childBox(*this, 1);
    const double axis = jax().face().parameters.required("axis_height");
    const auto tuv = getTUV(display, thickness);
    box.combine(numerator, 0, axis + tuv.T + sourceMax(numerator.d * numerator.rscale, tuv.u));
    box.combine(denominator, 0,
                axis - tuv.T - sourceMax(denominator.h * denominator.rscale, tuv.v));
    box.w += 2 * pad + 0.2;
}

CommonMfrac::TUV CommonMfrac::getTUV(bool display, double thickness) const {
    const auto &parameters = jax().face().parameters;
    const double axis = parameters.required("axis_height");
    const double T = (display ? 3.5 : 1.5) * thickness;
    return {T, (display ? parameters.required("num1") : parameters.required("num2")) - axis - T,
            (display ? parameters.required("denom1") : parameters.required("denom2")) + axis - T};
}

void CommonMfrac::getAtopBBox(BBox &box, bool display) {
    const auto data = getUVQ(display);
    box.combine(data.nbox, 0, data.u);
    box.combine(data.dbox, 0, -data.v);
    box.w += 2 * pad;
}

CommonMfrac::UVQ CommonMfrac::getUVQ(bool display) const {
    const BBox numerator = childBox(*this, 0);
    const BBox denominator = childBox(*this, 1);
    const auto &parameters = jax().face().parameters;
    double u = display ? parameters.required("num1") : parameters.required("num3");
    double v = display ? parameters.required("denom1") : parameters.required("denom2");
    const double p = (display ? 7.0 : 3.0) * parameters.required("rule_thickness");
    double q = u - numerator.d * numerator.scale - (denominator.h * denominator.scale - v);
    if (q < p) {
        u += (p - q) / 2;
        v += (p - q) / 2;
        q = p;
    }
    return {u, v, q, numerator, denominator};
}

void CommonMfrac::getBevelledBBox(BBox &box, bool display) {
    if (!bevel)
        throw std::logic_error("bevelled CommonMfrac has no slash wrapper");
    const auto data = getBevelData(display);
    const BBox slash = bevel->getOuterBBox();
    box.combine(data.nbox, 0, data.u);
    box.combine(slash, box.w - data.delta / 2, 0);
    box.combine(data.dbox, box.w - data.delta / 2, data.v);
}

CommonMfrac::BevelData CommonMfrac::getBevelData(bool display) const {
    const BBox numerator = childBox(*this, 0);
    const BBox denominator = childBox(*this, 1);
    const double delta = display ? 0.4 : 0.15;
    const double height = sourceMax(numerator.scale * (numerator.h + numerator.d),
                                    denominator.scale * (denominator.h + denominator.d)) +
                          2 * delta;
    const double axis = jax().face().parameters.required("axis_height");
    const double u = numerator.scale * (numerator.d - numerator.h) / 2 + axis + delta;
    const double v = denominator.scale * (denominator.d - denominator.h) / 2 + axis - delta;
    return {height, delta, u, v, numerator, denominator};
}

CommonMfrac::FractionGeometry CommonMfrac::makeFraction(bool display, double thickness) {
    const BBox numerator = childBox(*this, 0);
    const BBox denominator = childBox(*this, 1);
    const double axis = jax().face().parameters.required("axis_height");
    const double d = 0.1;
    const double width = sourceMax((numerator.L + numerator.w + numerator.R) * numerator.rscale,
                                   (denominator.L + denominator.w + denominator.R) * denominator.rscale);
    const double numeratorX = getAlignX(width, numerator, getChildAlign(0)) + d + pad;
    const double denominatorX = getAlignX(width, denominator, getChildAlign(1)) + d + pad;
    const auto tuv = getTUV(display, thickness);
    FractionGeometry result;
    result.numerator_origin = {numeratorX,
        axis + tuv.T + sourceMax(numerator.d * numerator.rscale, tuv.u)};
    result.denominator_origin = {denominatorX,
        axis - tuv.T - sourceMax(denominator.h * denominator.rscale, tuv.v)};
    result.rule = scene::Rect{pad, axis - thickness / 2, width + 2 * d, thickness};
    return result;
}

CommonMfrac::FractionGeometry CommonMfrac::makeAtop(bool display) {
    const BBox numerator = childBox(*this, 0);
    const BBox denominator = childBox(*this, 1);
    const double width = sourceMax((numerator.L + numerator.w + numerator.R) * numerator.rscale,
                                   (denominator.L + denominator.w + denominator.R) * denominator.rscale);
    const double numeratorX = getAlignX(width, numerator, getChildAlign(0)) + pad;
    const double denominatorX = getAlignX(width, denominator, getChildAlign(1)) + pad;
    const auto data = getUVQ(display);
    return {{numeratorX, data.u}, {denominatorX, -data.v}, {}, {}};
}

CommonMfrac::FractionGeometry CommonMfrac::makeBevelled(bool display) {
    if (!bevel) throw std::logic_error("bevelled CommonMfrac has no slash wrapper");
    const auto data = getBevelData(display);
    const BBox slash = bevel->getOuterBBox();
    const double width = (data.nbox.L + data.nbox.w + data.nbox.R) * data.nbox.rscale;
    const double slashX = width - data.delta / 2;
    const double denominatorX = width + slash.w + data.dbox.L * data.dbox.rscale - data.delta;
    return {{data.nbox.L * data.nbox.rscale, data.u}, {denominatorX, data.v},
            scene::Point{slashX, 0}, {}};
}

CommonMfrac::FractionGeometry CommonMfrac::layoutGeometry() {
    // Keep the actual wrapper's ordinary BBox calculation and child wrappers as
    // the sole layout state; this returns only the additional SVG child geometry.
    (void)getBBox();
    const auto *bevelledValue = node->attributes().lookup(u"bevelled");
    if (truthy(bevelledValue)) return makeBevelled(isDisplay());
    const double thickness = length2em(
        lengthValue(node->attributes().lookup(u"linethickness")), 0.06);
    if (thickness == 0) return makeAtop(isDisplay());
    return makeFraction(isDisplay(), thickness);
}

SceneNodes CommonMfrac::emitScene(scene::Builder &builder, std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;
    auto containers = standardSceneNodes(builder, parents);
    auto &nodes = containers.nodes;
    auto const &content = containers.content;
    const auto geometry = layoutGeometry();
    if (truthy(node->attributes().lookup(u"bevelled"))) {
        const auto numerator = childNodes.at(0)->emitScene(builder, content);
        const auto slash = bevel->emitScene(builder, content);
        const auto denominator = childNodes.at(1)->emitScene(builder, content);
        childNodes.at(0)->placeScene(builder, numerator, geometry.numerator_origin.x,
                                     geometry.numerator_origin.y);
        bevel->placeScene(builder, slash, geometry.slash_origin->x, geometry.slash_origin->y);
        childNodes.at(1)->placeScene(builder, denominator, geometry.denominator_origin.x,
                                     geometry.denominator_origin.y);
    } else {
        const auto numerator = childNodes.at(0)->emitScene(builder, content);
        const auto denominator = childNodes.at(1)->emitScene(builder, content);
        childNodes.at(0)->placeScene(builder, numerator, geometry.numerator_origin.x,
                                     geometry.numerator_origin.y);
        childNodes.at(1)->placeScene(builder, denominator, geometry.denominator_origin.x,
                                     geometry.denominator_origin.y);
        if (geometry.rule) {
            scene::Rule rule;
            rule.bounds = *geometry.rule;
            rule.role = scene::ElementRole::fraction_rule;
            if (!nodes.empty() && nodes.front())
                builder.add_primitive(*nodes.front(), rule);
        }
    }
    return nodes;
}

bool CommonMfrac::isDisplay() const {
    const auto *level = node->attributes().lookup(u"scriptlevel");
    const auto *numericLevel = level ? std::get_if<double>(level) : nullptr;
    return truthy(node->attributes().lookup(u"displaystyle")) && numericLevel &&
           *numericLevel == 0.0;
}

void CommonMfrac::computeBBox(BBox &box, bool recompute) {
    box.empty();
    const auto &attributes = node->attributes();
    const bool bevelled = truthy(attributes.lookup(u"bevelled"));
    const bool display = isDisplay();
    std::optional<double> width;
    if (bevelled) {
        getBevelledBBox(box, display);
    } else {
        width = -2 * pad;
        const double thickness = length2em(lengthValue(attributes.lookup(u"linethickness")), 0.06);
        if (thickness == 0) {
            getAtopBBox(box, display);
        } else {
            getFractionBBox(box, display, thickness);
            *width -= 0.2;
        }
        *width += box.w;
    }
    box.clean();
    setChildPWidths(recompute, width);
}

bool CommonMfrac::canStretch(StretchDirection) {
    return false;
}

std::string CommonMfrac::getChildAlign(std::size_t index) const {
    if (truthy(node->attributes().lookup(u"bevelled")))
        return "left";
    std::u16string_view name;
    if (index == 0)
        name = u"numalign";
    else if (index == 1)
        name = u"denomalign";
    else
        return "undefined";
    const auto *value = node->attributes().lookup(name);
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr)
        return utf16::encode(*text);
    return {};
}

double CommonMfrac::getWrapWidth(std::size_t index) {
    if (truthy(node->attributes().lookup(u"bevelled")))
        return childBox(*this, index).w;
    const double width = getBBox().w;
    const double thickness = length2em(lengthValue(node->attributes().lookup(u"linethickness")));
    return width - (thickness ? 0.2 : 0.0) - 2 * pad;
}
} // namespace folia::math::output::common
