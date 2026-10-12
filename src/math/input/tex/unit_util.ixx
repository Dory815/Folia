// MathJax 4.1.3 input/tex/UnitUtil.ts. Apache-2.0.
export module folia.math.input.tex.unit_util;
export import folia.math.input.tex.unit_pattern;
import std;
export namespace folia::math::tex {
using UnitEntries=std::vector<std::pair<std::u16string,double>>;
class UnitMap {
    UnitEntries map_;
    void updateDimen();
public:
    std::u16string num=u"([-+]?([.,]\\d+|\\d+([.,]\\d*)?))";
    std::u16string unit;
    UnitPatternRef dimenEnd,dimenRest;
    explicit UnitMap(UnitEntries);
    virtual ~UnitMap()=default;
    UnitMap(const UnitMap&)=delete;
    UnitMap& operator=(const UnitMap&)=delete;
    virtual UnitMap* set(std::u16string name,double ems);
    virtual Property get(Property name)const;
    virtual bool erase(std::u16string_view name);
};
using Dimension=std::array<Property,3>;
class UnitUtil {
    UnitUtil();
public:
    std::shared_ptr<UnitMap> UNIT_CASES;
    UnitUtil(const UnitUtil&)=delete;
    UnitUtil& operator=(const UnitUtil&)=delete;
    static UnitUtil& instance();
    Dimension matchDimen(Property dim,Property rest=Missing{});
    double dimen2em(Property dim);
    std::string em(double m);
    Property trimSpaces(Property text);
};
std::shared_ptr<UnitMap> defaultUnitCases();
}
