/*************************************************************
 * Candidate typed port of MathJax 4.1.3 output/common/Wrappers/scriptbase.ts.
 * Uses the factory-owned child wrappers, MML AttributeSet and immutable face.
 *************************************************************/
module folia.math.output.common;
import :scriptbase;
import :wrapper;
import :mo;

import folia.math.core.mml_tree.attributes;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.core.mml_tree.node_kind;
import folia.math.util.lengths;
import std;

namespace folia::math::output::common {
namespace {
constexpr double sourceSkewIcFactor = 0.75; // Source FontData.ts:728.

const mml::AttributeValue *attribute(mml::MmlNode const &node, std::u16string_view name) {
    return node.attributes().lookup(name);
}

bool truthy(mml::AttributeValue const *value) {
    if (!value || std::holds_alternative<mml::AttributeNull>(*value))
        return false;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean;
    if (const auto *number = std::get_if<double>(value))
        return *number != 0 && !std::isnan(*number);
    return !std::get<std::u16string>(*value).empty();
}

std::u16string_view textValue(mml::AttributeValue const *value) {
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr)
        return *text;
    return {};
}

LengthInput lengthValue(mml::MmlNode const &node, std::u16string_view name) {
    const auto *value = attribute(node, name);
    if (!value)
        return 0.0;
    if (const auto *text = std::get_if<std::u16string>(value))
        return *text;
    if (const auto *number = std::get_if<double>(value))
        return *number;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean ? 1.0 : 0.0;
    return 0.0;
}

std::size_t codePointCount(std::u16string_view text) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto c = text[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < text.size() && text[i + 1] >= 0xDC00 &&
            text[i + 1] <= 0xDFFF)
            ++i;
        ++count;
    }
    return count;
}

CommonWrapper *childAt(CommonWrapper &wrapper, std::size_t index) {
    if (index >= wrapper.childNodes.size())
        return nullptr;
    return wrapper.childNodes[index];
}

std::optional<double> lineLeading(CommonScriptbase const &wrapper) {
    return wrapper.linebreakOptions().lineleading;
}

double parameter(CommonScriptbase const &wrapper, std::string_view name) {
    return wrapper.jax().face().parameters.required(name);
}
} // namespace

CommonScriptbase::CommonScriptbase(CommonWrapperFactory &factory, mml::MmlNode &node,
                                   CommonWrapper *parentValue, bool useItalicCorrection,
                                   std::size_t constructorScriptChild)
    : CommonWrapper(factory, node, parentValue), useItalicCorrection_(useItalicCorrection),
      constructorScriptChild_(constructorScriptChild) {
}

void CommonScriptbase::initializeSource() {
    CommonWrapper::initializeSource();
    baseCore = getBaseCore();
    if (!baseCore)
        return;
    setBaseAccentsFor(*baseCore);
    baseScale = getBaseScale();
    baseIc = getBaseIc();
    baseIsChar = isCharBase();
    auto *script = childAt(*this, constructorScriptChild_);
    if (baseIsChar && script) {
        if (auto *core = script->coreMO())
            isMathAccent = core->node->properties().tex.math_accent.has_value();
    }
    checkLineAccents();
    baseRemoveIc = !isLineAbove && !isLineBelow && (!useItalicCorrection_ || isMathAccent);
}

CommonWrapper *CommonScriptbase::baseChild() const {
    if (const auto *nodeValue = dynamic_cast<mml::MmlMsubsup const *>(node))
        return childAt(*const_cast<CommonScriptbase *>(this), nodeValue->base());
    if (const auto *limits = dynamic_cast<mml::MmlMunderover const *>(node))
        return childAt(*const_cast<CommonScriptbase *>(this), limits->base());
    return nullptr;
}

CommonWrapper *CommonScriptbase::scriptChild() const {
    return childAt(*const_cast<CommonScriptbase *>(this), 1);
}

