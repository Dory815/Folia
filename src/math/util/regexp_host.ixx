// Native language host for MathJax's UTF-16 non-Unicode scanner expressions.
// RegExp is a language primitive, not another MathJax parser or regex service.
export module folia.math.util.regexp_host;
export import folia.math.util.option_types;
import std;
export namespace folia::math::regexp_host {
struct Match {
    std::size_t index=0;
    std::u16string input;
    std::vector<std::optional<std::u16string>> captures;
    std::optional<std::vector<std::pair<std::u16string,std::optional<std::u16string>>>> groups;
    std::optional<std::u16string> at(std::size_t i)const {
        return i<captures.size()?captures[i]:std::nullopt;
    }
};
class RegExp {
    struct Compiled;
    std::unique_ptr<Compiled> compiled_;
    std::u16string pattern_,flags_;
    std::vector<std::pair<std::u16string,std::size_t>> namedGroups_;
    bool global_=false,sticky_=false,unicode_=false;
public:
    // One source-of-truth cell for the writable own lastIndex property. The
    // implicit numeric edge keeps native scanner callers source-compatible;
    // Context-bound exec performs the full JS ToNumber conversion instead.
    struct LastIndexCell {
        options::Value value=0.0;
        LastIndexCell& operator=(options::Value next){value=std::move(next);return *this;}
        template<class Number>
            requires std::is_arithmetic_v<Number>
        LastIndexCell& operator=(Number next){value=static_cast<double>(next);return *this;}
        operator double()const{
            if(const auto* number=std::get_if<double>(&value.data))return *number;
            throw std::logic_error("RegExp lastIndex requires Context conversion");
        }
    } lastIndex;
    using ToNumber=std::function<double(const options::Value&)>;
    using GetLastIndex=std::function<options::Value()>;
    using SetLastIndex=std::function<void(const options::Value&)>;
    explicit RegExp(std::u16string pattern,std::u16string flags={});
    ~RegExp();
    RegExp(const RegExp&)=delete;
    RegExp&operator=(const RegExp&)=delete;
    const std::u16string& pattern()const{return pattern_;}
    // ECMAScript RegExp.prototype.source serialization (distinct from the
    // raw constructor pattern used to compile and match).
    std::u16string source()const;
    const std::u16string& flags()const{return flags_;}
    std::optional<Match> exec(const std::u16string& input);
    std::optional<Match> exec(const std::u16string& input,const ToNumber& toNumber);
    std::optional<Match> exec(const std::u16string& input,const ToNumber& toNumber,
                              const SetLastIndex& setLastIndex);
    std::optional<Match> exec(const std::u16string& input,const ToNumber& toNumber,
                              const GetLastIndex& getLastIndex,const SetLastIndex& setLastIndex);
};
// ECMAScript String.prototype.split with a RegExp separator. Uses the same
// compiled RegExp host and returns captured groups with unmatched groups as
// Undefined; the u flag controls AdvanceStringIndex exactly as in Source.
std::vector<std::optional<std::u16string>> split(
    std::u16string_view input,std::u16string pattern,std::u16string flags);
using RegExpRef=std::shared_ptr<RegExp>;
}
