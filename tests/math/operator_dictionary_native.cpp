#include "support/folia_test.hpp"
import folia.math.core.mml_tree.operator_dictionary;
import folia.math.core.mml_tree.mo_patterns;

using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::mml;
using namespace folia::math::mml::operator_dict;

suite typed_operator_dictionary = [] {
    "operator_tables_preserve_source_order_and_records"_test = [] {
        const auto named = MO();
        const auto ranges = RANGES();
        const auto spacing = MMLSPACING();
        const auto operators = OPTABLE();

        expect(named.size() == 27_u);
        expect(named.front().first == u"REL");
        expect(named.front().second.left == 5.0);
        expect(named.front().second.right == 5.0);
        expect(named.front().second.texClass == TexClass::rel);
        expect(named[1].first == u"WIDEREL");
        expect(named[1].second.properties.size() == 2_u);
        expect(named[1].second.properties[0].first == u"accent");
        const auto* accent = std::get_if<bool>(&named[1].second.properties[0].second);
        expect(accent != nullptr);
        if (accent) expect(*accent);
        expect(named[1].second.properties[1].first == u"stretchy");
        const auto* stretchy = std::get_if<bool>(&named[1].second.properties[1].second);
        expect(stretchy != nullptr);
        if (stretchy) expect(*stretchy);

        expect(ranges.size() == 50_u);
        expect(ranges.front().first == U' ');
        expect(ranges.front().last == U'\x7f');
        expect(ranges.front().texClass == TexClass::rel);
        expect(ranges.front().kind == u"mo");
        expect(!ranges.front().variant.has_value());
        expect(getRange(u"中").variant == u"normal");
        expect(getRange(u"あ").variant == u"normal");
        expect(getRange(u"𠀀").variant == u"normal");
        expect(!getRange(u"a").variant.has_value());
        expect(spacing.size() == 8_u);
        expect(spacing[0] == std::array<double, 2>{0.0, 0.0});
        expect(spacing[1] == std::array<double, 2>{1.0, 2.0});
        expect(spacing[7] == std::array<double, 2>{1.0, 1.0});

        expect(operators.size() == 1252_u);
        expect(operators.front().form == u"prefix");
        expect(operators.front().text == u"!");
        expect(operators.front().definition.left == 0.0);
        expect(operators.front().definition.texClass == TexClass::ord);
        const auto* prefixPlus = findOperatorDefinition(u"prefix", u"+");
        const auto* infixPlus = findOperatorDefinition(u"infix", u"+");
        expect(prefixPlus != nullptr);
        expect(infixPlus != nullptr);
        if (prefixPlus) expect(prefixPlus->left == 0.0);
        if (infixPlus) expect(infixPlus->left == 4.0);
        expect(findOperatorDefinition(u"unknown", u"+") == nullptr);
    };

    "range_lookup_preserves_operator_precedence_and_unicode"_test = [] {
        const auto operatorRange = getRange(u"!");
        expect(operatorRange.texClass == TexClass::ord);
        expect(operatorRange.kind == u"mo");
        const auto* postfixBang = findOperatorDefinition(u"postfix", u"!");
        expect(postfixBang != nullptr);
        if (postfixBang) expect(postfixBang->texClass == TexClass::close);

        const auto latin = getRange(u"a");
        expect(latin.first == 0x20);
        expect(latin.last == 0x7f);
        expect(latin.texClass == TexClass::rel);
        const auto greek = getRange(u"\u03b1");
        expect(greek.first == 0x370);
        expect(greek.texClass == TexClass::ord);
        expect(greek.kind == u"mi");

        const std::u16string emoji{char16_t{0xd83d}, char16_t{0xde00}};
        const auto unmatched = getRange(emoji);
        expect(unmatched.first == 0x1df00);
        expect(unmatched.last == 0x1f7ff);
        expect(unmatched.texClass == TexClass::ord);
        expect(unmatched.kind == u"mo");
        // Source has an explicit empty infix operator, preceding range fallback.
        expect(getRange(u"").texClass == TexClass::ord);
    };

    "opdef_defaults_are_typed_values"_test = [] {
        const auto defaulted = OPDEF(1.25, 2.5);
        expect(defaulted.left == 1.25);
        expect(defaulted.right == 2.5);
        expect(defaulted.texClass == TexClass::bin);
        expect(defaulted.properties.empty());

        const auto explicitDef = OPDEF(0.0, 3.0, TexClass::punct,
            {{u"linebreakstyle", AttributeValue{std::u16string{u"after"}}}});
        expect(explicitDef.texClass == TexClass::punct);
        expect(explicitDef.properties.size() == 1_u);
        expect(explicitDef.properties[0].first == u"linebreakstyle");
        const auto* value = std::get_if<std::u16string>(&explicitDef.properties[0].second);
        expect(value != nullptr);
        if (value) expect(*value == u"after");
    };
};

