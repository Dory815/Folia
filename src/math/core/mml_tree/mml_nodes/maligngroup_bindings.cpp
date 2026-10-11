// Generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/maligngroup.ts; Apache-2.0.
// SHA256 4d8ecdae93aaff85887ec4eb09c8821811866a9e9b56e037ac61d0b6d81bf2b1
module folia.math.core.mml_tree.mml_nodes.maligngroup;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlMaligngroupDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlLayoutNode::defaults());
value.setOwn("groupalign","_inherit_");
return value;}();}
PropertyList& MmlMaligngroup::defaults(){return MmlMaligngroupDefaults;}
std::string MmlMaligngroup::kind()const{return "maligngroup";}
bool MmlMaligngroup::isSpacelike()const{return true;}
}
