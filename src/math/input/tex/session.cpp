/*************************************************************
 * MathJax 4.1.3 session data: Tags.ts and Token.ts.
 * Apache-2.0. Source hashes and responsibility details are in README.md.
 *************************************************************/
module folia.math.input.tex.session;
import std;

namespace folia::math::tex {
void MacroDefinition::validate() const {
    if (arity > 9) throw std::invalid_argument("TeX macros support at most nine parameters");
    if (parameter_delimiters.size() != arity)
        throw std::invalid_argument("Macro arity and delimiter template disagree");
    if (default_first_argument && arity == 0)
        throw std::invalid_argument("A default argument requires at least one parameter");
}
MacroDefinition const* TexSessionState::findMacro(std::u16string_view name) const {
    auto const it = macros_.find(std::u16string(name));
    return it == macros_.end() ? nullptr : &it->second;
}
void TexSessionState::defineMacro(std::u16string name, MacroDefinition definition) {
    definition.validate();
    macros_.insert_or_assign(std::move(name), std::move(definition));
}
bool TexSessionState::eraseMacro(std::u16string_view name) {
    return macros_.erase(std::u16string(name)) != 0;
}
ColorModel& TexSessionState::enableColor() {
    if (!color_) color_.emplace();
    return *color_;
}
ColorModel* TexSessionState::colorModel() noexcept {
    return color_ ? &*color_ : nullptr;
}
ColorModel const* TexSessionState::colorModel() const noexcept {
    return color_ ? &*color_ : nullptr;
}
LabelRecord const* TexSessionState::findLabel(std::u16string_view name) const {
    auto const it = all_labels_.find(std::u16string(name));
    return it == all_labels_.end() ? nullptr : &it->second;
}
bool TexSessionState::containsId(std::u16string_view id) const {
    return all_ids_.contains(std::u16string(id));
}
TexEquationState TexSessionState::beginEquation(
    bool ref_update, std::optional<std::int64_t> recompile_counter) const {
    if (ref_update && !recompile_counter)
        throw std::invalid_argument("A reference-update equation requires its Source recompile counter");
    TexEquationState result;
    result.counter.equation = ref_update && recompile_counter
        ? *recompile_counter : all_counter_.equation;
    result.ref_update = ref_update;
    return result;
}
EquationFinishResult TexSessionState::finishEquation(TexEquationState const& equation) {
    EquationFinishResult result;
    // Tags.finishEquation reports redo with allCounter before committing this
    // equation. State changes are not rolled back if parsing failed first.
    if (equation.redo) result.recompile_counter = all_counter_.equation;
    if (!equation.ref_update) all_counter_ = equation.counter;
    for (auto const& [name, label] : equation.labels)
        all_labels_.insert_or_assign(name, label);
    all_ids_.insert(equation.ids.begin(), equation.ids.end());
    return result;
}
void TexSessionState::resetTags(std::int64_t offset) noexcept {
    all_labels_.clear();
    all_ids_.clear();
    all_counter_ = TagCounterState{offset};
}
}
