// Root actual-CJS observations of raw operator definition value branches.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,M,u}=require('./mo-pattern-reference.cjs');
const {MmlFactory}=require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/MmlTree/MmlFactory.js'));
class Mo extends M{table(s){this.checkOperatorTable(s);}op(s){return this.getOperatorDef(s);}}
const out=['// Generated from pinned CJS raw definition branches.','#include "support/folia_test.hpp"','import folia.math.core.mml_tree.mml_nodes.mo;','import folia.math.core.mml_tree.mo_host;','import folia.math.core.mml_tree.class_binding;','import folia.math.core.tree.node_factory;',...['math','mrow'].map(n=>'import folia.math.core.mml_tree.mml_nodes.'+n+';'),
 'namespace m=folia::math::mml;namespace h=m::host;namespace tr=folia::math::tree;using folia::math::Property;using folia::math::PropertyArray;using folia::math::PropertyList;using folia::math::Missing;using folia::math::PropertyNull;using namespace boost::ut;namespace {',
 'std::u16string units(std::initializer_list<unsigned>v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}',
 'm::Children children(std::initializer_list<m::NodeValue>v={}){return std::make_shared<tr::ChildArray<m::MmlNode>>(v);}',
 'struct Mo:m::MmlMo{using MmlMo::MmlMo;void table(std::u16string_view s){checkOperatorTable(s);}Property op(std::u16string_view s){return getOperatorDef(s);}};',
 'using NF=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode>classes(){return {{"mo",m::classBinding<Mo>()},{"math",m::classBinding<m::MmlMath>()},{"mrow",m::classBinding<m::MmlMrow>()},{"inferredMrow",m::classBinding<m::MmlInferredMrow>()},{"text",m::classBinding<m::TextNode>()}};}',
 'Mo*mo(NF&f){auto*t=dynamic_cast<m::TextNode*>(f.create("text"));t->setText(u"!");return dynamic_cast<Mo*>(f.create("mo",std::array<std::any,2>{PropertyList{},children({t})}));}',
 'bool same(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return (std::isnan(*x)&&std::isnan(*y))||(*x==*y&&std::signbit(*x)==std::signbit(*y));return a==b;}',
 '}','suite math_port_mo_value_tests=[] {'];
let count=0,cases=0;const emit=s=>out.push(s),q=JSON.stringify;
function p(v){if(v===undefined)return 'Property(Missing{})';if(v===null)return 'Property(PropertyNull{})';if(typeof v==='boolean')return 'Property('+v+')';if(typeof v==='number')return 'Property('+(Number.isNaN(v)?'std::numeric_limits<double>::quiet_NaN()':Object.is(v,-0)?'-0.0':v)+')';if(typeof v==='string')return 'Property(h::debugText('+u(v)+'))';if(Array.isArray(v))return 'Property(PropertyArray{'+v.map(p).join(',')+'})';return 'Property(PropertyList{'+Object.keys(v).map(k=>'{'+q(k)+','+p(v[k])+'}').join(',')+'})';}
function pp(v,code){emit('expect(same('+code+','+p(v)+'));');count++;}
function begin(n){cases++;emit(q('math_port_mo_value_'+n)+'_test=[] {NF f(classes());auto*n=mo(f);auto old=m::MmlMo::operatorTable();');}
function end(){emit('m::MmlMo::operatorTable()=old;};');}
function sourceNode(){const f=new MmlFactory();f.setNodeClass('mo',Mo);return f.create('mo',{},[f.create('text').setText('!')]);}
for(const [name,value]of [['string','😀xy'],['number',17],['boolean',true],['array',['x','y']],['null',null],['undefined',undefined],['false',false],['zero',0]]){
 begin('definition_properties_'+name);const old=M.OPTABLE,n=sourceNode();M.OPTABLE={infix:{'!':[9,8,6,value]},prefix:{},postfix:{}};n.table('!');emit('m::MmlMo::operatorTable()=PropertyList{{"infix",PropertyList{{"!",PropertyArray{9,8,6,'+p(value)+'}}}},{"prefix",PropertyList{}},{"postfix",PropertyList{}}};n->table(u"!");');
 for(const k of Object.keys(value||{}))pp(n.attributes.getInherited(k),'n->attributes()->getInherited('+q(k)+')');emit('expect(n->attributes()->getInheritedNames()==std::vector<std::string>{'+n.attributes.getInheritedNames().map(q).join(',')+'});');count++;pp(n._texClass,'n->_texClass');pp(n.lspace,'n->lspace');pp(n.rspace,'n->rspace');M.OPTABLE=old;end();
}
for(const [name,table]of [['null',null],['undefined',undefined],['missing-form',{}],['null-form',{infix:null}],['false-form',{infix:false}],['number-form',{infix:3}]]){
 begin('table_failure_'+name);const old=M.OPTABLE,n=sourceNode();M.OPTABLE=table;let error;try{n.table('!');}catch(e){error=e;}emit('m::MmlMo::operatorTable()='+p(table)+';');assert.ok(error,'Expected failure must originate from actual CJS');emit('{bool caught=false;try{n->table(u"!");}catch(const m::Error&e){caught=e.name=='+q(error.name)+';}expect(caught);}');count++;pp(n.attributes.getInherited('form'),'n->attributes()->getInherited("form")');pp(n.getProperty('noDictDef'),'n->getProperty("noDictDef")');pp(n._texClass,'n->_texClass');pp(n.lspace,'n->lspace');pp(n.rspace,'n->rspace');M.OPTABLE=old;end();
}
emit('};');const output=out.join('\n')+'\n',target=path.join(root,'tests/math/reference_mo_values.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);console.log(JSON.stringify({assertions:count,cases,mode:process.argv.includes('--check')?'check':'generate'}));
