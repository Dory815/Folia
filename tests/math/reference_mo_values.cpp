// Generated from pinned CJS raw definition branches.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mo_host;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.tree.node_factory;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mrow;
namespace m=folia::math::mml;namespace h=m::host;namespace tr=folia::math::tree;using folia::math::Property;using folia::math::PropertyArray;using folia::math::PropertyList;using folia::math::Missing;using folia::math::PropertyNull;using namespace boost::ut;namespace {
std::u16string units(std::initializer_list<unsigned>v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}
m::Children children(std::initializer_list<m::MmlNode*>v={}){return std::make_shared<std::vector<m::MmlNode*>>(v);}
struct Mo:m::MmlMo{using MmlMo::MmlMo;void table(std::u16string_view s){checkOperatorTable(s);}Property op(std::u16string_view s){return getOperatorDef(s);}};
using NF=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode>classes(){return {{"mo",m::classBinding<Mo>()},{"math",m::classBinding<m::MmlMath>()},{"mrow",m::classBinding<m::MmlMrow>()},{"inferredMrow",m::classBinding<m::MmlInferredMrow>()},{"text",m::classBinding<m::TextNode>()}};}
Mo*mo(NF&f){auto*t=dynamic_cast<m::TextNode*>(f.create("text"));t->setText(u"!");return dynamic_cast<Mo*>(f.create("mo",std::array<std::any,2>{PropertyList{},children({t})}));}
bool same(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return (std::isnan(*x)&&std::isnan(*y))||(*x==*y&&std::signbit(*x)==std::signbit(*y));return a==b;}
}
suite math_port_mo_value_tests=[] {
"math_port_mo_value_definition_properties_string"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"!",PropertyArray{9,8,6,Property(h::debugText(units({55357,56832,120,121})))}}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};n->table(u"!");
expect(same(n->attributes()->getInherited("0"),Property(h::debugText(units({55357})))));
expect(same(n->attributes()->getInherited("1"),Property(h::debugText(units({56832})))));
expect(same(n->attributes()->getInherited("2"),Property(h::debugText(units({120})))));
expect(same(n->attributes()->getInherited("3"),Property(h::debugText(units({121})))));
expect(n->attributes()->getInheritedNames()==std::vector<std::string>{"0","1","2","3","form"});
expect(same(n->_texClass,Property(6)));
expect(same(n->lspace,Property(0.5)));
expect(same(n->rspace,Property(0.4444444444444444)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_definition_properties_number"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"!",PropertyArray{9,8,6,Property(17)}}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};n->table(u"!");
expect(n->attributes()->getInheritedNames()==std::vector<std::string>{"form"});
expect(same(n->_texClass,Property(6)));
expect(same(n->lspace,Property(0.5)));
expect(same(n->rspace,Property(0.4444444444444444)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_definition_properties_boolean"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"!",PropertyArray{9,8,6,Property(true)}}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};n->table(u"!");
expect(n->attributes()->getInheritedNames()==std::vector<std::string>{"form"});
expect(same(n->_texClass,Property(6)));
expect(same(n->lspace,Property(0.5)));
expect(same(n->rspace,Property(0.4444444444444444)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_definition_properties_array"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"!",PropertyArray{9,8,6,Property(PropertyArray{Property(h::debugText(units({120}))),Property(h::debugText(units({121})))})}}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};n->table(u"!");
expect(same(n->attributes()->getInherited("0"),Property(h::debugText(units({120})))));
expect(same(n->attributes()->getInherited("1"),Property(h::debugText(units({121})))));
expect(n->attributes()->getInheritedNames()==std::vector<std::string>{"0","1","form"});
expect(same(n->_texClass,Property(6)));
expect(same(n->lspace,Property(0.5)));
expect(same(n->rspace,Property(0.4444444444444444)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_definition_properties_null"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"!",PropertyArray{9,8,6,Property(PropertyNull{})}}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};n->table(u"!");
expect(n->attributes()->getInheritedNames()==std::vector<std::string>{"form"});
expect(same(n->_texClass,Property(6)));
expect(same(n->lspace,Property(0.5)));
expect(same(n->rspace,Property(0.4444444444444444)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_definition_properties_undefined"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"!",PropertyArray{9,8,6,Property(Missing{})}}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};n->table(u"!");
expect(n->attributes()->getInheritedNames()==std::vector<std::string>{"form"});
expect(same(n->_texClass,Property(6)));
expect(same(n->lspace,Property(0.5)));
expect(same(n->rspace,Property(0.4444444444444444)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_definition_properties_false"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"!",PropertyArray{9,8,6,Property(false)}}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};n->table(u"!");
expect(n->attributes()->getInheritedNames()==std::vector<std::string>{"form"});
expect(same(n->_texClass,Property(6)));
expect(same(n->lspace,Property(0.5)));
expect(same(n->rspace,Property(0.4444444444444444)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_definition_properties_zero"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"!",PropertyArray{9,8,6,Property(0)}}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};n->table(u"!");
expect(n->attributes()->getInheritedNames()==std::vector<std::string>{"form"});
expect(same(n->_texClass,Property(6)));
expect(same(n->lspace,Property(0.5)));
expect(same(n->rspace,Property(0.4444444444444444)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_table_failure_null"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=Property(PropertyNull{});
{bool caught=false;try{n->table(u"!");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect(same(n->attributes()->getInherited("form"),Property(h::debugText(units({105,110,102,105,120})))));
expect(same(n->getProperty("noDictDef"),Property(Missing{})));
expect(same(n->_texClass,Property(PropertyNull{})));
expect(same(n->lspace,Property(0.2777777777777778)));
expect(same(n->rspace,Property(0.2777777777777778)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_table_failure_undefined"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=Property(Missing{});
{bool caught=false;try{n->table(u"!");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect(same(n->attributes()->getInherited("form"),Property(h::debugText(units({105,110,102,105,120})))));
expect(same(n->getProperty("noDictDef"),Property(Missing{})));
expect(same(n->_texClass,Property(PropertyNull{})));
expect(same(n->lspace,Property(0.2777777777777778)));
expect(same(n->rspace,Property(0.2777777777777778)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_table_failure_missing-form"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=Property(PropertyList{});
{bool caught=false;try{n->table(u"!");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect(same(n->attributes()->getInherited("form"),Property(h::debugText(units({105,110,102,105,120})))));
expect(same(n->getProperty("noDictDef"),Property(Missing{})));
expect(same(n->_texClass,Property(PropertyNull{})));
expect(same(n->lspace,Property(0.2777777777777778)));
expect(same(n->rspace,Property(0.2777777777777778)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_table_failure_null-form"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=Property(PropertyList{{"infix",Property(PropertyNull{})}});
{bool caught=false;try{n->table(u"!");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect(same(n->attributes()->getInherited("form"),Property(h::debugText(units({105,110,102,105,120})))));
expect(same(n->getProperty("noDictDef"),Property(Missing{})));
expect(same(n->_texClass,Property(PropertyNull{})));
expect(same(n->lspace,Property(0.2777777777777778)));
expect(same(n->rspace,Property(0.2777777777777778)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_table_failure_false-form"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=Property(PropertyList{{"infix",Property(false)}});
{bool caught=false;try{n->table(u"!");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect(same(n->attributes()->getInherited("form"),Property(h::debugText(units({105,110,102,105,120})))));
expect(same(n->getProperty("noDictDef"),Property(Missing{})));
expect(same(n->_texClass,Property(PropertyNull{})));
expect(same(n->lspace,Property(0.2777777777777778)));
expect(same(n->rspace,Property(0.2777777777777778)));
m::MmlMo::operatorTable()=old;};
"math_port_mo_value_table_failure_number-form"_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();
m::MmlMo::operatorTable()=Property(PropertyList{{"infix",Property(3)}});
{bool caught=false;try{n->table(u"!");}catch(const m::Error&e){caught=e.name=="TypeError";}expect(caught);}
expect(same(n->attributes()->getInherited("form"),Property(h::debugText(units({105,110,102,105,120})))));
expect(same(n->getProperty("noDictDef"),Property(Missing{})));
expect(same(n->_texClass,Property(PropertyNull{})));
expect(same(n->lspace,Property(0.2777777777777778)));
expect(same(n->rspace,Property(0.2777777777777778)));
m::MmlMo::operatorTable()=old;};
};
