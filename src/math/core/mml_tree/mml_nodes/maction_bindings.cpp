// MathJax 4.1.3 ts/core/MmlTree/MmlNodes/maction.ts; Apache-2.0.
// SHA256 fa173b7e2b25b90e0f83ac201098357a8abf28462fa86fa69d92b65906527895
module folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {
namespace {PropertyList actionDefaults=[] {auto result=host::arithmeticCopy(AbstractMmlNode::defaults());result.setOwn("actiontype","toggle");result.setOwn("selection",1);return result;}();}
PropertyList& MmlMaction::defaults(){return actionDefaults;}
std::string MmlMaction::kind()const{return "maction";}
double MmlMaction::arity()const{return 1;}
}
