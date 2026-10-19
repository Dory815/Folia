#!/usr/bin/env node
'use strict';

// Source witness for the three distinct FontData mergeOptions array forms.
// It calls the pinned MathJax Options implementation directly and records
// alias, hole, and value behavior without substituting a native algorithm.
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'../..');
const bundle=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax');
const {mergeOptions}=require(path.join(bundle,'src/cjs/output/common/FontData.js'));
const normalize=value=>{
  if(value===undefined)return {$:'undefined'};
  if(value===null||typeof value==='string'||typeof value==='boolean')return value;
  if(typeof value==='number')return Object.is(value,-0)?{$:'-0'}:Number.isNaN(value)?{$:'NaN'}:value;
  if(Array.isArray(value))return Array.from({length:value.length},(_,i)=>
    Object.hasOwn(value,i)?normalize(value[i]):{$:'hole'});
  return Object.fromEntries(Object.keys(value).sort().map(key=>[key,normalize(value[key])]));
};
const rows=[];
function observe(name,base,patch,retain){
  const object={items:base},old=object.items;
  mergeOptions(object,'items',patch);
  const result=object.items;
  rows.push({name,before:normalize(old),result:normalize(result),sameArray:result===old,
    oldAfter:normalize(old),...retain(result,old)});
}
const marker={value:1},other={value:2};
// Falsy src takes mergeOptions' direct obj[dst] branch. A getter witness
// distinguishes it from calling defaultOptions with an empty source record.
for(const [name,source] of [['undefined-source',undefined],['null-source',null],
  ['false-source',false],['zero-source',0],['empty-string-source','']]){
  let reads=0;
  const object={get items(){reads++;return marker;},set items(_value){reads++;}};
  const result=mergeOptions(object,'items',source);
  rows.push({name,result:normalize(result),sameValue:result===marker,propertyReads:reads});
}
observe('empty-patch-keeps-array-identity',[marker],{},(result,old)=>({sameNested:result[0]===marker,oldSame:old[0]===marker}));
observe('empty-append-copies-array-but-keeps-elements',[marker],{'[+]':[]},(result,old)=>({sameNested:result[0]===marker,oldSame:old[0]===marker}));
observe('empty-remove-filters-to-fresh-dense-array',[marker],{'[-]':[]},(result,old)=>({sameNested:result[0]===marker,oldSame:old[0]===marker}));
observe('remove-uses-source-object-identity',[marker,other],{'[-]':[marker]},(result,old)=>({keptOther:result[0]===other,removedMarker:!result.includes(marker),oldLength:old.length}));
const nested={keep:marker,drop:other};
observe('ordinary-object-source-merges-in-place',[nested],{keep:{added:1}},(result,old)=>({sameArray:result===old,sameNested:result[0]===nested,keepIdentity:nested.keep===marker,added:nested.keep.added}));
observe('remove-runs-before-append',[marker,other],{'[+]':[marker],'[-]':[marker]},(result,old)=>({keptOther:result[0]===other,appendedMarker:result[1]===marker,oldLength:old.length}));
const sparse=[];sparse.length=3;sparse[2]=other;
observe('append-spreads-sparse-array-as-undefined',[marker],{'[+]':sparse},(result,old)=>({denseResult:[0,1,2].every(i=>Object.hasOwn(result,i)),sourceStillSparse:!Object.hasOwn(sparse,0)&&Object.hasOwn(sparse,2),oldLength:old.length}));
const replacement=[];replacement.length=3;replacement[1]={nested:[marker]};
observe('plain-array-replacement-copies-own-indices', [other], replacement,(result,old)=>({preservesHole:!Object.hasOwn(result,0)&&!Object.hasOwn(result,2),oldUnchanged:old.length===1&&old[0]===other,
  outerFresh:result!==replacement,nestedFresh:result[1]!==replacement[1],nestedArrayFresh:result[1].nested!==replacement[1].nested,leafSame:result[1].nested[0]===marker}));

const output={source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e',rows};
const destination=path.join(root,'build/math-port/font-array-patch-original.json');
fs.mkdirSync(path.dirname(destination),{recursive:true});
fs.writeFileSync(destination,JSON.stringify(output,null,2)+'\n','utf8');
console.log(`${path.relative(root,destination)}: ${rows.length} pinned Options merge observations`);
