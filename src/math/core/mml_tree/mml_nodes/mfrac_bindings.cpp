// Generated root source bindings; MathJax 4.1.3, Apache-2.0.
// ts/core/MmlTree/MmlNodes/mfrac.ts SHA256 4ded5fc9922d462d02818523d676c30299d08cfe7794067bd4e20950c8fccfc7
module folia.math.core.mml_tree.mml_nodes.mfrac;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {namespace {
PropertyList sourceDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlBaseNode::defaults());
value.setOwn("linethickness","medium");
value.setOwn("numalign","center");
value.setOwn("denomalign","center");
value.setOwn("bevelled",false);
return value;}();
}
PropertyList& MmlMfrac::defaults(){return sourceDefaults;}
std::string MmlMfrac::kind()const{return "mfrac";}
double MmlMfrac::arity()const{return 2.0;}
bool MmlMfrac::linebreakContainer()const{return true;}
std::string MmlMfrac::linebreakAlign()const{return "";}
}
