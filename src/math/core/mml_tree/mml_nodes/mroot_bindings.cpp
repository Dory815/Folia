// Generated root source bindings; MathJax 4.1.3, Apache-2.0.
// ts/core/MmlTree/MmlNodes/mroot.ts SHA256 b4b8f9deddb22efe7580067d8845d7cb225046e244eb93da8b0e931eff76e99a
module folia.math.core.mml_tree.mml_nodes.mroot;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {namespace {
PropertyList sourceDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlNode::defaults());
value.setOwn("data-vertical-align","bottom");
return value;}();
}
PropertyList& MmlMroot::defaults(){return sourceDefaults;}
void MmlMroot::initializeClassFields(){initializeTexClass(TEXCLASS.ORD);}
std::string MmlMroot::kind()const{return "mroot";}
double MmlMroot::arity()const{return 2.0;}
bool MmlMroot::linebreakContainer()const{return true;}
}
