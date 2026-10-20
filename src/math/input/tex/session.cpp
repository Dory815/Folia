/*************************************************************
 * MathJax 4.1.3 session data: Tags.ts and Token.ts.
 * Apache-2.0. Source hashes and responsibility details are in README.md.
 *************************************************************/
module folia.math.input.tex.session;
import std;

namespace folia::math::tex {
void MacroDefinition::validate() const {
    if (std::isnan(arity) || arity < 0 || std::floor(arity) != arity)
        throw std::invalid_argument("Macro parameter count must be a nonnegative integer");
    if (parameter_template && arity > 9)
        throw std::invalid_argument("A def template supports at most nine numbered parameters");
    if ((parameter_template || !parameter_delimiters.empty()) &&
        parameter_delimiters.size() != arity)
        throw std::invalid_argument("Macro arity and delimiter template disagree");
}
void CommandDefinition::validate() const {
    std::visit(
        [](auto const &definition) {
            using T = std::remove_cvref_t<decltype(definition)>;
            if constexpr (std::same_as<T, MacroDefinition>)
                definition.validate();
            else if constexpr (std::same_as<T, NativeMacroAlias>) {
                if (!definition.handler.value)
                    throw std::invalid_argument(
                        "A native macro alias needs its compiled handler ID");
            } else if constexpr (std::same_as<T, CharacterMacroAlias>) {
                if (!definition.parser.value)
                    throw std::invalid_argument(
                        "A character alias needs its compiled token-parser ID");
            }
        },
        body);
}
bool DefinitionLayer::erase(DefinitionKey const &key) {
    switch (key.kind) {
    case DefinitionKind::command:
        return commands.erase(key.name) != 0;
    case DefinitionKind::delimiter:
        return delimiters.erase(key.name) != 0;
    case DefinitionKind::environment:
        return environments.erase(key.name) != 0;
    }
    return false;
}
MacroColumnDefinition const *ColumnMacroDefinitions::find(char16_t specifier) const noexcept {
    auto const it = definitions_.find(specifier);
    return it == definitions_.end() ? nullptr : &it->second;
}
void ColumnMacroDefinitions::set(char16_t specifier, MacroColumnDefinition definition) {
    definitions_.insert_or_assign(specifier, std::move(definition));
}
bool ColumnMacroDefinitions::erase(char16_t specifier) noexcept {
    return definitions_.erase(specifier) != 0;
}
namespace {
template <class Member>
auto findDefinition(TexDefinitionState const &state, Member member, std::u16string_view name) {
    using Map = std::remove_cvref_t<decltype(state.global.*member)>;
    using Value = typename Map::mapped_type;
    const std::u16string key(name);
    for (auto layer = state.groups.rbegin(); layer != state.groups.rend(); ++layer) {
        auto const &map = (*layer).*member;
        if (auto found = map.find(key); found != map.end())
            return static_cast<Value const *>(&found->second);
    }
    auto const &map = state.global.*member;
    auto found = map.find(key);
    return found == map.end() ? static_cast<Value const *>(nullptr) : &found->second;
}
} // namespace
DefinitionLayer &TexSessionState::currentDefinitionLayer() noexcept {
    return definitions_.groups.empty() ? definitions_.global : definitions_.groups.back();
}
CommandDefinition const *TexSessionState::findCommand(std::u16string_view name) const {
    return findDefinition(definitions_, &DefinitionLayer::commands, name);
}
void TexSessionState::defineCommand(std::u16string name, CommandDefinition definition) {
    definition.validate();
    currentDefinitionLayer().commands.insert_or_assign(std::move(name), std::move(definition));
}
bool TexSessionState::eraseCommand(std::u16string_view name) {
    return currentDefinitionLayer().commands.erase(std::u16string(name)) != 0;
}
MacroDefinition const *TexSessionState::findMacro(std::u16string_view name) const {
    auto const *command = findCommand(name);
    return command ? std::get_if<MacroDefinition>(&command->body) : nullptr;
}
void TexSessionState::defineMacro(std::u16string name, MacroDefinition definition) {
    CommandDefinition command{name, std::move(definition)};
    defineCommand(std::move(name), std::move(command));
}
bool TexSessionState::eraseMacro(std::u16string_view name) {
    return eraseCommand(name);
}
void TexSessionState::clearMacros() noexcept {
    definitions_.global.commands.clear();
    for (auto &layer : definitions_.groups)
        layer.commands.clear();
}
DelimiterDefinition const *TexSessionState::findDelimiter(std::u16string_view name) const {
    return findDefinition(definitions_, &DefinitionLayer::delimiters, name);
}
void TexSessionState::defineDelimiter(std::u16string name, DelimiterDefinition definition) {
    currentDefinitionLayer().delimiters.insert_or_assign(std::move(name), std::move(definition));
}
bool TexSessionState::eraseDelimiter(std::u16string_view name) {
    return currentDefinitionLayer().delimiters.erase(std::u16string(name)) != 0;
}
EnvironmentDefinition const *TexSessionState::findEnvironment(std::u16string_view name) const {
    return findDefinition(definitions_, &DefinitionLayer::environments, name);
}
void TexSessionState::defineEnvironment(std::u16string name, EnvironmentDefinition definition) {
    currentDefinitionLayer().environments.insert_or_assign(std::move(name), std::move(definition));
}
bool TexSessionState::eraseEnvironment(std::u16string_view name) {
    return currentDefinitionLayer().environments.erase(std::u16string(name)) != 0;
}
ColorModel &TexSessionState::enableColor() {
    if (!color_)
        color_.emplace();
    return *color_;
}
ColorModel *TexSessionState::colorModel() noexcept {
    return color_ ? &*color_ : nullptr;
}
ColorModel const *TexSessionState::colorModel() const noexcept {
    return color_ ? &*color_ : nullptr;
}
LabelRecord const *TexSessionState::findLabel(std::u16string_view name) const {
    auto const it = all_labels_.find(std::u16string(name));
    return it == all_labels_.end() ? nullptr : &it->second;
}
bool TexSessionState::containsId(std::u16string_view id) const {
    return all_ids_.contains(std::u16string(id));
}
TexEquationState
TexSessionState::beginEquation(bool ref_update,
                               std::optional<std::int64_t> recompile_counter) const {
    if (ref_update && !recompile_counter)
        throw std::invalid_argument(
            "A reference-update equation requires its Source recompile counter");
    TexEquationState result;
    result.counter.equation =
        ref_update && recompile_counter ? *recompile_counter : all_counter_.equation;
    result.ref_update = ref_update;
    return result;
}
EquationFinishResult TexSessionState::finishEquation(TexEquationState const &equation) {
    EquationFinishResult result;
    // Tags.finishEquation reports redo with allCounter before committing this
    // equation. State changes are not rolled back if parsing failed first.
    if (equation.redo)
        result.recompile_counter = all_counter_.equation;
    if (!equation.ref_update)
        all_counter_ = equation.counter;
    for (auto const &[name, label] : equation.labels)
        all_labels_.insert_or_assign(name, label);
    all_ids_.insert(equation.ids.begin(), equation.ids.end());
    return result;
}
void TexSessionState::resetTags(std::int64_t offset) noexcept {
    all_labels_.clear();
    all_ids_.clear();
    all_counter_ = TagCounterState{offset};
}
} // namespace folia::math::tex
