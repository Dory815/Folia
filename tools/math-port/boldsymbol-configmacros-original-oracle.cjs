'use strict';
// Locked CJS observations for the two Source-owned packages. Native tests must
// compare full compiled MML, configuration identities and callback metadata.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),bundle=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const sourceRoot=path.join(root,'.reference/MathJax-src');
const locks={
 'ts/input/tex/boldsymbol/BoldsymbolConfiguration.ts':'f017f7d31a5f6bf17a2d9d2468cf515b1f7e5948009a55304578bb93ffddb334',
 'ts/input/tex/configmacros/ConfigMacrosConfiguration.ts':'1578e2cec78cd03e83dff7439c0b2eedbb4564cef2002dbd2dba528c25197c7a',
 'cjs/input/tex/boldsymbol/BoldsymbolConfiguration.js':'b552f0c41e5929e19d967897118cb92960f873eb7a8a21e467d532158f6dfccb',
 'cjs/input/tex/configmacros/ConfigMacrosConfiguration.js':'050cfb81fdc2914459e6c4128ab2ba8a3162207f72db921351cc1fb3320914a0',
};
const sources=[];
for(const [name,expected] of Object.entries(locks)){
 const file=name.startsWith('ts/')?path.join(sourceRoot,name):path.join(bundle,name);
 const hash=crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
 assert.equal(hash,expected,`${name} changed from locked package`);sources.push({file:name,sha256:hash});
}
const load=name=>require(path.join(bundle,'cjs',name));
load('input/tex/base/BaseConfiguration.js');
load('input/tex/newcommand/NewcommandConfiguration.js');
const {BoldsymbolConfiguration}=load('input/tex/boldsymbol/BoldsymbolConfiguration.js');
const {ConfigMacrosConfiguration}=load('input/tex/configmacros/ConfigMacrosConfiguration.js');
const {MapHandler}=load('input/tex/MapHandler.js');
const {TeX}=load('input/tex.js'),{AbstractMathItem}=load('core/MathItem.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js');
function plain(v,seen=new WeakSet()){
 if(v===undefined)return {$:'undefined'};if(v===null)return null;
 if(typeof v==='number')return Number.isNaN(v)?{$:'NaN'}:Object.is(v,-0)?{$:'-0'}:v;
 if(typeof v==='function')return {$:'function',name:v.name||'',length:v.length};
 if(typeof v!=='object')return v;if(seen.has(v))return {$:'cycle'};seen.add(v);
 if(Array.isArray(v))return {$:'array',items:Array.from({length:v.length},(_,i)=>Object.hasOwn(v,i)?plain(v[i],seen):{$:'hole'})};
 if(v instanceof Map)return {$:'map',entries:[...v].map(([k,x])=>[plain(k,seen),plain(x,seen)])};
 const out={};for(const k of Reflect.ownKeys(v))out[typeof k==='symbol'?`symbol:${k.description||''}`:k]=plain(v[k],seen);return out;
}
function node(n,seen=new WeakSet()){
 if(n==null)return null;if(seen.has(n))return {$:'cycle'};seen.add(n);
 return {kind:n.kind,text:n.getText?.()??null,attrs:plain(n.attributes?.getAllAttributes?.()??{}),properties:plain(n.getAllProperties?.()??{}),children:(n.childNodes||[]).map(x=>node(x,seen))};
}
function outcome(fn){try{return {result:fn(),error:null};}catch(e){return {result:null,error:{name:e.name||e.constructor.name,id:e.id??null,message:String(e.message)}};}}
function fresh(packages,options={}){const jax=new TeX({...options,packages});jax.setMmlFactory(new MmlFactory());return jax;}
function compile(jax,input){return node(jax.compile(new AbstractMathItem(input,jax,true),{}));}
function maps(jax,names){
 const selected=Object.fromEntries(names.map(name=>{const global=MapHandler.getMap(name),active=jax.parseOptions.handlers.retrieve(name);return[name,{same:active===global,constructor:global?.constructor.name,entries:plain(global?.map)}];}));
 const handlers={};
 for(const kind of ['character','delimiter','macro','environment']){
  const handler=jax.parseOptions.handlers.get(kind);
  handlers[kind]=(handler?handler._configuration.items:[]).map(({item,priority})=>({name:item.name,priority,registeredSame:item===MapHandler.getMap(item.name)}));
 }
 return {maps:selected,handlers};
}
function optionMetadata(configuration){
 return Object.fromEntries(Object.entries(configuration.options||{}).map(([name,value])=>[name,{ownKeys:Reflect.ownKeys(value).map(String),prototype:value&&Object.getPrototypeOf(value)?.constructor?.name||null,constructor:value?.constructor?.name||null}]));
}
const rows=[];
function observe(name,input,packages,options={}){
 let jax,result;const captured=outcome(()=>{jax=fresh(packages,options);result=compile(jax,input);return result;});
 rows.push({name,input,packages,options:plain(options),...captured,
  maps:jax?maps(jax,['boldsymbol','configmacros-map','configmacros-active-map','configmacros-env-map']):null,
  packageData:jax?plain(jax.parseOptions.packageData):null});
}
for(const [name,input] of [
 ['single-token',String.raw`\boldsymbol{x}`],
 ['greek-and-operator',String.raw`\boldsymbol{\alpha+x}`],
 ['nested-scope',String.raw`\boldsymbol{x+\boldsymbol{y}}+z`],
 ['text-exclusion',String.raw`\boldsymbol{\text{abc}+x}`],
 ['variant-rewrites',String.raw`\boldsymbol{\mathit{x}+\mathfrak{y}+\mathcal{z}}`],
 ['unsupported-variant-retained',String.raw`\boldsymbol{\mathbb{R}}`],
])observe(`bold:${name}`,input,['base','boldsymbol']);
for(const [name,input,options] of [
 ['string-macro',String.raw`\foo`,{macros:{foo:'x+y'}}],
 ['array-macro',String.raw`\foo`,{macros:{foo:['x+y']}}],
 ['parameter-template',String.raw`\foo{a}{b}`,{macros:{foo:[2,'#1+#2']}}],
 ['optional-default',String.raw`\foo{x}`,{macros:{foo:[2,'#1+#2','d']}}],
 ['template-list',String.raw`\foo a,b`,{macros:{foo:['#1+#2',2,['',',']]}}],
 ['active-character',String.raw`~`,{active:{'~':'x+y'}}],
 ['configured-environment',String.raw`\begin{xx}x\end{xx}`,{environments:{xx:['a+','+b']}}],
 ['environment-option-array',String.raw`\begin{xx}{q}x\end{xx}`,{environments:{xx:['#1+','+end',1,'d']}}],
 ['bad-macro-arity',String.raw`\foo`,{macros:{foo:[4,'#1']}}],
])observe(`configmacros:${name}`,input,['base','newcommand','configmacros'],options);
observe('configmacros:standalone-package',String.raw`\foo`,['base','configmacros'],{macros:{foo:'x+y'}});
const metadata={upstream:{version:'4.1.3',commit:'fb987178c2d279a99b0db5ea02e751691703955e'},sources,cases:rows,
 configurations:{boldsymbol:{name:BoldsymbolConfiguration.name,handler:plain(BoldsymbolConfiguration.handler),nodes:Object.fromEntries(Object.entries(BoldsymbolConfiguration.nodes||{}).map(([k,v])=>[k,{name:v.name||'',length:v.length}])),postprocessors:(BoldsymbolConfiguration.postprocessors||[]).map(x=>({processor:{name:x[0]?.name||'',length:x[0]?.length},priority:x[1]}))},
 configmacros:{name:ConfigMacrosConfiguration.name,handler:plain(ConfigMacrosConfiguration.handler),items:Object.keys(ConfigMacrosConfiguration.items||{}),options:optionMetadata(ConfigMacrosConfiguration),init:{name:ConfigMacrosConfiguration.init?.name,length:ConfigMacrosConfiguration.init?.length},config:{name:ConfigMacrosConfiguration.config?.name,length:ConfigMacrosConfiguration.config?.length}}}};
const out=process.argv[2]||path.join(root,'tools/math-port/boldsymbol-configmacros-reference/original.json');
fs.mkdirSync(path.dirname(out),{recursive:true});fs.writeFileSync(out,JSON.stringify(metadata,null,2)+'\n');
console.log(JSON.stringify({locked:Object.keys(locks).length,cases:rows.length,output:path.relative(root,out)}));
