// Root original copy metadata expectations; actual MML factory/classes.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct Case{std::string kind;bool keep,nullAttributes;std::string expected;};
std::string scalar(const Property& p){if(p.missing())return "U";if(std::holds_alternative<PropertyNull>(p.value))return "N";if(auto b=std::get_if<bool>(&p.value))return *b?"T":"F";if(auto n=std::get_if<double>(&p.value))return "D"+std::to_string(static_cast<int>(*n));return "S"+std::get<std::string>(p.value);}
std::string join(const std::vector<std::string>& keys){std::string s;for(const auto& k:keys){if(!s.empty())s+=',';s+=k;}return s;}
std::string own(const std::shared_ptr<Attributes>& a){if(!a)return "N";std::string s;for(const auto& k:a->getExplicitNames()){if(!s.empty())s+=',';s+=k+'='+scalar(a->getExplicit(k));}return s;}
std::string run(const Case& c){try{MmlFactory f;auto* n=f.create(c.kind,PropertyList{{"10",10},{"2",2},{"id","root"},{"mathcolor","blue"},{"accent",true}});auto* child=f.create("mi",PropertyList{{"id","child"},{"mathcolor","orange"}});n->appendChild(child);n->attributes()->setInherited("mathsize","big");n->setProperty("10",10);n->setProperty("2",2);n->setProperty("top",5);n->setProperty("nested",PropertyList{{"v",7}});if(c.nullAttributes)n->setAttributes(nullptr);
 auto* copy=n->copy(c.keep);auto children=copy->arity()<0?copy->childNodes->get(0).requireNode("childNodes")->childNodes:copy->childNodes;auto* copiedChild=children->get(0).requireNode("attributes");
 std::vector<std::string> fields{copy->kind(),scalar(Property(n->getAllProperties())!=Property(copy->getAllProperties())),scalar(n->getProperty("nested")==copy->getProperty("nested")),join(n->getPropertyNames()),join(copy->getPropertyNames()),own(n->attributes()),own(copy->attributes()),scalar(copy->attributes()->getInherited("mathsize")),scalar(copiedChild->attributes()->getExplicit("id")),scalar(copiedChild->attributes()->getExplicit("mathcolor"))};
 copy->setProperty("top",99);PropertyList::object(copy->getProperty("nested")).value().set("v",9);copy->attributes()->set("mathcolor","green");fields.push_back(scalar(n->getProperty("top")));fields.push_back(scalar(copy->getProperty("top")));fields.push_back(scalar(PropertyList::object(n->getProperty("nested")).value().get("v")));fields.push_back(scalar(n->attributes()?n->attributes()->getExplicit("mathcolor"):Property(Missing{})));fields.push_back(scalar(copy->attributes()->getExplicit("mathcolor")));fields.push_back(own(copy->attributes()));std::string out;for(const auto& value:fields){if(!out.empty())out+='|';out+=value;}return out;
 }catch(const Error& e){std::string m;for(auto c:e.message)m+=static_cast<char>(c);return e.name+'|'+m;}}
suite metadata=[] {"math_port_mml_copy_metadata_actual_source"_test=[] {const Case cases[]={
 {"mi",false,false,"mi|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,mathcolor=Sgreen,accent=T"},
 {"mi",false,true,"mi|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mi",true,false,"mi|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,id=Sroot,mathcolor=Sgreen,accent=T"},
 {"mi",true,true,"mi|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mrow",false,false,"mrow|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,mathcolor=Sgreen,accent=T"},
 {"mrow",false,true,"mrow|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mrow",true,false,"mrow|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,id=Sroot,mathcolor=Sgreen,accent=T"},
 {"mrow",true,true,"mrow|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mfrac",false,false,"mfrac|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,mathcolor=Sgreen,accent=T"},
 {"mfrac",false,true,"mfrac|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mfrac",true,false,"mfrac|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,id=Sroot,mathcolor=Sgreen,accent=T"},
 {"mfrac",true,true,"mfrac|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"math",false,false,"math|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,mathcolor=Sblue,accent=T|Snormal|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,mathcolor=Sgreen,accent=T"},
 {"math",false,true,"math|T|T|2,10,top,nested|2,10,top,nested|N||Snormal|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"math",true,false,"math|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|Snormal|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,id=Sroot,mathcolor=Sgreen,accent=T"},
 {"math",true,true,"math|T|T|2,10,top,nested|2,10,top,nested|N||Snormal|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mstyle",false,false,"mstyle|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,mathcolor=Sgreen,accent=T"},
 {"mstyle",false,true,"mstyle|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mstyle",true,false,"mstyle|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,id=Sroot,mathcolor=Sgreen,accent=T"},
 {"mstyle",true,true,"mstyle|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"msubsup",false,false,"msubsup|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,mathcolor=Sgreen,accent=T"},
 {"msubsup",false,true,"msubsup|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"msubsup",true,false,"msubsup|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,id=Sroot,mathcolor=Sgreen,accent=T"},
 {"msubsup",true,true,"msubsup|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mtable",false,false,"mtable|T|T|2,10,useHeight,top,nested|2,10,useHeight,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,mathcolor=Sgreen,accent=T"},
 {"mtable",false,true,"mtable|T|T|2,10,useHeight,top,nested|2,10,useHeight,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mtable",true,false,"mtable|T|T|2,10,useHeight,top,nested|2,10,useHeight,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,id=Sroot,mathcolor=Sgreen,accent=T"},
 {"mtable",true,true,"mtable|T|T|2,10,useHeight,top,nested|2,10,useHeight,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mtr",false,false,"mtr|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,mathcolor=Sgreen,accent=T"},
 {"mtr",false,true,"mtr|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 {"mtr",true,false,"mtr|T|T|2,10,top,nested|2,10,top,nested|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|2=D2,10=D10,id=Sroot,mathcolor=Sblue,accent=T|S_inherit_|U|Sorange|D5|D99|D9|Sblue|Sgreen|2=D2,10=D10,id=Sroot,mathcolor=Sgreen,accent=T"},
 {"mtr",true,true,"mtr|T|T|2,10,top,nested|2,10,top,nested|N||S_inherit_|U|Sorange|D5|D99|D9|U|Sgreen|mathcolor=Sgreen"},
 };for(const auto& c:cases)expect(eq(run(c),c.expected))<<c.kind<<c.keep<<c.nullAttributes;};};
}
