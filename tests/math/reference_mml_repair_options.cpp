// Root generated original raw verification options and partial graphs.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_node_host;
import folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.mml_nodes.mmultiscripts;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;namespace h=host;bool active=false;std::vector<std::string>trace;std::vector<MmlNode*>nodes;Property original;void note(std::string s){if(active)trace.push_back(s);}template<class Base>struct Probe:Base{using Base::Base;void hook(Property o){Base::verifyChildren(o);}};struct Leaf:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "verify-leaf";}void verifyTree(Property o=Missing{})override{note(std::string("leaf/")+(o==original?"original":"other"));}};struct Factory:MmlFactory{MmlNode*create(std::string_view k,tree::FactoryArgs args)override{note("factory/"+std::string(k)+"/"+std::to_string(args.size()));auto*n=MmlFactory::create(k,args);if(n&&std::find(nodes.begin(),nodes.end(),n)==nodes.end())nodes.push_back(n);return n;}};
std::string tag(NodeValue n){if(n.kind()==tree::ChildKind::Undefined)return "U";if(n.kind()==tree::ChildKind::Null)return "N";auto it=std::find(nodes.begin(),nodes.end(),n.nodeOrNull());return it==nodes.end()?"-1":std::to_string(it-nodes.begin());}struct Case{std::string_view kind;bool child;std::string_view option,expected;};std::string run(const Case&i){active=false;trace.clear();nodes.clear();Factory f;f.setNodeClass("mtable",classBinding<Probe<MmlMtable>>());f.setNodeClass("mtr",classBinding<Probe<MmlMtr>>());f.setNodeClass("mmultiscripts",classBinding<Probe<MmlMmultiscripts>>());f.setNodeClass("verify-leaf",classBinding<Leaf>());auto*target=f.create(i.kind=="table"?"mtable":i.kind=="row"?"mtr":"mmultiscripts",{});auto*leaf=f.create("verify-leaf",{});target->childNodes=std::make_shared<tree::ChildArray<MmlNode>>();if(i.child){target->childNodes->push_back(leaf);leaf->setParent(target);}original=i.option=="U"?Property(Missing{}):i.option=="N"?Property(PropertyNull{}):Property(PropertyList{{"fixMtables",true},{"fixMmultiscripts",true}});std::string error="ok";active=true;try{if(i.kind=="table")dynamic_cast<Probe<MmlMtable>*>(target)->hook(original);else if(i.kind=="row")dynamic_cast<Probe<MmlMtr>*>(target)->hook(original);else dynamic_cast<Probe<MmlMmultiscripts>*>(target)->hook(original);}catch(const Error&e){error=e.name+"|"+h::debugText(e.message);}active=false;std::string graph;for(auto*n:nodes){if(!graph.empty())graph+=';';graph+=n->kind()+"/"+tag(n->getParent())+"/";if(!n->childNodes)graph+='N';else for(std::size_t j=0;j<n->childNodes->size();++j){if(j)graph+=',';graph+=tag(n->childNodes->get(j));}}std::string events;for(const auto&s:trace){if(!events.empty())events+=',';events+=s;}return error+"|"+graph+"|"+events;}
suite repairOptions=[] {"math_port_repair_raw_options_source"_test=[] {const Case cases[]={
{"table",false,"U","ok|mtable/N/;verify-leaf/N/|"},
{"table",false,"N","ok|mtable/N/;verify-leaf/N/|"},
{"table",false,"object","ok|mtable/N/;verify-leaf/N/|"},
{"table",true,"U","TypeError|Cannot read properties of undefined (reading 'fixMtables')|mtable/N/2;verify-leaf/3/;mtr/0/4;inferredMrow/4/1;mtd/2/3|factory/mtr/0,factory/mtd/2,factory/inferredMrow/0"},
{"table",true,"N","TypeError|Cannot read properties of null (reading 'fixMtables')|mtable/N/2;verify-leaf/3/;mtr/0/4;inferredMrow/4/1;mtd/2/3|factory/mtr/0,factory/mtd/2,factory/inferredMrow/0"},
{"table",true,"object","ok|mtable/N/2;verify-leaf/3/;mtr/0/4;inferredMrow/4/1;mtd/2/3|factory/mtr/0,factory/mtd/2,factory/inferredMrow/0,leaf/original"},
{"row",false,"U","ok|mtr/N/;verify-leaf/N/|"},
{"row",false,"N","ok|mtr/N/;verify-leaf/N/|"},
{"row",false,"object","ok|mtr/N/;verify-leaf/N/|"},
{"row",true,"U","TypeError|Cannot read properties of undefined (reading 'fixMtables')|mtr/N/3;verify-leaf/2/;inferredMrow/3/1;mtd/0/2|factory/mtd/0,factory/inferredMrow/0"},
{"row",true,"N","TypeError|Cannot read properties of null (reading 'fixMtables')|mtr/N/3;verify-leaf/2/;inferredMrow/3/1;mtd/0/2|factory/mtd/0,factory/inferredMrow/0"},
{"row",true,"object","ok|mtr/N/3;verify-leaf/2/;inferredMrow/3/1;mtd/0/2|factory/mtd/0,factory/inferredMrow/0,leaf/original"},
{"multi",false,"U","TypeError|Cannot read properties of undefined (reading 'fixMmultiscripts')|mmultiscripts/N/;verify-leaf/N/|"},
{"multi",false,"N","TypeError|Cannot read properties of null (reading 'fixMmultiscripts')|mmultiscripts/N/;verify-leaf/N/|"},
{"multi",false,"object","ok|mmultiscripts/N/;verify-leaf/N/|"},
{"multi",true,"U","TypeError|Cannot read properties of undefined (reading 'fixMmultiscripts')|mmultiscripts/N/1;verify-leaf/0/|"},
{"multi",true,"N","TypeError|Cannot read properties of null (reading 'fixMmultiscripts')|mmultiscripts/N/1;verify-leaf/0/|"},
{"multi",true,"object","ok|mmultiscripts/N/1;verify-leaf/0/|leaf/original"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.kind<<i.child<<i.option;};};}
