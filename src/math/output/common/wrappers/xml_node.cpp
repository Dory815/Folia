/*************************************************************
 * Native mathematical services from MathJax 4.1.3
 * output/common/Wrappers/XmlNode.ts.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :xml_node;
import :wrapper;
import :output_jax;
import folia.math.core.mml_tree.html_adaptor;
import folia.math.core.mml_tree.mml_document;
import folia.math.util.string;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
namespace {

std::u16string decode(std::string_view value) {
    return utf16::decode(value).text;
}

std::string fixed(double value) {
    if (std::abs(value) < .0006)
        return "0";
    auto result = lengths::format_decimal(value, 3);
    if (result.find('.') != std::string::npos) {
        while (result.ends_with('0'))
            result.pop_back();
        if (result.ends_with('.'))
            result.pop_back();
    }
    return result;
}

void setStyles(AbstractDOMAdaptor &adaptor, DOMNodeRef const &node,
               styles::StyleList const &styles) {
    for (const auto &[name, value] : styles)
        adaptor.setStyle(node, decode(name), value);
}

DOMNodeRef makeElement(AbstractDOMAdaptor &adaptor, std::u16string_view kind,
                       std::span<DOMNodeRef const> children = {},
                       styles::StyleList const &styles = {},
                       std::optional<std::u16string_view> variant = {}) {
    auto result = adaptor.node(kind, std::span<AttributeData const>{}, children);
    if (!result)
        throw std::runtime_error("DOM adaptor failed to create an HTML measurement node");
    setStyles(adaptor, result, styles);
    if (variant)
        adaptor.setAttribute(result, u"variant", *variant);
    return result;
}

struct MeasurementCleanup {
    AbstractDOMAdaptor &adaptor;
    DOMNodeRef container;
    DOMNodeRef measurement;
    bool containerMayBeAttached = false;
    bool measurementMayBeAttached = false;

    ~MeasurementCleanup() noexcept {
        if (containerMayBeAttached) {
            try {
                (void)adaptor.remove(container);
            } catch (...) {
            }
        }
        if (measurementMayBeAttached) {
            try {
                (void)adaptor.remove(measurement);
            } catch (...) {
            }
        }
    }
};

std::string_view htmlHdwName(CommonOptions::HtmlHDW value) {
    switch (value) {
    case CommonOptions::HtmlHDW::automatic:
        return "automatic";
    case CommonOptions::HtmlHDW::use:
        return "use";
    case CommonOptions::HtmlHDW::force:
        return "force";
    case CommonOptions::HtmlHDW::ignore:
        return "ignore";
    }
    return "ignore";
}

} // namespace

CommonXmlNode::CommonXmlNode(CommonWrapperFactory &factory, mml::MmlNode &source,
                             CommonWrapper *parentValue)
    : CommonWrapper(factory, source, parentValue) {}

void CommonXmlNode::initializeSource() {
    CommonWrapper::initializeSource();
    rscale = getRScale();
}

void CommonXmlNode::getStyles() {}
void CommonXmlNode::getScale() {}
void CommonXmlNode::getVariant() {}

std::optional<std::u16string> CommonXmlNode::getHDW(DOMNodeRef xml, std::string_view use,
                                                    std::optional<std::string_view> force) {
    if (!xml)
        throw std::invalid_argument("CommonXmlNode::getHDW requires an XML DOM node");
    auto const &service = jax().htmlMeasurement();
    auto hdw = service.adaptor.getAttribute(xml, u"data-mjx-hdw");
    if (!hdw || hdw->empty())
        return {};
    const auto forceValue = force.value_or(use);
    const auto option = htmlHdwName(jax().options().htmlHDW);
    if (option == use || option == forceValue)
        return hdw;
    return {};
}

UnknownBBox CommonXmlNode::splitHDW(std::u16string_view hdw) const {
    const double scale = 1 / metrics().scale;
    const auto values = string_util::split(hdw);
    std::array<double, 3> result{};
    for (std::size_t i = 0; i < result.size(); ++i) {
        const auto value =
            i < values.size() && !values[i].empty() ? values[i] : std::u16string(u"0");
        result[i] = length2em(LengthInput(value)) * scale;
    }
    return {.w = result[2], .h = result[0], .d = result[1]};
}

styles::StyleList CommonXmlNode::getFontStyles() const {
    if (!parent)
        throw std::logic_error("CommonXmlNode requires a Common parent for font styles");
    const auto &m = metrics();
    std::u16string family;
    if (parent->styles)
        family = parent->styles->get(u"font-family");
    if (family.empty() && !m.family.empty())
        family = decode(m.family);
    if (family.empty()) {
        const auto &service = jax().htmlMeasurement();
        family = service.adaptor.fontFamily(service.parent);
        if (family.empty())
            family = u"initial";
    }
    return {{"font-family", std::move(family)}, {"font-size", decode(fixed(m.em * rscale) + "px")}};
}

DOMNodeRef CommonXmlNode::cloneXML() const {
    auto const *xmlNode = dynamic_cast<mml::XMLNode const *>(node);
    if (!xmlNode)
        throw std::logic_error("CommonXmlNode requires an XMLNode");
    auto const &owner = xmlNode->xmlAdaptor();
    auto const *htmlAdaptor = dynamic_cast<mml::HtmlAdaptor const *>(owner.get());
    if (!htmlAdaptor)
        throw std::logic_error("XMLNode owner cannot clone its payload into the DOM");
    auto result = htmlAdaptor->cloneDOM(xmlNode->getXML());
    if (!result)
        throw std::runtime_error("XMLNode owner returned no DOM clone");
    return result;
}

DOMNodeRef CommonXmlNode::getHTML() {
    auto &jaxValue = jax();
    auto &adaptor = jaxValue.htmlMeasurement().adaptor;
    auto html = cloneXML();
    auto styles = getFontStyles();
    const auto hdw = getHDW(html, "force");
    if (hdw || jaxValue.options().scale != 1)
        html = addHDW(std::move(html), styles);
    if (!parent)
        throw std::logic_error("CommonXmlNode requires a Common parent for HTML variant");
    const std::array<DOMNodeRef, 1> children{html};
    const auto variant = decode(parent->variant);
    auto result = makeElement(adaptor, u"mjx-html", children, styles, std::u16string_view(variant));
    return result;
}

UnknownBBox CommonXmlNode::measureXmlNode(DOMNodeRef xml) {
    if (!xml)
        throw std::invalid_argument("CommonXmlNode::measureXmlNode requires an XML DOM node");
    auto &output = jax();
    auto const &service = output.htmlMeasurement();
    auto &adaptor = service.adaptor;

    const std::array<DOMNodeRef, 1> xmlChildren{adaptor.clone(xml, true)};
    auto content =
        makeElement(adaptor, u"mjx-xml-block", xmlChildren, {{"display", u"inline-block"}});
    auto base = makeElement(adaptor, u"mjx-baseline", {},
                            {{"display", u"inline-block"}, {"width", u"0"}, {"height", u"0"}});
    auto measurement = makeElement(adaptor, u"mjx-measure-xml", {}, getFontStyles());
    (void)adaptor.append(measurement, base);
    (void)adaptor.append(measurement, content);

    MeasurementCleanup cleanup{adaptor, service.container, measurement};
    cleanup.containerMayBeAttached = true;
    (void)adaptor.append(service.parent, service.container);
    cleanup.measurementMayBeAttached = true;
    (void)adaptor.append(service.container, measurement);

    const auto contentBox = adaptor.nodeBBox(content);
    const auto baseBox = adaptor.nodeBBox(base);
    const double em = metrics().em * metrics().scale * rscale;
    const double width = (contentBox.right - contentBox.left) / em;
    const double height = (baseBox.top - contentBox.top) / em;
    const double depth = (contentBox.bottom - contentBox.top) / em - height;
    return {.w = width, .h = height, .d = depth};
}

void CommonXmlNode::computeBBox(BBox &target, bool) {
    auto *xmlNode = dynamic_cast<mml::XMLNode *>(node);
    if (!xmlNode)
        throw std::logic_error("CommonXmlNode requires an XMLNode");
    target.empty();
    const auto xml = cloneXML();
    const auto hdw = getHDW(xml, "use", "force");
    const auto measured = hdw ? splitHDW(*hdw) : measureXmlNode(xml);
    target.w = measured.w;
    target.h = measured.h;
    target.d = measured.d;
}

} // namespace folia::math::output::common
