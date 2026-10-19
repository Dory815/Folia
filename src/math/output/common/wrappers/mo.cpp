module folia.math.output.common;
import :wrapper;
import :mo;
import :wrapper_factory;
import :output_jax;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.util.lengths;
import folia.math.util.js_number;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
namespace {
using mml::AttributeNull;
using mml::AttributeValue;

AttributeValue const *attribute(mml::MmlNode const &node, std::string_view asciiName) {
    std::u16string name;
    name.reserve(asciiName.size());
    for (const unsigned char c : asciiName)
        name.push_back(static_cast<char16_t>(c));
    return node.attributes().lookup(name);
}

bool truthy(AttributeValue const *value) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return false;
    if (auto const *b = std::get_if<bool>(value))
        return *b;
    if (auto const *n = std::get_if<double>(value))
        return *n != 0 && !std::isnan(*n);
    return !std::get<std::u16string>(*value).empty();
}

std::u16string ascii16(std::string_view value) {
    return utf16::decode(value).text;
}

std::u16string attributeText(AttributeValue const &value) {
    if (auto const *text = std::get_if<std::u16string>(&value))
        return *text;
    if (auto const *b = std::get_if<bool>(&value))
        return *b ? u"true" : u"false";
    if (auto const *n = std::get_if<double>(&value))
        return ascii16(js_number::toString(*n));
    return u"null";
}

std::string ascii(AttributeValue const *value) {
    if (!value)
        return {};
    if (std::holds_alternative<AttributeNull>(*value))
        return "null";
    if (auto const *b = std::get_if<bool>(value))
        return *b ? "true" : "false";
    if (auto const *n = std::get_if<double>(value))
        return js_number::toString(*n);
    return utf16::encode(std::get<std::u16string>(*value));
}

LengthInput lengthInput(AttributeValue const *value) {
    if (!value)
        return 0.0;
    if (auto const *n = std::get_if<double>(value))
        return *n;
    if (auto const *text = std::get_if<std::u16string>(value))
        return *text;
    if (auto const *b = std::get_if<bool>(value))
        return *b ? std::u16string(u"true") : std::u16string(u"false");
    return std::u16string(u"null");
}

double parameter(FontFaceData const &face, std::string_view name) {
    return face.parameters.required(name);
}

std::uint32_t recipeCharacter(DelimiterRecipe const &recipe, std::uint32_t original) {
    return recipe.c && *recipe.c != 0 ? *recipe.c : original;
}

bool sourceTruthySize(
    std::optional<std::variant<FixedStretchSize, AssemblyStretchSize>> const &size) {
    if (!size)
        return false;
    if (std::holds_alternative<AssemblyStretchSize>(*size))
        return true;                                     // Source -1
    return std::get<FixedStretchSize>(*size).index != 0; // Source's numeric 0 is falsy
}

std::string indexedVariant(std::vector<std::string> const &variants,
                           std::vector<std::uint32_t> const &recipeIndices, std::size_t index,
                           std::string_view category) {
    const auto mapped = recipeIndices.empty() ? index : recipeIndices.at(index);
    if (mapped >= variants.size())
        throw std::out_of_range(std::string(category) + " variant index is outside face data");
    return variants[mapped];
}
} // namespace

CommonMo::CommonMo(CommonWrapperFactory &factory, mml::MmlNode &source, CommonWrapper *parentValue)
    : CommonWrapper(factory, source, parentValue) {
}

mml::MmlMo &CommonMo::mo() const {
    auto *result = dynamic_cast<mml::MmlMo *>(node);
    if (!result)
        throw std::logic_error("CommonMo requires MmlMo");
    return *result;
}

ActiveStretch &CommonMo::activeStretch() {
    auto *result = std::get_if<ActiveStretch>(&stretch);
    if (!result)
        throw std::logic_error("CommonMo has no active delimiter recipe");
    return *result;
}

ActiveStretch const &CommonMo::activeStretch() const {
    auto const *result = std::get_if<ActiveStretch>(&stretch);
    if (!result)
        throw std::logic_error("CommonMo has no active delimiter recipe");
    return *result;
}

void CommonMo::initializeSource() {
    CommonWrapper::initializeSource();
    isAccent = mo().isAccent();
    getMultChar();
    setBreakStyle();
}

void CommonMo::getMultChar() {
    auto const *value = attribute(*node, "linebreakmultchar");
    if (!truthy(value) || getText() != u"\u2062")
        return;
    const auto text = attributeText(*value);
    if (text == u"\u2062")
        return;
    multChar = &createMo(text);
}

