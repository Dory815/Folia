// Root binds immutable Source299 predicate observations to the actual MML registry.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..');
const source=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23b-sparse-mml-source.json'))),cases=source.cases.filter(c=>/^mrow-is(?:Empty|Spacelike|Embellished)-/.test(c.name));assert.equal(cases.length,75);
function array(a){return a.length+':'+a.own.map(Number).join('')+':'+a.has.map(Number).join('')+':'+a.values.join(',');}
function fingerprint(c){return(c.error?c.error.name+'|'+c.error.message:'ok')+'|'+c.nodes.map(n=>n.id+':'+n.kind+':'+n.parent+':'+array(n.children)).join(';')+'|'+(Object.hasOwn(c.extra,'value')?String(c.extra.value):'');}
const rows=cases.map(c=>{const m=c.name.match(/^mrow-(isEmpty|isSpacelike|isEmbellished)-([HUNab])-([HUNab])-c$/);assert.ok(m);return '        {'+[c.name,m[1],m[2]+m[3]+'c',fingerprint(c)].map(JSON.stringify).join(',')+'},';}).join('\n');
const data=`// Root actual full-registry Source299 predicate subset; no boolean fixture substitution.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
namespace {
using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct Observed:MmlFactory{
 std::vector<MmlNode*> nodes;
 MmlNode* create(std::string_view kind,tree::FactoryArgs args)override{auto* n=MmlFactory::create(kind,args);nodes.push_back(n);return n;}
 std::string tag(NodeValue v)const{if(v.kind()==tree::ChildKind::Undefined)return "U";if(v.kind()==tree::ChildKind::Null)return "N";auto it=std::find(nodes.begin(),nodes.end(),v.nodeOrNull());return it==nodes.end()?"UNBOUND":"n"+std::to_string(it-nodes.begin());}
};
struct Case{std::string name,member,shape,expected;};
std::string ascii(std::u16string_view s){std::string v;for(auto c:s)v+=static_cast<char>(c);return v;}
std::string array(const Children& a,const Observed& f){std::string own,has,values;for(std::size_t i=0;i<a->size();++i){own+=a->hasOwn(i)?'1':'0';has+=a->has(i)?'1':'0';if(i)values+=',';values+=f.tag(a->get(i));}return std::to_string(a->size())+':'+own+':'+has+':'+values;}
std::string run(const Case& c){Observed f;std::array<MmlNode*,3> seed;for(auto& n:seed)n=f.create("mi",{});auto* node=f.create("mrow",{});auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->resize(c.shape.size());for(std::size_t i=0;i<c.shape.size();++i){auto v=c.shape[i];if(v!='H')a->set(i,v=='U'?NodeValue(Missing{}):v=='N'?NodeValue(PropertyNull{}):NodeValue(seed[v-'a']));}node->childNodes=a;
 std::string error="ok",result;try{bool v=c.member=="isEmpty"?node->isEmpty():c.member=="isSpacelike"?node->isSpacelike():node->isEmbellished();result=v?"true":"false";}catch(const Error& e){error=e.name+'|'+ascii(e.message);}
 std::string out=error+'|';for(std::size_t i=0;i<f.nodes.size();++i){if(i)out+=';';auto* n=f.nodes[i];out+=f.tag(n)+':'+n->kind()+':'+f.tag(n->getParent())+':'+array(n->childNodes,f);}return out+'|'+result;
}
suite mml_predicate_source=[] {"math_port_mml_predicates_actual_source"_test=[] {const Case cases[]={
${rows}
 };for(const auto& c:cases)expect(eq(run(c),c.expected))<<c.name;};};
}
`;
const file=path.join(root,'tests/math/reference_mml_predicates.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(file));fs.writeFileSync(file,data);}console.log('75 actual MML Source predicates frozen; positive and callback evidence remains separate');
