// Frozen observations from pinned MathJax 4.1.3 Begingroup CJS and real peers.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..');
const packageRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const sources={
 'BegingroupStack.ts':'39150fade01fc10e021541380770a782f5735d4ddad0fb093ec4bd635a66f30d',
 'BegingroupMethods.ts':'e3abda9266a0100cfef4abd346180096ca6768b0b402770575d6490798719202',
 'BegingroupConfiguration.ts':'5109804cf89666cf43b215ef62867fd6c5c57ba112ffa147c8bf33cd7dc601c0',
};
const cjs={
 'BegingroupStack.js':'f4b5408d18a22da1289d9be4ff310b751de0e4703ddd0dbea4d60d6a07294725',
 'BegingroupMethods.js':'c13631818b600ab46e28f399731cc8695a0d26472b34271a15106c2fe918e993',
 'BegingroupConfiguration.js':'4e25653273ff39a916deb4aa8879292fa88f75a49d37af1189d88d02f7cf0789',
};
const directory='input/tex/begingroup';
for(const [name,expected] of Object.entries(sources)){
 const bytes=fs.readFileSync(path.join(root,'.reference/MathJax-src/ts/input/tex/begingroup',name));
 assert.equal(crypto.createHash('sha256').update(bytes).digest('hex'),expected,`${name} pinned TS changed`);
 const installed=fs.readFileSync(path.join(packageRoot,'ts/input/tex/begingroup',name),'utf8').replaceAll('\r\n','\n');
 assert.equal(installed,bytes.toString('utf8').replaceAll('\r\n','\n'),`${name} installed TS differs`);
 const js=name.replace(/\.ts$/,'.js'), compiled=fs.readFileSync(path.join(packageRoot,'cjs/input/tex/begingroup',js));
 assert.equal(crypto.createHash('sha256').update(compiled).digest('hex'),cjs[js],`${js} pinned CJS changed`);
}
const load=name=>require(path.join(packageRoot,'cjs',name));
// Package modules are evaluated before TeX instances are constructed.  The
// real upstream base package supplies the parser maps and base item setup that
// Newcommand and Begingroup extend.
load('input/tex/base/BaseConfiguration.js');
load('input/tex/newcommand/NewcommandConfiguration.js');
const {BegingroupConfiguration}=load('input/tex/begingroup/BegingroupConfiguration.js');
const {BegingroupMethods}=load('input/tex/begingroup/BegingroupMethods.js');
const {BegingroupStack,begingroupStack}=load('input/tex/begingroup/BegingroupStack.js');
const {MapHandler}=load('input/tex/MapHandler.js');
const TeX=load('input/tex.js').TeX, MathItem=load('core/MathItem.js').AbstractMathItem;
const {MmlFactory}=load('core/MmlTree/MmlFactory.js');
function plain(v,seen=new WeakSet()){
 if(v===undefined)return {$:'undefined'};if(v===null)return null;
 if(typeof v==='number')return Number.isNaN(v)?{$:'NaN'}:Object.is(v,-0)?{$:'-0'}:v;
 if(typeof v==='symbol')return {$:'symbol',description:v.description||''};
 if(typeof v==='function')return {$:'function',name:v.name||'',length:v.length};
 if(typeof v!=='object')return v;if(seen.has(v))return {$:'cycle'};seen.add(v);
 if(Array.isArray(v))return {$:'array',items:Array.from({length:v.length},(_,i)=>Object.hasOwn(v,i)?plain(v[i],seen):{$:'hole'})};
 if(v instanceof Map)return {$:'map',entries:[...v].map(([k,x])=>[plain(k,seen),plain(x,seen)])};
 const out={};for(const key of Reflect.ownKeys(v))out[typeof key==='symbol'?`symbol:${key.description||''}`:key]=plain(v[key],seen);return out;
}
function node(n,seen=new WeakSet()){
 if(!n)return null;if(seen.has(n))return {$:'cycle'};seen.add(n);
 return {kind:n.kind,text:n.getText?.()??null,attrs:plain(n.attributes?.getAllAttributes?.()??{}),properties:plain(n.getAllProperties?.()??{}),children:(n.childNodes||[]).map(x=>node(x,seen))};
}
function error(fn){try{return {result:fn(),error:null};}catch(e){return {result:null,error:{name:e.name||e.constructor.name,id:e.id??null,message:String(e.message)}};}}
const rows=[];
function fresh(){const jax=new TeX({packages:['base','begingroup','newcommand']});jax.setMmlFactory(new MmlFactory());return jax;}
function compile(jax,input){const item=new MathItem(input,jax,true);return node(jax.compile(item,{}));}
function record(name,fn){rows.push({name,...error(fn)});}
function stackSnapshot(jax){const stack=begingroupStack(jax.parseOptions);return {i:stack.i,top:stack.top,base:stack.base,marker:plain(stack.MARKER),globalNames:Object.keys(stack.global),globalMaps:Object.fromEntries(Object.entries(stack.global).map(([k,v])=>[k,{name:v.name,identity:v===MapHandler.getMap(k)}]))};}
record('source-class-and-method-metadata',()=>({
 stack:{name:BegingroupStack.name,length:BegingroupStack.length,prototype:Object.getOwnPropertyNames(BegingroupStack.prototype),handlerConfig:plain(BegingroupStack.handlerConfig),handlerMap:plain(BegingroupStack.handlerMap)},
 methods:Object.fromEntries(Object.entries(BegingroupMethods).map(([k,v])=>[k,{name:v.name||'',length:v.length,identity:k==='macro'?v===load('input/tex/base/BaseMethods.js').default.Macro:undefined}])),
 configuration:{name:BegingroupConfiguration.name,handler:plain(BegingroupConfiguration.handler),options:plain(BegingroupConfiguration.options),preprocessors:BegingroupConfiguration.preprocessors.map(x=>({name:x.name,length:x.length})),postprocessors:BegingroupConfiguration.postprocessors.map(x=>({name:x.name,length:x.length}))}
}));
for(const [name,input] of [
 ['plain-package','x+1'],
 ['nested-local-shadow',String.raw`\newcommand{\foo}{z}\begingroup\renewcommand{\foo}{x}\begingroup\renewcommand{\foo}{y}\foo\endgroup\foo\endgroup\foo`],
 ['global-def-inside-group',String.raw`\begingroup\global\def\foo{x}\endgroup\foo`],
 ['global-newcommand-inside-group',String.raw`\begingroup\global\newcommand{\foo}{x}\endgroup\foo`],
 ['global-forbidden-command',String.raw`\global\newcommand{\foo}{x}`],
 ['local-delimiter-restoration',String.raw`\begingroup\newcommand{\foo}{x}\endgroup\foo`],
 ['global-delimiter-definition',String.raw`\begingroup\global\def\foo{x}\endgroup\foo`],
 ['unmatched-endgroup',String.raw`\endgroup`],
 ['reset-clears-open-groups',String.raw`\begingroup\newcommand{\foo}{x}\begingroupReset\foo`],
 ['sandbox-baseline-and-state',String.raw`\begingroupSandbox\newcommand{\foo}{x}\endgroup\foo`],
 ['retry-cleanup-open-group',String.raw`\begingroup\newcommand{\foo}{x}`],
]){
 record(name,()=>{const jax=fresh();const tree=compile(jax,input);return {tree,stack:stackSnapshot(jax),packageData:plain(jax.parseOptions.packageData)};});
}
record('repeated-formulas-share-real-stack',()=>{
 const jax=fresh(),inputs=[String.raw`\begingroup\newcommand{\foo}{x}`,String.raw`\foo`,String.raw`\endgroup`],out=[];
 for(const input of inputs){out.push({input,tree:compile(jax,input),stack:stackSnapshot(jax)});}
 return out;
});
const metadata={upstream:{version:'4.1.3',commit:'fb987178c2d279a99b0db5ea02e751691703955e'},
 sources:Object.entries(sources).map(([name,sha256])=>({source:`ts/input/tex/begingroup/${name}`,sha256,cjs:`${directory}/${name.replace(/\.ts$/,'.js')}`,cjs_sha256:cjs[name.replace(/\.ts$/,'.js')]})),cases:rows};
const out=path.join(root,'tools/math-port/begingroup-reference/begingroup-source.json');
fs.mkdirSync(path.dirname(out),{recursive:true});fs.writeFileSync(out,JSON.stringify(metadata,null,2)+'\n');
console.log(JSON.stringify({sources:Object.keys(sources).length,cases:rows.length,output:path.relative(root,out)}));
