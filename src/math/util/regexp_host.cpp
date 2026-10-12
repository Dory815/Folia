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
namespace {
std::u16string compilePattern(std::u16string_view source){
    std::u16string result;
    constexpr std::u16string_view syntax=u"^$\\.*+?()[]{}|/";
    constexpr char16_t hex[]=u"0123456789abcdef";
    for(std::size_t i=0;i<source.size();++i){
        auto unit=source[i];
        if(unit!=u'\\'||i+1==source.size()){result+=unit;continue;}
        const auto escaped=source[++i];
        const bool alphanumeric=(escaped>=u'a'&&escaped<=u'z')||
            (escaped>=u'A'&&escaped<=u'Z')||(escaped>=u'0'&&escaped<=u'9');
        // Non-Unicode IdentityEscape accepts escaped punctuation such as
        // quotePattern's colon/hyphen, even where SRELL's strict grammar does
        // not. Emit a character escape, keeping literal class/range semantics.
        if(!alphanumeric&&syntax.find(escaped)==syntax.npos&&escaped<0x80){
            result+=u"\\x";result+=hex[(escaped>>4)&15];result+=hex[escaped&15];
        }else{result+=u'\\';result+=escaped;}
    }
    return result;
}
double cursorLength(double value){
    if(std::isnan(value)||value<=0)return 0;
    return std::min(std::floor(value),9007199254740991.0);
}
}
RegExp::RegExp(std::u16string pattern,std::u16string flags)
    :pattern_(std::move(pattern)),flags_(std::move(flags)) {
    std::u16string seen;
    for(auto flag:flags_){
        if(std::u16string_view(u"dgimsuvy").find(flag)==std::u16string_view::npos||seen.find(flag)!=seen.npos)
            throw options::Exception("SyntaxError",u"Invalid regular expression flags");
        seen+=flag;
    }
    if(seen.find(u'u')!=seen.npos&&seen.find(u'v')!=seen.npos)
        throw options::Exception("SyntaxError",u"Incompatible regular expression flags");
    for(auto flag:flags_)if(flag!=u'g'&&flag!=u'y')
        throw options::Exception("Unimplemented",u"Unverified native regular expression flag");
    global_=flags_.find(u'g')!=flags_.npos;sticky_=flags_.find(u'y')!=flags_.npos;
    auto compiled=std::make_unique<Compiled>();
    const auto translated=compilePattern(pattern_);
    try{compiled->expression.assign(translated.begin(),translated.end(),srell::regex_constants::ECMAScript);}
    catch(const srell::regex_error&){throw options::Exception("SyntaxError",u"Invalid regular expression pattern");}
    compiled_=std::move(compiled);
}
RegExp::~RegExp()=default;
std::optional<Match> RegExp::exec(const std::u16string&input){
    const bool stateful=global_||sticky_;
    const double cursor=stateful?cursorLength(lastIndex):0;
    if(cursor>static_cast<double>(input.size())){if(stateful)lastIndex=0;return {};}
    srell::match_results<std::u16string::const_iterator> matches;
    auto flags=srell::regex_constants::match_default;
    if(sticky_)flags|=srell::regex_constants::match_continuous;
    // Preserve the whole input as the lookbehind/index base; do not slice it.
    const bool found=srell::regex_search(input,static_cast<std::size_t>(cursor),matches,compiled_->expression,flags);
    if(!found){if(stateful)lastIndex=0;return {};}
    Match result;result.index=static_cast<std::size_t>(matches.position());result.input=input;
    result.captures.reserve(matches.size());
    for(const auto&capture:matches){
        if(capture.matched)result.captures.emplace_back(capture.str());
        else result.captures.emplace_back(std::nullopt);
    }
    if(stateful)lastIndex=static_cast<double>(result.index+matches.length());
    return result;
}
}
