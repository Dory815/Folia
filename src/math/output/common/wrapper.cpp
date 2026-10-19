/*************************************************************
 * Native mathematical services from MathJax 4.1.3 output/common/Wrapper.ts.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :wrapper;
import :wrapper_factory;
import :output_jax;
import :mo;
import :mrow;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
namespace {
using mml::AttributeNull;
using mml::AttributeValue;

bool truthy(AttributeValue const *value) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return false;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean;
    if (const auto *number = std::get_if<double>(value))
        return *number != 0 && !std::isnan(*number);
    return !std::get<std::u16string>(*value).empty();
}

std::u16string_view text(AttributeValue const *value) {
    if (value) {
        if (const auto *string = std::get_if<std::u16string>(value))
            return *string;
    }
    return {};
}

double number(AttributeValue const *value, double fallback = 0) {
    if (!value)
        return fallback;
    if (std::holds_alternative<AttributeNull>(*value))
        return 0;
    if (const auto *numeric = std::get_if<double>(value))
        return *numeric;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean ? 1 : 0;
    return js_input::number(std::get<std::u16string>(*value));
}

std::string utf8(std::u16string_view s) {
    return utf16::encode(s);
}

std::u16string decodeUtf8(std::string_view value) {
    return utf16::decode(value).text;
}

std::string valueString(AttributeValue const *value, std::string fallback = {}) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return fallback;
    if (const auto *string = std::get_if<std::u16string>(value))
        return utf8(*string);
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean ? "true" : "false";
    return js_number::toString(std::get<double>(*value));
}

LengthInput input(std::u16string_view value) {
    return std::u16string(value);
}

LengthInput input(AttributeValue const *value, std::u16string_view fallback = {}) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return std::u16string(fallback);
    if (const auto *string = std::get_if<std::u16string>(value))
        return *string;
    if (const auto *numeric = std::get_if<double>(value))
        return *numeric;
    return std::u16string(std::get<bool>(*value) ? u"true" : u"false");
}

std::string sideKey(std::string_view prefix, std::string_view side, std::string_view suffix = {}) {
    std::string result(prefix);
    result.append(side);
    result.append(suffix);
    return result;
}

bool isZeroIndent(std::u16string_view value) {
    while (!value.empty() && js_input::isWhitespace(value.front()))
        value.remove_prefix(1);
    while (!value.empty() && js_input::isWhitespace(value.back()))
        value.remove_suffix(1);
    if (value.empty() || value.front() != u'0')
        return false;
    return std::all_of(value.begin() + 1, value.end(),
                       [](char16_t c) { return c >= u'a' && c <= u'z'; });
}

constexpr double smallMathSpace = 2.0 / 18.0;
constexpr double operatorMathSpace = 5.0 / 18.0;

double mathMax(double a, double b) {
    if (std::isnan(a) || std::isnan(b))
        return std::numeric_limits<double>::quiet_NaN();
    return std::max(a, b);
}

double mathMin(double a, double b) {
    if (std::isnan(a) || std::isnan(b))
        return std::numeric_limits<double>::quiet_NaN();
    return std::min(a, b);
}
} // namespace

CommonWrapper::CommonWrapper(CommonWrapperFactory &factoryValue, mml::MmlNode &source,
                             CommonWrapper *parentValue)
    : tree::AbstractWrapper<CommonWrapper, mml::MmlNode>(factoryValue, source),
      parent(parentValue) {
}

CommonWrapperFactory &CommonWrapper::factory() const {
    return dynamic_cast<CommonWrapperFactory &>(factory_);
}

CommonOutputJax &CommonWrapper::jax() const {
    return factory().jax();
}

ExtendedMetrics const &CommonWrapper::metrics() const {
    return jax().currentMetrics();
}

double CommonWrapper::containerWidth() const {
    return parent ? parent->containerWidth() : jax().containerWidth();
}

Linebreaks &CommonWrapper::linebreaks() const {
    return jax().linebreaks();
}

LinebreakOptions const &CommonWrapper::linebreakOptions() const {
    return jax().currentLinebreakOptions();
}

void CommonWrapper::initializeSource() {
    bbox = BBox::zero();
    bboxComputed = false;
    _breakCount = -1;
    getStyles();
    getStyleData();
    getVariant();
    getScale();
    getSpace();
    for (auto *child : node->children()) {
        auto &wrapped = wrapNode(*child);
        childNodes.push_back(&wrapped);
        if (!wrapped.bbox.pwidth.empty() &&
            (node->notParent() || node->kind() == mml::NodeKind::math))
            bbox.pwidth = std::string(BBox::fullWidth);
    }
}

void CommonWrapper::getStyles() {
    // Source reads only an explicit style attribute and returns before parsing
    // an empty string.
    auto const *raw = node->attributes().own(u"style");
    if (!raw || !std::holds_alternative<std::u16string>(*raw) ||
        std::get<std::u16string>(*raw).empty())
        return;
    styles.emplace(std::get<std::u16string>(*raw));
    const auto &removals = factory().classData(node->kind()).removeStyles;
    for (const auto &name : removals) {
        const auto unicodeName = decodeUtf8(name);
        const auto removed = styles->get(unicodeName);
        if (removed.empty())
            continue;
        removedStyles.insert_or_assign(name, utf8(removed));
        styles->set(unicodeName, styles::StyleValue(u""));
    }
}

void CommonWrapper::getStyleData() {
    if (!styles)
        return;
    StyleData data;
    bool hasPadding = false;
    bool hasBorder = false;
    bool hasMargin = false;
    for (const auto &side : BBox::boxSides) {
        const auto index = static_cast<std::size_t>(side.index);
        const auto suffix = side.name;
        const auto borderWidth = styles->get(decodeUtf8(sideKey("border", suffix, "Width")));
        if (!borderWidth.empty()) {
            hasBorder = true;
            if (!data.border)
                data.border.emplace();
            data.border->width[index] = mathMax(0.0, length2em(input(borderWidth)));
            const auto borderStyle = styles->get(decodeUtf8(sideKey("border", suffix, "Style")));
            const auto borderColor = styles->get(decodeUtf8(sideKey("border", suffix, "Color")));
            data.border->style[index] = borderStyle.empty() ? "solid" : utf8(borderStyle);
            data.border->color[index] = utf8(borderColor);
        }
        const auto padding = styles->get(decodeUtf8(sideKey("padding", suffix)));
        if (!padding.empty()) {
            hasPadding = true;
            data.padding[index] = mathMax(0.0, length2em(input(padding)));
        }
        const auto margin = styles->get(decodeUtf8(sideKey("margin", suffix)));
        if (!margin.empty()) {
            hasMargin = true;
            data.margin[index] = length2em(input(margin));
        }
    }
    if (hasPadding || hasBorder || hasMargin)
        styleData = std::move(data);
}

void CommonWrapper::getVariant() {
    if (!node->isToken())
        return;
    const auto &attributes = node->attributes();
    if (attributes.own(u"mathvariant")) {
        auto selected = valueString(attributes.lookup(u"mathvariant"), "null");
        if (!jax().face().variants.contains(selected)) {
            jax().recordUnknownMathVariant(node->kind(), selected);
            variant = "normal";
        } else {
            variant = std::move(selected);
        }
        return;
    }
    std::string family;
    std::string weight;
    std::string style;
    if (auto found = removedStyles.find("fontFamily"); found != removedStyles.end())
        family = found->second;
    if (auto found = removedStyles.find("fontWeight"); found != removedStyles.end())
        weight = found->second;
    if (auto found = removedStyles.find("fontStyle"); found != removedStyles.end())
        style = found->second;
    if (const auto *attribute = attributes.lookup(u"fontfamily"); truthy(attribute))
        family = valueString(attribute);
    if (const auto *attribute = attributes.lookup(u"fontweight"); truthy(attribute))
        weight = valueString(attribute);
    if (const auto *attribute = attributes.lookup(u"fontstyle"); truthy(attribute))
        style = valueString(attribute);

    if (!family.empty() && family != "undefined") {
        const bool numericWeight =
            !weight.empty() && std::all_of(weight.begin(), weight.end(),
                                           [](unsigned char c) { return c >= '0' && c <= '9'; });
        if (numericWeight) {
            const auto numeric = js_input::parseInt(decodeUtf8(weight));
            weight = numeric > 600 ? "bold" : "normal";
        }
        variant = explicitVariant(family, weight, style);
        return;
    }
    variant = valueString(attributes.lookup(u"mathvariant"), "italic");
    // MmlNode properties are the native, already-resolved counterpart of
    // Source getProperty("variantForm"); this is a property lookup, not an
    // additional wrapper-side state.
    if (node->properties().tex.variant_form.value_or(false))
        variant = "-tex-variant";

    const auto &classData = factory().classData(node->kind());
    if (!weight.empty()) {
        if (const auto outer = classData.boldVariants.find(weight);
            outer != classData.boldVariants.end()) {
            if (const auto inner = outer->second.find(variant); inner != outer->second.end())
                variant = inner->second;
        }
    }
    if (!style.empty()) {
        if (const auto outer = classData.italicVariants.find(style);
            outer != classData.italicVariants.end()) {
            if (const auto inner = outer->second.find(variant); inner != outer->second.end())
                variant = inner->second;
        }
    }
}

std::string CommonWrapper::explicitVariant(std::string_view family, std::string_view weight,
                                           std::string_view style) {
    if (!styles)
        styles.emplace();
    styles->set(u"fontFamily", styles::StyleValue(decodeUtf8(family)));
    if (!weight.empty())
        styles->set(u"fontWeight", styles::StyleValue(decodeUtf8(weight)));
    if (!style.empty())
        styles->set(u"fontStyle", styles::StyleValue(decodeUtf8(style)));
    return "-explicitFont";
}

void CommonWrapper::getScale() {
    const auto &attributes = node->attributes();
    const double parentScale = parent ? parent->bbox.scale : 1;
    const double scriptLevel = mathMin(number(attributes.lookup(u"scriptlevel")), 2.0);
    double scale =
        scriptLevel == 0
            ? 1
            : std::pow(number(attributes.lookup(u"scriptsizemultiplier"), .71), scriptLevel);
    const auto *fontSize = attributes.lookup(u"fontsize");
    const auto *mathSize = (node->isToken() || node->kind() == mml::NodeKind::mstyle)
                               ? attributes.lookup(u"mathsize")
                               : attributes.effectiveInherited(u"mathsize");
    std::u16string removedSize;
    if (!truthy(fontSize)) {
        if (auto found = removedStyles.find("fontSize"); found != removedStyles.end())
            removedSize = decodeUtf8(found->second);
    }
    if (!removedSize.empty() && !attributes.own(u"mathsize"))
        mathSize = nullptr;
    if ((truthy(fontSize) || !removedSize.empty()) && !attributes.own(u"mathsize")) {
        const LengthInput size = !removedSize.empty() ? LengthInput(removedSize) : input(fontSize);
        scale *= length2em(size, 1, 1);
    } else if (mathSize && (!std::holds_alternative<std::u16string>(*mathSize) ||
                            std::get<std::u16string>(*mathSize) != u"1")) {
        scale *= length2em(input(mathSize), 1, 1);
    }
    if (scriptLevel != 0) {
        const double minimum =
            length2em(input(attributes.lookup(u"scriptminsize"), u".4em"), .4, 1);
        if (scale < minimum)
            scale = minimum;
    }
    bbox.scale = scale;
    bbox.rscale = scale / parentScale;
}

bool CommonWrapper::isTopEmbellished() const {
    if (!node->isEmbellished())
        return false;
    auto *sourceParent = node->parent();
    return !sourceParent || !sourceParent->isEmbellished();
}

void CommonWrapper::getSpace() {
    const bool top = isTopEmbellished();
    const bool hasSpacing = node->hasSpacingAttributes();
    if (jax().options().mathmlSpacing || hasSpacing) {
        if (top)
            getMathMLSpacing();
    } else {
        getTeXSpacing(top, hasSpacing);
    }
}

void CommonWrapper::getTeXSpacing(bool top, bool hasSpacing) {
    if (!hasSpacing) {
        const auto spacing = node->texSpacing();
        if (!spacing.empty())
            bbox.L = length2em(LengthInput(decodeUtf8(spacing)));
    }
    if (top || hasSpacing) {
        auto *mo = node->coreMO();
        if (!mo)
            return;
        const auto &attributes = mo->attributes();
        if (attributes.isSet(u"lspace"))
            bbox.L = mathMax(0.0, length2em(input(attributes.lookup(u"lspace"))));
        if (attributes.isSet(u"rspace"))
            bbox.R = mathMax(0.0, length2em(input(attributes.lookup(u"rspace"))));
    }
}

void CommonWrapper::getMathMLSpacing() {
    auto *mo = dynamic_cast<mml::MmlMo *>(node->coreMO());
    if (!mo)
        return;

    auto *child = mo->coreParent();
    if (!child)
        return;
    auto *row = child->parent();
    if (!row || (row->kind() != mml::NodeKind::mrow && row->kind() != mml::NodeKind::inferred_mrow))
        return;

    const auto index = child->childPosition();
    if (!index || row->children().size() == 1)
        return;

    const bool script = number(mo->attributes().lookup(u"scriptlevel")) > 0;
    const bool noDictionary = mo->properties().tex.no_dictionary_definition.value_or(false);
    const auto defaultSpace = [=](double size) {
        if (noDictionary)
            return script ? smallMathSpace : operatorMathSpace;
        return script ? (size < smallMathSpace ? 0.0 : smallMathSpace) : size;
    };
    const auto &attributes = mo->attributes();
    bbox.L = attributes.isSet(u"lspace")
                 ? mathMax(0.0, length2em(input(attributes.lookup(u"lspace"))))
                 : defaultSpace(mo->lspace.value_or(operatorMathSpace));
    bbox.R = attributes.isSet(u"rspace")
                 ? mathMax(0.0, length2em(input(attributes.lookup(u"rspace"))))
                 : defaultSpace(mo->rspace.value_or(operatorMathSpace));

    if (*index == 0)
        return;
    auto *previous = row->children()[*index - 1];
    if (!previous->isEmbellished())
        return;
    if (auto *previousWrapper = jax().wrapperFor(*previous);
        previousWrapper && previousWrapper->getBBox().R) {
        bbox.L = mathMax(0.0, bbox.L - previousWrapper->getBBox().R);
    }
}

double CommonWrapper::length2em(LengthInput const &value, double size,
                                std::optional<double> scale) const {
    const auto &face = jax().face();
    lengths::LengthContext context;
    context.size_em = size;
    context.scale = scale.value_or(bbox.scale);
    context.em_pixels = jax().pxPerEm();
    context.ex_em = metrics().em ? metrics().ex / metrics().em : .5;
    context.mu_em = 1.0 / 18.0;
    if (const auto *number = std::get_if<double>(&value))
        return lengths::length2em(*number, context);

    const auto string = utf8(std::get<std::u16string>(value));
    if (string == "medium")
        return face.parameters.required("rule_thickness");
    if (string == "thin")
        return face.parameters.required("rule_thickness") * 2.0 / 3.0;
    if (string == "thick")
        return face.parameters.required("rule_thickness") * 5.0 / 3.0;
    return lengths::length2em(string, context);
}

std::string CommonWrapper::percent(double value) const {
    return lengths::percent(value);
}

std::string CommonWrapper::em(double value) const {
    return lengths::em(value);
}

std::string CommonWrapper::px(double value, double minimum) const {
    return lengths::px(value, minimum, metrics().em);
}

double CommonWrapper::getAlignX(double width, BBox const &box, std::string_view align) const {
    if (align == "right")
        return width - (box.w + box.R) * box.rscale;
    if (align == "left")
        return box.L * box.rscale;
    return (width - box.w * box.rscale) / 2;
}

double CommonWrapper::getAlignY(double parentHeight, double parentDepth, double childHeight,
                                double childDepth, std::string_view align) const {
    if (align == "top")
        return parentHeight - childHeight;
    if (align == "bottom")
        return childDepth - parentDepth;
    if (align == "center")
        return (parentHeight - childHeight - (parentDepth - childDepth)) / 2;
    return 0;
}

bool CommonWrapper::fixesPWidth() const {
    return !node->notParent() && !node->isToken();
}

std::size_t CommonWrapper::breakCount() {
    if (_breakCount < 0) {
        if (node->isEmbellished()) {
            auto *mo = dynamic_cast<CommonMo *>(coreMO());
            _breakCount = mo ? static_cast<std::int64_t>(mo->embellishedBreakCount()) : 0;
        } else if (node->arity() < 0 && !node->linebreakContainer() && !childNodes.empty()) {
            auto *row = dynamic_cast<CommonMrow *>(childNodes.front());
            _breakCount = row && row->isStack ? static_cast<std::int64_t>(row->breakCount()) : 0;
        } else {
            _breakCount = 0;
        }
    }
    return static_cast<std::size_t>(_breakCount);
}

CommonWrapper *CommonWrapper::breakTop(CommonWrapper *mrow, CommonWrapper *) {
    return node->linebreakContainer() || !parent ? mrow : parent->breakTop(mrow, this);
}

CommonWrapper &CommonWrapper::wrapNode(mml::MmlNode &source, CommonWrapper *parentValue) {
    auto &result = factory().wrap(source, parentValue ? parentValue : this);
    if (parentValue)
        parentValue->childNodes.push_back(&result);
    jax().registerWrapper(source, result);
    return result;
}

BBox &CommonWrapper::getBBox() {
    if (!bboxComputed)
        (void)getBBox(true);
    return bbox;
}

BBox CommonWrapper::getBBox(bool save) {
    if (bboxComputed)
        return bbox;
    if (save) {
        computeBBox(bbox, false);
        bboxComputed = true;
        return bbox;
    }
    // Source seeds a non-saving computation from BBox.zero(), rather than
    // copying wrapper state (scale, side spacing, or other cached metadata).
    BBox result = BBox::zero();
    computeBBox(result, false);
    return result;
}

void CommonWrapper::computeBBox(BBox &target, bool recompute) {
    target.empty();
    for (auto *child : childNodes)
        target.append(child->getOuterBBox());
    target.clean();
    if (fixesPWidth() && setChildPWidths(recompute) && !recompute)
        computeBBox(target, true);
}

std::shared_ptr<LineBBox> CommonWrapper::computeLineBBox(std::size_t index) {
    if (childNodes.empty())
        throw std::logic_error("line box requires a child wrapper");
    return getChildLineBBox(*childNodes.front(), index);
}

std::shared_ptr<LineBBox> CommonWrapper::getLineBBox(std::size_t index) {
    if (index >= lineBBox.size())
        lineBBox.resize(index + 1);
    if (lineBBox[index])
        return lineBBox[index];

    const auto count = breakCount();
    if (count) {
        auto line = embellishedBBox(index);
        if (!line)
            line = computeLineBBox(index);
        lineBBox[index] = line;
        if (index == 0) {
            if (node->kind() != mml::NodeKind::mo && node->isEmbellished())
                line->originalL = getBBox().L;
            else
                line->L = getBBox().L;
        }
        if (index == count)
            line->R = getBBox().R;
    } else {
        lineBBox[index] = std::make_shared<LineBBox>(
            LineBBox::from(getOuterBBox(), jax().currentLinebreakOptions().lineleading));
    }
    return lineBBox[index];
}

std::shared_ptr<LineBBox> CommonWrapper::embellishedBBox(std::size_t index) {
    if (!node->isEmbellished() || node->kind() == mml::NodeKind::mo)
        return {};
    auto *mo = dynamic_cast<CommonMo *>(coreMO());
    if (!mo)
        return {};
    auto box = getOuterBBox();
    return std::make_shared<LineBBox>(mo->moLineBBox(index, mo->embellishedBreakStyle(), &box));
}

std::shared_ptr<LineBBox> CommonWrapper::getChildLineBBox(CommonWrapper &child, std::size_t index) {
    const auto count = breakCount();
    auto result = child.getLineBBox(index);
    if (styleData || bbox.L || bbox.R)
        result = std::make_shared<LineBBox>(result->copy());
    addMiddleBorders(*result);
    if (index == 0) {
        result->L += bbox.L;
        addLeftBorders(*result);
    } else if (index == count) {
        result->R += bbox.R;
        addRightBorders(*result);
    }
    return result;
}

std::array<CommonWrapper *, 2> CommonWrapper::getBreakNode(LineBBox &line) {
    if (!line.start)
        return {this, nullptr};
    const auto childIndex = static_cast<std::size_t>((*line.start)[0]);
    const auto lineIndex = static_cast<std::size_t>((*line.start)[1]);
    if (node->isEmbellished())
        return {this, coreMO()};

    auto *source = this;
    if (!childNodes.empty() &&
        (childNodes.front()->node->isInferred() || node->kind() == mml::NodeKind::semantics))
        source = childNodes.front();
    if (node->isToken() || childIndex >= source->childNodes.size())
        return {this, nullptr};
    auto *child = source->childNodes[childIndex];
    return child->getBreakNode(*child->getLineBBox(lineIndex));
}

double CommonWrapper::sideStyleSize(std::size_t side) const {
    if (!styleData || side >= 4)
        return 0;
    const auto border = styleData->border ? styleData->border->width[side] : 0;
    return border + styleData->padding[side] + styleData->margin[side];
}

void CommonWrapper::addLeftBorders(BBox &target) const {
    if (styleData)
        target.w += sideStyleSize(3);
}

void CommonWrapper::addMiddleBorders(BBox &target) const {
    if (styleData) {
        target.h += sideStyleSize(0);
        target.d += sideStyleSize(2);
    }
}

void CommonWrapper::addRightBorders(BBox &target) const {
    if (styleData)
        target.w += sideStyleSize(1);
}

void CommonWrapper::copySkewIC(BBox &target) const {
    if (childNodes.empty())
        return;
    const auto &first = childNodes.front()->bbox;
    const auto &last = childNodes.back()->bbox;
    if (first.sk)
        target.sk = first.sk;
    if (first.dx)
        target.dx = first.dx;
    if (last.ic) {
        target.ic = last.ic;
        target.w += target.ic;
    }
}

BBox CommonWrapper::getOuterBBox(bool save) {
    BBox outer = getBBox(save);
    if (!styleData)
        return outer;
    const auto border = styleData->border ? styleData->border->width : std::array<double, 4>{};
    for (const auto &side : BBox::boxSides) {
        const auto index = static_cast<std::size_t>(side.index);
        const auto amount = styleData->margin[index] + styleData->padding[index] + border[index];
        if (side.dimension == "w")
            outer.w += amount;
        else if (side.dimension == "h")
            outer.h += amount;
        else
            outer.d += amount;
    }
    return outer;
}

std::array<double, 2> CommonWrapper::getUnbrokenHD() {
    double height = 0;
    double depth = 0;
    for (std::size_t i = 0, count = breakCount() + 1; i < count; ++i) {
        auto line = getLineBBox(i);
        height = mathMax(height, line->h);
        depth = mathMax(depth, line->d);
    }
    return {height, depth};
}

void CommonWrapper::breakToWidth(double) {
    // The pinned Source base wrapper deliberately leaves this to linebreaking
    // subclasses; retain the existing base behavior here.
}

bool CommonWrapper::setChildPWidths(bool recompute, std::optional<double> width, bool clear) {
    if (recompute)
        return false;
    if (clear)
        bbox.pwidth.clear();
    bool changed = false;
    for (auto *child : childNodes) {
        if (!child->bbox.pwidth.empty() &&
            child->setChildPWidths(false, width.value_or(child->bbox.w), clear))
            changed = true;
    }
    return changed;
}

void CommonWrapper::invalidateBBox(bool bubble) {
    if (bboxComputed || _breakCount >= 0) {
        bboxComputed = false;
        lineBBox.clear();
        _breakCount = -1;
        if (parent && bubble)
            parent->invalidateBBox();
    }
}

CommonWrapper *CommonWrapper::core() {
    auto *source = node->core();
    return source ? jax().wrapperFor(*source) : nullptr;
}

CommonWrapper *CommonWrapper::coreMO() {
    auto *source = node->coreMO();
    return source ? jax().wrapperFor(*source) : nullptr;
}

double CommonWrapper::coreRScale() const {
    double result = bbox.rscale;
    CommonWrapper *current = const_cast<CommonWrapper *>(this)->coreMO();
    while (current && current != this) {
        result *= current->bbox.rscale;
        current = current->parent;
    }
    return result;
}

double CommonWrapper::getRScale() const {
    double result = 1;
    for (auto *current = const_cast<CommonWrapper *>(this); current; current = current->parent)
        result *= current->bbox.rscale;
    return result;
}

std::u16string CommonWrapper::getText() const {
    if (!node->isToken())
        return {};
    std::u16string result;
    for (auto *child : node->children()) {
        if (const auto *textNode = dynamic_cast<mml::TextNode const *>(child))
            result += textNode->getText();
    }
    return result;
}

std::vector<std::uint32_t> CommonWrapper::unicodeChars(std::u16string_view s,
                                                       std::string_view selected) const {
    const auto points = utf16::codePoints(s);
    std::vector<std::uint32_t> out;
    out.reserve(points.size());
    for (const auto point : points)
        out.push_back(static_cast<std::uint32_t>(point));
    for (auto &cp : out) {
        if (auto remap = jax().fonts().remap(jax().face().name, selected, cp))
            cp = remap->codepoint;
    }
    return out;
}

std::vector<std::uint32_t> CommonWrapper::remapChars(std::span<std::uint32_t const> chars) const {
    return {chars.begin(), chars.end()};
}

mml::TextNode &CommonWrapper::mmlText(std::u16string value) {
    auto &result = node->document().factory().make(mml::NodeKind::text);
    auto *textNode = dynamic_cast<mml::TextNode *>(&result);
    if (!textNode)
        throw std::logic_error("text creator returned a non-TextNode");
    return textNode->setText(std::move(value));
}

mml::MmlNode &CommonWrapper::mmlNode(mml::NodeKind kind, mml::AttributeSet attributes,
                                     std::vector<mml::MmlNode *> children) {
    return node->document().factory().make(kind, std::move(attributes), children);
}

CommonMo &CommonWrapper::createMo(std::u16string_view value) {
    auto &textNode = mmlText(std::u16string(value));
    mml::AttributeSet attributes;
    attributes.set(u"stretchy", true);
    std::array<mml::MmlNode *, 1> children{&textNode};
    auto &moNode =
        node->document().factory().make(mml::NodeKind::mo, std::move(attributes), children);
    moNode.inheritAttributesFrom(*node);
    if (auto *parentNode = node->Parent())
        node->document().attachAuxiliary(*parentNode, moNode);
    auto &wrapper = wrapNode(moNode);
    auto *mo = dynamic_cast<CommonMo *>(&wrapper);
    if (!mo)
        throw std::logic_error("mo creator returned incompatible wrapper");
    return *mo;
}

double CommonWrapper::getWrapWidth(std::size_t index) {
    return childNodes.at(index)->getBBox().w;
}

std::optional<ResolvedFontGlyph> CommonWrapper::getVariantChar(std::string_view selected,
                                                               std::uint32_t codepoint) {
    return jax().fonts().resolveGlyph(jax().face().name, selected, codepoint);
}

bool CommonWrapper::canStretch(StretchDirection direction) {
    stretch = NoStretch{};
    if (direction == StretchDirection::none)
        return false;
    if (node->isEmbellished()) {
        if (auto *inner = core(); inner && inner != this && inner->canStretch(direction))
            stretch = inner->stretch;
    }
    return std::holds_alternative<ActiveStretch>(stretch);
}

std::pair<std::string, double> CommonWrapper::getAlignShift() {
    const auto &attributes = node->attributes();
    auto align = valueString(attributes.lookup(u"indentalign"));
    auto shift = valueString(attributes.lookup(u"indentshift"));
    const auto firstAlign = valueString(attributes.lookup(u"indentalignfirst"), "indentalign");
    const auto firstShift = valueString(attributes.lookup(u"indentshiftfirst"), "indentshift");
    if (firstAlign != "indentalign")
        align = firstAlign;
    if (firstShift != "indentshift")
        shift = firstShift;
    return processIndent(align, shift);
}

std::pair<std::string, double> CommonWrapper::processIndent(std::string_view alignment,
                                                            std::string_view shift,
                                                            std::string_view defaultAlign,
                                                            std::string_view defaultShift,
                                                            std::optional<double> width) {
    if (!metrics().display)
        return {"left", 0};

    const auto *root = jax().document().root();
    const bool inlineMarked = root && root->properties().layout.inline_marked.value_or(false);
    std::string align(alignment);
    std::string indent(shift);
    std::string baseAlign(defaultAlign);
    std::string baseShift(defaultShift);
    if (baseAlign.empty() || baseAlign == "auto")
        baseAlign = inlineMarked ? "left" : jax().options().displayAlign;
    if (baseShift.empty() || baseShift == "auto")
        baseShift = inlineMarked ? "0" : utf8(jax().options().displayIndent);
    if (align == "auto")
        align = baseAlign;
    if (indent == "auto") {
        indent = baseShift;
        if (align == "right" && !isZeroIndent(decodeUtf8(indent))) {
            auto trimmed = decodeUtf8(indent);
            while (!trimmed.empty() && js_input::isWhitespace(trimmed.front()))
                trimmed.erase(trimmed.begin());
            while (!trimmed.empty() && js_input::isWhitespace(trimmed.back()))
                trimmed.pop_back();
            indent = "-" + utf8(trimmed);
            if (indent.starts_with("--"))
                indent.erase(0, 2);
        }
    }
    return {align, length2em(LengthInput(decodeUtf8(indent)), width.value_or(containerWidth()))};
}

std::string CommonWrapper::getChildAlign(std::size_t) const {
    return "left";
}
} // namespace folia::math::output::common
