module;
#include <srell.hpp>
module folia.math.util.regexp_host;
import std;
namespace folia::math::regexp_host {
struct CodeUnitTraits:srell::regex_traits<char16_t> {
    using utf_traits=srell::re_detail::utf_traits_core<char16_t>;
};
struct RegExp::Compiled {
    srell::basic_regex<char16_t,CodeUnitTraits> expression;
};
RegExp::RegExp(std::u16string pattern,std::u16string flags)
    :pattern_(std::move(pattern)),flags_(std::move(flags)) {
    throw options::Exception("Unimplemented",u"Native scanner RegExp compilation");
}
RegExp::~RegExp()=default;
std::optional<Match> RegExp::exec(const std::u16string&){
    throw options::Exception("Unimplemented",u"Native scanner RegExp execution");
}
}
