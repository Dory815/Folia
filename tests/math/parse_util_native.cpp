#include "../support/folia_test.hpp"
import folia.math.input.tex.parse_util;

using namespace boost::ut;
using namespace folia::math::tex;
using namespace folia::math::tex::parse_util;

namespace {
template<class Operation>
void expectTexError(
    Operation&& operation,
    std::string_view expected_id,
    std::u16string_view expected_message) {
    bool caught = false;
    try {
        operation();
    } catch (TexError const& error) {
        caught = true;
        expect(error.id() == expected_id);
        expect(error.message() == expected_message);
    } catch (...) {
        expect(false) << "wrong exception type";
    }
    expect(caught) << "expected a typed TexError";
}
}

suite parse_util_tests = [] {
    "add_args_control_word_boundaries"_test = [] {
        expect(addArgs(u"left", u"right", 20) == u"leftright");
        expect(addArgs(u"\\alpha", u"beta", 20) == u"\\alpha beta");
        expect(addArgs(u"prefix\\alpha", u"beta", 20) == u"prefix\\alpha beta");
        expect(addArgs(u"\\\\alpha", u"beta", 20) == u"\\\\alphabeta");
        expect(addArgs(u"\\\\\\alpha", u"beta", 20) == u"\\\\\\alpha beta");
        expect(addArgs(u"\\#", u"beta", 20) == u"\\#beta");
        expect(addArgs(u"", u"beta", 20) == u"beta");
    };

    "add_args_buffer_limit_includes_separator"_test = [] {
        expect(addArgs(u"\\alpha", u"b", 8) == u"\\alpha b");
        expectTexError(
            [] { static_cast<void>(addArgs(u"\\alpha", u"b", 7)); },
            "MaxBufferSize",
            u"MathJax internal buffer size exceeded; is there a recursive macro call?");
        expect(addArgs(u"123", u"45", 5) == u"12345");
        expectTexError(
            [] { static_cast<void>(addArgs(u"123", u"45", 4)); },
            "MaxBufferSize",
            u"MathJax internal buffer size exceeded; is there a recursive macro call?");
    };

    "substitute_args_preserves_escapes_and_boundaries"_test = [] {
        const std::array<std::u16string, 2> arguments{u"one", u"😀"};
        const std::array<std::u16string, 1> one_argument{u"beta"};
        const std::array<std::u16string, 2> extra_arguments{u"one", u"unused"};
        const std::array<std::u16string, 9> nine_arguments{
            u"1", u"2", u"3", u"4", u"5", u"6", u"7", u"8", u"9"};
        expect(substituteArgs(arguments, u"#1/#2", 20) == u"one/😀");
        expect(substituteArgs(arguments, u"\\#1", 20) == u"\\#1");
        expect(substituteArgs(one_argument, u"\\{#1}", 20) == u"\\{beta}");
        expect(substituteArgs(one_argument, u"\\\\#1", 20) == u"\\\\beta");
        expect(substituteArgs(arguments, u"##1", 20) == u"#1");
        expect(substituteArgs(one_argument, u"\\alpha#1", 20) == u"\\alpha beta");
        expect(substituteArgs(extra_arguments, u"#1", 20) == u"one");
        expect(substituteArgs(nine_arguments, u"#9", 20) == u"9");
        expect(substituteArgs(std::span<std::u16string const>{}, u"tail\\", 20) == u"tail\\");
        expect(substituteArgs(arguments, u"plain text", 20) == u"plain text");
    };

    "substitute_args_parameter_errors_and_order"_test = [] {
        const std::array<std::u16string, 1> arguments{u"one"};
        for (const auto replacement : {u"#", u"#0", u"#x", u"#2", u"#9"}) {
            expectTexError(
                [replacement, &arguments] {
                    static_cast<void>(substituteArgs(arguments, replacement, 0));
                },
                "IllegalMacroParam",
                u"Illegal macro parameter reference");
        }
        expectTexError(
            [] { static_cast<void>(substituteArgs(std::span<std::u16string const>{}, u"#1", 100)); },
            "IllegalMacroParam",
            u"Illegal macro parameter reference");
        expectTexError(
            [&arguments] { static_cast<void>(substituteArgs(arguments, u"#1", 2)); },
            "MaxBufferSize",
            u"MathJax internal buffer size exceeded; is there a recursive macro call?");
    };

    "substitute_args_limit_counts_utf16_code_units"_test = [] {
        const std::array<std::u16string, 1> emoji{u"😀"};
        expect(addArgs(u"", u"😀", 2) == u"😀");
        expectTexError(
            [] { static_cast<void>(addArgs(u"", u"😀", 1)); },
            "MaxBufferSize",
            u"MathJax internal buffer size exceeded; is there a recursive macro call?");
        expectTexError(
            [&emoji] { static_cast<void>(substituteArgs(emoji, u"#1", 1)); },
            "MaxBufferSize",
            u"MathJax internal buffer size exceeded; is there a recursive macro call?");
    };

    "check_max_macros_increments_before_error"_test = [] {
        ExpansionBudget budget{2, 100, 0};
        checkMaxMacros(budget);
        checkMaxMacros(budget, false);
        expect(budget.macroCount == 2);
        expectTexError(
            [&budget] { checkMaxMacros(budget, true); },
            "MaxMacroSub1",
            u"MathJax maximum macro substitution count exceeded; is here a recursive macro call?");
        expect(budget.macroCount == 3);

        ExpansionBudget environment{0, 100, 0};
        expectTexError(
            [&environment] { checkMaxMacros(environment, false); },
            "MaxMacroSub2",
            u"MathJax maximum substitution count exceeded; is there a recursive latex environment?");
        expect(environment.macroCount == 1);
    };
};
