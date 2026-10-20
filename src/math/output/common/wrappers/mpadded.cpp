/*************************************************************
 * Native CommonMpadded methods mirroring MathJax 4.1.3.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :mpadded;
import :wrapper;
import folia.math.core.mml_tree.mml_nodes.mpadded;
import folia.math.util.js_number;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
namespace {
using mml::AttributeNull;
using mml::AttributeValue;

AttributeValue const *attribute(mml::MmlNode const &node, std::u16string_view name) {
    return node.attributes().lookup(name);
}

std::u16string stringValue(AttributeValue const *value) {
    if (!value)
        return {};
    if (auto const *text = std::get_if<std::u16string>(value))
        return *text;
    if (auto const *boolean = std::get_if<bool>(value))
        return *boolean ? u"true" : u"false";
    if (auto const *number = std::get_if<double>(value))
        return utf16::decode(js_number::toString(*number)).text;
    if (std::holds_alternative<AttributeNull>(*value))
        return u"null";
    throw std::logic_error("unhandled MML attribute value");
}

bool truthy(AttributeValue const *value) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return false;
    if (auto const *boolean = std::get_if<bool>(value))
        return *boolean;
    if (auto const *number = std::get_if<double>(value))
        return *number != 0.0 && !std::isnan(*number);
    return !std::get<std::u16string>(*value).empty();
}

CommonWrapper &content(CommonMpadded &wrapper) {
    if (wrapper.childNodes.empty() || !wrapper.childNodes.front())
        throw std::out_of_range("mpadded requires its first Common child");
    return *wrapper.childNodes.front();
}
} // namespace

SceneNodes CommonMpadded::emitScene(scene::Builder &builder,
                                    std::span<scene::NodeId const> parents) {
    if (auto embellished = toEmbellishedScene(builder, parents))
        return *embellished;

    auto containers = standardSceneNodes(builder, parents);
    const auto [H, D, W, dh, dd, dw, x, y, dx] = getDimens();
    (void)H;
    (void)D;
    (void)W;
    (void)dh;
    (void)dd;
    const auto *alignValue = attribute(*node, u"data-align");
    const auto align = truthy(alignValue) ? stringValue(alignValue) : std::u16string(u"left");
    const double dW = dw < 0 && align != u"left" ? (align == u"center" ? dw / 2 : dw) : 0;
    const double X = x + dx - dW;
    const auto hasShift = [](double value) { return value != 0.0 && !std::isnan(value); };

    SceneParents childParents;
    if (hasShift(X) || hasShift(y)) {
        if (containers.nodes.empty() || !containers.nodes.front())
            throw std::logic_error(
                "shifted mpadded Scene emission requires its first output segment");
        const auto group =
            builder.add_group(*containers.nodes.front(), {}, {}, {}, {}, scene::SemanticMetadata{},
                              scene::ElementRole::anonymous_group);
        const std::array<SceneNode, 1> emittedGroup{SceneNode{group}};
        this->placeScene(builder, emittedGroup, X, y, group);
        childParents.push_back(group);
    } else {
        childParents = std::move(containers.content);
    }

    addSceneChildren(builder, childParents);
    return std::move(containers.nodes);
}

double CommonMpadded::containerWidth() const {
    const auto width = stringValue(attribute(*node, u"width"));
    const auto overflow = stringValue(attribute(*node, u"data-overflow"));
    const bool relative =
        !width.empty() && (width.front() == u'+' || width.front() == u'-' || width.back() == u'%');
    if (!relative && overflow == u"linebreak")
        return length2em(LengthInput(width));
    if (!parent)
        throw std::logic_error("mpadded needs its parent container width");
    return parent->containerWidth();
}

std::array<double, 9> CommonMpadded::getDimens() {
    const auto box = content(*this).getOuterBBox();
    const double W = box.w, H = box.h, D = box.d;
    double w = W, h = H, d = D, x = 0, y = 0, dx = 0;
    const auto width = stringValue(attribute(*node, u"width"));
    const auto height = stringValue(attribute(*node, u"height"));
    const auto depth = stringValue(attribute(*node, u"depth"));
    const auto lspace = stringValue(attribute(*node, u"lspace"));
    const auto voffset = stringValue(attribute(*node, u"voffset"));
    if (!width.empty())
        w = dimen(width, box, "w", 0);
    if (!height.empty())
        h = dimen(height, box, "h", 0);
    if (!depth.empty())
        d = dimen(depth, box, "d", 0);
    if (!voffset.empty())
        y = dimen(voffset, box);
    if (!lspace.empty())
        x = dimen(lspace, box);

    const auto *align = attribute(*node, u"data-align");
    if (truthy(align))
        dx = getAlignX(w, box, utf16::encode(stringValue(align)));
    return {H, D, W, h - H, d - D, w - W, x, y, dx};
}

double CommonMpadded::dimen(std::u16string_view length, BBox const &box, std::string_view direction,
                            std::optional<double> minimum) const {
    const auto width = length.find(u"width");
    const auto height = length.find(u"height");
    const auto depth = length.find(u"depth");
    const auto at = std::min({width, height, depth});
    double size = 0;
    if (at != std::u16string_view::npos) {
        if (at == width)
            size = box.w;
        else if (at == height)
            size = box.h;
        else
            size = box.d;
    } else if (!direction.empty()) {
        if (direction == "w")
            size = box.w;
        else if (direction == "h")
            size = box.h;
        else if (direction == "d")
            size = box.d;
    }

    double result = length2em(LengthInput(std::u16string(length)), size);
    if (result == 0.0 || std::isnan(result))
        result = 0.0;
    if (!length.empty() && (length.front() == u'+' || length.front() == u'-') && !direction.empty())
        result += size;
    if (minimum)
        result = std::max(*minimum, result);
    return result;
}

void CommonMpadded::setBBoxDimens(BBox &box) {
    const auto [H, D, W, dh, dd, dw, x, y, dx] = getDimens();
    (void)x;
    (void)y;
    (void)dx;
    box.w = W + dw;
    box.h = H + dh;
    box.d = D + dd;
}

void CommonMpadded::computeBBox(BBox &box, bool recompute) {
    setBBoxDimens(box);
    auto &child = content(*this);
    if (child.getOuterBBox().w > box.w) {
        const auto overflow = stringValue(attribute(*node, u"data-overflow"));
        bool rootLinebreak = false;
        if (overflow == u"auto") {
            const auto *root = node->document().root();
            if (!root)
                throw std::logic_error("mpadded auto-overflow requires an MML document root");
            rootLinebreak = stringValue(attribute(*root, u"overflow")) == u"linebreak";
        }
        if (overflow == u"linebreak" || (overflow == u"auto" && rootLinebreak)) {
            child.breakToWidth(box.w);
            setBBoxDimens(box);
        }
    }
    (void)setChildPWidths(recompute, box.w);
}

std::string CommonMpadded::getChildAlign(std::size_t) const {
    const auto *align = attribute(*node, u"data-align");
    return truthy(align) ? utf16::encode(stringValue(align)) : "left";
}

double CommonMpadded::getWrapWidth(std::size_t) {
    return getBBox().w;
}
} // namespace folia::math::output::common
