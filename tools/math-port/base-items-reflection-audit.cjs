'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..');
const pkg=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const cjs=path.join(pkg,'cjs');
const source=fs.readFileSync(path.join(root,'.reference/MathJax-src/ts/input/tex/base/BaseItems.ts'));
const expectedTS='0c0d8448f3577be6642c95a412682310362819d2d383ab83b817d2888e4e5f0c';
const expectedCJS='5310d0429b7b62d405ae9b93c8d21ad17b787e5eb3fd1f01439c62f2094f7d1a';
const sourceCjs=fs.readFileSync(path.join(pkg,'cjs/input/tex/base/BaseItems.js'));
if(crypto.createHash('sha256').update(source).digest('hex')!==expectedTS||crypto.createHash('sha256').update(sourceCjs).digest('hex')!==expectedCJS)throw Error('Pinned BaseItems source/CJS hash mismatch');
const load=n=>require(path.join(cjs,n));
const {TeX}=load('input/tex.js'),{MmlFactory}=load('core/MmlTree/MmlFactory.js');
load('input/tex/ams/AmsConfiguration.js');
const classes=load('input/tex/base/BaseItems.js'),Factory=load('input/tex/StackItemFactory.js').default;
const {BaseItem}=load('input/tex/StackItem.js');
const args={StartItem:[()=>({isInner:false})],StopItem:[],OpenItem:[],CloseItem:[],NullItem:[],PrimeItem:[],SubsupItem:[],OverItem:[],LeftItem:[()=> '('],Middle:[()=> '|',()=> 'red'],RightItem:[()=> ')',()=> 'blue'],BreakItem:[()=> 'newline',()=> true],BeginItem:[],EndItem:[],StyleItem:[],PositionItem:[],CellItem:[],MmlItem:[],FnItem:[],NotItem:[],NonscriptItem:[],DotsItem:[],ArrayItem:[],EqnArrayItem:[()=> 'fixture-tag',()=> true,()=> false],EquationItem:[()=> 'fixture-tag'],MstyleItem:[()=>({mathcolor:'red'}),()=> 'fixture-style']};
function value(v,seen=new WeakSet()){
 if(v===undefined)return {tag:'undefined'};if(v===null)return null;
 if(typeof v==='symbol')return {tag:'symbol',description:v.description||''};
 if(typeof v==='function')return {tag:'function',name:v.name||'',length:v.length};
 if(typeof v!=='object')return v;
 if(seen.has(v))return {tag:'cycle'};seen.add(v);
 if(Array.isArray(v))return {tag:'array',length:v.length,items:Array.from({length:v.length},(_,i)=>Object.hasOwn(v,i)?value(v[i],seen):{tag:'hole'})};
 if(v instanceof Map)return {tag:'map',entries:[...v].map(([k,x])=>[value(k,seen),value(x,seen)])};
 return {tag:'object',keys:Reflect.ownKeys(v).map(k=>typeof k==='symbol'?`symbol:${k.description||''}`:String(k))};
}
function descriptor(d){if(!d)return null;return {kind:d.get||d.set?'accessor':'data',enumerable:!!d.enumerable,configurable:!!d.configurable,writable:'writable'in d?!!d.writable:undefined,get:d.get?{name:d.get.name||'',length:d.get.length}:undefined,set:d.set?{name:d.set.name||'',length:d.set.length}:undefined,value:typeof d.value==='function'?{name:d.value.name||'',length:d.value.length}:undefined};}
const rows=[];
for(const [className,Ctor] of Object.entries(classes)){
 const jax=new TeX({packages:['base'],tags:'base'});jax.setMmlFactory(new MmlFactory());
 const factory=jax.parseOptions.itemFactory;
 const instance=new Ctor(factory,...(args[className]||[]).map(f=>f()));
 const ownKeys=Reflect.ownKeys(instance).map(k=>typeof k==='symbol'?`symbol:${k.description||''}`:String(k));
 const ownFields=Object.fromEntries(Reflect.ownKeys(instance).map(k=>[typeof k==='symbol'?`symbol:${k.description||''}`:String(k),value(instance[k])]));
 const ownPrototype=Object.getOwnPropertyNames(Ctor.prototype).map(k=>({key:k,descriptor:descriptor(Object.getOwnPropertyDescriptor(Ctor.prototype,k))}));
 let proto=Ctor.prototype,chain=[];
 while(proto){const p=Object.getOwnPropertyDescriptor(proto,'constructor')?.value;chain.push({name:p?.name||'',ownMembers:Object.getOwnPropertyNames(proto).filter(k=>k!=='constructor')});proto=Object.getPrototypeOf(proto);}
 const errors=Object.hasOwn(Ctor,'errors')?Ctor.errors:undefined;
 rows.push({className,constructor:{name:Ctor.name,length:Ctor.length,prototype:descriptor(Object.getOwnPropertyDescriptor(Ctor,'prototype')),ownKeys:Object.getOwnPropertyNames(Ctor)},prototype:{ownMembers:ownPrototype,sourceChain:chain},instance:{kind:instance.kind,isOpen:instance.isOpen,isClose:instance.isClose,isFinal:instance.isFinal,copyEnv:instance.copyEnv,ownKeys,ownFields},staticErrors:{own:Object.hasOwn(Ctor,'errors'),value:value(errors),prototypeIsBase:!!errors&&Object.getPrototypeOf(errors)===BaseItem.errors}});
}
const output={upstream:{version:'4.1.3',tsSha256:expectedTS,cjsSha256:expectedCJS},count:rows.length,classes:rows};
const target=process.argv[2]||path.join(root,'build/math-port/base-items-reflection-audit.json');fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,JSON.stringify(output,null,2)+'\n');console.log(JSON.stringify({count:rows.length,path:path.relative(root,target)}));
