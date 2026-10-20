// MathJax 4.1.3 input/tex/UnitUtil.ts; Apache-2.0.
export module folia.math.input.tex.unit_util;
export import folia.math.util.regexp_host;
import std;

export namespace folia::math::tex {
using UnitEntries = std::vector<std::pair<std::u16string, double>>;

class UnitMap {
    UnitEntries map_;
    void updateDimen();
public:
    std::u16string num = u"([-+]?([.,]\\d+|\\d+([.,]\\d*)?))";
    std::u16string unit;
    std::unique_ptr<regexp_host::RegExp> dimenEnd, dimenRest;

    explicit UnitMap(UnitEntries);
    virtual ~UnitMap() = default;
    UnitMap(UnitMap const&) = delete;
    UnitMap& operator=(UnitMap const&) = delete;
    UnitMap(UnitMap&&) noexcept = default;
    UnitMap& operator=(UnitMap&&) noexcept = default;
    virtual UnitMap* set(std::u16string name, double ems);
    virtual std::optional<double> get(std::u16string_view name) const;
    virtual bool erase(std::u16string_view name);
};

struct DimensionMatch final {
    std::u16string value;
    std::u16string unit;
    std::size_t matched_length{};
    friend bool operator==(DimensionMatch const&, DimensionMatch const&) = default;
};
using Dimension = std::optional<DimensionMatch>;

class UnitUtil {
    UnitUtil();
public:
    UnitMap UNIT_CASES;
    UnitUtil(UnitUtil const&) = delete;
    UnitUtil& operator=(UnitUtil const&) = delete;
    static UnitUtil& instance();
    Dimension matchDimen(std::u16string_view dim, bool rest = false);
    double dimen2em(std::u16string_view dim);
    std::string em(double value);
    std::u16string trimSpaces(std::u16string_view text);
};

UnitMap defaultUnitCases(); // a fresh value, never a second global map owner
}
