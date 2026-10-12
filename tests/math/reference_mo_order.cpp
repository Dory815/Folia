// Generated from pinned actual-CJS call order and identities.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mo_host;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.operator_dictionary;
import folia.math.core.tree.node_factory;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mstyle;
import folia.math.core.mml_tree.mml_nodes.mphantom;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
namespace m=folia::math::mml;namespace h=m::host;namespace d=m::operator_dict;namespace tr=folia::math::tree;using folia::math::Property;using folia::math::PropertyArray;using folia::math::PropertyList;using folia::math::Missing;using folia::math::PropertyNull;using namespace boost::ut;namespace {
bool enabled=false;std::vector<std::string> traceLog;void note(std::string s){if(enabled)traceLog.push_back(std::move(s));}
std::string traceText(){std::string s;for(const auto&v:traceLog){s+=v;s+=" | ";}return s;}
std::u16string units(std::initializer_list<unsigned>v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}
m::Children children(std::initializer_list<m::NodeValue>v={}){auto result=std::make_shared<tr::ChildArray<m::MmlNode>>();for(auto node:v)result->push_back(node);return result;}
struct Probe:m::MmlMo{using MmlMo::MmlMo;m::NodeValue Parent()const override{note("Parent");return MmlMo::Parent();}std::shared_ptr<folia::math::Attributes>attributes()const override{note("attributes");return MmlMo::attributes();}std::u16string getText()const override{note("text");return MmlMo::getText();}Property getProperty(std::string_view k)const override{note("get/"+std::string(k));return MmlMo::getProperty(k);}void setProperty(std::string_view k,Property v)override{note("set/"+std::string(k));MmlMo::setProperty(k,std::move(v));}Property texClass()override{note("tex.read");return MmlMo::texClass();}void setTexClass(Property v)override{note("tex.write");MmlMo::setTexClass(std::move(v));}PropertyArray getForms()override{note("forms");return MmlMo::getForms();}PropertyArray handleExplicitForm(PropertyArray a)override{note("explicit");return MmlMo::handleExplicitForm(a);}
static Property&operatorTable(){note("static.table");return MmlMo::operatorTable();}
static Property&mathSpacing(){note("static.spacing");return MmlMo::mathSpacing();}
static Property&remapPrimes(){note("static.remap");return MmlMo::remapPrimes();}
static m::MoPatternHandle&opPattern(){note("static.op");return MmlMo::opPattern();}
static m::MoPatternHandle&pseudoScripts(){note("static.pseudo");return MmlMo::pseudoScripts();}
static m::MoPatternHandle&primes(){note("static.primes");return MmlMo::primes();}
static m::MoPatternHandle&mathaccents(){note("static.accent");return MmlMo::mathaccents();}
static m::MoPatternHandle&mathaccentsWithWidth(){note("static.width");return MmlMo::mathaccentsWithWidth();}
Property op(std::u16string_view s){return getOperatorDef(s);}void table(std::u16string_view s){checkOperatorTable(s);}PropertyArray forms(PropertyArray a){return handleExplicitForm(a);}void accent(std::u16string_view s){checkMathAccent(s);}void pseudo(std::u16string_view s){checkPseudoScripts(s);}void prime(std::u16string_view s){checkPrimes(s);}};
struct Dispatch:Probe{using Probe::Probe;void checkOperatorTable(std::u16string_view s)override{note("table/"+h::debugText(s));}void checkPseudoScripts(std::u16string_view s)override{note("pseudo/"+h::debugText(s));}void checkPrimes(std::u16string_view s)override{note("prime/"+h::debugText(s));}void checkMathAccent(std::u16string_view s)override{note("accent/"+h::debugText(s));}};
struct NullCore:Probe{using Probe::Probe;m::MmlNode*coreParent()override{return nullptr;}};
struct Phase:Probe{using Probe::Probe;void setChildren(m::Children c)override{note("phase/"+std::string(_texClass.missing()?"missing":h::debugText(h::string(_texClass)))+"/"+std::string(lspace.missing()?"missing":h::debugText(h::string(lspace)))+"/"+std::string(rspace.missing()?"missing":h::debugText(h::string(rspace))));Probe::setChildren(c);}};
PropertyArray changingDef;int attributeReads=0;struct AttrMutation:Probe{using Probe::Probe;std::shared_ptr<folia::math::Attributes>attributes()const override{auto a=Probe::attributes();if(enabled&&++attributeReads==3)changingDef.set(3,PropertyList{{"a",9},{"b",8}});return a;}};
struct Snapshot:m::MmlMi{using MmlMi::MmlMi;bool isEmbellished()const override{if(enabled){note("snapshot");auto*parent=getParent().requireNode("childNodes");parent->childNodes->set(1,m::NodeValue(const_cast<Snapshot*>(this)));parent->childNodes->set(2,m::NodeValue(const_cast<Snapshot*>(this)));}return false;}};
struct Atom:m::AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "TeXAtom";}};Probe*current=nullptr;struct Previous:Probe{using Probe::Probe;Property texClass()override{note("prev.tex.read");return Probe::texClass();}void setTexClass(Property v)override{note("prev.tex.write/"+h::debugText(h::string(current->prevClass())));Probe::setTexClass(std::move(v));}};
using NF=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode>classes(){return {{"probe",m::classBinding<Probe>()},{"dispatch",m::classBinding<Dispatch>()},{"nullcore",m::classBinding<NullCore>()},{"phase",m::classBinding<Phase>()},{"mo",m::classBinding<m::MmlMo>()},{"math",m::classBinding<m::MmlMath>()},{"mrow",m::classBinding<m::MmlMrow>()},{"inferredMrow",m::classBinding<m::MmlInferredMrow>()},{"mi",m::classBinding<m::MmlMi>()},{"munderover",m::classBinding<m::MmlMunderover>()},{"munder",m::classBinding<m::MmlMunder>()},{"mover",m::classBinding<m::MmlMover>()},{"text",m::classBinding<m::TextNode>()}};}
m::MmlNode*create(NF&f,std::string_view k,PropertyList a={},m::Children c=children()){return f.create(k,std::array<std::any,2>{a,c});}m::MmlNode*token(NF&f,std::string_view k,std::u16string s,PropertyList a={}){auto*t=dynamic_cast<m::TextNode*>(f.create("text"));t->setText(std::move(s));return create(f,k,a,children({t}));}Probe*mo(NF&f,std::u16string s,PropertyList a={},std::string_view k="probe"){return dynamic_cast<Probe*>(token(f,k,std::move(s),a));}
}
suite math_port_mo_order_tests=[] {
"math_port_mo_order_lookup_trace_2b"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,units({43}));
traceLog.clear();enabled=true;n->table(units({43}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
enabled=false;};
"math_port_mo_order_lookup_trace_6162"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,units({97,98}));
traceLog.clear();enabled=true;n->table(units({97,98}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","get/texClass","tex.write"})<<traceText();
enabled=false;};
"math_port_mo_order_lookup_trace_21"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,units({33}));
traceLog.clear();enabled=true;n->table(units({33}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
enabled=false;};
"math_port_mo_order_lookup_trace_f09f9880"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,units({55357,56832}));
traceLog.clear();enabled=true;n->table(units({55357,56832}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","static.table","set/noDictDef","attributes","static.op","static.spacing","get/texClass","tex.write"})<<traceText();
enabled=false;};
"math_port_mo_order_inherited_gettext_once_dispatch"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"x",{},"dispatch");
traceLog.clear();enabled=true;n->setInheritedAttributes(PropertyList{},false,2,true);enabled=false;
expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","set/texprimestyle","text","table/x","pseudo/x","prime/x","accent/x"})<<traceText();
enabled=false;};
"math_port_mo_order_phase_missing_before_initializers"_test=[] {enabled=false;traceLog.clear();
NF f(classes());
traceLog.clear();enabled=true;mo(f,u"+",{},"phase");enabled=false;
expect(traceLog==std::vector<std::string>{"phase/missing/missing/missing","attributes"})<<traceText();
enabled=false;};
"math_port_mo_order_parent_read_before_predefined_accent"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");n->setProperty("mathaccent",false);
traceLog.clear();enabled=true;n->accent(u"^");enabled=false;
expect(traceLog==std::vector<std::string>{"Parent","get/mathaccent"})<<traceText();
enabled=false;};
"math_port_mo_order_null_core_pseudo"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"'",{},"nullcore");
traceLog.clear();enabled=true;{bool caught=false;try{n->pseudo(u"'");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"static.pseudo"})<<traceText();
enabled=false;};
"math_port_mo_order_null_core_isAccent"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"'",{},"nullcore");
traceLog.clear();enabled=true;{bool caught=false;try{n->isAccent();}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{})<<traceText();
enabled=false;};
"math_port_mo_order_remap_missing_range_error"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"'");auto old=m::MmlMo::remapPrimes();m::MmlMo::remapPrimes()=PropertyList{};
traceLog.clear();enabled=true;{bool caught=false;try{n->prime(u"'");}catch(const m::Error&e){caught=e.name=="RangeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"static.primes","static.remap"})<<traceText();
expect((n->getProperty("primes")).value==Property(Missing{}).value);
m::MmlMo::remapPrimes()=old;
enabled=false;};
"math_port_mo_order_sparse_forms_keep_inherited_slots"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+",PropertyList{{"form","infix"}});PropertyArray a{"infix",Missing{},"postfix",Missing{},"prefix"};a.record().erase("1");a.record().erase("3");PropertyList proto{{"1","prefix"}};a.record().set("__proto__",proto);auto result=n->forms(a);
expect(bool((result.size())==4));
expect((result.get(0)).value==Property(h::debugText(units({105,110,102,105,120}))).value);
expect((result.get(1)).value==Property(h::debugText(units({112,114,101,102,105,120}))).value);
expect((result.get(2)).value==Property(h::debugText(units({112,111,115,116,102,105,120}))).value);
expect((result.get(3)).value==Property(h::debugText(units({112,114,101,102,105,120}))).value);
enabled=false;};
"math_port_mo_order_module_rebind_is_separate_from_class_alias"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");auto old=d::OPTABLE();auto spacing=d::MMLSPACING();d::OPTABLE()=PropertyList{{"infix",PropertyList{}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};d::MMLSPACING()=PropertyArray{};auto result=n->op(u"+");
expect(bool((result==h::get(h::get(m::MmlMo::operatorTable(),"infix"),"+"))==true));
d::OPTABLE()=old;d::MMLSPACING()=spacing;
enabled=false;};
"math_port_mo_order_class_table_live_rebinding_after_construction"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");auto old=m::MmlMo::operatorTable();m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"+",PropertyArray{7,8,6,PropertyList{{"accent","custom"}}}}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};n->table(u"+");
expect((n->_texClass).value==Property(6).value);
expect((n->attributes()->get("accent")).value==Property(h::debugText(units({99,117,115,116,111,109}))).value);
m::MmlMo::operatorTable()=old;
enabled=false;};
"math_port_mo_order_live_pattern_and_remap_after_construction"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"x");auto old=m::MmlMo::primes();auto remap=m::MmlMo::remapPrimes();m::MmlMo::primes()=std::make_shared<m::Pattern>([](std::u16string_view s){return s==u"x";});m::MmlMo::remapPrimes()=PropertyList{{"120",128512}};n->prime(u"x");
expect((n->getProperty("primes")).value==Property(h::debugText(units({55357,56832}))).value);
m::MmlMo::primes()=old;m::MmlMo::remapPrimes()=remap;
enabled=false;};
"math_port_mo_order_attribute_receiver_before_definition_slot"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("mutation",m::classBinding<AttrMutation>());auto*n=mo(f,u"+",{},"mutation");auto old=m::MmlMo::operatorTable();changingDef=PropertyArray{1,2,3,PropertyList{{"a",1},{"b",2}}};m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"+",changingDef}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};attributeReads=0;
traceLog.clear();enabled=true;n->table(u"+");enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","static.table","get/texClass","tex.write","attributes","attributes"})<<traceText();
expect((n->attributes()->getInherited("a")).value==Property(9).value);
expect((n->attributes()->getInherited("b")).value==Property(8).value);
m::MmlMo::operatorTable()=old;
enabled=false;};
"math_port_mo_order_accent_children_snapshot_before_virtual_base"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("snapshot",m::classBinding<Snapshot>());auto*n=mo(f,u"^");auto*baseNode=token(f,"snapshot",u"b");auto*over=mo(f,u"^");create(f,"munderover",{},children({baseNode,n,over}));
traceLog.clear();enabled=true;n->accent(u"^");enabled=false;
expect(traceLog==std::vector<std::string>{"Parent","get/mathaccent","snapshot","static.accent","set/mathaccent"})<<traceText();
expect((n->getProperty("mathaccent")).value==Property(true).value);
enabled=false;};
"math_port_mo_order_coreparent_actual_registered_instanceof"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");create(f,"mrow",{},children({n}));f.setNodeClass("math",m::classBinding<Probe>());
expect(bool((n->coreParent()==n)==true));
enabled=false;};
"math_port_mo_order_replacement_pattern_survives_its_own_rebinding"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"x");auto old=m::MmlMo::primes();auto remap=m::MmlMo::remapPrimes();m::MmlMo::primes()=std::make_shared<m::Pattern>([](std::u16string_view s){m::MmlMo::primes()=std::make_shared<m::Pattern>([](std::u16string_view){return false;});m::MmlMo::remapPrimes()=PropertyList{{"120",8242}};return s==u"x";});
traceLog.clear();enabled=true;n->prime(u"x");enabled=false;
expect(traceLog==std::vector<std::string>{"static.primes","static.remap","set/primes"})<<traceText();
expect((n->getProperty("primes")).value==Property(h::debugText(units({8242}))).value);
m::MmlMo::primes()=old;m::MmlMo::remapPrimes()=remap;
enabled=false;};
"math_port_mo_order_lookup_limits_shortcircuit_ab"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,units({97,98}),PropertyList{{"movablelimits","raw"}});n->setProperty("texClass",false);
traceLog.clear();enabled=true;n->op(units({97,98}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","static.spacing"})<<traceText();
enabled=false;};
"math_port_mo_order_lookup_limits_shortcircuit_x"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,units({120}),PropertyList{{"movablelimits","raw"}});n->setProperty("texClass",false);
traceLog.clear();enabled=true;n->op(units({120}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","static.spacing"})<<traceText();
enabled=false;};
"math_port_mo_order_null_attributes_failure_table"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"'");n->setAttributes(nullptr);
traceLog.clear();enabled=true;{bool caught=false;try{n->table(u"'");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes"})<<traceText();
expect((n->getProperty("pseudoscript")).value==Property(Missing{}).value);
enabled=false;};
"math_port_mo_order_null_attributes_failure_pseudo"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"'");n->setAttributes(nullptr);
traceLog.clear();enabled=true;{bool caught=false;try{n->pseudo(u"'");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"static.pseudo","Parent","set/pseudoscript","attributes"})<<traceText();
expect((n->getProperty("pseudoscript")).value==Property(true).value);
enabled=false;};
"math_port_mo_order_string_spacing_iterator"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"!");auto old=m::MmlMo::operatorTable();auto spacing=m::MmlMo::mathSpacing();m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};PropertyArray a;for(int i=0;i<8;++i)a.push(h::debugText(units({55357,56832,120})));m::MmlMo::mathSpacing()=a;auto result=n->op(u"!");
expect((h::get(result,"0")).value==Property(h::debugText(units({55357,56832}))).value);
expect((h::get(result,"1")).value==Property(h::debugText(units({120}))).value);
expect((h::get(result,"2")).value==Property(0).value);
m::MmlMo::operatorTable()=old;m::MmlMo::mathSpacing()=spacing;
enabled=false;};
"math_port_mo_order_malformed_accent_empty"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*over=mo(f,u"^");auto*p=create(f,"munderover",{},children({b,n,over}));p->childNodes=children();n->setParent(m::NodeValue(p));
traceLog.clear();enabled=true;{bool caught=false;try{n->accent(u"^");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"Parent","get/mathaccent"})<<traceText();
expect((n->getProperty("mathaccent")).value==Property(Missing{}).value);
enabled=false;};
"math_port_mo_order_malformed_accent_null-array"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*over=mo(f,u"^");auto*p=create(f,"munderover",{},children({b,n,over}));p->childNodes=nullptr;n->setParent(m::NodeValue(p));
traceLog.clear();enabled=true;{bool caught=false;try{n->accent(u"^");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"Parent","get/mathaccent"})<<traceText();
expect((n->getProperty("mathaccent")).value==Property(Missing{}).value);
enabled=false;};
"math_port_mo_order_malformed_accent_null-base"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*over=mo(f,u"^");auto*p=create(f,"munderover",{},children({b,n,over}));p->childNodes=children({nullptr,n,over});n->setParent(m::NodeValue(p));
traceLog.clear();enabled=true;{bool caught=false;try{n->accent(u"^");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"Parent","get/mathaccent"})<<traceText();
expect((n->getProperty("mathaccent")).value==Property(Missing{}).value);
enabled=false;};
"math_port_mo_order_malformed_accent_missing-under-embellished-over"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*over=mo(f,u"^");auto*p=create(f,"munderover",{},children({b,n,over}));p->childNodes=children({b,m::NodeValue(folia::math::Missing{}),over});n->setParent(m::NodeValue(p));
traceLog.clear();enabled=true;{bool caught=false;try{n->accent(u"^");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"Parent","get/mathaccent"})<<traceText();
expect((n->getProperty("mathaccent")).value==Property(Missing{}).value);
enabled=false;};
"math_port_mo_order_accent_missing_script_mover"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*p=create(f,"mover",{},children({b,n}));p->childNodes=children({b});n->setParent(m::NodeValue(p));
traceLog.clear();enabled=true;{bool caught=false;try{n->isAccent();}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{})<<traceText();
enabled=false;};
"math_port_mo_order_accent_missing_script_munder"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*p=create(f,"munder",{},children({b,n}));p->childNodes=children({b});n->setParent(m::NodeValue(p));
traceLog.clear();enabled=true;{bool caught=false;try{n->isAccent();}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{})<<traceText();
enabled=false;};
"math_port_mo_order_accent_missing_script_munderover"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*p=create(f,"munderover",{},children({b,n,n}));p->childNodes=children({b});n->setParent(m::NodeValue(p));
traceLog.clear();enabled=true;{bool caught=false;try{n->isAccent();}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{})<<traceText();
enabled=false;};
"math_port_mo_order_adjust_null_prev_-1"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");n->_texClass=-1;
traceLog.clear();enabled=true;n->adjustTeXclass(nullptr);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read"})<<traceText();
expect((n->_texClass).value==Property(-1).value);
expect((n->prevClass()).value==Property(PropertyNull{}).value);
enabled=false;};
"math_port_mo_order_adjust_null_prev_0"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");n->_texClass=0;
traceLog.clear();enabled=true;n->adjustTeXclass(nullptr);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read"})<<traceText();
expect((n->_texClass).value==Property(0).value);
expect((n->prevClass()).value==Property(-1).value);
enabled=false;};
"math_port_mo_order_adjust_null_prev_1"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");n->_texClass=1;
traceLog.clear();enabled=true;n->adjustTeXclass(nullptr);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read"})<<traceText();
expect((n->_texClass).value==Property(1).value);
expect((n->prevClass()).value==Property(-1).value);
enabled=false;};
"math_port_mo_order_adjust_null_prev_2"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");n->_texClass=2;
traceLog.clear();enabled=true;n->adjustTeXclass(nullptr);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read","tex.write"})<<traceText();
expect((n->_texClass).value==Property(0).value);
expect((n->prevClass()).value==Property(-1).value);
enabled=false;};
"math_port_mo_order_adjust_null_prev_3"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");n->_texClass=3;
traceLog.clear();enabled=true;n->adjustTeXclass(nullptr);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read"})<<traceText();
expect((n->_texClass).value==Property(3).value);
expect((n->prevClass()).value==Property(-1).value);
enabled=false;};
"math_port_mo_order_adjust_null_prev_4"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");n->_texClass=4;
traceLog.clear();enabled=true;n->adjustTeXclass(nullptr);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read"})<<traceText();
expect((n->_texClass).value==Property(4).value);
expect((n->prevClass()).value==Property(-1).value);
enabled=false;};
"math_port_mo_order_adjust_null_prev_5"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");n->_texClass=5;
traceLog.clear();enabled=true;n->adjustTeXclass(nullptr);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read"})<<traceText();
expect((n->_texClass).value==Property(5).value);
expect((n->prevClass()).value==Property(-1).value);
enabled=false;};
"math_port_mo_order_adjust_null_prev_6"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");n->_texClass=6;
traceLog.clear();enabled=true;n->adjustTeXclass(nullptr);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read"})<<traceText();
expect((n->_texClass).value==Property(6).value);
expect((n->prevClass()).value==Property(-1).value);
enabled=false;};
"math_port_mo_order_adjust_null_prev_7"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"+");n->_texClass=7;
traceLog.clear();enabled=true;n->adjustTeXclass(nullptr);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read"})<<traceText();
expect((n->_texClass).value==Property(7).value);
expect((n->prevClass()).value==Property(-1).value);
enabled=false;};
"math_port_mo_order_function_prototype_form_hits_and_poison_accessors"_test=[] {enabled=false;traceLog.clear();
NF f(classes());
{auto*n=mo(f,units({99,97,108,108,101,114}),PropertyList{{"form","constructor"}});
traceLog.clear();enabled=true;{bool caught=false;try{n->table(units({99,97,108,108,101,114}));}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table"})<<traceText();
expect((n->_texClass).value==Property(PropertyNull{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({97,114,103,117,109,101,110,116,115}),PropertyList{{"form","constructor"}});
traceLog.clear();enabled=true;{bool caught=false;try{n->table(units({97,114,103,117,109,101,110,116,115}));}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table"})<<traceText();
expect((n->_texClass).value==Property(PropertyNull{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({95,95,112,114,111,116,111,95,95}),PropertyList{{"form","constructor"}});
traceLog.clear();enabled=true;n->table(units({95,95,112,114,111,116,111,95,95}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({99,97,108,108}),PropertyList{{"form","constructor"}});
traceLog.clear();enabled=true;n->table(units({99,97,108,108}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({97,112,112,108,121}),PropertyList{{"form","constructor"}});
traceLog.clear();enabled=true;n->table(units({97,112,112,108,121}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({98,105,110,100}),PropertyList{{"form","constructor"}});
traceLog.clear();enabled=true;n->table(units({98,105,110,100}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({116,111,83,116,114,105,110,103}),PropertyList{{"form","constructor"}});
traceLog.clear();enabled=true;n->table(units({116,111,83,116,114,105,110,103}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({99,111,110,115,116,114,117,99,116,111,114}),PropertyList{{"form","constructor"}});
traceLog.clear();enabled=true;n->table(units({99,111,110,115,116,114,117,99,116,111,114}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({110,97,109,101}),PropertyList{{"form","constructor"}});
traceLog.clear();enabled=true;n->table(units({110,97,109,101}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write","attributes"})<<traceText();
expect((n->_texClass).value==Property(h::debugText(units({106}))).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({108,101,110,103,116,104}),PropertyList{{"form","constructor"}});
traceLog.clear();enabled=true;n->table(units({108,101,110,103,116,104}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({99,97,108,108,101,114}),PropertyList{{"form","toString"}});
traceLog.clear();enabled=true;{bool caught=false;try{n->table(units({99,97,108,108,101,114}));}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table"})<<traceText();
expect((n->_texClass).value==Property(PropertyNull{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({97,114,103,117,109,101,110,116,115}),PropertyList{{"form","toString"}});
traceLog.clear();enabled=true;{bool caught=false;try{n->table(units({97,114,103,117,109,101,110,116,115}));}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table"})<<traceText();
expect((n->_texClass).value==Property(PropertyNull{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({95,95,112,114,111,116,111,95,95}),PropertyList{{"form","toString"}});
traceLog.clear();enabled=true;n->table(units({95,95,112,114,111,116,111,95,95}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({99,97,108,108}),PropertyList{{"form","toString"}});
traceLog.clear();enabled=true;n->table(units({99,97,108,108}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({97,112,112,108,121}),PropertyList{{"form","toString"}});
traceLog.clear();enabled=true;n->table(units({97,112,112,108,121}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({98,105,110,100}),PropertyList{{"form","toString"}});
traceLog.clear();enabled=true;n->table(units({98,105,110,100}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({116,111,83,116,114,105,110,103}),PropertyList{{"form","toString"}});
traceLog.clear();enabled=true;n->table(units({116,111,83,116,114,105,110,103}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({99,111,110,115,116,114,117,99,116,111,114}),PropertyList{{"form","toString"}});
traceLog.clear();enabled=true;n->table(units({99,111,110,115,116,114,117,99,116,111,114}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({110,97,109,101}),PropertyList{{"form","toString"}});
traceLog.clear();enabled=true;n->table(units({110,97,109,101}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write","attributes"})<<traceText();
expect((n->_texClass).value==Property(h::debugText(units({83}))).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({108,101,110,103,116,104}),PropertyList{{"form","toString"}});
traceLog.clear();enabled=true;n->table(units({108,101,110,103,116,104}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(1).value);
expect((n->getProperty("noDictDef")).value==Property(true).value);
}
{auto*n=mo(f,units({99,97,108,108,101,114}),PropertyList{{"form","valueOf"}});
traceLog.clear();enabled=true;{bool caught=false;try{n->table(units({99,97,108,108,101,114}));}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table"})<<traceText();
expect((n->_texClass).value==Property(PropertyNull{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({97,114,103,117,109,101,110,116,115}),PropertyList{{"form","valueOf"}});
traceLog.clear();enabled=true;{bool caught=false;try{n->table(units({97,114,103,117,109,101,110,116,115}));}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table"})<<traceText();
expect((n->_texClass).value==Property(PropertyNull{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({95,95,112,114,111,116,111,95,95}),PropertyList{{"form","valueOf"}});
traceLog.clear();enabled=true;n->table(units({95,95,112,114,111,116,111,95,95}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({99,97,108,108}),PropertyList{{"form","valueOf"}});
traceLog.clear();enabled=true;n->table(units({99,97,108,108}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({97,112,112,108,121}),PropertyList{{"form","valueOf"}});
traceLog.clear();enabled=true;n->table(units({97,112,112,108,121}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({98,105,110,100}),PropertyList{{"form","valueOf"}});
traceLog.clear();enabled=true;n->table(units({98,105,110,100}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({116,111,83,116,114,105,110,103}),PropertyList{{"form","valueOf"}});
traceLog.clear();enabled=true;n->table(units({116,111,83,116,114,105,110,103}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({99,111,110,115,116,114,117,99,116,111,114}),PropertyList{{"form","valueOf"}});
traceLog.clear();enabled=true;n->table(units({99,111,110,115,116,114,117,99,116,111,114}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({110,97,109,101}),PropertyList{{"form","valueOf"}});
traceLog.clear();enabled=true;n->table(units({110,97,109,101}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write","attributes"})<<traceText();
expect((n->_texClass).value==Property(h::debugText(units({108}))).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({108,101,110,103,116,104}),PropertyList{{"form","valueOf"}});
traceLog.clear();enabled=true;n->table(units({108,101,110,103,116,104}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(1).value);
expect((n->getProperty("noDictDef")).value==Property(true).value);
}
{auto*n=mo(f,units({99,97,108,108,101,114}),PropertyList{{"form","__proto__"}});
traceLog.clear();enabled=true;n->table(units({99,97,108,108,101,114}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(1).value);
expect((n->getProperty("noDictDef")).value==Property(true).value);
}
{auto*n=mo(f,units({97,114,103,117,109,101,110,116,115}),PropertyList{{"form","__proto__"}});
traceLog.clear();enabled=true;n->table(units({97,114,103,117,109,101,110,116,115}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(1).value);
expect((n->getProperty("noDictDef")).value==Property(true).value);
}
{auto*n=mo(f,units({95,95,112,114,111,116,111,95,95}),PropertyList{{"form","__proto__"}});
traceLog.clear();enabled=true;n->table(units({95,95,112,114,111,116,111,95,95}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({99,97,108,108}),PropertyList{{"form","__proto__"}});
traceLog.clear();enabled=true;n->table(units({99,97,108,108}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(1).value);
expect((n->getProperty("noDictDef")).value==Property(true).value);
}
{auto*n=mo(f,units({97,112,112,108,121}),PropertyList{{"form","__proto__"}});
traceLog.clear();enabled=true;n->table(units({97,112,112,108,121}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(1).value);
expect((n->getProperty("noDictDef")).value==Property(true).value);
}
{auto*n=mo(f,units({98,105,110,100}),PropertyList{{"form","__proto__"}});
traceLog.clear();enabled=true;n->table(units({98,105,110,100}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(1).value);
expect((n->getProperty("noDictDef")).value==Property(true).value);
}
{auto*n=mo(f,units({116,111,83,116,114,105,110,103}),PropertyList{{"form","__proto__"}});
traceLog.clear();enabled=true;n->table(units({116,111,83,116,114,105,110,103}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({99,111,110,115,116,114,117,99,116,111,114}),PropertyList{{"form","__proto__"}});
traceLog.clear();enabled=true;n->table(units({99,111,110,115,116,114,117,99,116,111,114}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(Missing{}).value);
expect((n->getProperty("noDictDef")).value==Property(Missing{}).value);
}
{auto*n=mo(f,units({110,97,109,101}),PropertyList{{"form","__proto__"}});
traceLog.clear();enabled=true;n->table(units({110,97,109,101}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(1).value);
expect((n->getProperty("noDictDef")).value==Property(true).value);
}
{auto*n=mo(f,units({108,101,110,103,116,104}),PropertyList{{"form","__proto__"}});
traceLog.clear();enabled=true;n->table(units({108,101,110,103,116,104}));enabled=false;
expect(traceLog==std::vector<std::string>{"forms","Parent","explicit","attributes","attributes","attributes","static.table","set/noDictDef","attributes","static.op","get/texClass","get/texClass","tex.write"})<<traceText();
expect((n->_texClass).value==Property(1).value);
expect((n->getProperty("noDictDef")).value==Property(true).value);
}
enabled=false;};
"math_port_mo_order_coretext_non_embellished_nested-singletons"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("mstyle",m::classBinding<m::MmlMstyle>());f.setNodeClass("mphantom",m::classBinding<m::MmlMphantom>());f.setNodeClass("TeXAtom",m::classBinding<Atom>());auto*n=mo(f,u"+");auto*x=token(f,"mi",u"x");auto*y=token(f,"mi",u"y");m::MmlNode*node=x;node=create(f,"mphantom",{},children({node}));node=create(f,"mstyle",{},children({node}));node=create(f,"mrow",{},children({node}));
expect((Property(h::debugText(n->coreText(node)))).value==Property(h::debugText(units({120}))).value);
enabled=false;};
"math_port_mo_order_coretext_non_embellished_multiple-row"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("mstyle",m::classBinding<m::MmlMstyle>());f.setNodeClass("mphantom",m::classBinding<m::MmlMphantom>());f.setNodeClass("TeXAtom",m::classBinding<Atom>());auto*n=mo(f,u"+");auto*x=token(f,"mi",u"x");auto*y=token(f,"mi",u"y");m::MmlNode*node=x;node=create(f,"mrow",{},children({x,y}));
expect((Property(h::debugText(n->coreText(node)))).value==Property(h::debugText(units({}))).value);
enabled=false;};
"math_port_mo_order_coretext_non_embellished_under-over"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("mstyle",m::classBinding<m::MmlMstyle>());f.setNodeClass("mphantom",m::classBinding<m::MmlMphantom>());f.setNodeClass("TeXAtom",m::classBinding<Atom>());auto*n=mo(f,u"+");auto*x=token(f,"mi",u"x");auto*y=token(f,"mi",u"y");m::MmlNode*node=x;node=create(f,"munderover",{},children({x,y,y}));
expect((Property(h::debugText(n->coreText(node)))).value==Property(h::debugText(units({120}))).value);
enabled=false;};
"math_port_mo_order_coretext_non_embellished_atom"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("mstyle",m::classBinding<m::MmlMstyle>());f.setNodeClass("mphantom",m::classBinding<m::MmlMphantom>());f.setNodeClass("TeXAtom",m::classBinding<Atom>());auto*n=mo(f,u"+");auto*x=token(f,"mi",u"x");auto*y=token(f,"mi",u"y");m::MmlNode*node=x;node=create(f,"TeXAtom",{},children({x}));
expect((Property(h::debugText(n->coreText(node)))).value==Property(h::debugText(units({120}))).value);
enabled=false;};
"math_port_mo_order_binary_parent_walk_row-last"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("mstyle",m::classBinding<m::MmlMstyle>());auto*n=mo(f,u"+");auto*prev=token(f,"mi",u"p");auto*other=token(f,"mi",u"q");n->_texClass=2;prev->setTexClass(0);m::MmlNode*node=n;create(f,"mrow",{},children({prev,node}));
traceLog.clear();enabled=true;n->adjustTeXclass(prev);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read","attributes","tex.write"})<<traceText();
expect((n->_texClass).value==Property(0).value);
enabled=false;};
"math_port_mo_order_binary_parent_walk_style-chain-last"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("mstyle",m::classBinding<m::MmlMstyle>());auto*n=mo(f,u"+");auto*prev=token(f,"mi",u"p");auto*other=token(f,"mi",u"q");n->_texClass=2;prev->setTexClass(0);m::MmlNode*node=n;node=create(f,"mstyle",{},children({n}));create(f,"mrow",{},children({prev,node}));
traceLog.clear();enabled=true;n->adjustTeXclass(prev);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read","attributes","tex.write"})<<traceText();
expect((n->_texClass).value==Property(0).value);
enabled=false;};
"math_port_mo_order_binary_parent_walk_script-chain-last"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("mstyle",m::classBinding<m::MmlMstyle>());auto*n=mo(f,u"+");auto*prev=token(f,"mi",u"p");auto*other=token(f,"mi",u"q");n->_texClass=2;prev->setTexClass(0);m::MmlNode*node=n;node=create(f,"munderover",{},children({n,other,other}));create(f,"mrow",{},children({prev,node}));
traceLog.clear();enabled=true;n->adjustTeXclass(prev);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read","attributes"})<<traceText();
expect((n->_texClass).value==Property(2).value);
enabled=false;};
"math_port_mo_order_binary_parent_walk_style-chain-not-last"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("mstyle",m::classBinding<m::MmlMstyle>());auto*n=mo(f,u"+");auto*prev=token(f,"mi",u"p");auto*other=token(f,"mi",u"q");n->_texClass=2;prev->setTexClass(0);m::MmlNode*node=n;node=create(f,"mstyle",{},children({n}));create(f,"mrow",{},children({node,other}));
traceLog.clear();enabled=true;n->adjustTeXclass(prev);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read","attributes"})<<traceText();
expect((n->_texClass).value==Property(2).value);
enabled=false;};
"math_port_mo_order_binary_previous_assignment_right_to_left"_test=[] {enabled=false;traceLog.clear();
NF f(classes());f.setNodeClass("previous",m::classBinding<Previous>());auto*n=mo(f,u"=");auto*prev=mo(f,u"+",{},"previous");current=n;n->_texClass=3;prev->_texClass=2;
traceLog.clear();enabled=true;n->adjustTeXclass(prev);enabled=false;
expect(traceLog==std::vector<std::string>{"tex.read","get/autoOP","prev.tex.read","tex.read","attributes","prev.tex.write/0","tex.write"})<<traceText();
current=nullptr;
enabled=false;};
"math_port_mo_order_intrinsic_function_identity_metadata"_test=[] {enabled=false;traceLog.clear();
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::constructor))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,79,98,106,101,99,116,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::constructor),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::constructor),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::constructor),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::constructor),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::constructor),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::constructor),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect(h::get(Property(folia::math::BuiltinProperty::constructor),"prototype")==Property(folia::math::BuiltinProperty::prototype));
expect((h::get(Property(folia::math::BuiltinProperty::constructor),"name")).value==Property(h::debugText(units({79,98,106,101,99,116}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::constructor),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::constructor),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::constructor),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::defineGetter))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,95,95,100,101,102,105,110,101,71,101,116,116,101,114,95,95,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::defineGetter),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::defineGetter),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::defineGetter),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::defineGetter),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::defineGetter),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::defineGetter),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::defineGetter),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::defineGetter),"name")).value==Property(h::debugText(units({95,95,100,101,102,105,110,101,71,101,116,116,101,114,95,95}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::defineGetter),"length")).value==Property(2).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::defineGetter),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::defineGetter),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::defineSetter))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,95,95,100,101,102,105,110,101,83,101,116,116,101,114,95,95,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::defineSetter),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::defineSetter),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::defineSetter),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::defineSetter),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::defineSetter),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::defineSetter),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::defineSetter),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::defineSetter),"name")).value==Property(h::debugText(units({95,95,100,101,102,105,110,101,83,101,116,116,101,114,95,95}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::defineSetter),"length")).value==Property(2).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::defineSetter),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::defineSetter),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::hasOwnProperty))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,104,97,115,79,119,110,80,114,111,112,101,114,116,121,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"name")).value==Property(h::debugText(units({104,97,115,79,119,110,80,114,111,112,101,114,116,121}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::hasOwnProperty),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::lookupGetter))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,95,95,108,111,111,107,117,112,71,101,116,116,101,114,95,95,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::lookupGetter),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::lookupGetter),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::lookupGetter),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::lookupGetter),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::lookupGetter),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::lookupGetter),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::lookupGetter),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::lookupGetter),"name")).value==Property(h::debugText(units({95,95,108,111,111,107,117,112,71,101,116,116,101,114,95,95}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::lookupGetter),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::lookupGetter),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::lookupGetter),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::lookupSetter))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,95,95,108,111,111,107,117,112,83,101,116,116,101,114,95,95,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::lookupSetter),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::lookupSetter),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::lookupSetter),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::lookupSetter),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::lookupSetter),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::lookupSetter),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::lookupSetter),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::lookupSetter),"name")).value==Property(h::debugText(units({95,95,108,111,111,107,117,112,83,101,116,116,101,114,95,95}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::lookupSetter),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::lookupSetter),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::lookupSetter),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::isPrototypeOf))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,105,115,80,114,111,116,111,116,121,112,101,79,102,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"name")).value==Property(h::debugText(units({105,115,80,114,111,116,111,116,121,112,101,79,102}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::isPrototypeOf),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::propertyIsEnumerable))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,112,114,111,112,101,114,116,121,73,115,69,110,117,109,101,114,97,98,108,101,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"name")).value==Property(h::debugText(units({112,114,111,112,101,114,116,121,73,115,69,110,117,109,101,114,97,98,108,101}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::propertyIsEnumerable),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::toString))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,116,111,83,116,114,105,110,103,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::toString),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::toString),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::toString),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::toString),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::toString),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::toString),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::toString),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::toString),"name")).value==Property(h::debugText(units({116,111,83,116,114,105,110,103}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::toString),"length")).value==Property(0).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::toString),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::toString),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::valueOf))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,118,97,108,117,101,79,102,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::valueOf),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::valueOf),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::valueOf),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::valueOf),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::valueOf),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::valueOf),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::valueOf),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::valueOf),"name")).value==Property(h::debugText(units({118,97,108,117,101,79,102}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::valueOf),"length")).value==Property(0).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::valueOf),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::valueOf),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::toLocaleString))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,116,111,76,111,99,97,108,101,83,116,114,105,110,103,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::toLocaleString),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::toLocaleString),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::toLocaleString),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::toLocaleString),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::toLocaleString),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::toLocaleString),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::toLocaleString),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::toLocaleString),"name")).value==Property(h::debugText(units({116,111,76,111,99,97,108,101,83,116,114,105,110,103}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::toLocaleString),"length")).value==Property(0).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::toLocaleString),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::toLocaleString),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::arrayConstructor))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,65,114,114,97,121,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect(h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"prototype")==Property(folia::math::BuiltinProperty::arrayPrototype));
expect((h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"name")).value==Property(h::debugText(units({65,114,114,97,121}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::arrayConstructor),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::arrayToString))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,116,111,83,116,114,105,110,103,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::arrayToString),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::arrayToString),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::arrayToString),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::arrayToString),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::arrayToString),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::arrayToString),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::arrayToString),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayToString),"name")).value==Property(h::debugText(units({116,111,83,116,114,105,110,103}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayToString),"length")).value==Property(0).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::arrayToString),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::arrayToString),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::arrayJoin))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,106,111,105,110,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::arrayJoin),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::arrayJoin),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::arrayJoin),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::arrayJoin),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::arrayJoin),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::arrayJoin),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::arrayJoin),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayJoin),"name")).value==Property(h::debugText(units({106,111,105,110}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayJoin),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::arrayJoin),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::arrayJoin),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::numberConstructor))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,78,117,109,98,101,114,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::numberConstructor),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::numberConstructor),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::numberConstructor),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::numberConstructor),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::numberConstructor),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::numberConstructor),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect(h::get(Property(folia::math::BuiltinProperty::numberConstructor),"prototype")==Property(folia::math::BuiltinProperty::numberPrototype));
expect((h::get(Property(folia::math::BuiltinProperty::numberConstructor),"name")).value==Property(h::debugText(units({78,117,109,98,101,114}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberConstructor),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::numberConstructor),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::numberConstructor),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::numberToString))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,116,111,83,116,114,105,110,103,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::numberToString),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::numberToString),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::numberToString),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::numberToString),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::numberToString),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::numberToString),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::numberToString),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberToString),"name")).value==Property(h::debugText(units({116,111,83,116,114,105,110,103}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberToString),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::numberToString),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::numberToString),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::numberValueOf))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,118,97,108,117,101,79,102,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::numberValueOf),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::numberValueOf),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::numberValueOf),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::numberValueOf),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::numberValueOf),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::numberValueOf),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::numberValueOf),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberValueOf),"name")).value==Property(h::debugText(units({118,97,108,117,101,79,102}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberValueOf),"length")).value==Property(0).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::numberValueOf),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::numberValueOf),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::functionPrototype))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::functionPrototype),"__proto__")==Property(folia::math::BuiltinProperty::prototype));
expect(h::get(Property(folia::math::BuiltinProperty::functionPrototype),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::functionPrototype),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::functionPrototype),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::functionPrototype),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::functionPrototype),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::functionPrototype),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionPrototype),"name")).value==Property(h::debugText(units({}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionPrototype),"length")).value==Property(0).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionPrototype),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionPrototype),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::functionConstructor))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,70,117,110,99,116,105,111,110,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::functionConstructor),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::functionConstructor),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::functionConstructor),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::functionConstructor),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::functionConstructor),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::functionConstructor),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect(h::get(Property(folia::math::BuiltinProperty::functionConstructor),"prototype")==Property(folia::math::BuiltinProperty::functionPrototype));
expect((h::get(Property(folia::math::BuiltinProperty::functionConstructor),"name")).value==Property(h::debugText(units({70,117,110,99,116,105,111,110}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionConstructor),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionConstructor),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionConstructor),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::functionCall))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,99,97,108,108,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::functionCall),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::functionCall),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::functionCall),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::functionCall),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::functionCall),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::functionCall),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::functionCall),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionCall),"name")).value==Property(h::debugText(units({99,97,108,108}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionCall),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionCall),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionCall),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::functionApply))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,97,112,112,108,121,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::functionApply),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::functionApply),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::functionApply),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::functionApply),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::functionApply),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::functionApply),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::functionApply),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionApply),"name")).value==Property(h::debugText(units({97,112,112,108,121}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionApply),"length")).value==Property(2).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionApply),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionApply),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::functionBind))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,98,105,110,100,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::functionBind),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::functionBind),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::functionBind),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::functionBind),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::functionBind),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::functionBind),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::functionBind),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionBind),"name")).value==Property(h::debugText(units({98,105,110,100}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionBind),"length")).value==Property(1).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionBind),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionBind),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::functionToString))))).value==Property(h::debugText(units({102,117,110,99,116,105,111,110,32,116,111,83,116,114,105,110,103,40,41,32,123,32,91,110,97,116,105,118,101,32,99,111,100,101,93,32,125}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::functionToString),"__proto__")==Property(folia::math::BuiltinProperty::functionPrototype));
expect(h::get(Property(folia::math::BuiltinProperty::functionToString),"constructor")==Property(folia::math::BuiltinProperty::functionConstructor));
expect(h::get(Property(folia::math::BuiltinProperty::functionToString),"call")==Property(folia::math::BuiltinProperty::functionCall));
expect(h::get(Property(folia::math::BuiltinProperty::functionToString),"apply")==Property(folia::math::BuiltinProperty::functionApply));
expect(h::get(Property(folia::math::BuiltinProperty::functionToString),"bind")==Property(folia::math::BuiltinProperty::functionBind));
expect(h::get(Property(folia::math::BuiltinProperty::functionToString),"toString")==Property(folia::math::BuiltinProperty::functionToString));
expect((h::get(Property(folia::math::BuiltinProperty::functionToString),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionToString),"name")).value==Property(h::debugText(units({116,111,83,116,114,105,110,103}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::functionToString),"length")).value==Property(0).value);
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionToString),"caller");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
{bool caught=false;try{h::get(Property(folia::math::BuiltinProperty::functionToString),"arguments");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::prototype))))).value==Property(h::debugText(units({91,111,98,106,101,99,116,32,79,98,106,101,99,116,93}))).value);
expect((h::get(Property(folia::math::BuiltinProperty::prototype),"__proto__")).value==Property(PropertyNull{}).value);
expect(h::get(Property(folia::math::BuiltinProperty::prototype),"constructor")==Property(folia::math::BuiltinProperty::constructor));
expect((h::get(Property(folia::math::BuiltinProperty::prototype),"call")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::prototype),"apply")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::prototype),"bind")).value==Property(Missing{}).value);
expect(h::get(Property(folia::math::BuiltinProperty::prototype),"toString")==Property(folia::math::BuiltinProperty::toString));
expect((h::get(Property(folia::math::BuiltinProperty::prototype),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::prototype),"name")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::prototype),"length")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::prototype),"caller")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::prototype),"arguments")).value==Property(Missing{}).value);
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::arrayPrototype))))).value==Property(h::debugText(units({}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"__proto__")==Property(folia::math::BuiltinProperty::prototype));
expect(h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"constructor")==Property(folia::math::BuiltinProperty::arrayConstructor));
expect((h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"call")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"apply")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"bind")).value==Property(Missing{}).value);
expect(h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"toString")==Property(folia::math::BuiltinProperty::arrayToString));
expect((h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"name")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"length")).value==Property(0).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"caller")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::arrayPrototype),"arguments")).value==Property(Missing{}).value);
expect((Property(h::debugText(h::string(Property(folia::math::BuiltinProperty::numberPrototype))))).value==Property(h::debugText(units({48}))).value);
expect(h::get(Property(folia::math::BuiltinProperty::numberPrototype),"__proto__")==Property(folia::math::BuiltinProperty::prototype));
expect(h::get(Property(folia::math::BuiltinProperty::numberPrototype),"constructor")==Property(folia::math::BuiltinProperty::numberConstructor));
expect((h::get(Property(folia::math::BuiltinProperty::numberPrototype),"call")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberPrototype),"apply")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberPrototype),"bind")).value==Property(Missing{}).value);
expect(h::get(Property(folia::math::BuiltinProperty::numberPrototype),"toString")==Property(folia::math::BuiltinProperty::numberToString));
expect((h::get(Property(folia::math::BuiltinProperty::numberPrototype),"prototype")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberPrototype),"name")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberPrototype),"length")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberPrototype),"caller")).value==Property(Missing{}).value);
expect((h::get(Property(folia::math::BuiltinProperty::numberPrototype),"arguments")).value==Property(Missing{}).value);
enabled=false;};
};
