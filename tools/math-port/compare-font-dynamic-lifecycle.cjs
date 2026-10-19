#!/usr/bin/env node
'use strict';
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'../..');
const source=JSON.parse(fs.readFileSync(path.join(root,'build/math-port/font-dynamic-lifecycle-full-original.json'),'utf8'));
const file=process.argv[2];if(!file)throw new Error('usage: node tools/math-port/compare-font-dynamic-lifecycle.cjs <native-json>');
const native=JSON.parse(fs.readFileSync(path.resolve(file),'utf8'));
let failures=0;
function firstDiff(a,b,p='$'){
  if(Object.is(a,b))return null;
  if(typeof a!==typeof b||a===null||b===null)return {path:p,source:a,native:b};
  if(Array.isArray(a)||Array.isArray(b)){
    if(!Array.isArray(a)||!Array.isArray(b)||a.length!==b.length)return {path:`${p}.length`,source:Array.isArray(a)?a.length:typeof a,native:Array.isArray(b)?b.length:typeof b};
    for(let i=0;i<a.length;i++){const d=firstDiff(a[i],b[i],`${p}[${i}]`);if(d)return d;}return null;
  }
  if(typeof a==='object'){
    const ak=Object.keys(a).sort(),bk=Object.keys(b).sort();
    if(JSON.stringify(ak)!==JSON.stringify(bk))return {path:`${p}.keys`,source:ak,native:bk};
    for(const k of ak){const d=firstDiff(a[k],b[k],`${p}.${k}`);if(d)return d;}return null;
  }
  return {path:p,source:a,native:b};
}
for(const expected of source.scenarios){
  const actual=(native.scenarios||[]).find(x=>x.name===expected.name);
  if(!actual){console.error(`missing native scenario ${expected.name}`);failures++;continue;}
  const d=firstDiff(expected,actual,expected.name);
  if(d){console.error(`${d.path}: source=${JSON.stringify(d.source)} native=${JSON.stringify(d.native)}`);failures++;}
}
if((native.scenarios||[]).length!==source.scenarios.length){console.error(`scenario count ${native.scenarios?.length} != ${source.scenarios.length}`);failures++;}
if(failures)process.exitCode=1;else console.log(`all ${source.scenarios.length} sync/async dynamic-font lifecycle traces match pinned MathJax`);
