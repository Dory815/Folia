// Port owner: MathJax 4.1.3 input/tex/FindTeX.ts, Apache-2.0.
module folia.math.input.tex.find_tex;
import folia.math.util.string;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
void FindTeX::initializeFindTeX(){throw options::Exception("Unimplemented",u"FindTeX constructor");}
void FindTeX::getPatterns(){throw options::Exception("Unimplemented",u"FindTeX getPatterns");}
void FindTeX::addPattern(StringArray,Delims,bool){throw options::Exception("Unimplemented",u"FindTeX addPattern");}
options::Value FindTeX::endPattern(options::Value,options::Value){throw options::Exception("Unimplemented",u"FindTeX endPattern");}
Property FindTeX::findEnd(options::Value,double,const regexp_host::Match&,EndItem){throw options::Exception("Unimplemented",u"FindTeX findEnd");}
void FindTeX::findMathInString(core::ProtoMathItems,double,options::Value){throw options::Exception("Unimplemented",u"FindTeX findMathInString");}
core::ProtoMathItems FindTeX::findMath(std::any){throw options::Exception("Unimplemented",u"FindTeX findMath");}
}
