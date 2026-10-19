// MathJax 4.1.3 util/Styles.ts; Apache-2.0.
export module folia.math.util.styles;
export import folia.math.util.regexp_host;
import std;

export namespace folia::math::styles {
using StyleList = std::vector<std::pair<std::string, std::u16string>>;
extern const std::vector<std::string> TRBL;
extern const std::vector<std::string> WSC;

struct Styles;
struct StylesAccess;
struct StylesConnection {
    std::vector<std::string> children;
    std::vector<std::string> parts;
    bool hasParts = false;
    bool subPart = false;
    std::function<void(Styles&, std::string_view)> split;
    std::function<void(Styles&, std::string_view)> combine;
};
struct StylesPatterns {
    regexp_host::RegExpRef sanitize;
    regexp_host::RegExpRef value;
    regexp_host::RegExpRef style;
    regexp_host::RegExpRef comment;
};
// An ordered own-key list, as the Source object literal defines it. Algorithms
// only look up entries, but observers also need Source Object.keys order.
using StylesConnections = std::vector<std::pair<std::string, StylesConnection>>;

struct StyleValue {
    std::variant<std::u16string, double, bool> value;
    StyleValue(std::u16string text) : value(std::move(text)) {}
    StyleValue(const char16_t* text) : value(std::u16string(text)) {}
    StyleValue(double number) : value(number) {}
    StyleValue(bool boolean) : value(boolean) {}
};

struct Styles {
    static const StylesPatterns pattern;
    static const StylesConnections connect;

protected:
    StyleList styles_;
    std::u16string const* lookup(std::string_view name) const noexcept;
    void store(std::string_view name, std::u16string value);
    void erase(std::string_view name) noexcept;
    // A class/module static owns this pattern table for longer than any Styles instance.
    const StylesPatterns* patterns_;
    explicit Styles(std::u16string_view cssText, const StylesPatterns& classPatterns);
    const StylesPatterns& patternTable() const { return *patterns_; }
    friend struct StylesAccess;

    void parse(std::u16string_view cssText = {});
    std::u16string sanitizeValue(std::u16string_view text) const;
    void setStyle(std::string_view name, std::u16string_view value);
    void combineChildren(std::string_view name);
    std::string parentName(std::string_view name) const;
    std::string childName(std::string_view name, std::string_view child) const;
    std::string normalizeName(std::u16string_view name) const;

public:
    explicit Styles(std::u16string_view cssText = {});
    virtual ~Styles() = default;
    Styles(const Styles&) = delete;
    Styles& operator=(const Styles&) = delete;

    virtual std::u16string cssText() const;
    virtual StyleList styleList() const;
    virtual void set(std::u16string_view name, StyleValue value);
    virtual std::u16string get(std::u16string_view name) const;
    virtual void combineParent(std::string_view name);
};
}
