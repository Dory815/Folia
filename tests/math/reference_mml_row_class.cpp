// Root generated from pinned original mrow callback observations.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_node_host;
import folia.math.core.mml_tree.mml_nodes.mrow;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;namespace h=host;
bool active=false;std::string mode;std::vector<std::string>trace;MmlNode*row=nullptr,*a=nullptr,*b=nullptr;Children captured;
void note(std::string s){if(active)trace.push_back(s);}std::string tag(NodeValue n){if(n.kind()==tree::ChildKind::Undefined)return "U";if(n.kind()==tree::ChildKind::Null)return "N";auto*p=n.nodeOrNull();return p==row?"row":p==a?"a":p==b?"b":"prev";}
struct Child:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "row-child";}NodeValue setTeXclass(NodeValue prev)override{note("child/"+tag(this)+"/"+tag(prev));if(this==a){if(mode=="empty")row->childNodes=std::make_shared<tree::ChildArray<MmlNode>>();if(mode=="null")row->childNodes.reset();if(mode=="rebind"){row->childNodes=std::make_shared<tree::ChildArray<MmlNode>>();row->childNodes->push_back(b);}if(mode=="grow")captured->push_back(b);if(mode=="throw")throw Error("WitnessError",u"witness");if(mode=="return-prev")return prev;}return this;}};
struct Row:MmlMrow{using MmlMrow::MmlMrow;Property getProperty(std::string_view name)const override{note("property/"+std::string(name));return MmlMrow::getProperty(name);}void getPrevClass(NodeValue prev)override{note("previous/"+tag(prev));MmlMrow::getPrevClass(prev);}void updateTeXclass(NodeValue child)override{note("update/"+tag(child));MmlMrow::updateTeXclass(child);}};
struct Case{bool delimited,previous;std::string_view mode,expected;};
std::string state(MmlNode*n){return h::debugText(h::string(n->texClass()))+","+h::debugText(h::string(n->prevClass()))+","+h::debugText(h::string(n->prevLevel()));}
std::string array(Children x){if(!x)return "N";std::string s;for(std::size_t j=0;j<x->size();++j){if(j)s+=',';s+=tag(x->get(j));}return s;}
std::string run(const Case&i){active=false;trace.clear();mode=i.mode;MmlFactory f;f.setNodeClass("row-child",classBinding<Child>());f.setNodeClass("mrow",classBinding<Row>());row=f.create("mrow");a=f.create("row-child");b=f.create("row-child");auto*previous=f.create("mi");previous->setTexClass(1);row->childNodes=std::make_shared<tree::ChildArray<MmlNode>>();row->childNodes->push_back(a);captured=row->childNodes;a->setTexClass(2);b->setTexClass(3);a->setPrevClass(4);a->setPrevLevel(5);row->setTexClass(PropertyNull{});if(i.delimited)row->setProperty("open",std::string{});
std::string error="ok",result="THREW";active=true;try{result=tag(row->setTeXclass(i.previous?NodeValue(previous):NodeValue(PropertyNull{})));}catch(const Error&e){error=e.name+"|"+h::debugText(e.message);}active=false;std::string t;for(std::size_t j=0;j<trace.size();++j){if(j)t+=',';t+=trace[j];}return error+"|"+result+"|"+state(row)+"|"+state(a)+"|"+state(b)+"|"+array(row->childNodes)+"|"+array(captured)+"|"+t;}
suite rowClass=[] {"math_port_row_class_callbacks_source"_test=[] {const Case cases[]={
{false,false,"","ok|a|2,4,5|2,null,null|3,null,null|a|a|property/open,property/close,child/a/N,update/a"},
{false,false,"empty","ok|a|null,null,null|2,4,5|3,null,null||a|property/open,property/close,child/a/N"},
{false,false,"null","TypeError|Cannot read properties of null (reading '0')|THREW|null,null,null|2,4,5|3,null,null|N|a|property/open,property/close,child/a/N"},
{false,false,"rebind","ok|a|3,null,null|2,4,5|3,null,null|b|a|property/open,property/close,child/a/N,update/b"},
{false,false,"grow","ok|b|2,4,5|2,null,null|3,null,null|a,b|a,b|property/open,property/close,child/a/N,child/b/a,update/a"},
{false,false,"return-prev","ok|N|2,4,5|2,null,null|3,null,null|a|a|property/open,property/close,child/a/N,update/a"},
{false,false,"throw","WitnessError|witness|THREW|null,null,null|2,4,5|3,null,null|a|a|property/open,property/close,child/a/N"},
{false,true,"","ok|a|2,4,5|2,null,null|3,null,null|a|a|property/open,property/close,child/a/prev,update/a"},
{false,true,"empty","ok|a|null,null,null|2,4,5|3,null,null||a|property/open,property/close,child/a/prev"},
{false,true,"null","TypeError|Cannot read properties of null (reading '0')|THREW|null,null,null|2,4,5|3,null,null|N|a|property/open,property/close,child/a/prev"},
{false,true,"rebind","ok|a|3,null,null|2,4,5|3,null,null|b|a|property/open,property/close,child/a/prev,update/b"},
{false,true,"grow","ok|b|2,4,5|2,null,null|3,null,null|a,b|a,b|property/open,property/close,child/a/prev,child/b/a,update/a"},
{false,true,"return-prev","ok|prev|2,4,5|2,null,null|3,null,null|a|a|property/open,property/close,child/a/prev,update/a"},
{false,true,"throw","WitnessError|witness|THREW|null,null,null|2,4,5|3,null,null|a|a|property/open,property/close,child/a/prev"},
{true,false,"","ok|row|7,null,null|2,4,5|3,null,null|a|a|property/open,previous/N,child/a/N"},
{true,false,"empty","ok|row|7,null,null|2,4,5|3,null,null||a|property/open,previous/N,child/a/N"},
{true,false,"null","ok|row|7,null,null|2,4,5|3,null,null|N|a|property/open,previous/N,child/a/N"},
{true,false,"rebind","ok|row|7,null,null|2,4,5|3,null,null|b|a|property/open,previous/N,child/a/N"},
{true,false,"grow","ok|row|7,null,null|2,4,5|3,null,null|a,b|a,b|property/open,previous/N,child/a/N,child/b/a"},
{true,false,"return-prev","ok|row|7,null,null|2,4,5|3,null,null|a|a|property/open,previous/N,child/a/N"},
{true,false,"throw","WitnessError|witness|THREW|null,null,null|2,4,5|3,null,null|a|a|property/open,previous/N,child/a/N"},
{true,true,"","ok|row|7,1,0|2,4,5|3,null,null|a|a|property/open,previous/prev,child/a/N"},
{true,true,"empty","ok|row|7,1,0|2,4,5|3,null,null||a|property/open,previous/prev,child/a/N"},
{true,true,"null","ok|row|7,1,0|2,4,5|3,null,null|N|a|property/open,previous/prev,child/a/N"},
{true,true,"rebind","ok|row|7,1,0|2,4,5|3,null,null|b|a|property/open,previous/prev,child/a/N"},
{true,true,"grow","ok|row|7,1,0|2,4,5|3,null,null|a,b|a,b|property/open,previous/prev,child/a/N,child/b/a"},
{true,true,"return-prev","ok|row|7,1,0|2,4,5|3,null,null|a|a|property/open,previous/prev,child/a/N"},
{true,true,"throw","WitnessError|witness|THREW|null,1,0|2,4,5|3,null,null|a|a|property/open,previous/prev,child/a/N"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.delimited<<i.previous<<i.mode;};};}
