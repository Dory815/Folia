module folia.math.output.common;
import :text_node;
import :output_jax;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.util.string;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
namespace {
mml::MmlNode *nextSibling(mml::MmlNode &parent, mml::MmlNode &child) {
    if (parent.kind() != mml::NodeKind::mrow && !parent.isInferred())
        return nullptr;
    const auto index = parent.childIndex(child);
    if (!index)
        return nullptr;
    const auto children = parent.children();
    auto next = *index + 1;
    if (next < children.size()) {
        if (auto *mo = dynamic_cast<mml::MmlMo *>(children[next]);
            mo && mo->getText() == u"\u2062") {
            ++next;
        }
    }
    return next < children.size() ? children[next] : nullptr;
}
} // namespace

std::vector<std::uint32_t> CommonTextNode::remappedText(std::u16string_view text,
                                                        std::string_view variantName) const {
    if (!parent)
        throw std::logic_error("CommonTextNode requires its borrowed parent wrapper");
    if (const auto *active = std::get_if<ActiveStretch>(&parent->stretch); active) {
        if (active->selectedCodepoint && *active->selectedCodepoint != 0)
            return {*active->selectedCodepoint};
        if (const auto *delimiter =
                jax().fonts().delimiter(jax().face().name, active->sourceCodepoint);
            delimiter && delimiter->c && *delimiter->c != 0) {
            return {*delimiter->c};
        }
    }
    return parent->remapChars(parent->unicodeChars(text, variantName));
}

void CommonTextNode::addMeasuredText(BBox &box, std::u16string_view text,
                                     std::string_view variantName, CommonOutputJax &output) {
    if (text.empty())
        return;
    const auto measured = output.measureText(text, variantName);
    updateBBox(box, measured.h, measured.d, measured.w);
}

void CommonTextNode::updateBBox(BBox &box, double height, double depth, double width) {
    box.w += width;
    if (height > box.h)
        box.h = height;
    if (depth > box.d)
        box.d = depth;
}

void CommonTextNode::computeBBox(BBox &box, bool) {
    if (!parent)
        throw std::logic_error("CommonTextNode requires its borrowed parent wrapper");
    const auto variantName = parent->variant;
    const auto *textNode = dynamic_cast<mml::TextNode *>(node);
    if (!textNode)
        throw std::logic_error("CommonTextNode requires an MML TextNode");
    const auto text = textNode->getText();
    auto &output = jax();

    if (variantName == "-explicitFont") {
        if (!parent->styles)
            throw std::logic_error("explicit-font text requires parent Styles");
        const auto font = output.getFontData(*parent->styles);
        const auto measured = output.measureText(text, variantName, font);
        box.h = measured.h;
        box.d = measured.d;
        box.w = measured.w;
        return;
    }

    const auto characters = remappedText(text, variantName);
    std::u16string unknownText;
    box.empty();
    for (std::size_t index = 0; index < characters.size(); ++index) {
        const auto resolved = getVariantChar(variantName, characters[index]);
        if (!resolved) {
            const std::array<double, 1> codepoint{static_cast<double>(characters[index])};
            unknownText += string_util::unicodeString(codepoint);
            continue;
        }

        addMeasuredText(box, unknownText, variantName, output);
        unknownText.clear();
        const auto &metrics = resolved->data->metrics;
        updateBBox(box, metrics.h, metrics.d, metrics.w);
        box.ic = metrics.ic;
        box.sk = metrics.sk;
        box.dx = metrics.dx;
        if (metrics.oc == 0 || index + 1 != characters.size())
            continue;

        const auto parentChildren = parent->childNodes;
        if (parentChildren.empty() || parentChildren.back() != this)
            continue;
        if (!parent->parent)
            throw std::logic_error("text parent wrapper has no MML parent wrapper");
        auto *outer = parent->node->parent();
        if (!outer)
            throw std::logic_error("text parent MML node has no parent");
        const auto *next = nextSibling(*outer, *parent->node);
        const auto *nextVariant = next ? next->attributes().lookup(u"mathvariant") : nullptr;
        const auto *nextVariantText =
            nextVariant ? std::get_if<std::u16string>(nextVariant) : nullptr;
        if (!nextVariantText || *nextVariantText != utf16::decode(variantName).text)
            box.ic = metrics.oc;
        else
            box.oc = metrics.oc;
    }

    addMeasuredText(box, unknownText, variantName, output);
    if (characters.size() > 1)
        box.sk = 0;
    box.clean();
}
} // namespace folia::math::output::common
