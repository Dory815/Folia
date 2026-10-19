module;
#define SRELL_ECMASCRIPT_LEGACY_ICASE 1
#define SRELL_NAMESPACE folia_math_legacy_srell
#include <srell.hpp>
#undef SRELL_NAMESPACE
#undef SRELL_ECMASCRIPT_LEGACY_ICASE

// The u flag has distinct code-point iteration and Unicode CaseFolding
// semantics.  Build a second SRELL namespace so the already-pinned legacy
// matcher keeps its UTF-16-unit iteration and ECMAScript legacy folding.
#undef SRELL_HPP_
#define SRELL_NAMESPACE folia_math_unicode_srell
#include <srell.hpp>
#undef SRELL_NAMESPACE

module folia.math.util.regexp_host;
import std;
namespace folia::math::regexp_host {
namespace legacy_rx = folia_math_legacy_srell;
namespace unicode_rx = folia_math_unicode_srell;
namespace rx = legacy_rx;
struct CodeUnitTraits:rx::regex_traits<char16_t> {
    using utf_traits=rx::re_detail::utf_traits_core<char16_t>;
};
struct RegExp::Compiled {
    legacy_rx::basic_regex<char16_t,CodeUnitTraits> legacyExpression;
    unicode_rx::basic_regex<char16_t,unicode_rx::u16regex_traits<char16_t>> unicodeExpression;
};
namespace {
struct UnicodeSyntaxError { std::u16string reason; };
std::u16string regexpSource(std::u16string_view pattern){
    if(pattern.empty())return u"(?:)";
    std::u16string result;
    result.reserve(pattern.size()+4);
    bool escaped=false,inClass=false;
    for(const auto c:pattern){
        // RegExp.prototype.source needs an escaped slash in the pattern body,
        // but slashes inside a character class are already unambiguous.
        if(c==u'/'&&!escaped&&!inClass)result.push_back(u'\\');
        if(c==u'\n'){result+=u"\\n";escaped=false;continue;}
        if(c==u'\r'){result+=u"\\r";escaped=false;continue;}
        if(c==0x2028){result+=u"\\u2028";escaped=false;continue;}
        if(c==0x2029){result+=u"\\u2029";escaped=false;continue;}
        result.push_back(c);
        if(!escaped&&c==u'[')inClass=true;
        else if(!escaped&&c==u']')inClass=false;
        if(c==u'\\')escaped=!escaped;else escaped=false;
    }
    return result;
}
std::u16string compilePattern(std::u16string_view source){
    std::u16string result;
    constexpr std::u16string_view syntax=u"^$\\.*+?()[]{}|/";
    constexpr char16_t hex[]=u"0123456789abcdef";
    // In legacy ECMAScript mode, malformed braced quantifiers are literal
    // pattern characters. Keep syntactically valid tokens intact so the
    // matcher can still reject bad placement and reversed bounds.
    const auto bracedQuantifierEnd=[&](std::size_t start)->std::optional<std::size_t>{
        std::size_t i=start+1;
        const auto digits=[&](std::size_t& at){
            const auto first=at;
            while(at<source.size()&&source[at]>=u'0'&&source[at]<=u'9')++at;
            return at!=first;
        };
        if(!digits(i))return std::nullopt;
        if(i<source.size()&&source[i]==u'}')return i;
        if(i>=source.size()||source[i++]!=u',')return std::nullopt;
        (void)digits(i);
        return i<source.size()&&source[i]==u'}'?std::optional<std::size_t>(i):std::nullopt;
    };
    std::vector<bool> wrapLookaheadStart(source.size()),wrapLookaheadEnd(source.size());
    std::vector<std::pair<std::size_t,bool>> groups;
    bool scanClass=false,scanEscape=false;
    for(std::size_t i=0;i<source.size();++i){
        const auto c=source[i];
        if(scanEscape){scanEscape=false;continue;}
        if(c==u'\\'){scanEscape=true;continue;}
        if(c==u'['){scanClass=true;continue;}
        if(c==u']'){scanClass=false;continue;}
        if(scanClass)continue;
        if(c==u'('){
            const bool lookahead=i+2<source.size()&&source[i+1]==u'?'&&
                (source[i+2]==u'='||source[i+2]==u'!');
            groups.emplace_back(i,lookahead);
        }else if(c==u')'&&!groups.empty()){
            const auto [open,lookahead]=groups.back();groups.pop_back();
            if(lookahead&&bracedQuantifierEnd(i+1)){
                // Legacy ECMAScript permits quantifying lookahead assertions;
                // grouping the assertion keeps that grammar for SRELL.
                wrapLookaheadStart[open]=true;wrapLookaheadEnd[i]=true;
            }
        }
    }
    bool inClass=false;
    for(std::size_t i=0;i<source.size();++i){
        auto unit=source[i];
        if(unit==u'['&&!inClass){inClass=true;result+=unit;continue;}
        if(unit==u']'&&inClass){inClass=false;result+=unit;continue;}
        if(!inClass&&unit==u'{'){
            if(const auto end=bracedQuantifierEnd(i)){
                result.append(source.substr(i,*end-i+1));i=*end;
            }else result+=u"\\{";
            continue;
        }
        if(!inClass&&unit==u'}'){result+=u"\\}";continue;}
        if(unit!=u'\\'||i+1==source.size()){
            if(wrapLookaheadStart[i])result+=u"(?:";
            result+=unit;
            if(wrapLookaheadEnd[i])result+=u")";
            continue;
        }
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
std::u16string compileUnicodePattern(std::u16string_view source){
    std::u16string result;
    constexpr std::u16string_view syntax=u"^$\\.*+?()[]{}|/";
    constexpr char16_t hex[]=u"0123456789abcdef";
    auto appendLiteralEscape=[&](std::uint32_t codepoint){
        auto appendUnit=[&](std::uint16_t unit){
            result+=u"\\u";
            for(int shift=12;shift>=0;shift-=4)result+=hex[(unit>>shift)&15];
        };
        if(codepoint<=0xFFFF){appendUnit(static_cast<std::uint16_t>(codepoint));return;}
        const auto scalar=codepoint-0x10000;
        appendUnit(static_cast<std::uint16_t>(0xD800+(scalar>>10)));
        appendUnit(static_cast<std::uint16_t>(0xDC00+(scalar&0x3FF)));
    };
    bool inClass=false;
    for(std::size_t i=0;i<source.size();++i){
        const auto unit=source[i];
        // ECMAScript's empty negated class is the any-code-point matcher;
        // SRELL does not accept this production grammar directly.
        if(!inClass&&unit==u'['&&i+2<source.size()&&source[i+1]==u'^'&&source[i+2]==u']'){
            result+=u"[\\s\\S]";i+=2;continue;
        }
        if(unit==u'[')inClass=true;
        else if(unit==u']')inClass=false;
        if(unit!=u'\\'||i+1==source.size()){result+=unit;continue;}
        const auto escaped=source[++i];
        if(escaped==u'u'&&i+1<source.size()&&source[i+1]==u'{'){
            i+=2;std::uint32_t codepoint=0;bool any=false;
            while(i<source.size()&&source[i]!=u'}'){
                const auto c=source[i];std::uint32_t digit;
                if(c>=u'0'&&c<=u'9')digit=c-u'0';
                else if(c>=u'a'&&c<=u'f')digit=c-u'a'+10;
                else if(c>=u'A'&&c<=u'F')digit=c-u'A'+10;
                else throw UnicodeSyntaxError{u"Invalid Unicode escape"};
                if(codepoint>0x10FFFF/16)throw UnicodeSyntaxError{u"Invalid Unicode escape"};
                codepoint=codepoint*16+digit;any=true;
                if(codepoint>0x10FFFF)throw UnicodeSyntaxError{u"Invalid Unicode escape"};
                ++i;
            }
            if(i>=source.size()||source[i]!=u'}'||!any)
                throw UnicodeSyntaxError{u"Invalid Unicode escape"};
            // A code-point escape must remain a literal even when its value
            // is a RegExp metacharacter such as '.' or '('.
            appendLiteralEscape(codepoint);
            continue;
        }
        const bool alphanumeric=(escaped>=u'a'&&escaped<=u'z')||
            (escaped>=u'A'&&escaped<=u'Z')||(escaped>=u'0'&&escaped<=u'9');
        const bool classHyphen=inClass&&escaped==u'-';
        if(!alphanumeric&&syntax.find(escaped)==syntax.npos&&!classHyphen&&escaped<0x80)
            throw UnicodeSyntaxError{u"Invalid escape"};
        result+=u'\\';result+=escaped;
    }
    return result;
}
double cursorLength(double value){
    if(std::isnan(value)||value<=0)return 0;
    return std::min(std::floor(value),9007199254740991.0);
}
std::vector<std::pair<std::u16string,std::size_t>> namedGroups(std::u16string_view pattern){
    std::vector<std::pair<std::u16string,std::size_t>> result;
    std::size_t capture=0;
    bool escaped=false,inClass=false;
    for(std::size_t i=0;i<pattern.size();++i){
        const auto c=pattern[i];
        if(escaped){escaped=false;continue;}
        if(c==u'\\'){escaped=true;continue;}
        if(c==u'['){inClass=true;continue;}
        if(c==u']'){inClass=false;continue;}
        if(inClass||c!=u'(')continue;
        if(i+1<pattern.size()&&pattern[i+1]==u'?'){
            if(i+2<pattern.size()&&pattern[i+2]==u'<'){
                if(i+3<pattern.size()&&(pattern[i+3]==u'='||pattern[i+3]==u'!'))continue;
                const auto end=pattern.find(u'>',i+3);
                if(end==std::u16string_view::npos)continue;
                ++capture;result.emplace_back(std::u16string(pattern.substr(i+3,end-(i+3))),capture);i=end;
            }
            continue;
        }
        ++capture;
    }
    return result;
}
std::size_t captureCount(std::u16string_view pattern){
    std::size_t capture=0;
    bool escaped=false,inClass=false;
    for(std::size_t i=0;i<pattern.size();++i){
        const auto c=pattern[i];
        if(escaped){escaped=false;continue;}
        if(c==u'\\'){escaped=true;continue;}
        if(c==u'['){inClass=true;continue;}
        if(c==u']'){inClass=false;continue;}
        if(inClass||c!=u'(')continue;
        if(i+1<pattern.size()&&pattern[i+1]==u'?'){
            // Named captures are the only capturing form introduced by `?`.
            // Lookarounds, non-capturing groups, and other assertions do not
            // contribute to decimal-backreference numbering.
            if(i+2<pattern.size()&&pattern[i+2]==u'<'){
                if(i+3<pattern.size()&&(pattern[i+3]==u'='||pattern[i+3]==u'!'))continue;
                const auto end=pattern.find(u'>',i+3);
                if(end!=std::u16string_view::npos){++capture;i=end;}
            }
            continue;
        }
        ++capture;
    }
    return capture;
}
void validateUnicodeDecimalEscapes(std::u16string_view pattern,std::size_t captures){
    bool inClass=false;
    for(std::size_t i=0;i<pattern.size();++i){
        if(pattern[i]==u'['){inClass=true;continue;}
        if(pattern[i]==u']'){inClass=false;continue;}
        if(pattern[i]!=u'\\'||i+1>=pattern.size())continue;
        const auto escaped=pattern[++i];
        if(escaped<u'0'||escaped>u'9')continue;
        // Decimal escapes are never backreferences inside a character class
        // in Unicode mode. The digit itself is an invalid identity escape.
        if(inClass&&escaped!=u'0')throw UnicodeSyntaxError{u"Invalid escape"};
        if(escaped==u'0'){
            if(i+1<pattern.size()&&pattern[i+1]>=u'0'&&pattern[i+1]<=u'9')
                throw UnicodeSyntaxError{u"Invalid decimal escape"};
            continue;
        }
        std::size_t reference=static_cast<std::size_t>(escaped-u'0');
        while(i+1<pattern.size()&&pattern[i+1]>=u'0'&&pattern[i+1]<=u'9'){
            reference=reference*10+static_cast<std::size_t>(pattern[++i]-u'0');
        }
        if(reference>captures)throw UnicodeSyntaxError{u"Invalid escape"};
    }
}
std::u16string unicodeSyntaxMessage(std::u16string_view pattern,std::u16string_view flags,std::u16string_view reason){
    std::u16string message=u"Invalid regular expression: /";
    message+=regexpSource(pattern);message+=u"/";message+=flags;message+=u": ";message+=reason;
    return message;
}
std::size_t advanceStringIndex(std::u16string_view input,std::size_t index,bool unicode){
    if(!unicode||index+1>=input.size())return index+1;
    const auto first=static_cast<std::uint32_t>(input[index]);
    const auto second=static_cast<std::uint32_t>(input[index+1]);
    return first>=0xD800&&first<=0xDBFF&&second>=0xDC00&&second<=0xDFFF?index+2:index+1;
}
}
RegExp::RegExp(std::u16string pattern,std::u16string flags)
    :pattern_(std::move(pattern)),flags_(std::move(flags)) {
    namedGroups_=namedGroups(pattern_);
    std::u16string seen;
    for(auto flag:flags_){
        if(std::u16string_view(u"dgimsuvy").find(flag)==std::u16string_view::npos||seen.find(flag)!=seen.npos)
            throw options::Exception("SyntaxError",u"Invalid regular expression flags");
        seen+=flag;
    }
    if(seen.find(u'u')!=seen.npos&&seen.find(u'v')!=seen.npos)
        throw options::Exception("SyntaxError",u"Incompatible regular expression flags");
    for(auto flag:flags_)if(flag!=u'g'&&flag!=u'y'&&flag!=u'i'&&flag!=u'u')
        throw options::Exception("Unimplemented",u"Unverified native regular expression flag");
    std::u16string canonicalFlags;
    for(const auto flag:std::u16string_view(u"dgimsuvy"))if(seen.find(flag)!=seen.npos)canonicalFlags+=flag;
    flags_=std::move(canonicalFlags);
    global_=flags_.find(u'g')!=flags_.npos;sticky_=flags_.find(u'y')!=flags_.npos;
    unicode_=flags_.find(u'u')!=flags_.npos;
    auto compiled=std::make_unique<Compiled>();
    try{
        if(unicode_){
            validateUnicodeDecimalEscapes(pattern_,captureCount(pattern_));
            const auto translated=compileUnicodePattern(pattern_);
            auto syntax=unicode_rx::regex_constants::ECMAScript;
            if(flags_.find(u'i')!=flags_.npos)syntax|=unicode_rx::regex_constants::icase;
            compiled->unicodeExpression.assign(translated.begin(),translated.end(),syntax);
        }else{
            const auto translated=compilePattern(pattern_);
            auto syntax=legacy_rx::regex_constants::ECMAScript;
            if(flags_.find(u'i')!=flags_.npos)syntax|=legacy_rx::regex_constants::icase;
            compiled->legacyExpression.assign(translated.begin(),translated.end(),syntax);
        }
    }
    catch(const UnicodeSyntaxError& error){
        throw options::Exception("SyntaxError",unicodeSyntaxMessage(pattern_,flags_,error.reason));
    }
    catch(const legacy_rx::regex_error&){throw options::Exception("SyntaxError",u"Invalid regular expression pattern");}
    catch(const unicode_rx::regex_error& error){
        std::u16string_view reason=u"Invalid regular expression";
        if(error.code()==unicode_rx::regex_constants::error_range)reason=u"Range out of order in character class";
        else if(error.code()==unicode_rx::regex_constants::error_escape)reason=u"Invalid escape";
        throw options::Exception("SyntaxError",unicodeSyntaxMessage(pattern_,flags_,reason));
    }
    compiled_=std::move(compiled);
}
RegExp::~RegExp()=default;
std::u16string RegExp::source()const{return regexpSource(pattern_);}
std::optional<Match> RegExp::exec(const std::u16string&input){
    return exec(input,[](const options::Value& raw){
        if(const auto* number=std::get_if<double>(&raw.data))return *number;
        throw options::Exception("TypeError",u"RegExp lastIndex requires Context numeric conversion");
    },[this](const options::Value& next){lastIndex=next;});
}
std::optional<Match> RegExp::exec(const std::u16string&input,const ToNumber&toNumber){
    return exec(input,toNumber,[this](const options::Value& next){lastIndex=next;});
}
std::optional<Match> RegExp::exec(const std::u16string&input,const ToNumber&toNumber,
                                  const SetLastIndex&setLastIndex){
    return exec(input,toNumber,[this]{return lastIndex.value;},setLastIndex);
}
std::optional<Match> RegExp::exec(const std::u16string&input,const ToNumber&toNumber,
                                  const GetLastIndex&getLastIndex,
                                  const SetLastIndex&setLastIndex){
    const bool stateful=global_||sticky_;
    // Source: Get(lastIndex), then ToLength. If conversion throws, the raw
    // writable property remains unchanged; all successful conversions happen
    // exactly once before any later matching or writeback.
    const double requestedCursor=cursorLength(toNumber(getLastIndex()));
    double cursor=stateful?requestedCursor:0;
    if(unicode_&&cursor>0&&cursor<input.size()){
        const auto current=static_cast<std::uint32_t>(input[static_cast<std::size_t>(cursor)]);
        const auto previous=static_cast<std::uint32_t>(input[static_cast<std::size_t>(cursor)-1]);
        if(current>=0xDC00&&current<=0xDFFF&&previous>=0xD800&&previous<=0xDBFF)--cursor;
    }
    if(cursor>static_cast<double>(input.size())){if(stateful)setLastIndex(options::Value(0));return {};}
    auto buildMatch=[&](const auto& matches)->Match{
        Match result;result.index=static_cast<std::size_t>(matches.position());result.input=input;
        result.captures.reserve(matches.size());
        for(const auto&capture:matches){
            if(capture.matched)result.captures.emplace_back(capture.str());
            else result.captures.emplace_back(std::nullopt);
        }
        if(!namedGroups_.empty()){
            result.groups.emplace();
            for(const auto& [name,index]:namedGroups_){
                if(index<matches.size()&&matches[index].matched)
                    result.groups->emplace_back(name,std::u16string(matches[index].str()));
                else result.groups->emplace_back(name,std::nullopt);
            }
        }
        if(stateful)setLastIndex(options::Value(static_cast<double>(result.index+matches.length())));
        return result;
    };
    if(unicode_){
        unicode_rx::match_results<std::u16string::const_iterator> matches;
        auto flags=unicode_rx::regex_constants::match_default;
        if(sticky_)flags|=unicode_rx::regex_constants::match_continuous;
        if(!unicode_rx::regex_search(input,static_cast<std::size_t>(cursor),matches,compiled_->unicodeExpression,flags)){
            if(stateful)setLastIndex(options::Value(0));return {};
        }
        return buildMatch(matches);
    }
    legacy_rx::match_results<std::u16string::const_iterator> matches;
    auto flags=legacy_rx::regex_constants::match_default;
    if(sticky_)flags|=legacy_rx::regex_constants::match_continuous;
    if(!legacy_rx::regex_search(input,static_cast<std::size_t>(cursor),matches,compiled_->legacyExpression,flags)){
        if(stateful)setLastIndex(options::Value(0));return {};
    }
    return buildMatch(matches);
}
std::vector<std::optional<std::u16string>> split(
    std::u16string_view source,std::u16string pattern,std::u16string flags){
    const bool unicode=flags.find(u'u')!=flags.npos;
    std::u16string splitFlags;
    for(const auto flag:flags)if(flag!=u'g'&&flag!=u'y')splitFlags+=flag;
    splitFlags+=u'y';
    RegExp splitter(std::move(pattern),std::move(splitFlags));
    const std::u16string input(source);
    if(input.empty()){
        splitter.lastIndex=options::Value(0);
        return splitter.exec(input)?std::vector<std::optional<std::u16string>>{}:
                                    std::vector<std::optional<std::u16string>>{std::u16string{}};
    }
    std::vector<std::optional<std::u16string>> output;
    std::size_t p=0,q=0;
    while(q<input.size()){
        splitter.lastIndex=static_cast<double>(q);
        const auto found=splitter.exec(input);
        if(!found){q=advanceStringIndex(input,q,unicode);continue;}
        const auto whole=found->at(0);
        if(!whole)throw options::Exception("TypeError",u"RegExp split match has no full capture");
        const auto e=found->index+whole->size();
        if(e==p){q=advanceStringIndex(input,q,unicode);continue;}
        output.emplace_back(input.substr(p,found->index-p));
        for(std::size_t i=1;i<found->captures.size();++i)output.push_back(found->captures[i]);
        p=e;q=p;
    }
    output.emplace_back(input.substr(p));
    return output;
}
}
