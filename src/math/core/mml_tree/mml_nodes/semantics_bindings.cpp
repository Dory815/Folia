// Generated MathJax 4.1.3 source bindings; Apache-2.0.
// ts/core/MmlTree/MmlNodes/semantics.ts SHA256 e609dfc3d7510a996a87804e3929cd4f53a5aa060003e351f45f1fc5dcfb6e96
module folia.math.core.mml_tree.mml_nodes.semantics;
import folia.math.core.mml_tree.arithmetic_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlSemanticsDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlBaseNode::defaults());
value.setOwn("definitionUrl",PropertyNull{});
value.setOwn("encoding",PropertyNull{});
return value;}();}
PropertyList& MmlSemantics::defaults(){return MmlSemanticsDefaults;}
std::string MmlSemantics::kind()const{return "semantics";}
double MmlSemantics::arity()const{return 1.0;}
Property MmlSemantics::notParent()const{return true;}
namespace {PropertyList MmlAnnotationXMLDefaults=[] {auto value=host::arithmeticCopy(AbstractMmlNode::defaults());
value.setOwn("definitionUrl",PropertyNull{});
value.setOwn("encoding",PropertyNull{});
value.setOwn("cd","mathmlkeys");
value.setOwn("name","");
value.setOwn("src",PropertyNull{});
return value;}();}
PropertyList& MmlAnnotationXML::defaults(){return MmlAnnotationXMLDefaults;}
std::string MmlAnnotationXML::kind()const{return "annotation-xml";}
namespace {PropertyList MmlAnnotationDefaults=[] {auto value=host::arithmeticCopy(MmlAnnotationXML::defaults());
return value;}();}
PropertyList& MmlAnnotation::defaults(){return MmlAnnotationDefaults;}
void MmlAnnotation::initializeClassFields(){propertyOwner(PropertyList{{"isChars",true}});}
std::string MmlAnnotation::kind()const{return "annotation";}
}
