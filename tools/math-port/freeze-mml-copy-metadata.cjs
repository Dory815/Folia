// Root fixture reads original Source gold only; alias metrics are direct observations.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..'),gold=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23g-copy-metadata-source.json')));assert.equal(gold.cases.length,32);
const rows=gold.cases.map(c=>' {'+JSON.stringify(c.input.kind)+','+c.input.keep+','+c.input.nullAttributes+','+JSON.stringify(c.expected)+'},').join('\n');
const text=`// Root original copy metadata expectations; actual MML factory/classes.
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
${rows}
 };for(const auto& c:cases)expect(eq(run(c),c.expected))<<c.kind<<c.keep<<c.nullAttributes;};};
}
`;
const out=path.join(root,'tests/math/reference_mml_copy_metadata.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),text);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,text);}console.log('32 actual Source copy metadata cases frozen');