CommonWrapper *CommonScriptbase::getBaseCore() {
    auto *current = getSemanticBase();
    if (!current)
        current = childAt(*this, 0);
    while (current) {
        const auto kind = current->node->kind();
        const bool oneChild = current->childNodes.size() == 1;
        const auto &vbox = current->node->properties().tex.vertical_box;
        const bool sourceVboxTruthy = vbox && !vbox->empty();
        const bool unwrapKind = kind == mml::NodeKind::mrow || kind == mml::NodeKind::tex_atom ||
                                kind == mml::NodeKind::mstyle || kind == mml::NodeKind::mphantom ||
                                kind == mml::NodeKind::semantics ||
                                (kind == mml::NodeKind::mpadded && !sourceVboxTruthy);
        bool unwrap = oneChild && unwrapKind;
        if (kind == mml::NodeKind::munderover) {
            if (auto *script = dynamic_cast<CommonScriptbase *>(current);
                script && script->isMathAccent)
                unwrap = true;
        }
        if (!unwrap)
            break;
        setBaseAccentsFor(*current);
        current = childAt(*current, 0);
    }
    if (!current) {
        baseHasAccentOver = false;
        baseHasAccentUnder = false;
    }
    return current ? current : childAt(*this, 0);
}

void CommonScriptbase::setBaseAccentsFor(CommonWrapper &core) {
    if (core.node->kind() != mml::NodeKind::munderover)
        return;
    if (!baseHasAccentOver)
        baseHasAccentOver = truthy(attribute(*core.node, u"accent"));
    if (!baseHasAccentUnder)
        baseHasAccentUnder = truthy(attribute(*core.node, u"accentunder"));
}

CommonWrapper *CommonScriptbase::getSemanticBase() {
    const auto *value = node->attributes().own(u"data-semantic-fencepointer");
    const auto id = textValue(value);
    if (id.empty())
        return nullptr;
    return getBaseFence(baseChild(), id);
}

CommonWrapper *CommonScriptbase::getBaseFence(CommonWrapper *fence, std::u16string_view id) {
    if (!fence || id.empty())
        return nullptr;
    const auto *value = fence->node->attributes().own(u"data-semantic-id");
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr) {
        if (*text == id)
            return fence;
    }
    for (auto *child : fence->childNodes)
        if (auto *result = getBaseFence(child, id))
            return result;
    return nullptr;
}

double CommonScriptbase::getBaseScale() const {
    double scale = 1;
    auto *child = baseCore;
    while (child && child != this) {
        scale *= child->getOuterBBox().rscale;
        child = child->parent;
    }
    return scale;
}

double CommonScriptbase::getBaseIc() const {
    return baseCore ? baseCore->getOuterBBox().ic * baseScale : 0;
}

double CommonScriptbase::getAdjustedIc() const {
    return baseIc ? 1.05 * baseIc + 0.05 : 0;
}

bool CommonScriptbase::isCharBase() const {
    if (!baseCore)
        return false;
    const auto kind = baseCore->node->kind();
    bool accepted = kind == mml::NodeKind::mi || kind == mml::NodeKind::mn;
    if (kind == mml::NodeKind::mo) {
        if (auto *mo = dynamic_cast<CommonMo *>(baseCore))
            accepted = !mo->size.has_value();
    }
    return accepted && baseCore->getOuterBBox().rscale == 1 &&
           codePointCount(baseCore->getText()) == 1;
}

void CommonScriptbase::checkLineAccents() {
    const auto kind = node->kind();
    if (kind != mml::NodeKind::munderover && kind != mml::NodeKind::mover &&
        kind != mml::NodeKind::munder)
        return;
    if (kind == mml::NodeKind::mover)
        isLineAbove = isLineAccent(scriptChild());
    else if (kind == mml::NodeKind::munder)
        isLineBelow = isLineAccent(scriptChild());
    else if (const auto *limits = dynamic_cast<mml::MmlMunderover const *>(node)) {
        isLineAbove = isLineAccent(childAt(*this, limits->over()));
        isLineBelow = isLineAccent(childAt(*this, limits->under()));
    }
}

bool CommonScriptbase::isLineAccent(CommonWrapper *script) const {
    if (!script)
        return false;
    auto *core = script->coreMO();
    return core && core->node->isToken() && core->getText() == u"\u2015";
}

double CommonScriptbase::getBaseWidth() const {
    auto *child = baseChild();
    if (!child)
        return 0;
    const auto line = child->getLineBBox(child->breakCount());
    return line->w * line->rscale - (baseRemoveIc ? baseIc : 0) + parameter(*this, "extra_ic");
}

std::array<double, 2> CommonScriptbase::getOffset() const {
    return {0, 0};
}

double CommonScriptbase::baseCharZero(double value) const {
    const bool largeop = baseCore && truthy(attribute(*baseCore->node, u"largeop"));
    bool sized = false;
    if (baseCore && baseCore->node->kind() == mml::NodeKind::mo) {
        if (auto *mo = dynamic_cast<CommonMo *>(baseCore)) {
            if (const auto *fixed = mo->size ? std::get_if<FixedStretchSize>(&*mo->size) : nullptr)
                sized = fixed->index != 0;
            else
                sized = mo->size.has_value();
        }
    }
    return baseIsChar && !largeop && !sized && baseScale == 1 ? 0 : value;
}

