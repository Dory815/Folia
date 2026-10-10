// Generated source fields/bindings from locked MathJax 4.1.3 mo.ts.
// Apache-2.0; see LICENSE-MathJax.txt. Root owns this file.
module folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.operator_dictionary;
import std;
namespace folia::math::mml {namespace {
PropertyList moDefaults=[] {PropertyList result;const auto base=AbstractMmlTokenNode::defaults();for(const auto&key:base.keys())result.setOwn(key,base.getOwn(key));
result.setOwn("form","infix");
result.setOwn("fence",false);
result.setOwn("separator",false);
result.setOwn("lspace","thickmathspace");
result.setOwn("rspace","thickmathspace");
result.setOwn("stretchy",false);
result.setOwn("symmetric",false);
result.setOwn("maxsize","infinity");
result.setOwn("minsize","0em");
result.setOwn("largeop",false);
result.setOwn("movablelimits",false);
result.setOwn("accent",false);
result.setOwn("linebreak","auto");
result.setOwn("lineleading","100%");
result.setOwn("linebreakstyle","before");
result.setOwn("indentalign","auto");
result.setOwn("indentshift","0");
result.setOwn("indenttarget","");
result.setOwn("indentalignfirst","indentalign");
result.setOwn("indentshiftfirst","indentshift");
result.setOwn("indentalignlast","indentalign");
result.setOwn("indentshiftlast","indentshift");
return result;}();
Property classTable=operator_dict::OPTABLE();
Property classSpacing=operator_dict::MMLSPACING();
Property primeMap=PropertyList{{"34",8243.0},{"39",8242.0},{"8216",8245.0},{"8217",8242.0},{"8218",8242.0},{"8219",8245.0},{"8220",8246.0},{"8221",8243.0},{"8222",8243.0},{"8223",8246.0}};
MoPatternHandle pseudoScriptsPattern=std::make_shared<Pattern>(makeMoPattern(MoPattern::pseudoScripts));
MoPatternHandle primesPattern=std::make_shared<Pattern>(makeMoPattern(MoPattern::primes));
MoPatternHandle opPatternPattern=std::make_shared<Pattern>(makeMoPattern(MoPattern::opPattern));
MoPatternHandle mathaccentsPattern=std::make_shared<Pattern>(makeMoPattern(MoPattern::mathaccents));
MoPatternHandle mathaccentsWithWidthPattern=std::make_shared<Pattern>(makeMoPattern(MoPattern::mathaccentsWithWidth));
}
PropertyList& MmlMo::defaults(){return moDefaults;}
Property& MmlMo::operatorTable(){return classTable;}
Property& MmlMo::mathSpacing(){return classSpacing;}
Property& MmlMo::remapPrimes(){return primeMap;}
MoPatternHandle& MmlMo::pseudoScripts(){return pseudoScriptsPattern;}
MoPatternHandle& MmlMo::primes(){return primesPattern;}
MoPatternHandle& MmlMo::opPattern(){return opPatternPattern;}
MoPatternHandle& MmlMo::mathaccents(){return mathaccentsPattern;}
MoPatternHandle& MmlMo::mathaccentsWithWidth(){return mathaccentsWithWidthPattern;}
const MoClassServices& MmlMo::constructorOperators()const{const auto*p=std::any_cast<std::shared_ptr<MoClassServices>>(&constructorStaticServices());if(!p||!*p)throw Error("TypeError",u"Missing Mo constructor static services");return **p;}
void MmlMo::initializeClassFields(){_texClass=PropertyNull{};lspace=5.0/18.0;rspace=5.0/18.0;}
std::string MmlMo::kind()const{return "mo";}
bool MmlMo::isEmbellished()const{return true;}
void MmlMo::setTexClass(Property value){_texClass=std::move(value);}
}
