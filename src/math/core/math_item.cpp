// MathJax 4.1.3 core/MathItem.ts lifecycle methods; Apache-2.0.
module folia.math.core.math_item;
import std;

namespace folia::math::core {
void AbstractMathItem::render(MathRenderActionsService& actions,
                              mml::MmlDocument& document) {
    actions.renderMath(*this, document);
}

void AbstractMathItem::rerender(MathRenderActionsService& actions,
                                mml::MmlDocument& document, MathState start) {
    if (this->state() >= start) {
        this->state(static_cast<MathState>(std::to_underlying(start) - 1));
    }
    actions.renderMath(*this, document, start);
}

void AbstractMathItem::convert(MathRenderActionsService& actions,
                               mml::MmlDocument& document, MathState end) {
    actions.renderConvert(*this, document, end);
}

void AbstractMathItem::compile(mml::MmlDocument& document, MathOutputJaxService* output) {
    if (this->state() < STATE.COMPILED) {
        if (!inputJax) {
            throw std::logic_error("MathItem compile requires an input jax");
        }
        auto& compiled = inputJax->compile(*this, document, output);
        if (&compiled.document() != &document) {
            throw std::invalid_argument("MathItem compile result belongs to another MML document");
        }
        root = &compiled;
        this->state(STATE.COMPILED);
    }
}

void AbstractMathItem::typeset(MathOutputJaxService& output,
                              mml::MmlDocument& document) {
    if (this->state() < STATE.TYPESET) {
        typesetRoot = this->isEscaped() ? output.escaped(*this, document)
                                      : output.typeset(*this, document);
        this->state(STATE.TYPESET);
    }
}

void AbstractMathItem::updateDocument() {}

void AbstractMathItem::removeFromDocument(bool) {
    this->clear();
}

void AbstractMathItem::clear() {}
}
