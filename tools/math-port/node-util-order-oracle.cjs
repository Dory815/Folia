// Source callbacks witness aliasing, evaluation order and partial writes.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,MmlFactory}=require('./simple-mml-reference.cjs'),base=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const u=require(path.join(base,'input/tex/NodeUtil.js')).default,Mo=require(path.join(base,'core/MmlTree/MmlNodes/mo.js')).MmlMo;
const val=p=>p===undefined?'U':p===null?'N':String(p),rows=[];
for(const mode of ['normal','getter-throw','setter-throw','props-throw','second-throw','setter-add','first-mutate','null-new','undefined-new']){
 const f=new MmlFactory(),old=f.create('mi'),next=f.create('mn'),trace=[],attrs=old.attributes;let installed=next.attributes;
 old.setProperty('a',1);old.setProperty('b',2);
 Object.defineProperty(old,'attributes',{get(){trace.push('old.attrs');if(mode==='getter-throw')throw Error('getter');return attrs;}});
 Object.defineProperty(next,'attributes',{get(){return installed;},set(value){trace.push('next.attrs');if(mode==='setter-throw')throw Error('setter');installed=value;if(mode==='setter-add')old.setProperty('c',3);}});
 const all=old.getAllProperties;old.getAllProperties=function(){trace.push('old.props');if(mode==='props-throw')throw Error('props');return all.call(this);};
 const set=next.setProperty;next.setProperty=function(k,v){trace.push('set:'+k+':'+val(v));if(mode==='second-throw'&&k==='b')throw Error('second');if(mode==='first-mutate'&&k==='a')old.setProperty('b',9);return set.call(this,k,v);};
 let error='ok';try{u.copyAttributes(old,mode==='null-new'?null:mode==='undefined-new'?undefined:next);}catch(e){error=e.name+'|'+e.message;}
 rows.push({mode,expected:error+'|'+trace.join(',')+'|'+(installed===attrs?'same':'other')+'|'+['a','b','c'].map(k=>val(next.getProperty(k))).join(',')});
}
for(const mode of ['global','capture','false-op','null-def','false-attr','null-attr','missing-attr','explicit','inherited']){
 const f=new MmlFactory(),mo=f.create('mo',{},[f.create('text').setText('x')]),trace=[],saved=Mo.OPTABLE;
 const def=v=>[0,0,0,{test:v}];let t={infix:{x:def('first')},postfix:{x:def('second')},prefix:{x:def('third')}};Mo.OPTABLE=t;
 if(mode==='false-op')t.infix.x=false;if(mode==='null-def')t.infix.x=[0,0,0,null];if(mode==='false-attr')t.infix.x=def(false);if(mode==='null-attr')t.infix.x=def(null);if(mode==='missing-attr')t.infix.x=def(undefined);
 if(mode==='explicit')mo.attributes.set('test',false);if(mode==='inherited')mo.attributes.setInherited('test',null);
 const text=mo.getText;mo.getText=function(){trace.push('text');if(mode==='capture')Mo.OPTABLE={infix:{x:def('replacement')}};return text.call(this);};
 let result,error='ok';try{result=mode==='global'||mode==='capture'?u.getOp(mo)[3].test:u.getMoAttribute(mo,'test');}catch(e){error=e.name+'|'+e.message;}finally{Mo.OPTABLE=saved;}
 rows.push({mode,expected:error+'|'+trace.join(',')+'|'+val(result)});
}
const out=['// Generated from original NodeUtil callback observations.','#include "support/folia_test.hpp"','import folia.math.input.tex.node_util;','import folia.math.core.mml_tree.mml_factory;','import folia.math.core.mml_tree.class_binding;','import folia.math.core.mml_tree.mml_nodes.mi;','import folia.math.core.mml_tree.mml_nodes.mo;','import folia.math.core.mml_tree.mml_node_host;',
`namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
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
suite order=[] {`];
for(const [i,row]of rows.entries())out.push('"math_port_node_util_order_'+i+'"_test=[] {expect(eq('+ (i<9?'copy':'op')+'('+JSON.stringify(row.mode)+'),std::string('+JSON.stringify(row.expected)+')))<<'+JSON.stringify(row.mode)+';};');out.push('};}');
for(const[file,data]of [['docs/math-port/evidence/p40-node-util-order-source.json',JSON.stringify(rows)+'\n'],['tests/math/reference_node_util_order.cpp',out.join('\n')+'\n']]){const target=path.join(root,file);if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),data);else fs.writeFileSync(target,data);}console.log(JSON.stringify({source_calls:rows.length}));