void CommonMo::getVariant() {
    if (truthy(attribute(*node, "largeop"))) {
        variant = truthy(attribute(*node, "displaystyle")) ? "-largeop" : "-smallop";
        return;
    }
    const auto &pseudo = node->properties().tex.pseudo_script;
    if (!node->attributes().own(u"mathvariant") && pseudo && !*pseudo) {
        variant = "-tex-variant";
        return;
    }
    CommonWrapper::getVariant();
}

std::vector<std::uint32_t> CommonMo::remapChars(std::span<std::uint32_t const> chars) const {
    auto const &properties = node->properties().tex;
    if (properties.primes && !properties.primes->empty())
        return unicodeChars(*properties.primes);
    if (chars.size() != 1)
        return std::vector<std::uint32_t>(chars.begin(), chars.end());
    auto const &face = jax().face();
    bool accentOutsideRow = false;
    if (isAccent) {
        auto *coreParent = mo().coreParent();
        if (!coreParent || !coreParent->parent())
            throw std::logic_error("CommonMo accent remap requires its Source core parent");
        accentOutsideRow = !coreParent->parent()->isKind(mml::NodeKind::mrow);
    }
    auto const &remaps = accentOutsideRow ? face.accentRemap : face.operatorRemap;
    if (auto it = remaps.find(chars.front()); it != remaps.end())
        return unicodeChars(it->second);
    return std::vector<std::uint32_t>(chars.begin(), chars.end());
}

std::size_t CommonMo::breakCount() {
    return breakStyle.empty() ? 0 : 1;
}

std::size_t CommonMo::embellishedBreakCount() const {
    return embellishedBreakStyle().empty() ? 0 : 1;
}

std::string CommonMo::embellishedBreakStyle() const {
    return breakStyle.empty() ? getBreakStyle() : breakStyle;
}

std::string CommonMo::getBreakStyle(std::string_view forced) const {
    std::string style(forced);
    auto const *linebreak = attribute(*node, "linebreak");
    auto const *force =
        node->properties().layout.force_break ? &*node->properties().layout.force_break : nullptr;
    if (style.empty() && ((linebreak && std::get_if<std::u16string>(linebreak) &&
                           std::get<std::u16string>(*linebreak) == u"newline") ||
                          (force && *force)))
        style = ascii(attribute(*node, "linebreakstyle"));
    if (style == "infixlinebreakstyle")
        style = ascii(attribute(*node, style));
    return style;
}

void CommonMo::setBreakStyle(std::string_view linebreak) {
    auto *parentNode = node->parent();
    const bool parentEmbellished = parentNode && parentNode->isEmbellished();
    breakStyle = parentEmbellished && linebreak.empty() ? std::string{} : getBreakStyle(linebreak);
    if (!breakCount() || !multChar)
        return;
    if (!parent)
        throw std::logic_error("linebreak multChar requires a parent wrapper");
    const auto index = parent->node->childIndex(*node);
    if (!index)
        return;
    auto siblings = parent->node->children();
    if (*index + 1 >= siblings.size())
        return;
    siblings[*index + 1]->setTeXclass(multChar->node);
}

void CommonMo::protoBBox(BBox &target) {
    auto *active = std::get_if<ActiveStretch>(&stretch);
    if (active && !size)
        getStretchedVariant(std::array<double, 1>{0});
    if (active && size && std::holds_alternative<AssemblyStretchSize>(*size))
        return;
    CommonWrapper::computeBBox(target);
    auto const texclass = mo().texClass();
    if (target.w == 0 && node->attributes().own(u"fence") && getText().empty() && texclass &&
        (*texclass == mml::TexClass::open || *texclass == mml::TexClass::close) &&
        !jax().options().mathmlSpacing)
        target.R = parameter(jax().face(), "nulldelimiterspace");
    copySkewIC(target);
}

void CommonMo::computeBBox(BBox &target, bool) {
    protoBBox(target);
    auto *active = std::get_if<ActiveStretch>(&stretch);
    const auto *recipe =
        active ? jax().fonts().delimiter(jax().face().name, active->sourceCodepoint) : nullptr;
    if (active && !recipe)
        throw std::out_of_range("active delimiter recipe is missing");
    if (truthy(attribute(*node, "symmetric")) &&
        (!recipe || recipe->direction != StretchDirection::horizontal)) {
        const double offset =
            (target.h + target.d) / 2 + parameter(jax().face(), "axis_height") - target.h;
        target.h += offset;
        target.d -= offset;
    }
    const bool nonnegativeSize = !size || std::holds_alternative<FixedStretchSize>(*size);
    if (node->properties().tex.math_accent.value_or(false) && (!active || nonnegativeSize))
        target.w = 0;
}

