// Generated from original NodeUtil callback observations.
#include "support/folia_test.hpp"
import folia.math.input.tex.node_util;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_node_host;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
std::string val(Property p){return p.missing()?"U":std::holds_alternative<PropertyNull>(p.value)?"N":host::debugText(host::string(p));}
struct Probe:MmlMi{using MmlMi::MmlMi;std::function<void()> readAttrs,readProps;std::function<void(std::shared_ptr<Attributes>)> writeAttrs;std::function<void(std::string_view,Property)> writeProp;
std::shared_ptr<Attributes> attributes()const override{if(readAttrs)readAttrs();return MmlMi::attributes();}
void setAttributes(std::shared_ptr<Attributes> a)override{if(writeAttrs)writeAttrs(a);MmlMi::setAttributes(a);}
PropertyList getAllProperties()const override{if(readProps)readProps();return MmlMi::getAllProperties();}
void setProperty(std::string_view k,Property p)override{if(writeProp)writeProp(k,p);MmlMi::setProperty(k,p);}};
struct OpProbe:MmlMo{using MmlMo::MmlMo;std::function<void()> text;std::u16string getText()const override{if(text)text();return MmlMo::getText();}};
std::string join(const std::vector<std::string>& trace){std::string s;for(auto& t:trace){if(!s.empty())s+=",";s+=t;}return s;}
std::string copy(std::string_view mode){MmlFactory f;f.setNodeClass("probe",classBinding<Probe>());auto* old=dynamic_cast<Probe*>(f.create("probe"));auto* next=dynamic_cast<Probe*>(f.create("probe"));std::vector<std::string> trace;const auto attrs=old->attributes();old->setProperty("a",1);old->setProperty("b",2);
old->readAttrs=[&]{trace.push_back("old.attrs");if(mode=="getter-throw")throw Error("Error",u"getter");};next->writeAttrs=[&](auto){trace.push_back("next.attrs");if(mode=="setter-throw")throw Error("Error",u"setter");if(mode=="setter-add")old->setProperty("c",3);};old->readProps=[&]{trace.push_back("old.props");if(mode=="props-throw")throw Error("Error",u"props");};next->writeProp=[&](auto k,auto v){trace.push_back("set:"+std::string(k)+":"+val(v));if(mode=="second-throw"&&k=="b")throw Error("Error",u"second");if(mode=="first-mutate"&&k=="a")old->setProperty("b",9);};std::string error="ok";
try{tex::NodeUtil u;u.copyAttributes(old,mode=="null-new"?NodeValue(PropertyNull{}):mode=="undefined-new"?NodeValue(Missing{}):NodeValue(next));}catch(const Error&e){error=e.name+"|"+host::debugText(e.message);}return error+"|"+join(trace)+"|"+(next->attributes()==attrs?"same":"other")+"|"+val(next->getProperty("a"))+","+val(next->getProperty("b"))+","+val(next->getProperty("c"));}
std::string op(std::string_view mode){MmlFactory f;f.setNodeClass("mo",classBinding<OpProbe>());auto* mo=dynamic_cast<OpProbe*>(f.create("mo"));mo->appendChild(dynamic_cast<TextNode*>(f.create("text"))->setText(u"x"));std::vector<std::string> trace;struct Restore{Property old=MmlMo::operatorTable();~Restore(){MmlMo::operatorTable()=old;}}restore;
auto def=[](Property v){return Property(PropertyArray{0,0,0,PropertyList{{"test",v}}});};PropertyList infix{{"x",def("first")}},postfix{{"x",def("second")}},prefix{{"x",def("third")}};MmlMo::operatorTable()=PropertyList{{"infix",infix},{"postfix",postfix},{"prefix",prefix}};
if(mode=="false-op")infix.set("x",false);if(mode=="null-def")infix.set("x",PropertyArray{0,0,0,PropertyNull{}});if(mode=="false-attr")infix.set("x",def(false));if(mode=="null-attr")infix.set("x",def(PropertyNull{}));if(mode=="missing-attr")infix.set("x",def(Missing{}));if(mode=="explicit")mo->attributes()->set("test",false);if(mode=="inherited")mo->attributes()->setInherited("test",PropertyNull{});
mo->text=[&]{trace.push_back("text");if(mode=="capture")MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"x",def("replacement")}}}};};Property result=Missing{};std::string error="ok";try{tex::NodeUtil u;result=mode=="global"||mode=="capture"?host::get(host::get(u.getOp(mo),"3"),"test"):u.getMoAttribute(mo,"test");}catch(const Error&e){error=e.name+"|"+host::debugText(e.message);}return error+"|"+join(trace)+"|"+val(result);}
suite order=[] {
"math_port_node_util_order_0"_test=[] {expect(eq(copy("normal"),std::string("ok|old.attrs,next.attrs,old.props,set:a:1,set:b:2|same|1,2,U")))<<"normal";};
"math_port_node_util_order_1"_test=[] {expect(eq(copy("getter-throw"),std::string("Error|getter|old.attrs|other|U,U,U")))<<"getter-throw";};
"math_port_node_util_order_2"_test=[] {expect(eq(copy("setter-throw"),std::string("Error|setter|old.attrs,next.attrs|other|U,U,U")))<<"setter-throw";};
"math_port_node_util_order_3"_test=[] {expect(eq(copy("props-throw"),std::string("Error|props|old.attrs,next.attrs,old.props|same|U,U,U")))<<"props-throw";};
"math_port_node_util_order_4"_test=[] {expect(eq(copy("second-throw"),std::string("Error|second|old.attrs,next.attrs,old.props,set:a:1,set:b:2|same|1,U,U")))<<"second-throw";};
"math_port_node_util_order_5"_test=[] {expect(eq(copy("setter-add"),std::string("ok|old.attrs,next.attrs,old.props,set:a:1,set:b:2,set:c:3|same|1,2,3")))<<"setter-add";};
"math_port_node_util_order_6"_test=[] {expect(eq(copy("first-mutate"),std::string("ok|old.attrs,next.attrs,old.props,set:a:1,set:b:2|same|1,2,U")))<<"first-mutate";};
"math_port_node_util_order_7"_test=[] {expect(eq(copy("null-new"),std::string("TypeError|Cannot set properties of null (setting 'attributes')|old.attrs|other|U,U,U")))<<"null-new";};
"math_port_node_util_order_8"_test=[] {expect(eq(copy("undefined-new"),std::string("TypeError|Cannot set properties of undefined (setting 'attributes')|old.attrs|other|U,U,U")))<<"undefined-new";};
"math_port_node_util_order_9"_test=[] {expect(eq(op("global"),std::string("ok|text|first")))<<"global";};
"math_port_node_util_order_10"_test=[] {expect(eq(op("capture"),std::string("ok|text|first")))<<"capture";};
"math_port_node_util_order_11"_test=[] {expect(eq(op("false-op"),std::string("ok|text,text|second")))<<"false-op";};
"math_port_node_util_order_12"_test=[] {expect(eq(op("null-def"),std::string("ok|text,text|second")))<<"null-def";};
"math_port_node_util_order_13"_test=[] {expect(eq(op("false-attr"),std::string("ok|text|false")))<<"false-attr";};
"math_port_node_util_order_14"_test=[] {expect(eq(op("null-attr"),std::string("ok|text|N")))<<"null-attr";};
"math_port_node_util_order_15"_test=[] {expect(eq(op("missing-attr"),std::string("ok|text,text|second")))<<"missing-attr";};
"math_port_node_util_order_16"_test=[] {expect(eq(op("explicit"),std::string("ok||false")))<<"explicit";};
"math_port_node_util_order_17"_test=[] {expect(eq(op("inherited"),std::string("ok||N")))<<"inherited";};
};}
