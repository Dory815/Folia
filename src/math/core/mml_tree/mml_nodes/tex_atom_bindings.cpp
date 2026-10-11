// Generated MathJax 4.1.3 source bindings; Apache-2.0.
// ts/core/MmlTree/MmlNodes/TeXAtom.ts SHA256 bad114224392b94852456a4636b85ad90a5a2c96b63e6c7911d155db299f7028
module folia.math.core.mml_tree.mml_nodes.tex_atom;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlTeXAtomDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlBaseNode::defaults());
return value;}();}
PropertyList& MmlTeXAtom::defaults(){return MmlTeXAtomDefaults;}
void MmlTeXAtom::initializeClassFields(){initializeTexClass(TEXCLASS.ORD);const Property value=texClass();setProperty("texClass",value);}
std::string MmlTeXAtom::kind()const{return "TeXAtom";}
double MmlTeXAtom::arity()const{return -1.0;}
Property MmlTeXAtom::notParent()const{return true;}
}
