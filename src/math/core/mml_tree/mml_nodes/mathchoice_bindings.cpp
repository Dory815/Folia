// MathJax 4.1.3 ts/core/MmlTree/MmlNodes/mathchoice.ts; Apache-2.0.
// SHA256 e1f9fdf92e345c7bc09e3333c4f436ae8f2d1e4c8fa9a2280877590cf9025d76
module folia.math.core.mml_tree.mml_nodes.mathchoice;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {
namespace {PropertyList choiceDefaults=host::arithmeticCopy(AbstractMmlBaseNode::defaults());}
PropertyList& MathChoice::defaults(){return choiceDefaults;}
std::string MathChoice::kind()const{return "MathChoice";}
double MathChoice::arity()const{return 4;}
bool MathChoice::notParent()const{return true;}
}
