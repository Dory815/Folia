// Generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/mpadded.ts; Apache-2.0.
// SHA256 b5aac516c32fb019ac5ace504f86312f3e0849e1faa44006567bac05815c7271
module folia.math.core.mml_tree.mml_nodes.mpadded;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlMpaddedDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlLayoutNode::defaults());
value.setOwn("width","");
value.setOwn("height","");
value.setOwn("depth","");
value.setOwn("lspace",0.0);
value.setOwn("voffset",0.0);
return value;}();}
PropertyList& MmlMpadded::defaults(){return MmlMpaddedDefaults;}
std::string MmlMpadded::kind()const{return "mpadded";}
bool MmlMpadded::linebreakContainer()const{return true;}
}
