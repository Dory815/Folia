// Generated MathJax 4.1.3 source bindings; Apache-2.0.
// ts/core/MmlTree/MmlNodes/mmultiscripts.ts SHA256 bb8d8d2180ef483b86b0b1570b919ba424ed54db1f30ef6e0e697cfab0bf5cf7
module folia.math.core.mml_tree.mml_nodes.mmultiscripts;
import folia.math.core.mml_tree.multiscript_host;
import std;
namespace folia::math::mml {
namespace {PropertyList MmlMmultiscriptsDefaults=host::arithmeticCopy(MmlMsubsup::defaults());}
PropertyList& MmlMmultiscripts::defaults(){return MmlMmultiscriptsDefaults;}
std::string MmlMmultiscripts::kind()const{return "mmultiscripts";}
double MmlMmultiscripts::arity()const{return 1.0;}
namespace {PropertyList MmlMprescriptsDefaults=host::arithmeticCopy(AbstractMmlNode::defaults());}
PropertyList& MmlMprescripts::defaults(){return MmlMprescriptsDefaults;}
std::string MmlMprescripts::kind()const{return "mprescripts";}
double MmlMprescripts::arity()const{return 0.0;}
namespace {PropertyList MmlNoneDefaults=host::arithmeticCopy(AbstractMmlNode::defaults());}
PropertyList& MmlNone::defaults(){return MmlNoneDefaults;}
std::string MmlNone::kind()const{return "none";}
double MmlNone::arity()const{return 0.0;}
}
