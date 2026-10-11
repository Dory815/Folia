// Root uses already frozen original Source299 results, never native output.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..');
const source=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23b-sparse-mml-source.json'))),cases=source.cases.filter(c=>c.name.startsWith('append-inferred-')||c.name.startsWith('setChildren-parent-setter-'));assert.equal(cases.length,62);
function array(a){return a.length+':'+a.own.map(Number).join('')+':'+a.has.map(Number).join('')+':'+a.values.join(',');}
function fingerprint(c){return (c.error?c.error.name+'|'+c.error.message:'ok')+'|'+c.nodes.map(n=>n.id+':'+n.kind+':'+n.parent+':'+array(n.children)).join(';')+'|'+Object.keys(c.captured).sort().map(k=>k+'='+array(c.captured[k])).join(';')+'|'+(c.extra.result??'')+'|'+c.trace.join(',');}
const rows=cases.map(c=>{let kind='mrow',shape='ab',mutation='',op='append',proto=false;
 if(c.name.startsWith('setChildren-'))op='set';
 if(c.name.includes('-parent-setter-'))mutation=c.name.split('-parent-setter-')[1];
 else if(c.name.includes('-prototype-')){shape='aHc';proto=c.name.endsWith('true');}
 else{const match=c.name.match(/^append-inferred-(mrow|mfrac)-([HUNabc])-([HUNabc])-c$/);assert.ok(match);kind=match[1];shape=match[2]+match[3]+'c';}
 return '        {'+[c.name,kind,shape,mutation,op].map(JSON.stringify).join(',')+','+proto+','+JSON.stringify(fingerprint(c))+'},';}).join('\n');
const data=`// Root Source299 append/child subset: actual native full MML factory and classes.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mi;
namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::mml;
struct ParentHook:MmlMi {
 using MmlMi::MmlMi;
 std::function<void(NodeValue)> after;
 void setParent(NodeValue v)override{MmlMi::setParent(v);if(after)after(v);}
};
struct Observed:MmlFactory {
 std::vector<MmlNode*> nodes;
 std::vector<std::string> trace;
 Observed(){setNodeClass("mi",classBinding<ParentHook>());}
 MmlNode* create(std::string_view kind,tree::FactoryArgs args)override{auto* n=MmlFactory::create(kind,args);nodes.push_back(n);return n;}
 std::string tag(NodeValue v)const{if(v.kind()==tree::ChildKind::Undefined)return "U";if(v.kind()==tree::ChildKind::Null)return "N";auto it=std::find(nodes.begin(),nodes.end(),v.nodeOrNull());return it==nodes.end()?"UNBOUND":"n"+std::to_string(it-nodes.begin());}
};
struct Case{std::string name,kind,shape,mutation,op;bool proto;std::string expected;};
std::string ascii(std::u16string_view s){std::string v;for(auto c:s)v+=static_cast<char>(c);return v;}
Children shape(std::string_view s,const std::array<MmlNode*,3>& nodes){auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->resize(s.size());for(std::size_t i=0;i<s.size();++i){auto c=s[i];if(c=='H')continue;a->set(i,c=='U'?NodeValue(Missing{}):c=='N'?NodeValue(PropertyNull{}):NodeValue(nodes[c-'a']));}return a;}
std::string array(const Children& a,const Observed& f){std::string own,has,values;for(std::size_t i=0;i<a->size();++i){own+=a->hasOwn(i)?'1':'0';has+=a->has(i)?'1':'0';if(i)values+=',';values+=f.tag(a->get(i));}return std::to_string(a->size())+':'+own+':'+has+':'+values;}
std::string run(const Case& c){Observed f;std::array<MmlNode*,3> seed;for(auto& n:seed)n=f.create("mi",{});auto* row=f.create("inferredMrow",{});auto* node=f.create(c.kind,{});row->childNodes=shape(c.shape,seed);auto original=row->childNodes;std::vector<std::pair<std::string,Children>> captured{{"original",original}};
 if(c.proto)original->setPrototype(shape("Hb",seed));
 if(!c.mutation.empty()){
  captured.push_back({"target",node->childNodes});
  dynamic_cast<ParentHook*>(seed[0])->after=[&](NodeValue v){f.trace.push_back("set:"+f.tag(v));if(c.mutation=="append")original->push_back(seed[2]);if(c.mutation=="delete")original->erase(1);if(c.mutation=="shrink")original->resize(1);if(c.mutation=="rebind-source")row->childNodes=shape("c",seed);if(c.mutation=="rebind-target")node->childNodes=shape("c",seed);};
 }
 std::string error="ok",result;try{if(c.op=="set")node->setChildren(original);else result=f.tag(node->appendChild(row));}catch(const Error& e){error=e.name+'|'+ascii(e.message);}
 std::string out=error+'|';for(std::size_t i=0;i<f.nodes.size();++i){if(i)out+=';';auto* n=f.nodes[i];out+=f.tag(n)+':'+n->kind()+':'+f.tag(n->getParent())+':'+array(n->childNodes,f);}out+='|';std::sort(captured.begin(),captured.end(),[](auto&a,auto&b){return a.first<b.first;});for(std::size_t i=0;i<captured.size();++i){if(i)out+=';';out+=captured[i].first+'='+array(captured[i].second,f);}out+='|';out+=result;out+='|';for(std::size_t i=0;i<f.trace.size();++i){if(i)out+=',';out+=f.trace[i];}return out;
}
suite mml_append_source=[] {
 "math_port_mml_append_actual_source"_test=[] {
  const Case cases[]={
${rows}
  };for(const auto& c:cases)expect(eq(run(c),c.expected))<<c.name;
 };
};
}
`;
const file=path.join(root,'tests/math/reference_mml_append.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(file));fs.writeFileSync(file,data);}console.log('62 original Source append/child cases frozen');
