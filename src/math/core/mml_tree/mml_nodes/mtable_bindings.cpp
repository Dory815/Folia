// Generated MathJax 4.1.3 source bindings; Apache-2.0.
// ts/core/MmlTree/MmlNodes/mtable.ts SHA256 8c701c9e8e55565c7ea905bd2d0e23e8a433547c5ed11bfb868f74e3089a3bda
module folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.table_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlMtableDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlNode::defaults());
value.setOwn("align","axis");
value.setOwn("rowalign","baseline");
value.setOwn("columnalign","center");
value.setOwn("groupalign","{left}");
value.setOwn("alignmentscope",true);
value.setOwn("columnwidth","auto");
value.setOwn("width","auto");
value.setOwn("rowspacing","1ex");
value.setOwn("columnspacing",".8em");
value.setOwn("rowlines","none");
value.setOwn("columnlines","none");
value.setOwn("frame","none");
value.setOwn("framespacing","0.4em 0.5ex");
value.setOwn("equalrows",false);
value.setOwn("equalcolumns",false);
value.setOwn("displaystyle",false);
value.setOwn("side","right");
value.setOwn("minlabelspacing","0.8em");
value.setOwn("data-break-align","top");
return value;}();}
PropertyList& MmlMtable::defaults(){return MmlMtableDefaults;}
void MmlMtable::initializeClassFields(){propertyOwner(PropertyList{{"useHeight",true}});initializeTexClass(TEXCLASS.ORD);}
std::string MmlMtable::kind()const{return "mtable";}
bool MmlMtable::linebreakContainer()const{return true;}
std::string MmlMtable::linebreakAlign()const{return "";}
}
