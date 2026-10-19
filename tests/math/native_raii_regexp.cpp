#include "support/folia_test.hpp"
import folia.math.util.regexp_host;

using namespace boost::ut;
using folia::math::regexp_host::RegExp;
using folia::math::regexp_host::RegexError;

suite native_regexp_host_tests = [] {
    "legacy and Unicode folding retain their distinct SRELL policies"_test = [] {
        RegExp legacy_k(u"k", u"i"), legacy_s(u"s", u"i");
        RegExp unicode_k(u"k", u"iu"), unicode_s(u"s", u"iu");
        expect(!legacy_k.exec(u"K").has_value());
        expect(!legacy_s.exec(u"ſ").has_value());
        expect(unicode_k.exec(u"K").has_value());
        expect(unicode_s.exec(u"ſ").has_value());
    };
    "typed cursor preserves global capture and failure behavior"_test = [] {
        RegExp rx(u"(?<letter>[a-z])", u"g");
        rx.last_index = 2;
        const auto match = rx.exec(u"--ab");
        expect(match.has_value());
        if (match) {
            expect(match->index == 2u);
            expect(match->at(1) == std::optional<std::u16string>(u"a"));
            expect(match->groups.has_value());
            expect(rx.last_index == 3u);
        }
        const auto second = rx.exec(u"--ab");
        expect(second.has_value());
        if (second) {
            expect(second->index == 3u);
            expect(second->at(1) == std::optional<std::u16string>(u"b"));
            expect(rx.last_index == 4u);
        }
        expect(!rx.exec(u"--ab").has_value());
        expect(rx.last_index == 0u);
    };
    "Unicode cursor backs up to a surrogate pair boundary"_test = [] {
        const std::u16string input{u'x', char16_t(0xD83D), char16_t(0xDE00), u'y'};
        RegExp rx(u".", u"uy");
        rx.last_index = 2;
        const auto match = rx.exec(input);
        expect(match.has_value());
        if (match) {
            expect(match->index == 1u);
            expect(match->at(0)->size() == 2u);
            expect(rx.last_index == 3u);
        }
    };
    "split retains unmatched capture as an empty optional"_test = [] {
        auto pieces = folia::math::regexp_host::split(u"a1b", u"([0-9])|(x)", u"");
        expect(pieces.size() == 4u);
        expect(pieces[1] == std::optional<std::u16string>(u"1"));
        expect(!pieces[2].has_value());
    };
    "invalid syntax has a typed native exception"_test = [] {
        bool syntax = false;
        try { RegExp rx(u"("); }
        catch (const RegexError& error) { syntax = error.kind == folia::math::regexp_host::ErrorKind::syntax; }
        expect(syntax);
    };
    "source serialization and canonical flags stay available"_test = [] {
        RegExp rx(u"/a/", u"ig");
        expect(rx.flags() == u"gi");
        expect(rx.source() == u"\\/a\\/");
    };
};
