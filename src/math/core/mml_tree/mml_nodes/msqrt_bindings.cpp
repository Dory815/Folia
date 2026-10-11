// Generated root source bindings; MathJax 4.1.3, Apache-2.0.
// ts/core/MmlTree/MmlNodes/msqrt.ts SHA256 8d522b96f39f35c170fe12ab7e59e0999c65551335466010522280bfa3570b7e
module folia.math.core.mml_tree.mml_nodes.msqrt;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {namespace {
PropertyList sourceDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlNode::defaults());
value.setOwn("data-vertical-align","bottom");
return value;}();
}
PropertyList& MmlMsqrt::defaults(){return sourceDefaults;}
void MmlMsqrt::initializeClassFields(){initializeTexClass(TEXCLASS.ORD);}
std::string MmlMsqrt::kind()const{return "msqrt";}
double MmlMsqrt::arity()const{return -1.0;}
bool MmlMsqrt::linebreakContainer()const{return true;}
}
