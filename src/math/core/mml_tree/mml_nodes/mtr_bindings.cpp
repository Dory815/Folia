// Generated MathJax 4.1.3 source bindings; Apache-2.0.
// ts/core/MmlTree/MmlNodes/mtr.ts SHA256 ddf536d1e1a615d542052cd60db6440ad1698c0a13ff2e2307097d05c1b0ed2f
module folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.table_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlMtrDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlNode::defaults());
value.setOwn("rowalign","_inherit_");
value.setOwn("columnalign","_inherit_");
value.setOwn("groupalign","_inherit_");
value.setOwn("data-break-align","top");
return value;}();}
PropertyList& MmlMtr::defaults(){return MmlMtrDefaults;}
std::string MmlMtr::kind()const{return "mtr";}
bool MmlMtr::linebreakContainer()const{return true;}
std::string MmlMtr::linebreakAlign()const{return "";}
std::string MmlMlabeledtr::kind()const{return "mlabeledtr";}
double MmlMlabeledtr::arity()const{return 1.0;}
}
