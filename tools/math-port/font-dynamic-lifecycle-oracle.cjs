#!/usr/bin/env node
'use strict';

// Pinned-source lifecycle witness for FontData's dynamic setup registration.
// Run in a fresh process because MathJax module setup mutates class data.
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'../..');
const pkg=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax');
const {MathJaxNewcmFont:Font}=require(path.join(pkg,'mathjax-newcm-font/cjs/svg.js'));
const mathjax=require(path.join(pkg,'src/cjs/mathjax.js')).mathjax;
const normalize=(v)=>v===undefined?{$:'undefined'}:v===null?null:Array.isArray(v)?v.map(normalize):typeof v==='function'?{$:'function'}:typeof v==='object'?Object.fromEntries(Object.keys(v).sort().map(k=>[k,normalize(v[k])])):v;
const original={};

function installProbe(fileName){
  const definition={extension:'',file:fileName,variants:{},delimiters:[],failed:false,promise:null,setup:undefined};
  Font.dynamicFiles={...Font.dynamicFiles,[fileName]:definition};
  return definition;
}

// Success: setup is absent until asyncLoad executes the module registration.
{
  const file='__lifecycle_success__';const definition=installProbe(file);const font=new Font();
  const initialSetup=definition.setup;
  const previousSynchronous=mathjax.asyncIsSynchronous,previousLoad=mathjax.asyncLoad;
  const previousWarn=console.warn;const warnings=[];console.warn=(...args)=>warnings.push(args.map(String));
  let loads=0;
  mathjax.asyncIsSynchronous=true;
  mathjax.asyncLoad=()=>{loads++;Font.dynamicSetup('',file,{normal:{0xE000:[1,0,0.5,{p:'M0 0'}]}});};
  try{
    font.loadDynamicFileSync(definition);const promise=definition.promise;const installed=definition.setup;
    font.loadDynamicFileSync(definition);
    original.syncSuccess={initialSetup:normalize(initialSetup),installedSetup:normalize(installed),setupRemainsSame:installed===definition.setup,loadCount:loads,promiseRemainsSame:promise===definition.promise,failed:definition.failed,glyph:normalize(font.getChar('normal',0xE000))};
  }finally{mathjax.asyncIsSynchronous=previousSynchronous;mathjax.asyncLoad=previousLoad;console.warn=previousWarn;}
}

// Failed sync loading catches the provider error, then Source still invokes
// the missing setup callback. Repeated calls retain the resolved promise and
// again attempt that absent callback without retrying the provider.
{
  const file='__lifecycle_failure__';const definition=installProbe(file);const font=new Font();
  const previousSynchronous=mathjax.asyncIsSynchronous,previousLoad=mathjax.asyncLoad;
  const previousWarn=console.warn;const warnings=[];console.warn=(...args)=>warnings.push(args.map(String));
  let loads=0;mathjax.asyncIsSynchronous=true;mathjax.asyncLoad=()=>{loads++;throw new Error('probe loader failure');};
  let firstError,secondError;
  try{
    try{font.loadDynamicFileSync(definition);}catch(error){firstError={name:error.name,message:error.message};}
    const promise=definition.promise;
    let secondCompleted=false;try{font.loadDynamicFileSync(definition);secondCompleted=true;}catch(error){secondError={name:error.name,message:error.message};}
    original.syncFailure={failed:definition.failed,setup:normalize(definition.setup),promiseRetained:promise===definition.promise,loadCount:loads,firstError,secondCompleted,secondError,warnings};
  }finally{mathjax.asyncIsSynchronous=previousSynchronous;mathjax.asyncLoad=previousLoad;console.warn=previousWarn;}
}

async function asynchronousCases(){
  {
    const file='__lifecycle_async_success__';const definition=installProbe(file);const font=new Font();
    const previousLoad=mathjax.asyncLoad;let loads=0,setups=0;
    mathjax.asyncLoad=()=>Promise.resolve().then(()=>{loads++;Font.dynamicSetup('',file,{normal:{0xE001:[1,0,0.5,{p:'M1 1'}]}});});
    try{
      const first=font.loadDynamicFile(definition);const promise=definition.promise;
      await first;const installed=definition.setup;
      await font.loadDynamicFile(definition);
      original.asyncSuccess={initialSetup:'absent',installed:typeof installed,loadCount:loads,promiseRetained:promise===definition.promise,failed:definition.failed,glyph:normalize(font.getChar('normal',0xE001))};
    }finally{mathjax.asyncLoad=previousLoad;}
  }
  {
    const file='__lifecycle_async_failure__';const definition=installProbe(file);const font=new Font();
    const previousLoad=mathjax.asyncLoad,previousWarn=console.warn;let loads=0;const warnings=[];
    mathjax.asyncLoad=()=>{loads++;return Promise.reject(new Error('async probe loader failure'));};
    console.warn=(...args)=>warnings.push(args.map(String));
    const capture=async(promise)=>{try{await promise;return {resolved:true};}catch(error){return {name:error.name,message:error.message};}};
    try{
      const first=font.loadDynamicFile(definition);const statePromise=definition.promise;
      const firstResult=await capture(first);
      const secondResult=await capture(font.loadDynamicFile(definition));
      original.asyncFailure={failed:definition.failed,setup:normalize(definition.setup),promiseRetained:statePromise===definition.promise,loadCount:loads,firstResult,secondResult,warnings};
    }finally{mathjax.asyncLoad=previousLoad;console.warn=previousWarn;}
  }
  const destination=path.join(root,'build/math-port/font-dynamic-lifecycle-original.json');
  fs.mkdirSync(path.dirname(destination),{recursive:true});
  fs.writeFileSync(destination,JSON.stringify({source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e',original},null,2)+'\n');
  console.log(`${path.relative(root,destination)}: sync/async success/failure lifecycle`);
}
asynchronousCases().catch(error=>{console.error(error);process.exitCode=1;});
