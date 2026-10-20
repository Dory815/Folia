/*************************************************************
 * Native CommonMenclose algorithms from MathJax 4.1.3.
 *************************************************************/
module folia.math.output.common;
import :menclose;
import :wrapper;
import :wrapper_factory;
import folia.math.core.mml_tree.mml_document;
import folia.math.util.js_numeric_input;
import folia.math.util.string;
import std;

namespace folia::math::output::common {
namespace {
using mml::AttributeNull;
using mml::AttributeValue;

[[noreturn]] void missingChild(char const *operation) {
    throw std::logic_error(std::string("menclose requires a child for ") + operation);
}

mml::AttributeValue const *attribute(CommonMenclose const &wrapper, std::u16string_view name) {
    return wrapper.node->attributes().lookup(name);
}

LengthInput lengthInput(AttributeValue const &value) {
    if (auto const *number = std::get_if<double>(&value))
        return *number;
    if (auto const *text = std::get_if<std::u16string>(&value))
        return *text;
    throw std::invalid_argument("menclose length attribute must be numeric or text");
}

double parseFloatPrefix(std::u16string_view value, double fallback) {
    if (value.empty())
        return fallback;
    std::size_t start = 0;
    while (start < value.size() && js_input::isWhitespace(value[start]))
        ++start;
    std::size_t end = start;
    if (end < value.size() && (value[end] == u'+' || value[end] == u'-'))
        ++end;
    constexpr std::u16string_view infinity = u"Infinity";
    if (value.substr(end, infinity.size()) == infinity)
        return js_input::number(value.substr(start, end - start + infinity.size()));
    std::size_t digits = 0;
    while (end < value.size() && value[end] >= u'0' && value[end] <= u'9') {
        ++end;
        ++digits;
    }
    if (end < value.size() && value[end] == u'.') {
        ++end;
        while (end < value.size() && value[end] >= u'0' && value[end] <= u'9') {
            ++end;
            ++digits;
        }
    }
    if (digits == 0)
        return std::numeric_limits<double>::quiet_NaN();
    if (end < value.size() && (value[end] == u'e' || value[end] == u'E')) {
        const auto exponent = end++;
        if (end < value.size() && (value[end] == u'+' || value[end] == u'-'))
            ++end;
        const auto exponentDigits = end;
        while (end < value.size() && value[end] >= u'0' && value[end] <= u'9')
            ++end;
        if (end == exponentDigits)
            end = exponent;
    }
    return js_input::number(value.substr(start, end - start));
}

} // namespace

CommonMenclose::CommonMenclose(CommonWrapperFactory &factory, mml::MmlNode &node,
                               CommonWrapper *parent, std::span<NotationSpec const> profileCatalog)
    : CommonWrapper(factory, node, parent), catalog_(profileCatalog) {
}

void CommonMenclose::initializeSource() {
    CommonWrapper::initializeSource();
    padding = defaultPadding;
    thickness = defaultThickness;
    arrowhead = defaultArrowhead;
    renderChild_.reset();
    msqrt_ = nullptr;
    notations_.clear();
    trbl_ = {};
    getParameters();
    getNotations();
    removeRedundantNotations();
    initializeNotations();
    trbl_ = getBBoxExtenders();
}

void CommonMenclose::getParameters() {
    if (auto const *value = attribute(*this, u"data-padding"))
        padding = length2em(lengthInput(*value), defaultPadding);
    if (auto const *value = attribute(*this, u"data-thickness"))
        thickness = length2em(lengthInput(*value), defaultThickness);
    if (auto const *value = attribute(*this, u"data-arrowhead")) {
        auto const *text = std::get_if<std::u16string>(value);
        if (!text)
            throw std::invalid_argument("data-arrowhead must be text");
        const auto parts = string_util::split(*text);
        arrowhead = {
            parts.empty() || parts[0].empty() ? defaultArrowhead.x
                                              : parseFloatPrefix(parts[0], defaultArrowhead.x),
            parts.size() < 2 || parts[1].empty() ? defaultArrowhead.y
                                                 : parseFloatPrefix(parts[1], defaultArrowhead.y),
            parts.size() < 3 || parts[2].empty() ? defaultArrowhead.dx
                                                 : parseFloatPrefix(parts[2], defaultArrowhead.dx)};
    }
}

void CommonMenclose::getNotations() {
    auto const *value = attribute(*this, u"notation");
    if (!value || std::holds_alternative<AttributeNull>(*value))
        throw std::invalid_argument("menclose notation attribute is absent or null");
    auto const *text = std::get_if<std::u16string>(value);
    if (!text)
        throw std::invalid_argument("menclose notation attribute must be text");
    for (const auto &name : string_util::split(*text)) {
        auto kind = findNotation(catalog_, name);
        if (!kind)
            continue; // Source Map.get(name) ignores unknown spellings.
        auto found = std::find_if(catalog_.begin(), catalog_.end(),
                                  [&](auto const &spec) { return spec.kind == *kind; });
        if (found == catalog_.end())
            throw std::logic_error("notation catalog lookup lost its typed entry");
        auto existing = std::find_if(notations_.begin(), notations_.end(),
                                     [&](auto const *spec) { return spec->kind == *kind; });
        if (existing == notations_.end())
            notations_.push_back(&*found);
        else
            *existing = &*found;
        if (found->render_child)
            renderChild_ = found->kind;
    }
}

void CommonMenclose::removeNotation(std::string_view name) {
    notations_.erase(std::remove_if(notations_.begin(), notations_.end(),
                                    [&](auto const *spec) { return spec->source_name == name; }),
                     notations_.end());
    if (renderChild_ && std::none_of(notations_.begin(), notations_.end(),
                                     [&](auto const *spec) { return spec->kind == *renderChild_; }))
        renderChild_.reset();
}

void CommonMenclose::removeRedundantNotations() {
    const auto selected = notations_; // JS Object.keys snapshots keys before deletion.
    for (auto const *spec : selected) {
        if (std::find(notations_.begin(), notations_.end(), spec) == notations_.end())
            continue;
        for (std::string_view name : spec->removes)
            removeNotation(name);
    }
}

void CommonMenclose::initializeNotations() {
    if (!renderChild_ || *renderChild_ != NotationKind::radical)
        return;
    if (childNodes.empty() || !childNodes.front())
        missingChild("radical initialization");
    msqrt_ = &createMsqrt(*childNodes.front());
}

void CommonMenclose::maximizeEntries(PaddingData &current, PaddingData const &next) noexcept {
    for (std::size_t i = 0; i < current.size(); ++i)
        if (current[i] < next[i])
            current[i] = next[i];
}

BBox const *CommonMenclose::childBoxFor(NotationSpec const &spec) const {
    if (spec.bbox != BBoxRecipe::arrow && spec.bbox != BBoxRecipe::diagonal_arrow)
        return nullptr;
    if (childNodes.empty() || !childNodes.front())
        missingChild("arrow extent calculation");
    return &childNodes.front()->getBBox();
}

PaddingData CommonMenclose::getBBoxExtenders() const {
    PaddingData result{};
    const PaddingData radical = msqrt_ ? sqrtTRBL() : PaddingData{};
    for (auto const *spec : notations_) {
        if (spec->bbox == BBoxRecipe::radical && !msqrt_)
            throw std::logic_error("selected radical notation was not initialized");
        const auto extents =
            notationBBox(*spec, padding, thickness, arrowhead, childBoxFor(*spec), radical);
        maximizeEntries(result, extents);
    }
    return result;
}

PaddingData CommonMenclose::getPadding() const {
    PaddingData borders{};
    for (auto const *spec : notations_)
        maximizeEntries(borders, notationBorder(*spec, thickness));
    PaddingData result{};
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = trbl_[i] - borders[i];
    return result;
}

double CommonMenclose::getOffset(std::string_view direction) const noexcept {
    return ::folia::math::output::common::getOffset(trbl_, direction);
}

std::pair<double, double> CommonMenclose::getArgMod(double width, double height) const noexcept {
    return ::folia::math::output::common::getArgMod(width, height);
}

ArrowGeometry CommonMenclose::arrow(double width, double angle, bool doubleHeaded,
                                    std::string_view offset, double distance) {
    return arrowGeometry(width, angle, doubleHeaded, offset, distance, thickness, arrowhead,
                         getBBox());
}

ArrowMetrics CommonMenclose::arrowData() const {
    if (childNodes.empty() || !childNodes.front())
        missingChild("arrowData");
    return ::folia::math::output::common::arrowData(padding, thickness, arrowhead,
                                                    childNodes.front()->getBBox());
}

std::pair<double, double> CommonMenclose::arrowAW() const {
    if (childNodes.empty() || !childNodes.front())
        missingChild("arrowAW");
    return ::folia::math::output::common::arrowAW(trbl_, childNodes.front()->getBBox());
}

CommonMsqrt &CommonMenclose::createMsqrt(CommonWrapper &child) {
    auto &document = node->document();
    auto &sqrt = document.factory().make(mml::NodeKind::msqrt);
    sqrt.inheritAttributesFrom(*node);
    if (!sqrt.children().empty())
        document.remove(*sqrt.children().front());
    auto projected = std::make_unique<CommonMsqrt>(factory(), sqrt, this, child.node);
    auto &result = static_cast<CommonMsqrt &>(factory().adopt(std::move(projected)));
    jax().registerWrapper(sqrt, result);
    msqrt_ = &result;
    return result;
}

PaddingData CommonMenclose::sqrtTRBL() const {
    if (!msqrt_)
        throw std::logic_error("radical notation has no synthetic msqrt wrapper");
    if (msqrt_->childNodes.empty() || !msqrt_->childNodes.front())
        missingChild("sqrtTRBL");
    const auto &outer = msqrt_->getBBox();
    const auto &inner = msqrt_->childNodes.front()->getBBox();
    return {outer.h - inner.h, 0, outer.d - inner.d, outer.w - inner.w};
}

std::vector<NotationGeometry> CommonMenclose::sceneGeometry() {
    std::vector<NotationGeometry> result;
    const auto &box = getBBox();
    if (childNodes.empty() || !childNodes.front())
        missingChild("Scene geometry");
    const auto &childBox = childNodes.front()->getBBox();
    for (auto const *spec : notations_) {
        if (spec->render_child)
            continue;
        auto entries = ::folia::math::output::common::notationGeometry(
            *spec, box, thickness, padding, arrowhead, trbl_, &childBox);
        result.insert(result.end(), std::make_move_iterator(entries.begin()),
                      std::make_move_iterator(entries.end()));
    }
    return result;
}

SceneNodes CommonMenclose::emitScene(scene::Builder &builder,
                                     std::span<scene::NodeId const> parents) {
    for (const auto *spec : notations_) {
        if (spec->render_child && spec->kind == NotationKind::radical)
            continue;
        switch (spec->paint) {
        case PaintRecipe::side_line:
        case PaintRecipe::diagonal_line:
        case PaintRecipe::horizontal_line:
        case PaintRecipe::vertical_line:
            break;
        default:
            throw std::logic_error("CommonMenclose Scene emission does not support notation: " +
                                   std::string(spec->source_name));
        }
    }

    auto containers = standardSceneNodes(builder, parents);
    auto &nodes = containers.nodes;
    if (nodes.empty() || !nodes.front())
        throw std::logic_error("CommonMenclose requires its first Source output segment");
    if (childNodes.empty() || !childNodes.front())
        missingChild("Scene emission");

    const double left = trbl_[3];
    scene::Transform blockTransform;
    if (left > 0)
        blockTransform = scene::Transform(
            std::vector<scene::TransformOperation>{scene::Translation{left, 0}});
    const auto block = builder.add_group(*nodes.front(), blockTransform, {}, {}, {}, {},
                                         scene::ElementRole::enclosure_content);
    const std::array<scene::NodeId, 1> blockParent{block};

    if (renderChild_ == NotationKind::radical) {
        if (!msqrt_)
            throw std::logic_error("CommonMenclose radical child has no msqrt wrapper");
        const auto radicalNodes = msqrt_->emitScene(builder, blockParent);
        this->placeScene(builder, radicalNodes, -sqrtTRBL()[3], 0, block);
    } else {
        const auto emittedChild = this->childNodes.front()->emitScene(builder, blockParent);
        this->childNodes.front()->placeScene(builder, emittedChild, 0, 0);
    }

    for (auto geometry : sceneGeometry()) {
        if (!geometry.transform.operations().empty())
            throw std::logic_error("CommonMenclose line notation unexpectedly needs a transform");
        scene::Path path;
        path.geometry = std::move(geometry.path);
        path.paint = std::move(geometry.paint);
        path.role = scene::ElementRole::enclosure_mark;
        path.element = scene::PathElement::line;
        builder.add_primitive(*nodes.front(), std::move(path));
    }
    return nodes;
}

void CommonMenclose::computeBBox(BBox &box, bool recompute) {
    if (childNodes.empty() || !childNodes.front())
        missingChild("bbox computation");
    auto *child = childNodes.front();
    box.combine(child->getBBox(), trbl_[3], 0);
    box.h += trbl_[0];
    box.d += trbl_[2];
    box.w += trbl_[1];
    (void)setChildPWidths(recompute);
}
} // namespace folia::math::output::common
