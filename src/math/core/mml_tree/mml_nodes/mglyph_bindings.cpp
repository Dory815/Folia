// Generated MathJax 4.1.3 ts/core/MmlTree/MmlNodes/mglyph.ts; Apache-2.0.
// SHA256 2c2d336067d3970c55f3fcdafb82b55d4339a2c11d69088c8c7eff4a6d953504
module folia.math.core.mml_tree.mml_nodes.mglyph;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlMglyphDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlTokenNode::defaults());
value.setOwn("alt","");
value.setOwn("src","");
value.setOwn("index","");
value.setOwn("width","auto");
value.setOwn("height","auto");
value.setOwn("valign","0em");
return value;}();}
PropertyList& MmlMglyph::defaults(){return MmlMglyphDefaults;}
void MmlMglyph::initializeClassFields(){initializeTexClass(TEXCLASS.ORD);}
std::string MmlMglyph::kind()const{return "mglyph";}
}
