// Generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/malignmark.ts; Apache-2.0.
// SHA256 f35d8b9edcf1afce67f74eef1dc1c04434f9caea944b3f3fd439d1aaf484cb61
module folia.math.core.mml_tree.mml_nodes.malignmark;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlMalignmarkDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlNode::defaults());
value.setOwn("edge","left");
return value;}();}
PropertyList& MmlMalignmark::defaults(){return MmlMalignmarkDefaults;}
std::string MmlMalignmark::kind()const{return "malignmark";}
double MmlMalignmark::arity()const{return 0.0;}
bool MmlMalignmark::isSpacelike()const{return true;}
}
