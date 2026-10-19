/*************************************************************
 * Candidate native port of MathJax 4.1.3 output/common/LinebreakVisitor.ts.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common;
import :linebreak_visitor;
import :output_jax;
import :mo;
import :mspace;
import :mrow;
import :mfrac;
import :msqrt;
import :mroot;
import :mfenced;
import :msubsup;
import :mmultiscripts;
import :mtext;
import :maction;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mmultiscripts;
import folia.math.util.utf16;
import folia.math.util.js_number;
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

std::string attributeString(AttributeValue const *value, std::string fallback = {}) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return fallback;
    if (const auto *text = std::get_if<std::u16string>(value))
        return utf16::encode(*text);
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean ? "true" : "false";
    return js_number::toString(std::get<double>(*value));
}

double jsMax(double left, double right) {
    if (std::isnan(left) || std::isnan(right))
        return std::numeric_limits<double>::quiet_NaN();
    return std::max(left, right);
}

std::optional<double> lineLeading(CommonWrapper const &wrapper) {
    return wrapper.linebreakOptions().lineleading;
}

std::vector<std::u16string> splitSpaces(std::u16string_view value) {
    std::vector<std::u16string> words;
    std::size_t start = 0;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == u' ') {
            words.emplace_back(value.substr(start, i - start));
            start = i + 1;
        }
    }
    words.emplace_back(value.substr(start));
    return words;
}

CommonWrapper &requireChild(CommonWrapper &wrapper, std::size_t index, std::string_view operation) {
    if (index >= wrapper.childNodes.size() || !wrapper.childNodes[index])
        throw std::logic_error(std::string(operation) + " requires an existing child wrapper");
    return *wrapper.childNodes[index];
}

CommonMo &requireMo(CommonWrapper &wrapper) {
    auto *result = dynamic_cast<CommonMo *>(&wrapper);
    if (!result)
        throw std::logic_error("mo factory returned an incompatible Common wrapper");
    return *result;
}
} // namespace

LinebreakVisitorBase::HandlerTable LinebreakVisitor::handlers(LinebreakVisitor *self) {
    LinebreakVisitorBase::HandlerTable result;
    const auto add = [&result, self](mml::NodeKind kind, auto method) {
        result.emplace(kind, [self, method](CommonWrapper &wrapper, std::size_t const &line) {
            (self->*method)(wrapper, line);
        });
    };
    add(mml::NodeKind::mo, &LinebreakVisitor::visitMoNode);
    add(mml::NodeKind::mspace, &LinebreakVisitor::visitMspaceNode);
    add(mml::NodeKind::mtext, &LinebreakVisitor::visitMtextNode);
    add(mml::NodeKind::mrow, &LinebreakVisitor::visitMrowNode);
    add(mml::NodeKind::inferred_mrow, &LinebreakVisitor::visitInferredMrowNode);
    add(mml::NodeKind::mfrac, &LinebreakVisitor::visitMfracNode);
    add(mml::NodeKind::msqrt, &LinebreakVisitor::visitMsqrtNode);
    add(mml::NodeKind::mroot, &LinebreakVisitor::visitMrootNode);
    add(mml::NodeKind::msub, &LinebreakVisitor::visitMsubNode);
    add(mml::NodeKind::msup, &LinebreakVisitor::visitMsupNode);
    add(mml::NodeKind::msubsup, &LinebreakVisitor::visitMsubsupNode);
    add(mml::NodeKind::mmultiscripts, &LinebreakVisitor::visitMmultiscriptsNode);
    add(mml::NodeKind::mfenced, &LinebreakVisitor::visitMfencedNode);
    add(mml::NodeKind::maction, &LinebreakVisitor::visitMactionNode);
    return result;
}

tree::VisitorChildren<CommonWrapper> LinebreakVisitor::liveChildren() {
    return {
        [](CommonWrapper &wrapper) { return wrapper.childNodes.size(); },
        [](CommonWrapper &wrapper, std::size_t index) -> CommonWrapper * {
            return wrapper.childNodes.at(index);
        },
    };
}

LinebreakVisitor::LinebreakVisitor(CommonWrapperFactory &factory)
    : Linebreaks(factory),
      LinebreakVisitorBase(
          static_cast<tree::FactoryServices<CommonWrapper, mml::NodeKind, WrapperBuildArguments> &>(
              factory),
          handlers(this), liveChildren()) {
}

void LinebreakVisitor::visitNode(CommonWrapper &wrapper, std::size_t const &line) {
    ++state_.depth;
    if (wrapper.node->isEmbellished() && wrapper.node->kind() != mml::NodeKind::mo)
        visitEmbellishedOperator(wrapper, line);
    else
        LinebreakVisitorBase::visitNode(wrapper, line);
    --state_.depth;
}

void LinebreakVisitor::addWidth(BBox const &box, std::optional<double> width) {
    double value = width.value_or(box.L + box.w + box.R);
    if (!value)
        return;
    value *= box.rscale;
    state_.w += value;
    if (!state_.potential.empty())
        state_.potential.front().widthToNext += value;
    processBreak();
}

void LinebreakVisitor::processBreak() {
    while (!state_.potential.empty() && state_.w > state_.width) {
        const auto point = state_.potential.back();
        state_.potential.pop_back();
        state_.breaks.push_back({point.identity, point.wrapperAndIndex});
        double newWidth = point.afterWidth + point.widthToNext;
        for (const auto &candidate : state_.potential)
            newWidth += candidate.widthToNext;
        state_.w = newWidth;
        if (state_.prevBreak && state_.prevWidth + point.beforeWidth <= state_.width) {
            std::erase_if(state_.breaks, [&](BreakEntry const &entry) {
                return entry.identity == state_.prevBreak->identity;
            });
            state_.prevWidth += point.beforeWidth;
        } else {
            state_.prevWidth = point.beforeWidth + point.afterWidth;
        }
        for (auto &candidate : state_.potential)
            candidate.beforeWidth -= point.beforeWidth;
        state_.prevBreak = point;
        state_.mathLeft -= point.beforeWidth;
    }
}

void LinebreakVisitor::pushBreak(CommonWrapper &wrapper, double penalty, double width,
                                 IndexData index) {
    if (penalty >= NOBREAK || (state_.w == 0 && state_.prevWidth == 0))
        return;
    while (!state_.potential.empty() && state_.potential.front().penalty > penalty * .99) {
        const auto removed = state_.potential.front();
        state_.potential.erase(state_.potential.begin());
        if (!state_.potential.empty())
            state_.potential.front().widthToNext += removed.widthToNext;
    }
    BreakData point;
    point.identity = ++state_.nextIdentity;
    point.wrapperAndIndex = {&wrapper, index};
    point.penalty = penalty;
    point.beforeWidth = state_.w - (state_.prevBreak ? state_.prevBreak->afterWidth : 0);
    point.afterWidth = width;
    state_.potential.insert(state_.potential.begin(), std::move(point));
}

LinebreakState LinebreakVisitor::createState(CommonWrapper &wrapper) {
    const double width = wrapper.getBBox().w;
    LinebreakState result;
    result.mathWidth = width;
    result.mathLeft = width;
    return result;
}

std::pair<double, double> LinebreakVisitor::borderLR(CommonWrapper const &wrapper) const {
    if (!wrapper.styleData)
        return {0, 0};
    const auto &data = *wrapper.styleData;
    const auto border = data.border ? data.border->width : std::array<double, 4>{};
    return {border[3] + data.padding[3], border[1] + data.padding[1]};
}

void LinebreakVisitor::breakLineToWidth(CommonWrapper &wrapper, std::size_t line) {
    state_.potential.clear();
    state_.w = state_.prevWidth = 0;
    state_.prevBreak.reset();
    state_.depth = 0;
    visitNode(wrapper, line);
}

void LinebreakVisitor::breakToWidth(CommonWrapper &wrapper, double width) {
    auto previous = std::move(state_);
    state_ = createState(wrapper);
    state_.width = width;
    const auto count = wrapper.breakCount();
    for (std::size_t line = 0; line <= count; ++line) {
        auto box = line < wrapper.lineBBox.size() && wrapper.lineBBox[line]
                       ? wrapper.lineBBox[line]
                       : wrapper.getLineBBox(line);
        if (box->w > width)
            breakLineToWidth(wrapper, line);
    }
    const auto breaks = state_.breaks;
    for (const auto &entry : breaks) {
        auto *target = entry.wrapperAndIndex.first;
        if (!target)
            continue;
        const auto &index = entry.wrapperAndIndex.second;
        if (!index) {
            auto *core = target->coreMO();
            if (!core)
                throw std::logic_error("linebreak target has no core wrapper");
            const auto *raw = core->node->attributes().lookup(u"linebreakstyle");
            const auto style = truthy(raw) ? attributeString(raw) : "before";
            if (auto *mo = dynamic_cast<CommonMo *>(core))
                mo->setBreakStyle(style);
            else if (auto *space = dynamic_cast<CommonMspace *>(core))
                space->setBreakStyle(style);
            else
                throw std::logic_error("linebreak target core cannot accept a break style");
        } else {
            auto *text = dynamic_cast<CommonMtext *>(target);
            if (!text)
                throw std::logic_error("mtext breakpoint has a non-mtext target");
            text->setBreakAt(*index);
        }
        target->invalidateBBox();
    }
    state_ = std::move(previous);
}

void LinebreakVisitor::visitDefault(CommonWrapper &wrapper, std::size_t const &index) {
    auto line = wrapper.getLineBBox(index);
    if (wrapper.node->isToken() || wrapper.node->linebreakContainer() ||
        wrapper.childNodes.empty()) {
        addWidth(*line);
        return;
    }
    const auto [left, right] = borderLR(wrapper);
    if (index == 0)
        addWidth(*line, line->L + left);
    visitNode(requireChild(wrapper, 0, "visitDefault"), index);
    if (index == wrapper.breakCount())
        addWidth(*line, line->R + right);
}

void LinebreakVisitor::visitEmbellishedOperator(CommonWrapper &wrapper, std::size_t const &) {
    auto *mo = dynamic_cast<CommonMo *>(wrapper.coreMO());
    if (!mo)
        throw std::logic_error("embellished wrapper has no CommonMo core");
    auto box = LineBBox::from(wrapper.getOuterBBox(), lineLeading(wrapper));
    box.getIndentData(*mo->node);
    const auto style =
        mo->getBreakStyle(attributeString(mo->node->attributes().lookup(u"linebreakstyle")));
    const auto data = box.indentData;
    if (!data)
        throw std::logic_error("LineBBox did not produce indent data");
    const auto &outer = data->lines[1];
    const auto &inner = data->lines[0];
    const auto shift =
        mo->processIndent("", outer.shift ? utf16::encode(*outer.shift) : "", "",
                          inner.shift ? utf16::encode(*inner.shift) : "", state_.width)
            .second;
    const auto penalty = moPenalty(*mo);
    if (style == "before") {
        pushBreak(wrapper, penalty, shift - box.L, {});
        addWidth(box);
    } else {
        addWidth(box);
        const double width =
            (style == "after" ? 0 : (mo->multChar ? mo->multChar->getBBox().w : box.w)) + shift;
        pushBreak(wrapper, penalty, width, {});
    }
}

void LinebreakVisitor::visitMoNode(CommonWrapper &wrapper, std::size_t const &) {
    auto &mo = requireMo(wrapper);
    auto box = LineBBox::from(mo.getOuterBBox(), lineLeading(mo));
    box.getIndentData(*mo.node);
    const auto style =
        mo.getBreakStyle(attributeString(mo.node->attributes().lookup(u"linebreakstyle")));
    if (!box.indentData)
        throw std::logic_error("LineBBox did not produce indent data");
    const auto &outer = box.indentData->lines[1];
    const auto &inner = box.indentData->lines[0];
    const double shift =
        mo.processIndent("", outer.shift ? utf16::encode(*outer.shift) : "", "",
                         inner.shift ? utf16::encode(*inner.shift) : "", state_.width)
            .second;
    const double penalty = moPenalty(mo);
    if (style == "before") {
        pushBreak(wrapper, penalty, shift - box.L, {});
        addWidth(box);
    } else {
        addWidth(box);
        const double width =
            (style == "after" ? 0 : (mo.multChar ? mo.multChar->getBBox().w : box.w)) + shift;
        pushBreak(wrapper, penalty, width, {});
    }
}

void LinebreakVisitor::visitMspaceNode(CommonWrapper &wrapper, std::size_t const &index) {
    auto *space = dynamic_cast<CommonMspace *>(&wrapper);
    if (!space)
        throw std::logic_error("mspace kind has incompatible wrapper");
    auto line = space->getLineBBox(index);
    if (space->canBreak()) {
        const double penalty = mspacePenalty(*space);
        line->getIndentData(*space->node);
        if (!line->indentData)
            throw std::logic_error("LineBBox did not produce indent data");
        const auto &outer = line->indentData->lines[1];
        const auto &inner = line->indentData->lines[0];
        const double shift =
            space
                ->processIndent("", outer.shift ? utf16::encode(*outer.shift) : "", "",
                                inner.shift ? utf16::encode(*inner.shift) : "", state_.width)
                .second;
        pushBreak(wrapper, penalty, shift - line->w, {});
    }
    addWidth(*line);
}

void LinebreakVisitor::visitMtextNode(CommonWrapper &wrapper, std::size_t const &index) {
    (void)index;
    const auto value = wrapper.getText();
    if (value.find(u' ') == std::u16string::npos) {
        visitDefault(wrapper, index);
        return;
    }
    auto *text = dynamic_cast<CommonMtext *>(&wrapper);
    if (!text)
        throw std::logic_error("mtext kind has incompatible wrapper");
    text->clearBreakPoints();
    const double space = text->textWidth(u" ");
    auto &box = wrapper.getBBox();
    const auto [left, right] = borderLR(wrapper);
    addWidth(box, box.L + left);
    for (std::size_t childIndex = 0; childIndex < wrapper.childNodes.size(); ++childIndex) {
        auto *child = wrapper.childNodes[childIndex];
        if (!child)
            throw std::logic_error("mtext contains a null wrapper child");
        if (child->node->kind() == mml::NodeKind::text) {
            auto *textNode = dynamic_cast<mml::TextNode *>(child->node);
            if (!textNode)
                throw std::logic_error("text kind is not TextNode");
            auto words = splitSpaces(textNode->getText());
            const auto last = words.back();
            words.pop_back();
            for (std::size_t word = 0; word < words.size(); ++word) {
                addWidth(box, text->textWidth(words[word]));
                pushBreak(wrapper, mtextPenalty(), -space,
                          IndexData{std::array<std::size_t, 2>{childIndex, word + 1}});
                addWidth(box, space);
            }
            addWidth(box, text->textWidth(last));
        } else {
            addWidth(child->getBBox());
        }
    }
    addWidth(box, box.R + right);
}

double LinebreakVisitor::mtextPenalty() const {
    return std::floor((state_.width / jsMax(.0001, state_.mathLeft - state_.w)) * 500) +
           std::floor(((state_.width - state_.w) / state_.width) * 2500) + 800 * state_.depth;
}

void LinebreakVisitor::visitMrowNode(CommonWrapper &wrapper, std::size_t const &index) {
    auto line = index < wrapper.lineBBox.size() && wrapper.lineBBox[index]
                    ? wrapper.lineBBox[index]
                    : wrapper.getLineBBox(index);
    const auto start = line->start.value_or(std::array<double, 2>{0, 0});
    const double defaultEnd =
        wrapper.childNodes.empty() ? -1.0 : static_cast<double>(wrapper.childNodes.size() - 1);
    const auto end = line->end.value_or(std::array<double, 2>{defaultEnd, 0});
    const auto [left, right] = borderLR(wrapper);
    addWidth(*line, line->L + left);
    for (std::int64_t childIndex = static_cast<std::int64_t>(start[0]);
         childIndex <= static_cast<std::int64_t>(end[0]); ++childIndex) {
        if (childIndex < 0)
            throw std::logic_error("linebox child index is negative");
        const auto child = static_cast<std::size_t>(childIndex);
        const auto childLine =
            child == static_cast<std::size_t>(start[0]) ? static_cast<std::size_t>(start[1])
            : child == static_cast<std::size_t>(end[0]) ? static_cast<std::size_t>(end[1])
                                                        : 0;
        visitNode(requireChild(wrapper, child, "visitMrowNode"), childLine);
    }
    addWidth(*line, line->R + right);
}

void LinebreakVisitor::visitInferredMrowNode(CommonWrapper &wrapper, std::size_t const &index) {
    --state_.depth;
    visitMrowNode(wrapper, index);
    ++state_.depth;
}

void LinebreakVisitor::visitMfracNode(CommonWrapper &wrapper, std::size_t const &index) {
    auto *fraction = dynamic_cast<CommonMfrac *>(&wrapper);
    if (!fraction)
        throw std::logic_error("mfrac kind has incompatible wrapper");
    if (!truthy(fraction->node->attributes().lookup(u"bevelled")) &&
        fraction->getOuterBBox().w > state_.width) {
        breakToWidth(requireChild(*fraction, 0, "mfrac numerator"), state_.width);
        breakToWidth(requireChild(*fraction, 1, "mfrac denominator"), state_.width);
    }
    visitDefault(wrapper, index);
}

void LinebreakVisitor::visitMsqrtNode(CommonWrapper &wrapper, std::size_t const &index) {
    auto *root = dynamic_cast<CommonMsqrt *>(&wrapper);
    if (!root)
        throw std::logic_error("msqrt kind has incompatible wrapper");
    if (root->getOuterBBox().w > state_.width) {
        breakToWidth(requireChild(*root, root->base(), "msqrt base"),
                     state_.width - root->rootWidth());
        root->getStretchedSurd();
    }
    visitDefault(wrapper, index);
}

void LinebreakVisitor::visitMrootNode(CommonWrapper &wrapper, std::size_t const &index) {
    visitMsqrtNode(wrapper, index);
}

void LinebreakVisitor::visitMsubNode(CommonWrapper &wrapper, std::size_t const &index) {
    visitDefault(wrapper, index);
    auto *sub = dynamic_cast<CommonMsub *>(&wrapper);
    if (!sub)
        throw std::logic_error("msub kind has incompatible wrapper");
    auto *script = sub->scriptChild();
    if (!script)
        throw std::logic_error("msub has no script child");
    const auto box = script->getOuterBBox();
    const auto [left, right] = borderLR(wrapper);
    const double scriptSpace = sub->jax().face().parameters.required("scriptspace");
    addWidth(*sub->getLineBBox(index),
             sub->getOffset()[0] + left + box.rscale * box.w + scriptSpace + right);
}

void LinebreakVisitor::visitMsupNode(CommonWrapper &wrapper, std::size_t const &index) {
    visitDefault(wrapper, index);
    auto *sup = dynamic_cast<CommonMsup *>(&wrapper);
    if (!sup)
        throw std::logic_error("msup kind has incompatible wrapper");
    auto *script = sup->scriptChild();
    if (!script)
        throw std::logic_error("msup has no script child");
    const auto box = script->getOuterBBox();
    const auto [left, right] = borderLR(wrapper);
    const double scriptSpace = sup->jax().face().parameters.required("scriptspace");
    addWidth(*sup->getLineBBox(index),
             sup->getOffset()[0] + left + box.rscale * box.w + scriptSpace + right);
}

void LinebreakVisitor::visitMsubsupNode(CommonWrapper &wrapper, std::size_t const &index) {
    visitDefault(wrapper, index);
    auto *both = dynamic_cast<CommonMsubsup *>(&wrapper);
    if (!both)
        throw std::logic_error("msubsup kind has incompatible wrapper");
    auto *sub = both->subChild();
    auto *sup = both->supChild();
    if (!sub || !sup)
        throw std::logic_error("msubsup has missing script child");
    const auto subBox = sub->getOuterBBox();
    const auto supBox = sup->getOuterBBox();
    const double scriptSpace = both->jax().face().parameters.required("scriptspace");
    const double width =
        std::max(subBox.rscale * subBox.w, both->getAdjustedIc() + supBox.rscale * supBox.w) +
        scriptSpace;
    const auto [left, right] = borderLR(wrapper);
    addWidth(*both->getLineBBox(index), left + width + right);
}

void LinebreakVisitor::visitMmultiscriptsNode(CommonWrapper &wrapper, std::size_t const &index) {
    auto *scripts = dynamic_cast<CommonMmultiscripts *>(&wrapper);
    if (!scripts)
        throw std::logic_error("mmultiscripts kind has incompatible wrapper");
    const auto &data = scripts->scriptsData();
    const double scriptSpace = scripts->jax().face().parameters.required("scriptspace");
    if (data.numPrescripts) {
        const double width =
            std::max(data.psup.rscale * data.psup.w, data.psub.rscale * data.psub.w);
        addWidth(*wrapper.getLineBBox(index), width + scriptSpace);
    }
    visitDefault(wrapper, index);
    if (data.numScripts) {
        const double width = std::max(data.sup.rscale * data.sup.w, data.sub.rscale * data.sub.w);
        addWidth(*wrapper.getLineBBox(index), width + scriptSpace);
    }
}

void LinebreakVisitor::visitMfencedNode(CommonWrapper &wrapper, std::size_t const &index) {
    auto *fenced = dynamic_cast<CommonMfenced *>(&wrapper);
    if (!fenced)
        throw std::logic_error("mfenced kind has incompatible wrapper");
    auto line = fenced->getLineBBox(index);
    const auto [left, right] = borderLR(wrapper);
    if (index == 0)
        addWidth(*line, line->L + left);
    if (!fenced->mrow)
        throw std::logic_error("mfenced has no generated row wrapper");
    visitNode(*fenced->mrow, index);
    if (index == wrapper.breakCount())
        addWidth(*line, line->R + right);
}

void LinebreakVisitor::visitMactionNode(CommonWrapper &wrapper, std::size_t const &index) {
    auto *action = dynamic_cast<CommonMaction *>(&wrapper);
    if (!action)
        throw std::logic_error("maction kind has incompatible wrapper");
    auto line = action->getLineBBox(index);
    const auto [left, right] = borderLR(wrapper);
    if (index == 0)
        addWidth(*line, line->L + left);
    auto *selected = action->selected();
    if (!selected)
        throw std::logic_error("maction has no selected wrapper");
    visitNode(*selected, index);
    if (index == action->breakCount())
        addWidth(*line, line->R + right);
}

CommonWrapper *LinebreakVisitor::getPrevious(CommonMo &mo) const {
    auto *child = mo.node;
    auto *parent = child->parent();
    auto index = parent ? parent->childIndex(*child) : std::nullopt;
    while (parent && (parent->notParent() || parent->isKind(mml::NodeKind::mrow)) && index &&
           *index == 0) {
        child = parent;
        parent = child->parent();
        index = parent ? parent->childIndex(*child) : std::nullopt;
    }
    if (!parent || !index || *index == 0)
        return nullptr;
    const auto children = parent->children();
    auto *previous = children[*index - 1];
    if (!previous || !previous->isEmbellished())
        return nullptr;
    auto *core = previous->coreMO();
    return core ? mo.jax().wrapperFor(*core) : nullptr;
}

double LinebreakVisitor::moPenalty(CommonMo &mo) {
    auto &attributes = mo.node->attributes();
    const auto *linebreak = attributes.lookup(u"linebreak");
    const auto *fence = attributes.lookup(u"fence");
    const auto *form = attributes.lookup(u"form");
    double penalty = std::floor((state_.width / jsMax(.0001, state_.mathLeft - state_.w)) * 500) +
                     std::floor(((state_.width - state_.w) / state_.width) * 2500);
    const auto texClass = mo.node->texClass();
    const bool open =
        (truthy(fence) && attributeString(form) == "prefix") || texClass == mml::TexClass::open;
    const bool close =
        (truthy(fence) && attributeString(form) == "postfix") || texClass == mml::TexClass::close;
    if (open) {
        const auto previous = mo.node->prevClass();
        if (previous == mml::TexClass::bin || previous == mml::TexClass::rel ||
            previous == mml::TexClass::op) {
            penalty += 5000;
        } else {
            auto *prior = getPrevious(mo);
            if (prior &&
                (attributeString(prior->node->attributes().lookup(u"form")) != "postfix" ||
                 attributeString(prior->node->attributes().lookup(u"linebreak")) == "nobreak"))
                penalty += 5000;
            else
                penalty -= 500;
        }
        auto *parent = mo.node->Parent();
        if (parent && parent->isKind(mml::NodeKind::mmultiscripts)) {
            auto *first = parent;
            while (!first->isToken()) {
                const auto children = first->children();
                if (children.empty() || !children.front())
                    throw std::logic_error("mmultiscripts first-token path is empty");
                first = children.front();
            }
            if (first == mo.node) {
                const auto children = parent->children();
                if (std::ranges::any_of(children, [](mml::MmlNode *child) {
                        return child && child->kind() == mml::NodeKind::mprescripts;
                    }))
                    penalty = NOBREAK;
            }
        }
        ++state_.depth;
    }
    if (close) {
        bool scriptClose = false;
        auto *parent = mo.node->Parent();
        const auto scriptChildren = parent ? parent->children() : std::span<mml::MmlNode *>{};
        const bool rightPrescripts = parent && parent->isKind(mml::NodeKind::mmultiscripts) &&
                                     scriptChildren.size() > 1 && scriptChildren[1] &&
                                     scriptChildren[1]->kind() == mml::NodeKind::mprescripts;
        if (parent && parent->isKind(mml::NodeKind::msubsup) && !rightPrescripts) {
            const auto siblings = parent->children();
            if (!siblings.empty() && siblings[0]) {
                auto *last = siblings[0];
                while (!last->isToken()) {
                    const auto children = last->children();
                    if (children.empty() || !children.back())
                        throw std::logic_error("script last-token path is empty");
                    last = children.back();
                }
                scriptClose = last == mo.node;
            }
        }
        if (scriptClose)
            penalty = NOBREAK;
        else
            penalty += 500;
        --state_.depth;
    }
    if (texClass == mml::TexClass::bin)
        penalty -= 250;
    else if (texClass == mml::TexClass::rel)
        penalty -= 500;
    penalty += 800 * state_.depth;
    const auto style = attributeString(linebreak, "auto");
    if (style == "newline")
        return 0;
    if (style == "nobreak")
        return NOBREAK;
    if (style == "goodbreak")
        return penalty - 200 * state_.depth;
    if (style == "badbreak")
        return penalty + 200 * state_.depth;
    return penalty;
}

double LinebreakVisitor::mspacePenalty(CommonMspace &space) {
    const auto *linebreak = space.node->attributes().lookup(u"linebreak");
    const double width = space.getBBox().w;
    double penalty = std::floor((state_.width / jsMax(.0001, state_.mathLeft - state_.w)) * 500) +
                     std::floor(((state_.width - state_.w) / state_.width) * 2500);
    if (!space.canBreak() || width < 0)
        penalty = NOBREAK;
    else if (width >= 1)
        penalty -= 100 * (width + 4);
    penalty += 800 * state_.depth;
    const auto style = attributeString(linebreak, "auto");
    if (style == "newline")
        return 0;
    if (style == "nobreak")
        return NOBREAK;
    if (style == "goodbreak")
        penalty -= 200 * state_.depth;
    if (style == "badbreak")
        penalty += 200 * state_.depth;
    return penalty;
}
} // namespace folia::math::output::common
