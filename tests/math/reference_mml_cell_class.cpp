// Root generated original mtd literal child-slot observations.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_node_host;
import folia.math.core.mml_tree.mml_nodes.mtd;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;namespace h=host;bool active=false;std::string mode;std::vector<std::string>trace;void note(std::string s){if(active)trace.push_back(s);}
struct Cell:MmlMtd{using MmlMtd::MmlMtd;void getPrevClass(NodeValue prev)override{note(std::string("previous/")+(prev.isNode()?"node":"N"));MmlMtd::getPrevClass(prev);}};struct Child:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "class-child";}NodeValue setTeXclass(NodeValue prev)override{note(std::string("child/")+(prev.kind()==tree::ChildKind::Null?"null":"unexpected"));if(mode=="throw")throw Error("WitnessError",u"witness");return this;}};
struct Case{bool previous;std::string_view mode,expected;};std::string run(const Case&i){active=false;trace.clear();mode=i.mode;MmlFactory f;f.setNodeClass("mtd",classBinding<Cell>());f.setNodeClass("class-child",classBinding<Child>());auto*cell=f.create("mtd");auto*child=f.create("class-child");auto*prev=f.create("mi");prev->setTexClass(3);auto a=std::make_shared<tree::ChildArray<MmlNode>>();if(mode=="null-array")a.reset();else if(mode=="hole"||mode=="prototype")a->resize(1);else if(mode=="null")a->push_back(PropertyNull{});else if(mode!="empty")a->push_back(child);if(mode=="prototype"){auto proto=std::make_shared<tree::ChildArray<MmlNode>>();proto->push_back(child);a->setPrototype(proto);}cell->childNodes=a;std::string error="ok",result="THREW";active=true;try{result=cell->setTeXclass(i.previous?NodeValue(prev):NodeValue(PropertyNull{})).nodeOrNull()==cell?"cell":"other";}catch(const Error&e){error=e.name+"|"+h::debugText(e.message);}active=false;std::string events;for(const auto&s:trace){if(!events.empty())events+=',';events+=s;}return error+"|"+result+"|"+h::debugText(h::string(cell->prevClass()))+"|"+h::debugText(h::string(cell->prevLevel()))+"|"+events;}
suite cellClass=[] {"math_port_cell_class_literal_source"_test=[] {const Case cases[]={
{false,"null-array","TypeError|Cannot read properties of null (reading '0')|THREW|null|null|previous/N"},
{false,"empty","TypeError|Cannot read properties of undefined (reading 'setTeXclass')|THREW|null|null|previous/N"},
{false,"hole","TypeError|Cannot read properties of undefined (reading 'setTeXclass')|THREW|null|null|previous/N"},
{false,"null","TypeError|Cannot read properties of null (reading 'setTeXclass')|THREW|null|null|previous/N"},
{false,"child","ok|cell|null|null|previous/N,child/null"},
{false,"prototype","ok|cell|null|null|previous/N,child/null"},
{false,"throw","WitnessError|witness|THREW|null|null|previous/N,child/null"},
{true,"null-array","TypeError|Cannot read properties of null (reading '0')|THREW|3|0|previous/node"},
{true,"empty","TypeError|Cannot read properties of undefined (reading 'setTeXclass')|THREW|3|0|previous/node"},
{true,"hole","TypeError|Cannot read properties of undefined (reading 'setTeXclass')|THREW|3|0|previous/node"},
{true,"null","TypeError|Cannot read properties of null (reading 'setTeXclass')|THREW|3|0|previous/node"},
{true,"child","ok|cell|3|0|previous/node,child/null"},
{true,"prototype","ok|cell|3|0|previous/node,child/null"},
{true,"throw","WitnessError|witness|THREW|3|0|previous/node,child/null"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.previous<<i.mode;};};}
