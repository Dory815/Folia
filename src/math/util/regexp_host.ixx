// Native language host for MathJax's UTF-16 non-Unicode scanner expressions.
// RegExp is a language primitive, not another MathJax parser or regex service.
export module folia.math.util.regexp_host;
import std;
export namespace folia::math::regexp_host {
enum class ErrorKind { syntax, unsupported, internal };
class RegexError final : public std::runtime_error {
public:
    ErrorKind kind;
    std::u16string message;
    RegexError(ErrorKind k, std::u16string text)
        : std::runtime_error("native regular expression error"), kind(k), message(std::move(text)) {}
};
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
    // Source cursor measured in UTF-16 code units. Stateful expressions read
    // and write this native integer directly; no JS property conversion exists.
    std::size_t last_index = 0;
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
};
// ECMAScript String.prototype.split with a RegExp separator. Uses the same
// compiled RegExp host and returns captured groups with unmatched groups as
// Undefined; the u flag controls AdvanceStringIndex exactly as in Source.
std::vector<std::optional<std::u16string>> split(
    std::u16string_view input,std::u16string pattern,std::u16string flags);
using RegExpRef=std::shared_ptr<RegExp>;
}
