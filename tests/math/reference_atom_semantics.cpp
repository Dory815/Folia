// Generated pinned actual-CJS TeXAtom and semantics observations.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_nodes.tex_atom;
import folia.math.core.mml_tree.mml_nodes.semantics;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.mml_nodes.mstyle;
import folia.math.core.mml_tree.mml_nodes.mphantom;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.arithmetic_host;
import folia.math.core.mml_tree.operator_class;
import folia.math.core.tree.node_factory;
namespace m=folia::math::mml;namespace h=m::host;namespace tr=folia::math::tree;using folia::math::Property;using folia::math::PropertyList;using folia::math::PropertyArray;using folia::math::Missing;using folia::math::PropertyNull;using namespace boost::ut;namespace {
std::u16string units(std::initializer_list<unsigned>v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}
m::Children children(std::initializer_list<m::NodeValue>v={}){return std::make_shared<tr::ChildArray<m::MmlNode>>(v);}m::MmlNode*parentNode(m::MmlNode*n){return n?n->getParent().nodeOrNull():nullptr;}Property parentValue(m::MmlNode*n,Property absent){auto*p=parentNode(n);return p?Property(p->kind()):absent;}Property snapshot(m::MmlNode*);Property childrenSnapshot(m::Children a){PropertyArray result;if(!a)return result;const auto length=a->size();result.resize(length);for(std::size_t i=0;i<length;i++)if(a->has(i))result.set(i,snapshot(a->get(i).nodeOrNull()));return result;}
Property clone(const Property&v){if(auto a=PropertyArray::array(v)){PropertyArray r;for(std::size_t i=0;i<a->size();i++)r.push(clone(a->get(i)));return r;}if(auto a=PropertyList::object(v)){PropertyList r;for(const auto&k:a->keys())r.setOwn(k,clone(a->getOwn(k)));return r;}return v;}
bool deep(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return(std::isnan(*x)&&std::isnan(*y))||(*x==*y&&std::signbit(*x)==std::signbit(*y));auto x=PropertyList::record(a),y=PropertyList::record(b);if(x&&y){if(PropertyArray::array(a).has_value()!=PropertyArray::array(b).has_value()||x->keys()!=y->keys())return false;for(const auto&k:x->keys())if(!deep(x->getOwn(k),y->getOwn(k)))return false;if(auto ax=PropertyArray::array(a))return ax->size()==PropertyArray::array(b)->size();return true;}return a==b;}
bool enabled=false,phase=false;std::vector<std::string>atomTrace;std::string mode;m::MmlNode*owner=nullptr,*late=nullptr;PropertyList phaseBag;
void note(std::string s){if(enabled)atomTrace.push_back(std::move(s));}
void reset(){enabled=phase=false;atomTrace.clear();mode.clear();owner=late=nullptr;phaseBag=PropertyList{};}
Property traceValues(){PropertyArray a;for(const auto&s:atomTrace)a.push(s);return a;}
struct Record:m::AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;Property received=PropertyList{};Property d,l,p;std::string kind()const override{return "record";}m::NodeValue setTeXclass(m::NodeValue prev)override{note(std::string("child/")+(prev.nodeOrNull()?"prev":"null"));if(mode=="child-throw")throw m::Error("WitnessError",u"witness");if(mode=="child-rebind")owner->childNodes=children({late});if(mode=="child-write")owner->setTexClass(2);return m::NodeValue(late);}void inheritAttributes(Property a,Property display,Property level,Property prime)override{note("inherit/record");received=a;d=display;l=level;p=prime;}};
template<class T>struct Probe:T{using T::T;Property texClass()override{note("tex.read");auto v=T::texClass();return mode=="constructor-getter"&&v==Property(0)?Property("virtual"):v;}void setTexClass(Property v)override{note("tex.write");T::setTexClass(v);}Property prevClass()override{note("prev.read");return T::prevClass();}void setPrevClass(Property v)override{note("prev.write");if(mode=="prev-setter-throw"){owner=this;throw m::Error("WitnessError",u"witness");}T::setPrevClass(v);}Property prevLevel()const override{note("level.read");return T::prevLevel();}void setPrevLevel(Property v)override{note("level.write");if(mode=="level-setter-throw"){owner=this;throw m::Error("WitnessError",u"witness");}T::setPrevLevel(v);}std::shared_ptr<folia::math::Attributes>attributes()const override{note("attributes");return T::attributes();}void setAttributes(std::shared_ptr<folia::math::Attributes>a)override{note("attributes.set");T::setAttributes(std::move(a));}Property getProperty(std::string_view k)const override{note("get/"+std::string(k));return T::getProperty(k);}void setProperty(std::string_view k,Property v)override{note("set/"+std::string(k));T::setProperty(k,v);}void setChildren(m::Children c)override{if(phase){note("phase/"+h::debugText(h::string(T::texClass())));phaseBag=T::getAllProperties();}T::setChildren(c);}void hook(PropertyList a,Property d,Property l,Property p){T::setChildInheritedAttributes(a,d,l,p);}};
using Atom=Probe<m::MmlTeXAtom>;using Semantics=Probe<m::MmlSemantics>;using AnnotationXML=Probe<m::MmlAnnotationXML>;using Annotation=Probe<m::MmlAnnotation>;using Operator=Probe<m::MmlMo>;using Previous=Probe<Record>;
struct Dispatch:Atom{using Atom::Atom;m::NodeValue adjustTeXclass(m::NodeValue p)override{note(std::string("adjust/")+(p.nodeOrNull()?"prev":"null"));return m::NodeValue(late);}};
using NF=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode>bindings(){return{{"TeXAtom",m::classBinding<m::MmlTeXAtom>()},{"semantics",m::classBinding<m::MmlSemantics>()},{"annotation-xml",m::classBinding<m::MmlAnnotationXML>()},{"annotation",m::classBinding<m::MmlAnnotation>()},{"probe-TeXAtom",m::classBinding<Atom>()},{"probe-semantics",m::classBinding<Semantics>()},{"probe-annotation-xml",m::classBinding<AnnotationXML>()},{"probe-annotation",m::classBinding<Annotation>()},{"probe-mo",m::classBinding<Operator>()},{"probe-record",m::classBinding<Previous>()},{"dispatch",m::classBinding<Dispatch>()},{"record",m::classBinding<Record>()},{"mo",m::classBinding<m::MmlMo>()},{"math",m::classBinding<m::MmlMath>()},{"mrow",m::classBinding<m::MmlMrow>()},{"inferredMrow",m::classBinding<m::MmlInferredMrow>()},{"mi",m::classBinding<m::MmlMi>()},{"mtext",m::classBinding<m::MmlMtext>()},{"mstyle",m::classBinding<m::MmlMstyle>()},{"mphantom",m::classBinding<m::MmlMphantom>()},{"msubsup",m::classBinding<m::MmlMsubsup>()},{"munderover",m::classBinding<m::MmlMunderover>()},{"text",m::classBinding<m::TextNode>()}};}
m::MmlNode*create(NF&f,std::string_view k,PropertyList a={},m::Children c=children()){return f.create(k,std::array<std::any,2>{a,c});}
Property snapshot(m::MmlNode*n){if(!n)return PropertyNull{};auto a=n->attributes();Property childValue=Missing{};if(n->childNodes)childValue=childrenSnapshot(n->childNodes);return PropertyList{{"kind",n->kind()},{"parent",parentValue(n,Property(Missing{}))},{"properties",n->getAllProperties()},{"tex",n->texClass()},{"prev",n->prevClass()},{"level",n->prevLevel()},{"explicit",a?clone(Property(a->getAllAttributes())):Property(Missing{})},{"inherited",a?clone(Property(a->getAllInherited())):Property(Missing{})},{"children",childValue}};}
void hook(m::MmlNode*n,PropertyList a,Property d,Property l,Property p){if(auto*x=dynamic_cast<Atom*>(n))x->hook(a,d,l,p);else if(auto*x=dynamic_cast<Semantics*>(n))x->hook(a,d,l,p);else if(auto*x=dynamic_cast<AnnotationXML*>(n))x->hook(a,d,l,p);else dynamic_cast<Annotation*>(n)->hook(a,d,l,p);}
m::NodeValue adjust(m::MmlNode*n,m::NodeValue p){if(auto*x=dynamic_cast<Atom*>(n))return x->adjustTeXclass(p);return dynamic_cast<Operator*>(n)->adjustTeXclass(p);}
}
namespace {const std::vector<Property> sourceValues=[] {std::vector<Property> values;values.reserve(157);
values.push_back(Property(0.0));
values.push_back(Property(PropertyList{{"texClass",values[0]}}));
values.push_back(Property(h::debugText(units({95,105,110,104,101,114,105,116,95}))));
values.push_back(Property(PropertyList{{"mathbackground",values[2]},{"mathcolor",values[2]},{"mathsize",values[2]},{"dir",values[2]}}));
values.push_back(Property(PropertyNull{}));
values.push_back(Property(PropertyList{}));
values.push_back(Property(PropertyList{{"mathbackground",values[2]},{"mathcolor",values[2]},{"mathsize",values[2]},{"dir",values[2]},{"definitionUrl",values[4]},{"encoding",values[4]}}));
values.push_back(Property(h::debugText(units({109,97,116,104,109,108,107,101,121,115}))));
values.push_back(Property(h::debugText(units({}))));
values.push_back(Property(PropertyList{{"mathbackground",values[2]},{"mathcolor",values[2]},{"mathsize",values[2]},{"dir",values[2]},{"definitionUrl",values[4]},{"encoding",values[4]},{"cd",values[7]},{"name",values[8]},{"src",values[4]}}));
values.push_back(Property(true));
values.push_back(Property(PropertyList{{"isChars",values[10]}}));
values.push_back(Property(h::debugText(units({112,114,101,118,46,119,114,105,116,101}))));
values.push_back(Property(h::debugText(units({108,101,118,101,108,46,119,114,105,116,101}))));
values.push_back(Property(h::debugText(units({112,104,97,115,101,47,110,117,108,108}))));
values.push_back(Property(h::debugText(units({97,116,116,114,105,98,117,116,101,115,46,115,101,116}))));
values.push_back(Property(h::debugText(units({97,116,116,114,105,98,117,116,101,115}))));
values.push_back(Property(h::debugText(units({116,101,120,46,114,101,97,100}))));
values.push_back(Property(h::debugText(units({115,101,116,47,116,101,120,67,108,97,115,115}))));
values.push_back(Property(PropertyArray{values[12],values[13],values[14],values[15],values[16],values[17],values[18]}));
values.push_back(Property(h::debugText(units({84,101,88,65,116,111,109}))));
values.push_back(Property(Missing{}));
values.push_back(Property(h::debugText(units({105,110,102,101,114,114,101,100,77,114,111,119}))));
values.push_back(Property(PropertyArray{}));
values.push_back(Property(PropertyList{{"kind",values[22]},{"parent",values[20]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[24]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[21]},{"properties",values[1]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[25]}}));
values.push_back(Property(h::debugText(units({118,105,114,116,117,97,108}))));
values.push_back(Property(PropertyList{{"texClass",values[27]}}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[21]},{"properties",values[28]},{"tex",values[27]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[25]}}));
values.push_back(Property(PropertyArray{values[12],values[13],values[14],values[15],values[16]}));
values.push_back(Property(h::debugText(units({97,110,110,111,116,97,116,105,111,110}))));
values.push_back(Property(PropertyList{{"kind",values[31]},{"parent",values[21]},{"properties",values[11]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[12]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[21]},{"properties",values[5]},{"tex",values[21]},{"prev",values[21]},{"level",values[21]},{"explicit",values[21]},{"inherited",values[21]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[12],values[13]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[21]},{"properties",values[5]},{"tex",values[21]},{"prev",values[4]},{"level",values[21]},{"explicit",values[21]},{"inherited",values[21]},{"children",values[23]}}));
values.push_back(Property(h::debugText(units({112,114,101,118,46,114,101,97,100}))));
values.push_back(Property(h::debugText(units({103,101,116,47,97,117,116,111,79,80}))));
values.push_back(Property(h::debugText(units({116,101,120,46,119,114,105,116,101}))));
values.push_back(Property(PropertyArray{values[17],values[37],values[38],values[17],values[12],values[16],values[13],values[39]}));
values.push_back(Property(PropertyArray{values[17],values[37],values[38],values[17],values[12],values[16],values[13]}));
values.push_back(Property(2.0));
values.push_back(Property(-1.0));
values.push_back(Property(false));
values.push_back(Property(h::debugText(units({115,101,101,100}))));
values.push_back(Property(h::debugText(units({108,101,118,101,108,45,115,101,101,100}))));
values.push_back(Property(h::debugText(units({114,97,119}))));
values.push_back(Property(1.0));
values.push_back(Property(3.0));
values.push_back(Property(4.0));
values.push_back(Property(5.0));
values.push_back(Property(6.0));
values.push_back(Property(h::debugText(units({114,97,119,45,108,101,118,101,108}))));
values.push_back(Property(7.0));
values.push_back(Property(h::debugText(units({50}))));
values.push_back(Property(std::numeric_limits<double>::quiet_NaN()));
values.push_back(Property(PropertyArray{values[17],values[37]}));
values.push_back(Property(PropertyArray{values[17],values[37],values[38],values[39],values[17],values[12],values[16],values[13],values[39]}));
values.push_back(Property(PropertyArray{values[17],values[37],values[38],values[17],values[12],values[16],values[13],values[12],values[39]}));
values.push_back(Property(PropertyArray{values[17],values[37],values[38],values[17],values[12],values[16]}));
values.push_back(Property(h::debugText(units({99,104,105,108,100,47,110,117,108,108}))));
values.push_back(Property(PropertyArray{values[61],values[17],values[37],values[12]}));
values.push_back(Property(h::debugText(units({109,114,111,119}))));
values.push_back(Property(h::debugText(units({114,101,99,111,114,100}))));
values.push_back(Property(PropertyList{{"kind",values[64]},{"parent",values[20]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[65]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[43]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[66]}}));
values.push_back(Property(PropertyArray{values[61],values[17],values[37],values[38],values[17],values[12],values[16],values[13]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[0]},{"level",values[0]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[66]}}));
values.push_back(Property(PropertyArray{values[61],values[39],values[17],values[37],values[12],values[39]}));
values.push_back(Property(PropertyArray{values[61],values[39],values[17],values[37],values[38],values[17],values[12],values[16],values[13],values[39]}));
values.push_back(Property(PropertyList{{"kind",values[64]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[72]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[43]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[73]}}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[0]},{"level",values[0]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[73]}}));
values.push_back(Property(PropertyArray{values[61]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[66]}}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[4]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[79]}}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[21]}}));
values.push_back(Property(h::debugText(units({97,100,106,117,115,116,47,110,117,108,108}))));
values.push_back(Property(PropertyArray{values[61],values[82]}));
values.push_back(Property(h::debugText(units({97,100,106,117,115,116,47,112,114,101,118}))));
values.push_back(Property(PropertyArray{values[61],values[84]}));
values.push_back(Property(h::debugText(units({111,114,105,103,105,110}))));
values.push_back(Property(h::debugText(units({118,97,108,117,101}))));
values.push_back(Property(PropertyArray{values[86],values[87]}));
values.push_back(Property(PropertyList{{"shared",values[88]}}));
values.push_back(Property(h::debugText(units({105,110,104,101,114,105,116,47,114,101,99,111,114,100}))));
values.push_back(Property(PropertyArray{values[90]}));
values.push_back(Property(h::debugText(units({112,114,105,109,101}))));
values.push_back(Property(PropertyList{{"a",values[89]},{"d",values[47]},{"l",values[55]},{"p",values[92]}}));
values.push_back(Property(PropertyList{{"displaystyle",values[10]}}));
values.push_back(Property(h::debugText(units({98,108,117,101}))));
values.push_back(Property(PropertyList{{"mathcolor",values[95]},{"scriptlevel",values[0]}}));
values.push_back(Property(PropertyList{{"mathcolor",values[95]},{"displaystyle",values[10]},{"scriptlevel",values[0]}}));
values.push_back(Property(PropertyList{{"kind",values[22]},{"parent",values[20]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[97]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[98]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[99]}}));
values.push_back(Property(PropertyArray{values[100]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[101]}}));
values.push_back(Property(h::debugText(units({109,105}))));
values.push_back(Property(PropertyList{{"kind",values[103]},{"parent",values[22]},{"properties",values[5]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[97]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[104]}));
values.push_back(Property(PropertyList{{"kind",values[22]},{"parent",values[20]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[97]},{"children",values[105]}}));
values.push_back(Property(PropertyArray{values[106]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[107]}}));
values.push_back(Property(PropertyArray{values[108]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[109]}}));
values.push_back(Property(PropertyArray{values[104],values[104]}));
values.push_back(Property(PropertyList{{"kind",values[22]},{"parent",values[20]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[97]},{"children",values[111]}}));
values.push_back(Property(PropertyArray{values[112]}));
values.push_back(Property(PropertyList{{"kind",values[20]},{"parent",values[63]},{"properties",values[1]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[113]}}));
values.push_back(Property(PropertyArray{values[114]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[115]}}));
values.push_back(Property(h::debugText(units({115,101,109,97,110,116,105,99,115}))));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[117]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[97]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[118]}));
values.push_back(Property(PropertyList{{"kind",values[117]},{"parent",values[63]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[119]}}));
values.push_back(Property(PropertyArray{values[120]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[121]}}));
values.push_back(Property(PropertyList{{"kind",values[103]},{"parent",values[117]},{"properties",values[5]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[97]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[123]}));
values.push_back(Property(PropertyList{{"kind",values[117]},{"parent",values[63]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[124]}}));
values.push_back(Property(PropertyArray{values[125]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[126]}}));
values.push_back(Property(PropertyArray{values[123],values[123]}));
values.push_back(Property(PropertyList{{"kind",values[117]},{"parent",values[63]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[128]}}));
values.push_back(Property(PropertyArray{values[129]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[130]}}));
values.push_back(Property(h::debugText(units({97,110,110,111,116,97,116,105,111,110,45,120,109,108}))));
values.push_back(Property(PropertyList{{"kind",values[132]},{"parent",values[63]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[133]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[134]}}));
values.push_back(Property(PropertyList{{"kind",values[103]},{"parent",values[132]},{"properties",values[5]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[136]}));
values.push_back(Property(PropertyList{{"kind",values[132]},{"parent",values[63]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[137]}}));
values.push_back(Property(PropertyArray{values[138]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[139]}}));
values.push_back(Property(PropertyArray{values[136],values[136]}));
values.push_back(Property(PropertyList{{"kind",values[132]},{"parent",values[63]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[141]}}));
values.push_back(Property(PropertyArray{values[142]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[143]}}));
values.push_back(Property(PropertyList{{"kind",values[31]},{"parent",values[63]},{"properties",values[11]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[145]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[146]}}));
values.push_back(Property(PropertyList{{"kind",values[103]},{"parent",values[31]},{"properties",values[5]},{"tex",values[0]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[23]}}));
values.push_back(Property(PropertyArray{values[148]}));
values.push_back(Property(PropertyList{{"kind",values[31]},{"parent",values[63]},{"properties",values[11]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[149]}}));
values.push_back(Property(PropertyArray{values[150]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[151]}}));
values.push_back(Property(PropertyArray{values[148],values[148]}));
values.push_back(Property(PropertyList{{"kind",values[31]},{"parent",values[63]},{"properties",values[11]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[94]},{"inherited",values[96]},{"children",values[153]}}));
values.push_back(Property(PropertyArray{values[154]}));
values.push_back(Property(PropertyList{{"kind",values[63]},{"parent",values[21]},{"properties",values[5]},{"tex",values[4]},{"prev",values[4]},{"level",values[4]},{"explicit",values[5]},{"inherited",values[5]},{"children",values[155]}}));
return values;}();}
suite math_port_atom_semantics_tests=[] {
"math_port_atom_semantics_defaults_getters_class_identity"_test=[] {reset();
NF f(bindings());
{auto*n=create(f,"TeXAtom");
expect(bool((n->kind())=="TeXAtom"));
expect(bool((n->arity())==-1));
expect(bool((n->notParent())==true));
expect(bool((n->linebreakContainer())==false));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[1])));
expect(deep(Property(m::MmlTeXAtom::defaults()),clone(sourceValues[3])));
}
{auto*n=create(f,"semantics");
expect(bool((n->kind())=="semantics"));
expect(bool((n->arity())==1));
expect(bool((n->notParent())==true));
expect(bool((n->linebreakContainer())==false));
expect(deep(n->texClass(),clone(sourceValues[4])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[5])));
expect(deep(Property(m::MmlSemantics::defaults()),clone(sourceValues[6])));
}
{auto*n=create(f,"annotation-xml");
expect(bool((n->kind())=="annotation-xml"));
expect(bool((n->arity())==std::numeric_limits<double>::infinity()));
expect(bool((n->notParent())==false));
expect(bool((n->linebreakContainer())==false));
expect(deep(n->texClass(),clone(sourceValues[4])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[5])));
expect(deep(Property(m::MmlAnnotationXML::defaults()),clone(sourceValues[9])));
}
{auto*n=create(f,"annotation");
expect(bool((n->kind())=="annotation"));
expect(bool((n->arity())==std::numeric_limits<double>::infinity()));
expect(bool((n->notParent())==false));
expect(bool((n->linebreakContainer())==false));
expect(deep(n->texClass(),clone(sourceValues[4])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[11])));
expect(deep(Property(m::MmlAnnotation::defaults()),clone(sourceValues[9])));
}
expect(bool((Property(m::MmlTeXAtom::defaults())==Property(m::AbstractMmlBaseNode::defaults()))==false));
expect(bool((Property(m::MmlSemantics::defaults())==Property(m::AbstractMmlBaseNode::defaults()))==false));
expect(bool((Property(m::MmlAnnotationXML::defaults())==Property(m::AbstractMmlNode::defaults()))==false));
expect(bool((Property(m::MmlAnnotation::defaults())==Property(m::MmlAnnotationXML::defaults()))==false));
enabled=false;};
"math_port_atom_semantics_constructor_TeXAtom_normal"_test=[] {reset();
NF f(bindings());mode="";phase=true;m::MmlNode*n=nullptr;
enabled=true;atomTrace.clear();std::string failure;try{n=create(f,"probe-TeXAtom");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[19])));
phase=false;
expect(deep(snapshot(n),clone(sourceValues[26])));
expect(bool((Property(phaseBag)==Property(n->getAllProperties()))==true));
enabled=false;};
"math_port_atom_semantics_constructor_TeXAtom_constructor-getter"_test=[] {reset();
NF f(bindings());mode="constructor-getter";phase=true;m::MmlNode*n=nullptr;
enabled=true;atomTrace.clear();std::string failure;try{n=create(f,"probe-TeXAtom");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[19])));
phase=false;
expect(deep(snapshot(n),clone(sourceValues[29])));
expect(bool((Property(phaseBag)==Property(n->getAllProperties()))==true));
enabled=false;};
"math_port_atom_semantics_constructor_annotation_normal"_test=[] {reset();
NF f(bindings());mode="";phase=true;m::MmlNode*n=nullptr;
enabled=true;atomTrace.clear();std::string failure;try{n=create(f,"probe-annotation");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[30])));
phase=false;
expect(deep(snapshot(n),clone(sourceValues[32])));
expect(bool((Property(phaseBag)==Property(n->getAllProperties()))==false));
enabled=false;};
"math_port_atom_semantics_constructor_annotation_constructor-getter"_test=[] {reset();
NF f(bindings());mode="constructor-getter";phase=true;m::MmlNode*n=nullptr;
enabled=true;atomTrace.clear();std::string failure;try{n=create(f,"probe-annotation");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[30])));
phase=false;
expect(deep(snapshot(n),clone(sourceValues[32])));
expect(bool((Property(phaseBag)==Property(n->getAllProperties()))==false));
enabled=false;};
"math_port_atom_semantics_constructor_partial_prev-setter-throw"_test=[] {reset();
NF f(bindings());mode="prev-setter-throw";
enabled=true;atomTrace.clear();std::string failure;try{create(f,"probe-TeXAtom");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(traceValues(),clone(sourceValues[33])));
expect(deep(snapshot(owner),clone(sourceValues[34])));
enabled=false;};
"math_port_atom_semantics_constructor_partial_level-setter-throw"_test=[] {reset();
NF f(bindings());mode="level-setter-throw";
enabled=true;atomTrace.clear();std::string failure;try{create(f,"probe-TeXAtom");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(traceValues(),clone(sourceValues[35])));
expect(deep(snapshot(owner),clone(sourceValues[36])));
enabled=false;};
"math_port_atom_semantics_real_shared_parent_TeXAtom_false_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);auto*atom=n;auto*sem=create(f,"semantics",{},children({atom}));auto*holder=create(f,"mrow",{},children({sem}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[40])));
expect(bool((result==m::NodeValue(n))==true));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_real_shared_parent_TeXAtom_false_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);auto*atom=n;auto*sem=create(f,"semantics",{},children({atom}));auto*holder=create(f,"mrow",{},children({sem,create(f,"mi")}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[40])));
expect(bool((result==m::NodeValue(n))==true));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_real_shared_parent_TeXAtom_true_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);auto*atom=n;auto*sem=create(f,"semantics",{},children({atom,create(f,"annotation")}));auto*holder=create(f,"mrow",{},children({sem}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[41])));
expect(bool((result==m::NodeValue(n))==true));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_real_shared_parent_TeXAtom_true_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);auto*atom=n;auto*sem=create(f,"semantics",{},children({atom,create(f,"annotation")}));auto*holder=create(f,"mrow",{},children({sem,create(f,"mi")}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[41])));
expect(bool((result==m::NodeValue(n))==true));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_real_shared_parent_mo_false_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);auto*atom=create(f,"TeXAtom",{},children({n}));auto*sem=create(f,"semantics",{},children({atom}));auto*holder=create(f,"mrow",{},children({sem}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[40])));
expect(bool((result==m::NodeValue(n))==true));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_real_shared_parent_mo_false_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);auto*atom=create(f,"TeXAtom",{},children({n}));auto*sem=create(f,"semantics",{},children({atom}));auto*holder=create(f,"mrow",{},children({sem,create(f,"mi")}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[41])));
expect(bool((result==m::NodeValue(n))==true));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_real_shared_parent_mo_true_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);auto*atom=create(f,"TeXAtom",{},children({n}));auto*sem=create(f,"semantics",{},children({atom,create(f,"annotation")}));auto*holder=create(f,"mrow",{},children({sem}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[40])));
expect(bool((result==m::NodeValue(n))==true));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_real_shared_parent_mo_true_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);auto*atom=create(f,"TeXAtom",{},children({n}));auto*sem=create(f,"semantics",{},children({atom,create(f,"annotation")}));auto*holder=create(f,"mrow",{},children({sem,create(f,"mi")}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[41])));
expect(bool((result==m::NodeValue(n))==true));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_raw_adjustment_TeXAtom"_test=[] {reset();
struct Rule{Property current,previous,autoOp;std::string error;bool self,prevResult;Property tex,prevClass,prevLevel,previousTex;};const Rule rowsData[]={
{clone(sourceValues[43]),clone(sourceValues[4]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[43]),clone(sourceValues[4]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[43]),clone(sourceValues[4]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[43]),clone(sourceValues[43]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[43])},
{clone(sourceValues[43]),clone(sourceValues[43]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[43])},
{clone(sourceValues[43]),clone(sourceValues[43]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[43])},
{clone(sourceValues[43]),clone(sourceValues[0]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[0])},
{clone(sourceValues[43]),clone(sourceValues[0]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[0])},
{clone(sourceValues[43]),clone(sourceValues[0]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[0])},
{clone(sourceValues[43]),clone(sourceValues[48]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[48])},
{clone(sourceValues[43]),clone(sourceValues[48]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[48])},
{clone(sourceValues[43]),clone(sourceValues[48]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[48])},
{clone(sourceValues[43]),clone(sourceValues[42]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[42])},
{clone(sourceValues[43]),clone(sourceValues[42]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[42])},
{clone(sourceValues[43]),clone(sourceValues[42]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[42])},
{clone(sourceValues[43]),clone(sourceValues[49]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[49])},
{clone(sourceValues[43]),clone(sourceValues[49]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[49])},
{clone(sourceValues[43]),clone(sourceValues[49]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[49])},
{clone(sourceValues[43]),clone(sourceValues[50]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[50])},
{clone(sourceValues[43]),clone(sourceValues[50]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[50])},
{clone(sourceValues[43]),clone(sourceValues[50]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[50])},
{clone(sourceValues[43]),clone(sourceValues[51]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[51])},
{clone(sourceValues[43]),clone(sourceValues[51]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[51])},
{clone(sourceValues[43]),clone(sourceValues[51]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[51])},
{clone(sourceValues[43]),clone(sourceValues[52]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[52])},
{clone(sourceValues[43]),clone(sourceValues[52]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[52])},
{clone(sourceValues[43]),clone(sourceValues[52]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[52])},
{clone(sourceValues[43]),clone(sourceValues[47]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[47])},
{clone(sourceValues[43]),clone(sourceValues[47]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[47])},
{clone(sourceValues[43]),clone(sourceValues[47]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[47])},
{clone(sourceValues[43]),clone(sourceValues[44]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[44])},
{clone(sourceValues[43]),clone(sourceValues[44]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[44])},
{clone(sourceValues[43]),clone(sourceValues[44]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[44])},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[0]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[0]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[0]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[48]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[48]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[48]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[48]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[48]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[48]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[42]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[42]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[42]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[42]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[42]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[42]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[42]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[42]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[42]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[42]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[42]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[42]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[42]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[49]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[49]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[49]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[49]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[49]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[49]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[49]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[49]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[49]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[50]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[50]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[50]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[50]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[50]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[50]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[51]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[51]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[51]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[51]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[51]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[51]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[51]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[52]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[52]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[52]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[52]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[52]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[52]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[52]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[54]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[54]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[54]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[54]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[54]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[54]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[4]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[4]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[4]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[4]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[4]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[4]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[4]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[4]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[4]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[4]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[4]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[4]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[4]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[4]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[4]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[4]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[4]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[4]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[4]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[4]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[4]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[4]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[4]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[4]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[21]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[21]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[21]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[21]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[21]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[21]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[55]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[55]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[55]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[55]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[55]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[55]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[44]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[44]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[44]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[44]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[44]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[44]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[56]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[56]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[56]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
};for(const auto&r:rowsData){NF f(bindings());auto*n=create(f,"probe-TeXAtom");m::MmlNode*prev=h::nullish(r.previous)?nullptr:create(f,"probe-record");n->setTexClass(clone(r.current));n->setPrevClass("seed");n->setPrevLevel("level-seed");n->attributes()->setInherited("scriptlevel","raw-level");if(prev){prev->setTexClass(clone(r.previous));prev->setProperty("autoOP",clone(r.autoOp));}auto*holder=create(f,"mrow",{},children({n}));m::NodeValue result;std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}expect(failure==r.error);expect((result==m::NodeValue(n))==r.self);expect((result==m::NodeValue(prev))==r.prevResult);expect(deep(n->texClass(),r.tex));expect(deep(n->prevClass(),r.prevClass));expect(deep(n->prevLevel(),r.prevLevel));expect(deep(prev?prev->texClass():Property(Missing{}),r.previousTex));}
enabled=false;};
"math_port_atom_semantics_raw_adjustment_mo"_test=[] {reset();
struct Rule{Property current,previous,autoOp;std::string error;bool self,prevResult;Property tex,prevClass,prevLevel,previousTex;};const Rule rowsData[]={
{clone(sourceValues[43]),clone(sourceValues[4]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[43]),clone(sourceValues[4]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[43]),clone(sourceValues[4]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[43]),clone(sourceValues[43]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[43])},
{clone(sourceValues[43]),clone(sourceValues[43]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[43])},
{clone(sourceValues[43]),clone(sourceValues[43]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[43])},
{clone(sourceValues[43]),clone(sourceValues[0]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[0])},
{clone(sourceValues[43]),clone(sourceValues[0]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[0])},
{clone(sourceValues[43]),clone(sourceValues[0]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[0])},
{clone(sourceValues[43]),clone(sourceValues[48]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[48])},
{clone(sourceValues[43]),clone(sourceValues[48]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[48])},
{clone(sourceValues[43]),clone(sourceValues[48]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[48])},
{clone(sourceValues[43]),clone(sourceValues[42]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[42])},
{clone(sourceValues[43]),clone(sourceValues[42]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[42])},
{clone(sourceValues[43]),clone(sourceValues[42]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[42])},
{clone(sourceValues[43]),clone(sourceValues[49]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[49])},
{clone(sourceValues[43]),clone(sourceValues[49]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[49])},
{clone(sourceValues[43]),clone(sourceValues[49]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[49])},
{clone(sourceValues[43]),clone(sourceValues[50]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[50])},
{clone(sourceValues[43]),clone(sourceValues[50]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[50])},
{clone(sourceValues[43]),clone(sourceValues[50]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[50])},
{clone(sourceValues[43]),clone(sourceValues[51]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[51])},
{clone(sourceValues[43]),clone(sourceValues[51]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[51])},
{clone(sourceValues[43]),clone(sourceValues[51]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[51])},
{clone(sourceValues[43]),clone(sourceValues[52]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[52])},
{clone(sourceValues[43]),clone(sourceValues[52]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[52])},
{clone(sourceValues[43]),clone(sourceValues[52]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[52])},
{clone(sourceValues[43]),clone(sourceValues[47]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[47])},
{clone(sourceValues[43]),clone(sourceValues[47]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[47])},
{clone(sourceValues[43]),clone(sourceValues[47]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[47])},
{clone(sourceValues[43]),clone(sourceValues[44]),clone(sourceValues[44]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[44])},
{clone(sourceValues[43]),clone(sourceValues[44]),clone(sourceValues[10]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[44])},
{clone(sourceValues[43]),clone(sourceValues[44]),clone(sourceValues[47]),"",false,true,clone(sourceValues[43]),clone(sourceValues[45]),clone(sourceValues[46]),clone(sourceValues[44])},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[0]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[0]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[0]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[0]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[48]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[48]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[48]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[48]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[48]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[48]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[48]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[42]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[42]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[42]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[42]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[42]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[42]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[42]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[42]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[42]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[42]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[42]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[42]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[42]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[42]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[49]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[49]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[49]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[49]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[49]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[49]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[49]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[49]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[49]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[49]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[49]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[50]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[50]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[50]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[50]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[50]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[50]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[50]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[51]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[51]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[51]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[51]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[51]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[51]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[51]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[51]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[52]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[52]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[52]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[52]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[52]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[52]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[52]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[52]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[54]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[54]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[54]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[54]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[54]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[54]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[54]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[4]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[4]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[4]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[4]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[4]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[4]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[4]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[4]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[4]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[4]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[4]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[4]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[4]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[4]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[4]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[4]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[4]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[4]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[4]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[4]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[4]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[4]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[4]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[4]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[4]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[4]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[4]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[0]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[21]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[21]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[21]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[21]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[21]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[21]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[21]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[55]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[55]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[55]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[55]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[55]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[55]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[55]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[44]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[44]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[44]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[44]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[44]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[44]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[44]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[56]),clone(sourceValues[4]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[46]),clone(sourceValues[21])},
{clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[43]),clone(sourceValues[53]),clone(sourceValues[43])},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[0])},
{clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[48]),clone(sourceValues[53]),clone(sourceValues[48])},
{clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[42]),clone(sourceValues[53]),clone(sourceValues[42])},
{clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[49]),clone(sourceValues[53]),clone(sourceValues[49])},
{clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[50]),clone(sourceValues[53]),clone(sourceValues[50])},
{clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[51]),clone(sourceValues[53]),clone(sourceValues[51])},
{clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[52]),clone(sourceValues[53]),clone(sourceValues[52])},
{clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[47]),clone(sourceValues[53]),clone(sourceValues[47])},
{clone(sourceValues[56]),clone(sourceValues[44]),clone(sourceValues[44]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[56]),clone(sourceValues[44]),clone(sourceValues[10]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
{clone(sourceValues[56]),clone(sourceValues[44]),clone(sourceValues[47]),"",true,false,clone(sourceValues[56]),clone(sourceValues[0]),clone(sourceValues[53]),clone(sourceValues[44])},
};for(const auto&r:rowsData){NF f(bindings());auto*n=create(f,"probe-mo");m::MmlNode*prev=h::nullish(r.previous)?nullptr:create(f,"probe-record");n->setTexClass(clone(r.current));n->setPrevClass("seed");n->setPrevLevel("level-seed");n->attributes()->setInherited("scriptlevel","raw-level");if(prev){prev->setTexClass(clone(r.previous));prev->setProperty("autoOP",clone(r.autoOp));}auto*holder=create(f,"mrow",{},children({n}));m::NodeValue result;std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}expect(failure==r.error);expect((result==m::NodeValue(n))==r.self);expect((result==m::NodeValue(prev))==r.prevResult);expect(deep(n->texClass(),r.tex));expect(deep(n->prevClass(),r.prevClass));expect(deep(n->prevLevel(),r.prevLevel));expect(deep(prev?prev->texClass():Property(Missing{}),r.previousTex));}
enabled=false;};
"math_port_atom_semantics_adjust_order_TeXAtom_none"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(-1);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[57])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(prev))==true));
expect(deep(n->texClass(),clone(sourceValues[43])));
expect(deep(n->prevClass(),clone(sourceValues[4])));
expect(deep(n->prevLevel(),clone(sourceValues[4])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_TeXAtom_auto-op"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",true);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[58])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_TeXAtom_prev-binary"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(3);prev->setTexClass(2);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[59])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[49])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_TeXAtom_last"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[40])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_TeXAtom_first"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n,create(f,"mi")}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[41])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_TeXAtom_detached"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[41])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_TeXAtom_nested"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));create(f,"mrow",{},children({holder}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[40])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_TeXAtom_null-array"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));holder->childNodes.reset();m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[41])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_TeXAtom_attributes-null"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));n->setAttributes(nullptr);m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[60])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[4])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_mo_none"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(-1);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[57])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(prev))==true));
expect(deep(n->texClass(),clone(sourceValues[43])));
expect(deep(n->prevClass(),clone(sourceValues[4])));
expect(deep(n->prevLevel(),clone(sourceValues[4])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_mo_auto-op"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",true);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[58])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_mo_prev-binary"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(3);prev->setTexClass(2);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[59])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[49])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_mo_last"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[40])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_mo_first"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n,create(f,"mi")}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[41])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_mo_detached"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[41])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_mo_nested"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));create(f,"mrow",{},children({holder}));m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[40])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[0])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_mo_null-array"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));holder->childNodes.reset();m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[41])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[55])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_adjust_order_mo_attributes-null"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mo");auto*prev=create(f,"probe-record");n->setTexClass(2);prev->setTexClass(0);prev->setProperty("autoOP",false);n->attributes()->setInherited("scriptlevel","2");auto*holder=create(f,"mrow",{},children({n}));n->setAttributes(nullptr);m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=adjust(n,prev);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[60])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(prev))==false));
expect(deep(n->texClass(),clone(sourceValues[42])));
expect(deep(n->prevClass(),clone(sourceValues[0])));
expect(deep(n->prevLevel(),clone(sourceValues[4])));
expect(deep(prev->texClass(),clone(sourceValues[0])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_35"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=nullptr;owner=n;late=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));mode="";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[62])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[67])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_36"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=create(f,"probe-record");owner=n;late=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));mode="";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[68])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[69])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_37"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=nullptr;owner=n;late=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));mode="child-write";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[70])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[67])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_38"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=create(f,"probe-record");owner=n;late=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));mode="child-write";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[71])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[69])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_39"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=nullptr;owner=n;late=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));mode="child-rebind";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[62])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[74])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_40"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=create(f,"probe-record");owner=n;late=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));mode="child-rebind";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[68])));
expect(bool((result==m::NodeValue(n))==true));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[75])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_41"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=nullptr;owner=n;late=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));mode="child-throw";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(traceValues(),clone(sourceValues[76])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[77])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_42"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=create(f,"probe-record");owner=n;late=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));mode="child-throw";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="WitnessError"));
expect(deep(traceValues(),clone(sourceValues[76])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[77])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_43"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=nullptr;owner=n;late=create(f,"record");n->childNodes=children();c->setParent(m::NodeValue(n));mode="empty";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[78])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_44"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=create(f,"probe-record");owner=n;late=create(f,"record");n->childNodes=children();c->setParent(m::NodeValue(n));mode="empty";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[78])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_45"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=nullptr;owner=n;late=create(f,"record");n->childNodes=children({nullptr});c->setParent(m::NodeValue(n));mode="null-child";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[80])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_46"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=create(f,"probe-record");owner=n;late=create(f,"record");n->childNodes=children({nullptr});c->setParent(m::NodeValue(n));mode="null-child";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[80])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_47"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=nullptr;owner=n;late=create(f,"record");n->childNodes=m::Children{};c->setParent(m::NodeValue(n));mode="null-array";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[81])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_48"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-TeXAtom");auto*c=create(f,"record");m::MmlNode*prev=create(f,"probe-record");owner=n;late=create(f,"record");n->childNodes=m::Children{};c->setParent(m::NodeValue(n));mode="null-array";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(late))==false));
expect(deep(snapshot(n),clone(sourceValues[81])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_49"_test=[] {reset();
NF f(bindings());auto*n=create(f,"dispatch");auto*c=create(f,"record");m::MmlNode*prev=nullptr;owner=n;late=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));mode="dispatch";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[83])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(late))==true));
expect(deep(snapshot(n),clone(sourceValues[77])));
enabled=false;};
"math_port_atom_semantics_child_dispatch_50"_test=[] {reset();
NF f(bindings());auto*n=create(f,"dispatch");auto*c=create(f,"record");m::MmlNode*prev=create(f,"probe-record");owner=n;late=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));mode="dispatch";auto*holder=create(f,"mrow");n->setParent(m::NodeValue(holder));holder->childNodes=children({n});m::NodeValue result;
enabled=true;atomTrace.clear();std::string failure;try{result=n->setTeXclass(m::NodeValue(prev));}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[85])));
expect(bool((result==m::NodeValue(n))==false));
expect(bool((result==m::NodeValue(late))==true));
expect(deep(snapshot(n),clone(sourceValues[77])));
enabled=false;};
"math_port_atom_semantics_inherit_hook_semantics_normal"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-semantics");auto*c=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));PropertyList attrs=PropertyList::record(clone(sourceValues[89])).value();
enabled=true;atomTrace.clear();std::string failure;try{hook(n,attrs,"raw","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[91])));
expect(deep(h::ownKeys(dynamic_cast<Record*>(c)->received).empty()?Property(Missing{}):PropertyList{{"a",dynamic_cast<Record*>(c)->received},{"d",dynamic_cast<Record*>(c)->d},{"l",dynamic_cast<Record*>(c)->l},{"p",dynamic_cast<Record*>(c)->p}},clone(sourceValues[93])));
enabled=false;};
"math_port_atom_semantics_inherit_hook_semantics_null-child"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-semantics");auto*c=create(f,"record");n->childNodes=children({nullptr});c->setParent(m::NodeValue(n));PropertyList attrs=PropertyList::record(clone(sourceValues[89])).value();
enabled=true;atomTrace.clear();std::string failure;try{hook(n,attrs,"raw","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(h::ownKeys(dynamic_cast<Record*>(c)->received).empty()?Property(Missing{}):PropertyList{{"a",dynamic_cast<Record*>(c)->received},{"d",dynamic_cast<Record*>(c)->d},{"l",dynamic_cast<Record*>(c)->l},{"p",dynamic_cast<Record*>(c)->p}},clone(sourceValues[21])));
enabled=false;};
"math_port_atom_semantics_inherit_hook_semantics_null-array"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-semantics");auto*c=create(f,"record");n->childNodes=m::Children{};c->setParent(m::NodeValue(n));PropertyList attrs=PropertyList::record(clone(sourceValues[89])).value();
enabled=true;atomTrace.clear();std::string failure;try{hook(n,attrs,"raw","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)=="TypeError"));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(h::ownKeys(dynamic_cast<Record*>(c)->received).empty()?Property(Missing{}):PropertyList{{"a",dynamic_cast<Record*>(c)->received},{"d",dynamic_cast<Record*>(c)->d},{"l",dynamic_cast<Record*>(c)->l},{"p",dynamic_cast<Record*>(c)->p}},clone(sourceValues[21])));
enabled=false;};
"math_port_atom_semantics_inherit_hook_annotation-xml_normal"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-annotation-xml");auto*c=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));PropertyList attrs=PropertyList::record(clone(sourceValues[89])).value();
enabled=true;atomTrace.clear();std::string failure;try{hook(n,attrs,"raw","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(h::ownKeys(dynamic_cast<Record*>(c)->received).empty()?Property(Missing{}):PropertyList{{"a",dynamic_cast<Record*>(c)->received},{"d",dynamic_cast<Record*>(c)->d},{"l",dynamic_cast<Record*>(c)->l},{"p",dynamic_cast<Record*>(c)->p}},clone(sourceValues[21])));
enabled=false;};
"math_port_atom_semantics_inherit_hook_annotation-xml_null-child"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-annotation-xml");auto*c=create(f,"record");n->childNodes=children({nullptr});c->setParent(m::NodeValue(n));PropertyList attrs=PropertyList::record(clone(sourceValues[89])).value();
enabled=true;atomTrace.clear();std::string failure;try{hook(n,attrs,"raw","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(h::ownKeys(dynamic_cast<Record*>(c)->received).empty()?Property(Missing{}):PropertyList{{"a",dynamic_cast<Record*>(c)->received},{"d",dynamic_cast<Record*>(c)->d},{"l",dynamic_cast<Record*>(c)->l},{"p",dynamic_cast<Record*>(c)->p}},clone(sourceValues[21])));
enabled=false;};
"math_port_atom_semantics_inherit_hook_annotation-xml_null-array"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-annotation-xml");auto*c=create(f,"record");n->childNodes=m::Children{};c->setParent(m::NodeValue(n));PropertyList attrs=PropertyList::record(clone(sourceValues[89])).value();
enabled=true;atomTrace.clear();std::string failure;try{hook(n,attrs,"raw","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(h::ownKeys(dynamic_cast<Record*>(c)->received).empty()?Property(Missing{}):PropertyList{{"a",dynamic_cast<Record*>(c)->received},{"d",dynamic_cast<Record*>(c)->d},{"l",dynamic_cast<Record*>(c)->l},{"p",dynamic_cast<Record*>(c)->p}},clone(sourceValues[21])));
enabled=false;};
"math_port_atom_semantics_inherit_hook_annotation_normal"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-annotation");auto*c=create(f,"record");n->childNodes=children({c});c->setParent(m::NodeValue(n));PropertyList attrs=PropertyList::record(clone(sourceValues[89])).value();
enabled=true;atomTrace.clear();std::string failure;try{hook(n,attrs,"raw","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(h::ownKeys(dynamic_cast<Record*>(c)->received).empty()?Property(Missing{}):PropertyList{{"a",dynamic_cast<Record*>(c)->received},{"d",dynamic_cast<Record*>(c)->d},{"l",dynamic_cast<Record*>(c)->l},{"p",dynamic_cast<Record*>(c)->p}},clone(sourceValues[21])));
enabled=false;};
"math_port_atom_semantics_inherit_hook_annotation_null-child"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-annotation");auto*c=create(f,"record");n->childNodes=children({nullptr});c->setParent(m::NodeValue(n));PropertyList attrs=PropertyList::record(clone(sourceValues[89])).value();
enabled=true;atomTrace.clear();std::string failure;try{hook(n,attrs,"raw","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(h::ownKeys(dynamic_cast<Record*>(c)->received).empty()?Property(Missing{}):PropertyList{{"a",dynamic_cast<Record*>(c)->received},{"d",dynamic_cast<Record*>(c)->d},{"l",dynamic_cast<Record*>(c)->l},{"p",dynamic_cast<Record*>(c)->p}},clone(sourceValues[21])));
enabled=false;};
"math_port_atom_semantics_inherit_hook_annotation_null-array"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-annotation");auto*c=create(f,"record");n->childNodes=m::Children{};c->setParent(m::NodeValue(n));PropertyList attrs=PropertyList::record(clone(sourceValues[89])).value();
enabled=true;atomTrace.clear();std::string failure;try{hook(n,attrs,"raw","2","prime");}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(h::ownKeys(dynamic_cast<Record*>(c)->received).empty()?Property(Missing{}):PropertyList{{"a",dynamic_cast<Record*>(c)->received},{"d",dynamic_cast<Record*>(c)->d},{"l",dynamic_cast<Record*>(c)->l},{"p",dynamic_cast<Record*>(c)->p}},clone(sourceValues[21])));
enabled=false;};
"math_port_atom_semantics_outer_TeXAtom_0"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<0;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"TeXAtom",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[102])));
enabled=false;};
"math_port_atom_semantics_outer_TeXAtom_1"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<1;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"TeXAtom",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[110])));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(holder))==true));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(n))==false));
enabled=false;};
"math_port_atom_semantics_outer_TeXAtom_2"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<2;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"TeXAtom",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[116])));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(holder))==true));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(n))==false));
expect(bool((cs->size()>1&&cs->get(1).requireNode("Parent")->Parent()==m::NodeValue(holder))==true));
expect(bool((cs->size()>1&&cs->get(1).requireNode("Parent")->Parent()==m::NodeValue(n))==false));
enabled=false;};
"math_port_atom_semantics_outer_semantics_0"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<0;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"semantics",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[122])));
enabled=false;};
"math_port_atom_semantics_outer_semantics_1"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<1;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"semantics",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[127])));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(holder))==true));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(n))==false));
enabled=false;};
"math_port_atom_semantics_outer_semantics_2"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<2;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"semantics",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[131])));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(holder))==true));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(n))==false));
expect(bool((cs->size()>1&&cs->get(1).requireNode("Parent")->Parent()==m::NodeValue(holder))==true));
expect(bool((cs->size()>1&&cs->get(1).requireNode("Parent")->Parent()==m::NodeValue(n))==false));
enabled=false;};
"math_port_atom_semantics_outer_annotation-xml_0"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<0;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"annotation-xml",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[135])));
enabled=false;};
"math_port_atom_semantics_outer_annotation-xml_1"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<1;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"annotation-xml",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[140])));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(holder))==false));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(n))==true));
enabled=false;};
"math_port_atom_semantics_outer_annotation-xml_2"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<2;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"annotation-xml",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[144])));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(holder))==false));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(n))==true));
expect(bool((cs->size()>1&&cs->get(1).requireNode("Parent")->Parent()==m::NodeValue(holder))==false));
expect(bool((cs->size()>1&&cs->get(1).requireNode("Parent")->Parent()==m::NodeValue(n))==true));
enabled=false;};
"math_port_atom_semantics_outer_annotation_0"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<0;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"annotation",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[147])));
enabled=false;};
"math_port_atom_semantics_outer_annotation_1"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<1;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"annotation",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[152])));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(holder))==false));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(n))==true));
enabled=false;};
"math_port_atom_semantics_outer_annotation_2"_test=[] {reset();
NF f(bindings());auto cs=children();for(int i=0;i<2;i++)cs->push_back(create(f,"mi"));auto*n=create(f,"annotation",PropertyList{{"displaystyle",true}},cs);auto*holder=create(f,"mrow",{},children({n}));
enabled=true;atomTrace.clear();std::string failure;try{n->setInheritedAttributes(PropertyList{{"mathcolor",PropertyArray{"math","blue"}}},true,0,false);}catch(const m::Error&e){failure=e.name;}enabled=false;
expect(bool((failure)==""));
expect(deep(traceValues(),clone(sourceValues[23])));
expect(deep(snapshot(holder),clone(sourceValues[156])));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(holder))==false));
expect(bool((cs->size()>0&&cs->get(0).requireNode("Parent")->Parent()==m::NodeValue(n))==true));
expect(bool((cs->size()>1&&cs->get(1).requireNode("Parent")->Parent()==m::NodeValue(holder))==false));
expect(bool((cs->size()>1&&cs->get(1).requireNode("Parent")->Parent()==m::NodeValue(n))==true));
enabled=false;};
"math_port_atom_semantics_logical_parent"_test=[] {reset();
NF f(bindings());auto*child=create(f,"mi");auto*atom=create(f,"TeXAtom",{},children({child}));auto*sem=create(f,"semantics",{},children({atom,create(f,"annotation")}));auto*holder=create(f,"mrow",{},children({sem}));
expect(bool((child->Parent()==holder)==true));
expect(bool((atom->Parent()==holder)==true));
expect(bool((sem->Parent()==holder)==true));
enabled=false;};
"math_port_atom_semantics_native_shared_guard_depth"_test=[] {reset();for(const auto&kind:{"probe-TeXAtom","probe-mo"}){NF f(bindings());auto*n=create(f,kind);n->setTexClass(-1);const std::size_t margin=std::string_view(kind)=="probe-mo"?2:1;std::vector<std::unique_ptr<tr::TreeCallGuard<m::MmlNode>>>guards;for(std::size_t i=0;i<tr::TreeCallGuard<m::MmlNode>::limit-margin;++i)guards.push_back(std::make_unique<tr::TreeCallGuard<m::MmlNode>>());m::NodeValue result(n);std::string failure;try{result=adjust(n,nullptr);}catch(const m::Error&e){failure=e.name;}expect(failure.empty());expect(result==m::NodeValue(PropertyNull{}));}};
};
