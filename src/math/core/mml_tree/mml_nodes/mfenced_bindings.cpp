// Root-generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/mfenced.ts; Apache-2.0.
// SHA256 64b3f4d89d47765eb83034b480b843b2769d8de12e31d7571c50e5c4bb2bd397
module folia.math.core.mml_tree.mml_nodes.mfenced;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MfencedDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlNode::defaults());
value.setOwn("open","(");
value.setOwn("close",")");
value.setOwn("separators",",");
return value;}();}
PropertyList& MmlMfenced::defaults(){return MfencedDefaults;}
void MmlMfenced::initializeClassFields(){
initializeTexClass(TEXCLASS.INNER);
separators=std::make_shared<std::vector<MmlNode*>>();
open=nullptr;
close=nullptr;
}
std::string MmlMfenced::kind()const{return "mfenced";}
}
