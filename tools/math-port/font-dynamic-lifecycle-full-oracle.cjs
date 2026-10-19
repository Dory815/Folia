#!/usr/bin/env node
'use strict';

// Independent lifecycle trace oracle for the pinned MathJax dynamic-font path.
// A fresh process is required because Font.dynamicFiles and asyncLoad are mutable.
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'../..');
const pkg=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax');
const {MathJaxNewcmFont:Font}=require(path.join(pkg,'mathjax-newcm-font/cjs/svg.js'));
const mathjax=require(path.join(pkg,'src/cjs/mathjax.js')).mathjax;
const normalize=(v)=>v===undefined?{$:'undefined'}:v===null?null:Array.isArray(v)?Array.from({length:v.length},(_,i)=>Object.hasOwn(v,i)?normalize(v[i]):{$:'hole'}):typeof v==='function'?{$:'function'}:typeof v==='object'?Object.fromEntries(Object.keys(v).sort().map(k=>[k,normalize(v[k])])):v;
const scenarios=[];
function definition(name){
  const d={extension:'',file:`__full_lifecycle_${name}__`,variants:{},delimiters:[],failed:false,promise:null,setup:undefined};
  Font.dynamicFiles={...Font.dynamicFiles,[d.file]:d};return d;
}
function withWarn(trace,fn){const before=console.warn;console.warn=(...args)=>trace.push(`warn:${args.map(x=>x instanceof Error?`${x.name}:${x.message}`:String(x)).join('|')}`);try{return fn();}finally{console.warn=before;}}
function err(e){return {name:e?.name||typeof e,message:e?.message||String(e)};}
function outcome(p){return p.then(()=>({state:'fulfilled'}),e=>({state:'rejected',error:err(e)}));}

// Sync success, provider setup and repeat. The trace wraps the exact Source
// setup callback after dynamicSetup installs it; no production behavior is stubbed.
{
  const trace=[],d=definition('sync_success');let loads=0;const oldLoad=mathjax.asyncLoad,oldSync=mathjax.asyncIsSynchronous;
  mathjax.asyncIsSynchronous=true;
  mathjax.asyncLoad=(name)=>{trace.push(`load:${name}`);loads++;Font.dynamicSetup('',d.file,{normal:{0xE000:[1,0,.5,{p:'M0 0'}]}});const setup=d.setup;d.setup=(font)=>{trace.push('setup');return setup(font);};};
  const oldWarn=console.warn;
  try{const font=new Font();const initial=typeof d.setup;let firstError;try{font.loadDynamicFileSync(d);}catch(e){firstError=err(e);}const promise=d.promise;let secondError;try{font.loadDynamicFileSync(d);}catch(e){secondError=err(e);}scenarios.push({name:'sync_success',trace,initialSetup:initial,loads,failed:d.failed,setup:typeof d.setup,promiseRetained:promise===d.promise,firstError:firstError??null,secondError:secondError??null,glyph:normalize(font.getChar('normal',0xE000))});}
  finally{mathjax.asyncLoad=oldLoad;mathjax.asyncIsSynchronous=oldSync;console.warn=oldWarn;}
}

// Sync missing provider, provider throw, and provider-installed setup throw.
for(const [name,provider] of [
  ['sync_missing',undefined],
  ['sync_provider_throw',()=>{throw new Error('provider boom');}],
  ['sync_setup_throw',(name,d)=>{d.setup=()=>{throw new Error('setup boom');};}],
]){
  const trace=[],d=definition(name),oldLoad=mathjax.asyncLoad,oldSync=mathjax.asyncIsSynchronous;
  mathjax.asyncIsSynchronous=true;mathjax.asyncLoad=typeof provider==='function'?((filename)=>{trace.push(`load:${filename}`);return provider(filename,d);}):provider;
  let warningRestore;const warn=console.warn;console.warn=(...args)=>trace.push(`warn:${args.map(x=>x instanceof Error?`${x.name}:${x.message}`:String(x)).join('|')}`);
  try{const font=new Font();const initialSetup=typeof d.setup;let firstError;try{font.loadDynamicFileSync(d);}catch(e){firstError=err(e);}const promise=d.promise;let secondError;try{font.loadDynamicFileSync(d);}catch(e){secondError=err(e);}scenarios.push({name,trace,initialSetup,loads:trace.filter(x=>x.startsWith('load:')).length,failed:d.failed,setup:typeof d.setup,promiseRetained:promise===d.promise,firstError:firstError??null,secondError:secondError??null});}
  finally{mathjax.asyncLoad=oldLoad;mathjax.asyncIsSynchronous=oldSync;console.warn=warn;}
}

async function asyncCase(name, provider){
  const trace=[],d=definition(name),oldLoad=mathjax.asyncLoad,oldWarn=console.warn;let loads=0;
  mathjax.asyncLoad=provider===null?undefined:(filename)=>{trace.push(`load:${filename}`);loads++;return provider(filename,d,trace);};
  console.warn=(...args)=>trace.push(`warn:${args.map(x=>x instanceof Error?`${x.name}:${x.message}`:String(x)).join('|')}`);
  try{
    const font=new Font();const initialSetup=typeof d.setup;
    const first=font.loadDynamicFile(d),stored=d.promise;const firstResult=await outcome(first);const storedResult=await outcome(stored);
    const second=font.loadDynamicFile(d),secondResult=await outcome(second);
    scenarios.push({name,trace,initialSetup,loads,failed:d.failed,setup:typeof d.setup,promiseRetained:stored===d.promise,storedResult,firstResult,secondResult});
  }finally{mathjax.asyncLoad=oldLoad;console.warn=oldWarn;}
}
async function main(){
  await asyncCase('async_success',(filename,d,trace)=>Promise.resolve().then(()=>{Font.dynamicSetup('',d.file,{normal:{0xE001:[1,0,.5,{p:'M1 1'}]}});const setup=d.setup;d.setup=(font)=>{trace.push('setup');return setup(font);};}));
  await asyncCase('async_missing',null);
  await asyncCase('async_provider_reject',()=>Promise.reject(new Error('async provider boom')));
  await asyncCase('async_provider_sync_throw',()=>{throw new Error('sync throw from asyncLoad');});
  await asyncCase('async_setup_throw',(_filename,d,trace)=>{d.setup=()=>{trace.push('setup');throw new Error('async setup boom');};return Promise.resolve();});
  const output={source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e',scenarios};
  const dest=path.join(root,'build/math-port/font-dynamic-lifecycle-full-original.json');fs.mkdirSync(path.dirname(dest),{recursive:true});fs.writeFileSync(dest,JSON.stringify(output,null,2)+'\n');
  console.log(`${path.relative(root,dest)}: ${scenarios.length} full lifecycle traces`);
}
main().catch(e=>{console.error(e);process.exitCode=1;});