LineBBox CommonMo::moLineBBox(std::size_t index, std::string_view style, BBox const *outer) {
    auto const *lineLeading = attribute(*node, "lineleading");
    const double leading = lineLeading ? length2em(lengthInput(lineLeading),
                                                   jax().currentLinebreakOptions().lineleading)
                                       : jax().currentLinebreakOptions().lineleading;
    if (index == 0 && style == "before") {
        auto line = LineBBox::from(BBox::zero(), leading);
        line.originalL = bbox.L;
        bbox.L = 0;
        return line;
    }
    auto const base = outer ? *outer : getOuterBBox();
    auto line = LineBBox::from(base, leading);
    if (index == 1) {
        if (style == "after") {
            line.w = line.h = line.d = 0;
            line.isFirst = true;
            bbox.R = 0;
        } else if (style == "duplicate")
            line.L = 0;
        else if (multChar)
            line = LineBBox::from(multChar->getOuterBBox(), leading);
        line.getIndentData(*node);
    }
    return line;
}

std::shared_ptr<LineBBox> CommonMo::computeLineBBox(std::size_t index) {
    return std::make_shared<LineBBox>(moLineBBox(index, breakStyle));
}

double CommonMo::getAccentOffset() {
    auto target = BBox::makeEmpty();
    protoBBox(target);
    return -target.w / 2;
}

double CommonMo::getCenterOffset(BBox *target) {
    BBox temporary;
    if (!target) {
        CommonWrapper::computeBBox(temporary);
        target = &temporary;
    }
    return (target->h + target->d) / 2 + parameter(jax().face(), "axis_height") - target->h;
}

double CommonMo::getSize(std::string_view name, double fallback) {
    if (!node->attributes().isSet(std::u16string(name.begin(), name.end())))
        return fallback;
    return length2em(lengthInput(attribute(*node, name)), 1, 1);
}

double CommonMo::getWH(std::span<double const> dimensions) const {
    if (dimensions.empty())
        return 0;
    if (dimensions.size() == 1)
        return dimensions[0];
    const double h = dimensions[0], d = dimensions[1];
    const double axis = parameter(jax().face(), "axis_height");
    return truthy(attribute(*node, "symmetric")) ? 2 * std::max(h - axis, d + axis) : h + d;
}

double CommonMo::checkExtendedHeight(double dimension, DelimiterRecipe const &recipe) const {
    if (!recipe.fullExtent)
        return dimension;
    const auto [extSize, endSize] = *recipe.fullExtent;
    if (!(extSize > 0))
        throw std::domain_error("delimiter fullExtent extSize must be positive");
    return endSize + std::ceil(std::max(0.0, dimension - endSize) / extSize) * extSize;
}

std::array<double, 2> CommonMo::getBaseline(std::span<double const> requested, double dimension,
                                            DelimiterRecipe const &recipe) const {
    const bool hasPair = requested.size() == 2 && requested[0] + requested[1] == dimension;
    const bool symmetric = truthy(attribute(*node, "symmetric"));
    const double h0 = hasPair ? requested[0] : dimension, d0 = hasPair ? requested[1] : 0;
    double h = h0 + d0, d = 0;
    if (symmetric) {
        const double axis = parameter(jax().face(), "axis_height");
        if (hasPair)
            h = 2 * std::max(h0 - axis, d0 + axis);
        d = h / 2 - axis;
    } else if (hasPair)
        d = d0;
    else {
        const auto hd = recipe.heightDepthWidth
                            ? std::array<double, 2>{(*recipe.heightDepthWidth)[0],
                                                    (*recipe.heightDepthWidth)[1]}
                            : std::array<double, 2>{.75, .25};
        d = hd[1] * (h / (hd[0] + hd[1]));
    }
    return {h - d, d};
}

void CommonMo::getStretchBBox(std::span<double const> requested, double dimension,
                              DelimiterRecipe const &recipe) {
    if (recipe.min && *recipe.min > dimension)
        dimension = *recipe.min;
    if (!recipe.heightDepthWidth)
        throw std::invalid_argument("stretch assembly requires heightDepthWidth");
    auto [h, d, w] = *recipe.heightDepthWidth;
    if (recipe.direction == StretchDirection::vertical) {
        const auto baseline = getBaseline(requested, dimension, recipe);
        h = baseline[0];
        d = baseline[1];
    } else {
        w = dimension;
        if (recipe.heightDepth && !jax().options().mathmlSpacing) {
            const double factor = parameter(jax().face(), "extender_factor");
            h = h * (1 - factor) + (*recipe.heightDepth)[0] * factor;
            d = d * (1 - factor) + (*recipe.heightDepth)[1] * factor;
        }
    }
    bbox.h = h;
    bbox.d = d;
    bbox.w = w;
}

