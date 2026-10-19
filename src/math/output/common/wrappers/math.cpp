module folia.math.output.common;
import :math;
import :output_jax;
import std;

namespace folia::math::output::common {
double CommonMath::getWrapWidth(std::size_t) {
    if (parent)
        return getBBox().w;
    return metrics().containerWidth / jax().pxPerEm();
}

bool CommonMath::setChildPWidths(bool recompute, std::optional<double> width, bool) {
    // The numerical part of Source SvgMath.setChildPWidths belongs to the
    // shared layout: both output backends consume the same resolved widths.
    return CommonWrapper::setChildPWidths(
        recompute, parent ? width : metrics().containerWidth / jax().pxPerEm(), false);
}

void CommonMath::computeBBox(BBox &box, bool recompute) {
    CommonWrapper::computeBBox(box, recompute);
    const auto *overflowValue = node->attributes().lookup(u"overflow");
    const auto *overflow = overflowValue ? std::get_if<std::u16string>(overflowValue) : nullptr;
    if (parent || !metrics().display || !overflow || *overflow != u"linebreak")
        return;

    const double width = containerWidth();
    if (box.w > width) {
        if (childNodes.empty())
            throw std::logic_error("linebreak math wrapper has no children");
        childNodes.front()->breakToWidth(width);
    }
    if (childNodes.empty())
        throw std::logic_error("linebreak math wrapper has no children");
    box.updateFrom(childNodes.front()->getBBox());
}
} // namespace folia::math::output::common
