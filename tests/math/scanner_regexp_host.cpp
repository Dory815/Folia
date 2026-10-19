#include "support/folia_test.hpp"
import folia.math.util.regexp_host;
namespace {using namespace boost::ut;namespace host=folia::math::regexp_host;
suite scanner_host=[] {
    "math_port_scanner_regexp_supported_flags_and_invalid_flags_are_explicit"_test=[] {
        auto failure=[](std::u16string flags){try{host::RegExp pattern(u"x",std::move(flags));return std::string("ok");}catch(const folia::math::options::Exception&error){return error.name;}};
        expect(eq(failure(u"i"),std::string("ok")));
        expect(eq(failure(u"u"),std::string("ok")));
        expect(eq(failure(u"uv"),std::string("SyntaxError")));
        expect(eq(failure(u"ii"),std::string("SyntaxError")));
    };
};}
