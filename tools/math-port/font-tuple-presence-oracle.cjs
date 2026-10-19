#!/usr/bin/env node
'use strict';

// Supplemental source observations for tuple presence and identity. This does
// not modify or regenerate the frozen full-table/lifecycle golds.
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'../..');
const packageRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax');
const {FontData}=require(path.join(packageRoot,'src/cjs/output/common/FontData.js'));
const {SvgFontData,AddPaths}=require(path.join(packageRoot,'src/cjs/output/svg/FontData.js'));
const {MathJaxNewcmFont:Newcm}=require(path.join(packageRoot,'mathjax-newcm-font/cjs/svg.js'));
const encode=(x)=>x===undefined?{$:'undefined'}:x===null?null:Array.isArray(x)?Array.from({length:x.length},(_,i)=>Object.hasOwn(x,i)?encode(x[i]):{$:'hole'}):typeof x==='object'?Object.fromEntries(Object.keys(x).sort().map(k=>[k,encode(x[k])])):typeof x==='number'&&!Number.isFinite(x)?{$:Number.isNaN(x)?'NaN':x<0?'-Infinity':'Infinity'}:x;
const observe=(f)=>{try{return {value:encode(f())};}catch(e){return {error:{name:e?.name||typeof e,message:e?.message||String(e)}};}};
const results={};

for(const [name,tuple] of [
  ['length0',[]],['length2',[1,0]],['length3',[1,0,.5]],
  ['length4Undefined',[1,0,.5,undefined]],['length4Null',[1,0,.5,null]],
  ['length4Object',[1,0,.5,{keep:'same'}]],['length5',[1,0,.5,{keep:'same'},'extra']],
]){
  const table={0x41:tuple};
  const before=tuple[3];
  const option=observe(()=>FontData.charOptions(table,0x41));
  results[name]={tuple:encode(tuple),option,
    fourthSame:tuple.length>3?tuple[3]===before:null};
}

for(const [name,tuple] of [
  ['object',[1,0,.5,{keep:'old'}]],
  ['undefined',[1,0,.5,undefined]],
  ['null',[1,0,.5,null]],
]){
  const map={0x41:tuple};
  const tupleRef=map[0x41], fourthRef=tuple.length>3?tuple[3]:undefined;
  results['addPaths_'+name]=observe(()=>{
    const returned=AddPaths(map,{0x41:'M65'},{0x41:'A'});
    return {sameMap:returned===map,sameTuple:map[0x41]===tupleRef,
      sameFourth:tuple.length>3?tuple[3]===fourthRef:null,tuple:encode(tuple)};
  });
}

{
  const font=new Newcm();
  const a=font.getChar('normal',65),b=font.getChar('normal',65);
  results.repeatedGetChar={sameTuple:a===b,tuple:encode(a)};
}

const destination=path.join(root,'build/math-port/font-tuple-presence-original.json');
fs.mkdirSync(path.dirname(destination),{recursive:true});
fs.writeFileSync(destination,JSON.stringify({source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e',results},null,2)+'\n');
console.log(`${path.relative(root,destination)}: supplemental FontData tuple-presence observations`);
