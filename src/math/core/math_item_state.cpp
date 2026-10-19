// MathJax 4.1.3 core/MathItem.ts constructor and state methods; Apache-2.0.
module folia.math.core.math_item;
import std;

namespace folia::math::core {
AbstractMathItem::AbstractMathItem(std::u16string source, MathInputJaxService* jax,
                                  MathDisplay mode) {
    math = std::move(source);
    inputJax = jax;
    display = mode;
}

bool MathItem::isDisplay() const noexcept {
    return display == MathDisplay::display_math;
}

void MathItem::setDisplay(bool value) noexcept {
    display = value ? MathDisplay::display_math : MathDisplay::inline_math;
}

bool AbstractMathItem::isEscaped() const noexcept {
    return display == MathDisplay::escaped;
}

void AbstractMathItem::setMetrics(double em, double ex, double cwidth, double scale) {
    metrics = MathMetrics{em, ex, cwidth, scale};
}

MathState AbstractMathItem::state() const noexcept {
    return state_;
}

MathState AbstractMathItem::state(MathState target, bool restore) {
    if (target < STATE.INSERTED && state_ >= STATE.INSERTED) {
        this->removeFromDocument(restore);
    }
    // Read the current state after the virtual removal, as Source does.
    if (target < STATE.TYPESET && state_ >= STATE.TYPESET) {
        outputData = {};
    }
    if (target < STATE.COMPILED && state_ >= STATE.COMPILED) {
        inputData = {};
    }
    state_ = target;
    return state_;
}

void AbstractMathItem::reset(bool restore) {
    this->state(STATE.UNPROCESSED, restore);
}
}
