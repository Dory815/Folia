// Root generated from original embellished/coreMO spacing dispatch.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_node_host;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;namespace h=host;bool active=false,embellished=false;std::string target,mode;std::vector<std::string>trace;MmlNode*child=nullptr;void note(std::string s){if(active)trace.push_back(s);}void fail(){throw Error("WitnessError",u"witness");}
struct Child:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "spacing-child";}bool hasSpacingAttributes()override{note("child");if(mode=="child-throw")fail();return target=="true";}};
struct Probe:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "spacing-probe";}bool isEmbellished()const override{note("emb");if(mode=="emb-throw")fail();return embellished;}NodeValue coreMO()override{note("coreMO");if(mode=="coremo-throw")fail();return target=="U"?NodeValue(Missing{}):target=="N"?NodeValue(PropertyNull{}):NodeValue(child);}NodeValue core()override{note("core");return child;}};
struct Case{bool embellished;std::string_view target,mode,expected;};std::string run(const Case&i){active=false;trace.clear();embellished=i.embellished;target=i.target;mode=i.mode;MmlFactory f;f.setNodeClass("spacing-child",classBinding<Child>());f.setNodeClass("spacing-probe",classBinding<Probe>());auto*n=f.create("spacing-probe");child=f.create("spacing-child");std::string error="ok",result="THREW";active=true;try{result=n->hasSpacingAttributes()?"true":"false";}catch(const Error&e){error=e.name+"|"+h::debugText(e.message);}active=false;std::string output=error+"|"+result+"|";for(std::size_t j=0;j<trace.size();++j){if(j)output+=',';output+=trace[j];}return output;}
suite spacing=[] {"math_port_error_spacing_source"_test=[] {const Case cases[]={
{false,"U","","ok|false|emb"},
{false,"U","emb-throw","WitnessError|witness|THREW|emb"},
{false,"U","coremo-throw","ok|false|emb"},
{false,"U","child-throw","ok|false|emb"},
{false,"N","","ok|false|emb"},
{false,"N","emb-throw","WitnessError|witness|THREW|emb"},
{false,"N","coremo-throw","ok|false|emb"},
{false,"N","child-throw","ok|false|emb"},
{false,"false","","ok|false|emb"},
{false,"false","emb-throw","WitnessError|witness|THREW|emb"},
{false,"false","coremo-throw","ok|false|emb"},
{false,"false","child-throw","ok|false|emb"},
{false,"true","","ok|false|emb"},
{false,"true","emb-throw","WitnessError|witness|THREW|emb"},
{false,"true","coremo-throw","ok|false|emb"},
{false,"true","child-throw","ok|false|emb"},
{true,"U","","TypeError|Cannot read properties of undefined (reading 'hasSpacingAttributes')|THREW|emb,coreMO"},
{true,"U","emb-throw","WitnessError|witness|THREW|emb"},
{true,"U","coremo-throw","WitnessError|witness|THREW|emb,coreMO"},
{true,"U","child-throw","TypeError|Cannot read properties of undefined (reading 'hasSpacingAttributes')|THREW|emb,coreMO"},
{true,"N","","TypeError|Cannot read properties of null (reading 'hasSpacingAttributes')|THREW|emb,coreMO"},
{true,"N","emb-throw","WitnessError|witness|THREW|emb"},
{true,"N","coremo-throw","WitnessError|witness|THREW|emb,coreMO"},
{true,"N","child-throw","TypeError|Cannot read properties of null (reading 'hasSpacingAttributes')|THREW|emb,coreMO"},
{true,"false","","ok|false|emb,coreMO,child"},
{true,"false","emb-throw","WitnessError|witness|THREW|emb"},
{true,"false","coremo-throw","WitnessError|witness|THREW|emb,coreMO"},
{true,"false","child-throw","WitnessError|witness|THREW|emb,coreMO,child"},
{true,"true","","ok|true|emb,coreMO,child"},
{true,"true","emb-throw","WitnessError|witness|THREW|emb"},
{true,"true","coremo-throw","WitnessError|witness|THREW|emb,coreMO"},
{true,"true","child-throw","WitnessError|witness|THREW|emb,coreMO,child"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.embellished<<i.target<<i.mode;};};}
