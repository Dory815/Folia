#!/usr/bin/env node
'use strict';

// Source-only witness for raw FontData array-length reads and dynamicExtensions
// replacement. Uses the pinned CJS constructors and records live identities by
// explicit comparisons instead of reducing Maps to a copied value model.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const root = path.resolve(__dirname, '../..');
const pkg = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/mathjax-newcm-font/cjs/svg.js');
const {MathJaxNewcmFont: Font} = require(pkg);
const sourcePath = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/output/common/FontData.js');
const sourceSha256 = crypto.createHash('sha256').update(fs.readFileSync(sourcePath)).digest('hex');

const normalize = value => {
  if (value === undefined) return {$:'undefined'};
  if (value === null || typeof value === 'string' || typeof value === 'boolean') return value;
  if (typeof value === 'number') return Number.isNaN(value) ? {$:'NaN'} : Object.is(value,-0) ? {$:'-0'} : value;
  if (Array.isArray(value)) return Array.from({length:value.length},(_,i)=>
    Object.hasOwn(value,i) ? normalize(value[i]) : {$:'hole'});
  if (value instanceof Map) return [...value].map(([k,v])=>[normalize(k),normalize(v)]);
  return Object.fromEntries(Object.keys(value).sort().map(k=>[k,normalize(value[k])]));
};
const events = [];
const record = (target, mapOwner, name) => {
  const originalDescriptor = Object.getOwnPropertyDescriptor(mapOwner,'dynamicExtensions');
  const old = mapOwner.dynamicExtensions;
  const marker = {name:'old-marker'};
  old.set('old-marker',marker);
  const mapReads=[];
  let activeMap=old;
  Object.defineProperty(mapOwner,'dynamicExtensions',{configurable:true,get(){mapReads.push('get');return activeMap;},
    set(value){mapReads.push('set');activeMap=value;}});
  const replacement = new Map([['old-marker',{name:'old-marker'}]]);
  mapOwner.dynamicExtensions = replacement;
  const before = {oldRetained:old.get('old-marker')===marker, replacementSeedPreserved:replacement.size===1};
  const result = target.addExtension({name,ranges:[]});
  const sourceMapReads=mapReads.slice();
  const stored = activeMap.get(name);
  events.push({name, before, returnValue:normalize(result), replacementIsCurrent:activeMap===replacement,
    oldMapUnchanged:old.get('old-marker')===marker && !old.has(name), newMapHasExtension:stored?.name===name,
    mapEntries:normalize([...replacement.keys()]), mapReads:sourceMapReads, sizeN:normalize(stored?.sizeN), stretchN:normalize(stored?.stretchN)});
  // Restore an ordinary writable data field after the observation. If the
  // original property was inherited (as it is on this class), the preceding
  // explicit replacement correctly creates an own Source data property.
  Object.defineProperty(mapOwner,'dynamicExtensions',originalDescriptor
    ? {...originalDescriptor,value:activeMap}
    : {configurable:true,enumerable:true,writable:true,value:activeMap});
};

// Static Source reads `.length` before it inserts the new extension into the
// live Map, then reads the options during merge. Getter results are deliberately
// not coerced to numbers by this observer.
const originalClassArray = Font.defaultSizeVariants;
const originalStretchArray = Font.defaultStretchVariants;
const originalDynamicExtensions = Font.dynamicExtensions;
const classReads=[];
Object.defineProperty(Font,'defaultSizeVariants',{configurable:true,get(){
  classReads.push('defaultSizeVariants');
  return {get length(){classReads.push('defaultSizeVariants.length');return '4';}};
}});
Object.defineProperty(Font,'defaultStretchVariants',{configurable:true,get(){
  classReads.push('defaultStretchVariants');
  return {get length(){classReads.push('defaultStretchVariants.length');return undefined;}};
}});
record(Font,Font,'raw-class-length');
events.push({name:'static-read-order',reads:classReads});
originalDynamicExtensions.delete('old-marker');
Object.defineProperty(Font,'defaultSizeVariants',{configurable:true,writable:true,value:originalClassArray});
Object.defineProperty(Font,'defaultStretchVariants',{configurable:true,writable:true,value:originalStretchArray});
Font.dynamicExtensions=originalDynamicExtensions;

// Instance Source reads both raw lengths before its Map.set and then runs the
// normal merge operations. The accessor returns distinct tagged values so the
// native side can distinguish raw preservation from eager size_t conversion.
const instance = new Font();
const instanceReads=[];
Object.defineProperty(instance,'sizeVariants',{configurable:true,get(){
  instanceReads.push('sizeVariants');
  return {get length(){instanceReads.push('sizeVariants.length');return undefined;}};
}});
Object.defineProperty(instance,'stretchVariants',{configurable:true,get(){
  instanceReads.push('stretchVariants');
  return {get length(){instanceReads.push('stretchVariants.length');return '6';}};
}});
record(instance,Font,'raw-instance-length');
events.push({name:'instance-read-order',reads:instanceReads});
originalDynamicExtensions.delete('old-marker');

const output={source:'MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e',
  sourcePath:'@mathjax/src/cjs/output/common/FontData.js',sourceSha256,
  constructor:'MathJaxNewcmFont',events};
const destination=path.join(root,'build/math-port/font-data-raw-fields-original.json');
fs.mkdirSync(path.dirname(destination),{recursive:true});
fs.writeFileSync(destination,JSON.stringify(output,null,2)+'\n','utf8');
console.log(`${path.relative(root,destination)}: ${events.length} raw field and Map replacement observations`);
