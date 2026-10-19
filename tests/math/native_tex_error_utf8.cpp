#include "support/folia_test.hpp"
import folia.math.input.tex.tex_error;

using namespace boost::ut;
using folia::math::tex::TexError;

suite tex_error_utf8_tests = [] {
    "what_encodes_valid_utf16_and_replaces_unpaired_surrogates"_test = [] {
        TexError valid("ID", u"A\U0001F600Z");
        expect(valid.message() == u"A\U0001F600Z");
        expect(std::string_view(valid.what()) == std::string_view("A\xF0\x9F\x98\x80Z"));

        std::u16string malformed;
        malformed.push_back(static_cast<char16_t>(0xD800));
        malformed.push_back(u'x');
        malformed.push_back(static_cast<char16_t>(0xDC00));
        TexError replaced("ID", std::move(malformed));
        expect(replaced.message().size() == 3_u);
        expect(static_cast<unsigned>(replaced.message()[0]) == 0xD800);
        expect(static_cast<unsigned>(replaced.message()[2]) == 0xDC00);
        expect(std::string_view(replaced.what()) == std::string_view("\xEF\xBF\xBDx\xEF\xBF\xBD"));
    };
};
