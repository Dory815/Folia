#!/usr/bin/env node
'use strict';

// Compare a standalone native candidate dump with the pinned original oracle.
// Usage: node tools/math-port/compare-font-data.cjs <native-json>
const fs=require('node:fs');
const path=require('node:path');
const crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..');
const original=JSON.parse(fs.readFileSync(path.join(root,'build/math-port/font-original.json'),'utf8'));
const candidatePath=process.argv[2];
if(!candidatePath)throw new Error('usage: compare-font-data.cjs <native-json>');
const candidate=JSON.parse(fs.readFileSync(path.resolve(candidatePath),'utf8'));
const normalize=(value)=>{
  if(value===undefined)return {$:'undefined'};
  if(value===null||typeof value==='string'||typeof value==='boolean')return value;
  if(typeof value==='number'){
    if(Number.isNaN(value))return {$:'NaN'};
    if(!Number.isFinite(value))return {$:value<0?'-Infinity':'Infinity'};
    if(Object.is(value,-0))return {$:'-0'};
    return value;
  }
  if(Array.isArray(value))return Array.from({length:value.length},(_,i)=>Object.hasOwn(value,i)?normalize(value[i]):{$:'hole'});
  const out={};for(const key of Object.keys(value).sort())out[key]=normalize(value[key]);return out;
};
const digest=(value)=>crypto.createHash('sha256').update(JSON.stringify(normalize(value))).digest('hex');
const equal=(label,got,want)=>{
  if(JSON.stringify(normalize(got))!==JSON.stringify(normalize(want))){
    console.error(`${label}: ${JSON.stringify(normalize(got))} != ${JSON.stringify(normalize(want))}`);failures++;return false;
  }
  return true;
};
const compareClassState=(label,state,want)=>{
  if(!state){console.error(`${label}: missing native state`);failures++;return;}
  const variants=Object.keys(state.chars||{});
  equal(`${label} variantNames`,variants,want.variantNames);
  equal(`${label} glyphCount`,Object.values(state.chars||{}).reduce((n,map)=>n+Object.keys(map).length,0),want.glyphCount);
  equal(`${label} glyphTableSha256`,digest(state.chars||{}),want.glyphTableSha256);
  equal(`${label} delimiterCount`,Object.keys(state.delimiters||{}).length,want.delimiters);
  equal(`${label} delimiterTableSha256`,digest(state.delimiters||{}),want.delimiterTableSha256);
  for(const key of ['cacheIds','options','variants','cssFonts','variantSmp','accentMap','moMap','mnMap','sizeVariants','stretchVariants','parameters','dynamicFiles','dynamicExtensions']){
    if(Object.hasOwn(want,key))equal(`${label} ${key}`,state[key],want[key]);
  }
};
const nativeByName=new Map((candidate.tables||[]).map(table=>[table.name,table]));
let failures=0;
for(const source of original.results){
  const actual=nativeByName.get(source.package);
  if(!actual){console.error(`missing native source ${source.package}`);failures++;continue;}
  const variants=Object.keys(actual.chars||{});
  const glyphCount=Object.values(actual.chars||{}).reduce((n,map)=>n+Object.keys(map).length,0);
  const delimiterCount=Object.keys(actual.delimiters||{}).length;
  const checks=[
    ['variantCount',variants.length,source.variantCount],
    ['glyphCount',glyphCount,source.glyphCount],
    ['delimiterCount',delimiterCount,source.delimiterCount],
    ['variantNames',variants,source.variantNames],
    ['glyphTableSha256',digest(actual.chars||{}),source.glyphTableSha256],
    ['delimiterTableSha256',digest(actual.delimiters||{}),source.delimiterTableSha256],
  ];
  for(const [label,got,want] of checks){
    if(JSON.stringify(got)!==JSON.stringify(want)){console.error(`${source.package} ${label}: ${JSON.stringify(got)} != ${JSON.stringify(want)}`);failures++;}
  }
}
if(candidate.tables?.length!==original.results.length){console.error(`native table count ${candidate.tables?.length} != ${original.results.length}`);failures++;}
const sourceBase=original.results.find(item=>item.package==='@mathjax/mathjax-newcm-font');
if(sourceBase?.instance){
  if(!candidate.instance){console.error('missing native NewCM instance resolver observations');failures++;}
  else equal('NewCM instance resolver observations',candidate.instance,sourceBase.instance);
}
const sourceMerges=sourceBase?.classExtensionMerges;
if(sourceMerges){
  const nativeMerges=candidate.classExtensionMerges;
  if(!nativeMerges){console.error('missing native class extension merge observations');failures++;}
  else {
    equal('class extension merge count',nativeMerges.length,sourceMerges.length);
    for(let i=0;i<Math.min(nativeMerges.length,sourceMerges.length);i++){
      const got=nativeMerges[i],want=sourceMerges[i];
      equal(`class extension merge[${i}] name`,got.name,want.name);
      equal(`class extension merge[${i}] returnValue`,got.returnValue,want.returnValue);
      if(Object.hasOwn(want,'before'))compareClassState(`class extension merge[${i}] before`,got.before,want.before);
      compareClassState(`class extension merge[${i}] state`,got.state,want.state);
    }
  }
}
if(sourceBase?.samples){
  if(!candidate.samples){console.error('missing native NewCM resolver sample observations');failures++;}
  else equal('NewCM resolver sample observations',candidate.samples,sourceBase.samples);
}
const sourceDynamic=original.results[0]?.dynamicTables;
if(sourceDynamic){
  const nativeDynamic=candidate.dynamic;
  if(!nativeDynamic){console.error('missing native synchronous dynamic-table load');failures++;}
  else {
    const variants=Object.keys(nativeDynamic.chars||{}),sourceVariants=Object.keys(sourceDynamic.chars||{});
    const nativeCount=Object.values(nativeDynamic.chars||{}).reduce((n,map)=>n+Object.keys(map).length,0);
    const sourceCount=Object.values(sourceDynamic.chars||{}).reduce((n,map)=>n+Object.keys(map).length,0);
    for(const [label,got,want] of [
      ['dynamic variant order',variants,sourceVariants],
      ['dynamic glyph count',nativeCount,sourceCount],
      ['dynamic per-variant full-table digest',digest(nativeDynamic.chars||{}),digest(sourceDynamic.chars||{})],
      ['dynamic delimiter full-table digest',digest(nativeDynamic.delimiters||{}),digest(sourceDynamic.delimiters||{})],
    ])if(JSON.stringify(got)!==JSON.stringify(want)){console.error(`${label}: ${JSON.stringify(got)} != ${JSON.stringify(want)}`);failures++;}
  }
}
if(failures)process.exitCode=1;
else console.log(`full font tables match pinned MathJax 4.1.3: ${original.results.length} static sources plus every synchronous dynamic glyph, option record and delimiter`);
