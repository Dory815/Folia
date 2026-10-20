'use strict';
// Development witness: pinned ParseUtil helpers and primitive option results.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const digest=file=>crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex');
assert.equal(digest('.reference/MathJax-src/ts/input/tex/ParseUtil.ts'),'b43646071997f88f27df51610c7972a8cb5791ea76194e7318d17d1a2f81f76a');
assert.equal(digest('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/ParseUtil.js'),'adafa18cefad39e8408bcc0e4eb7103dceb6d16c9e7ea2eb013f23602c3f8941');
const reference=require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256,'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const {ParseUtil,KeyValueDef,KeyValueTypes:T}=require(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/ParseUtil.js'));
let cases=0;
function parse(source,allowed=null,error=false,l3keys=false){++cases;return ParseUtil.keyvalOptions(source,allowed,error,l3keys);}
const examples=[
 ['',false,{}],['\t \n',false,{}],
 ['flag,yes=true,no=false,flag=false,a=1,a=2,=empty',false,{flag:false,yes:true,no:false,a:'2','':'empty'}],
 [' key = { a,b=c } , other={ {x} }',false,{key:' a,b=c ',other:'x'}],
 ['{a=1,b=2}',false,{a:'1',b:'2'}],['{{a=1,b=2}}',false,{'a=1,b=2':true}],
 [' { a=1, b=2 } ',false,{a:'1',b:'2'}],
 ['a={{x}},b={y} z,c=x{y},d={x}{y}',false,{a:'x',b:'{y} z',c:'x{y}',d:'x}{y'}],
 ['a=x\\,y,b=x\\=y,c=\\{x\\},d=\\\\',false,{a:'x\\,y',b:'x\\=y',c:'\\{x\\}',d:'\\\\'}],
 ['a=x\\',false,{a:'x\\'}],['tail\\',false,{'tail\\':true}],
 ['a= x\\  ,b= x\\\\  ,c= \t y \t',false,{a:'x\\ ',b:'x\\\\',c:'y'}],
 ['a={x\\}y},b={{x\\{y}}',false,{a:'x\\}y',b:'x\\{y'}],
 ['a="x,y",b=\'x=y\'',false,{a:'"x','y"':true,b:"'x=y'"}],
 [',,a=,b= ,,,, =,last',false,{a:'',b:'','':'',last:true}],
 ['a={{{x}}},b={{ y }},c= true ',false,{a:'x',b:' y ',c:true}],
 ['a={{{x}}},b={{ y }},c= true ',true,{a:'{{x}}',b:'{ y }',c:true}],
 ['{a={{x}},b={{y}}}',true,{a:'{x}',b:'{y}'}]
];
for(const [source,l3keys,expected]of examples)assert.deepEqual(parse(source,null,false,l3keys),expected,source);
const allowed={free:0,flag:T.boolean,n:T.number,i:T.integer,s:T.string,d:T.dimen,dir:KeyValueDef.oneof('up','down')};
assert.deepEqual(parse('flag,n=-.5e+2,i=+012,s=false,d=1em,dir=up,free=true,dropped=x',allowed),{flag:true,n:-50,i:12,s:'false',d:'1em',dir:'up',free:true});
assert.deepEqual(parse('x=1,y',{}),{});assert.deepEqual(parse('x=1',null),{x:'1'});
for(const [source,filter,id]of [['a=}',null,'ExtraCloseMissingOpen'],['a={x',null,'ExtraOpenMissingClose'],['bad=x,good={missing',{good:0},'ExtraOpenMissingClose'],['unknown=1',{},'InvalidOption'],['flag=yes',allowed,'InvalidValue'],['n=1E3',allowed,'InvalidValue'],['i=1.2',allowed,'InvalidValue'],['d=no',allowed,'InvalidValue'],['dir=left',allowed,'InvalidValue']]) {
  assert.throws(()=>parse(source,filter,true),e=>e.id===id,source);
}
{
 const choices=['up','down'],type=KeyValueDef.oneof(...choices);choices[0]='left';assert.equal(type.name,'string');assert.equal(type.verify('up'),true);assert.equal(type.verify('left'),false);assert.equal(type.convert('anything'),'anything');assert.equal(KeyValueDef.oneof().verify(''),false);
 assert.throws(()=>parse('dir=up',{dir:KeyValueDef.oneof()},false),e=>e.id==='InvalidValue');
}
const validators=[
 [T.boolean,['true','false'],['','True','1']],
 [T.number,['0','-0','+1','.5','1.','-1.25e+2'],['','Infinity','NaN','1E3','0x10',' 1','1\n']],
 [T.integer,['0','-0','+12','0012'],['','1.0','1e2',' 1']],
 [T.string,['','true','{x}','x\\'],[]],
 [T.dimen,['1em','-.5pt'],['','1','no','1emx','50%']]
];
for(const [type,yes,no]of validators){for(const value of yes)assert.equal(type.verify(value),true,type.name+':'+value);for(const value of no)assert.equal(type.verify(value),false,type.name+':'+value);}
assert.equal(T.boolean.convert('yes'),false);assert.equal(T.integer.convert('0xF'),15);assert.equal(T.integer.convert('2e3'),2);assert.equal(T.string.convert(' x '),' x ');assert.equal(T.dimen.convert(' 1em '),' 1em ');
for(const [value,expected]of [['1e+',1],['2E3x',2000],[' Infinitystuff',Infinity],['\uFEFF -Infinityfoo',-Infinity],['0xF',0],['.5tail',.5],['',NaN],['hello',NaN],['.',NaN],['+',NaN],['1e-999',0],['1e999',Infinity],['-0',-0]])assert.equal(Object.is(T.number.convert(value),expected),true,value);
const custom=new KeyValueDef('positive',v=>/^\d+$/.test(v)&&Number(v)>0,v=>2*Number(v));assert.deepEqual(parse('x=3',{x:custom}),{x:6});assert.throws(()=>parse('x=0',{x:custom}),e=>e.id==='InvalidValue');
console.log(JSON.stringify({parseCases:cases,validators:5,compiled_source_sha256:reference.provenance.compiled_source_sha256}));
