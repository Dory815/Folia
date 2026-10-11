// Generated pinned actual-CJS table-family observations.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.mml_nodes.mtd;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.mml_nodes.merror;
import folia.math.core.mml_tree.mml_nodes.mfrac;
import folia.math.core.mml_tree.mml_nodes.msqrt;
import folia.math.core.mml_tree.mml_nodes.mroot;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.table_host;
import folia.math.core.tree.node_factory;
namespace m=folia::math::mml;namespace h=m::host;namespace tr=folia::math::tree;using folia::math::Property;using folia::math::PropertyList;using folia::math::PropertyArray;using folia::math::Missing;using folia::math::PropertyNull;using namespace boost::ut;namespace {
std::u16string units(std::initializer_list<unsigned>v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}
m::Children children(std::initializer_list<m::MmlNode*>v={}){return std::make_shared<std::vector<m::MmlNode*>>(v);}
bool deep(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return (std::isnan(*x)&&std::isnan(*y))||(*x==*y&&std::signbit(*x)==std::signbit(*y));auto x=PropertyList::record(a),y=PropertyList::record(b);if(x&&y){if(PropertyArray::array(a).has_value()!=PropertyArray::array(b).has_value()||x->keys()!=y->keys())return false;for(const auto&k:x->keys())if(!deep(x->getOwn(k),y->getOwn(k)))return false;return true;}return a==b;}
Property clone(const Property&v){if(auto a=PropertyArray::array(v)){PropertyArray out;for(std::size_t i=0;i<a->size();++i)out.push(clone(a->get(i)));return Property(out);}if(auto a=PropertyList::record(v)){PropertyList out;for(const auto&k:a->keys())out.setOwn(k,clone(a->getOwn(k)));return Property(out);}return v;}
bool enabled=false;std::vector<std::string>traceLog;std::string mode;struct Call{m::MmlNode*node;PropertyList a;Property d,l,p,snapshot;};std::vector<Call>calls;std::vector<m::MmlNode*>texCalls;m::Children originalChildren;m::MmlNode*target=nullptr,*late=nullptr;Property phaseBag,partialInherited;int attrReads=0;PropertyList changingInherited;void note(std::string s){if(enabled)traceLog.push_back(std::move(s));}
template<class T>struct Probe:T{using T::T;std::shared_ptr<folia::math::Attributes>attributes()const override{note("attributes");if(enabled&&mode=="indent-grow"&&++attrReads==1)m::indentAttributes.push_back("root-extra");if(enabled&&mode=="indent-argument"&&++attrReads==1)h::tableSetIndex(changingInherited.get("indentalign"),"1","after");return T::attributes();}void setChildren(m::Children c)override{note("phase/"+h::debugText(h::string(T::texClass())));if(enabled)phaseBag=T::getAllProperties();T::setChildren(c);}void getPrevClass(m::MmlNode*p)override{note("previous");T::getPrevClass(p);}PropertyList addInheritedAttributes(PropertyList a,PropertyList b)override{auto result=T::addInheritedAttributes(a,b);if(enabled&&mode=="null-inherit"){(*this->childNodes)[0]=nullptr;partialInherited=result;}return result;}void hook(PropertyList a,Property d,Property l,Property p){T::setChildInheritedAttributes(a,d,l,p);}void verify(PropertyList o){T::verifyChildren(o);}};
void receive(m::MmlNode*n,std::string_view label,PropertyList a,Property d,Property l,Property p){note("inherit/"+std::string(label));calls.push_back({n,a,d,l,p,clone(Property(a))});if(label=="first"&&mode=="tuple"){h::tableSetIndex(a.get("rowalign"),"1","callback");h::tableSetIndex(a.get("columnalign"),"1","callback");h::tableSetIndex(a.get("data-break-align"),"1","callback");}if(label=="first"&&mode=="swap")target->childNodes=children({n,late});}
m::MmlNode*tex(m::MmlNode*n,std::string_view label,m::MmlNode*p){note("tex/"+std::string(label)+"/"+(p?"prev":"null"));texCalls.push_back(n);if(label=="first"&&mode=="grow-rebind"){originalChildren->push_back(late);target->childNodes=children({late});}if(label=="first"&&mode=="swap")target->childNodes=children({n,late});return n;}
template<class T>struct Sink:T{using T::T;std::string label;void setInheritedAttributes(PropertyList a={},Property d=false,Property l=0,Property p=false)override{receive(this,label,a,d,l,p);}m::MmlNode*setTeXclass(m::MmlNode*p)override{return tex(this,label,p);}};using SinkRow=Sink<m::MmlMtr>;using SinkCell=Sink<m::MmlMtd>;using Table=Probe<m::MmlMtable>;using Row=Probe<m::MmlMtr>;using Label=Probe<m::MmlMlabeledtr>;using Cell=Probe<m::MmlMtd>;
using NF=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode>bindings(){return {{"probe-mtable",m::classBinding<Table>()},{"probe-mtr",m::classBinding<Row>()},{"probe-mlabeledtr",m::classBinding<Label>()},{"probe-mtd",m::classBinding<Cell>()},{"sinkrow",m::classBinding<SinkRow>()},{"sinkcell",m::classBinding<SinkCell>()},{"mtable",m::classBinding<m::MmlMtable>()},{"mtr",m::classBinding<m::MmlMtr>()},{"mlabeledtr",m::classBinding<m::MmlMlabeledtr>()},{"mtd",m::classBinding<m::MmlMtd>()},{"math",m::classBinding<m::MmlMath>()},{"mrow",m::classBinding<m::MmlMrow>()},{"inferredMrow",m::classBinding<m::MmlInferredMrow>()},{"mi",m::classBinding<m::MmlMi>()},{"mtext",m::classBinding<m::MmlMtext>()},{"merror",m::classBinding<m::MmlMerror>()},{"mfrac",m::classBinding<m::MmlMfrac>()},{"msqrt",m::classBinding<m::MmlMsqrt>()},{"mroot",m::classBinding<m::MmlMroot>()},{"text",m::classBinding<m::TextNode>()}};}
m::MmlNode*create(NF&f,std::string_view k,PropertyList a={},m::Children c=children()){return f.create(k,std::array<std::any,2>{a,c});}
void reset(){enabled=false;traceLog.clear();mode.clear();calls.clear();texCalls.clear();originalChildren.reset();target=late=nullptr;phaseBag=Missing{};partialInherited=Missing{};attrReads=0;changingInherited=PropertyList{};}
struct Setup{NF f{bindings()};m::MmlNode*n,*a,*b,*c;Setup(std::string_view k){reset();n=create(f,"probe-"+std::string(k));const auto sink=k=="mtable"?"sinkrow":"sinkcell";a=create(f,sink);b=create(f,sink);c=create(f,sink);if(auto*x=dynamic_cast<SinkRow*>(a)){x->label="first";dynamic_cast<SinkRow*>(b)->label="second";dynamic_cast<SinkRow*>(c)->label="late";}else{dynamic_cast<SinkCell*>(a)->label="first";dynamic_cast<SinkCell*>(b)->label="second";dynamic_cast<SinkCell*>(c)->label="late";}n->childNodes=children({a,b});a->parent=b->parent=c->parent=n;if(k=="mtr"||k=="mlabeledtr"){n->attributes()->set("columnalign","center");n->attributes()->set("rowalign","baseline");}if(k=="mlabeledtr")n->parent=create(f,"mtable");}};
void hook(m::MmlNode*n,PropertyList a,Property d,Property l,Property p){if(auto*x=dynamic_cast<Table*>(n))x->hook(a,d,l,p);else if(auto*x=dynamic_cast<Label*>(n))x->hook(a,d,l,p);else if(auto*x=dynamic_cast<Row*>(n))x->hook(a,d,l,p);else dynamic_cast<Cell*>(n)->hook(a,d,l,p);}
void verify(m::MmlNode*n,PropertyList o){if(auto*x=dynamic_cast<Table*>(n))x->verify(o);else if(auto*x=dynamic_cast<Label*>(n))x->verify(o);else if(auto*x=dynamic_cast<Row*>(n))x->verify(o);else dynamic_cast<Cell*>(n)->verify(o);}
m::MmlNode*token(NF&f,std::u16string s){auto*t=dynamic_cast<m::TextNode*>(f.create("text"));t->setText(s);return create(f,"mi",{},children({t}));}
Property snapshot(m::MmlNode*n){if(!n)return Property(PropertyNull{});PropertyList result{{"kind",n->kind()},{"parent",n->parent?Property(n->parent->kind()):Property(PropertyNull{})},{"properties",n->getAllProperties()},{"tex",n->texClass()}};if(auto a=n->attributes()){result.setOwn("explicit",a->getAllAttributes());result.setOwn("inherited",a->getAllInherited());}if(auto*t=dynamic_cast<m::TextNode*>(n))result.setOwn("text",h::debugText(t->getText()));if(!n->childNodes)result.setOwn("children",PropertyNull{});else{PropertyArray items;for(auto*c:*n->childNodes)items.push(snapshot(c));result.setOwn("children",items);}return Property(result);}
}
namespace { const std::vector<Property> sourceValues=[] {std::vector<Property> values;values.reserve(428);
values.push_back(Property(std::numeric_limits<double>::infinity()));
values.push_back(Property(0));
values.push_back(Property(true));
values.push_back(Property(PropertyList{{"useHeight",values[2]}}));
values.push_back(Property(h::debugText(units({95,105,110,104,101,114,105,116,95}))));
values.push_back(Property(h::debugText(units({97,120,105,115}))));
values.push_back(Property(h::debugText(units({98,97,115,101,108,105,110,101}))));
values.push_back(Property(h::debugText(units({99,101,110,116,101,114}))));
values.push_back(Property(h::debugText(units({123,108,101,102,116,125}))));
values.push_back(Property(h::debugText(units({97,117,116,111}))));
values.push_back(Property(h::debugText(units({49,101,120}))));
values.push_back(Property(h::debugText(units({46,56,101,109}))));
values.push_back(Property(h::debugText(units({110,111,110,101}))));
values.push_back(Property(h::debugText(units({48,46,52,101,109,32,48,46,53,101,120}))));
values.push_back(Property(false));
values.push_back(Property(h::debugText(units({114,105,103,104,116}))));
values.push_back(Property(h::debugText(units({48,46,56,101,109}))));
values.push_back(Property(h::debugText(units({116,111,112}))));
values.push_back(Property(PropertyNull{}));
values.push_back(Property(PropertyList{}));
values.push_back(Property(1));
values.push_back(Property(-1));
values.push_back(Property(Missing{}));
values.push_back(Property(h::debugText(units({116,101,115,116}))));
values.push_back(Property(5));
values.push_back(Property(PropertyArray{values[23],values[24]}));
values.push_back(Property(h::debugText(units({109,116,97,98,108,101}))));
values.push_back(Property(PropertyArray{values[26],values[7]}));
values.push_back(Property(PropertyArray{values[26],values[6]}));
values.push_back(Property(PropertyArray{values[26],values[17]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"columnalign",values[27]},{"rowalign",values[28]},{"data-break-align",values[29]}}));
values.push_back(Property(h::debugText(units({114,97,119}))));
values.push_back(Property(h::debugText(units({50}))));
values.push_back(Property(h::debugText(units({109,116,114}))));
values.push_back(Property(PropertyArray{values[33],values[6]}));
values.push_back(Property(PropertyArray{values[33],values[7]}));
values.push_back(Property(PropertyArray{values[33],values[17]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"rowalign",values[34]},{"columnalign",values[35]},{"data-break-align",values[36]},{"data-vertical-align",values[36]}}));
values.push_back(Property(h::debugText(units({109,108,97,98,101,108,101,100,116,114}))));
values.push_back(Property(PropertyArray{values[38],values[6]}));
values.push_back(Property(PropertyArray{values[38],values[15]}));
values.push_back(Property(PropertyArray{values[38],values[17]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"rowalign",values[39]},{"columnalign",values[40]},{"data-break-align",values[41]},{"data-vertical-align",values[41]}}));
values.push_back(Property(PropertyArray{values[38],values[7]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"rowalign",values[39]},{"columnalign",values[43]},{"data-break-align",values[41]},{"data-vertical-align",values[41]}}));
values.push_back(Property(h::debugText(units({98,111,116,116,111,109,32,99,101,110,116,101,114}))));
values.push_back(Property(PropertyArray{values[26],values[45]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"columnalign",values[27]},{"rowalign",values[29]},{"data-break-align",values[46]}}));
values.push_back(Property(h::debugText(units({98,111,116,116,111,109}))));
values.push_back(Property(PropertyArray{values[26],values[48]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"columnalign",values[27]},{"rowalign",values[49]},{"data-break-align",values[46]}}));
values.push_back(Property(PropertyList{{"extra",values[25]}}));
values.push_back(Property(h::debugText(units({99,97,108,108,98,97,99,107}))));
values.push_back(Property(PropertyArray{values[26],values[52]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"columnalign",values[53]},{"rowalign",values[49]},{"data-break-align",values[53]}}));
values.push_back(Property(h::debugText(units({108,101,102,116}))));
values.push_back(Property(PropertyArray{values[33],values[55]}));
values.push_back(Property(PropertyArray{values[33],values[48]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"rowalign",values[34]},{"columnalign",values[56]},{"data-break-align",values[36]},{"data-vertical-align",values[57]}}));
values.push_back(Property(PropertyArray{values[33],values[15]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"rowalign",values[34]},{"columnalign",values[59]},{"data-break-align",values[36]},{"data-vertical-align",values[35]}}));
values.push_back(Property(PropertyArray{values[33],values[52]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"rowalign",values[61]},{"columnalign",values[59]},{"data-break-align",values[61]},{"data-vertical-align",values[35]}}));
values.push_back(Property(PropertyArray{values[38],values[55]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"rowalign",values[39]},{"columnalign",values[63]},{"data-break-align",values[41]},{"data-vertical-align",values[41]}}));
values.push_back(Property(PropertyArray{values[38],values[48]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"rowalign",values[39]},{"columnalign",values[63]},{"data-break-align",values[41]},{"data-vertical-align",values[65]}}));
values.push_back(Property(PropertyArray{values[38],values[52]}));
values.push_back(Property(PropertyList{{"extra",values[25]},{"rowalign",values[67]},{"columnalign",values[63]},{"data-break-align",values[67]},{"data-vertical-align",values[65]}}));
values.push_back(Property(2));
values.push_back(Property(PropertyArray{}));
values.push_back(Property(PropertyArray{values[55]}));
values.push_back(Property(h::debugText(units({}))));
values.push_back(Property(PropertyList{{"columnalign",values[27]},{"rowalign",values[27]},{"data-break-align",values[29]}}));
values.push_back(Property(h::debugText(units({32,32,108,101,102,116,160,114,105,103,104,116,10,99,101,110,116,101,114,32,32}))));
values.push_back(Property(PropertyArray{values[26],values[55]}));
values.push_back(Property(PropertyList{{"columnalign",values[27]},{"rowalign",values[75]},{"data-break-align",values[29]}}));
values.push_back(Property(PropertyArray{values[26],values[15]}));
values.push_back(Property(PropertyList{{"columnalign",values[27]},{"rowalign",values[77]},{"data-break-align",values[29]}}));
values.push_back(Property(PropertyList{{"rowalign",values[34]},{"columnalign",values[35]},{"data-break-align",values[36]},{"data-vertical-align",values[36]}}));
values.push_back(Property(PropertyList{{"rowalign",values[34]},{"columnalign",values[56]},{"data-break-align",values[36]},{"data-vertical-align",values[36]}}));
values.push_back(Property(PropertyList{{"rowalign",values[34]},{"columnalign",values[59]},{"data-break-align",values[36]},{"data-vertical-align",values[36]}}));
values.push_back(Property(PropertyList{{"fixMtables",values[22]},{"fullErrors",values[14]}}));
values.push_back(Property(h::debugText(units({109,114,111,119}))));
values.push_back(Property(h::debugText(units({109,116,100}))));
values.push_back(Property(h::debugText(units({105,110,102,101,114,114,101,100,77,114,111,119}))));
values.push_back(Property(h::debugText(units({109,101,114,114,111,114}))));
values.push_back(Property(h::debugText(units({67,104,105,108,100,114,101,110,32,111,102,32,109,116,97,98,108,101,32,109,117,115,116,32,98,101,32,109,116,114,32,111,114,32,109,108,97,98,101,108,101,100,116,114}))));
values.push_back(Property(PropertyList{{"data-mjx-message",values[87]},{"title",values[87]}}));
values.push_back(Property(h::debugText(units({109,116,101,120,116}))));
values.push_back(Property(h::debugText(units({116,101,120,116}))));
values.push_back(Property(PropertyList{{"kind",values[90]},{"parent",values[89]},{"properties",values[19]},{"tex",values[21]},{"text",values[84]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[91]}));
values.push_back(Property(PropertyList{{"kind",values[89]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[92]}}));
values.push_back(Property(PropertyArray{values[93]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[86]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[94]}}));
values.push_back(Property(PropertyArray{values[95]}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[88]},{"inherited",values[19]},{"children",values[96]}}));
values.push_back(Property(PropertyArray{values[97]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[98]}}));
values.push_back(Property(PropertyArray{values[99]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[33]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[100]}}));
values.push_back(Property(PropertyArray{values[101],values[101]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[102]}}));
values.push_back(Property(PropertyArray{values[103]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[83]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[104]}}));
values.push_back(Property(PropertyArray{values[105]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[106]}}));
values.push_back(Property(PropertyList{{"data-mjx-message",values[87]}}));
values.push_back(Property(h::debugText(units({109,105}))));
values.push_back(Property(h::debugText(units({120}))));
values.push_back(Property(PropertyList{{"kind",values[90]},{"parent",values[109]},{"properties",values[19]},{"tex",values[21]},{"text",values[110]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[111]}));
values.push_back(Property(PropertyList{{"kind",values[109]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[112]}}));
values.push_back(Property(PropertyArray{values[113]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[86]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[114]}}));
values.push_back(Property(PropertyArray{values[115]}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[108]},{"inherited",values[19]},{"children",values[116]}}));
values.push_back(Property(PropertyArray{values[117]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[118]}}));
values.push_back(Property(PropertyArray{values[119]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[33]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[120]}}));
values.push_back(Property(PropertyArray{values[101],values[121],values[101]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[122]}}));
values.push_back(Property(PropertyArray{values[123]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[83]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[124]}}));
values.push_back(Property(PropertyArray{values[125]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[126]}}));
values.push_back(Property(PropertyArray{values[101]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[128]}}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[130]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[33]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[131]}}));
values.push_back(Property(PropertyArray{values[132]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[133]}}));
values.push_back(Property(PropertyArray{values[121],values[101]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[135]}}));
values.push_back(Property(PropertyArray{values[129],values[134],values[136]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[83]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[137]}}));
values.push_back(Property(PropertyArray{values[138]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[139]}}));
values.push_back(Property(PropertyList{{"fixMtables",values[22]},{"fullErrors",values[2]}}));
values.push_back(Property(PropertyList{{"kind",values[90]},{"parent",values[89]},{"properties",values[19]},{"tex",values[21]},{"text",values[87]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[142]}));
values.push_back(Property(PropertyList{{"kind",values[89]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[143]}}));
values.push_back(Property(PropertyArray{values[144]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[86]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[145]}}));
values.push_back(Property(PropertyArray{values[146]}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[108]},{"inherited",values[19]},{"children",values[147]}}));
values.push_back(Property(PropertyArray{values[148]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[149]}}));
values.push_back(Property(PropertyArray{values[150]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[33]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[151]}}));
values.push_back(Property(PropertyArray{values[152],values[152]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[153]}}));
values.push_back(Property(PropertyArray{values[154]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[83]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[155]}}));
values.push_back(Property(PropertyArray{values[156]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[157]}}));
values.push_back(Property(PropertyArray{values[152],values[152],values[152]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[159]}}));
values.push_back(Property(PropertyArray{values[160]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[83]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[161]}}));
values.push_back(Property(PropertyArray{values[162]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[163]}}));
values.push_back(Property(PropertyArray{values[152]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[165]}}));
values.push_back(Property(PropertyArray{values[166],values[134],values[154]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[83]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[167]}}));
values.push_back(Property(PropertyArray{values[168]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[169]}}));
values.push_back(Property(PropertyList{{"fixMtables",values[14]},{"fullErrors",values[14]}}));
values.push_back(Property(PropertyList{{"fixMtables",values[14]},{"fullErrors",values[2]}}));
values.push_back(Property(PropertyList{{"fixMtables",values[2]},{"fullErrors",values[14]}}));
values.push_back(Property(PropertyArray{values[132],values[132]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[174]}}));
values.push_back(Property(PropertyArray{values[175]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[83]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[176]}}));
values.push_back(Property(PropertyArray{values[177]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[178]}}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[114]}}));
values.push_back(Property(PropertyArray{values[180]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[33]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[181]}}));
values.push_back(Property(PropertyArray{values[132],values[182],values[132]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[183]}}));
values.push_back(Property(PropertyArray{values[184]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[83]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[185]}}));
values.push_back(Property(PropertyArray{values[186]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[187]}}));
values.push_back(Property(PropertyArray{values[182],values[132]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[189]}}));
values.push_back(Property(PropertyArray{values[134],values[134],values[190]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[83]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[191]}}));
values.push_back(Property(PropertyArray{values[192]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[193]}}));
values.push_back(Property(PropertyList{{"fixMtables",values[2]},{"fullErrors",values[2]}}));
values.push_back(Property(PropertyList{{"fixMtables",values[31]},{"fullErrors",values[14]}}));
values.push_back(Property(PropertyList{{"fixMtables",values[31]},{"fullErrors",values[2]}}));
values.push_back(Property(h::debugText(units({67,104,105,108,100,114,101,110,32,111,102,32,109,116,114,32,109,117,115,116,32,98,101,32,109,116,100}))));
values.push_back(Property(PropertyList{{"data-mjx-message",values[198]}}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[199]},{"inherited",values[19]},{"children",values[116]}}));
values.push_back(Property(PropertyArray{values[200]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[201]}}));
values.push_back(Property(PropertyArray{values[202]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[33]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[203]}}));
values.push_back(Property(PropertyArray{values[204]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[205]}}));
values.push_back(Property(PropertyArray{values[182]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[207]}}));
values.push_back(Property(h::debugText(units({109,116,114,32,99,97,110,32,111,110,108,121,32,98,101,32,97,32,99,104,105,108,100,32,111,102,32,97,110,32,109,116,97,98,108,101}))));
values.push_back(Property(PropertyList{{"data-mjx-message",values[209]},{"title",values[209]}}));
values.push_back(Property(PropertyList{{"kind",values[90]},{"parent",values[89]},{"properties",values[19]},{"tex",values[21]},{"text",values[33]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[211]}));
values.push_back(Property(PropertyList{{"kind",values[89]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[212]}}));
values.push_back(Property(PropertyArray{values[213]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[86]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[214]}}));
values.push_back(Property(PropertyArray{values[215]}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[83]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[210]},{"inherited",values[19]},{"children",values[216]}}));
values.push_back(Property(PropertyArray{values[217]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[218]}}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[205]}}));
values.push_back(Property(PropertyArray{values[220]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[221]}}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[207]}}));
values.push_back(Property(PropertyArray{values[223]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[224]}}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[33]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[210]},{"inherited",values[19]},{"children",values[216]}}));
values.push_back(Property(PropertyArray{values[226]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[227]}}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[38]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[210]},{"inherited",values[19]},{"children",values[216]}}));
values.push_back(Property(PropertyArray{values[229]}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[230]}}));
values.push_back(Property(h::debugText(units({67,104,105,108,100,114,101,110,32,111,102,32,109,108,97,98,101,108,101,100,116,114,32,109,117,115,116,32,98,101,32,109,116,100}))));
values.push_back(Property(PropertyList{{"data-mjx-message",values[232]}}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[233]},{"inherited",values[19]},{"children",values[116]}}));
values.push_back(Property(PropertyArray{values[234]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[235]}}));
values.push_back(Property(PropertyArray{values[236]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[38]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[237]}}));
values.push_back(Property(PropertyArray{values[238]}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[239]}}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[38]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[181]}}));
values.push_back(Property(PropertyArray{values[241]}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[242]}}));
values.push_back(Property(h::debugText(units({109,108,97,98,101,108,101,100,116,114,32,99,97,110,32,111,110,108,121,32,98,101,32,97,32,99,104,105,108,100,32,111,102,32,97,110,32,109,116,97,98,108,101}))));
values.push_back(Property(PropertyList{{"data-mjx-message",values[244]},{"title",values[244]}}));
values.push_back(Property(PropertyList{{"kind",values[90]},{"parent",values[89]},{"properties",values[19]},{"tex",values[21]},{"text",values[38]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[246]}));
values.push_back(Property(PropertyList{{"kind",values[89]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[247]}}));
values.push_back(Property(PropertyArray{values[248]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[86]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[249]}}));
values.push_back(Property(PropertyArray{values[250]}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[83]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[245]},{"inherited",values[19]},{"children",values[251]}}));
values.push_back(Property(PropertyArray{values[252]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[253]}}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[239]}}));
values.push_back(Property(PropertyArray{values[255]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[256]}}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[242]}}));
values.push_back(Property(PropertyArray{values[258]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[259]}}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[33]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[245]},{"inherited",values[19]},{"children",values[251]}}));
values.push_back(Property(PropertyArray{values[261]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[262]}}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[38]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[245]},{"inherited",values[19]},{"children",values[251]}}));
values.push_back(Property(PropertyArray{values[264]}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[265]}}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[181]}}));
values.push_back(Property(h::debugText(units({109,116,100,32,99,97,110,32,111,110,108,121,32,98,101,32,97,32,99,104,105,108,100,32,111,102,32,97,110,32,109,116,114,32,111,114,32,109,108,97,98,101,108,101,100,116,114}))));
values.push_back(Property(PropertyList{{"data-mjx-message",values[268]},{"title",values[268]}}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[83]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[269]},{"inherited",values[19]},{"children",values[96]}}));
values.push_back(Property(PropertyArray{values[270]}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[271]}}));
values.push_back(Property(PropertyList{{"kind",values[86]},{"parent",values[26]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[269]},{"inherited",values[19]},{"children",values[96]}}));
values.push_back(Property(PropertyArray{values[273]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[274]}}));
values.push_back(Property(PropertyList{{"displaystyle",values[14]},{"scriptlevel",values[32]},{"data-cramped",values[2]}}));
values.push_back(Property(h::debugText(units({109,97,116,104}))));
values.push_back(Property(h::debugText(units({98,108,117,101}))));
values.push_back(Property(PropertyArray{values[277],values[278]}));
values.push_back(Property(PropertyArray{values[277],values[15]}));
values.push_back(Property(h::debugText(units({50,101,109}))));
values.push_back(Property(PropertyArray{values[277],values[281]}));
values.push_back(Property(PropertyList{{"mathcolor",values[279]},{"indentalign",values[280]},{"indentshift",values[282]}}));
values.push_back(Property(PropertyList{{"useHeight",values[2]},{"texprimestyle",values[2]}}));
values.push_back(Property(PropertyList{{"indentalign",values[15]},{"indentshift",values[281]},{"mathcolor",values[278]}}));
values.push_back(Property(PropertyList{{"texprimestyle",values[2]}}));
values.push_back(Property(PropertyList{{"mathcolor",values[278]},{"columnalign",values[7]},{"rowalign",values[6]},{"data-break-align",values[17]},{"displaystyle",values[14]},{"scriptlevel",values[1]}}));
values.push_back(Property(PropertyList{{"mathcolor",values[278]},{"columnalign",values[7]},{"rowalign",values[6]},{"data-vertical-align",values[17]},{"displaystyle",values[14]},{"scriptlevel",values[1]}}));
values.push_back(Property(PropertyList{{"mathcolor",values[278]},{"displaystyle",values[14]},{"scriptlevel",values[1]}}));
values.push_back(Property(h::debugText(units({105,116,97,108,105,99}))));
values.push_back(Property(PropertyList{{"mathcolor",values[278]},{"displaystyle",values[14]},{"scriptlevel",values[1]},{"mathvariant",values[290]}}));
values.push_back(Property(PropertyList{{"kind",values[109]},{"parent",values[85]},{"properties",values[286]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[291]},{"children",values[112]}}));
values.push_back(Property(PropertyArray{values[292]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[289]},{"children",values[293]}}));
values.push_back(Property(PropertyArray{values[294]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[33]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[288]},{"children",values[295]}}));
values.push_back(Property(PropertyArray{values[296]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[287]},{"children",values[297]}}));
values.push_back(Property(PropertyArray{values[298]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[284]},{"tex",values[1]},{"explicit",values[276]},{"inherited",values[285]},{"children",values[299]}}));
values.push_back(Property(PropertyList{{"displaystyle",values[2]},{"scriptlevel",values[32]},{"data-cramped",values[2]}}));
values.push_back(Property(PropertyList{{"mathcolor",values[278]},{"columnalign",values[7]},{"rowalign",values[6]},{"data-break-align",values[17]},{"displaystyle",values[2]},{"scriptlevel",values[1]}}));
values.push_back(Property(PropertyList{{"mathcolor",values[278]},{"columnalign",values[7]},{"rowalign",values[6]},{"data-vertical-align",values[17]},{"displaystyle",values[2]},{"scriptlevel",values[1]}}));
values.push_back(Property(PropertyList{{"mathcolor",values[278]},{"displaystyle",values[2]},{"scriptlevel",values[1]}}));
values.push_back(Property(PropertyList{{"mathcolor",values[278]},{"displaystyle",values[2]},{"scriptlevel",values[1]},{"mathvariant",values[290]}}));
values.push_back(Property(PropertyList{{"kind",values[109]},{"parent",values[85]},{"properties",values[286]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[305]},{"children",values[112]}}));
values.push_back(Property(PropertyArray{values[306]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[304]},{"children",values[307]}}));
values.push_back(Property(PropertyArray{values[308]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[33]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[303]},{"children",values[309]}}));
values.push_back(Property(PropertyArray{values[310]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[302]},{"children",values[311]}}));
values.push_back(Property(PropertyArray{values[312]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[284]},{"tex",values[1]},{"explicit",values[301]},{"inherited",values[285]},{"children",values[313]}}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[284]},{"tex",values[1]},{"explicit",values[276]},{"inherited",values[285]},{"children",values[70]}}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[284]},{"tex",values[1]},{"explicit",values[301]},{"inherited",values[285]},{"children",values[70]}}));
values.push_back(Property(PropertyList{{"mathcolor",values[278]}}));
values.push_back(Property(h::debugText(units({121}))));
values.push_back(Property(PropertyList{{"kind",values[90]},{"parent",values[109]},{"properties",values[19]},{"tex",values[21]},{"text",values[318]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[319]}));
values.push_back(Property(PropertyList{{"kind",values[109]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[320]}}));
values.push_back(Property(PropertyArray{values[321]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[322]}}));
values.push_back(Property(PropertyArray{values[323]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[33]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[324]}}));
values.push_back(Property(PropertyArray{values[182],values[325]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[18]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[276]},{"inherited",values[317]},{"children",values[326]}}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[18]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[301]},{"inherited",values[317]},{"children",values[326]}}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[18]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[276]},{"inherited",values[317]},{"children",values[70]}}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[18]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[301]},{"inherited",values[317]},{"children",values[70]}}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[38]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[324]}}));
values.push_back(Property(PropertyArray{values[241],values[331]}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[26]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[276]},{"inherited",values[317]},{"children",values[332]}}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[26]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[301]},{"inherited",values[317]},{"children",values[332]}}));
values.push_back(Property(PropertyList{{"kind",values[83]},{"parent",values[85]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[335]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[336]}}));
values.push_back(Property(PropertyArray{values[337]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[38]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[338]}}));
values.push_back(Property(PropertyArray{values[339]}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[26]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[276]},{"inherited",values[317]},{"children",values[340]}}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[26]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[301]},{"inherited",values[317]},{"children",values[340]}}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[18]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[276]},{"inherited",values[317]},{"children",values[309]}}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[18]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[301]},{"inherited",values[317]},{"children",values[309]}}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[304]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[345]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[18]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[276]},{"inherited",values[317]},{"children",values[346]}}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[18]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[301]},{"inherited",values[317]},{"children",values[346]}}));
values.push_back(Property(PropertyList{{"columnalign",values[75]},{"rowalign",values[29]},{"data-break-align",values[49]}}));
values.push_back(Property(PropertyList{{"columnalign",values[53]},{"rowalign",values[53]},{"data-break-align",values[53]}}));
values.push_back(Property(PropertyList{{"rowalign",values[36]},{"columnalign",values[56]},{"data-break-align",values[36]},{"data-vertical-align",values[57]}}));
values.push_back(Property(PropertyList{{"rowalign",values[61]},{"columnalign",values[61]},{"data-break-align",values[61]},{"data-vertical-align",values[61]}}));
values.push_back(Property(PropertyList{{"rowalign",values[41]},{"columnalign",values[40]},{"data-break-align",values[41]},{"data-vertical-align",values[41]}}));
values.push_back(Property(PropertyList{{"rowalign",values[67]},{"columnalign",values[63]},{"data-break-align",values[67]},{"data-vertical-align",values[67]}}));
values.push_back(Property(PropertyList{{"rowalign",values[67]},{"columnalign",values[63]},{"data-break-align",values[67]},{"data-vertical-align",values[65]}}));
values.push_back(Property(PropertyList{{"columnalign",values[77]},{"rowalign",values[29]},{"data-break-align",values[49]}}));
values.push_back(Property(PropertyList{{"rowalign",values[17]},{"columnalign",values[15]},{"data-break-align",values[48]}}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[18],values[358]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[357]},{"inherited",values[19]},{"children",values[359]}}));
values.push_back(Property(PropertyList{{"rowalign",values[36]},{"columnalign",values[59]},{"data-break-align",values[36]},{"data-vertical-align",values[57]}}));
values.push_back(Property(PropertyList{{"columnalign",values[15]},{"rowalign",values[17]},{"data-break-align",values[48]}}));
values.push_back(Property(PropertyArray{values[18],values[132]}));
values.push_back(Property(PropertyList{{"kind",values[33]},{"parent",values[18]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[362]},{"inherited",values[19]},{"children",values[363]}}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[38]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[19]},{"children",values[131]}}));
values.push_back(Property(PropertyArray{values[18],values[365]}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[362]},{"inherited",values[19]},{"children",values[366]}}));
values.push_back(Property(h::debugText(units({98,101,102,111,114,101}))));
values.push_back(Property(PropertyArray{values[277],values[368]}));
values.push_back(Property(h::debugText(units({52,50,112,120}))));
values.push_back(Property(PropertyArray{values[277],values[370]}));
values.push_back(Property(PropertyList{{"indentalign",values[369]},{"root-extra",values[371]}}));
values.push_back(Property(PropertyList{{"indentalign",values[368]},{"root-extra",values[370]},{"displaystyle",values[14]},{"scriptlevel",values[1]}}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[373]},{"children",values[70]}}));
values.push_back(Property(h::debugText(units({101,120,112,108,105,99,105,116}))));
values.push_back(Property(PropertyList{{"root-extra",values[375]}}));
values.push_back(Property(h::debugText(units({97,102,116,101,114}))));
values.push_back(Property(PropertyList{{"indentalign",values[377]},{"displaystyle",values[14]},{"scriptlevel",values[1]}}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[376]},{"inherited",values[378]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[277],values[377]}));
values.push_back(Property(PropertyList{{"indentalign",values[380]},{"root-extra",values[371]}}));
values.push_back(Property(PropertyList{{"displaystyle",values[2]},{"scriptlevel",values[1]}}));
values.push_back(Property(PropertyList{{"columnalign",values[7]},{"rowalign",values[6]},{"data-break-align",values[17]},{"displaystyle",values[14]},{"scriptlevel",values[1]}}));
values.push_back(Property(PropertyList{{"columnalign",values[15]},{"rowalign",values[6]},{"data-vertical-align",values[17]},{"displaystyle",values[14]},{"scriptlevel",values[1]}}));
values.push_back(Property(PropertyList{{"displaystyle",values[14]},{"scriptlevel",values[1]}}));
values.push_back(Property(h::debugText(units({108,97,98,101,108}))));
values.push_back(Property(PropertyList{{"kind",values[90]},{"parent",values[109]},{"properties",values[19]},{"tex",values[21]},{"text",values[386]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[387]}));
values.push_back(Property(PropertyList{{"kind",values[109]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[385]},{"children",values[388]}}));
values.push_back(Property(PropertyArray{values[389]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[385]},{"children",values[390]}}));
values.push_back(Property(PropertyArray{values[391]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[38]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[384]},{"children",values[392]}}));
values.push_back(Property(PropertyList{{"columnalign",values[7]},{"rowalign",values[6]},{"data-vertical-align",values[17]},{"displaystyle",values[14]},{"scriptlevel",values[1]}}));
values.push_back(Property(h::debugText(units({109,114,111,111,116}))));
values.push_back(Property(h::debugText(units({109,115,113,114,116}))));
values.push_back(Property(h::debugText(units({109,102,114,97,99}))));
values.push_back(Property(PropertyList{{"displaystyle",values[14]},{"scriptlevel",values[20]},{"mathvariant",values[290]}}));
values.push_back(Property(h::debugText(units({97}))));
values.push_back(Property(PropertyList{{"kind",values[90]},{"parent",values[109]},{"properties",values[19]},{"tex",values[21]},{"text",values[399]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[400]}));
values.push_back(Property(PropertyList{{"kind",values[109]},{"parent",values[397]},{"properties",values[286]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[398]},{"children",values[401]}}));
values.push_back(Property(h::debugText(units({98}))));
values.push_back(Property(PropertyList{{"kind",values[90]},{"parent",values[109]},{"properties",values[19]},{"tex",values[21]},{"text",values[403]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[404]}));
values.push_back(Property(PropertyList{{"kind",values[109]},{"parent",values[397]},{"properties",values[286]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[398]},{"children",values[405]}}));
values.push_back(Property(PropertyArray{values[402],values[406]}));
values.push_back(Property(PropertyList{{"kind",values[397]},{"parent",values[85]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[385]},{"children",values[407]}}));
values.push_back(Property(PropertyArray{values[408]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[396]},{"properties",values[286]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[385]},{"children",values[409]}}));
values.push_back(Property(PropertyArray{values[410]}));
values.push_back(Property(PropertyList{{"kind",values[396]},{"parent",values[395]},{"properties",values[286]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[385]},{"children",values[411]}}));
values.push_back(Property(PropertyList{{"displaystyle",values[14]},{"scriptlevel",values[69]},{"mathvariant",values[290]}}));
values.push_back(Property(h::debugText(units({110}))));
values.push_back(Property(PropertyList{{"kind",values[90]},{"parent",values[109]},{"properties",values[19]},{"tex",values[21]},{"text",values[414]},{"children",values[70]}}));
values.push_back(Property(PropertyArray{values[415]}));
values.push_back(Property(PropertyList{{"kind",values[109]},{"parent",values[395]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[413]},{"children",values[416]}}));
values.push_back(Property(PropertyArray{values[412],values[417]}));
values.push_back(Property(PropertyList{{"kind",values[395]},{"parent",values[85]},{"properties",values[19]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[385]},{"children",values[418]}}));
values.push_back(Property(PropertyArray{values[419]}));
values.push_back(Property(PropertyList{{"kind",values[85]},{"parent",values[84]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[385]},{"children",values[420]}}));
values.push_back(Property(PropertyArray{values[421]}));
values.push_back(Property(PropertyList{{"kind",values[84]},{"parent",values[38]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[394]},{"children",values[422]}}));
values.push_back(Property(PropertyArray{values[393],values[423]}));
values.push_back(Property(PropertyList{{"kind",values[38]},{"parent",values[26]},{"properties",values[19]},{"tex",values[18]},{"explicit",values[19]},{"inherited",values[383]},{"children",values[424]}}));
values.push_back(Property(PropertyArray{values[425]}));
values.push_back(Property(PropertyList{{"kind",values[26]},{"parent",values[18]},{"properties",values[3]},{"tex",values[1]},{"explicit",values[19]},{"inherited",values[382]},{"children",values[426]}}));
return values;}(); }
suite math_port_table_tests=[] {
"math_port_table_defaults_fields_and_class_identity"_test=[] {reset();
NF f(bindings());
{auto*n=create(f,"mtable");
expect(bool((n->kind())=="mtable"));
expect(deep(Property(n->arity()),clone(sourceValues[0])));
expect(bool((n->linebreakContainer())==true));
expect(bool((n->linebreakAlign())==""));
expect(deep(n->texClass(),clone(sourceValues[1])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[3])));
expect(deep(m::MmlMtable::defaults().getOwn("mathbackground"),clone(sourceValues[4])));
expect(deep(m::MmlMtable::defaults().getOwn("mathcolor"),clone(sourceValues[4])));
expect(deep(m::MmlMtable::defaults().getOwn("mathsize"),clone(sourceValues[4])));
expect(deep(m::MmlMtable::defaults().getOwn("dir"),clone(sourceValues[4])));
expect(deep(m::MmlMtable::defaults().getOwn("align"),clone(sourceValues[5])));
expect(deep(m::MmlMtable::defaults().getOwn("rowalign"),clone(sourceValues[6])));
expect(deep(m::MmlMtable::defaults().getOwn("columnalign"),clone(sourceValues[7])));
expect(deep(m::MmlMtable::defaults().getOwn("groupalign"),clone(sourceValues[8])));
expect(deep(m::MmlMtable::defaults().getOwn("alignmentscope"),clone(sourceValues[2])));
expect(deep(m::MmlMtable::defaults().getOwn("columnwidth"),clone(sourceValues[9])));
expect(deep(m::MmlMtable::defaults().getOwn("width"),clone(sourceValues[9])));
expect(deep(m::MmlMtable::defaults().getOwn("rowspacing"),clone(sourceValues[10])));
expect(deep(m::MmlMtable::defaults().getOwn("columnspacing"),clone(sourceValues[11])));
expect(deep(m::MmlMtable::defaults().getOwn("rowlines"),clone(sourceValues[12])));
expect(deep(m::MmlMtable::defaults().getOwn("columnlines"),clone(sourceValues[12])));
expect(deep(m::MmlMtable::defaults().getOwn("frame"),clone(sourceValues[12])));
expect(deep(m::MmlMtable::defaults().getOwn("framespacing"),clone(sourceValues[13])));
expect(deep(m::MmlMtable::defaults().getOwn("equalrows"),clone(sourceValues[14])));
expect(deep(m::MmlMtable::defaults().getOwn("equalcolumns"),clone(sourceValues[14])));
expect(deep(m::MmlMtable::defaults().getOwn("displaystyle"),clone(sourceValues[14])));
expect(deep(m::MmlMtable::defaults().getOwn("side"),clone(sourceValues[15])));
expect(deep(m::MmlMtable::defaults().getOwn("minlabelspacing"),clone(sourceValues[16])));
expect(deep(m::MmlMtable::defaults().getOwn("data-break-align"),clone(sourceValues[17])));
}
{auto*n=create(f,"mtr");
expect(bool((n->kind())=="mtr"));
expect(deep(Property(n->arity()),clone(sourceValues[0])));
expect(bool((n->linebreakContainer())==true));
expect(bool((n->linebreakAlign())==""));
expect(deep(n->texClass(),clone(sourceValues[18])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[19])));
expect(deep(m::MmlMtr::defaults().getOwn("mathbackground"),clone(sourceValues[4])));
expect(deep(m::MmlMtr::defaults().getOwn("mathcolor"),clone(sourceValues[4])));
expect(deep(m::MmlMtr::defaults().getOwn("mathsize"),clone(sourceValues[4])));
expect(deep(m::MmlMtr::defaults().getOwn("dir"),clone(sourceValues[4])));
expect(deep(m::MmlMtr::defaults().getOwn("rowalign"),clone(sourceValues[4])));
expect(deep(m::MmlMtr::defaults().getOwn("columnalign"),clone(sourceValues[4])));
expect(deep(m::MmlMtr::defaults().getOwn("groupalign"),clone(sourceValues[4])));
expect(deep(m::MmlMtr::defaults().getOwn("data-break-align"),clone(sourceValues[17])));
}
{auto*n=create(f,"mlabeledtr");
expect(bool((n->kind())=="mlabeledtr"));
expect(deep(Property(n->arity()),clone(sourceValues[20])));
expect(bool((n->linebreakContainer())==true));
expect(bool((n->linebreakAlign())==""));
expect(deep(n->texClass(),clone(sourceValues[18])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[19])));
expect(deep(m::MmlMlabeledtr::defaults().getOwn("mathbackground"),clone(sourceValues[4])));
expect(deep(m::MmlMlabeledtr::defaults().getOwn("mathcolor"),clone(sourceValues[4])));
expect(deep(m::MmlMlabeledtr::defaults().getOwn("mathsize"),clone(sourceValues[4])));
expect(deep(m::MmlMlabeledtr::defaults().getOwn("dir"),clone(sourceValues[4])));
expect(deep(m::MmlMlabeledtr::defaults().getOwn("rowalign"),clone(sourceValues[4])));
expect(deep(m::MmlMlabeledtr::defaults().getOwn("columnalign"),clone(sourceValues[4])));
expect(deep(m::MmlMlabeledtr::defaults().getOwn("groupalign"),clone(sourceValues[4])));
expect(deep(m::MmlMlabeledtr::defaults().getOwn("data-break-align"),clone(sourceValues[17])));
}
{auto*n=create(f,"mtd");
expect(bool((n->kind())=="mtd"));
expect(deep(Property(n->arity()),clone(sourceValues[21])));
expect(bool((n->linebreakContainer())==true));
expect(bool((n->linebreakAlign())=="columnalign"));
expect(deep(n->texClass(),clone(sourceValues[18])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[19])));
expect(deep(m::MmlMtd::defaults().getOwn("mathbackground"),clone(sourceValues[4])));
expect(deep(m::MmlMtd::defaults().getOwn("mathcolor"),clone(sourceValues[4])));
expect(deep(m::MmlMtd::defaults().getOwn("mathsize"),clone(sourceValues[4])));
expect(deep(m::MmlMtd::defaults().getOwn("dir"),clone(sourceValues[4])));
expect(deep(m::MmlMtd::defaults().getOwn("rowspan"),clone(sourceValues[20])));
expect(deep(m::MmlMtd::defaults().getOwn("columnspan"),clone(sourceValues[20])));
expect(deep(m::MmlMtd::defaults().getOwn("rowalign"),clone(sourceValues[4])));
expect(deep(m::MmlMtd::defaults().getOwn("columnalign"),clone(sourceValues[4])));
expect(deep(m::MmlMtd::defaults().getOwn("groupalign"),clone(sourceValues[4])));
expect(deep(m::MmlMtd::defaults().getOwn("data-vertical-align"),clone(sourceValues[17])));
}
expect(bool((Property(m::MmlMtr::defaults())==Property(m::MmlMlabeledtr::defaults()))==true));
expect(bool((create(f,"mlabeledtr")->isKind("mtr"))==true));
enabled=false;};
"math_port_table_constructor_phase_mtable"_test=[] {reset();
NF f(bindings());m::MmlNode*n=nullptr;
traceLog.clear();enabled=true;
n=create(f,"probe-mtable",PropertyList{{"initial","value"}});
enabled=false;expect(traceLog==std::vector<std::string>{"phase/null","attributes"});
expect(deep(n->texClass(),clone(sourceValues[1])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[3])));
expect(bool((phaseBag==Property(n->getAllProperties()))==false));
enabled=false;};
"math_port_table_constructor_phase_mtr"_test=[] {reset();
NF f(bindings());m::MmlNode*n=nullptr;
traceLog.clear();enabled=true;
n=create(f,"probe-mtr",PropertyList{{"initial","value"}});
enabled=false;expect(traceLog==std::vector<std::string>{"phase/null","attributes"});
expect(deep(n->texClass(),clone(sourceValues[18])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[19])));
expect(bool((phaseBag==Property(n->getAllProperties()))==true));
enabled=false;};
"math_port_table_constructor_phase_mlabeledtr"_test=[] {reset();
NF f(bindings());m::MmlNode*n=nullptr;
traceLog.clear();enabled=true;
n=create(f,"probe-mlabeledtr",PropertyList{{"initial","value"}});
enabled=false;expect(traceLog==std::vector<std::string>{"phase/null","attributes"});
expect(deep(n->texClass(),clone(sourceValues[18])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[19])));
expect(bool((phaseBag==Property(n->getAllProperties()))==true));
enabled=false;};
"math_port_table_constructor_phase_mtd"_test=[] {reset();
NF f(bindings());m::MmlNode*n=nullptr;
traceLog.clear();enabled=true;
n=create(f,"probe-mtd",PropertyList{{"initial","value"}});
enabled=false;expect(traceLog==std::vector<std::string>{"phase/null","attributes"});
expect(deep(n->texClass(),clone(sourceValues[18])));
expect(deep(Property(n->getAllProperties()),clone(sourceValues[19])));
expect(bool((phaseBag==Property(n->getAllProperties()))==true));
enabled=false;};
"math_port_table_raw_inheritance_matrix"_test=[] {reset();
const std::vector<Property>pool={clone(sourceValues[22]),clone(sourceValues[30]),clone(sourceValues[14]),clone(sourceValues[2]),clone(sourceValues[31]),clone(sourceValues[21]),clone(sourceValues[1]),clone(sourceValues[32]),clone(sourceValues[19]),clone(sourceValues[37]),clone(sourceValues[42]),clone(sourceValues[44])};struct Row{std::string_view kind;std::array<unsigned,3>input;std::array<std::array<unsigned,5>,2>calls;std::array<bool,2>aliases;};const Row rows[]={
{"mtable",{0,0,0},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{0,0,2},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{0,0,3},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{0,0,4},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{0,5,0},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{0,5,2},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{0,5,3},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{0,5,4},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{0,6,0},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{0,6,2},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{0,6,3},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{0,6,4},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{0,7,0},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{0,7,2},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{0,7,3},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{0,7,4},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{0,8,0},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{0,8,2},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{0,8,3},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{0,8,4},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{2,0,0},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{2,0,2},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{2,0,3},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{2,0,4},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{2,5,0},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{2,5,2},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{2,5,3},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{2,5,4},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{2,6,0},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{2,6,2},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{2,6,3},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{2,6,4},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{2,7,0},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{2,7,2},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{2,7,3},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{2,7,4},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{2,8,0},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{2,8,2},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{2,8,3},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{2,8,4},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{3,0,0},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{3,0,2},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{3,0,3},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{3,0,4},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{3,5,0},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{3,5,2},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{3,5,3},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{3,5,4},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{3,6,0},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{3,6,2},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{3,6,3},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{3,6,4},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{3,7,0},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{3,7,2},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{3,7,3},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{3,7,4},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{3,8,0},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{3,8,2},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{3,8,3},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{3,8,4},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{4,0,0},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{4,0,2},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{4,0,3},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{4,0,4},{{{1,1,2,0,2},{1,1,2,0,2}}},{false,false}},
{"mtable",{4,5,0},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{4,5,2},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{4,5,3},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{4,5,4},{{{1,1,2,5,2},{1,1,2,5,2}}},{false,false}},
{"mtable",{4,6,0},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{4,6,2},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{4,6,3},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{4,6,4},{{{1,1,2,6,2},{1,1,2,6,2}}},{false,false}},
{"mtable",{4,7,0},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{4,7,2},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{4,7,3},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{4,7,4},{{{1,1,2,7,2},{1,1,2,7,2}}},{false,false}},
{"mtable",{4,8,0},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{4,8,2},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{4,8,3},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtable",{4,8,4},{{{1,1,2,8,2},{1,1,2,8,2}}},{false,false}},
{"mtr",{0,0,0},{{{9,9,0,0,0},{9,9,0,0,0}}},{false,false}},
{"mtr",{0,0,2},{{{9,9,0,0,2},{9,9,0,0,2}}},{false,false}},
{"mtr",{0,0,3},{{{9,9,0,0,3},{9,9,0,0,3}}},{false,false}},
{"mtr",{0,0,4},{{{9,9,0,0,4},{9,9,0,0,4}}},{false,false}},
{"mtr",{0,5,0},{{{9,9,0,5,0},{9,9,0,5,0}}},{false,false}},
{"mtr",{0,5,2},{{{9,9,0,5,2},{9,9,0,5,2}}},{false,false}},
{"mtr",{0,5,3},{{{9,9,0,5,3},{9,9,0,5,3}}},{false,false}},
{"mtr",{0,5,4},{{{9,9,0,5,4},{9,9,0,5,4}}},{false,false}},
{"mtr",{0,6,0},{{{9,9,0,6,0},{9,9,0,6,0}}},{false,false}},
{"mtr",{0,6,2},{{{9,9,0,6,2},{9,9,0,6,2}}},{false,false}},
{"mtr",{0,6,3},{{{9,9,0,6,3},{9,9,0,6,3}}},{false,false}},
{"mtr",{0,6,4},{{{9,9,0,6,4},{9,9,0,6,4}}},{false,false}},
{"mtr",{0,7,0},{{{9,9,0,7,0},{9,9,0,7,0}}},{false,false}},
{"mtr",{0,7,2},{{{9,9,0,7,2},{9,9,0,7,2}}},{false,false}},
{"mtr",{0,7,3},{{{9,9,0,7,3},{9,9,0,7,3}}},{false,false}},
{"mtr",{0,7,4},{{{9,9,0,7,4},{9,9,0,7,4}}},{false,false}},
{"mtr",{0,8,0},{{{9,9,0,8,0},{9,9,0,8,0}}},{false,false}},
{"mtr",{0,8,2},{{{9,9,0,8,2},{9,9,0,8,2}}},{false,false}},
{"mtr",{0,8,3},{{{9,9,0,8,3},{9,9,0,8,3}}},{false,false}},
{"mtr",{0,8,4},{{{9,9,0,8,4},{9,9,0,8,4}}},{false,false}},
{"mtr",{2,0,0},{{{9,9,2,0,0},{9,9,2,0,0}}},{false,false}},
{"mtr",{2,0,2},{{{9,9,2,0,2},{9,9,2,0,2}}},{false,false}},
{"mtr",{2,0,3},{{{9,9,2,0,3},{9,9,2,0,3}}},{false,false}},
{"mtr",{2,0,4},{{{9,9,2,0,4},{9,9,2,0,4}}},{false,false}},
{"mtr",{2,5,0},{{{9,9,2,5,0},{9,9,2,5,0}}},{false,false}},
{"mtr",{2,5,2},{{{9,9,2,5,2},{9,9,2,5,2}}},{false,false}},
{"mtr",{2,5,3},{{{9,9,2,5,3},{9,9,2,5,3}}},{false,false}},
{"mtr",{2,5,4},{{{9,9,2,5,4},{9,9,2,5,4}}},{false,false}},
{"mtr",{2,6,0},{{{9,9,2,6,0},{9,9,2,6,0}}},{false,false}},
{"mtr",{2,6,2},{{{9,9,2,6,2},{9,9,2,6,2}}},{false,false}},
{"mtr",{2,6,3},{{{9,9,2,6,3},{9,9,2,6,3}}},{false,false}},
{"mtr",{2,6,4},{{{9,9,2,6,4},{9,9,2,6,4}}},{false,false}},
{"mtr",{2,7,0},{{{9,9,2,7,0},{9,9,2,7,0}}},{false,false}},
{"mtr",{2,7,2},{{{9,9,2,7,2},{9,9,2,7,2}}},{false,false}},
{"mtr",{2,7,3},{{{9,9,2,7,3},{9,9,2,7,3}}},{false,false}},
{"mtr",{2,7,4},{{{9,9,2,7,4},{9,9,2,7,4}}},{false,false}},
{"mtr",{2,8,0},{{{9,9,2,8,0},{9,9,2,8,0}}},{false,false}},
{"mtr",{2,8,2},{{{9,9,2,8,2},{9,9,2,8,2}}},{false,false}},
{"mtr",{2,8,3},{{{9,9,2,8,3},{9,9,2,8,3}}},{false,false}},
{"mtr",{2,8,4},{{{9,9,2,8,4},{9,9,2,8,4}}},{false,false}},
{"mtr",{3,0,0},{{{9,9,3,0,0},{9,9,3,0,0}}},{false,false}},
{"mtr",{3,0,2},{{{9,9,3,0,2},{9,9,3,0,2}}},{false,false}},
{"mtr",{3,0,3},{{{9,9,3,0,3},{9,9,3,0,3}}},{false,false}},
{"mtr",{3,0,4},{{{9,9,3,0,4},{9,9,3,0,4}}},{false,false}},
{"mtr",{3,5,0},{{{9,9,3,5,0},{9,9,3,5,0}}},{false,false}},
{"mtr",{3,5,2},{{{9,9,3,5,2},{9,9,3,5,2}}},{false,false}},
{"mtr",{3,5,3},{{{9,9,3,5,3},{9,9,3,5,3}}},{false,false}},
{"mtr",{3,5,4},{{{9,9,3,5,4},{9,9,3,5,4}}},{false,false}},
{"mtr",{3,6,0},{{{9,9,3,6,0},{9,9,3,6,0}}},{false,false}},
{"mtr",{3,6,2},{{{9,9,3,6,2},{9,9,3,6,2}}},{false,false}},
{"mtr",{3,6,3},{{{9,9,3,6,3},{9,9,3,6,3}}},{false,false}},
{"mtr",{3,6,4},{{{9,9,3,6,4},{9,9,3,6,4}}},{false,false}},
{"mtr",{3,7,0},{{{9,9,3,7,0},{9,9,3,7,0}}},{false,false}},
{"mtr",{3,7,2},{{{9,9,3,7,2},{9,9,3,7,2}}},{false,false}},
{"mtr",{3,7,3},{{{9,9,3,7,3},{9,9,3,7,3}}},{false,false}},
{"mtr",{3,7,4},{{{9,9,3,7,4},{9,9,3,7,4}}},{false,false}},
{"mtr",{3,8,0},{{{9,9,3,8,0},{9,9,3,8,0}}},{false,false}},
{"mtr",{3,8,2},{{{9,9,3,8,2},{9,9,3,8,2}}},{false,false}},
{"mtr",{3,8,3},{{{9,9,3,8,3},{9,9,3,8,3}}},{false,false}},
{"mtr",{3,8,4},{{{9,9,3,8,4},{9,9,3,8,4}}},{false,false}},
{"mtr",{4,0,0},{{{9,9,4,0,0},{9,9,4,0,0}}},{false,false}},
{"mtr",{4,0,2},{{{9,9,4,0,2},{9,9,4,0,2}}},{false,false}},
{"mtr",{4,0,3},{{{9,9,4,0,3},{9,9,4,0,3}}},{false,false}},
{"mtr",{4,0,4},{{{9,9,4,0,4},{9,9,4,0,4}}},{false,false}},
{"mtr",{4,5,0},{{{9,9,4,5,0},{9,9,4,5,0}}},{false,false}},
{"mtr",{4,5,2},{{{9,9,4,5,2},{9,9,4,5,2}}},{false,false}},
{"mtr",{4,5,3},{{{9,9,4,5,3},{9,9,4,5,3}}},{false,false}},
{"mtr",{4,5,4},{{{9,9,4,5,4},{9,9,4,5,4}}},{false,false}},
{"mtr",{4,6,0},{{{9,9,4,6,0},{9,9,4,6,0}}},{false,false}},
{"mtr",{4,6,2},{{{9,9,4,6,2},{9,9,4,6,2}}},{false,false}},
{"mtr",{4,6,3},{{{9,9,4,6,3},{9,9,4,6,3}}},{false,false}},
{"mtr",{4,6,4},{{{9,9,4,6,4},{9,9,4,6,4}}},{false,false}},
{"mtr",{4,7,0},{{{9,9,4,7,0},{9,9,4,7,0}}},{false,false}},
{"mtr",{4,7,2},{{{9,9,4,7,2},{9,9,4,7,2}}},{false,false}},
{"mtr",{4,7,3},{{{9,9,4,7,3},{9,9,4,7,3}}},{false,false}},
{"mtr",{4,7,4},{{{9,9,4,7,4},{9,9,4,7,4}}},{false,false}},
{"mtr",{4,8,0},{{{9,9,4,8,0},{9,9,4,8,0}}},{false,false}},
{"mtr",{4,8,2},{{{9,9,4,8,2},{9,9,4,8,2}}},{false,false}},
{"mtr",{4,8,3},{{{9,9,4,8,3},{9,9,4,8,3}}},{false,false}},
{"mtr",{4,8,4},{{{9,9,4,8,4},{9,9,4,8,4}}},{false,false}},
{"mlabeledtr",{0,0,0},{{{10,11,0,0,0},{11,11,0,0,0}}},{false,false}},
{"mlabeledtr",{0,0,2},{{{10,11,0,0,2},{11,11,0,0,2}}},{false,false}},
{"mlabeledtr",{0,0,3},{{{10,11,0,0,3},{11,11,0,0,3}}},{false,false}},
{"mlabeledtr",{0,0,4},{{{10,11,0,0,4},{11,11,0,0,4}}},{false,false}},
{"mlabeledtr",{0,5,0},{{{10,11,0,5,0},{11,11,0,5,0}}},{false,false}},
{"mlabeledtr",{0,5,2},{{{10,11,0,5,2},{11,11,0,5,2}}},{false,false}},
{"mlabeledtr",{0,5,3},{{{10,11,0,5,3},{11,11,0,5,3}}},{false,false}},
{"mlabeledtr",{0,5,4},{{{10,11,0,5,4},{11,11,0,5,4}}},{false,false}},
{"mlabeledtr",{0,6,0},{{{10,11,0,6,0},{11,11,0,6,0}}},{false,false}},
{"mlabeledtr",{0,6,2},{{{10,11,0,6,2},{11,11,0,6,2}}},{false,false}},
{"mlabeledtr",{0,6,3},{{{10,11,0,6,3},{11,11,0,6,3}}},{false,false}},
{"mlabeledtr",{0,6,4},{{{10,11,0,6,4},{11,11,0,6,4}}},{false,false}},
{"mlabeledtr",{0,7,0},{{{10,11,0,7,0},{11,11,0,7,0}}},{false,false}},
{"mlabeledtr",{0,7,2},{{{10,11,0,7,2},{11,11,0,7,2}}},{false,false}},
{"mlabeledtr",{0,7,3},{{{10,11,0,7,3},{11,11,0,7,3}}},{false,false}},
{"mlabeledtr",{0,7,4},{{{10,11,0,7,4},{11,11,0,7,4}}},{false,false}},
{"mlabeledtr",{0,8,0},{{{10,11,0,8,0},{11,11,0,8,0}}},{false,false}},
{"mlabeledtr",{0,8,2},{{{10,11,0,8,2},{11,11,0,8,2}}},{false,false}},
{"mlabeledtr",{0,8,3},{{{10,11,0,8,3},{11,11,0,8,3}}},{false,false}},
{"mlabeledtr",{0,8,4},{{{10,11,0,8,4},{11,11,0,8,4}}},{false,false}},
{"mlabeledtr",{2,0,0},{{{10,11,2,0,0},{11,11,2,0,0}}},{false,false}},
{"mlabeledtr",{2,0,2},{{{10,11,2,0,2},{11,11,2,0,2}}},{false,false}},
{"mlabeledtr",{2,0,3},{{{10,11,2,0,3},{11,11,2,0,3}}},{false,false}},
{"mlabeledtr",{2,0,4},{{{10,11,2,0,4},{11,11,2,0,4}}},{false,false}},
{"mlabeledtr",{2,5,0},{{{10,11,2,5,0},{11,11,2,5,0}}},{false,false}},
{"mlabeledtr",{2,5,2},{{{10,11,2,5,2},{11,11,2,5,2}}},{false,false}},
{"mlabeledtr",{2,5,3},{{{10,11,2,5,3},{11,11,2,5,3}}},{false,false}},
{"mlabeledtr",{2,5,4},{{{10,11,2,5,4},{11,11,2,5,4}}},{false,false}},
{"mlabeledtr",{2,6,0},{{{10,11,2,6,0},{11,11,2,6,0}}},{false,false}},
{"mlabeledtr",{2,6,2},{{{10,11,2,6,2},{11,11,2,6,2}}},{false,false}},
{"mlabeledtr",{2,6,3},{{{10,11,2,6,3},{11,11,2,6,3}}},{false,false}},
{"mlabeledtr",{2,6,4},{{{10,11,2,6,4},{11,11,2,6,4}}},{false,false}},
{"mlabeledtr",{2,7,0},{{{10,11,2,7,0},{11,11,2,7,0}}},{false,false}},
{"mlabeledtr",{2,7,2},{{{10,11,2,7,2},{11,11,2,7,2}}},{false,false}},
{"mlabeledtr",{2,7,3},{{{10,11,2,7,3},{11,11,2,7,3}}},{false,false}},
{"mlabeledtr",{2,7,4},{{{10,11,2,7,4},{11,11,2,7,4}}},{false,false}},
{"mlabeledtr",{2,8,0},{{{10,11,2,8,0},{11,11,2,8,0}}},{false,false}},
{"mlabeledtr",{2,8,2},{{{10,11,2,8,2},{11,11,2,8,2}}},{false,false}},
{"mlabeledtr",{2,8,3},{{{10,11,2,8,3},{11,11,2,8,3}}},{false,false}},
{"mlabeledtr",{2,8,4},{{{10,11,2,8,4},{11,11,2,8,4}}},{false,false}},
{"mlabeledtr",{3,0,0},{{{10,11,3,0,0},{11,11,3,0,0}}},{false,false}},
{"mlabeledtr",{3,0,2},{{{10,11,3,0,2},{11,11,3,0,2}}},{false,false}},
{"mlabeledtr",{3,0,3},{{{10,11,3,0,3},{11,11,3,0,3}}},{false,false}},
{"mlabeledtr",{3,0,4},{{{10,11,3,0,4},{11,11,3,0,4}}},{false,false}},
{"mlabeledtr",{3,5,0},{{{10,11,3,5,0},{11,11,3,5,0}}},{false,false}},
{"mlabeledtr",{3,5,2},{{{10,11,3,5,2},{11,11,3,5,2}}},{false,false}},
{"mlabeledtr",{3,5,3},{{{10,11,3,5,3},{11,11,3,5,3}}},{false,false}},
{"mlabeledtr",{3,5,4},{{{10,11,3,5,4},{11,11,3,5,4}}},{false,false}},
{"mlabeledtr",{3,6,0},{{{10,11,3,6,0},{11,11,3,6,0}}},{false,false}},
{"mlabeledtr",{3,6,2},{{{10,11,3,6,2},{11,11,3,6,2}}},{false,false}},
{"mlabeledtr",{3,6,3},{{{10,11,3,6,3},{11,11,3,6,3}}},{false,false}},
{"mlabeledtr",{3,6,4},{{{10,11,3,6,4},{11,11,3,6,4}}},{false,false}},
{"mlabeledtr",{3,7,0},{{{10,11,3,7,0},{11,11,3,7,0}}},{false,false}},
{"mlabeledtr",{3,7,2},{{{10,11,3,7,2},{11,11,3,7,2}}},{false,false}},
{"mlabeledtr",{3,7,3},{{{10,11,3,7,3},{11,11,3,7,3}}},{false,false}},
{"mlabeledtr",{3,7,4},{{{10,11,3,7,4},{11,11,3,7,4}}},{false,false}},
{"mlabeledtr",{3,8,0},{{{10,11,3,8,0},{11,11,3,8,0}}},{false,false}},
{"mlabeledtr",{3,8,2},{{{10,11,3,8,2},{11,11,3,8,2}}},{false,false}},
{"mlabeledtr",{3,8,3},{{{10,11,3,8,3},{11,11,3,8,3}}},{false,false}},
{"mlabeledtr",{3,8,4},{{{10,11,3,8,4},{11,11,3,8,4}}},{false,false}},
{"mlabeledtr",{4,0,0},{{{10,11,4,0,0},{11,11,4,0,0}}},{false,false}},
{"mlabeledtr",{4,0,2},{{{10,11,4,0,2},{11,11,4,0,2}}},{false,false}},
{"mlabeledtr",{4,0,3},{{{10,11,4,0,3},{11,11,4,0,3}}},{false,false}},
{"mlabeledtr",{4,0,4},{{{10,11,4,0,4},{11,11,4,0,4}}},{false,false}},
{"mlabeledtr",{4,5,0},{{{10,11,4,5,0},{11,11,4,5,0}}},{false,false}},
{"mlabeledtr",{4,5,2},{{{10,11,4,5,2},{11,11,4,5,2}}},{false,false}},
{"mlabeledtr",{4,5,3},{{{10,11,4,5,3},{11,11,4,5,3}}},{false,false}},
{"mlabeledtr",{4,5,4},{{{10,11,4,5,4},{11,11,4,5,4}}},{false,false}},
{"mlabeledtr",{4,6,0},{{{10,11,4,6,0},{11,11,4,6,0}}},{false,false}},
{"mlabeledtr",{4,6,2},{{{10,11,4,6,2},{11,11,4,6,2}}},{false,false}},
{"mlabeledtr",{4,6,3},{{{10,11,4,6,3},{11,11,4,6,3}}},{false,false}},
{"mlabeledtr",{4,6,4},{{{10,11,4,6,4},{11,11,4,6,4}}},{false,false}},
{"mlabeledtr",{4,7,0},{{{10,11,4,7,0},{11,11,4,7,0}}},{false,false}},
{"mlabeledtr",{4,7,2},{{{10,11,4,7,2},{11,11,4,7,2}}},{false,false}},
{"mlabeledtr",{4,7,3},{{{10,11,4,7,3},{11,11,4,7,3}}},{false,false}},
{"mlabeledtr",{4,7,4},{{{10,11,4,7,4},{11,11,4,7,4}}},{false,false}},
{"mlabeledtr",{4,8,0},{{{10,11,4,8,0},{11,11,4,8,0}}},{false,false}},
{"mlabeledtr",{4,8,2},{{{10,11,4,8,2},{11,11,4,8,2}}},{false,false}},
{"mlabeledtr",{4,8,3},{{{10,11,4,8,3},{11,11,4,8,3}}},{false,false}},
{"mlabeledtr",{4,8,4},{{{10,11,4,8,4},{11,11,4,8,4}}},{false,false}},
};for(const auto&r:rows){Setup s(r.kind);PropertyList attrs{{"extra",PropertyArray{"test",5}}};hook(s.n,attrs,clone(pool[r.input[0]]),clone(pool[r.input[1]]),clone(pool[r.input[2]]));expect(calls.size()==2);for(std::size_t i=0;i<calls.size()&&i<2;++i){expect(deep(calls[i].snapshot,pool[r.calls[i][0]]));expect(deep(Property(calls[i].a),pool[r.calls[i][1]]));expect(deep(calls[i].d,pool[r.calls[i][2]]));expect(deep(calls[i].l,pool[r.calls[i][3]]));expect(deep(calls[i].p,pool[r.calls[i][4]]));expect(bool((Property(calls[i].a)==Property(attrs))==r.aliases[i]));}}
enabled=false;};
"math_port_table_alignment_alias_mtable_normal"_test=[] {reset();
Setup s("mtable");PropertyList attrs{{"extra",PropertyArray{"test",5}}};s.n->attributes()->set("rowalign","top bottom");s.n->attributes()->set("data-break-align","bottom center");mode="";target=s.n;late=s.c;
traceLog.clear();enabled=true;
hook(s.n,attrs,"raw","2","raw");
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[47])));
expect(deep(Property(calls[0].a),clone(sourceValues[50])));
expect(deep(calls[0].d,clone(sourceValues[14])));
expect(deep(calls[0].l,clone(sourceValues[32])));
expect(deep(calls[0].p,clone(sourceValues[14])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[50])));
expect(deep(Property(calls[1].a),clone(sourceValues[50])));
expect(deep(calls[1].d,clone(sourceValues[14])));
expect(deep(calls[1].l,clone(sourceValues[32])));
expect(deep(calls[1].p,clone(sourceValues[14])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
expect(deep(Property(attrs),clone(sourceValues[51])));
expect(bool((calls.size()>0&&calls[0].node==s.c)==false));
expect(bool((calls.size()>1&&calls[1].node==s.c)==false));
enabled=false;};
"math_port_table_alignment_alias_mtable_tuple"_test=[] {reset();
Setup s("mtable");PropertyList attrs{{"extra",PropertyArray{"test",5}}};s.n->attributes()->set("rowalign","top bottom");s.n->attributes()->set("data-break-align","bottom center");mode="tuple";target=s.n;late=s.c;
traceLog.clear();enabled=true;
hook(s.n,attrs,"raw","2","raw");
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[47])));
expect(deep(Property(calls[0].a),clone(sourceValues[54])));
expect(deep(calls[0].d,clone(sourceValues[14])));
expect(deep(calls[0].l,clone(sourceValues[32])));
expect(deep(calls[0].p,clone(sourceValues[14])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[54])));
expect(deep(Property(calls[1].a),clone(sourceValues[54])));
expect(deep(calls[1].d,clone(sourceValues[14])));
expect(deep(calls[1].l,clone(sourceValues[32])));
expect(deep(calls[1].p,clone(sourceValues[14])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
expect(deep(Property(attrs),clone(sourceValues[51])));
expect(bool((calls.size()>0&&calls[0].node==s.c)==false));
expect(bool((calls.size()>1&&calls[1].node==s.c)==false));
enabled=false;};
"math_port_table_alignment_alias_mtable_swap"_test=[] {reset();
Setup s("mtable");PropertyList attrs{{"extra",PropertyArray{"test",5}}};s.n->attributes()->set("rowalign","top bottom");s.n->attributes()->set("data-break-align","bottom center");mode="swap";target=s.n;late=s.c;
traceLog.clear();enabled=true;
hook(s.n,attrs,"raw","2","raw");
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[47])));
expect(deep(Property(calls[0].a),clone(sourceValues[50])));
expect(deep(calls[0].d,clone(sourceValues[14])));
expect(deep(calls[0].l,clone(sourceValues[32])));
expect(deep(calls[0].p,clone(sourceValues[14])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[50])));
expect(deep(Property(calls[1].a),clone(sourceValues[50])));
expect(deep(calls[1].d,clone(sourceValues[14])));
expect(deep(calls[1].l,clone(sourceValues[32])));
expect(deep(calls[1].p,clone(sourceValues[14])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
expect(deep(Property(attrs),clone(sourceValues[51])));
expect(bool((calls.size()>0&&calls[0].node==s.c)==false));
expect(bool((calls.size()>1&&calls[1].node==s.c)==false));
enabled=false;};
"math_port_table_alignment_alias_mtr_normal"_test=[] {reset();
Setup s("mtr");PropertyList attrs{{"extra",PropertyArray{"test",5}}};s.n->attributes()->set("columnalign","left right");s.n->attributes()->set("data-break-align","bottom center");mode="";target=s.n;late=s.c;
traceLog.clear();enabled=true;
hook(s.n,attrs,"raw","2","raw");
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[58])));
expect(deep(Property(calls[0].a),clone(sourceValues[60])));
expect(deep(calls[0].d,clone(sourceValues[31])));
expect(deep(calls[0].l,clone(sourceValues[32])));
expect(deep(calls[0].p,clone(sourceValues[31])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[60])));
expect(deep(Property(calls[1].a),clone(sourceValues[60])));
expect(deep(calls[1].d,clone(sourceValues[31])));
expect(deep(calls[1].l,clone(sourceValues[32])));
expect(deep(calls[1].p,clone(sourceValues[31])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
expect(deep(Property(attrs),clone(sourceValues[51])));
expect(bool((calls.size()>0&&calls[0].node==s.c)==false));
expect(bool((calls.size()>1&&calls[1].node==s.c)==false));
enabled=false;};
"math_port_table_alignment_alias_mtr_tuple"_test=[] {reset();
Setup s("mtr");PropertyList attrs{{"extra",PropertyArray{"test",5}}};s.n->attributes()->set("columnalign","left right");s.n->attributes()->set("data-break-align","bottom center");mode="tuple";target=s.n;late=s.c;
traceLog.clear();enabled=true;
hook(s.n,attrs,"raw","2","raw");
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[58])));
expect(deep(Property(calls[0].a),clone(sourceValues[62])));
expect(deep(calls[0].d,clone(sourceValues[31])));
expect(deep(calls[0].l,clone(sourceValues[32])));
expect(deep(calls[0].p,clone(sourceValues[31])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[62])));
expect(deep(Property(calls[1].a),clone(sourceValues[62])));
expect(deep(calls[1].d,clone(sourceValues[31])));
expect(deep(calls[1].l,clone(sourceValues[32])));
expect(deep(calls[1].p,clone(sourceValues[31])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
expect(deep(Property(attrs),clone(sourceValues[51])));
expect(bool((calls.size()>0&&calls[0].node==s.c)==false));
expect(bool((calls.size()>1&&calls[1].node==s.c)==false));
enabled=false;};
"math_port_table_alignment_alias_mtr_swap"_test=[] {reset();
Setup s("mtr");PropertyList attrs{{"extra",PropertyArray{"test",5}}};s.n->attributes()->set("columnalign","left right");s.n->attributes()->set("data-break-align","bottom center");mode="swap";target=s.n;late=s.c;
traceLog.clear();enabled=true;
hook(s.n,attrs,"raw","2","raw");
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[58])));
expect(deep(Property(calls[0].a),clone(sourceValues[60])));
expect(deep(calls[0].d,clone(sourceValues[31])));
expect(deep(calls[0].l,clone(sourceValues[32])));
expect(deep(calls[0].p,clone(sourceValues[31])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[60])));
expect(deep(Property(calls[1].a),clone(sourceValues[60])));
expect(deep(calls[1].d,clone(sourceValues[31])));
expect(deep(calls[1].l,clone(sourceValues[32])));
expect(deep(calls[1].p,clone(sourceValues[31])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
expect(deep(Property(attrs),clone(sourceValues[51])));
expect(bool((calls.size()>0&&calls[0].node==s.c)==false));
expect(bool((calls.size()>1&&calls[1].node==s.c)==false));
enabled=false;};
"math_port_table_alignment_alias_mlabeledtr_normal"_test=[] {reset();
Setup s("mlabeledtr");PropertyList attrs{{"extra",PropertyArray{"test",5}}};s.n->attributes()->set("columnalign","left right");s.n->attributes()->set("data-break-align","bottom center");s.n->parent->attributes()->set("side","left");mode="";target=s.n;late=s.c;
traceLog.clear();enabled=true;
hook(s.n,attrs,"raw","2","raw");
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[64])));
expect(deep(Property(calls[0].a),clone(sourceValues[66])));
expect(deep(calls[0].d,clone(sourceValues[31])));
expect(deep(calls[0].l,clone(sourceValues[32])));
expect(deep(calls[0].p,clone(sourceValues[31])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[66])));
expect(deep(Property(calls[1].a),clone(sourceValues[66])));
expect(deep(calls[1].d,clone(sourceValues[31])));
expect(deep(calls[1].l,clone(sourceValues[32])));
expect(deep(calls[1].p,clone(sourceValues[31])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
expect(deep(Property(attrs),clone(sourceValues[51])));
expect(bool((calls.size()>0&&calls[0].node==s.c)==false));
expect(bool((calls.size()>1&&calls[1].node==s.c)==false));
enabled=false;};
"math_port_table_alignment_alias_mlabeledtr_tuple"_test=[] {reset();
Setup s("mlabeledtr");PropertyList attrs{{"extra",PropertyArray{"test",5}}};s.n->attributes()->set("columnalign","left right");s.n->attributes()->set("data-break-align","bottom center");s.n->parent->attributes()->set("side","left");mode="tuple";target=s.n;late=s.c;
traceLog.clear();enabled=true;
hook(s.n,attrs,"raw","2","raw");
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[64])));
expect(deep(Property(calls[0].a),clone(sourceValues[68])));
expect(deep(calls[0].d,clone(sourceValues[31])));
expect(deep(calls[0].l,clone(sourceValues[32])));
expect(deep(calls[0].p,clone(sourceValues[31])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[68])));
expect(deep(Property(calls[1].a),clone(sourceValues[68])));
expect(deep(calls[1].d,clone(sourceValues[31])));
expect(deep(calls[1].l,clone(sourceValues[32])));
expect(deep(calls[1].p,clone(sourceValues[31])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
expect(deep(Property(attrs),clone(sourceValues[51])));
expect(bool((calls.size()>0&&calls[0].node==s.c)==false));
expect(bool((calls.size()>1&&calls[1].node==s.c)==false));
enabled=false;};
"math_port_table_alignment_alias_mlabeledtr_swap"_test=[] {reset();
Setup s("mlabeledtr");PropertyList attrs{{"extra",PropertyArray{"test",5}}};s.n->attributes()->set("columnalign","left right");s.n->attributes()->set("data-break-align","bottom center");s.n->parent->attributes()->set("side","left");mode="swap";target=s.n;late=s.c;
traceLog.clear();enabled=true;
hook(s.n,attrs,"raw","2","raw");
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[64])));
expect(deep(Property(calls[0].a),clone(sourceValues[66])));
expect(deep(calls[0].d,clone(sourceValues[31])));
expect(deep(calls[0].l,clone(sourceValues[32])));
expect(deep(calls[0].p,clone(sourceValues[31])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[66])));
expect(deep(Property(calls[1].a),clone(sourceValues[66])));
expect(deep(calls[1].d,clone(sourceValues[31])));
expect(deep(calls[1].l,clone(sourceValues[32])));
expect(deep(calls[1].p,clone(sourceValues[31])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
expect(deep(Property(attrs),clone(sourceValues[51])));
expect(bool((calls.size()>0&&calls[0].node==s.c)==false));
expect(bool((calls.size()>1&&calls[1].node==s.c)==false));
enabled=false;};
"math_port_table_split_raw_mtable_15"_test=[] {reset();
Setup s("mtable");PropertyList attrs;s.n->attributes()->set("rowalign",clone(sourceValues[18]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtable_16"_test=[] {reset();
Setup s("mtable");PropertyList attrs;s.n->attributes()->set("rowalign",clone(sourceValues[14]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtable_17"_test=[] {reset();
Setup s("mtable");PropertyList attrs;s.n->attributes()->set("rowalign",clone(sourceValues[69]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtable_18"_test=[] {reset();
Setup s("mtable");PropertyList attrs;s.n->attributes()->set("rowalign",clone(sourceValues[19]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtable_19"_test=[] {reset();
Setup s("mtable");PropertyList attrs;s.n->attributes()->set("rowalign",clone(sourceValues[70]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtable_20"_test=[] {reset();
Setup s("mtable");PropertyList attrs;s.n->attributes()->set("rowalign",clone(sourceValues[71]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtable_21"_test=[] {reset();
Setup s("mtable");PropertyList attrs;s.n->attributes()->set("rowalign",clone(sourceValues[72]));
traceLog.clear();enabled=true;
hook(s.n,attrs,false,0,false);
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[73])));
expect(deep(Property(calls[0].a),clone(sourceValues[73])));
expect(deep(calls[0].d,clone(sourceValues[14])));
expect(deep(calls[0].l,clone(sourceValues[1])));
expect(deep(calls[0].p,clone(sourceValues[14])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[73])));
expect(deep(Property(calls[1].a),clone(sourceValues[73])));
expect(deep(calls[1].d,clone(sourceValues[14])));
expect(deep(calls[1].l,clone(sourceValues[1])));
expect(deep(calls[1].p,clone(sourceValues[14])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
enabled=false;};
"math_port_table_split_raw_mtable_22"_test=[] {reset();
Setup s("mtable");PropertyList attrs;s.n->attributes()->set("rowalign",clone(sourceValues[74]));
traceLog.clear();enabled=true;
hook(s.n,attrs,false,0,false);
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[76])));
expect(deep(Property(calls[0].a),clone(sourceValues[78])));
expect(deep(calls[0].d,clone(sourceValues[14])));
expect(deep(calls[0].l,clone(sourceValues[1])));
expect(deep(calls[0].p,clone(sourceValues[14])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[78])));
expect(deep(Property(calls[1].a),clone(sourceValues[78])));
expect(deep(calls[1].d,clone(sourceValues[14])));
expect(deep(calls[1].l,clone(sourceValues[1])));
expect(deep(calls[1].p,clone(sourceValues[14])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
enabled=false;};
"math_port_table_split_raw_mtr_23"_test=[] {reset();
Setup s("mtr");PropertyList attrs;s.n->attributes()->set("columnalign",clone(sourceValues[18]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtr_24"_test=[] {reset();
Setup s("mtr");PropertyList attrs;s.n->attributes()->set("columnalign",clone(sourceValues[14]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtr_25"_test=[] {reset();
Setup s("mtr");PropertyList attrs;s.n->attributes()->set("columnalign",clone(sourceValues[69]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtr_26"_test=[] {reset();
Setup s("mtr");PropertyList attrs;s.n->attributes()->set("columnalign",clone(sourceValues[19]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtr_27"_test=[] {reset();
Setup s("mtr");PropertyList attrs;s.n->attributes()->set("columnalign",clone(sourceValues[70]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtr_28"_test=[] {reset();
Setup s("mtr");PropertyList attrs;s.n->attributes()->set("columnalign",clone(sourceValues[71]));
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes"});
expect(bool((calls.size())==0));
enabled=false;};
"math_port_table_split_raw_mtr_29"_test=[] {reset();
Setup s("mtr");PropertyList attrs;s.n->attributes()->set("columnalign",clone(sourceValues[72]));
traceLog.clear();enabled=true;
hook(s.n,attrs,false,0,false);
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[79])));
expect(deep(Property(calls[0].a),clone(sourceValues[79])));
expect(deep(calls[0].d,clone(sourceValues[14])));
expect(deep(calls[0].l,clone(sourceValues[1])));
expect(deep(calls[0].p,clone(sourceValues[14])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[79])));
expect(deep(Property(calls[1].a),clone(sourceValues[79])));
expect(deep(calls[1].d,clone(sourceValues[14])));
expect(deep(calls[1].l,clone(sourceValues[1])));
expect(deep(calls[1].p,clone(sourceValues[14])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
enabled=false;};
"math_port_table_split_raw_mtr_30"_test=[] {reset();
Setup s("mtr");PropertyList attrs;s.n->attributes()->set("columnalign",clone(sourceValues[74]));
traceLog.clear();enabled=true;
hook(s.n,attrs,false,0,false);
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","inherit/first","inherit/second"});
expect(bool((calls.size())==2));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[80])));
expect(deep(Property(calls[0].a),clone(sourceValues[81])));
expect(deep(calls[0].d,clone(sourceValues[14])));
expect(deep(calls[0].l,clone(sourceValues[1])));
expect(deep(calls[0].p,clone(sourceValues[14])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[81])));
expect(deep(Property(calls[1].a),clone(sourceValues[81])));
expect(deep(calls[1].d,clone(sourceValues[14])));
expect(deep(calls[1].l,clone(sourceValues[1])));
expect(deep(calls[1].p,clone(sourceValues[14])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
enabled=false;};
"math_port_table_tex_iteration_mtable_normal"_test=[] {reset();
Setup s("mtable");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null","tex/second/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==2));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
expect(bool((texCalls.size()>1&&texCalls[1]==s.c)==false));
enabled=false;};
"math_port_table_tex_iteration_mtable_swap"_test=[] {reset();
Setup s("mtable");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="swap";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null","tex/second/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==2));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
expect(bool((texCalls.size()>1&&texCalls[1]==s.c)==false));
enabled=false;};
"math_port_table_tex_iteration_mtable_grow-rebind"_test=[] {reset();
Setup s("mtable");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="grow-rebind";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null","tex/second/null","tex/late/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==3));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
expect(bool((texCalls.size()>1&&texCalls[1]==s.c)==false));
expect(bool((texCalls.size()>2&&texCalls[2]==s.c)==true));
enabled=false;};
"math_port_table_tex_iteration_mtr_normal"_test=[] {reset();
Setup s("mtr");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null","tex/second/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==2));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
expect(bool((texCalls.size()>1&&texCalls[1]==s.c)==false));
enabled=false;};
"math_port_table_tex_iteration_mtr_swap"_test=[] {reset();
Setup s("mtr");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="swap";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null","tex/second/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==2));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
expect(bool((texCalls.size()>1&&texCalls[1]==s.c)==false));
enabled=false;};
"math_port_table_tex_iteration_mtr_grow-rebind"_test=[] {reset();
Setup s("mtr");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="grow-rebind";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null","tex/second/null","tex/late/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==3));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
expect(bool((texCalls.size()>1&&texCalls[1]==s.c)==false));
expect(bool((texCalls.size()>2&&texCalls[2]==s.c)==true));
enabled=false;};
"math_port_table_tex_iteration_mlabeledtr_normal"_test=[] {reset();
Setup s("mlabeledtr");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null","tex/second/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==2));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
expect(bool((texCalls.size()>1&&texCalls[1]==s.c)==false));
enabled=false;};
"math_port_table_tex_iteration_mlabeledtr_swap"_test=[] {reset();
Setup s("mlabeledtr");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="swap";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null","tex/second/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==2));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
expect(bool((texCalls.size()>1&&texCalls[1]==s.c)==false));
enabled=false;};
"math_port_table_tex_iteration_mlabeledtr_grow-rebind"_test=[] {reset();
Setup s("mlabeledtr");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="grow-rebind";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null","tex/second/null","tex/late/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==3));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
expect(bool((texCalls.size()>1&&texCalls[1]==s.c)==false));
expect(bool((texCalls.size()>2&&texCalls[2]==s.c)==true));
enabled=false;};
"math_port_table_tex_iteration_mtd_normal"_test=[] {reset();
Setup s("mtd");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==1));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
enabled=false;};
"math_port_table_tex_iteration_mtd_swap"_test=[] {reset();
Setup s("mtd");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="swap";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==1));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
enabled=false;};
"math_port_table_tex_iteration_mtd_grow-rebind"_test=[] {reset();
Setup s("mtd");auto*prev=create(s.f,"mrow");prev->setTexClass("raw");prev->attributes()->set("scriptlevel","2");mode="grow-rebind";target=s.n;late=s.c;originalChildren=s.n->childNodes;m::MmlNode*result=nullptr;
traceLog.clear();enabled=true;
result=s.n->setTeXclass(prev);
enabled=false;expect(traceLog==std::vector<std::string>{"previous","tex/first/null"});
expect(bool((result==s.n)==true));
expect(deep(s.n->prevClass(),clone(sourceValues[31])));
expect(deep(s.n->prevLevel(),clone(sourceValues[32])));
expect(bool((texCalls.size())==1));
expect(bool((texCalls.size()>0&&texCalls[0]==s.c)==false));
enabled=false;};
"math_port_table_table_verify_cells_43"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[82])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[107])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[22])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_mixed_44"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[82])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[127])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_interleaved_45"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,row,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[82])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[140])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_cells_46"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[141])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[158])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[22])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_mixed_47"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[141])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[164])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_interleaved_48"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,row,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[141])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[170])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_cells_49"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[107])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[22])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_mixed_50"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[127])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_interleaved_51"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,row,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[140])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_cells_52"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[172])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[158])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[22])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_mixed_53"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[172])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[164])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_interleaved_54"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,row,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[172])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[170])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[26])));
enabled=false;};
"math_port_table_table_verify_cells_55"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[179])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[22])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_mixed_56"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[188])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_interleaved_57"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,row,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[194])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_cells_58"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[195])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[179])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[22])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_mixed_59"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[195])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[188])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_interleaved_60"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,row,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[195])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[194])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_cells_61"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[196])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[179])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[22])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_mixed_62"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[196])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[188])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_interleaved_63"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,row,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[196])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[194])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_cells_64"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[197])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[179])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[22])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_mixed_65"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[197])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[188])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_table_verify_interleaved_66"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable");auto*a=create(f,"mtd");auto*b=token(f,u"x");auto*c=create(f,"mtd");auto*row=create(f,"mtr",{},children({create(f,"mtd")}));n->setChildren(children({a,row,b,c}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[197])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[194])));
expect(deep((a->parent?Property(a->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
expect(deep((b->parent?Property(b->parent->kind()):Property(Missing{})),clone(sourceValues[85])));
expect(deep((c->parent?Property(c->parent->kind()):Property(Missing{})),clone(sourceValues[33])));
enabled=false;};
"math_port_table_parent_verify_mtr_67"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtr",{},children({token(f,u"x")}));auto*holder=n;PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[206])));
enabled=false;};
"math_port_table_parent_verify_mtr_68"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtr",{},children({token(f,u"x")}));auto*holder=n;PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[208])));
enabled=false;};
"math_port_table_parent_verify_mtr_69"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtr",{},children({token(f,u"x")}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[219])));
enabled=false;};
"math_port_table_parent_verify_mtr_70"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtr",{},children({token(f,u"x")}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[219])));
enabled=false;};
"math_port_table_parent_verify_mtr_71"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtr",{},children({token(f,u"x")}));auto*holder=create(f,"mtable",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[222])));
enabled=false;};
"math_port_table_parent_verify_mtr_72"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtr",{},children({token(f,u"x")}));auto*holder=create(f,"mtable",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[225])));
enabled=false;};
"math_port_table_parent_verify_mtr_73"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtr",{},children({token(f,u"x")}));auto*holder=create(f,"mtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[228])));
enabled=false;};
"math_port_table_parent_verify_mtr_74"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtr",{},children({token(f,u"x")}));auto*holder=create(f,"mtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[228])));
enabled=false;};
"math_port_table_parent_verify_mtr_75"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtr",{},children({token(f,u"x")}));auto*holder=create(f,"mlabeledtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[231])));
enabled=false;};
"math_port_table_parent_verify_mtr_76"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtr",{},children({token(f,u"x")}));auto*holder=create(f,"mlabeledtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[231])));
enabled=false;};
"math_port_table_parent_verify_mlabeledtr_77"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mlabeledtr",{},children({token(f,u"x")}));auto*holder=n;PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[240])));
enabled=false;};
"math_port_table_parent_verify_mlabeledtr_78"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mlabeledtr",{},children({token(f,u"x")}));auto*holder=n;PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[243])));
enabled=false;};
"math_port_table_parent_verify_mlabeledtr_79"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mlabeledtr",{},children({token(f,u"x")}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[254])));
enabled=false;};
"math_port_table_parent_verify_mlabeledtr_80"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mlabeledtr",{},children({token(f,u"x")}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[254])));
enabled=false;};
"math_port_table_parent_verify_mlabeledtr_81"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mlabeledtr",{},children({token(f,u"x")}));auto*holder=create(f,"mtable",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[257])));
enabled=false;};
"math_port_table_parent_verify_mlabeledtr_82"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mlabeledtr",{},children({token(f,u"x")}));auto*holder=create(f,"mtable",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[260])));
enabled=false;};
"math_port_table_parent_verify_mlabeledtr_83"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mlabeledtr",{},children({token(f,u"x")}));auto*holder=create(f,"mtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[263])));
enabled=false;};
"math_port_table_parent_verify_mlabeledtr_84"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mlabeledtr",{},children({token(f,u"x")}));auto*holder=create(f,"mtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[263])));
enabled=false;};
"math_port_table_parent_verify_mlabeledtr_85"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mlabeledtr",{},children({token(f,u"x")}));auto*holder=create(f,"mlabeledtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[266])));
enabled=false;};
"math_port_table_parent_verify_mlabeledtr_86"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mlabeledtr",{},children({token(f,u"x")}));auto*holder=create(f,"mlabeledtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[266])));
enabled=false;};
"math_port_table_parent_verify_mtd_87"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtd",{},children({token(f,u"x")}));auto*holder=n;PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[267])));
enabled=false;};
"math_port_table_parent_verify_mtd_88"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtd",{},children({token(f,u"x")}));auto*holder=n;PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[267])));
enabled=false;};
"math_port_table_parent_verify_mtd_89"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtd",{},children({token(f,u"x")}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[272])));
enabled=false;};
"math_port_table_parent_verify_mtd_90"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtd",{},children({token(f,u"x")}));auto*holder=create(f,"mrow",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[272])));
enabled=false;};
"math_port_table_parent_verify_mtd_91"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtd",{},children({token(f,u"x")}));auto*holder=create(f,"mtable",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[275])));
enabled=false;};
"math_port_table_parent_verify_mtd_92"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtd",{},children({token(f,u"x")}));auto*holder=create(f,"mtable",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[275])));
enabled=false;};
"math_port_table_parent_verify_mtd_93"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtd",{},children({token(f,u"x")}));auto*holder=create(f,"mtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[208])));
enabled=false;};
"math_port_table_parent_verify_mtd_94"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtd",{},children({token(f,u"x")}));auto*holder=create(f,"mtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[208])));
enabled=false;};
"math_port_table_parent_verify_mtd_95"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtd",{},children({token(f,u"x")}));auto*holder=create(f,"mlabeledtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[171])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[243])));
enabled=false;};
"math_port_table_parent_verify_mtd_96"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtd",{},children({token(f,u"x")}));auto*holder=create(f,"mlabeledtr",{},children({n}));PropertyList options=PropertyList::record(clone(sourceValues[173])).value();
traceLog.clear();enabled=true;
verify(n,options);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(holder),clone(sourceValues[243])));
enabled=false;};
"math_port_table_outer_mtable_false_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtable",PropertyList::record(clone(sourceValues[276])).value(),children({create(f,"mtr",{},children({create(f,"mtd",{},children({token(f,u"x")}))}))}));PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
n->setInheritedAttributes(attrs,true,0,true);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[300])));
enabled=false;};
"math_port_table_outer_mtable_false_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtable",PropertyList::record(clone(sourceValues[301])).value(),children({create(f,"mtr",{},children({create(f,"mtd",{},children({token(f,u"x")}))}))}));PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
n->setInheritedAttributes(attrs,true,0,true);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[314])));
enabled=false;};
"math_port_table_outer_mtable_true_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtable",PropertyList::record(clone(sourceValues[276])).value(),children());PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
n->setInheritedAttributes(attrs,true,0,true);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[315])));
enabled=false;};
"math_port_table_outer_mtable_true_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtable",PropertyList::record(clone(sourceValues[301])).value(),children());PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
n->setInheritedAttributes(attrs,true,0,true);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[316])));
enabled=false;};
"math_port_table_outer_mtr_false_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtr",PropertyList::record(clone(sourceValues[276])).value(),children({create(f,"mtd",{},children({token(f,u"x")})),create(f,"mtd",{},children({token(f,u"y")}))}));PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
{bool caught=false;try{n->setInheritedAttributes(attrs,true,0,true);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[327])));
enabled=false;};
"math_port_table_outer_mtr_false_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtr",PropertyList::record(clone(sourceValues[301])).value(),children({create(f,"mtd",{},children({token(f,u"x")})),create(f,"mtd",{},children({token(f,u"y")}))}));PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
{bool caught=false;try{n->setInheritedAttributes(attrs,true,0,true);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[328])));
enabled=false;};
"math_port_table_outer_mtr_true_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtr",PropertyList::record(clone(sourceValues[276])).value(),children());PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
{bool caught=false;try{n->setInheritedAttributes(attrs,true,0,true);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[329])));
enabled=false;};
"math_port_table_outer_mtr_true_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtr",PropertyList::record(clone(sourceValues[301])).value(),children());PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
{bool caught=false;try{n->setInheritedAttributes(attrs,true,0,true);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[330])));
enabled=false;};
"math_port_table_outer_mlabeledtr_false_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mlabeledtr",PropertyList::record(clone(sourceValues[276])).value(),children({create(f,"mtd",{},children({token(f,u"x")})),create(f,"mtd",{},children({token(f,u"y")}))}));n->parent=create(f,"mtable",PropertyList{{"side","left"}});PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
{bool caught=false;try{n->setInheritedAttributes(attrs,true,0,true);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[333])));
enabled=false;};
"math_port_table_outer_mlabeledtr_false_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mlabeledtr",PropertyList::record(clone(sourceValues[301])).value(),children({create(f,"mtd",{},children({token(f,u"x")})),create(f,"mtd",{},children({token(f,u"y")}))}));n->parent=create(f,"mtable",PropertyList{{"side","left"}});PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
{bool caught=false;try{n->setInheritedAttributes(attrs,true,0,true);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[334])));
enabled=false;};
"math_port_table_outer_mlabeledtr_true_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mlabeledtr",PropertyList::record(clone(sourceValues[276])).value(),children());n->parent=create(f,"mtable",PropertyList{{"side","left"}});PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
{bool caught=false;try{n->setInheritedAttributes(attrs,true,0,true);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[341])));
enabled=false;};
"math_port_table_outer_mlabeledtr_true_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mlabeledtr",PropertyList::record(clone(sourceValues[301])).value(),children());n->parent=create(f,"mtable",PropertyList{{"side","left"}});PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
{bool caught=false;try{n->setInheritedAttributes(attrs,true,0,true);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[342])));
enabled=false;};
"math_port_table_outer_mtd_false_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtd",PropertyList::record(clone(sourceValues[276])).value(),children({token(f,u"x")}));PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
n->setInheritedAttributes(attrs,true,0,true);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[343])));
enabled=false;};
"math_port_table_outer_mtd_false_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtd",PropertyList::record(clone(sourceValues[301])).value(),children({token(f,u"x")}));PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
n->setInheritedAttributes(attrs,true,0,true);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[344])));
enabled=false;};
"math_port_table_outer_mtd_true_false"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtd",PropertyList::record(clone(sourceValues[276])).value(),children());PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
n->setInheritedAttributes(attrs,true,0,true);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[347])));
enabled=false;};
"math_port_table_outer_mtd_true_true"_test=[] {reset();
NF f(bindings());auto*n=create(f,"mtd",PropertyList::record(clone(sourceValues[301])).value(),children());PropertyList attrs=PropertyList::record(clone(sourceValues[283])).value();
traceLog.clear();enabled=true;
n->setInheritedAttributes(attrs,true,0,true);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[348])));
enabled=false;};
"math_port_table_alignment_exhaustion_and_callback_mtable"_test=[] {reset();
Setup s("mtable");PropertyList attrs;s.n->attributes()->set("rowalign","top");s.n->attributes()->set("columnalign","left");s.n->attributes()->set("data-break-align","bottom");s.n->childNodes->push_back(s.c);mode="tuple";
traceLog.clear();enabled=true;
hook(s.n,attrs,false,0,false);
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes","inherit/first","inherit/second","inherit/late"});
expect(bool((calls.size())==3));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[349])));
expect(deep(Property(calls[0].a),clone(sourceValues[350])));
expect(deep(calls[0].d,clone(sourceValues[14])));
expect(deep(calls[0].l,clone(sourceValues[1])));
expect(deep(calls[0].p,clone(sourceValues[14])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[350])));
expect(deep(Property(calls[1].a),clone(sourceValues[350])));
expect(deep(calls[1].d,clone(sourceValues[14])));
expect(deep(calls[1].l,clone(sourceValues[1])));
expect(deep(calls[1].p,clone(sourceValues[14])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
if(calls.size()>2){
expect(deep(calls[2].snapshot,clone(sourceValues[350])));
expect(deep(Property(calls[2].a),clone(sourceValues[350])));
expect(deep(calls[2].d,clone(sourceValues[14])));
expect(deep(calls[2].l,clone(sourceValues[1])));
expect(deep(calls[2].p,clone(sourceValues[14])));
expect(bool((Property(calls[2].a)==Property(attrs))==false));
expect(bool((Property(calls[2].a)==Property(calls[0].a))==true));
}
enabled=false;};
"math_port_table_alignment_exhaustion_and_callback_mtr"_test=[] {reset();
Setup s("mtr");PropertyList attrs;s.n->attributes()->set("rowalign","top");s.n->attributes()->set("columnalign","left");s.n->attributes()->set("data-break-align","bottom");s.n->childNodes->push_back(s.c);mode="tuple";
traceLog.clear();enabled=true;
hook(s.n,attrs,false,0,false);
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","inherit/first","inherit/second","inherit/late"});
expect(bool((calls.size())==3));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[351])));
expect(deep(Property(calls[0].a),clone(sourceValues[352])));
expect(deep(calls[0].d,clone(sourceValues[14])));
expect(deep(calls[0].l,clone(sourceValues[1])));
expect(deep(calls[0].p,clone(sourceValues[14])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[352])));
expect(deep(Property(calls[1].a),clone(sourceValues[352])));
expect(deep(calls[1].d,clone(sourceValues[14])));
expect(deep(calls[1].l,clone(sourceValues[1])));
expect(deep(calls[1].p,clone(sourceValues[14])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
if(calls.size()>2){
expect(deep(calls[2].snapshot,clone(sourceValues[352])));
expect(deep(Property(calls[2].a),clone(sourceValues[352])));
expect(deep(calls[2].d,clone(sourceValues[14])));
expect(deep(calls[2].l,clone(sourceValues[1])));
expect(deep(calls[2].p,clone(sourceValues[14])));
expect(bool((Property(calls[2].a)==Property(attrs))==false));
expect(bool((Property(calls[2].a)==Property(calls[0].a))==true));
}
enabled=false;};
"math_port_table_alignment_exhaustion_and_callback_mlabeledtr"_test=[] {reset();
Setup s("mlabeledtr");PropertyList attrs;s.n->attributes()->set("rowalign","top");s.n->attributes()->set("columnalign","left");s.n->attributes()->set("data-break-align","bottom");s.n->childNodes->push_back(s.c);mode="tuple";
traceLog.clear();enabled=true;
hook(s.n,attrs,false,0,false);
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","inherit/first","inherit/second","inherit/late"});
expect(bool((calls.size())==3));
if(calls.size()>0){
expect(deep(calls[0].snapshot,clone(sourceValues[353])));
expect(deep(Property(calls[0].a),clone(sourceValues[354])));
expect(deep(calls[0].d,clone(sourceValues[14])));
expect(deep(calls[0].l,clone(sourceValues[1])));
expect(deep(calls[0].p,clone(sourceValues[14])));
expect(bool((Property(calls[0].a)==Property(attrs))==false));
}
if(calls.size()>1){
expect(deep(calls[1].snapshot,clone(sourceValues[355])));
expect(deep(Property(calls[1].a),clone(sourceValues[354])));
expect(deep(calls[1].d,clone(sourceValues[14])));
expect(deep(calls[1].l,clone(sourceValues[1])));
expect(deep(calls[1].p,clone(sourceValues[14])));
expect(bool((Property(calls[1].a)==Property(attrs))==false));
expect(bool((Property(calls[1].a)==Property(calls[0].a))==true));
}
if(calls.size()>2){
expect(deep(calls[2].snapshot,clone(sourceValues[354])));
expect(deep(Property(calls[2].a),clone(sourceValues[354])));
expect(deep(calls[2].d,clone(sourceValues[14])));
expect(deep(calls[2].l,clone(sourceValues[1])));
expect(deep(calls[2].p,clone(sourceValues[14])));
expect(bool((Property(calls[2].a)==Property(attrs))==false));
expect(bool((Property(calls[2].a)==Property(calls[0].a))==true));
}
enabled=false;};
"math_port_table_null_inherited_receiver_partial_mtable"_test=[] {reset();
Setup s("mtable");PropertyList attrs;s.n->attributes()->set("rowalign","top");s.n->attributes()->set("columnalign","right");s.n->attributes()->set("data-break-align","bottom");mode="null-inherit";
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes"});
expect(deep(partialInherited,clone(sourceValues[356])));
expect(bool((calls.size())==0));
expect(deep(snapshot(s.n),clone(sourceValues[360])));
enabled=false;};
"math_port_table_null_inherited_receiver_partial_mtr"_test=[] {reset();
Setup s("mtr");PropertyList attrs;s.n->attributes()->set("rowalign","top");s.n->attributes()->set("columnalign","right");s.n->attributes()->set("data-break-align","bottom");mode="null-inherit";
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes"});
expect(deep(partialInherited,clone(sourceValues[361])));
expect(bool((calls.size())==0));
expect(deep(snapshot(s.n),clone(sourceValues[364])));
enabled=false;};
"math_port_table_null_inherited_receiver_partial_mlabeledtr"_test=[] {reset();
Setup s("mlabeledtr");PropertyList attrs;s.n->attributes()->set("rowalign","top");s.n->attributes()->set("columnalign","right");s.n->attributes()->set("data-break-align","bottom");mode="null-inherit";
traceLog.clear();enabled=true;
{bool caught=false;try{hook(s.n,attrs,false,0,false);}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes"});
expect(deep(partialInherited,clone(sourceValues[353])));
expect(bool((calls.size())==0));
expect(deep(snapshot(s.n),clone(sourceValues[367])));
enabled=false;};
"math_port_table_table_display_default_and_cramped_raw_values"_test=[] {reset();
struct Rule{Property explicitDisplay,defaultDisplay,cramped;bool display,prime;};const Rule rows[]={
{clone(sourceValues[22]),clone(sourceValues[14]),clone(sourceValues[22]),false,false},
{clone(sourceValues[22]),clone(sourceValues[14]),clone(sourceValues[14]),false,false},
{clone(sourceValues[22]),clone(sourceValues[14]),clone(sourceValues[2]),false,true},
{clone(sourceValues[22]),clone(sourceValues[14]),clone(sourceValues[31]),false,true},
{clone(sourceValues[22]),clone(sourceValues[14]),clone(sourceValues[1]),false,false},
{clone(sourceValues[22]),clone(sourceValues[2]),clone(sourceValues[22]),true,false},
{clone(sourceValues[22]),clone(sourceValues[2]),clone(sourceValues[14]),true,false},
{clone(sourceValues[22]),clone(sourceValues[2]),clone(sourceValues[2]),true,true},
{clone(sourceValues[22]),clone(sourceValues[2]),clone(sourceValues[31]),true,true},
{clone(sourceValues[22]),clone(sourceValues[2]),clone(sourceValues[1]),true,false},
{clone(sourceValues[22]),clone(sourceValues[18]),clone(sourceValues[22]),false,false},
{clone(sourceValues[22]),clone(sourceValues[18]),clone(sourceValues[14]),false,false},
{clone(sourceValues[22]),clone(sourceValues[18]),clone(sourceValues[2]),false,true},
{clone(sourceValues[22]),clone(sourceValues[18]),clone(sourceValues[31]),false,true},
{clone(sourceValues[22]),clone(sourceValues[18]),clone(sourceValues[1]),false,false},
{clone(sourceValues[22]),clone(sourceValues[31]),clone(sourceValues[22]),true,false},
{clone(sourceValues[22]),clone(sourceValues[31]),clone(sourceValues[14]),true,false},
{clone(sourceValues[22]),clone(sourceValues[31]),clone(sourceValues[2]),true,true},
{clone(sourceValues[22]),clone(sourceValues[31]),clone(sourceValues[31]),true,true},
{clone(sourceValues[22]),clone(sourceValues[31]),clone(sourceValues[1]),true,false},
{clone(sourceValues[18]),clone(sourceValues[14]),clone(sourceValues[22]),false,false},
{clone(sourceValues[18]),clone(sourceValues[14]),clone(sourceValues[14]),false,false},
{clone(sourceValues[18]),clone(sourceValues[14]),clone(sourceValues[2]),false,true},
{clone(sourceValues[18]),clone(sourceValues[14]),clone(sourceValues[31]),false,true},
{clone(sourceValues[18]),clone(sourceValues[14]),clone(sourceValues[1]),false,false},
{clone(sourceValues[18]),clone(sourceValues[2]),clone(sourceValues[22]),true,false},
{clone(sourceValues[18]),clone(sourceValues[2]),clone(sourceValues[14]),true,false},
{clone(sourceValues[18]),clone(sourceValues[2]),clone(sourceValues[2]),true,true},
{clone(sourceValues[18]),clone(sourceValues[2]),clone(sourceValues[31]),true,true},
{clone(sourceValues[18]),clone(sourceValues[2]),clone(sourceValues[1]),true,false},
{clone(sourceValues[18]),clone(sourceValues[18]),clone(sourceValues[22]),false,false},
{clone(sourceValues[18]),clone(sourceValues[18]),clone(sourceValues[14]),false,false},
{clone(sourceValues[18]),clone(sourceValues[18]),clone(sourceValues[2]),false,true},
{clone(sourceValues[18]),clone(sourceValues[18]),clone(sourceValues[31]),false,true},
{clone(sourceValues[18]),clone(sourceValues[18]),clone(sourceValues[1]),false,false},
{clone(sourceValues[18]),clone(sourceValues[31]),clone(sourceValues[22]),true,false},
{clone(sourceValues[18]),clone(sourceValues[31]),clone(sourceValues[14]),true,false},
{clone(sourceValues[18]),clone(sourceValues[31]),clone(sourceValues[2]),true,true},
{clone(sourceValues[18]),clone(sourceValues[31]),clone(sourceValues[31]),true,true},
{clone(sourceValues[18]),clone(sourceValues[31]),clone(sourceValues[1]),true,false},
{clone(sourceValues[14]),clone(sourceValues[14]),clone(sourceValues[22]),false,false},
{clone(sourceValues[14]),clone(sourceValues[14]),clone(sourceValues[14]),false,false},
{clone(sourceValues[14]),clone(sourceValues[14]),clone(sourceValues[2]),false,true},
{clone(sourceValues[14]),clone(sourceValues[14]),clone(sourceValues[31]),false,true},
{clone(sourceValues[14]),clone(sourceValues[14]),clone(sourceValues[1]),false,false},
{clone(sourceValues[14]),clone(sourceValues[2]),clone(sourceValues[22]),true,false},
{clone(sourceValues[14]),clone(sourceValues[2]),clone(sourceValues[14]),true,false},
{clone(sourceValues[14]),clone(sourceValues[2]),clone(sourceValues[2]),true,true},
{clone(sourceValues[14]),clone(sourceValues[2]),clone(sourceValues[31]),true,true},
{clone(sourceValues[14]),clone(sourceValues[2]),clone(sourceValues[1]),true,false},
{clone(sourceValues[14]),clone(sourceValues[18]),clone(sourceValues[22]),false,false},
{clone(sourceValues[14]),clone(sourceValues[18]),clone(sourceValues[14]),false,false},
{clone(sourceValues[14]),clone(sourceValues[18]),clone(sourceValues[2]),false,true},
{clone(sourceValues[14]),clone(sourceValues[18]),clone(sourceValues[31]),false,true},
{clone(sourceValues[14]),clone(sourceValues[18]),clone(sourceValues[1]),false,false},
{clone(sourceValues[14]),clone(sourceValues[31]),clone(sourceValues[22]),true,false},
{clone(sourceValues[14]),clone(sourceValues[31]),clone(sourceValues[14]),true,false},
{clone(sourceValues[14]),clone(sourceValues[31]),clone(sourceValues[2]),true,true},
{clone(sourceValues[14]),clone(sourceValues[31]),clone(sourceValues[31]),true,true},
{clone(sourceValues[14]),clone(sourceValues[31]),clone(sourceValues[1]),true,false},
{clone(sourceValues[2]),clone(sourceValues[14]),clone(sourceValues[22]),true,false},
{clone(sourceValues[2]),clone(sourceValues[14]),clone(sourceValues[14]),true,false},
{clone(sourceValues[2]),clone(sourceValues[14]),clone(sourceValues[2]),true,true},
{clone(sourceValues[2]),clone(sourceValues[14]),clone(sourceValues[31]),true,true},
{clone(sourceValues[2]),clone(sourceValues[14]),clone(sourceValues[1]),true,false},
{clone(sourceValues[2]),clone(sourceValues[2]),clone(sourceValues[22]),true,false},
{clone(sourceValues[2]),clone(sourceValues[2]),clone(sourceValues[14]),true,false},
{clone(sourceValues[2]),clone(sourceValues[2]),clone(sourceValues[2]),true,true},
{clone(sourceValues[2]),clone(sourceValues[2]),clone(sourceValues[31]),true,true},
{clone(sourceValues[2]),clone(sourceValues[2]),clone(sourceValues[1]),true,false},
{clone(sourceValues[2]),clone(sourceValues[18]),clone(sourceValues[22]),true,false},
{clone(sourceValues[2]),clone(sourceValues[18]),clone(sourceValues[14]),true,false},
{clone(sourceValues[2]),clone(sourceValues[18]),clone(sourceValues[2]),true,true},
{clone(sourceValues[2]),clone(sourceValues[18]),clone(sourceValues[31]),true,true},
{clone(sourceValues[2]),clone(sourceValues[18]),clone(sourceValues[1]),true,false},
{clone(sourceValues[2]),clone(sourceValues[31]),clone(sourceValues[22]),true,false},
{clone(sourceValues[2]),clone(sourceValues[31]),clone(sourceValues[14]),true,false},
{clone(sourceValues[2]),clone(sourceValues[31]),clone(sourceValues[2]),true,true},
{clone(sourceValues[2]),clone(sourceValues[31]),clone(sourceValues[31]),true,true},
{clone(sourceValues[2]),clone(sourceValues[31]),clone(sourceValues[1]),true,false},
{clone(sourceValues[1]),clone(sourceValues[14]),clone(sourceValues[22]),false,false},
{clone(sourceValues[1]),clone(sourceValues[14]),clone(sourceValues[14]),false,false},
{clone(sourceValues[1]),clone(sourceValues[14]),clone(sourceValues[2]),false,true},
{clone(sourceValues[1]),clone(sourceValues[14]),clone(sourceValues[31]),false,true},
{clone(sourceValues[1]),clone(sourceValues[14]),clone(sourceValues[1]),false,false},
{clone(sourceValues[1]),clone(sourceValues[2]),clone(sourceValues[22]),true,false},
{clone(sourceValues[1]),clone(sourceValues[2]),clone(sourceValues[14]),true,false},
{clone(sourceValues[1]),clone(sourceValues[2]),clone(sourceValues[2]),true,true},
{clone(sourceValues[1]),clone(sourceValues[2]),clone(sourceValues[31]),true,true},
{clone(sourceValues[1]),clone(sourceValues[2]),clone(sourceValues[1]),true,false},
{clone(sourceValues[1]),clone(sourceValues[18]),clone(sourceValues[22]),false,false},
{clone(sourceValues[1]),clone(sourceValues[18]),clone(sourceValues[14]),false,false},
{clone(sourceValues[1]),clone(sourceValues[18]),clone(sourceValues[2]),false,true},
{clone(sourceValues[1]),clone(sourceValues[18]),clone(sourceValues[31]),false,true},
{clone(sourceValues[1]),clone(sourceValues[18]),clone(sourceValues[1]),false,false},
{clone(sourceValues[1]),clone(sourceValues[31]),clone(sourceValues[22]),true,false},
{clone(sourceValues[1]),clone(sourceValues[31]),clone(sourceValues[14]),true,false},
{clone(sourceValues[1]),clone(sourceValues[31]),clone(sourceValues[2]),true,true},
{clone(sourceValues[1]),clone(sourceValues[31]),clone(sourceValues[31]),true,true},
{clone(sourceValues[1]),clone(sourceValues[31]),clone(sourceValues[1]),true,false},
{clone(sourceValues[72]),clone(sourceValues[14]),clone(sourceValues[22]),false,false},
{clone(sourceValues[72]),clone(sourceValues[14]),clone(sourceValues[14]),false,false},
{clone(sourceValues[72]),clone(sourceValues[14]),clone(sourceValues[2]),false,true},
{clone(sourceValues[72]),clone(sourceValues[14]),clone(sourceValues[31]),false,true},
{clone(sourceValues[72]),clone(sourceValues[14]),clone(sourceValues[1]),false,false},
{clone(sourceValues[72]),clone(sourceValues[2]),clone(sourceValues[22]),true,false},
{clone(sourceValues[72]),clone(sourceValues[2]),clone(sourceValues[14]),true,false},
{clone(sourceValues[72]),clone(sourceValues[2]),clone(sourceValues[2]),true,true},
{clone(sourceValues[72]),clone(sourceValues[2]),clone(sourceValues[31]),true,true},
{clone(sourceValues[72]),clone(sourceValues[2]),clone(sourceValues[1]),true,false},
{clone(sourceValues[72]),clone(sourceValues[18]),clone(sourceValues[22]),false,false},
{clone(sourceValues[72]),clone(sourceValues[18]),clone(sourceValues[14]),false,false},
{clone(sourceValues[72]),clone(sourceValues[18]),clone(sourceValues[2]),false,true},
{clone(sourceValues[72]),clone(sourceValues[18]),clone(sourceValues[31]),false,true},
{clone(sourceValues[72]),clone(sourceValues[18]),clone(sourceValues[1]),false,false},
{clone(sourceValues[72]),clone(sourceValues[31]),clone(sourceValues[22]),true,false},
{clone(sourceValues[72]),clone(sourceValues[31]),clone(sourceValues[14]),true,false},
{clone(sourceValues[72]),clone(sourceValues[31]),clone(sourceValues[2]),true,true},
{clone(sourceValues[72]),clone(sourceValues[31]),clone(sourceValues[31]),true,true},
{clone(sourceValues[72]),clone(sourceValues[31]),clone(sourceValues[1]),true,false},
{clone(sourceValues[31]),clone(sourceValues[14]),clone(sourceValues[22]),true,false},
{clone(sourceValues[31]),clone(sourceValues[14]),clone(sourceValues[14]),true,false},
{clone(sourceValues[31]),clone(sourceValues[14]),clone(sourceValues[2]),true,true},
{clone(sourceValues[31]),clone(sourceValues[14]),clone(sourceValues[31]),true,true},
{clone(sourceValues[31]),clone(sourceValues[14]),clone(sourceValues[1]),true,false},
{clone(sourceValues[31]),clone(sourceValues[2]),clone(sourceValues[22]),true,false},
{clone(sourceValues[31]),clone(sourceValues[2]),clone(sourceValues[14]),true,false},
{clone(sourceValues[31]),clone(sourceValues[2]),clone(sourceValues[2]),true,true},
{clone(sourceValues[31]),clone(sourceValues[2]),clone(sourceValues[31]),true,true},
{clone(sourceValues[31]),clone(sourceValues[2]),clone(sourceValues[1]),true,false},
{clone(sourceValues[31]),clone(sourceValues[18]),clone(sourceValues[22]),true,false},
{clone(sourceValues[31]),clone(sourceValues[18]),clone(sourceValues[14]),true,false},
{clone(sourceValues[31]),clone(sourceValues[18]),clone(sourceValues[2]),true,true},
{clone(sourceValues[31]),clone(sourceValues[18]),clone(sourceValues[31]),true,true},
{clone(sourceValues[31]),clone(sourceValues[18]),clone(sourceValues[1]),true,false},
{clone(sourceValues[31]),clone(sourceValues[31]),clone(sourceValues[22]),true,false},
{clone(sourceValues[31]),clone(sourceValues[31]),clone(sourceValues[14]),true,false},
{clone(sourceValues[31]),clone(sourceValues[31]),clone(sourceValues[2]),true,true},
{clone(sourceValues[31]),clone(sourceValues[31]),clone(sourceValues[31]),true,true},
{clone(sourceValues[31]),clone(sourceValues[31]),clone(sourceValues[1]),true,false},
{clone(sourceValues[70]),clone(sourceValues[14]),clone(sourceValues[22]),true,false},
{clone(sourceValues[70]),clone(sourceValues[14]),clone(sourceValues[14]),true,false},
{clone(sourceValues[70]),clone(sourceValues[14]),clone(sourceValues[2]),true,true},
{clone(sourceValues[70]),clone(sourceValues[14]),clone(sourceValues[31]),true,true},
{clone(sourceValues[70]),clone(sourceValues[14]),clone(sourceValues[1]),true,false},
{clone(sourceValues[70]),clone(sourceValues[2]),clone(sourceValues[22]),true,false},
{clone(sourceValues[70]),clone(sourceValues[2]),clone(sourceValues[14]),true,false},
{clone(sourceValues[70]),clone(sourceValues[2]),clone(sourceValues[2]),true,true},
{clone(sourceValues[70]),clone(sourceValues[2]),clone(sourceValues[31]),true,true},
{clone(sourceValues[70]),clone(sourceValues[2]),clone(sourceValues[1]),true,false},
{clone(sourceValues[70]),clone(sourceValues[18]),clone(sourceValues[22]),true,false},
{clone(sourceValues[70]),clone(sourceValues[18]),clone(sourceValues[14]),true,false},
{clone(sourceValues[70]),clone(sourceValues[18]),clone(sourceValues[2]),true,true},
{clone(sourceValues[70]),clone(sourceValues[18]),clone(sourceValues[31]),true,true},
{clone(sourceValues[70]),clone(sourceValues[18]),clone(sourceValues[1]),true,false},
{clone(sourceValues[70]),clone(sourceValues[31]),clone(sourceValues[22]),true,false},
{clone(sourceValues[70]),clone(sourceValues[31]),clone(sourceValues[14]),true,false},
{clone(sourceValues[70]),clone(sourceValues[31]),clone(sourceValues[2]),true,true},
{clone(sourceValues[70]),clone(sourceValues[31]),clone(sourceValues[31]),true,true},
{clone(sourceValues[70]),clone(sourceValues[31]),clone(sourceValues[1]),true,false},
};for(const auto&r:rows){Setup s("mtable");s.n->attributes()->set("displaystyle",clone(r.explicitDisplay));s.n->attributes()->getAllDefaults().setOwn("displaystyle",clone(r.defaultDisplay));s.n->attributes()->set("data-cramped",clone(r.cramped));hook(s.n,{},true,2,false);expect(calls.size()==2);for(std::size_t i=0;i<calls.size()&&i<2;++i){expect(calls[i].d==Property(r.display));expect(calls[i].p==Property(r.prime));}}
enabled=false;};
"math_port_table_indent-grow"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable",PropertyList{{"root-extra","explicit"},{"indentalign","explicit"}});PropertyList attrs=PropertyList::record(clone(sourceValues[372])).value();changingInherited=attrs;mode="indent-grow";const auto saved=m::indentAttributes;
traceLog.clear();enabled=true;
n->setInheritedAttributes(attrs,false,0,false);
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes"});
expect(deep(snapshot(n),clone(sourceValues[374])));
expect(deep(Property(attrs),clone(sourceValues[372])));
expect(bool((m::indentAttributes.size())==5));
m::indentAttributes=saved;
enabled=false;};
"math_port_table_indent-argument"_test=[] {reset();
NF f(bindings());auto*n=create(f,"probe-mtable",PropertyList{{"root-extra","explicit"},{"indentalign","explicit"}});PropertyList attrs=PropertyList::record(clone(sourceValues[372])).value();changingInherited=attrs;mode="indent-argument";const auto saved=m::indentAttributes;
traceLog.clear();enabled=true;
n->setInheritedAttributes(attrs,false,0,false);
enabled=false;expect(traceLog==std::vector<std::string>{"attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes","attributes"});
expect(deep(snapshot(n),clone(sourceValues[379])));
expect(deep(Property(attrs),clone(sourceValues[381])));
expect(bool((m::indentAttributes.size())==4));
m::indentAttributes=saved;
enabled=false;};
"math_port_table_nested_arithmetic_cells"_test=[] {reset();
NF f(bindings());auto*frac=create(f,"mfrac",{},children({token(f,u"a"),token(f,u"b")}));auto*radical=create(f,"mroot",{},children({create(f,"msqrt",{},children({frac})),token(f,u"n")}));auto*n=create(f,"mtable",{},children({create(f,"mlabeledtr",{},children({create(f,"mtd",{},children({token(f,u"label")})),create(f,"mtd",{},children({radical}))}))}));
traceLog.clear();enabled=true;
n->setInheritedAttributes({},true,0,false);
enabled=false;expect(traceLog==std::vector<std::string>{});
expect(deep(snapshot(n),clone(sourceValues[427])));
enabled=false;};
};
