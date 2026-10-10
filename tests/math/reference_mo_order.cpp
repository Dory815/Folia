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
m::Children children(std::initializer_list<m::MmlNode*>v={}){return std::make_shared<std::vector<m::MmlNode*>>(v);}
struct Probe:m::MmlMo{using MmlMo::MmlMo;m::MmlNode*Parent()const override{note("Parent");return MmlMo::Parent();}std::shared_ptr<folia::math::Attributes>attributes()const override{note("attributes");return MmlMo::attributes();}std::u16string getText()const override{note("text");return MmlMo::getText();}Property getProperty(std::string_view k)const override{note("get/"+std::string(k));return MmlMo::getProperty(k);}void setProperty(std::string_view k,Property v)override{note("set/"+std::string(k));MmlMo::setProperty(k,std::move(v));}Property texClass()override{note("tex.read");return MmlMo::texClass();}void setTexClass(Property v)override{note("tex.write");MmlMo::setTexClass(std::move(v));}PropertyArray getForms()override{note("forms");return MmlMo::getForms();}PropertyArray handleExplicitForm(PropertyArray a)override{note("explicit");return MmlMo::handleExplicitForm(a);}
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
struct Snapshot:m::MmlMi{using MmlMi::MmlMi;bool isEmbellished()const override{if(enabled){note("snapshot");(*parent->childNodes)[1]=const_cast<Snapshot*>(this);(*parent->childNodes)[2]=const_cast<Snapshot*>(this);}return false;}};
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
traceLog.clear();enabled=true;n->setInheritedAttributes({},false,2,true);enabled=false;
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
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*over=mo(f,u"^");auto*p=create(f,"munderover",{},children({b,n,over}));p->childNodes=children();n->parent=p;
traceLog.clear();enabled=true;{bool caught=false;try{n->accent(u"^");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"Parent","get/mathaccent"})<<traceText();
expect((n->getProperty("mathaccent")).value==Property(Missing{}).value);
enabled=false;};
"math_port_mo_order_malformed_accent_null-array"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*over=mo(f,u"^");auto*p=create(f,"munderover",{},children({b,n,over}));p->childNodes=nullptr;n->parent=p;
traceLog.clear();enabled=true;{bool caught=false;try{n->accent(u"^");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"Parent","get/mathaccent"})<<traceText();
expect((n->getProperty("mathaccent")).value==Property(Missing{}).value);
enabled=false;};
"math_port_mo_order_malformed_accent_null-base"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*over=mo(f,u"^");auto*p=create(f,"munderover",{},children({b,n,over}));p->childNodes=children({nullptr,n,over});n->parent=p;
traceLog.clear();enabled=true;{bool caught=false;try{n->accent(u"^");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"Parent","get/mathaccent"})<<traceText();
expect((n->getProperty("mathaccent")).value==Property(Missing{}).value);
enabled=false;};
"math_port_mo_order_malformed_accent_missing-under-embellished-over"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*over=mo(f,u"^");auto*p=create(f,"munderover",{},children({b,n,over}));p->childNodes=children({b,nullptr,over});n->parent=p;
traceLog.clear();enabled=true;{bool caught=false;try{n->accent(u"^");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{"Parent","get/mathaccent"})<<traceText();
expect((n->getProperty("mathaccent")).value==Property(Missing{}).value);
enabled=false;};
"math_port_mo_order_accent_missing_script_mover"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*p=create(f,"mover",{},children({b,n}));p->childNodes=children({b});n->parent=p;
traceLog.clear();enabled=true;{bool caught=false;try{n->isAccent();}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{})<<traceText();
enabled=false;};
"math_port_mo_order_accent_missing_script_munder"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*p=create(f,"munder",{},children({b,n}));p->childNodes=children({b});n->parent=p;
traceLog.clear();enabled=true;{bool caught=false;try{n->isAccent();}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}enabled=false;
expect(traceLog==std::vector<std::string>{})<<traceText();
enabled=false;};
"math_port_mo_order_accent_missing_script_munderover"_test=[] {enabled=false;traceLog.clear();
NF f(classes());auto*n=mo(f,u"^");auto*b=token(f,"mi",u"b");auto*p=create(f,"munderover",{},children({b,n,n}));p->childNodes=children({b});n->parent=p;
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
};