double CommonScriptbase::getV() const {
    if (!baseCore || !scriptChild())
        throw std::logic_error("script wrapper has no base or script child");
    const auto bbox = baseCore->getLineBBox(baseCore->breakCount());
    const auto script = scriptChild()->getOuterBBox();
    const double subShift =
        length2em(lengthValue(*node, u"subscriptshift"), parameter(*this, "sub1"));
    return std::max(
        {baseCharZero(bbox->d * baseScale + parameter(*this, "sub_drop") * script.rscale), subShift,
         script.h * script.rscale - .8 * parameter(*this, "x_height")});
}

double CommonScriptbase::getU() const {
    if (!baseCore || !scriptChild())
        throw std::logic_error("script wrapper has no base or script child");
    const auto bbox = baseCore->getLineBBox(baseCore->breakCount());
    const auto script = scriptChild()->getOuterBBox();
    const bool display = truthy(attribute(*node, u"displaystyle"));
    const bool prime = node->properties().tex.prime_style.value_or(false);
    const double initial = prime ? parameter(*this, "sup3")
                                 : (display ? parameter(*this, "sup1") : parameter(*this, "sup2"));
    const double supShift = length2em(lengthValue(*node, u"superscriptshift"), initial);
    return std::max(
        {baseCharZero(bbox->h * baseScale - parameter(*this, "sup_drop") * script.rscale), supShift,
         script.d * script.rscale + .25 * parameter(*this, "x_height")});
}

bool CommonScriptbase::hasMovableLimits() const {
    const auto *display = attribute(*node, u"displaystyle");
    auto *core = baseChild() ? baseChild()->coreMO() : nullptr;
    auto *mo =
        core && core->node->kind() == mml::NodeKind::mo ? dynamic_cast<CommonMo *>(core) : nullptr;
    return !truthy(display) && mo && truthy(attribute(*mo->node, u"movablelimits"));
}

std::array<double, 2> CommonScriptbase::getOverKU(const BBox &base, const BBox &over) const {
    const bool accent = truthy(attribute(*node, u"accent"));
    const double rule = parameter(*this, "rule_thickness");
    const double separation = parameter(*this, "separation_factor");
    const double d = over.d * over.rscale, t = rule * separation;
    const double delta = baseHasAccentOver.value_or(false) ? t : 0;
    const double top = isLineAbove ? 3 * rule : t;
    const double k = (accent ? top
                             : std::max(parameter(*this, "big_op_spacing1"),
                                        parameter(*this, "big_op_spacing3") - std::max(0.0, d))) -
                     delta;
    return {k, base.h * base.rscale + k + d};
}

std::array<double, 2> CommonScriptbase::getUnderKV(const BBox &base, const BBox &under) const {
    const bool accent = truthy(attribute(*node, u"accentunder"));
    const double rule = parameter(*this, "rule_thickness");
    const double separation = parameter(*this, "separation_factor");
    const double h = under.h * under.rscale, t = rule * separation;
    const double delta = baseHasAccentUnder.value_or(false) ? t : 0;
    const double bottom = isLineBelow ? 3 * rule : t;
    const double k = (accent ? bottom
                             : std::max(parameter(*this, "big_op_spacing2"),
                                        parameter(*this, "big_op_spacing4") - h)) -
                     delta;
    return {k, -(base.d * base.rscale + k + h)};
}

std::vector<double> CommonScriptbase::getDeltaW(const std::vector<BBox> &boxes,
                                                std::array<double, 3> delta) const {
    std::vector<double> widths;
    widths.reserve(boxes.size());
    for (const auto &box : boxes)
        widths.push_back(box.w * box.rscale);
    if (widths.empty())
        return {};
    if (baseRemoveIc && !truthy(baseCore ? attribute(*baseCore->node, u"largeop") : nullptr))
        widths[0] -= baseIc;
    const double width = *std::max_element(widths.begin(), widths.end());
    const auto align = textValue(attribute(*node, u"align"));
    std::vector<double> result(widths.size());
    double minShift = 0;
    for (std::size_t i = 0; i < widths.size(); ++i) {
        const double shift = align == u"center"  ? (width - widths[i]) / 2
                             : align == u"right" ? width - widths[i]
                                                 : 0;
        result[i] = shift + (i < delta.size() ? delta[i] : 0);
        if (result[i] < minShift)
            minShift = result[i];
    }
    if (minShift)
        for (auto &value : result)
            value -= minShift;
    for (std::size_t i = 1; i < std::min<std::size_t>(3, boxes.size()); ++i)
        result[i] += boxes[i].dx * boxes[0].rscale;
    return result;
}

