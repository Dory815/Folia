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
    std::optional<std::u16string> at(std::size_t i)const {
        return i<captures.size()?captures[i]:std::nullopt;
    }
};
class RegExp {
    struct Compiled;
    std::unique_ptr<Compiled> compiled_;
    std::u16string pattern_,flags_;
    bool global_=false,sticky_=false;
public:
    // FindTeX uses flags g. Plain, global and sticky non-Unicode scans are
    // supported here; other valid JS flags fail explicitly until verified.
    double lastIndex=0;
    explicit RegExp(std::u16string pattern,std::u16string flags={});
    ~RegExp();
    RegExp(const RegExp&)=delete;
    RegExp&operator=(const RegExp&)=delete;
    const std::u16string& pattern()const{return pattern_;}
    const std::u16string& flags()const{return flags_;}
    std::optional<Match> exec(const std::u16string& input);
};
using RegExpRef=std::shared_ptr<RegExp>;
}
