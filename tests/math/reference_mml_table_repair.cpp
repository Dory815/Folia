// Root generated from original table/row inheritance repair callbacks.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_node_host;
import folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.mml_nodes.mtd;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;namespace h=host;
bool active=false;std::string mode;std::vector<std::string>trace;std::vector<MmlNode*>nodes;MmlNode*target=nullptr;Children captured;
std::string tag(NodeValue n){if(n.kind()==tree::ChildKind::Null)return "N";if(n.kind()==tree::ChildKind::Undefined)return "U";auto it=std::find(nodes.begin(),nodes.end(),n.nodeOrNull());return it==nodes.end()?"-1":std::to_string(it-nodes.begin());}void note(std::string s){if(active)trace.push_back(s);}void fail(){throw Error("WitnessError",u"witness");}
template<class Base>struct Probe:Base{using Base::Base;void hook(){Base::setChildInheritedAttributes(PropertyList{},true,2,std::string("prime"));}void inheritAttributes(Property,Property d,Property l,Property p)override{note("inherit/"+tag(this)+"/"+h::debugText(h::string(d))+","+h::debugText(h::string(l))+","+h::debugText(h::string(p)));if(mode=="inherit-throw")fail();}NodeValue replaceChild(NodeValue n,NodeValue o)override{note("replace/"+tag(n)+"/"+tag(o));if(mode=="replace-throw")fail();auto result=Base::replaceChild(n,o);return mode=="replace-null"?NodeValue(PropertyNull{}):result;}NodeValue appendChild(NodeValue n)override{note("append/"+tag(this)+"/"+tag(n));if(active&&mode=="append-throw")fail();auto result=Base::appendChild(n);return active&&mode=="append-null"?NodeValue(PropertyNull{}):result;}};
using Table=Probe<MmlMtable>;using Row=Probe<MmlMtr>;using Cell=Probe<MmlMtd>;
struct Factory:MmlFactory{MmlNode*create(std::string_view k,tree::FactoryArgs args)override{note("factory/"+std::string(k)+"/"+std::to_string(args.size()));if(active&&(k=="mtr"||k=="mtd")){if(mode=="factory-throw")fail();if(mode=="factory-null")return nullptr;}auto*n=MmlFactory::create(k,args);if(n&&std::find(nodes.begin(),nodes.end(),n)==nodes.end())nodes.push_back(n);return n;}};
std::string array(Children a){if(!a)return "N";std::string result;for(std::size_t j=0;j<a->size();++j){if(j)result+=',';result+=tag(a->get(j));}return result;}
struct Case{std::string_view kind,shape,mode,expected;};std::string run(const Case&i){active=false;trace.clear();nodes.clear();mode=i.mode;Factory f;f.setNodeClass("mtable",classBinding<Table>());f.setNodeClass("mtr",classBinding<Row>());f.setNodeClass("mtd",classBinding<Cell>());target=f.create(i.kind=="table"?"mtable":"mtr",{});auto slots=std::make_shared<tree::ChildArray<MmlNode>>();std::size_t start=0;while(start<=i.shape.size()){auto end=i.shape.find('-',start);auto word=i.shape.substr(start,end==std::string_view::npos?i.shape.size()-start:end-start);auto*n=f.create(word=="valid"?(i.kind=="table"?"mtr":"mtd"):"mi",{});slots->push_back(n);n->setParent(target);if(end==std::string_view::npos)break;start=end+1;}target->childNodes=slots;captured=slots;target->attributes()->set("columnalign","center");target->attributes()->set("rowalign","baseline");target->attributes()->set("data-break-align","top");std::string error="ok";active=true;try{if(i.kind=="table")dynamic_cast<Table*>(target)->hook();else dynamic_cast<Row*>(target)->hook();}catch(const Error&e){error=e.name+"|"+h::debugText(e.message);}active=false;
std::string state;for(auto*n:nodes){if(!state.empty())state+=';';state+=n->kind()+"/"+tag(n->getParent())+"/"+array(n->childNodes);}std::string events;for(const auto&s:trace){if(!events.empty())events+=',';events+=s;}return error+"|"+state+"|captured/"+array(captured)+"|"+events;}
suite repair=[] {"math_port_table_repair_callbacks_source"_test=[] {const Case cases[]={
{"table","mi","","ok|mtable/N/2;mi/2/;mtr/0/1|captured/2|factory/mtr/0,replace/2/1,append/2/1,inherit/2/false,2,false"},
{"table","mi","factory-null","TypeError|Cannot set properties of null (setting 'parent')|mtable/N/N;mi/0/|captured/N|factory/mtr/0,replace/N/1"},
{"table","mi","factory-throw","WitnessError|witness|mtable/N/1;mi/0/|captured/1|factory/mtr/0"},
{"table","mi","replace-throw","WitnessError|witness|mtable/N/1;mi/0/;mtr/N/|captured/1|factory/mtr/0,replace/2/1"},
{"table","mi","replace-null","TypeError|Cannot read properties of null (reading 'appendChild')|mtable/N/2;mi/N/;mtr/0/|captured/2|factory/mtr/0,replace/2/1"},
{"table","mi","append-throw","WitnessError|witness|mtable/N/2;mi/N/;mtr/0/|captured/2|factory/mtr/0,replace/2/1,append/2/1"},
{"table","mi","append-null","ok|mtable/N/2;mi/2/;mtr/0/1|captured/2|factory/mtr/0,replace/2/1,append/2/1,inherit/2/false,2,false"},
{"table","mi","inherit-throw","WitnessError|witness|mtable/N/2;mi/2/;mtr/0/1|captured/2|factory/mtr/0,replace/2/1,append/2/1,inherit/2/false,2,false"},
{"table","mi-mi","","ok|mtable/N/3,4;mi/3/;mi/4/;mtr/0/1;mtr/0/2|captured/3,4|factory/mtr/0,replace/3/1,append/3/1,factory/mtr/0,replace/4/2,append/4/2,inherit/3/false,2,false,inherit/4/false,2,false"},
{"table","mi-mi","factory-null","TypeError|Cannot set properties of null (setting 'parent')|mtable/N/N,2;mi/0/;mi/0/|captured/N,2|factory/mtr/0,replace/N/1"},
{"table","mi-mi","factory-throw","WitnessError|witness|mtable/N/1,2;mi/0/;mi/0/|captured/1,2|factory/mtr/0"},
{"table","mi-mi","replace-throw","WitnessError|witness|mtable/N/1,2;mi/0/;mi/0/;mtr/N/|captured/1,2|factory/mtr/0,replace/3/1"},
{"table","mi-mi","replace-null","TypeError|Cannot read properties of null (reading 'appendChild')|mtable/N/3,2;mi/N/;mi/0/;mtr/0/|captured/3,2|factory/mtr/0,replace/3/1"},
{"table","mi-mi","append-throw","WitnessError|witness|mtable/N/3,2;mi/N/;mi/0/;mtr/0/|captured/3,2|factory/mtr/0,replace/3/1,append/3/1"},
{"table","mi-mi","append-null","ok|mtable/N/3,4;mi/3/;mi/4/;mtr/0/1;mtr/0/2|captured/3,4|factory/mtr/0,replace/3/1,append/3/1,factory/mtr/0,replace/4/2,append/4/2,inherit/3/false,2,false,inherit/4/false,2,false"},
{"table","mi-mi","inherit-throw","WitnessError|witness|mtable/N/3,4;mi/3/;mi/4/;mtr/0/1;mtr/0/2|captured/3,4|factory/mtr/0,replace/3/1,append/3/1,factory/mtr/0,replace/4/2,append/4/2,inherit/3/false,2,false"},
{"table","mi-valid-mi","","ok|mtable/N/4,2,5;mi/4/;mtr/0/;mi/5/;mtr/0/1;mtr/0/3|captured/4,2,5|factory/mtr/0,replace/4/1,append/4/1,factory/mtr/0,replace/5/3,append/5/3,inherit/4/false,2,false,inherit/2/false,2,false,inherit/5/false,2,false"},
{"table","mi-valid-mi","factory-null","TypeError|Cannot set properties of null (setting 'parent')|mtable/N/N,2,3;mi/0/;mtr/0/;mi/0/|captured/N,2,3|factory/mtr/0,replace/N/1"},
{"table","mi-valid-mi","factory-throw","WitnessError|witness|mtable/N/1,2,3;mi/0/;mtr/0/;mi/0/|captured/1,2,3|factory/mtr/0"},
{"table","mi-valid-mi","replace-throw","WitnessError|witness|mtable/N/1,2,3;mi/0/;mtr/0/;mi/0/;mtr/N/|captured/1,2,3|factory/mtr/0,replace/4/1"},
{"table","mi-valid-mi","replace-null","TypeError|Cannot read properties of null (reading 'appendChild')|mtable/N/4,2,3;mi/N/;mtr/0/;mi/0/;mtr/0/|captured/4,2,3|factory/mtr/0,replace/4/1"},
{"table","mi-valid-mi","append-throw","WitnessError|witness|mtable/N/4,2,3;mi/N/;mtr/0/;mi/0/;mtr/0/|captured/4,2,3|factory/mtr/0,replace/4/1,append/4/1"},
{"table","mi-valid-mi","append-null","ok|mtable/N/4,2,5;mi/4/;mtr/0/;mi/5/;mtr/0/1;mtr/0/3|captured/4,2,5|factory/mtr/0,replace/4/1,append/4/1,factory/mtr/0,replace/5/3,append/5/3,inherit/4/false,2,false,inherit/2/false,2,false,inherit/5/false,2,false"},
{"table","mi-valid-mi","inherit-throw","WitnessError|witness|mtable/N/4,2,5;mi/4/;mtr/0/;mi/5/;mtr/0/1;mtr/0/3|captured/4,2,5|factory/mtr/0,replace/4/1,append/4/1,factory/mtr/0,replace/5/3,append/5/3,inherit/4/false,2,false"},
{"table","valid","","ok|mtable/N/1;mtr/0/|captured/1|inherit/1/false,2,false"},
{"table","valid","factory-null","ok|mtable/N/1;mtr/0/|captured/1|inherit/1/false,2,false"},
{"table","valid","factory-throw","ok|mtable/N/1;mtr/0/|captured/1|inherit/1/false,2,false"},
{"table","valid","replace-throw","ok|mtable/N/1;mtr/0/|captured/1|inherit/1/false,2,false"},
{"table","valid","replace-null","ok|mtable/N/1;mtr/0/|captured/1|inherit/1/false,2,false"},
{"table","valid","append-throw","ok|mtable/N/1;mtr/0/|captured/1|inherit/1/false,2,false"},
{"table","valid","append-null","ok|mtable/N/1;mtr/0/|captured/1|inherit/1/false,2,false"},
{"table","valid","inherit-throw","WitnessError|witness|mtable/N/1;mtr/0/|captured/1|inherit/1/false,2,false"},
{"row","mi","","ok|mtr/N/3;mi/2/;inferredMrow/3/1;mtd/0/2|captured/3|factory/mtd/0,factory/inferredMrow/0,replace/3/1,append/3/1,inherit/3/true,2,prime"},
{"row","mi","factory-null","TypeError|Cannot set properties of null (setting 'parent')|mtr/N/N;mi/0/|captured/N|factory/mtd/0,replace/N/1"},
{"row","mi","factory-throw","WitnessError|witness|mtr/N/1;mi/0/|captured/1|factory/mtd/0"},
{"row","mi","replace-throw","WitnessError|witness|mtr/N/1;mi/0/;inferredMrow/3/;mtd/N/2|captured/1|factory/mtd/0,factory/inferredMrow/0,replace/3/1"},
{"row","mi","replace-null","TypeError|Cannot read properties of null (reading 'appendChild')|mtr/N/3;mi/N/;inferredMrow/3/;mtd/0/2|captured/3|factory/mtd/0,factory/inferredMrow/0,replace/3/1"},
{"row","mi","append-throw","WitnessError|witness|mtr/N/3;mi/N/;inferredMrow/3/;mtd/0/2|captured/3|factory/mtd/0,factory/inferredMrow/0,replace/3/1,append/3/1"},
{"row","mi","append-null","ok|mtr/N/3;mi/2/;inferredMrow/3/1;mtd/0/2|captured/3|factory/mtd/0,factory/inferredMrow/0,replace/3/1,append/3/1,inherit/3/true,2,prime"},
{"row","mi","inherit-throw","WitnessError|witness|mtr/N/3;mi/2/;inferredMrow/3/1;mtd/0/2|captured/3|factory/mtd/0,factory/inferredMrow/0,replace/3/1,append/3/1,inherit/3/true,2,prime"},
{"row","mi-mi","","ok|mtr/N/4,6;mi/3/;mi/5/;inferredMrow/4/1;mtd/0/3;inferredMrow/6/2;mtd/0/5|captured/4,6|factory/mtd/0,factory/inferredMrow/0,replace/4/1,append/4/1,factory/mtd/0,factory/inferredMrow/0,replace/6/2,append/6/2,inherit/4/true,2,prime,inherit/6/true,2,prime"},
{"row","mi-mi","factory-null","TypeError|Cannot set properties of null (setting 'parent')|mtr/N/N,2;mi/0/;mi/0/|captured/N,2|factory/mtd/0,replace/N/1"},
{"row","mi-mi","factory-throw","WitnessError|witness|mtr/N/1,2;mi/0/;mi/0/|captured/1,2|factory/mtd/0"},
{"row","mi-mi","replace-throw","WitnessError|witness|mtr/N/1,2;mi/0/;mi/0/;inferredMrow/4/;mtd/N/3|captured/1,2|factory/mtd/0,factory/inferredMrow/0,replace/4/1"},
{"row","mi-mi","replace-null","TypeError|Cannot read properties of null (reading 'appendChild')|mtr/N/4,2;mi/N/;mi/0/;inferredMrow/4/;mtd/0/3|captured/4,2|factory/mtd/0,factory/inferredMrow/0,replace/4/1"},
{"row","mi-mi","append-throw","WitnessError|witness|mtr/N/4,2;mi/N/;mi/0/;inferredMrow/4/;mtd/0/3|captured/4,2|factory/mtd/0,factory/inferredMrow/0,replace/4/1,append/4/1"},
{"row","mi-mi","append-null","ok|mtr/N/4,6;mi/3/;mi/5/;inferredMrow/4/1;mtd/0/3;inferredMrow/6/2;mtd/0/5|captured/4,6|factory/mtd/0,factory/inferredMrow/0,replace/4/1,append/4/1,factory/mtd/0,factory/inferredMrow/0,replace/6/2,append/6/2,inherit/4/true,2,prime,inherit/6/true,2,prime"},
{"row","mi-mi","inherit-throw","WitnessError|witness|mtr/N/4,6;mi/3/;mi/5/;inferredMrow/4/1;mtd/0/3;inferredMrow/6/2;mtd/0/5|captured/4,6|factory/mtd/0,factory/inferredMrow/0,replace/4/1,append/4/1,factory/mtd/0,factory/inferredMrow/0,replace/6/2,append/6/2,inherit/4/true,2,prime"},
{"row","mi-valid-mi","","ok|mtr/N/6,3,8;mi/5/;inferredMrow/3/;mtd/0/2;mi/7/;inferredMrow/6/1;mtd/0/5;inferredMrow/8/4;mtd/0/7|captured/6,3,8|factory/mtd/0,factory/inferredMrow/0,replace/6/1,append/6/1,factory/mtd/0,factory/inferredMrow/0,replace/8/4,append/8/4,inherit/6/true,2,prime,inherit/3/true,2,prime,inherit/8/true,2,prime"},
{"row","mi-valid-mi","factory-null","TypeError|Cannot set properties of null (setting 'parent')|mtr/N/N,3,4;mi/0/;inferredMrow/3/;mtd/0/2;mi/0/|captured/N,3,4|factory/mtd/0,replace/N/1"},
{"row","mi-valid-mi","factory-throw","WitnessError|witness|mtr/N/1,3,4;mi/0/;inferredMrow/3/;mtd/0/2;mi/0/|captured/1,3,4|factory/mtd/0"},
{"row","mi-valid-mi","replace-throw","WitnessError|witness|mtr/N/1,3,4;mi/0/;inferredMrow/3/;mtd/0/2;mi/0/;inferredMrow/6/;mtd/N/5|captured/1,3,4|factory/mtd/0,factory/inferredMrow/0,replace/6/1"},
{"row","mi-valid-mi","replace-null","TypeError|Cannot read properties of null (reading 'appendChild')|mtr/N/6,3,4;mi/N/;inferredMrow/3/;mtd/0/2;mi/0/;inferredMrow/6/;mtd/0/5|captured/6,3,4|factory/mtd/0,factory/inferredMrow/0,replace/6/1"},
{"row","mi-valid-mi","append-throw","WitnessError|witness|mtr/N/6,3,4;mi/N/;inferredMrow/3/;mtd/0/2;mi/0/;inferredMrow/6/;mtd/0/5|captured/6,3,4|factory/mtd/0,factory/inferredMrow/0,replace/6/1,append/6/1"},
{"row","mi-valid-mi","append-null","ok|mtr/N/6,3,8;mi/5/;inferredMrow/3/;mtd/0/2;mi/7/;inferredMrow/6/1;mtd/0/5;inferredMrow/8/4;mtd/0/7|captured/6,3,8|factory/mtd/0,factory/inferredMrow/0,replace/6/1,append/6/1,factory/mtd/0,factory/inferredMrow/0,replace/8/4,append/8/4,inherit/6/true,2,prime,inherit/3/true,2,prime,inherit/8/true,2,prime"},
{"row","mi-valid-mi","inherit-throw","WitnessError|witness|mtr/N/6,3,8;mi/5/;inferredMrow/3/;mtd/0/2;mi/7/;inferredMrow/6/1;mtd/0/5;inferredMrow/8/4;mtd/0/7|captured/6,3,8|factory/mtd/0,factory/inferredMrow/0,replace/6/1,append/6/1,factory/mtd/0,factory/inferredMrow/0,replace/8/4,append/8/4,inherit/6/true,2,prime"},
{"row","valid","","ok|mtr/N/2;inferredMrow/2/;mtd/0/1|captured/2|inherit/2/true,2,prime"},
{"row","valid","factory-null","ok|mtr/N/2;inferredMrow/2/;mtd/0/1|captured/2|inherit/2/true,2,prime"},
{"row","valid","factory-throw","ok|mtr/N/2;inferredMrow/2/;mtd/0/1|captured/2|inherit/2/true,2,prime"},
{"row","valid","replace-throw","ok|mtr/N/2;inferredMrow/2/;mtd/0/1|captured/2|inherit/2/true,2,prime"},
{"row","valid","replace-null","ok|mtr/N/2;inferredMrow/2/;mtd/0/1|captured/2|inherit/2/true,2,prime"},
{"row","valid","append-throw","ok|mtr/N/2;inferredMrow/2/;mtd/0/1|captured/2|inherit/2/true,2,prime"},
{"row","valid","append-null","ok|mtr/N/2;inferredMrow/2/;mtd/0/1|captured/2|inherit/2/true,2,prime"},
{"row","valid","inherit-throw","WitnessError|witness|mtr/N/2;inferredMrow/2/;mtd/0/1|captured/2|inherit/2/true,2,prime"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.kind<<i.shape<<i.mode;};};}
