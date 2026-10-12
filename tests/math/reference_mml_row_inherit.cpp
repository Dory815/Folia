// Root generated original row child capture before virtual kind.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_node_host;
import folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.mml_nodes.mtd;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;namespace h=host;bool active=false,armed=false,display=false;std::string mode;std::vector<std::string>trace;MmlNode*late=nullptr;struct Row:MmlMtr{using MmlMtr::MmlMtr;std::string kind()const override{if(active&&armed){trace.push_back("kind");if(mode=="swap")childNodes->set(0,late);else if(mode=="null")childNodes->set(0,PropertyNull{});else childNodes->erase(0);}return "mtr";}AttributeList addInheritedAttributes(Property a,Property b)override{auto result=MmlMtr::addInheritedAttributes(a,b);armed=true;return result;}void hook(){MmlMtr::setChildInheritedAttributes(PropertyList{},display,2,std::string("raw"));}};struct Cell:MmlMtd{using MmlMtd::MmlMtd;std::string label;void inheritAttributes(Property,Property d,Property l,Property p)override{trace.push_back("inherit/"+label+"/"+h::debugText(h::string(d))+","+h::debugText(h::string(l))+","+h::debugText(h::string(p)));}};
struct Case{bool display;std::string_view mode,expected;};std::string run(const Case&i){active=false;armed=false;trace.clear();display=i.display;mode=i.mode;MmlFactory f;f.setNodeClass("mtr",classBinding<Row>());f.setNodeClass("mtd",classBinding<Cell>());auto*row=dynamic_cast<Row*>(f.create("mtr"));auto*first=dynamic_cast<Cell*>(f.create("mtd"));auto*last=dynamic_cast<Cell*>(f.create("mtd"));late=last;first->label="first";last->label="late";row->childNodes=std::make_shared<tree::ChildArray<MmlNode>>();row->childNodes->push_back(first);row->attributes()->set("columnalign","center");row->attributes()->set("rowalign","baseline");row->attributes()->set("data-break-align","top");std::string error="ok";active=true;try{row->hook();}catch(const Error&e){error=e.name+"|"+h::debugText(e.message);}active=false;auto value=row->childNodes->get(0);std::string tag=value.kind()==tree::ChildKind::Undefined?"U":value.kind()==tree::ChildKind::Null?"N":dynamic_cast<Cell*>(value.nodeOrNull())->label;std::string events;for(const auto&s:trace){if(!events.empty())events+=',';events+=s;}return error+"|"+tag+"|"+events;}
suite rowInherit=[] {"math_port_row_inherit_capture_source"_test=[] {const Case cases[]={
{false,"swap","ok|late|kind,inherit/first/false,2,raw"},
{false,"null","ok|N|kind,inherit/first/false,2,raw"},
{false,"hole","ok|U|kind,inherit/first/false,2,raw"},
{true,"swap","ok|late|kind,inherit/first/true,2,raw"},
{true,"null","ok|N|kind,inherit/first/true,2,raw"},
{true,"hole","ok|U|kind,inherit/first/true,2,raw"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.display<<i.mode;};};}