suite typed_mo_pattern_data = [] {
    "pattern_specs_preserve_source_and_length_bounds"_test = [] {
        const auto& pseudo = moPatternSpec(MoPattern::pseudoScripts);
        expect(pseudo.source.starts_with(u"^[\"'*`"));
        expect(pseudo.flags.empty());
        expect(pseudo.minimum == 1_u);
        expect(!pseudo.maximum.has_value());

        const auto& names = moPatternSpec(MoPattern::opPattern);
        expect(names.source == u"^[a-zA-Z]{2,}$");
        expect(names.minimum == 2_u);
        expect(!names.maximum.has_value());

        const auto& accent = moPatternSpec(MoPattern::mathaccents);
        expect(accent.minimum == 1_u);
        expect(accent.maximum == std::size_t{1});
        const auto& width = moPatternSpec(MoPattern::mathaccentsWithWidth);
        expect(width.maximum == std::size_t{1});
    };

    "finite_mo_patterns_match_source_utf16_sets"_test = [] {
        expect(matchMoPattern(MoPattern::pseudoScripts, u"'\u00b9"));
        expect(!matchMoPattern(MoPattern::pseudoScripts, u"x"));
        expect(matchMoPattern(MoPattern::primes, u"\u2019\u201f"));
        expect(!matchMoPattern(MoPattern::primes, u"a"));
        expect(matchMoPattern(MoPattern::opPattern, u"sin"));
        expect(!matchMoPattern(MoPattern::opPattern, u"x"));
        expect(!matchMoPattern(MoPattern::opPattern, u"\u03b1\u03b2"));
        expect(matchMoPattern(MoPattern::mathaccents, u"\u0301"));
        expect(!matchMoPattern(MoPattern::mathaccents, u"\u0301\u0300"));
        expect(matchMoPattern(MoPattern::mathaccentsWithWidth, u"\u2190"));
        expect(!matchMoPattern(MoPattern::mathaccentsWithWidth, u"\u2195"));
    };

    "finite_mo_patterns_match_source_terminal_line_terminators"_test = [] {
        constexpr std::array<std::u16string_view, 5> suffixes{
            u"\n", u"\r", u"\u2028", u"\u2029", u"\r\n"};
        constexpr std::array<std::u16string_view, 5> examples{
            u"'\u00b9", u"\u2019", u"sin", u"\u0301", u"\u2190"};
        constexpr std::array<MoPattern, 5> kinds{
            MoPattern::pseudoScripts, MoPattern::primes, MoPattern::opPattern,
            MoPattern::mathaccents, MoPattern::mathaccentsWithWidth};

        for (std::size_t index = 0; index < kinds.size(); ++index) {
            expect(matchMoPattern(kinds[index], examples[index]));
            for (const auto suffix : suffixes) {
                std::u16string text{examples[index]};
                text.append(suffix);
                expect(!matchMoPattern(kinds[index], text));
            }
        }
    };
};
