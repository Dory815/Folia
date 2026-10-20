// MathJax 4.1.3 input/tex/FilterUtil.ts; Apache-2.0.
export module folia.math.input.tex.filter_util;
export import folia.math.input.tex.parser;

export namespace folia::math::tex {
struct FilterUtil final {
    static void cleanSubSup(TexFilterData &);
    static void setInherited(TexFilterData &);
    static void checkScriptlevel(TexFilterData &);
    static void moveLimits(TexFilterData &);
    static void cleanStretchy(TexFilterData &);
    static void cleanAttributes(TexFilterData &);
    static void combineRelations(TexFilterData &);
};
} // namespace folia::math::tex
