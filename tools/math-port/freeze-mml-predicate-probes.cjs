// Root native observer of independent original predicates; agents cannot alter this binding.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..'),source=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23h-predicate-probe-source.json')));assert.equal(source.cases.length,111);
const rows=source.cases.map(({input:i,expected})=>'        {'+[i.kind,i.shape,i.member,i.hook??'',i.mutation??'',i.prototype??''].map(JSON.stringify).join(',')+','+Boolean(i.arrayNull)+','+Object.hasOwn(i,'second')+','+JSON.stringify(expected)+'},').join('\n');
const data=`// Root actual-registry positive predicate / callback / TextNode observer.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.mspace;
namespace {
using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct MiHook:MmlMi{using MmlMi::MmlMi;std::function<void()> after;bool isEmpty()const override{auto v=MmlMi::isEmpty();if(after)after();return v;}};
struct MoHook:MmlMo{using MmlMo::MmlMo;std::function<void()> after;bool isEmbellished()const override{auto v=MmlMo::isEmbellished();if(after)after();return v;}};
struct SpaceHook:MmlMspace{using MmlMspace::MmlMspace;std::function<void()> after;bool isSpacelike()const override{auto v=MmlMspace::isSpacelike();if(after)after();return v;}};
struct TextHook:TextNode{using TextNode::TextNode;std::function<void()> after;std::u16string getText()const override{auto v=TextNode::getText();if(after)after();return v;}};
struct RowObserver:MmlMrow{using MmlMrow::MmlMrow;std::optional<std::size_t> observedCore()const{return core_;}};
struct Observed:MmlFactory{
 std::vector<MmlNode*> nodes;
 Observed(){setNodeClass("mi",classBinding<MiHook>());setNodeClass("mo",classBinding<MoHook>());setNodeClass("mspace",classBinding<SpaceHook>());setNodeClass("text",classBinding<TextHook>());setNodeClass("mrow",classBinding<RowObserver>());}
 MmlNode* create(std::string_view kind,tree::FactoryArgs args)override{auto* n=MmlFactory::create(kind,args);nodes.push_back(n);return n;}
 std::string tag(NodeValue v)const{if(v.kind()==tree::ChildKind::Undefined)return "U";if(v.kind()==tree::ChildKind::Null)return "N";auto it=std::find(nodes.begin(),nodes.end(),v.nodeOrNull());return it==nodes.end()?"UNBOUND":"n"+std::to_string(it-nodes.begin());}
};
struct Case{std::string kind,shape,member,hook,mutation,prototype;bool arrayNull,second;std::string expected;};
std::string ascii(std::u16string_view s){std::string v;for(auto c:s)v+=static_cast<char>(c);return v;}
std::string array(const Children& a,const Observed& f){if(!a)return "N";std::string own,has,values;for(std::size_t i=0;i<a->size();++i){own+=a->hasOwn(i)?'1':'0';has+=a->has(i)?'1':'0';if(i)values+=',';values+=f.tag(a->get(i));}return std::to_string(a->size())+':'+own+':'+has+':'+values;}
std::string run(const Case& c){Observed f;std::map<char,MmlNode*> nodes;for(const auto& [label,kind]:std::array<std::pair<char,const char*>,7>{{{'a',"mi"},{'b',"mo"},{'c',"mi"},{'s',"mspace"},{'e',"text"},{'t',"text"},{'z',"text"}}})nodes[label]=f.create(kind,{});
 dynamic_cast<TextNode*>(nodes['e'])->setText(u"");dynamic_cast<TextNode*>(nodes['t'])->setText(u"x");dynamic_cast<TextNode*>(nodes['z'])->setText(u"\\U0001F600");
 const auto shape=[&](std::string_view s){auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->resize(s.size());for(std::size_t i=0;i<s.size();++i){auto v=s[i];if(v!='H')a->set(i,v=='U'?NodeValue(Missing{}):v=='N'?NodeValue(PropertyNull{}):NodeValue(nodes.at(v)));}return a;};
 nodes['c']->childNodes=shape("t");auto* node=f.create(c.kind,{});node->childNodes=c.arrayNull?Children{}:shape(c.shape);auto original=node->childNodes;if(!c.prototype.empty())original->setPrototype(shape(c.prototype));std::vector<std::string> trace;bool mutated=false;
 const auto change=[&]{char label=c.hook=="isEmpty"?'a':c.hook=="isSpacelike"?'s':c.hook=="isEmbellished"?'b':'e';trace.push_back(c.hook+':'+label);if(mutated)return;mutated=true;if(c.mutation=="append")original->push_back(nodes['c']);if(c.mutation=="delete")original->erase(1);if(c.mutation=="shrink")original->resize(1);if(c.mutation=="rebind")node->childNodes=shape("c");if(c.mutation=="replace")original->set(1,nodes['c']);};
 if(c.hook=="isEmpty")dynamic_cast<MiHook*>(nodes['a'])->after=change;if(c.hook=="isSpacelike")dynamic_cast<SpaceHook*>(nodes['s'])->after=change;if(c.hook=="isEmbellished")dynamic_cast<MoHook*>(nodes['b'])->after=change;if(c.hook=="getText")dynamic_cast<TextHook*>(nodes['e'])->after=change;
 std::string result,error="ok";const auto predicate=[&]{return c.member=="isEmpty"?node->isEmpty():c.member=="isSpacelike"?node->isSpacelike():node->isEmbellished();};try{result=predicate()?"true":"false";if(c.second){node->childNodes=shape("a");result+=predicate()?",true":",false";}}catch(const Error& e){error=e.name+'|'+ascii(e.message);}
 std::string core="-";if(auto* row=dynamic_cast<RowObserver*>(node)){auto index=row->observedCore();core=index?std::to_string(*index):"N";}
 std::string out=error+'|'+result+'|'+core+'|';for(std::size_t i=0;i<f.nodes.size();++i){if(i)out+=';';auto* n=f.nodes[i];out+=f.tag(n)+':'+n->kind()+':'+f.tag(n->getParent())+':'+array(n->childNodes,f);}out+='|'+array(original,f)+'|';for(std::size_t i=0;i<trace.size();++i){if(i)out+=',';out+=trace[i];}return out;
}
suite mml_predicate_probes=[] {"math_port_mml_predicates_positive_callbacks"_test=[] {const Case cases[]={
${rows}
 };for(const auto& c:cases)expect(eq(run(c),c.expected))<<c.kind<<c.member<<c.shape<<c.hook<<c.mutation;};};
}
`;
const file=path.join(root,'tests/math/reference_mml_predicate_probes.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(file));fs.writeFileSync(file,data);}console.log('111 original positive/text/callback predicate observations frozen');
