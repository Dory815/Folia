module folia.math.input.tex.unit_util;
import std;
namespace folia::math::tex {
std::shared_ptr<UnitMap> defaultUnitCases(){
    constexpr double emPerInch=7.2,pxPerInch=72;
    return std::make_shared<UnitMap>(UnitEntries{{u"em",1},{u"ex",.43},{u"pt",1./10},
        {u"pc",1.2},{u"px",emPerInch/pxPerInch},{u"in",emPerInch},
        {u"cm",emPerInch/2.54},{u"mm",emPerInch/25.4},{u"mu",1./18}});
}
UnitUtil::UnitUtil():UNIT_CASES(defaultUnitCases()){}
UnitUtil& UnitUtil::instance(){static UnitUtil actual;return actual;}
}