void CommonMo::setDelimSize(std::uint32_t character, std::size_t index) {
    auto const &active = activeStretch();
    auto const *recipe = jax().fonts().delimiter(jax().face().name, active.sourceCodepoint);
    if (!recipe)
        throw std::out_of_range("selected delimiter recipe disappeared");
    // Source FontData::getSizeVariant(c,i) first looks up the delimiter at c,
    // which can differ from the original source codepoint after recipe.c.
    auto const *variantRecipe = jax().fonts().delimiter(jax().face().name, character);
    static const std::vector<std::uint32_t> noVariantMap;
    variant = indexedVariant(jax().face().size_variants,
                             variantRecipe ? variantRecipe->variants : noVariantMap, index, "size");
    size = FixedStretchSize{index};
    std::optional<std::uint32_t> selectedOverride;
    if (!recipe->schar.empty()) {
        const auto slot = std::min(index, recipe->schar.size() - 1);
        if (recipe->schar[slot])
            selectedOverride = recipe->schar[slot];
    }
    if (!selectedOverride && recipe->c && *recipe->c)
        selectedOverride = *recipe->c;
    activeStretch().selectedCodepoint = selectedOverride;
    if (childNodes.empty() || !childNodes.front())
        throw std::out_of_range("selected delimiter has no first child wrapper");
    childNodes.front()->invalidateBBox();
}

void CommonMo::getStretchedVariant(std::span<double const> requested, bool exact) {
    auto *active = std::get_if<ActiveStretch>(&stretch);
    if (!active)
        return;
    double dimension = getWH(requested);
    const double minimum = getSize("minsize", 0),
                 maximum = getSize("maxsize", std::numeric_limits<double>::infinity());
    const bool mathaccent = node->properties().tex.math_accent.value_or(false);
    dimension = std::max(minimum, std::min(maximum, dimension));
    const auto &params = jax().face().parameters;
    const double factor = params.required("delimiterfactor") / 1000.0;
    const double shortfall = params.required("delimitershortfall");
    const double target = (!std::isnan(minimum) && minimum != 0 || exact)
                              ? dimension
                              : (mathaccent ? std::min(dimension / factor, dimension + shortfall)
                                            : std::max(dimension * factor, dimension - shortfall));
    const auto characters = unicodeChars(getText());
    if (characters.empty())
        throw std::invalid_argument("cannot stretch an empty mo token");
    const auto original = characters.front();
    if (sourceTruthySize(size)) {
        auto const *current = jax().fonts().delimiter(jax().face().name, original);
        size.reset();
        if (!current)
            throw std::out_of_range("selected delimiter has no refreshed recipe");
        stretch = ActiveStretch{original, {}};
        active = &std::get<ActiveStretch>(stretch);
    }
    auto const *recipe = jax().fonts().delimiter(jax().face().name, active->sourceCodepoint);
    if (!recipe)
        throw std::out_of_range("missing selected delimiter recipe");
    const auto selected = active->selectedCodepoint.value_or(recipeCharacter(*recipe, original));
    for (std::size_t index = 0; index < recipe->sizes.size(); ++index) {
        if (recipe->sizes[index] >= target) {
            setDelimSize(selected, mathaccent && index ? index - 1 : index);
            return;
        }
    }
    if (!recipe->stretch.empty()) {
        size = AssemblyStretchSize{};
        invalidateBBox();
        getStretchBBox(requested, checkExtendedHeight(dimension, *recipe), *recipe);
    } else if (!recipe->sizes.empty()) {
        setDelimSize(selected, recipe->sizes.size() - 1);
    } else {
        throw std::invalid_argument("delimiter has neither a size variant nor assembly parts");
    }
}

bool CommonMo::canStretch(StretchDirection direction) {
    if (auto const *active = std::get_if<ActiveStretch>(&stretch)) {
        auto const *recipe = jax().fonts().delimiter(jax().face().name, active->sourceCodepoint);
        if (!recipe)
            throw std::out_of_range("active delimiter recipe is missing");
        return recipe->direction == direction;
    }
    if (direction == StretchDirection::none)
        return false;
    if (!truthy(attribute(*node, "stretchy"))) {
        stretch = NoStretch{};
        return false;
    }
    const auto characters = unicodeChars(getText());
    if (characters.size() != 1) {
        stretch = NoStretch{};
        return false;
    }
    auto const *recipe = jax().fonts().delimiter(jax().face().name, characters.front());
    if (!recipe || recipe->direction != direction) {
        stretch = NoStretch{};
        return false;
    }
    stretch = ActiveStretch{characters.front(), {}};
    return true;
}

} // namespace folia::math::output::common
