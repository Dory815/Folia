// MathJax 4.1.3 BegingroupStack.ts: original scope/checkpoint algorithms.
module folia.math.input.tex.begingroup.stack;
import std;

namespace folia::math::tex {
std::int32_t BegingroupStack::index() const noexcept { return state_.current_priority; }
std::int32_t BegingroupStack::top() const noexcept { return state_.top_priority; }
std::int32_t BegingroupStack::base() const noexcept { return state_.base_priority; }
DefinitionLayer& BegingroupStack::currentLayer() noexcept {
    return state_.groups.empty() ? state_.global : state_.groups.back();
}
DefinitionLayer& BegingroupStack::globalTarget() {
    if (base() == newcommandPriority) return state_.global;
    auto found = std::ranges::find(state_.groups, base(), &DefinitionLayer::priority);
    if (found == state_.groups.end()) throw std::logic_error("Missing definition base layer");
    return *found;
}
DefinitionLayer& BegingroupStack::checkGlobal(std::span<DefinitionKey const> keys) {
    auto& target = globalTarget();
    for (auto const& key : keys) {
        for (auto layer = state_.groups.rbegin(); layer != state_.groups.rend(); ++layer) {
            layer->erase(key);
            if (&*layer == &target) break;
        }
        if (&target == &state_.global) target.erase(key);
    }
    return target;
}
void BegingroupStack::push() {
    if (index() == std::numeric_limits<std::int32_t>::min())
        throw std::overflow_error("Definition scope depth exceeds native range");
    const auto priority = index() - 1;
    state_.groups.emplace_back(priority);
    state_.current_priority = priority;
}
void BegingroupStack::pop() {
    if (index() == base())
        throw TexError("MissingBegingroup", u"Missing \\begingroup or extra \\endgroup");
    if (state_.groups.empty()) throw std::logic_error("Missing active definition layer");
    state_.groups.pop_back();
    ++state_.current_priority;
}
void BegingroupStack::finish() noexcept { state_.top_priority = index(); }
void BegingroupStack::remove() { while (index() < top()) pop(); }
void BegingroupStack::reset() {
    state_.top_priority = base();
    remove();
}
void BegingroupStack::sandbox() {
    state_.base_priority = newcommandPriority;
    reset();
    push();
    state_.base_priority = newcommandPriority - 1;
}
}
