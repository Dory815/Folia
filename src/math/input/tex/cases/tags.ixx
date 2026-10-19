// MathJax 4.1.3 input/tex/cases/CasesConfiguration.ts.
export module folia.math.input.tex.cases.tags;
export import folia.math.input.tex.ams.configuration;
import std;
export namespace folia::math::tex {
class CasesTags : public AmsTags {
    double subcounter_=0;
    std::u16string casesNumber(double,double) const;
public:
    using AmsTags::AmsTags;
    void start(options::Value,options::Value,options::Value) override;
    void autoTag() override;
    static TagsClassRef initializeModule(ParserRuntime&);
};
}
