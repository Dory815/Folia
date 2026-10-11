// Root actual CJS comparisons for default ownership and raw attribute flow.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,u,MmlFactory,core}=require('./inheritance-reference.cjs');
const {valueEmitter}=require('./oracle-values.cjs'),v=valueEmitter(u),{p}=v;
let active=false,events=[],calls=[],raw,mode='';const note=s=>{if(active)events.push(s);};
class Plain extends core.AbstractMmlNode{get kind(){return 'plain';}}
function proxy(T){return class extends T{get kind(){note('kind');return super.kind;}get attributes(){note('attributes');return super.attributes;}set attributes(a){super.attributes=a;}
setChildInheritedAttributes(a,d,l,p){note('hook');calls.push({a,d,l,p,same:a===raw});if(mode==='throw')throw Object.assign(new Error('witness'),{name:'WitnessError'});}
add(a,b){return super.addInheritedAttributes(a,b);}};}
function setup(kind,attrs={}){active=false;events=[];calls=[];mode='';const f=new MmlFactory();f.setNodeClass('plain',Plain);const T=f.getNodeClass(kind);f.setNodeClass('probe-'+kind,proxy(T));const n=f.create('probe-'+kind,attrs);let captured=n.attributes;Object.defineProperty(n,'attributes',{get(){note('attributes');return captured;},set(a){captured=a;},configurable:true});return {f,n};}
function state(n){const a=n.attributes;return {inherited:a.getAllInherited(),explicit:a.getAllAttributes(),prime:n.getProperty('texprimestyle'),calls,events};}
function observed(run,n){let error='';active=true;try{run();}catch(e){error=e.name;}active=false;return {error,...state(n)};}
let assertions=0;const cases=[];
function test(name,cpp,wanted,expr='state(n,error)'){++assertions;cases.push('"math_port_raw_inheritance_'+name+'"_test=[] {reset();'+cpp+'expect(deep('+expr+','+p(wanted)+'));};');}
const values=[undefined,null,false,true,0,NaN,'','xy','😀\ud800',{mathcolor:['math','blue'],mathsize:['math','2em']},{mathcolor:'😀x'},{mathcolor:[null,'null-origin']},{mathcolor:[7,'number-origin']},{mathcolor:[]},{mathcolor:false},['x',null,3]];
for(const kind of ['plain','mi','mo','mstyle','mtable'])for(let i=0;i<values.length;++i)for(const count of [1,4]){
const {n}=setup(kind,{indentalign:'right'});raw=values[i];const args=[raw,undefined,undefined,undefined];const result=observed(()=>n.setInheritedAttributes(...args.slice(0,count)),n);
test(kind+'_'+i+'_'+count,'NF f(bindings());auto*n=create(f,"probe-'+kind+'",PropertyList{{"indentalign","right"}});raw='+p(raw)+';std::array<Property,4>a{raw,Missing{},Missing{},Missing{}};std::string error;active=true;try{call(n,'+count+',a);}catch(const m::Error&e){error=e.name;}active=false;',result);
}
for(const kind of ['plain','mi','mo','mstyle','mtable'])for(const count of [0,2,3,4]){
const {n}=setup(kind);raw=undefined;const a=[raw,'raw-display',null,'raw-prime'];const result=observed(()=>n.setInheritedAttributes(...a.slice(0,count)),n);
test(kind+'_omitted_'+count,'NF f(bindings());auto*n=create(f,"probe-'+kind+'");std::array<Property,4>a{Missing{},"raw-display",PropertyNull{},"raw-prime"};std::string error;active=true;try{call(n,'+count+',a);}catch(const m::Error&e){error=e.name;}active=false;',result);
}
for(const kind of ['plain','mi','mo','mstyle','mtable']){
const {n}=setup(kind);raw={mathcolor:['math','blue']};mode='throw';const result=observed(()=>n.setInheritedAttributes(raw,true,'2','yes'),n);
test(kind+'_hook_throw','NF f(bindings());auto*n=create(f,"probe-'+kind+'");raw='+p(raw)+';mode="throw";std::string error;active=true;try{n->setInheritedAttributes(raw,true,"2","yes");}catch(const m::Error&e){error=e.name;}active=false;',result);
}
// Strings enumerate UTF16 indices, but string tuple destructuring iterates code points.
for(const tuple of ['', 'a','😀x','\ud800y', [undefined,7],[false,'false-origin'],[null,'null-origin'],[7,'number-origin'],{},false,7,{0:'math',1:'indexed-record'},null,undefined]){
const {n}=setup('plain');raw={mathcolor:tuple};const result=observed(()=>n.setInheritedAttributes(raw,false,0,false),n);
test('tuple_'+cases.length,'NF f(bindings());auto*n=create(f,"probe-plain");raw='+p(raw)+';std::string error;active=true;try{n->setInheritedAttributes(raw,false,0,false);}catch(const m::Error&e){error=e.name;}active=false;',result);
}
for(const current of [undefined,null,false,3,'😀x',[],{shared:['origin','value']},JSON.parse('{"__proto__":["origin","value"],"a":1}')])for(const additions of [undefined,null,false,'ab',{a:7,displaystyle:true,scriptlevel:3,style:'s'}]){
const {n}=setup('plain');let result,error='';active=true;try{result=n.add(current,additions);}catch(e){error=e.name;}active=false;
const keys=result?Object.keys(result):[],protoArray=result?Array.isArray(Object.getPrototypeOf(result)):false;
const same=result===current,nestedAlias=result?.shared===current?.shared;
test('spread_'+cases.length,'NF f(bindings());auto*n=dynamic_cast<Probe<Plain>*>(create(f,"probe-plain"));Property current='+p(current)+';Property result;std::string error;active=true;try{result=n->add(current,'+p(additions)+');}catch(const m::Error&e){error=e.name;}active=false;', {result,error,keys,protoArray,same,nestedAlias,events},'spreadState(result,error,current)');
}
for(const kind of ['plain','mi','mo','mstyle']){
const {n}=setup(kind);const wanted=observed(()=>{n.setInheritedAttributes();n.setInheritedAttributes();},n);wanted.fresh=calls[0].a!==calls[1].a;
test(kind+'_fresh_defaults','NF f(bindings());auto*n=create(f,"probe-'+kind+'");std::string error;active=true;try{n->setInheritedAttributes();n->setInheritedAttributes();}catch(const m::Error&e){error=e.name;}active=false;auto captured=PropertyList::object(state(n,error)).value();captured.setOwn("fresh",h::get(calls.get(0),"a")!=h::get(calls.get(1),"a"));',wanted,'captured');
}
// Typed optional-chain lookup must accept raw tuple origins and avoid eager kind access.
for(const origin of [null,undefined,7,false,[],['mstyle']]){
const {n}=setup('plain');const key=String(origin);const old=core.AbstractMmlNode.noInherit[key];core.AbstractMmlNode.noInherit[key]={plain:{mathcolor:true}};raw={mathcolor:[origin,'blocked']};const result=observed(()=>n.setInheritedAttributes(raw,false,0,false),n);if(old===undefined)delete core.AbstractMmlNode.noInherit[key];else core.AbstractMmlNode.noInherit[key]=old;
test('raw_origin_'+cases.length,'NF f(bindings());auto*n=create(f,"probe-plain");const std::string key='+JSON.stringify(key)+';auto old=m::AbstractMmlNode::noInherit().getOwn(key);m::AbstractMmlNode::noInherit().setOwn(key,PropertyList{{"plain",PropertyList{{"mathcolor",true}}}});raw='+p(raw)+';std::string error;active=true;try{n->setInheritedAttributes(raw,false,0,false);}catch(const m::Error&e){error=e.name;}active=false;const auto captured=state(n,error);if(old.missing())m::AbstractMmlNode::noInherit().erase(key);else m::AbstractMmlNode::noInherit().setOwn(key,old);',result,'captured');
}
const header=`// Generated from actual pinned MathJax by inheritance-boundary-oracle.cjs.
#include "support/folia_test.hpp"
import folia.math.core.tree.node_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mstyle;
import folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.leaf_host;
namespace {using namespace boost::ut;namespace m=folia::math::mml;namespace tr=folia::math::tree;namespace h=m::host;using folia::math::Property;using folia::math::PropertyList;using folia::math::PropertyArray;using folia::math::Missing;using folia::math::PropertyNull;
std::u16string units(std::initializer_list<char16_t>v){return {v.begin(),v.end()};}
Property clone(const Property&v){if(auto a=PropertyArray::array(v)){PropertyArray r;for(std::size_t i=0;i<a->size();++i)r.push(clone(a->get(i)));return r;}if(auto a=PropertyList::object(v)){PropertyList r;for(const auto&k:a->keys())r.setOwn(k,clone(a->getOwn(k)));return r;}return v;}
bool deep(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return(std::isnan(*x)&&std::isnan(*y))||(*x==*y&&std::signbit(*x)==std::signbit(*y));auto x=PropertyList::record(a),y=PropertyList::record(b);if(x&&y){if(PropertyArray::array(a).has_value()!=PropertyArray::array(b).has_value()||x->keys()!=y->keys()){std::cerr<<"key mismatch actual:";for(auto&k:x->keys())std::cerr<<k<<",";std::cerr<<" expected:";for(auto&k:y->keys())std::cerr<<k<<",";std::cerr<<"\\n";return false;}for(const auto&k:x->keys())if(!deep(x->getOwn(k),y->getOwn(k))){std::cerr<<"at "<<k<<"\\n";return false;}return true;}if(a!=b){std::cerr<<"value mismatch actual "<<a.value.index()<<":"<<h::debugText(h::string(a))<<" expected "<<b.value.index()<<":"<<h::debugText(h::string(b))<<"\\n";return false;}return true;}
bool active=false;PropertyArray events,calls;Property raw;std::string mode;void note(std::string s){if(active)events.push(s);}void reset(){active=false;events=PropertyArray{};calls=PropertyArray{};raw=Missing{};mode.clear();}
struct Plain:m::AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "plain";}};
template<class T>struct Probe:T{using T::T;std::string kind()const override{note("kind");return T::kind();}std::shared_ptr<folia::math::Attributes>attributes()const override{note("attributes");return T::attributes();}void setChildInheritedAttributes(m::AttributeList a,Property d,Property l,Property p)override{note("hook");calls.push(PropertyList{{"a",a},{"d",d},{"l",l},{"p",p},{"same",a==raw}});if(mode=="throw")throw m::Error("WitnessError",u"witness");}Property add(Property a,Property b){return T::addInheritedAttributes(a,b);}};
using NF=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode>bindings(){return{{"math",m::classBinding<m::MmlMath>()},{"plain",m::classBinding<Plain>()},{"mi",m::classBinding<m::MmlMi>()},{"mo",m::classBinding<m::MmlMo>()},{"mstyle",m::classBinding<m::MmlMstyle>()},{"mtable",m::classBinding<m::MmlMtable>()},{"probe-plain",m::classBinding<Probe<Plain>>()},{"probe-mi",m::classBinding<Probe<m::MmlMi>>()},{"probe-mo",m::classBinding<Probe<m::MmlMo>>()},{"probe-mstyle",m::classBinding<Probe<m::MmlMstyle>>()},{"probe-mtable",m::classBinding<Probe<m::MmlMtable>>()},{"mrow",m::classBinding<m::MmlMrow>()},{"inferredMrow",m::classBinding<m::MmlInferredMrow>()},{"text",m::classBinding<m::TextNode>()}};}
m::MmlNode*create(NF&f,std::string_view k,PropertyList a={}){return f.create(k,std::array<std::any,2>{a,std::make_shared<std::vector<m::MmlNode*>>()});}
void call(m::MmlNode*n,int c,const std::array<Property,4>&a){switch(c){case 0:n->setInheritedAttributes();break;case 1:n->setInheritedAttributes(a[0]);break;case 2:n->setInheritedAttributes(a[0],a[1]);break;case 3:n->setInheritedAttributes(a[0],a[1],a[2]);break;default:n->setInheritedAttributes(a[0],a[1],a[2],a[3]);}}
Property state(m::MmlNode*n,std::string error){return PropertyList{{"error",error},{"inherited",n->attributes()->getAllInherited()},{"explicit",n->attributes()->getAllAttributes()},{"prime",n->getProperty("texprimestyle")},{"calls",calls},{"events",events}};}
Property spreadState(Property result,std::string error,Property current){PropertyArray keys;bool proto=false;if(auto r=PropertyList::record(result)){for(const auto&k:r->keys())keys.push(k);proto=PropertyArray::array(r->get("__proto__")).has_value();}auto shared=[](Property v){return h::nullish(v)?Property(Missing{}):h::get(v,"shared");};return PropertyList{{"result",result},{"error",error},{"keys",keys},{"protoArray",proto},{"same",result==current},{"nestedAlias",shared(result)==shared(current)},{"events",events}};}
}
`;
const output=header+v.declarations().join('\n')+'\nnamespace {suite raw_inheritance=[] {\n'+cases.join('\n')+'\n};}\n',file=path.join(root,'tests/math/reference_inheritance_boundary.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(file,output);console.log(JSON.stringify({cases:cases.length,assertions,source_values:v.size(),mode:process.argv.includes('--check')?'check':'generate'}));
