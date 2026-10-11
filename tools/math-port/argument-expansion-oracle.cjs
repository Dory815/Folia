// Root oracle: actual pinned MacroMap.parse/CJS helpers, never a helper model.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),base=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const hashes={
 'ts/input/tex/TokenMap.ts':'0209895b13569008b2129b61c88050e7c70e70ce9477d234b9a6e303a86a738e',
 'cjs/input/tex/TokenMap.js':'9e3f086080a6ab052b68b22a5d9ee3dbc5e17d7b0c03a31e789664b01df36cf7'};
for(const [file,sha] of Object.entries(hashes))assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(file.startsWith('ts/')?path.join(root,'.reference/MathJax-src'):base,file))).digest('hex'),sha);
assert.equal(fs.readFileSync(path.join(root,'.reference/MathJax-src/ts/input/tex/TokenMap.ts'),'utf8').replaceAll('\r\n','\n'),fs.readFileSync(path.join(base,'ts/input/tex/TokenMap.ts'),'utf8').replaceAll('\r\n','\n'));
const {MacroMap}=require(path.join(base,'cjs/input/tex/TokenMap.js')),{Macro}=require(path.join(base,'cjs/input/tex/Token.js'));
const u=s=>'units({'+s.split('').map(c=>'char16_t(0x'+c.charCodeAt(0).toString(16)+')').join(',')+'})';
const {valueEmitter}=require('./oracle-values.cjs'),v=valueEmitter(u),{p}=v,rows=[];
let observed,calls;const callback=(env,token,...args)=>{assert.equal(token,'x');observed=args;calls++;};
const map=new MacroMap('root-argument-expansion',{});map.add('x',new Macro('x',callback));
function witness(raw){observed=undefined;calls=0;map.lookup('x')._args=raw;try{map.parse([{},'x']);assert.equal(calls,1);return observed;}catch(e){assert.equal(calls,0);return {error:e.name};}}
function add(name,raw,native=p(raw)){rows.push('{'+native+','+p(witness(raw))+'}');}
const raw=[undefined,null,false,true,0,-0,NaN,Infinity,-Infinity,42,'','abc','A𝄞e\u0301','\ud800x\udfff','\ud800\ud800\udc00\udc00','a\0b',[],[false,null,undefined,-0,NaN],{}, {length:3,0:'zero',2:'two'}];
raw.forEach((r,i)=>add('raw-'+i,r));
for(const length of [undefined,null,false,true,NaN,-Infinity,Infinity,-5,-0,0,.1,.9,1,1.9,3.9,8,4294967296,9007199254740992,'','2',' 3.8 ','0x3','0b10','-1','Infinity','wat',[2],[],{}, {valueOf:0,toString:0}]){
 for(const shape of ['empty','slots','inherited','undefined']){
  const parent={0:'parent',2:[7,8]},o=shape==='inherited'?Object.create(parent):{};o.length=length;
  if(shape==='slots'){o[0]='zero';o[2]={shared:true};}if(shape==='undefined')o[0]=undefined;
  const native=shape==='inherited'?'[] {PropertyList parent{{"0","parent"},{"2",PropertyArray{7,8}}};auto input=PropertyList::withPrototype(parent);input.setOwn("length",'+p(length)+');return Property(input);}()':p(o);
  add('length-'+shape,o,native);
 }
}
for(const length of [0,1,4,8])for(let mask=0;mask<16;mask++)for(const proto of [false,true]){
 const a=new Array(length),parent=Object.create(Array.prototype);let body='[] {PropertyArray input;input.resize('+length+');';
 if(proto){parent[0]='parent';parent[3]=[1,2];Object.setPrototypeOf(a,parent);body+='PropertyList parent{{"0","parent"},{"3",PropertyArray{1,2}}};input.record().set("__proto__",parent);';}
 for(let i=0;i<length;i++)if(mask&(1<<(i%4))){const value=i%3===0?undefined:i%3===1?'slot-'+i:{index:i};a[i]=value;body+='input.set('+i+','+p(value)+');';}
 add('sparse',a,body+'return Property(input);}()');
}
const codeunits=[0,1,0x61,0x301,0xd7ff,0xd800,0xdbff,0xdc00,0xdfff,0xe000,0xffff];
for(const a of codeunits)for(const b of codeunits)add('string-units',String.fromCharCode(a,b));
for(let i=0;i<40;i++){const a=String.fromCodePoint(0x10000+i*25517);add('string-supplementary','A'+a+'e\u0301'+a);}
const intrinsics={constructor:Object,__defineGetter__:Object.prototype.__defineGetter__,__defineSetter__:Object.prototype.__defineSetter__,hasOwnProperty:Object.prototype.hasOwnProperty,lookupGetter:Object.prototype.__lookupGetter__,lookupSetter:Object.prototype.__lookupSetter__,isPrototypeOf:Object.prototype.isPrototypeOf,propertyIsEnumerable:Object.prototype.propertyIsEnumerable,toString:Object.prototype.toString,valueOf:Object.prototype.valueOf,toLocaleString:Object.prototype.toLocaleString,prototype:Object.prototype,arrayConstructor:Array,arrayToString:Array.prototype.toString,arrayJoin:Array.prototype.join,arrayPrototype:Array.prototype,numberPrototype:Number.prototype,numberConstructor:Number,numberToString:Number.prototype.toString,numberValueOf:Number.prototype.valueOf,functionPrototype:Function.prototype,functionConstructor:Function,functionCall:Function.prototype.call,functionApply:Function.prototype.apply,functionBind:Function.prototype.bind,functionToString:Function.prototype.toString};
const nativeIntrinsicNames=new Map([['__defineGetter__','defineGetter'],['__defineSetter__','defineSetter']]);
for(const [key,value] of Object.entries(intrinsics))add('intrinsic-'+key,value,'Property(folia::math::BuiltinProperty::'+(nativeIntrinsicNames.get(key)??key)+')');
const shared={x:7},input=[shared,,shared],sourceIdentity=witness(input);assert.equal(sourceIdentity[0],shared);assert.equal(sourceIdentity[2],shared);
input[0]=false;input.length=0;assert.equal(sourceIdentity.length,3);assert.equal(sourceIdentity[0],shared);shared.x=99;assert.equal(sourceIdentity[2].x,99);
const header=`// Generated by actual pinned MacroMap.parse; native host witnesses follow.
#include "support/folia_test.hpp"
import folia.math.util.argument_expansion;
import folia.math.util.property_host;
import folia.math.util.option_types;
namespace {using namespace boost::ut;using namespace folia::math;namespace h=folia::math::property_host;
std::u16string units(std::initializer_list<char16_t>v){return {v.begin(),v.end()};}
Property clone(const Property&v){if(auto a=PropertyArray::array(v)){PropertyArray r;for(std::size_t i=0;i<a->size();++i)r.push(clone(a->get(i)));return r;}if(auto a=PropertyList::object(v)){PropertyList r;for(const auto&k:a->keys())r.setOwn(k,clone(a->getOwn(k)));return r;}return v;}
bool deep(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return(std::isnan(*x)&&std::isnan(*y))||(*x==*y&&std::signbit(*x)==std::signbit(*y));auto x=PropertyList::record(a),y=PropertyList::record(b);if(x&&y){if(PropertyArray::array(a).has_value()!=PropertyArray::array(b).has_value()||x->keys()!=y->keys())return false;for(const auto&k:x->keys())if(!deep(x->getOwn(k),y->getOwn(k)))return false;return true;}return a==b;}
Property outcome(const Property&input){try{PropertyArray result;for(auto&v:arguments::expand(input))result.push(std::move(v));return result;}catch(const folia::math::options::Exception&e){return PropertyList{{"error",e.name}};}}
}
`;
const tests=`namespace {suite argument_expansion=[] {
"math_port_argument_expansion_source"_test=[] {const std::vector<std::pair<Property,Property>> cases={${rows.join(',\n')}};expect(cases.size()==${rows.length}u);for(const auto&[input,expected]:cases)expect(deep(outcome(input),expected));};
"math_port_argument_expansion_ownership"_test=[] {PropertyList shared{{"x",7}};PropertyArray input;input.resize(3);input.set(0,shared);input.set(2,shared);auto result=arguments::expand(input);auto second=arguments::expand(input);expect(result.size()==3u);if(result.size()!=3u||second.size()!=3u)return;expect(result[0]==Property(shared));expect(result[1].missing());expect(result[2]==Property(shared));expect(result.data()!=second.data());input.set(0,false);input.resize(0);expect(result.size()==3u);expect(result[0]==Property(shared));shared.setOwn("x",99);const auto nested=PropertyList::record(result[2]);expect(nested&&nested->get("x")==Property(99));result[0]=true;expect(second[0]==Property(shared));};
};}
`;
const output=header+v.declarations().join('\n')+'\n'+tests,target=path.join(root,'tests/math/reference_argument_expansion.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
console.log(JSON.stringify({cases:rows.length,source_assertions:rows.length+1,native_assertions:9,value_records:v.size(),mode:process.argv.includes('--check')?'check':'generate'}));
