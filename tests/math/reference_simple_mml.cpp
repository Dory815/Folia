// Generated actual-CJS P16a semantic observations.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_nodes.menclose;
import folia.math.core.mml_tree.mml_nodes.mpadded;
import folia.math.core.mml_tree.mml_nodes.maligngroup;
import folia.math.core.mml_tree.mml_nodes.malignmark;
import folia.math.core.mml_tree.mml_nodes.mglyph;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.merror;
import folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.arithmetic_host;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.tree.node_factory;
namespace m=folia::math::mml;namespace h=m::host;namespace tr=folia::math::tree;using folia::math::Property;using folia::math::PropertyList;using folia::math::PropertyArray;using folia::math::Missing;using folia::math::PropertyNull;using namespace boost::ut;namespace {
std::u16string units(std::initializer_list<unsigned>v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}m::Children children(std::initializer_list<m::MmlNode*>v={}){return std::make_shared<std::vector<m::MmlNode*>>(v);}
Property clone(const Property&v){if(auto a=PropertyArray::array(v)){PropertyArray r;for(std::size_t i=0;i<a->size();++i)r.push(clone(a->get(i)));return r;}if(auto a=PropertyList::object(v)){PropertyList r;for(const auto&k:a->keys())r.setOwn(k,clone(a->getOwn(k)));return r;}return v;}
bool deep(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return(std::isnan(*x)&&std::isnan(*y))||(*x==*y&&std::signbit(*x)==std::signbit(*y));auto x=PropertyList::record(a),y=PropertyList::record(b);if(x&&y){if(PropertyArray::array(a).has_value()!=PropertyArray::array(b).has_value()||x->keys()!=y->keys())return false;for(const auto&k:x->keys())if(!deep(x->getOwn(k),y->getOwn(k)))return false;return true;}return a==b;}
bool enabled=false,phase=false;std::string mode;std::vector<std::string>simpleEvents;m::MmlNode*target=nullptr,*late=nullptr,*returned=nullptr;PropertyList phaseBag,merged;std::optional<PropertyList>errorOptions;struct Call{PropertyList a;Property d,l,p;};std::vector<Call>calls;
void note(std::string s){if(enabled)simpleEvents.push_back(std::move(s));}void fail(){throw m::Error("WitnessError",u"witness");}void reset(){enabled=phase=false;mode.clear();simpleEvents.clear();calls.clear();target=late=returned=nullptr;phaseBag=PropertyList{};merged=PropertyList{};errorOptions.reset();}Property eventValue(){PropertyArray a;for(const auto&s:simpleEvents)a.push(s);return a;}
struct Record:m::AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string label;std::string kind()const override{return "record";}m::MmlNode*setTeXclass(m::MmlNode*prev)override{note("child/"+label+"/"+(prev?"prev":"null"));if(mode=="child-throw")fail();if(mode=="child-rebind")target->childNodes=children({late});if(mode=="child-empty")target->childNodes=children();if(mode=="child-null-array")target->childNodes.reset();return mode=="child-return-prev"?prev:returned;}void setInheritedAttributes(PropertyList a={},Property d=false,Property l=0,Property p=false)override{note("inherit/"+label);calls.push_back({a,d,l,p});if(mode=="inherit-mutate")a.setOwn("seen",PropertyArray{"record",label});if(mode=="inherit-rebind")target->childNodes=children({late});if(mode=="inherit-append"&&label=="first")target->childNodes->push_back(late);if(mode=="inherit-throw")fail();}};
std::string label(m::MmlNode*n){if(auto*r=dynamic_cast<Record*>(n))return r->label;return n?n->kind():"none";}
template<class T>struct Probe:T{using T::T;std::shared_ptr<folia::math::Attributes>attributes()const override{note("attributes");return T::attributes();}Property texClass()override{note("tex.read");return T::texClass();}void setTexClass(Property v)override{note("tex.write");T::setTexClass(v);if(enabled&&mode=="class-rebind")this->childNodes=children({late});}Property prevClass()override{note("prev.read");return T::prevClass();}void setPrevClass(Property v)override{note("prev.write");T::setPrevClass(v);}Property prevLevel()const override{note("level.read");return T::prevLevel();}void setPrevLevel(Property v)override{note("level.write");T::setPrevLevel(v);}Property getProperty(std::string_view k)const override{note("get/"+std::string(k));const auto v=T::getProperty(k);if(enabled&&mode=="get-rebind")const_cast<Probe*>(this)->childNodes=children({late});return v;}void getPrevClass(m::MmlNode*p)override{note(std::string("getPrev/")+(p?"prev":"null"));if(mode=="prev-throw")fail();T::getPrevClass(p);if(mode=="prev-rebind")this->childNodes=children({late});}void updateTeXclass(m::MmlNode*c)override{note("update/"+label(c));if(mode=="update-throw")fail();T::updateTeXclass(c);}PropertyList addInheritedAttributes(PropertyList a,PropertyList b)override{note("merge");if(mode=="merge-throw")fail();if(mode=="merge-rebind")this->childNodes=children({late});if(mode=="merge-mutate")a.setOwn("extra",PropertyArray{"origin","raw"});merged=mode=="merge-return-input"?a:T::addInheritedAttributes(a,b);return merged;}m::MmlNode*mError(std::string s,m::ErrorOptions o,bool shortError=false)override{note("error/"+s+"/"+(shortError?"true":"false"));errorOptions=o.value();if(mode=="error-throw")fail();if(!errorOptions)h::typeError("Null witness options");errorOptions->setOwn("observed",true);return returned;}void setChildren(m::Children c)override{if(phase){note("phase/"+h::debugText(h::string(T::texClass())));phaseBag=this->getAllProperties();}T::setChildren(c);}void hook(PropertyList a,Property d,Property l,Property p){T::setChildInheritedAttributes(a,d,l,p);}};
using NF=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode>bindings(){return{{"math",m::classBinding<m::MmlMath>()},{"mrow",m::classBinding<m::MmlMrow>()},{"inferredMrow",m::classBinding<m::MmlInferredMrow>()},{"mi",m::classBinding<m::MmlMi>()},{"merror",m::classBinding<m::MmlMerror>()},{"mtext",m::classBinding<m::MmlMtext>()},{"text",m::classBinding<m::TextNode>()},{"record",m::classBinding<Record>()},{"menclose",m::classBinding<m::MmlMenclose>()},{"probe-menclose",m::classBinding<Probe<m::MmlMenclose>>()},{"mpadded",m::classBinding<m::MmlMpadded>()},{"probe-mpadded",m::classBinding<Probe<m::MmlMpadded>>()},{"maligngroup",m::classBinding<m::MmlMaligngroup>()},{"probe-maligngroup",m::classBinding<Probe<m::MmlMaligngroup>>()},{"malignmark",m::classBinding<m::MmlMalignmark>()},{"probe-malignmark",m::classBinding<Probe<m::MmlMalignmark>>()},{"mglyph",m::classBinding<m::MmlMglyph>()},{"probe-mglyph",m::classBinding<Probe<m::MmlMglyph>>()}};}
m::MmlNode*create(NF&f,std::string_view k,PropertyList a={},m::Children c=children()){return f.create(k,std::array<std::any,2>{a,c});}
struct Setup{NF f{bindings()};m::MmlNode*n;Record*a,*b,*prev;explicit Setup(std::string_view k){reset();n=create(f,k);a=dynamic_cast<Record*>(create(f,"record"));b=dynamic_cast<Record*>(create(f,"record"));prev=dynamic_cast<Record*>(create(f,"record"));a->label="first";b->label="late";prev->label="previous";a->setTexClass(2);a->setPrevClass("first-prev");a->setPrevLevel("first-level");b->setTexClass(5);b->setPrevClass("late-prev");b->setPrevLevel("late-level");prev->setTexClass(1);prev->attributes()->set("scriptlevel","2");n->childNodes=children({a});a->parent=n;b->parent=n;target=n;late=returned=b;}};
Property state(m::MmlNode*n){Property c=Missing{};if(n->childNodes){PropertyArray a;for(auto*x:*n->childNodes)a.push(x?Property(label(x)):Property(Missing{}));c=a;}return PropertyList{{"tex",n->texClass()},{"prev",n->prevClass()},{"level",n->prevLevel()},{"properties",n->getAllProperties()},{"children",c}};}
Property tree(m::MmlNode*n){if(!n)return PropertyNull{};Property text=Missing{},c=Missing{};if(auto*t=dynamic_cast<m::TextNode*>(n))text=h::debugText(t->getText());if(n->childNodes){PropertyArray a;for(auto*x:*n->childNodes)a.push(tree(x));c=a;}const auto attrs=n->attributes();return PropertyList{{"kind",n->kind()},{"text",text},{"properties",n->getAllProperties()},{"tex",n->texClass()},{"attributes",attrs?Property(attrs->getAllAttributes()):Property(Missing{})},{"inherited",attrs?Property(attrs->getAllInherited()):Property(Missing{})},{"children",c}};}
}
namespace {const std::vector<Property> sourceValues=[] {std::vector<Property> values;values.reserve(296);
values.push_back(Property(0.0));
values.push_back(Property(h::debugText(units({95,105,110,104,101,114,105,116,95}))));
values.push_back(Property(h::debugText(units({108,111,110,103,100,105,118}))));
values.push_back(Property(PropertyList{{"mathbackground",values[1]},{"mathcolor",values[1]},{"mathsize",values[1]},{"dir",values[1]},{"notation",values[2]}}));
values.push_back(Property(PropertyNull{}));
values.push_back(Property(h::debugText(units({}))));
values.push_back(Property(PropertyList{{"mathbackground",values[1]},{"mathcolor",values[1]},{"mathsize",values[1]},{"dir",values[1]},{"width",values[5]},{"height",values[5]},{"depth",values[5]},{"lspace",values[0]},{"voffset",values[0]}}));
values.push_back(Property(PropertyList{{"mathbackground",values[1]},{"mathcolor",values[1]},{"mathsize",values[1]},{"dir",values[1]},{"groupalign",values[1]}}));
values.push_back(Property(h::debugText(units({108,101,102,116}))));
values.push_back(Property(PropertyList{{"mathbackground",values[1]},{"mathcolor",values[1]},{"mathsize",values[1]},{"dir",values[1]},{"edge",values[8]}}));
values.push_back(Property(h::debugText(units({110,111,114,109,97,108}))));
values.push_back(Property(h::debugText(units({97,117,116,111}))));
values.push_back(Property(h::debugText(units({48,101,109}))));
values.push_back(Property(PropertyList{{"mathbackground",values[1]},{"mathcolor",values[1]},{"mathsize",values[1]},{"dir",values[1]},{"mathvariant",values[10]},{"alt",values[5]},{"src",values[5]},{"index",values[5]},{"width",values[11]},{"height",values[11]},{"valign",values[12]}}));
values.push_back(Property(h::debugText(units({112,114,101,118,46,119,114,105,116,101}))));
values.push_back(Property(h::debugText(units({108,101,118,101,108,46,119,114,105,116,101}))));
values.push_back(Property(h::debugText(units({112,104,97,115,101,47,110,117,108,108}))));
values.push_back(Property(h::debugText(units({97,116,116,114,105,98,117,116,101,115}))));
values.push_back(Property(PropertyArray{values[14],values[15],values[16],values[17]}));
values.push_back(Property(PropertyList{}));
values.push_back(Property(h::debugText(units({105,110,102,101,114,114,101,100,77,114,111,119}))));
values.push_back(Property(PropertyArray{values[20]}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"properties",values[19]},{"children",values[21]}}));
values.push_back(Property(PropertyList{{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"properties",values[19]},{"children",values[21]}}));
values.push_back(Property(PropertyArray{}));
values.push_back(Property(PropertyList{{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"properties",values[19]},{"children",values[24]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"properties",values[19]},{"children",values[24]}}));
values.push_back(Property(h::debugText(units({99,104,105,108,100,47,102,105,114,115,116,47,110,117,108,108}))));
values.push_back(Property(h::debugText(units({117,112,100,97,116,101,47,102,105,114,115,116}))));
values.push_back(Property(h::debugText(units({116,101,120,46,119,114,105,116,101}))));
values.push_back(Property(PropertyArray{values[27],values[28],values[14],values[15],values[29]}));
values.push_back(Property(2.0));
values.push_back(Property(h::debugText(units({102,105,114,115,116,45,112,114,101,118}))));
values.push_back(Property(h::debugText(units({102,105,114,115,116,45,108,101,118,101,108}))));
values.push_back(Property(h::debugText(units({102,105,114,115,116}))));
values.push_back(Property(PropertyArray{values[34]}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[32]},{"level",values[33]},{"properties",values[19]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[4]},{"level",values[4]},{"properties",values[19]},{"children",values[24]}}));
values.push_back(Property(5.0));
values.push_back(Property(h::debugText(units({108,97,116,101,45,112,114,101,118}))));
values.push_back(Property(h::debugText(units({108,97,116,101,45,108,101,118,101,108}))));
values.push_back(Property(PropertyList{{"tex",values[38]},{"prev",values[39]},{"level",values[40]},{"properties",values[19]},{"children",values[24]}}));
values.push_back(Property(h::debugText(units({99,104,105,108,100,47,102,105,114,115,116,47,112,114,101,118}))));
values.push_back(Property(PropertyArray{values[42],values[28],values[14],values[15],values[29]}));
values.push_back(Property(h::debugText(units({117,112,100,97,116,101,47,108,97,116,101}))));
values.push_back(Property(PropertyArray{values[27],values[44],values[14],values[15],values[29]}));
values.push_back(Property(h::debugText(units({108,97,116,101}))));
values.push_back(Property(PropertyArray{values[46]}));
values.push_back(Property(PropertyList{{"tex",values[38]},{"prev",values[39]},{"level",values[40]},{"properties",values[19]},{"children",values[47]}}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[32]},{"level",values[33]},{"properties",values[19]},{"children",values[24]}}));
values.push_back(Property(PropertyList{{"tex",values[38]},{"prev",values[4]},{"level",values[4]},{"properties",values[19]},{"children",values[24]}}));
values.push_back(Property(PropertyArray{values[42],values[44],values[14],values[15],values[29]}));
values.push_back(Property(h::debugText(units({117,112,100,97,116,101,47,110,111,110,101}))));
values.push_back(Property(PropertyArray{values[27],values[52]}));
values.push_back(Property(PropertyArray{values[42],values[52]}));
values.push_back(Property(PropertyArray{values[27]}));
values.push_back(Property(Missing{}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"properties",values[19]},{"children",values[56]}}));
values.push_back(Property(PropertyArray{values[42]}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"properties",values[19]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[32]},{"level",values[33]},{"properties",values[19]},{"children",values[47]}}));
values.push_back(Property(PropertyArray{values[27],values[28]}));
values.push_back(Property(PropertyArray{values[42],values[28]}));
values.push_back(Property(PropertyArray{values[56]}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"properties",values[19]},{"children",values[63]}}));
values.push_back(Property(h::debugText(units({103,101,116,47,118,98,111,120}))));
values.push_back(Property(h::debugText(units({103,101,116,80,114,101,118,47,110,117,108,108}))));
values.push_back(Property(PropertyArray{values[65],values[66],values[29],values[27]}));
values.push_back(Property(true));
values.push_back(Property(PropertyList{{"vbox",values[68]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"properties",values[69]},{"children",values[35]}}));
values.push_back(Property(h::debugText(units({103,101,116,80,114,101,118,47,112,114,101,118}))));
values.push_back(Property(PropertyArray{values[65],values[71],values[14],values[15],values[29],values[27]}));
values.push_back(Property(1.0));
values.push_back(Property(h::debugText(units({50}))));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[69]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"properties",values[69]},{"children",values[47]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[69]},{"children",values[47]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"properties",values[69]},{"children",values[24]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[69]},{"children",values[24]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"properties",values[69]},{"children",values[56]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[69]},{"children",values[56]}}));
values.push_back(Property(h::debugText(units({99,104,105,108,100,47,108,97,116,101,47,110,117,108,108}))));
values.push_back(Property(PropertyArray{values[65],values[66],values[29],values[82]}));
values.push_back(Property(PropertyArray{values[65],values[71],values[14],values[15],values[29],values[82]}));
values.push_back(Property(PropertyArray{values[65],values[66]}));
values.push_back(Property(PropertyList{{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"properties",values[69]},{"children",values[35]}}));
values.push_back(Property(PropertyArray{values[65],values[71]}));
values.push_back(Property(PropertyArray{values[65],values[66],values[29]}));
values.push_back(Property(PropertyArray{values[65],values[71],values[14],values[15],values[29]}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"properties",values[69]},{"children",values[63]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[69]},{"children",values[63]}}));
values.push_back(Property(PropertyList{{"vbox",values[56]}}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[32]},{"level",values[33]},{"properties",values[92]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"vbox",values[4]}}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[32]},{"level",values[33]},{"properties",values[94]},{"children",values[35]}}));
values.push_back(Property(false));
values.push_back(Property(PropertyList{{"vbox",values[96]}}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[32]},{"level",values[33]},{"properties",values[97]},{"children",values[35]}}));
values.push_back(Property(-1.0));
values.push_back(Property(PropertyList{{"vbox",values[99]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[100]},{"children",values[35]}}));
values.push_back(Property(-0.0));
values.push_back(Property(PropertyList{{"vbox",values[102]}}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[32]},{"level",values[33]},{"properties",values[103]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"vbox",values[0]}}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[32]},{"level",values[33]},{"properties",values[105]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"vbox",values[73]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[107]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"vbox",values[31]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[109]},{"children",values[35]}}));
values.push_back(Property(std::numeric_limits<double>::quiet_NaN()));
values.push_back(Property(PropertyList{{"vbox",values[111]}}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[32]},{"level",values[33]},{"properties",values[112]},{"children",values[35]}}));
values.push_back(Property(std::numeric_limits<double>::infinity()));
values.push_back(Property(PropertyList{{"vbox",values[114]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[115]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"vbox",values[5]}}));
values.push_back(Property(PropertyList{{"tex",values[31]},{"prev",values[32]},{"level",values[33]},{"properties",values[117]},{"children",values[35]}}));
values.push_back(Property(h::debugText(units({48}))));
values.push_back(Property(PropertyList{{"vbox",values[119]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[120]},{"children",values[35]}}));
values.push_back(Property(h::debugText(units({102,97,108,115,101}))));
values.push_back(Property(PropertyList{{"vbox",values[122]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[123]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"vbox",values[24]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[125]},{"children",values[35]}}));
values.push_back(Property(PropertyArray{values[0]}));
values.push_back(Property(PropertyList{{"vbox",values[127]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[128]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"vbox",values[19]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[130]},{"children",values[35]}}));
values.push_back(Property(PropertyList{{"value",values[96]}}));
values.push_back(Property(PropertyList{{"vbox",values[132]}}));
values.push_back(Property(PropertyList{{"tex",values[0]},{"prev",values[73]},{"level",values[74]},{"properties",values[133]},{"children",values[35]}}));
values.push_back(Property(h::debugText(units({101,114,114,111,114,47,85,110,107,110,111,119,110,32,97,116,116,114,105,98,117,116,101,115,32,102,111,114,32,109,103,108,121,112,104,32,110,111,100,101,58,32,102,111,110,116,102,97,109,105,108,121,44,32,117,110,107,110,111,119,110,47,102,97,108,115,101}))));
values.push_back(Property(h::debugText(units({103,101,116,47,105,103,110,111,114,101,45,118,97,114,105,97,110,116}))));
values.push_back(Property(h::debugText(units({101,114,114,111,114,47,73,110,118,97,108,105,100,32,109,97,116,104,118,97,114,105,97,110,116,58,32,98,111,103,117,115,47,116,114,117,101}))));
values.push_back(Property(PropertyArray{values[17],values[17],values[135],values[17],values[136],values[137]}));
values.push_back(Property(PropertyList{{"checkAttributes",values[68]},{"checkMathvariants",values[68]},{"observed",values[68]}}));
values.push_back(Property(h::debugText(units({118,97,108,105,100}))));
values.push_back(Property(PropertyArray{values[5]}));
values.push_back(Property(h::debugText(units({101,114,114,111,114,47,109,103,108,121,112,104,32,109,117,115,116,32,104,97,118,101,32,101,105,116,104,101,114,32,115,114,99,32,111,114,32,102,111,110,116,102,97,109,105,108,121,32,97,110,100,32,105,110,100,101,120,32,97,116,116,114,105,98,117,116,101,115,47,116,114,117,101}))));
values.push_back(Property(PropertyArray{values[17],values[142]}));
values.push_back(Property(PropertyList{{"checkAttributes",values[96]},{"checkMathvariants",values[96]}}));
values.push_back(Property(PropertyArray{values[17]}));
values.push_back(Property(PropertyList{{"checkAttributes",values[96]},{"checkMathvariants",values[96]},{"observed",values[68]}}));
values.push_back(Property(PropertyList{{"src",values[5]},{"fontfamily",values[56]},{"index",values[5]}}));
values.push_back(Property(h::debugText(units({105,109,97,103,101}))));
values.push_back(Property(PropertyList{{"src",values[148]},{"fontfamily",values[56]},{"index",values[5]}}));
values.push_back(Property(PropertyArray{values[17],values[17],values[17]}));
values.push_back(Property(PropertyList{{"checkAttributes",values[68]},{"checkMathvariants",values[68]}}));
values.push_back(Property(h::debugText(units({102,111,110,116}))));
values.push_back(Property(h::debugText(units({52,50}))));
values.push_back(Property(PropertyList{{"src",values[5]},{"fontfamily",values[152]},{"index",values[153]}}));
values.push_back(Property(PropertyList{{"src",values[5]},{"fontfamily",values[152]},{"index",values[5]}}));
values.push_back(Property(PropertyList{{"src",values[5]}}));
values.push_back(Property(PropertyList{{"src",values[56]}}));
values.push_back(Property(PropertyList{{"src",values[56]},{"fontfamily",values[56]},{"index",values[5]}}));
values.push_back(Property(PropertyList{{"fontfamily",values[5]}}));
values.push_back(Property(PropertyList{{"src",values[5]},{"fontfamily",values[5]},{"index",values[153]}}));
values.push_back(Property(PropertyList{{"index",values[5]}}));
values.push_back(Property(h::debugText(units({97,110,99,101,115,116,111,114}))));
values.push_back(Property(h::debugText(units({114,101,100}))));
values.push_back(Property(PropertyArray{values[162],values[163]}));
values.push_back(Property(PropertyList{{"color",values[164]}}));
values.push_back(Property(h::debugText(units({109,101,114,103,101}))));
values.push_back(Property(h::debugText(units({105,110,104,101,114,105,116,47,102,105,114,115,116}))));
values.push_back(Property(h::debugText(units({105,110,104,101,114,105,116,47,108,97,116,101}))));
values.push_back(Property(PropertyArray{values[17],values[166],values[167],values[168]}));
values.push_back(Property(h::debugText(units({109,97,108,105,103,110,103,114,111,117,112}))));
values.push_back(Property(h::debugText(units({114,105,103,104,116}))));
values.push_back(Property(PropertyArray{values[170],values[171]}));
values.push_back(Property(h::debugText(units({98,108,117,101}))));
values.push_back(Property(PropertyArray{values[170],values[173]}));
values.push_back(Property(PropertyList{{"color",values[164]},{"groupalign",values[172]},{"mathcolor",values[174]}}));
values.push_back(Property(h::debugText(units({100,105,115,112,108,97,121}))));
values.push_back(Property(h::debugText(units({112,114,105,109,101}))));
values.push_back(Property(PropertyList{{"a",values[175]},{"d",values[176]},{"l",values[74]},{"p",values[177]}}));
values.push_back(Property(PropertyList{{"a",values[165]},{"d",values[176]},{"l",values[74]},{"p",values[177]}}));
values.push_back(Property(h::debugText(units({111,114,105,103,105,110}))));
values.push_back(Property(h::debugText(units({114,97,119}))));
values.push_back(Property(PropertyArray{values[180],values[181]}));
values.push_back(Property(PropertyList{{"color",values[164]},{"extra",values[182]},{"groupalign",values[172]},{"mathcolor",values[174]}}));
values.push_back(Property(PropertyList{{"a",values[183]},{"d",values[176]},{"l",values[74]},{"p",values[177]}}));
values.push_back(Property(PropertyList{{"color",values[164]},{"extra",values[182]}}));
values.push_back(Property(PropertyArray{values[17],values[166],values[168]}));
values.push_back(Property(PropertyArray{values[17],values[166]}));
values.push_back(Property(PropertyArray{values[17],values[166],values[167],values[168],values[168]}));
values.push_back(Property(h::debugText(units({114,101,99,111,114,100}))));
values.push_back(Property(PropertyArray{values[189],values[46]}));
values.push_back(Property(PropertyList{{"color",values[164]},{"groupalign",values[172]},{"mathcolor",values[174]},{"seen",values[190]}}));
values.push_back(Property(PropertyList{{"a",values[191]},{"d",values[176]},{"l",values[74]},{"p",values[177]}}));
values.push_back(Property(PropertyArray{values[17],values[166],values[167]}));
values.push_back(Property(h::debugText(units({118,97,108,117,101}))));
values.push_back(Property(PropertyArray{values[162],values[194]}));
values.push_back(Property(PropertyList{{"shared",values[195]}}));
values.push_back(Property(PropertyArray{values[31]}));
values.push_back(Property(h::debugText(units({109,114,111,119}))));
values.push_back(Property(h::debugText(units({109,101,110,99,108,111,115,101}))));
values.push_back(Property(PropertyList{{"texprimestyle",values[68]}}));
values.push_back(Property(PropertyList{{"mathcolor",values[173]}}));
values.push_back(Property(h::debugText(units({108,97,114,103,101}))));
values.push_back(Property(PropertyList{{"mathsize",values[202]},{"displaystyle",values[68]},{"scriptlevel",values[74]}}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[203]},{"children",values[24]}}));
values.push_back(Property(PropertyArray{values[204]}));
values.push_back(Property(PropertyList{{"kind",values[199]},{"text",values[56]},{"properties",values[200]},{"tex",values[0]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[205]}}));
values.push_back(Property(PropertyArray{values[206]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[207]}}));
values.push_back(Property(h::debugText(units({109,105}))));
values.push_back(Property(h::debugText(units({105,116,97,108,105,99}))));
values.push_back(Property(PropertyList{{"mathsize",values[202]},{"displaystyle",values[68]},{"scriptlevel",values[74]},{"mathvariant",values[210]}}));
values.push_back(Property(h::debugText(units({116,101,120,116}))));
values.push_back(Property(h::debugText(units({120}))));
values.push_back(Property(PropertyList{{"kind",values[212]},{"text",values[213]},{"properties",values[19]},{"tex",values[99]},{"attributes",values[56]},{"inherited",values[56]},{"children",values[24]}}));
values.push_back(Property(PropertyArray{values[214]}));
values.push_back(Property(PropertyList{{"kind",values[209]},{"text",values[56]},{"properties",values[200]},{"tex",values[0]},{"attributes",values[19]},{"inherited",values[211]},{"children",values[215]}}));
values.push_back(Property(PropertyArray{values[216]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[203]},{"children",values[217]}}));
values.push_back(Property(PropertyArray{values[218]}));
values.push_back(Property(PropertyList{{"kind",values[199]},{"text",values[56]},{"properties",values[200]},{"tex",values[0]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[219]}}));
values.push_back(Property(PropertyArray{values[220]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[221]}}));
values.push_back(Property(PropertyArray{values[216],values[216]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[203]},{"children",values[223]}}));
values.push_back(Property(PropertyArray{values[224]}));
values.push_back(Property(PropertyList{{"kind",values[199]},{"text",values[56]},{"properties",values[200]},{"tex",values[0]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[225]}}));
values.push_back(Property(PropertyArray{values[226]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[227]}}));
values.push_back(Property(h::debugText(units({109,112,97,100,100,101,100}))));
values.push_back(Property(PropertyList{{"kind",values[229]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[205]}}));
values.push_back(Property(PropertyArray{values[230]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[231]}}));
values.push_back(Property(PropertyList{{"kind",values[229]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[219]}}));
values.push_back(Property(PropertyArray{values[233]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[234]}}));
values.push_back(Property(PropertyList{{"kind",values[229]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[225]}}));
values.push_back(Property(PropertyArray{values[236]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[237]}}));
values.push_back(Property(PropertyList{{"mathsize",values[202]},{"mathcolor",values[173]},{"displaystyle",values[68]},{"scriptlevel",values[74]}}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[239]},{"children",values[24]}}));
values.push_back(Property(PropertyArray{values[240]}));
values.push_back(Property(PropertyList{{"kind",values[170]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[241]}}));
values.push_back(Property(PropertyArray{values[242]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[243]}}));
values.push_back(Property(PropertyList{{"mathsize",values[202]},{"mathcolor",values[173]},{"displaystyle",values[68]},{"scriptlevel",values[74]},{"mathvariant",values[210]}}));
values.push_back(Property(PropertyList{{"kind",values[209]},{"text",values[56]},{"properties",values[200]},{"tex",values[0]},{"attributes",values[19]},{"inherited",values[245]},{"children",values[215]}}));
values.push_back(Property(PropertyArray{values[246]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[239]},{"children",values[247]}}));
values.push_back(Property(PropertyArray{values[248]}));
values.push_back(Property(PropertyList{{"kind",values[170]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[249]}}));
values.push_back(Property(PropertyArray{values[250]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[251]}}));
values.push_back(Property(PropertyArray{values[246],values[246]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[239]},{"children",values[253]}}));
values.push_back(Property(PropertyArray{values[254]}));
values.push_back(Property(PropertyList{{"kind",values[170]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[255]}}));
values.push_back(Property(PropertyArray{values[256]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[257]}}));
values.push_back(Property(h::debugText(units({109,97,108,105,103,110,109,97,114,107}))));
values.push_back(Property(PropertyList{{"kind",values[259]},{"text",values[56]},{"properties",values[200]},{"tex",values[4]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[24]}}));
values.push_back(Property(PropertyArray{values[260]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[261]}}));
values.push_back(Property(h::debugText(units({109,103,108,121,112,104}))));
values.push_back(Property(PropertyList{{"kind",values[263]},{"text",values[56]},{"properties",values[200]},{"tex",values[0]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[24]}}));
values.push_back(Property(PropertyArray{values[264]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[265]}}));
values.push_back(Property(PropertyList{{"kind",values[263]},{"text",values[56]},{"properties",values[200]},{"tex",values[0]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[217]}}));
values.push_back(Property(PropertyArray{values[267]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[268]}}));
values.push_back(Property(PropertyList{{"kind",values[263]},{"text",values[56]},{"properties",values[200]},{"tex",values[0]},{"attributes",values[201]},{"inherited",values[203]},{"children",values[223]}}));
values.push_back(Property(PropertyArray{values[270]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[271]}}));
values.push_back(Property(h::debugText(units({109,101,114,114,111,114}))));
values.push_back(Property(h::debugText(units({109,103,108,121,112,104,32,109,117,115,116,32,104,97,118,101,32,101,105,116,104,101,114,32,115,114,99,32,111,114,32,102,111,110,116,102,97,109,105,108,121,32,97,110,100,32,105,110,100,101,120,32,97,116,116,114,105,98,117,116,101,115}))));
values.push_back(Property(PropertyList{{"data-mjx-message",values[274]},{"title",values[274]}}));
values.push_back(Property(h::debugText(units({109,116,101,120,116}))));
values.push_back(Property(PropertyList{{"kind",values[212]},{"text",values[263]},{"properties",values[19]},{"tex",values[99]},{"attributes",values[56]},{"inherited",values[56]},{"children",values[24]}}));
values.push_back(Property(PropertyArray{values[277]}));
values.push_back(Property(PropertyList{{"kind",values[276]},{"text",values[56]},{"properties",values[19]},{"tex",values[0]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[278]}}));
values.push_back(Property(PropertyArray{values[279]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[280]}}));
values.push_back(Property(PropertyArray{values[281]}));
values.push_back(Property(PropertyList{{"kind",values[273]},{"text",values[56]},{"properties",values[19]},{"tex",values[0]},{"attributes",values[275]},{"inherited",values[19]},{"children",values[282]}}));
values.push_back(Property(PropertyArray{values[283]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[284]}}));
values.push_back(Property(PropertyList{{"data-mjx-message",values[274]}}));
values.push_back(Property(PropertyList{{"kind",values[212]},{"text",values[274]},{"properties",values[19]},{"tex",values[99]},{"attributes",values[56]},{"inherited",values[56]},{"children",values[24]}}));
values.push_back(Property(PropertyArray{values[287]}));
values.push_back(Property(PropertyList{{"kind",values[276]},{"text",values[56]},{"properties",values[19]},{"tex",values[0]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[288]}}));
values.push_back(Property(PropertyArray{values[289]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[290]}}));
values.push_back(Property(PropertyArray{values[291]}));
values.push_back(Property(PropertyList{{"kind",values[273]},{"text",values[56]},{"properties",values[19]},{"tex",values[0]},{"attributes",values[286]},{"inherited",values[19]},{"children",values[292]}}));
values.push_back(Property(PropertyArray{values[293]}));
values.push_back(Property(PropertyList{{"kind",values[198]},{"text",values[56]},{"properties",values[19]},{"tex",values[4]},{"attributes",values[19]},{"inherited",values[19]},{"children",values[294]}}));
return values;}();}
suite math_port_simple_mml_tests=[] {
"math_port_simple_mml_defaults_getters_fields"_test=[] {reset();
NF f(bindings());
{auto*n=create(f,"menclose");
expect(bool((n->kind())=="menclose"));
expect(bool((n->arity())==-1));
expect(bool((n->isToken())==false));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==false));
expect(bool((n->linebreakContainer())==true));
expect(bool((n->notParent())==false));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(Property(m::MmlMenclose::defaults()),clone(sourceValues[3])));
expect(bool((Property(m::MmlMenclose::defaults())==Property(m::AbstractMmlNode::defaults()))==false));
expect(bool((n->childNodes&&(*n->childNodes)[0]->isInferred())==true));
}
{auto*n=create(f,"mpadded");
expect(bool((n->kind())=="mpadded"));
expect(bool((n->arity())==-1));
expect(bool((n->isToken())==false));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==true));
expect(bool((n->linebreakContainer())==true));
expect(bool((n->notParent())==false));
expect(deep(n->texClass(),clone(sourceValues[4])));
expect(deep(Property(m::MmlMpadded::defaults()),clone(sourceValues[6])));
expect(bool((Property(m::MmlMpadded::defaults())==Property(m::AbstractMmlLayoutNode::defaults()))==false));
expect(bool((n->childNodes&&(*n->childNodes)[0]->isInferred())==true));
}
{auto*n=create(f,"maligngroup");
expect(bool((n->kind())=="maligngroup"));
expect(bool((n->arity())==-1));
expect(bool((n->isToken())==false));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==true));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->notParent())==false));
expect(deep(n->texClass(),clone(sourceValues[4])));
expect(deep(Property(m::MmlMaligngroup::defaults()),clone(sourceValues[7])));
expect(bool((Property(m::MmlMaligngroup::defaults())==Property(m::AbstractMmlLayoutNode::defaults()))==false));
expect(bool((n->childNodes&&(*n->childNodes)[0]->isInferred())==true));
}
{auto*n=create(f,"malignmark");
expect(bool((n->kind())=="malignmark"));
expect(bool((n->arity())==0));
expect(bool((n->isToken())==false));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==true));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->notParent())==false));
expect(deep(n->texClass(),clone(sourceValues[4])));
expect(deep(Property(m::MmlMalignmark::defaults()),clone(sourceValues[9])));
expect(bool((Property(m::MmlMalignmark::defaults())==Property(m::AbstractMmlNode::defaults()))==false));
}
{auto*n=create(f,"mglyph");
expect(bool((n->kind())=="mglyph"));
expect(bool((n->arity())==std::numeric_limits<double>::infinity()));
expect(bool((n->isToken())==true));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==false));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->notParent())==false));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(Property(m::MmlMglyph::defaults()),clone(sourceValues[13])));
expect(bool((Property(m::MmlMglyph::defaults())==Property(m::AbstractMmlTokenNode::defaults()))==false));
}
enabled=false;};
"math_port_simple_mml_constructor_menclose"_test=[] {reset();
NF f(bindings());phase=true;m::MmlNode*n=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{n=create(f,"probe-menclose");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[18])));
}
phase=false;
expect(deep(state(n),clone(sourceValues[22])));
expect(bool((Property(phaseBag)==Property(n->getAllProperties()))==true));
enabled=false;};
"math_port_simple_mml_constructor_mpadded"_test=[] {reset();
NF f(bindings());phase=true;m::MmlNode*n=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{n=create(f,"probe-mpadded");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[18])));
}
phase=false;
expect(deep(state(n),clone(sourceValues[23])));
expect(bool((Property(phaseBag)==Property(n->getAllProperties()))==true));
enabled=false;};
"math_port_simple_mml_constructor_maligngroup"_test=[] {reset();
NF f(bindings());phase=true;m::MmlNode*n=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{n=create(f,"probe-maligngroup");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[18])));
}
phase=false;
expect(deep(state(n),clone(sourceValues[23])));
expect(bool((Property(phaseBag)==Property(n->getAllProperties()))==true));
enabled=false;};
"math_port_simple_mml_constructor_malignmark"_test=[] {reset();
NF f(bindings());phase=true;m::MmlNode*n=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{n=create(f,"probe-malignmark");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[18])));
}
phase=false;
expect(deep(state(n),clone(sourceValues[25])));
expect(bool((Property(phaseBag)==Property(n->getAllProperties()))==true));
enabled=false;};
"math_port_simple_mml_constructor_mglyph"_test=[] {reset();
NF f(bindings());phase=true;m::MmlNode*n=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{n=create(f,"probe-mglyph");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[18])));
}
phase=false;
expect(deep(state(n),clone(sourceValues[26])));
expect(bool((Property(phaseBag)==Property(n->getAllProperties()))==true));
enabled=false;};
"math_port_simple_mml_class_menclose_normal_false"_test=[] {reset();
Setup s("probe-menclose");mode="";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[30])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[36])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_normal_true"_test=[] {reset();
Setup s("probe-menclose");mode="";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[43])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[36])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_child-rebind_false"_test=[] {reset();
Setup s("probe-menclose");mode="child-rebind";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[45])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[48])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[50])));
enabled=false;};
"math_port_simple_mml_class_menclose_child-rebind_true"_test=[] {reset();
Setup s("probe-menclose");mode="child-rebind";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[51])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[48])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[50])));
enabled=false;};
"math_port_simple_mml_class_menclose_child-empty_false"_test=[] {reset();
Setup s("probe-menclose");mode="child-empty";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[53])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[26])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_child-empty_true"_test=[] {reset();
Setup s("probe-menclose");mode="child-empty";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[54])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[26])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_child-null-array_false"_test=[] {reset();
Setup s("probe-menclose");mode="child-null-array";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[55])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[57])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_child-null-array_true"_test=[] {reset();
Setup s("probe-menclose");mode="child-null-array";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[58])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[57])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_child-throw_false"_test=[] {reset();
Setup s("probe-menclose");mode="child-throw";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[55])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[59])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_child-throw_true"_test=[] {reset();
Setup s("probe-menclose");mode="child-throw";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[58])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[59])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_child-return-prev_false"_test=[] {reset();
Setup s("probe-menclose");mode="child-return-prev";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[30])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[36])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_child-return-prev_true"_test=[] {reset();
Setup s("probe-menclose");mode="child-return-prev";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[43])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==true));
expect(deep(state(s.n),clone(sourceValues[36])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_get-rebind_false"_test=[] {reset();
Setup s("probe-menclose");mode="get-rebind";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[30])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[36])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_get-rebind_true"_test=[] {reset();
Setup s("probe-menclose");mode="get-rebind";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[43])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[36])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_prev-rebind_false"_test=[] {reset();
Setup s("probe-menclose");mode="prev-rebind";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[30])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[36])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_prev-rebind_true"_test=[] {reset();
Setup s("probe-menclose");mode="prev-rebind";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[43])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[36])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_prev-throw_false"_test=[] {reset();
Setup s("probe-menclose");mode="prev-throw";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[30])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[36])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_prev-throw_true"_test=[] {reset();
Setup s("probe-menclose");mode="prev-throw";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[43])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[36])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_class-rebind_false"_test=[] {reset();
Setup s("probe-menclose");mode="class-rebind";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[30])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[60])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_class-rebind_true"_test=[] {reset();
Setup s("probe-menclose");mode="class-rebind";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[43])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==true));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[60])));
expect(deep(state(s.a),clone(sourceValues[37])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_update-throw_false"_test=[] {reset();
Setup s("probe-menclose");mode="update-throw";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[61])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[59])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_update-throw_true"_test=[] {reset();
Setup s("probe-menclose");mode="update-throw";m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[62])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[59])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_empty_false"_test=[] {reset();
Setup s("probe-menclose");mode="empty";s.n->childNodes=children();m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[26])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_empty_true"_test=[] {reset();
Setup s("probe-menclose");mode="empty";s.n->childNodes=children();m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[26])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_null-child_false"_test=[] {reset();
Setup s("probe-menclose");mode="null-child";s.n->childNodes=children({nullptr});m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[64])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_null-child_true"_test=[] {reset();
Setup s("probe-menclose");mode="null-child";s.n->childNodes=children({nullptr});m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[64])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_null-array_false"_test=[] {reset();
Setup s("probe-menclose");mode="null-array";s.n->childNodes.reset();m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[57])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_menclose_null-array_true"_test=[] {reset();
Setup s("probe-menclose");mode="null-array";s.n->childNodes.reset();m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[57])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_normal_false"_test=[] {reset();
Setup s("probe-mpadded");mode="";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[67])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[70])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_normal_true"_test=[] {reset();
Setup s("probe-mpadded");mode="";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[72])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[75])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_child-rebind_false"_test=[] {reset();
Setup s("probe-mpadded");mode="child-rebind";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[67])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[76])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_child-rebind_true"_test=[] {reset();
Setup s("probe-mpadded");mode="child-rebind";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[72])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[77])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_child-empty_false"_test=[] {reset();
Setup s("probe-mpadded");mode="child-empty";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[67])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[78])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_child-empty_true"_test=[] {reset();
Setup s("probe-mpadded");mode="child-empty";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[72])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[79])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_child-null-array_false"_test=[] {reset();
Setup s("probe-mpadded");mode="child-null-array";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[67])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[80])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_child-null-array_true"_test=[] {reset();
Setup s("probe-mpadded");mode="child-null-array";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[72])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[81])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_child-throw_false"_test=[] {reset();
Setup s("probe-mpadded");mode="child-throw";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[67])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[70])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_child-throw_true"_test=[] {reset();
Setup s("probe-mpadded");mode="child-throw";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[72])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[75])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_child-return-prev_false"_test=[] {reset();
Setup s("probe-mpadded");mode="child-return-prev";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[67])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[70])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_child-return-prev_true"_test=[] {reset();
Setup s("probe-mpadded");mode="child-return-prev";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[72])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[75])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_get-rebind_false"_test=[] {reset();
Setup s("probe-mpadded");mode="get-rebind";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[83])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[76])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_get-rebind_true"_test=[] {reset();
Setup s("probe-mpadded");mode="get-rebind";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[84])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[77])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_prev-rebind_false"_test=[] {reset();
Setup s("probe-mpadded");mode="prev-rebind";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[83])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[76])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_prev-rebind_true"_test=[] {reset();
Setup s("probe-mpadded");mode="prev-rebind";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[84])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[77])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_prev-throw_false"_test=[] {reset();
Setup s("probe-mpadded");mode="prev-throw";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[85])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[86])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_prev-throw_true"_test=[] {reset();
Setup s("probe-mpadded");mode="prev-throw";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[87])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[86])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_class-rebind_false"_test=[] {reset();
Setup s("probe-mpadded");mode="class-rebind";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[83])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[76])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_class-rebind_true"_test=[] {reset();
Setup s("probe-mpadded");mode="class-rebind";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[84])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[77])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_update-throw_false"_test=[] {reset();
Setup s("probe-mpadded");mode="update-throw";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[67])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==false));
expect(deep(state(s.n),clone(sourceValues[70])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_update-throw_true"_test=[] {reset();
Setup s("probe-mpadded");mode="update-throw";s.n->setProperty("vbox",true);m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[72])));
}
expect(bool((result==s.n)==true));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[75])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_empty_false"_test=[] {reset();
Setup s("probe-mpadded");mode="empty";s.n->setProperty("vbox",true);s.n->childNodes=children();m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[88])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[78])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_empty_true"_test=[] {reset();
Setup s("probe-mpadded");mode="empty";s.n->setProperty("vbox",true);s.n->childNodes=children();m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[89])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[79])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_null-child_false"_test=[] {reset();
Setup s("probe-mpadded");mode="null-child";s.n->setProperty("vbox",true);s.n->childNodes=children({nullptr});m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[88])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[90])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_null-child_true"_test=[] {reset();
Setup s("probe-mpadded");mode="null-child";s.n->setProperty("vbox",true);s.n->childNodes=children({nullptr});m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[89])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[91])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_null-array_false"_test=[] {reset();
Setup s("probe-mpadded");mode="null-array";s.n->setProperty("vbox",true);s.n->childNodes.reset();m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(nullptr);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[88])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==nullptr)==true));
expect(deep(state(s.n),clone(sourceValues[80])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_class_mpadded_null-array_true"_test=[] {reset();
Setup s("probe-mpadded");mode="null-array";s.n->setProperty("vbox",true);s.n->childNodes.reset();m::MmlNode*result=nullptr;
{enabled=true;simpleEvents.clear();std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[89])));
}
expect(bool((result==s.n)==false));
expect(bool((result==s.b)==false));
expect(bool((result==s.prev)==false));
expect(deep(state(s.n),clone(sourceValues[81])));
expect(deep(state(s.a),clone(sourceValues[49])));
expect(deep(state(s.b),clone(sourceValues[41])));
enabled=false;};
"math_port_simple_mml_raw_vbox_matrix"_test=[] {reset();
struct Row{Property input;std::string error;bool self,returned;Property n,a;};const Row rows[]={
{clone(sourceValues[56]),"",false,true,clone(sourceValues[93]),clone(sourceValues[37])},
{clone(sourceValues[4]),"",false,true,clone(sourceValues[95]),clone(sourceValues[37])},
{clone(sourceValues[96]),"",false,true,clone(sourceValues[98]),clone(sourceValues[37])},
{clone(sourceValues[68]),"",true,false,clone(sourceValues[75]),clone(sourceValues[49])},
{clone(sourceValues[99]),"",true,false,clone(sourceValues[101]),clone(sourceValues[49])},
{clone(sourceValues[102]),"",false,true,clone(sourceValues[104]),clone(sourceValues[37])},
{clone(sourceValues[0]),"",false,true,clone(sourceValues[106]),clone(sourceValues[37])},
{clone(sourceValues[73]),"",true,false,clone(sourceValues[108]),clone(sourceValues[49])},
{clone(sourceValues[31]),"",true,false,clone(sourceValues[110]),clone(sourceValues[49])},
{clone(sourceValues[111]),"",false,true,clone(sourceValues[113]),clone(sourceValues[37])},
{clone(sourceValues[114]),"",true,false,clone(sourceValues[116]),clone(sourceValues[49])},
{clone(sourceValues[5]),"",false,true,clone(sourceValues[118]),clone(sourceValues[37])},
{clone(sourceValues[119]),"",true,false,clone(sourceValues[121]),clone(sourceValues[49])},
{clone(sourceValues[122]),"",true,false,clone(sourceValues[124]),clone(sourceValues[49])},
{clone(sourceValues[24]),"",true,false,clone(sourceValues[126]),clone(sourceValues[49])},
{clone(sourceValues[127]),"",true,false,clone(sourceValues[129]),clone(sourceValues[49])},
{clone(sourceValues[19]),"",true,false,clone(sourceValues[131]),clone(sourceValues[49])},
{clone(sourceValues[132]),"",true,false,clone(sourceValues[134]),clone(sourceValues[49])},
};for(const auto&r:rows){Setup s("probe-mpadded");s.n->setProperty("vbox",clone(r.input));m::MmlNode*result=nullptr;std::string failure;try{result=s.n->setTeXclass(s.prev);}catch(const m::Error&e){failure=e.name;}expect(failure==r.error);expect((result==s.n)==r.self);expect((result==s.b)==r.returned);expect(deep(state(s.n),r.n));expect(deep(state(s.a),r.a));}
enabled=false;};
"math_port_simple_mml_raw_glyph_attribute_matrix"_test=[] {reset();
struct Row{Property src,family,index;std::string error;Property trace,opts;bool alias;};const Row rows[]={
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[143]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[140]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[141]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[140]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[141]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[56]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[4]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[96]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[68]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[0]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[73]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[5]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[140]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[24]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[141]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[19]),"",clone(sourceValues[138]),clone(sourceValues[139]),true},
};for(const auto&r:rows){Setup s("probe-mglyph");s.n->attributes()->setList(PropertyList{{"src",clone(r.src)},{"fontfamily",clone(r.family)},{"index",clone(r.index)},{"unknown","raw"},{"mathvariant","bogus"}});PropertyList opts{{"checkAttributes",true},{"checkMathvariants",true}};enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;expect(failure==r.error);expect(deep(eventValue(),r.trace));expect(deep(Property(opts),r.opts));expect((errorOptions&&Property(*errorOptions)==Property(opts))==r.alias);}
enabled=false;};
"math_port_simple_mml_glyph_error-throw"_test=[] {reset();
Setup s("probe-mglyph");mode="error-throw";s.n->attributes()->setList(PropertyList{{"src",""},{"fontfamily",""},{"index",""}});PropertyList opts{{"checkAttributes",false},{"checkMathvariants",false}};
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[143])));
}
expect(deep(Property(opts),clone(sourceValues[144])));
enabled=false;};
"math_port_simple_mml_glyph_attributes-null"_test=[] {reset();
Setup s("probe-mglyph");mode="attributes-null";s.n->setAttributes(nullptr);PropertyList opts{{"checkAttributes",false},{"checkMathvariants",false}};
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[145])));
}
expect(deep(Property(opts),clone(sourceValues[144])));
enabled=false;};
"math_port_simple_mml_glyph_valid-no-checks"_test=[] {reset();
Setup s("probe-mglyph");mode="valid-no-checks";s.n->attributes()->setList(PropertyList{{"src","image"},{"fontfamily",""},{"index",""}});PropertyList opts{{"checkAttributes",false},{"checkMathvariants",false}};
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[145])));
}
expect(deep(Property(opts),clone(sourceValues[144])));
enabled=false;};
"math_port_simple_mml_glyph_invalid-no-checks"_test=[] {reset();
Setup s("probe-mglyph");mode="invalid-no-checks";s.n->attributes()->setList(PropertyList{{"src",""},{"fontfamily",""},{"index",""}});PropertyList opts{{"checkAttributes",false},{"checkMathvariants",false}};
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[143])));
}
expect(deep(Property(opts),clone(sourceValues[146])));
enabled=false;};
"math_port_simple_mml_glyph_lookup_defaults"_test=[] {reset();
Setup s("probe-mglyph");PropertyList opts{{"checkAttributes",true},{"checkMathvariants",true}};
s.n->attributes()->setList(PropertyList::record(clone(sourceValues[19])).value());
expect(deep(Property(s.n->attributes()->getList({"src","fontfamily","index"})),clone(sourceValues[147])));
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[143])));
}
expect(deep(Property(opts),clone(sourceValues[139])));
expect(bool((errorOptions.has_value()&&Property(*errorOptions)==Property(opts))==true));
enabled=false;};
"math_port_simple_mml_glyph_lookup_inherited-src"_test=[] {reset();
Setup s("probe-mglyph");PropertyList opts{{"checkAttributes",true},{"checkMathvariants",true}};
s.n->attributes()->setInherited("src",clone(sourceValues[148]));
s.n->attributes()->setList(PropertyList::record(clone(sourceValues[19])).value());
expect(deep(Property(s.n->attributes()->getList({"src","fontfamily","index"})),clone(sourceValues[149])));
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[150])));
}
expect(deep(Property(opts),clone(sourceValues[151])));
expect(bool((errorOptions.has_value()&&Property(*errorOptions)==Property(opts))==false));
enabled=false;};
"math_port_simple_mml_glyph_lookup_inherited-family-index"_test=[] {reset();
Setup s("probe-mglyph");PropertyList opts{{"checkAttributes",true},{"checkMathvariants",true}};
s.n->attributes()->setInherited("fontfamily",clone(sourceValues[152]));
s.n->attributes()->setInherited("index",clone(sourceValues[153]));
s.n->attributes()->setList(PropertyList::record(clone(sourceValues[19])).value());
expect(deep(Property(s.n->attributes()->getList({"src","fontfamily","index"})),clone(sourceValues[154])));
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[150])));
}
expect(deep(Property(opts),clone(sourceValues[151])));
expect(bool((errorOptions.has_value()&&Property(*errorOptions)==Property(opts))==false));
enabled=false;};
"math_port_simple_mml_glyph_lookup_inherited-family-empty-index"_test=[] {reset();
Setup s("probe-mglyph");PropertyList opts{{"checkAttributes",true},{"checkMathvariants",true}};
s.n->attributes()->setInherited("fontfamily",clone(sourceValues[152]));
s.n->attributes()->setInherited("index",clone(sourceValues[5]));
s.n->attributes()->setList(PropertyList::record(clone(sourceValues[19])).value());
expect(deep(Property(s.n->attributes()->getList({"src","fontfamily","index"})),clone(sourceValues[155])));
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[143])));
}
expect(deep(Property(opts),clone(sourceValues[139])));
expect(bool((errorOptions.has_value()&&Property(*errorOptions)==Property(opts))==true));
enabled=false;};
"math_port_simple_mml_glyph_lookup_explicit-empty-src"_test=[] {reset();
Setup s("probe-mglyph");PropertyList opts{{"checkAttributes",true},{"checkMathvariants",true}};
s.n->attributes()->setInherited("src",clone(sourceValues[148]));
s.n->attributes()->setList(PropertyList::record(clone(sourceValues[156])).value());
expect(deep(Property(s.n->attributes()->getList({"src","fontfamily","index"})),clone(sourceValues[147])));
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[143])));
}
expect(deep(Property(opts),clone(sourceValues[139])));
expect(bool((errorOptions.has_value()&&Property(*errorOptions)==Property(opts))==true));
enabled=false;};
"math_port_simple_mml_glyph_lookup_explicit-missing-src"_test=[] {reset();
Setup s("probe-mglyph");PropertyList opts{{"checkAttributes",true},{"checkMathvariants",true}};
s.n->attributes()->setInherited("src",clone(sourceValues[148]));
s.n->attributes()->setList(PropertyList::record(clone(sourceValues[157])).value());
expect(deep(Property(s.n->attributes()->getList({"src","fontfamily","index"})),clone(sourceValues[158])));
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[150])));
}
expect(deep(Property(opts),clone(sourceValues[151])));
expect(bool((errorOptions.has_value()&&Property(*errorOptions)==Property(opts))==false));
enabled=false;};
"math_port_simple_mml_glyph_lookup_explicit-family-empty"_test=[] {reset();
Setup s("probe-mglyph");PropertyList opts{{"checkAttributes",true},{"checkMathvariants",true}};
s.n->attributes()->setInherited("fontfamily",clone(sourceValues[152]));
s.n->attributes()->setInherited("index",clone(sourceValues[153]));
s.n->attributes()->setList(PropertyList::record(clone(sourceValues[159])).value());
expect(deep(Property(s.n->attributes()->getList({"src","fontfamily","index"})),clone(sourceValues[160])));
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[143])));
}
expect(deep(Property(opts),clone(sourceValues[139])));
expect(bool((errorOptions.has_value()&&Property(*errorOptions)==Property(opts))==true));
enabled=false;};
"math_port_simple_mml_glyph_lookup_explicit-index-empty"_test=[] {reset();
Setup s("probe-mglyph");PropertyList opts{{"checkAttributes",true},{"checkMathvariants",true}};
s.n->attributes()->setInherited("fontfamily",clone(sourceValues[152]));
s.n->attributes()->setInherited("index",clone(sourceValues[153]));
s.n->attributes()->setList(PropertyList::record(clone(sourceValues[161])).value());
expect(deep(Property(s.n->attributes()->getList({"src","fontfamily","index"})),clone(sourceValues[155])));
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<m::MmlMglyph*>(s.n)->verifyAttributes(opts);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[143])));
}
expect(deep(Property(opts),clone(sourceValues[139])));
expect(bool((errorOptions.has_value()&&Property(*errorOptions)==Property(opts))==true));
enabled=false;};
"math_port_simple_mml_group_hook_normal"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="";PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[169])));
}
expect(bool((calls.size())==2));
expect(deep(PropertyList{{"a",calls[0].a},{"d",calls[0].d},{"l",calls[0].l},{"p",calls[0].p}},clone(sourceValues[178])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
expect(bool((Property(calls[0].a)==Property(merged))==true));
expect(deep(PropertyList{{"a",calls[1].a},{"d",calls[1].d},{"l",calls[1].l},{"p",calls[1].p}},clone(sourceValues[178])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(merged))==true));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_group_hook_merge-return-input"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="merge-return-input";PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[169])));
}
expect(bool((calls.size())==2));
expect(deep(PropertyList{{"a",calls[0].a},{"d",calls[0].d},{"l",calls[0].l},{"p",calls[0].p}},clone(sourceValues[179])));
expect(bool((Property(calls[0].a)==Property(attrs))==true));
expect(bool((Property(calls[0].a)==Property(merged))==true));
expect(deep(PropertyList{{"a",calls[1].a},{"d",calls[1].d},{"l",calls[1].l},{"p",calls[1].p}},clone(sourceValues[179])));
expect(bool((Property(calls[1].a)==Property(attrs))==true));
expect(bool((Property(calls[1].a)==Property(merged))==true));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_group_hook_merge-mutate"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="merge-mutate";PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[169])));
}
expect(bool((calls.size())==2));
expect(deep(PropertyList{{"a",calls[0].a},{"d",calls[0].d},{"l",calls[0].l},{"p",calls[0].p}},clone(sourceValues[184])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
expect(bool((Property(calls[0].a)==Property(merged))==true));
expect(deep(PropertyList{{"a",calls[1].a},{"d",calls[1].d},{"l",calls[1].l},{"p",calls[1].p}},clone(sourceValues[184])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(merged))==true));
expect(deep(Property(attrs),clone(sourceValues[185])));
enabled=false;};
"math_port_simple_mml_group_hook_merge-rebind"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="merge-rebind";PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[186])));
}
expect(bool((calls.size())==1));
expect(deep(PropertyList{{"a",calls[0].a},{"d",calls[0].d},{"l",calls[0].l},{"p",calls[0].p}},clone(sourceValues[178])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
expect(bool((Property(calls[0].a)==Property(merged))==true));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_group_hook_merge-throw"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="merge-throw";PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[187])));
}
expect(bool((calls.size())==0));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_group_hook_inherit-rebind"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="inherit-rebind";PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[169])));
}
expect(bool((calls.size())==2));
expect(deep(PropertyList{{"a",calls[0].a},{"d",calls[0].d},{"l",calls[0].l},{"p",calls[0].p}},clone(sourceValues[178])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
expect(bool((Property(calls[0].a)==Property(merged))==true));
expect(deep(PropertyList{{"a",calls[1].a},{"d",calls[1].d},{"l",calls[1].l},{"p",calls[1].p}},clone(sourceValues[178])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(merged))==true));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_group_hook_inherit-append"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="inherit-append";PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[188])));
}
expect(bool((calls.size())==3));
expect(deep(PropertyList{{"a",calls[0].a},{"d",calls[0].d},{"l",calls[0].l},{"p",calls[0].p}},clone(sourceValues[178])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
expect(bool((Property(calls[0].a)==Property(merged))==true));
expect(deep(PropertyList{{"a",calls[1].a},{"d",calls[1].d},{"l",calls[1].l},{"p",calls[1].p}},clone(sourceValues[178])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(merged))==true));
expect(deep(PropertyList{{"a",calls[2].a},{"d",calls[2].d},{"l",calls[2].l},{"p",calls[2].p}},clone(sourceValues[178])));
expect(bool((Property(calls[2].a)==Property(attrs))==false));
expect(bool((Property(calls[2].a)==Property(merged))==true));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_group_hook_inherit-mutate"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="inherit-mutate";PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[169])));
}
expect(bool((calls.size())==2));
expect(deep(PropertyList{{"a",calls[0].a},{"d",calls[0].d},{"l",calls[0].l},{"p",calls[0].p}},clone(sourceValues[192])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
expect(bool((Property(calls[0].a)==Property(merged))==true));
expect(deep(PropertyList{{"a",calls[1].a},{"d",calls[1].d},{"l",calls[1].l},{"p",calls[1].p}},clone(sourceValues[192])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(merged))==true));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_group_hook_inherit-throw"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="inherit-throw";PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(eventValue(),clone(sourceValues[193])));
}
expect(bool((calls.size())==1));
expect(deep(PropertyList{{"a",calls[0].a},{"d",calls[0].d},{"l",calls[0].l},{"p",calls[0].p}},clone(sourceValues[178])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
expect(bool((Property(calls[0].a)==Property(merged))==true));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_group_hook_attributes-null"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="attributes-null";s.n->setAttributes(nullptr);PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[145])));
}
expect(bool((calls.size())==0));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_group_hook_null-child"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="null-child";s.n->childNodes=children({nullptr});PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[187])));
}
expect(bool((calls.size())==0));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_group_hook_null-array"_test=[] {reset();
Setup s("probe-maligngroup");s.n->childNodes=children({s.a,s.b});s.n->attributes()->setList(PropertyList{{"groupalign","right"},{"mathcolor","blue"},{"style","color:red"},{"displaystyle",true},{"scriptlevel","2"}});mode="null-array";s.n->childNodes.reset();PropertyList attrs=PropertyList::record(clone(sourceValues[165])).value();
{enabled=true;simpleEvents.clear();std::string failure;try{dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,"display","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(eventValue(),clone(sourceValues[187])));
}
expect(bool((calls.size())==0));
expect(deep(Property(attrs),clone(sourceValues[165])));
enabled=false;};
"math_port_simple_mml_raw_group_inheritance_matrix"_test=[] {reset();
struct Row{Property d,l,a,display,level,prime;bool alias;};const Row rows[]={
{clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[56]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[4]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[96]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[68]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[99]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[99]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[102]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[102]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[0]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[73]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[31]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[31]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[111]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[111]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[114]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[114]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[5]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[74]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[74]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[24]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[197]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[197]),clone(sourceValues[19]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[56]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[4]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[96]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[68]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[99]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[99]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[102]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[102]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[0]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[73]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[31]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[31]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[111]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[111]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[114]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[114]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[5]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[74]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[74]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[24]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[197]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[197]),clone(sourceValues[177]),false},
{clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[196]),clone(sourceValues[19]),clone(sourceValues[19]),clone(sourceValues[177]),false},
};for(const auto&r:rows){Setup s("probe-maligngroup");PropertyList attrs{{"shared",PropertyArray{"ancestor","value"}}};dynamic_cast<Probe<m::MmlMaligngroup>*>(s.n)->hook(attrs,clone(r.d),clone(r.l),"prime");expect(calls.size()==1_u);if(calls.size()!=1)continue;expect(deep(Property(calls[0].a),r.a));expect(deep(calls[0].d,r.display));expect(deep(calls[0].l,r.level));expect(deep(calls[0].p,r.prime));expect((Property(calls[0].a)==Property(attrs))==r.alias);}
enabled=false;};
"math_port_simple_mml_real_outer_menclose_0"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<0;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"menclose",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[208])));
enabled=false;};
"math_port_simple_mml_real_outer_menclose_1"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<1;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"menclose",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[222])));
enabled=false;};
"math_port_simple_mml_real_outer_menclose_2"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<2;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"menclose",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[228])));
enabled=false;};
"math_port_simple_mml_real_outer_mpadded_0"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<0;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"mpadded",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[232])));
enabled=false;};
"math_port_simple_mml_real_outer_mpadded_1"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<1;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"mpadded",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[235])));
enabled=false;};
"math_port_simple_mml_real_outer_mpadded_2"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<2;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"mpadded",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[238])));
enabled=false;};
"math_port_simple_mml_real_outer_maligngroup_0"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<0;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"maligngroup",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[244])));
enabled=false;};
"math_port_simple_mml_real_outer_maligngroup_1"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<1;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"maligngroup",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[252])));
enabled=false;};
"math_port_simple_mml_real_outer_maligngroup_2"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<2;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"maligngroup",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[258])));
enabled=false;};
"math_port_simple_mml_real_outer_malignmark_0"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<0;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"malignmark",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[262])));
enabled=false;};
"math_port_simple_mml_real_outer_malignmark_1"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<1;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"malignmark",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[262])));
enabled=false;};
"math_port_simple_mml_real_outer_malignmark_2"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<2;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"malignmark",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[262])));
enabled=false;};
"math_port_simple_mml_real_outer_mglyph_0"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<0;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"mglyph",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[266])));
enabled=false;};
"math_port_simple_mml_real_outer_mglyph_1"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<1;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"mglyph",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[269])));
enabled=false;};
"math_port_simple_mml_real_outer_mglyph_2"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<2;++i)cs->push_back(create(f,"mi",{},children({dynamic_cast<m::TextNode*>(create(f,"text"))->setText(u"x")})));auto*n=create(f,"mglyph",PropertyList{{"mathcolor","blue"}},cs);auto*parent=create(f,"mrow",{},children({n}));
{enabled=true;simpleEvents.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathsize",PropertyArray{"math","large"}}},true,"2",true);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[272])));
enabled=false;};
"math_port_simple_mml_real_glyph_error_false"_test=[] {reset();
NF f(bindings());auto*n=dynamic_cast<m::MmlMglyph*>(create(f,"mglyph"));auto*parent=create(f,"mrow",{},children({n}));n->attributes()->setList(PropertyList{{"src",""},{"fontfamily",""},{"index",""}});
{enabled=true;simpleEvents.clear();std::string failure;try{n->verifyAttributes(PropertyList{{"fullErrors",false}});}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[285])));
enabled=false;};
"math_port_simple_mml_real_glyph_error_true"_test=[] {reset();
NF f(bindings());auto*n=dynamic_cast<m::MmlMglyph*>(create(f,"mglyph"));auto*parent=create(f,"mrow",{},children({n}));n->attributes()->setList(PropertyList{{"src",""},{"fontfamily",""},{"index",""}});
{enabled=true;simpleEvents.clear();std::string failure;try{n->verifyAttributes(PropertyList{{"fullErrors",true}});}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(eventValue(),clone(sourceValues[24])));
}
expect(deep(tree(parent),clone(sourceValues[295])));
enabled=false;};
};
