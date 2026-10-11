// Generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/menclose.ts; Apache-2.0.
// SHA256 ffa4ab507744cb4f190312ca8dcd692474c333bee6e19c86709554aa9dad4333
module folia.math.core.mml_tree.mml_nodes.menclose;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlMencloseDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlNode::defaults());
value.setOwn("notation","longdiv");
return value;}();}
PropertyList& MmlMenclose::defaults(){return MmlMencloseDefaults;}
void MmlMenclose::initializeClassFields(){initializeTexClass(TEXCLASS.ORD);}
std::string MmlMenclose::kind()const{return "menclose";}
double MmlMenclose::arity()const{return -1.0;}
bool MmlMenclose::linebreakContainer()const{return true;}
}
