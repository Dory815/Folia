// Generated actual-CJS HtmlNode observations; native results never generate expected values.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_nodes.html_node;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_node_host;
import folia.math.core.tree.node_factory;
namespace m=folia::math::mml;namespace h=m::host;namespace tr=folia::math::tree;using folia::math::Property;using folia::math::PropertyList;using folia::math::PropertyArray;using folia::math::PropertyNull;using folia::math::Missing;using namespace boost::ut;namespace {
std::u16string units(std::initializer_list<unsigned>v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}
Property clone(const Property&v){if(auto a=PropertyArray::array(v)){PropertyArray r;for(std::size_t i=0;i<a->size();++i)r.push(clone(a->get(i)));return r;}if(auto a=PropertyList::object(v)){PropertyList r;for(const auto&k:a->keys())r.setOwn(k,clone(a->getOwn(k)));return r;}return v;}
bool deep(const Property&a,const Property&b){auto x=PropertyList::record(a),y=PropertyList::record(b);if(x&&y){if(x->keys()!=y->keys())return false;for(const auto&k:x->keys())if(!deep(x->getOwn(k),y->getOwn(k)))return false;return true;}return a==b;}
struct Object:m::XmlObject{std::string id;explicit Object(std::string s):id(std::move(s)){};};std::string id(m::XmlRef x){return x?dynamic_cast<Object&>(*x).id:"null";}
std::string mode;std::vector<std::string>htmlTrace;m::XmlRef original,wrapped,other;m::XMLNode*late=nullptr;m::HtmlNode*owner=nullptr;std::optional<PropertyList>received;
void note(std::string s){htmlTrace.push_back(std::move(s));}void reset(){mode.clear();htmlTrace.clear();original=std::make_shared<Object>("original");wrapped=std::make_shared<Object>("wrapped");other=std::make_shared<Object>("other");late=nullptr;owner=nullptr;received.reset();}
Property traceValue(){PropertyArray a;for(const auto&s:htmlTrace)a.push(s);return a;}
void fail(){throw m::Error("HostError",u"witness");}
template<class F>std::string caught(F&&f){try{f();return "";}catch(const m::Error&e){return e.name;}catch(const std::exception&){return "Error";}catch(int){return "number";}}
struct Probe:m::HtmlNode{using HtmlNode::HtmlNode;m::XmlRef getXML()const override{note("getXML");if(mode=="getXML-other")return other;if(mode=="getXML-throw")fail();return HtmlNode::getXML();}m::XMLNode*setXML(m::XmlRef x,m::XmlAdaptor*a=nullptr)override{note("setXML/"+id(x)+"/"+(a?"host":"null"));if(mode=="setXML-throw")fail();if(mode=="setXML-other")return late;return HtmlNode::setXML(x,a);}std::u16string textContent()const override{note("textContent");if(mode=="text-override")return units({111,118,101,114,114,105,100,101,55296,55357,56832});return HtmlNode::textContent();}m::MmlNode*mError(std::string s,m::ErrorOptions opts,bool shortError=false)override{note("mError/"+s+"/"+(shortError?"true":"false"));received=opts.value();if(mode=="error-throw")fail();if(received)received->setOwn("observed",true);return late;}m::XmlRef raw()const{return xml_;}m::XmlAdaptor*host()const{return adaptor_;}};
struct Host:m::HtmlAdaptor{PropertyList lastAttrs;Property getAttribute(m::XmlRef x,std::string_view k)override{note("attribute/"+id(x)+"/"+std::string(k));if(mode=="attribute-throw"||mode.starts_with("node-")||mode=="wrap"||mode=="fresh-wrap")fail();if(mode=="attribute-primitive")throw 7;if(mode=="attribute-foreign")throw std::runtime_error("foreign");if(mode=="attribute-rebind")owner->setXML(other,nullptr);return mode=="attribute-false"?Property(false):mode=="attribute-null"?Property(PropertyNull{}):mode=="attribute-undefined"?Property(Missing{}):Property("hdw");}m::XmlRef node(std::string_view k,PropertyList a,std::vector<m::XmlRef>c)override{note("node/"+std::string(k)+"/"+std::to_string(h::ownKeys(a).size())+"/"+std::to_string(c.size())+"/"+id(c.at(0)));if(mode=="fresh-wrap"){note(std::string("fresh/")+(Property(lastAttrs)!=Property(a)?"true":"false"));lastAttrs=a;a.setOwn("touched",true);}if(mode=="node-throw")fail();if(mode=="node-null")return nullptr;return wrapped;}std::string outerHTML(m::XmlRef x)override{note("outerHTML/"+id(x));if(mode=="outer-throw")fail();return mode=="outer-unicode"?m::host::debugText(units({60,26631,31614,62,55296,55357,56832,38,10,60,47,26631,31614,62})):"<payload>"+id(x)+"</payload>";}std::u16string textContent(m::XmlRef x)override{note("hostText/"+id(x));if(mode=="text-throw")fail();return m::host::text(id(x))+units({55296,55357,56832,38,10});}std::string kind(m::XmlRef x)override{note("hostKind/"+id(x));if(mode=="kind-throw")fail();if(mode=="kind-rebind")owner->setXML(other,nullptr);return mode=="kind-unicode"?m::host::debugText(units({26631,31614,55296,55357,56832})):"tag-"+id(x);}m::XmlRef clone(m::XmlRef x)override{note("clone/"+id(x));if(mode=="clone-throw")fail();return wrapped;}std::string serializeXML(m::XmlRef x)override{note("serializeXML/"+id(x));return "<xml>"+id(x)+"</xml>";}};
struct XMLHost:m::XmlAdaptor{m::XmlRef clone(m::XmlRef x)override{return x;}std::string serializeXML(m::XmlRef)override{return "xml";}};
using NF=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode>bindings(){return{{"math",m::classBinding<m::MmlMath>()},{"html",m::classBinding<m::HtmlNode>()},{"probe",m::classBinding<Probe>()},{"XML",m::classBinding<m::XMLNode>()},{"text",m::classBinding<m::TextNode>()},{"mi",m::classBinding<m::MmlMi>()},{"mrow",m::classBinding<m::MmlMrow>()},{"inferredMrow",m::classBinding<m::MmlInferredMrow>()}};}
struct TracedFactory:NF{using NF::NF;using NF::create;m::MmlNode*create(std::string_view k,tr::FactoryArgs args)override{note("factory/"+std::string(k));if(mode=="factory-throw")fail();return NF::create(k,args);}};
Probe*probe(NF&f){return dynamic_cast<Probe*>(f.create("probe"));}m::Children children(std::initializer_list<m::MmlNode*>c={}){auto result=std::make_shared<tr::ChildArray<m::MmlNode>>();for(auto* node:c)result->push_back(m::NodeValue(node));return result;}
}
namespace {const std::vector<Property> sourceValues=[] {std::vector<Property> values;values.reserve(52);
values.push_back(Property(h::debugText(units({97,116,116,114,105,98,117,116,101,47,111,114,105,103,105,110,97,108,47,100,97,116,97,45,109,106,120,45,104,100,119}))));
values.push_back(Property(h::debugText(units({115,101,116,88,77,76,47,111,114,105,103,105,110,97,108,47,104,111,115,116}))));
values.push_back(Property(PropertyArray{values[0],values[1]}));
values.push_back(Property(h::debugText(units({97,116,116,114,105,98,117,116,101,47,110,117,108,108,47,100,97,116,97,45,109,106,120,45,104,100,119}))));
values.push_back(Property(h::debugText(units({115,101,116,88,77,76,47,110,117,108,108,47,104,111,115,116}))));
values.push_back(Property(PropertyArray{values[3],values[4]}));
values.push_back(Property(h::debugText(units({110,111,100,101,47,115,112,97,110,47,48,47,49,47,111,114,105,103,105,110,97,108}))));
values.push_back(Property(h::debugText(units({115,101,116,88,77,76,47,119,114,97,112,112,101,100,47,104,111,115,116}))));
values.push_back(Property(PropertyArray{values[0],values[6],values[7]}));
values.push_back(Property(h::debugText(units({110,111,100,101,47,115,112,97,110,47,48,47,49,47,110,117,108,108}))));
values.push_back(Property(PropertyArray{values[3],values[9],values[7]}));
values.push_back(Property(h::debugText(units({115,101,116,88,77,76,47,111,116,104,101,114,47,110,117,108,108}))));
values.push_back(Property(PropertyArray{values[0],values[11],values[1]}));
values.push_back(Property(PropertyArray{values[3],values[11],values[4]}));
values.push_back(Property(PropertyArray{values[0],values[6],values[4]}));
values.push_back(Property(PropertyArray{values[3],values[9],values[4]}));
values.push_back(Property(PropertyArray{values[0],values[6]}));
values.push_back(Property(PropertyArray{values[3],values[9]}));
values.push_back(Property(PropertyArray{}));
values.push_back(Property(h::debugText(units({103,101,116,88,77,76}))));
values.push_back(Property(PropertyArray{values[19]}));
values.push_back(Property(h::debugText(units({111,117,116,101,114,72,84,77,76,47,111,114,105,103,105,110,97,108}))));
values.push_back(Property(PropertyArray{values[21]}));
values.push_back(Property(h::debugText(units({111,117,116,101,114,72,84,77,76,47,110,117,108,108}))));
values.push_back(Property(PropertyArray{values[23]}));
values.push_back(Property(h::debugText(units({116,101,120,116,67,111,110,116,101,110,116}))));
values.push_back(Property(h::debugText(units({104,111,115,116,84,101,120,116,47,111,114,105,103,105,110,97,108}))));
values.push_back(Property(PropertyArray{values[25],values[26]}));
values.push_back(Property(h::debugText(units({104,111,115,116,84,101,120,116,47,110,117,108,108}))));
values.push_back(Property(PropertyArray{values[25],values[28]}));
values.push_back(Property(PropertyArray{values[25]}));
values.push_back(Property(h::debugText(units({104,111,115,116,75,105,110,100,47,111,114,105,103,105,110,97,108}))));
values.push_back(Property(PropertyArray{values[31]}));
values.push_back(Property(h::debugText(units({104,111,115,116,75,105,110,100,47,110,117,108,108}))));
values.push_back(Property(PropertyArray{values[33]}));
values.push_back(Property(PropertyArray{values[31],values[11]}));
values.push_back(Property(true));
values.push_back(Property(PropertyList{{"check",values[36]}}));
values.push_back(Property(PropertyNull{}));
values.push_back(Property(h::debugText(units({109,69,114,114,111,114,47,72,84,77,76,32,99,97,110,32,111,110,108,121,32,98,101,32,97,32,99,104,105,108,100,32,111,102,32,97,32,116,111,107,101,110,32,101,108,101,109,101,110,116,47,116,114,117,101}))));
values.push_back(Property(PropertyArray{values[39]}));
values.push_back(Property(PropertyList{{"check",values[36]},{"observed",values[36]}}));
values.push_back(Property(h::debugText(units({104,111,115,116,84,101,120,116,47,111,116,104,101,114}))));
values.push_back(Property(PropertyArray{values[25],values[42]}));
values.push_back(Property(h::debugText(units({102,97,99,116,111,114,121,47,104,116,109,108}))));
values.push_back(Property(h::debugText(units({99,108,111,110,101,47,111,114,105,103,105,110,97,108}))));
values.push_back(Property(h::debugText(units({115,101,116,88,77,76,47,119,114,97,112,112,101,100,47,110,117,108,108}))));
values.push_back(Property(PropertyArray{values[44],values[45],values[46]}));
values.push_back(Property(PropertyArray{values[44],values[45]}));
values.push_back(Property(PropertyArray{values[44]}));
values.push_back(Property(h::debugText(units({102,114,101,115,104,47,116,114,117,101}))));
values.push_back(Property(PropertyArray{values[0],values[6],values[50],values[7],values[3],values[9],values[50],values[7]}));
return values;}();}
suite math_port_html_node_tests=[] {
"math_port_html_node_default_state"_test=[] {reset();
NF f(bindings());auto*n=dynamic_cast<m::HtmlNode*>(f.create("html"));
expect(bool((n->kind())=="html"));
expect(bool((n->getHTML()==nullptr)==true));
expect(bool((n->isToken())==false));
expect(bool((n->isEmpty())==true));
expect(bool((n->arity())==0));
expect(bool((dynamic_cast<m::TextContentBinding*>(n)->textContent.has_value())==true));
};
"math_port_html_node_set_normal_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[2])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="original"));
expect(bool((n->raw()==original)==true));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_normal_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[5])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-false_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-false";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[2])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="original"));
expect(bool((n->raw()==original)==true));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-false_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-false";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[5])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-null_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-null";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[2])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="original"));
expect(bool((n->raw()==original)==true));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-null_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-null";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[5])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-undefined_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-undefined";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[2])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="original"));
expect(bool((n->raw()==original)==true));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-undefined_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-undefined";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[5])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-throw_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-throw";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[8])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="wrapped"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==true));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-throw_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-throw";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[10])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="wrapped"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==true));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-primitive_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-primitive";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[8])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="wrapped"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==true));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-primitive_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-primitive";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[10])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="wrapped"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==true));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-foreign_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-foreign";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[8])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="wrapped"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==true));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-foreign_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-foreign";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[10])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="wrapped"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==true));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-rebind_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-rebind";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[12])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="original"));
expect(bool((n->raw()==original)==true));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_attribute-rebind_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="attribute-rebind";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[13])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_wrap_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="wrap";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[8])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="wrapped"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==true));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_wrap_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="wrap";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[10])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="wrapped"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==true));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==false));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_node-null_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="node-null";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[14])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_node-null_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="node-null";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[15])));
}
expect(bool((result==n)==true));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==true));
};
"math_port_html_node_set_node-throw_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="node-throw";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[16])));
}
expect(bool((result==n)==false));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==false));
};
"math_port_html_node_set_node-throw_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="node-throw";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[17])));
}
expect(bool((result==n)==false));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==false));
};
"math_port_html_node_set_setXML-throw_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="setXML-throw";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[2])));
}
expect(bool((result==n)==false));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==false));
};
"math_port_html_node_set_setXML-throw_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="setXML-throw";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[5])));
}
expect(bool((result==n)==false));
expect(bool((result==late)==false));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==false));
};
"math_port_html_node_set_setXML-other_original"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="setXML-other";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(original,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[2])));
}
expect(bool((result==n)==false));
expect(bool((result==late)==true));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==false));
};
"math_port_html_node_set_setXML-other_null"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);late=dynamic_cast<m::XMLNode*>(f.create("XML"));owner=n;mode="setXML-other";m::XMLNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[5])));
}
expect(bool((result==n)==false));
expect(bool((result==late)==true));
expect(bool((id(n->raw()))=="null"));
expect(bool((n->raw()==original)==false));
expect(bool((n->raw()==wrapped)==false));
expect(bool((n->raw()==other)==false));
expect(bool((n->raw()==nullptr)==true));
expect(bool((n->host()==&host)==false));
};
"math_port_html_node_set_host_null"_test=[] {reset();
NF f(bindings());XMLHost host;auto*n=probe(f);
{htmlTrace.clear();auto failure=caught([&]{n->setHTML(original,nullptr);});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((id(n->raw()))=="null"));
expect(bool((n->host()==nullptr)==true));
};
"math_port_html_node_set_host_xml-only"_test=[] {reset();
NF f(bindings());XMLHost host;auto*n=probe(f);
{htmlTrace.clear();auto failure=caught([&]{n->setHTML(original,&host);});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((id(n->raw()))=="null"));
expect(bool((n->host()==nullptr)==true));
};
"math_port_html_node_get_normal"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);n->setXML(original,&host);mode="";m::XmlRef result;
{htmlTrace.clear();auto failure=caught([&]{result=n->getHTML();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[20])));
}
expect(bool((id(result))=="original"));
};
"math_port_html_node_get_getXML-other"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);n->setXML(original,&host);mode="getXML-other";m::XmlRef result;
{htmlTrace.clear();auto failure=caught([&]{result=n->getHTML();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[20])));
}
expect(bool((id(result))=="other"));
};
"math_port_html_node_get_getXML-throw"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);n->setXML(original,&host);mode="getXML-throw";m::XmlRef result;
{htmlTrace.clear();auto failure=caught([&]{result=n->getHTML();});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[20])));
}
};
"math_port_html_node_getSerializedHTML_normal"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&host));mode="";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->getSerializedHTML();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[22])));
}
expect(bool(m::host::text(result)==units({60,112,97,121,108,111,97,100,62,111,114,105,103,105,110,97,108,60,47,112,97,121,108,111,97,100,62})));
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_getSerializedHTML_null-payload"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(nullptr,static_cast<m::XmlAdaptor*>(&host));mode="null-payload";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->getSerializedHTML();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[24])));
}
expect(bool(m::host::text(result)==units({60,112,97,121,108,111,97,100,62,110,117,108,108,60,47,112,97,121,108,111,97,100,62})));
expect(bool((id(n->raw()))=="null"));
};
"math_port_html_node_getSerializedHTML_no-adaptor"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,nullptr);mode="no-adaptor";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->getSerializedHTML();});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_getSerializedHTML_xml-only"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&xmlHost));mode="xml-only";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->getSerializedHTML();});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_getSerializedHTML_outer-throw"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&host));mode="outer-throw";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->getSerializedHTML();});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[22])));
}
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_getSerializedHTML_outer-unicode"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&host));mode="outer-unicode";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->getSerializedHTML();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[22])));
}
expect(bool(m::host::text(result)==units({60,26631,31614,62,55296,55357,56832,38,10,60,47,26631,31614,62})));
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_textContent_normal"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&host));mode="";std::u16string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->textContent();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[27])));
}
expect(result==units({111,114,105,103,105,110,97,108,55296,55357,56832,38,10}));
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_textContent_null-payload"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(nullptr,static_cast<m::XmlAdaptor*>(&host));mode="null-payload";std::u16string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->textContent();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[29])));
}
expect(result==units({110,117,108,108,55296,55357,56832,38,10}));
expect(bool((id(n->raw()))=="null"));
};
"math_port_html_node_textContent_no-adaptor"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,nullptr);mode="no-adaptor";std::u16string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->textContent();});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[30])));
}
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_textContent_xml-only"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&xmlHost));mode="xml-only";std::u16string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->textContent();});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[30])));
}
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_textContent_text-throw"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&host));mode="text-throw";std::u16string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->textContent();});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[27])));
}
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_toString_normal"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&host));mode="";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->toString();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[32])));
}
expect(bool(m::host::text(result)==units({72,84,77,76,61,60,116,97,103,45,111,114,105,103,105,110,97,108,62,46,46,46,60,47,116,97,103,45,111,114,105,103,105,110,97,108,62})));
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_toString_null-payload"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(nullptr,static_cast<m::XmlAdaptor*>(&host));mode="null-payload";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->toString();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[34])));
}
expect(bool(m::host::text(result)==units({72,84,77,76,61,60,116,97,103,45,110,117,108,108,62,46,46,46,60,47,116,97,103,45,110,117,108,108,62})));
expect(bool((id(n->raw()))=="null"));
};
"math_port_html_node_toString_no-adaptor"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,nullptr);mode="no-adaptor";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->toString();});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_toString_xml-only"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&xmlHost));mode="xml-only";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->toString();});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_toString_kind-throw"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&host));mode="kind-throw";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->toString();});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[32])));
}
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_toString_kind-rebind"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&host));mode="kind-rebind";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->toString();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[35])));
}
expect(bool(m::host::text(result)==units({72,84,77,76,61,60,116,97,103,45,111,114,105,103,105,110,97,108,62,46,46,46,60,47,116,97,103,45,111,114,105,103,105,110,97,108,62})));
expect(bool((id(n->raw()))=="other"));
};
"math_port_html_node_toString_kind-unicode"_test=[] {reset();
NF f(bindings());Host host;XMLHost xmlHost;auto*n=probe(f);owner=n;n->setXML(original,static_cast<m::XmlAdaptor*>(&host));mode="kind-unicode";std::string result;
{htmlTrace.clear();auto failure=caught([&]{result=n->toString();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[32])));
}
expect(bool(m::host::text(result)==units({72,84,77,76,61,60,26631,31614,55296,55357,56832,62,46,46,46,60,47,26631,31614,55296,55357,56832,62})));
expect(bool((id(n->raw()))=="original"));
};
"math_port_html_node_verify_none_object_normal"_test=[] {reset();
NF f(bindings());auto*n=probe(f);PropertyList options{{"check",true}};mode="";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::optional<PropertyList>(options));});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((received&&Property(*received)==Property(options))==false));
expect(deep(Property(options),clone(sourceValues[37])));
};
"math_port_html_node_verify_none_object_error-throw"_test=[] {reset();
NF f(bindings());auto*n=probe(f);PropertyList options{{"check",true}};mode="error-throw";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::optional<PropertyList>(options));});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((received&&Property(*received)==Property(options))==false));
expect(deep(Property(options),clone(sourceValues[37])));
};
"math_port_html_node_verify_none_null_normal"_test=[] {reset();
NF f(bindings());auto*n=probe(f);PropertyList options{{"check",true}};mode="";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::nullopt);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((received&&Property(*received)==Property(options))==false));
expect(deep(Property(PropertyNull{}),clone(sourceValues[38])));
};
"math_port_html_node_verify_none_null_error-throw"_test=[] {reset();
NF f(bindings());auto*n=probe(f);PropertyList options{{"check",true}};mode="error-throw";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::nullopt);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((received&&Property(*received)==Property(options))==false));
expect(deep(Property(PropertyNull{}),clone(sourceValues[38])));
};
"math_port_html_node_verify_mi_object_normal"_test=[] {reset();
NF f(bindings());auto*n=probe(f);n->setParent(m::NodeValue(f.create("mi")));PropertyList options{{"check",true}};mode="";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::optional<PropertyList>(options));});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((received&&Property(*received)==Property(options))==false));
expect(deep(Property(options),clone(sourceValues[37])));
};
"math_port_html_node_verify_mi_object_error-throw"_test=[] {reset();
NF f(bindings());auto*n=probe(f);n->setParent(m::NodeValue(f.create("mi")));PropertyList options{{"check",true}};mode="error-throw";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::optional<PropertyList>(options));});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((received&&Property(*received)==Property(options))==false));
expect(deep(Property(options),clone(sourceValues[37])));
};
"math_port_html_node_verify_mi_null_normal"_test=[] {reset();
NF f(bindings());auto*n=probe(f);n->setParent(m::NodeValue(f.create("mi")));PropertyList options{{"check",true}};mode="";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::nullopt);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((received&&Property(*received)==Property(options))==false));
expect(deep(Property(PropertyNull{}),clone(sourceValues[38])));
};
"math_port_html_node_verify_mi_null_error-throw"_test=[] {reset();
NF f(bindings());auto*n=probe(f);n->setParent(m::NodeValue(f.create("mi")));PropertyList options{{"check",true}};mode="error-throw";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::nullopt);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((received&&Property(*received)==Property(options))==false));
expect(deep(Property(PropertyNull{}),clone(sourceValues[38])));
};
"math_port_html_node_verify_mrow_object_normal"_test=[] {reset();
NF f(bindings());auto*n=probe(f);n->setParent(m::NodeValue(f.create("mrow")));PropertyList options{{"check",true}};mode="";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::optional<PropertyList>(options));});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[40])));
}
expect(bool((received&&Property(*received)==Property(options))==true));
expect(deep(Property(options),clone(sourceValues[41])));
};
"math_port_html_node_verify_mrow_object_error-throw"_test=[] {reset();
NF f(bindings());auto*n=probe(f);n->setParent(m::NodeValue(f.create("mrow")));PropertyList options{{"check",true}};mode="error-throw";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::optional<PropertyList>(options));});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[40])));
}
expect(bool((received&&Property(*received)==Property(options))==true));
expect(deep(Property(options),clone(sourceValues[37])));
};
"math_port_html_node_verify_mrow_null_normal"_test=[] {reset();
NF f(bindings());auto*n=probe(f);n->setParent(m::NodeValue(f.create("mrow")));PropertyList options{{"check",true}};mode="";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::nullopt);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[40])));
}
expect(bool((received&&Property(*received)==Property(options))==false));
expect(deep(Property(PropertyNull{}),clone(sourceValues[38])));
};
"math_port_html_node_verify_mrow_null_error-throw"_test=[] {reset();
NF f(bindings());auto*n=probe(f);n->setParent(m::NodeValue(f.create("mrow")));PropertyList options{{"check",true}};mode="error-throw";
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::nullopt);});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[40])));
}
expect(bool((received&&Property(*received)==Property(options))==false));
expect(deep(Property(PropertyNull{}),clone(sourceValues[38])));
};
"math_port_html_node_verify_inherited_empty_error"_test=[] {reset();
NF f(bindings());auto*n=dynamic_cast<m::HtmlNode*>(f.create("html"));n->setParent(m::NodeValue(f.create("mrow")));
{htmlTrace.clear();auto failure=caught([&]{n->verifyTree(std::nullopt);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[18])));
}
};
"math_port_html_node_real_token_normal"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);auto*left=dynamic_cast<m::TextNode*>(f.create("text"));left->setText(u"left");auto*right=dynamic_cast<m::TextNode*>(f.create("text"));right->setText(u"right");auto*token=dynamic_cast<m::MmlMi*>(f.create("mi",std::array<std::any,2>{PropertyList{},children({left,n,f.create("XML"),right})}));n->setXML(original,&host);mode="";std::u16string result;
{htmlTrace.clear();auto failure=caught([&]{result=token->getText();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[27])));
}
expect(result==units({108,101,102,116,111,114,105,103,105,110,97,108,55296,55357,56832,38,10,114,105,103,104,116}));
};
"math_port_html_node_real_token_text-override"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);auto*left=dynamic_cast<m::TextNode*>(f.create("text"));left->setText(u"left");auto*right=dynamic_cast<m::TextNode*>(f.create("text"));right->setText(u"right");auto*token=dynamic_cast<m::MmlMi*>(f.create("mi",std::array<std::any,2>{PropertyList{},children({left,n,f.create("XML"),right})}));n->setXML(original,&host);mode="text-override";std::u16string result;
{htmlTrace.clear();auto failure=caught([&]{result=token->getText();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[30])));
}
expect(result==units({108,101,102,116,111,118,101,114,114,105,100,101,55296,55357,56832,114,105,103,104,116}));
};
"math_port_html_node_real_token_text-throw"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);auto*left=dynamic_cast<m::TextNode*>(f.create("text"));left->setText(u"left");auto*right=dynamic_cast<m::TextNode*>(f.create("text"));right->setText(u"right");auto*token=dynamic_cast<m::MmlMi*>(f.create("mi",std::array<std::any,2>{PropertyList{},children({left,n,f.create("XML"),right})}));n->setXML(original,&host);mode="text-throw";std::u16string result;
{htmlTrace.clear();auto failure=caught([&]{result=token->getText();});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[27])));
}
};
"math_port_html_node_real_token_rebound"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);auto*left=dynamic_cast<m::TextNode*>(f.create("text"));left->setText(u"left");auto*right=dynamic_cast<m::TextNode*>(f.create("text"));right->setText(u"right");auto*token=dynamic_cast<m::MmlMi*>(f.create("mi",std::array<std::any,2>{PropertyList{},children({left,n,f.create("XML"),right})}));n->setXML(original,&host);n->setXML(other,&host);mode="rebound";std::u16string result;
{htmlTrace.clear();auto failure=caught([&]{result=token->getText();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[43])));
}
expect(result==units({108,101,102,116,111,116,104,101,114,55296,55357,56832,38,10,114,105,103,104,116}));
};
"math_port_html_node_copy_normal"_test=[] {reset();
TracedFactory f(bindings());f.setNodeClass("html",m::classBinding<Probe>());Host host;auto*n=probe(f);n->setXML(original,&host);mode="";m::MmlNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->copy();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[47])));
}
expect(bool((result==n)==false));
expect(bool((dynamic_cast<Probe*>(result)!=nullptr)==true));
if(auto*copied=dynamic_cast<Probe*>(result)){
expect(bool((copied->kind())=="html"));
expect(bool((id(copied->raw()))=="wrapped"));
expect(bool((copied->host()==nullptr)==true));
{htmlTrace.clear();auto failure=caught([&]{copied->getSerializedHTML();});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[18])));
}
}
};
"math_port_html_node_copy_clone-throw"_test=[] {reset();
TracedFactory f(bindings());f.setNodeClass("html",m::classBinding<Probe>());Host host;auto*n=probe(f);n->setXML(original,&host);mode="clone-throw";m::MmlNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->copy();});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[48])));
}
};
"math_port_html_node_copy_no-adaptor"_test=[] {reset();
TracedFactory f(bindings());f.setNodeClass("html",m::classBinding<Probe>());Host host;auto*n=probe(f);n->setXML(original,nullptr);mode="no-adaptor";m::MmlNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->copy();});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[49])));
}
};
"math_port_html_node_copy_factory-throw"_test=[] {reset();
TracedFactory f(bindings());f.setNodeClass("html",m::classBinding<Probe>());Host host;auto*n=probe(f);n->setXML(original,&host);mode="factory-throw";m::MmlNode*result=nullptr;
{htmlTrace.clear();auto failure=caught([&]{result=n->copy();});
expect(bool((failure)=="HostError"));
expect(deep(traceValue(),clone(sourceValues[49])));
}
};
"math_port_html_node_wrapper_argument_freshness"_test=[] {reset();
NF f(bindings());Host host;auto*n=probe(f);mode="fresh-wrap";
{htmlTrace.clear();auto failure=caught([&]{n->setHTML(original,&host);n->setHTML(nullptr,&host);});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[51])));
}
expect(bool((id(n->raw()))=="wrapped"));
};
"math_port_html_node_copy_live_token_text"_test=[] {reset();
NF f(bindings());f.setNodeClass("html",m::classBinding<Probe>());Host host;auto*n=probe(f);n->setXML(original,&host);auto*copy=dynamic_cast<Probe*>(n->copy());expect(copy!=nullptr);if(!copy)return;copy->setXML(other,&host);n->setXML(nullptr,nullptr);auto*token=dynamic_cast<m::MmlMi*>(f.create("mi",std::array<std::any,2>{PropertyList{},children({copy})}));std::u16string result;
{htmlTrace.clear();auto failure=caught([&]{result=token->getText();});
expect(bool((failure)==""));
expect(deep(traceValue(),clone(sourceValues[43])));
}
expect(result==units({111,116,104,101,114,55296,55357,56832,38,10}));
};
"math_port_html_node_set_default_omitted"_test=[] {reset();
NF f(bindings());auto*n=probe(f);
{htmlTrace.clear();auto failure=caught([&]{n->setHTML(original);});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((n->raw()==nullptr)==true));
};
"math_port_html_node_set_default_undefined"_test=[] {reset();
NF f(bindings());auto*n=probe(f);
{htmlTrace.clear();auto failure=caught([&]{n->setHTML(original);});
expect(bool((failure)=="TypeError"));
expect(deep(traceValue(),clone(sourceValues[18])));
}
expect(bool((n->raw()==nullptr)==true));
};
};
