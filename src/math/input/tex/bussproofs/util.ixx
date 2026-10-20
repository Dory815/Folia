// MathJax 4.1.3 input/tex/bussproofs/BussproofsUtil.ts; Apache-2.0.
export module folia.math.input.tex.bussproofs.util;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex::bussproofs_util {
void saveDocument(TexFilterData &);
void clearDocument(TexFilterData &);
void makeBsprAttributes(TexFilterData &);
void balanceRules(TexFilterData &);
} // namespace folia::math::tex::bussproofs_util