double CommonScriptbase::getDelta(CommonWrapper &script, bool noSkew) const {
    const bool accent = truthy(attribute(*node, u"accent"));
    const auto base = baseCore->getOuterBBox();
    double skew = base.sk;
    const double ic = base.ic;
    if (accent)
        skew -= script.getOuterBBox().sk;
    return ((accent && !noSkew ? skew : 0) + sourceSkewIcFactor * ic) * baseScale;
}

void CommonScriptbase::stretchChildren() {
    std::vector<CommonWrapper *> stretchy;
    for (auto *child : childNodes)
        if (child && child->canStretch(StretchDirection::horizontal))
            stretchy.push_back(child);
    if (stretchy.empty() || childNodes.size() <= 1)
        return;
    const bool all = stretchy.size() > 1 && stretchy.size() == childNodes.size();
    double width = 0;
    for (auto *child : childNodes) {
        const bool noStretch = !std::holds_alternative<ActiveStretch>(child->stretch);
        if (all || noStretch) {
            const auto box = child->getOuterBBox(noStretch);
            width = std::max(width, box.w * box.rscale);
        }
    }
    for (auto *child : stretchy) {
        auto *coreWrapper = child->coreMO();
        auto *core = coreWrapper && coreWrapper->node->kind() == mml::NodeKind::mo
                         ? dynamic_cast<CommonMo *>(coreWrapper)
                         : nullptr;
        if (core && !core->size)
            core->getStretchedVariant(std::array<double, 1>{width / child->coreRScale()});
    }
}

void CommonScriptbase::computeBBox(BBox &box, bool recompute) {
    box.empty();
    auto *base = baseChild();
    if (!base)
        throw std::logic_error("script wrapper has no base child");
    box.append(base->getOuterBBox());
    appendScripts(box);
    box.clean();
    setChildPWidths(recompute);
}

BBox CommonScriptbase::appendScripts(BBox &box) const {
    const double width = getBaseWidth();
    const auto offset = getOffset();
    auto *script = scriptChild();
    if (!script)
        throw std::logic_error("script wrapper has no script child");
    box.combine(script->getOuterBBox(), width + offset[0], offset[1]);
    box.w += parameter(*this, "scriptspace");
    return box;
}

std::size_t CommonScriptbase::breakCount() {
    if (_breakCount < 0) {
        if (node->isEmbellished()) {
            auto *core = coreMO();
            auto *mo = core && core->node->kind() == mml::NodeKind::mo
                           ? dynamic_cast<CommonMo *>(core)
                           : nullptr;
            if (!mo)
                throw std::logic_error("embellished script core is not a CommonMo wrapper");
            _breakCount = static_cast<std::int64_t>(mo->embellishedBreakCount());
        } else if (!node->linebreakContainer()) {
            auto *base = childAt(*this, 0);
            _breakCount = base ? static_cast<std::int64_t>(base->breakCount()) : 0;
        } else
            _breakCount = 0;
    }
    return static_cast<std::size_t>(_breakCount);
}

CommonWrapper *CommonScriptbase::breakTop(CommonWrapper *row, CommonWrapper *child) {
    const auto position = child ? child->node->childPosition() : std::optional<std::size_t>{};
    if (node->linebreakContainer() || !parent || (position && *position))
        return row;
    return parent->breakTop(row, this);
}

std::shared_ptr<LineBBox> CommonScriptbase::computeLineBBox(std::size_t index) {
    const auto count = breakCount();
    if (!count)
        return std::make_shared<LineBBox>(LineBBox::from(getOuterBBox(), lineLeading(*this)));
    auto *base = baseChild();
    if (!base)
        throw std::logic_error("script wrapper has no base child for line box");
    auto box = base->getLineBBox(index)->copy();
    if (index < count) {
        if (index == 0)
            addLeftBorders(box);
        addMiddleBorders(box);
    } else {
        appendScripts(box);
        addMiddleBorders(box);
        addRightBorders(box);
    }
    return std::make_shared<LineBBox>(std::move(box));
}
} // namespace folia::math::output::common
