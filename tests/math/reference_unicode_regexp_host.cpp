#include "support/folia_test.hpp"
import folia.math.util.regexp_host;
import folia.math.util.option_types;
namespace {
using namespace boost::ut;
namespace host=folia::math::regexp_host;
using folia::math::options::Exception;

suite unicode_regexp_host=[] {
"unicode_regexp_u_scans_code_points_and_keeps_utf16_indices"_test=[] {
    host::RegExp expression(u".",u"gu");
    const std::u16string input=u"A\U0001F600B";
    auto a=expression.exec(input);auto face=expression.exec(input);const auto afterFace=expression.lastIndex;auto b=expression.exec(input);
    expect(a&&a->index==0&&a->at(0)==std::optional<std::u16string>{u"A"});
    expect(face&&face->index==1&&face->at(0)==std::optional<std::u16string>{u"\U0001F600"});
    expect(afterFace==3);
    expect(b&&b->index==3&&b->at(0)==std::optional<std::u16string>{u"B"});
    expect(expression.lastIndex==4);
};

"legacy_regexp_keeps_surrogate_unit_iteration"_test=[] {
    host::RegExp expression(u".",u"g");
    const std::u16string input=u"\U0001F600";
    auto high=expression.exec(input);auto low=expression.exec(input);
    expect(high&&high->index==0&&high->at(0)==std::optional<std::u16string>{std::u16string(1,char16_t(0xD83D))});
    expect(low&&low->index==1&&low->at(0)==std::optional<std::u16string>{std::u16string(1,char16_t(0xDE00))});
};

"unicode_stateful_start_inside_pair_rewinds_to_scalar_start"_test=[] {
    host::RegExp expression(u".",u"uy");expression.lastIndex=1;
    const auto match=expression.exec(u"\U0001F600x");
    expect(match&&match->index==0&&match->at(0)==std::optional<std::u16string>{u"\U0001F600"});
    expect(expression.lastIndex==2);
};

"unicode_escaped_astral_ranges_and_quantifiers_use_code_points"_test=[] {
    host::RegExp range(u"[\\u{1F600}-\\u{1F64F}]+",u"u");
    const auto match=range.exec(u"a\U0001F600\U0001F64Fx");
    expect(match&&match->index==1&&match->at(0)==std::optional<std::u16string>{u"\U0001F600\U0001F64F"});
    host::RegExp quantifier(u".{2}",u"u");
    const auto pair=quantifier.exec(u"\U0001F600x");
    expect(pair&&pair->at(0)==std::optional<std::u16string>{u"\U0001F600x"});
};

"unicode_casefolding_and_word_classes_use_unicode_mode"_test=[] {
    host::RegExp expression(u"[a-z]+",u"iu");
    const auto match=expression.exec(u"\u212A\u017Fx");
    expect(match&&match->at(0)==std::optional<std::u16string>{u"\u212A\u017Fx"});
    host::RegExp word(u"\\w+",u"iu");
    const auto wordMatch=word.exec(u"\u212A\u017Fx");
    expect(wordMatch&&wordMatch->at(0)==std::optional<std::u16string>{u"\u212A\u017Fx"});
};

"unicode_mode_preserves_isolated_surrogates_as_code_points"_test=[] {
    host::RegExp expression(u".",u"gu");
    const std::u16string input{char16_t(0xD800),u'A',char16_t(0xDC00)};
    const auto first=expression.exec(input);const auto second=expression.exec(input);const auto third=expression.exec(input);
    expect(first&&first->index==0&&first->at(0)==std::optional<std::u16string>{std::u16string(1,char16_t(0xD800))});
    expect(second&&second->index==1&&second->at(0)==std::optional<std::u16string>{u"A"});
    expect(third&&third->index==2&&third->at(0)==std::optional<std::u16string>{std::u16string(1,char16_t(0xDC00))});
};

"unicode_named_groups_preserve_capture_names_order_and_values"_test=[] {
    host::RegExp expression(u"(?<face>\\u{1F600})(?<next>.)",u"u");
    const auto match=expression.exec(u"\U0001F600x");
    expect(match.has_value());
    expect(match->groups.has_value());
    expect(match->groups->size()==2);
    expect((*match->groups)[0].first==u"face");
    expect((*match->groups)[0].second==std::optional<std::u16string>{u"\U0001F600"});
    expect((*match->groups)[1].first==u"next");
    expect((*match->groups)[1].second==std::optional<std::u16string>{u"x"});
};

"unicode_invalid_identity_escape_is_syntax_error"_test=[] {
    bool syntax=false;
    try{host::RegExp expression(u"\\q",u"u");}
    catch(const Exception& error){syntax=error.name=="SyntaxError";}
    expect(syntax);
};

"unicode_codepoint_escape_metacharacters_stay_literal"_test=[] {
    host::RegExp dot(u"\\u{2e}",u"u"),open(u"\\u{28}",u"u");
    expect(dot.exec(u".").has_value()&&!dot.exec(u"x").has_value());
    expect(open.exec(u"(").has_value()&&!open.exec(u"x").has_value());
};

"unicode_identity_escape_rejects_punctuation_but_class_hyphen_is_valid"_test=[] {
    bool rejected=false;
    try{host::RegExp invalid(u"\\!",u"u");}catch(const Exception& error){rejected=error.name=="SyntaxError";}
    expect(rejected);
    host::RegExp hyphen(u"[\\-a]",u"u");
    expect(hyphen.exec(u"-").has_value()&&hyphen.exec(u"a").has_value());
};

"unicode_decimal_backreferences_count_ordinary_captures"_test=[] {
    host::RegExp expression(u"(.)\\1",u"u");
    const auto match=expression.exec(u"aa");
    expect(match&&match->at(0)==std::optional<std::u16string>{u"aa"});
};

"unicode_decimal_escapes_are_rejected_inside_character_classes"_test=[] {
    bool rejected=false;
    try{host::RegExp invalid(u"(.)[\\1]",u"u");}
    catch(const Exception& error){rejected=error.name=="SyntaxError";}
    expect(rejected);
};

"regexp_split_uses_shared_sticky_host_and_flag_specific_advance"_test=[] {
    const std::vector<std::optional<std::u16string>> unicodeExpected{
        u"\U0001F600",u"x"};
    const auto unicode=host::split(u"\U0001F600x",u"(?:)",u"u");
    const bool unicodeMatches=unicode==unicodeExpected;
    expect(unicodeMatches);
    const std::u16string high(1,char16_t(0xD83D)),low(1,char16_t(0xDE00));
    const std::vector<std::optional<std::u16string>> legacyExpected{high,low,u"x"};
    const bool legacyMatches=host::split(u"\U0001F600x",u"(?:)",u"")==legacyExpected;
    expect(legacyMatches);
    expect(host::split(u"",u"(?:)",u"u").empty());
    const bool emptyInputMatches=host::split(u"",u"x",u"u")==std::vector<std::optional<std::u16string>>{u""};
    expect(emptyInputMatches);
    const std::vector<std::optional<std::u16string>> captureExpected{
        u"",u"A",u"",u"\U0001F600",u"",u"B",u""};
    const bool capturesMatch=host::split(u"A\U0001F600B",u"(.)",u"u")==captureExpected;
    expect(capturesMatch);
};
};
}
