// Native port of mhchemparser 4.2.1 src/mhchemParser.ts: parser engine.
// Copyright (c) 2015-2023 Martin Hensel. Apache-2.0.
module folia.math.dependencies.mhchem_parser;
import folia.math.util.regexp_host;
import std;
namespace folia::math::mhchem {
namespace {
regexp_host::RegExp& expressionFor(std::u16string_view pattern, std::u16string_view flags = {}) {
    // Source pattern literals retain their compiled RegExp objects. Retain the
    // equivalent native expressions per thread; no parser state lives here.
    thread_local std::map<std::pair<std::u16string, std::u16string>, std::unique_ptr<regexp_host::RegExp>> expressions;
    const auto key = std::pair{std::u16string(pattern), std::u16string(flags)};
    auto& expression = expressions[key];
    if (!expression) expression = std::make_unique<regexp_host::RegExp>(key.first, key.second);
    return *expression;
}
}
const std::vector<Machine>& machines() {
    struct ActionData { std::u16string_view type; int optionType; std::u16string_view text; double number; };
    struct RowData { std::u16string_view pattern; std::size_t begin, size; std::u16string_view nextState; bool revisit, toContinue; };
    struct StateData { std::u16string_view name; std::size_t begin, size; };
    struct MachineData { std::u16string_view name; std::size_t begin, size; };
#include "transitions_data.inc"
    static const auto value = [] {
        std::vector<Machine> result;
        for (const auto& data : machineData) {
            Machine machine; machine.name = data.name;
            for (std::size_t s = data.begin; s < data.begin + data.size; ++s) {
                const auto& stateEntry = stateData[s];
                State state; state.name = stateEntry.name;
                for (std::size_t r = stateEntry.begin; r < stateEntry.begin + stateEntry.size; ++r) {
                    const auto& row = rowData[r];
                    Transition transition;
                    transition.pattern = row.pattern; transition.nextState = row.nextState;
                    transition.revisit = row.revisit; transition.toContinue = row.toContinue;
                    for (std::size_t a = row.begin; a < row.begin + row.size; ++a) {
                        const auto& entry = actionData[a];
                        Action action; action.type = entry.type;
                        if (entry.optionType == 1) action.option = std::u16string(entry.text);
                        else if (entry.optionType == 2) action.option = entry.number;
                        else if (entry.optionType == 3) action.option = static_cast<bool>(entry.number);
                        transition.actions.push_back(std::move(action));
                    }
                    state.transitions.push_back(std::move(transition));
                }
                machine.states.push_back(std::move(state));
            }
            result.push_back(std::move(machine));
        }
        return result;
    }();
    return value;
}
const std::vector<RegexPattern>& regexPatterns() {
#include "patterns_data.inc"
    return value;
}
std::optional<Match> regexMatch(std::u16string_view input, std::u16string_view pattern,
                               bool captureArray) {
    const auto found = expressionFor(pattern).exec(std::u16string(input));
    if (!found) return {};
    Match result;
    const auto whole = found->at(0).value_or(u"");
    result.remainder = std::u16string(input.substr(whole.size()));
    for (std::size_t i = 1; i < found->captures.size(); ++i) result.captures.push_back(found->at(i));
    result.array = captureArray || found->captures.size() > 2;
    result.text = found->captures.size() == 2 && found->at(1) && !found->at(1)->empty()
        ? *found->at(1) : whole;
    return result;
}
std::u16string replaceRegex(std::u16string input, std::u16string_view pattern,
                           std::u16string_view replacement, bool global) {
    auto& expression = expressionFor(pattern, global ? u"g" : u"");
    expression.last_index = 0;
    std::u16string result;
    std::size_t offset = 0;
    while (const auto found = expression.exec(input)) {
        const auto whole = found->at(0).value_or(u"");
        const auto index = found->index;
        result += input.substr(offset, index - offset);
        for (std::size_t i = 0; i < replacement.size(); ++i) {
            if (replacement[i] != u'$' || i + 1 == replacement.size()) {
                result += replacement[i];
                continue;
            }
            const auto next = replacement[i + 1];
            if (next == u'$') { result += u'$'; ++i; }
            else if (next == u'&') { result += whole; ++i; }
            else if (next == u'`') { result += input.substr(0, index); ++i; }
            else if (next == u'\'') { result += input.substr(index + whole.size()); ++i; }
            else if (next >= u'1' && next <= u'9' && next - u'0' < found->captures.size()) {
                std::size_t group = next - u'0';
                ++i;
                if (i + 1 < replacement.size() && replacement[i + 1] >= u'0' && replacement[i + 1] <= u'9') {
                    const auto combined = group * 10 + replacement[i + 1] - u'0';
                    if (combined < found->captures.size()) { group = combined; ++i; }
                }
                result += found->at(group).value_or(u"");
            } else result += u'$';
        }
        offset = index + whole.size();
        if (!global) break;
        if (whole.empty()) {
            if (offset == input.size()) break;
            expression.last_index = offset + 1;
        }
    }
    result += input.substr(offset);
    return result;
}
Output parse(std::u16string input, std::u16string_view machineName) {
    if (input.empty()) return {};
    const auto machine = std::ranges::find(machines(), machineName, &Machine::name);
    if (machine == machines().end()) throw Error(u"MhchemBugA", u"Unknown mhchem state machine");
    std::u16string state = u"0";
    Buffer buffer;
    input = replaceRegex(std::move(input), u"\\n", u" ", true);
    input = replaceRegex(std::move(input), u"[\u2212\u2013\u2014\u2010]", u"-", true);
    input = replaceRegex(std::move(input), u"[\u2026]", u"...", true);
    std::optional<std::u16string> lastInput;
    int watchdog = 10;
    Output output;
    while (true) {
        if (!lastInput || *lastInput != input) { watchdog = 10; lastInput = input; }
        else --watchdog;
        auto current = std::ranges::find(machine->states, state, &State::name);
        if (current == machine->states.end()) current = std::ranges::find(machine->states, u"*", &State::name);
        if (current == machine->states.end()) throw Error(u"MhchemBugU", u"mhchem bug U. Please report.");
        for (const auto& transition : current->transitions) {
            const auto found = match(transition.pattern, input);
            if (!found) continue;
            for (const auto& task : transition.actions) {
                auto added = action(machineName, task.type, buffer, *found, task.option);
                output.insert(output.end(), std::make_move_iterator(added.begin()), std::make_move_iterator(added.end()));
            }
            if (!transition.nextState.empty()) state = transition.nextState;
            if (input.empty()) return output;
            if (!transition.revisit) input = found->remainder;
            if (!transition.toContinue) break;
        }
        if (watchdog <= 0) throw Error(u"MhchemBugU", u"mhchem bug U. Please report.");
    }
}
std::u16string toTex(std::u16string input, std::u16string_view machine) {
    auto output = parse(std::move(input), machine);
    return texify(output, machine != u"tex");
}
}
