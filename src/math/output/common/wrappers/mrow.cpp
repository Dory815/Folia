/*************************************************************
 * Candidate native port of MathJax 4.1.3 output/common/Wrappers/mrow.ts.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :mrow;
import :wrapper_factory;
import :output_jax;
import folia.math.util.utf16;
import std;

namespace folia::math::output::common {
CommonMrow::CommonMrow(CommonWrapperFactory &factory, mml::MmlNode &source,
                       CommonWrapper *parentValue)
    : CommonWrapper(factory, source, parentValue) {
}

void CommonMrow::initializeSource() {
    CommonWrapper::initializeSource();
    isStack = !parent || parent->node->isInferred() || parent->breakTop(this, this) != this;
    stretchChildren();
    for (auto *child : childNodes) {
        if (!child)
            throw std::logic_error("CommonMrow has a null child wrapper");
        if (!child->bbox.pwidth.empty()) {
            bbox.pwidth = std::string(BBox::fullWidth);
            break;
        }
    }
}

bool CommonMrow::fixesPWidth() const {
    return false;
}

std::size_t CommonMrow::breakCount() {
    if (_breakCount < 0) {
        _breakCount = 0;
        for (auto *child : childNodes) {
            if (!child)
                throw std::logic_error("CommonMrow has a null child wrapper");
            _breakCount += static_cast<std::int64_t>(child->breakCount());
        }
    }
    return static_cast<std::size_t>(_breakCount);
}

CommonWrapper *CommonMrow::breakTop(CommonWrapper *, CommonWrapper *) {
    if (!isStack)
        return this;
    if (!parent)
        throw std::logic_error("stack mrow requires a parent wrapper");
    return parent->breakTop(this, this);
}

void CommonMrow::stretchChildren() {
    std::vector<CommonWrapper *> stretchy;
    for (auto *child : childNodes) {
        if (!child)
            throw std::logic_error("CommonMrow has a null child wrapper");
        if (child->canStretch(StretchDirection::vertical))
            stretchy.push_back(child);
    }
    const auto count = stretchy.size();
    const auto nodeCount = childNodes.size();
    if (!count || nodeCount <= 1)
        return;

    double height = 0;
    double depth = 0;
    const bool all = count > 1 && count == nodeCount;
    for (auto *child : childNodes) {
        const bool noStretch = std::holds_alternative<NoStretch>(child->stretch);
        if (all || noStretch) {
            const auto scale = child->getBBox().rscale;
            const auto hd = child->getUnbrokenHD();
            height = std::max(height, hd[0] * scale);
            depth = std::max(depth, hd[1] * scale);
        }
    }
    for (auto *child : stretchy) {
        const auto scale = child->coreRScale();
        if (scale == 0)
            throw std::domain_error("stretch core relative scale is zero");
        auto *core = dynamic_cast<CommonMo *>(child->coreMO());
        if (!core)
            throw std::logic_error("stretchable wrapper has no core mo");
        core->getStretchedVariant(std::array<double, 2>{height / scale, depth / scale}, false);
    }
}

void CommonMrow::computeBBox(BBox &target, bool recompute) {
    const auto breaks = breakCount();
    lineBBox.clear();
    if (breaks)
        lineBBox.push_back(std::make_shared<LineBBox>(BBoxData{.w = 0.0, .h = .75, .d = .25},
                                                      std::array<double, 2>{0, 0}));

    target.empty();
    for (std::size_t index = 0; index < childNodes.size(); ++index) {
        auto *child = childNodes[index];
        target.append(child->getOuterBBox());
        if (breaks)
            computeChildLineBBox(*child, index);
    }
    target.clean();
    if (breaks) {
        auto *core = coreMO();
        if (!core)
            throw std::logic_error("breaking mrow has no core mo");
        if (!core->node->isEmbellished())
            computeLinebreakBBox(target);
    }
    if (fixesPWidth() && setChildPWidths(recompute) && !recompute)
        computeBBox(target, true);
    vboxAdjust(target);
}

void CommonMrow::computeChildLineBBox(CommonWrapper &child, std::size_t index) {
    if (lineBBox.empty())
        throw std::logic_error("linebreak count has no initial line bbox");
    auto &last = *lineBBox.back();
    last.end = std::array<double, 2>{static_cast<double>(index), 0};
    last.append(*child.getLineBBox(0));
    for (std::size_t line = 1, parts = child.breakCount() + 1; line < parts; ++line) {
        auto next = std::make_shared<LineBBox>(BBoxData{.w = 0.0, .h = .75, .d = .25});
        next->start = next->end =
            std::array<double, 2>{static_cast<double>(index), static_cast<double>(line)};
        next->isFirst = true;
        next->append(*child.getLineBBox(line));
        lineBBox.push_back(std::move(next));
    }
}

void CommonMrow::computeLinebreakBBox(BBox &target) {
    target.empty();
    if (lineBBox.empty())
        return;
    const auto last = lineBBox.size() - 1;
    if (isStack) {
        for (std::size_t index = 0; index < lineBBox.size(); ++index) {
            auto &line = *lineBBox[index];
            addMiddleBorders(line);
            if (index == 0)
                addLeftBorders(line);
            if (index == last)
                addRightBorders(line);
        }
    }
    double y = 0;
    for (std::size_t index = 0; index < lineBBox.size(); ++index) {
        auto const &line = *lineBBox[index];
        target.combine(line, 0, y);
        const double nextHeight = index + 1 < lineBBox.size() ? lineBBox[index + 1]->h : 0;
        y -= std::max(.25, line.d) + line.lineLeading.value_or(0) + std::max(.75, nextHeight);
    }
    if (isStack) {
        lineBBox.front()->L = bbox.L;
        lineBBox.back()->R = bbox.R;
    } else {
        for (auto const &line : lineBBox)
            target.w = std::max(target.w, line->w);
        shiftLines(target);
        if (!metrics().display && !linebreakOptions().inlineBreaks) {
            target.pwidth = std::string(BBox::fullWidth);
            if (node->isInferred()) {
                if (!parent)
                    throw std::logic_error("inferred linebreaking mrow requires a parent");
                parent->bbox.pwidth = std::string(BBox::fullWidth);
            }
        }
    }
    target.clean();
}

void CommonMrow::vboxAdjust(BBox &target) {
    if (!parent)
        return;
    const auto count = breakCount();
    const auto *valignValue = parent->node->attributes().lookup(u"data-vertical-align");
    const auto *valign = valignValue ? std::get_if<std::u16string>(valignValue) : nullptr;
    const bool bottom = valign && *valign == u"bottom";
    const bool center = valign && *valign == u"center";
    const bool middle = valign && *valign == u"middle";
    if (count && bottom) {
        dh = target.d - lineBBox.at(count - 1)->d;
    } else if (center || (count && middle)) {
        dh = (target.h + target.d) / 2 + jax().face().parameters.required("axis_height") - target.h;
    } else {
        dh = 0;
        return;
    }
    target.h += dh;
    target.d -= dh;
}

std::shared_ptr<LineBBox> CommonMrow::getLineBBox(std::size_t index) {
    (void)getBBox();
    if (isStack)
        return CommonWrapper::getLineBBox(index);
    return std::make_shared<LineBBox>(
        LineBBox::from(getOuterBBox(), linebreakOptions().lineleading));
}

void CommonMrow::shiftLines(BBox &outer) {
    if (lineBBox.size() < 2)
        throw std::logic_error("line indentation requires a following line");
    const double width = outer.w;
    const auto readPair = [](LineBBox const &line, std::size_t which) -> std::optional<IndentPair> {
        if (!line.indentData)
            return std::nullopt;
        return line.indentData->lines.at(which);
    };
    const auto firstValue = readPair(*lineBBox[1], 0);
    const IndentPair first = firstValue.value_or(IndentPair{u"left", u"0"});
    const std::string firstAlign = first.alignment ? utf16::encode(*first.alignment) : "";
    const std::string firstShift = first.shift ? utf16::encode(*first.shift) : "";
    const auto last = lineBBox.size() - 1;
    for (std::size_t index = 0; index < lineBBox.size(); ++index) {
        auto &line = *lineBBox[index];
        const auto pairValue =
            index == 0 ? std::optional<IndentPair>{first} : readPair(line, index == last ? 2 : 1);
        const IndentPair pair = pairValue.value_or(IndentPair{u"left", u"0"});
        const std::string align = pair.alignment ? utf16::encode(*pair.alignment) : "";
        const std::string shift = pair.shift ? utf16::encode(*pair.shift) : "";
        auto const processed = processIndent(align, shift, firstAlign, firstShift, width);
        line.L = 0;
        line.L = getAlignX(width, line, processed.first) + processed.second;
        outer.w = std::max(outer.w, line.L + line.w);
    }
}

bool CommonMrow::setChildPWidths(bool recompute, std::optional<double> width, bool clear) {
    if (!breakCount())
        return CommonWrapper::setChildPWidths(recompute, width, clear);
    if (recompute)
        return false;
    if (width && bbox.w != *width) {
        bbox.w = *width;
        shiftLines(bbox);
    }
    return true;
}

void CommonMrow::breakToWidth(double width) {
    linebreaks().breakToWidth(*this, width);
}
} // namespace folia::math::output::common
