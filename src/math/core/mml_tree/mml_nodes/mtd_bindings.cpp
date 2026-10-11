// Generated MathJax 4.1.3 source bindings; Apache-2.0.
// ts/core/MmlTree/MmlNodes/mtd.ts SHA256 8b888aec4069818407079e84c42247495e04bd93ff52a3fd28bff65ca7635cfb
module folia.math.core.mml_tree.mml_nodes.mtd;
import folia.math.core.mml_tree.table_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlMtdDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlBaseNode::defaults());
value.setOwn("rowspan",1.0);
value.setOwn("columnspan",1.0);
value.setOwn("rowalign","_inherit_");
value.setOwn("columnalign","_inherit_");
value.setOwn("groupalign","_inherit_");
value.setOwn("data-vertical-align","top");
return value;}();}
PropertyList& MmlMtd::defaults(){return MmlMtdDefaults;}
std::string MmlMtd::kind()const{return "mtd";}
double MmlMtd::arity()const{return -1.0;}
bool MmlMtd::linebreakContainer()const{return true;}
std::string MmlMtd::linebreakAlign()const{return "columnalign";}
}
