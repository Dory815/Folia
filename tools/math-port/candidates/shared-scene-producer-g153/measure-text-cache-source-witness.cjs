'use strict';
// Pinned Source Map capture order, nested cache writes, and SVG reset behavior.
const assert=require('node:assert/strict');
const crypto=require('node:crypto');
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'../../../..');
const output=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/output');
const hashes={
  'common.js':'b8a58d983e465f21ed36574472140bfd65e2696bb72f6c5647e0a2ae47eb1bd9',
  'svg.js':'1495e23a9da0bf6b0e8b44d71f54271fa12e902c11444e4f18cdd749ad3b800e'
};
for(const [file,expected] of Object.entries(hashes)){
  const actual=crypto.createHash('sha256').update(fs.readFileSync(path.join(output,file))).digest('hex');
  assert.equal(actual,expected,`pinned CJS hash mismatch: ${file}`);
}
const {CommonOutputJax}=require(path.join(output,'common.js'));
const {SVG}=require(path.join(output,'svg.js'));
const measure=CommonOutputJax.prototype.measureTextNodeWithCache;
const reset=SVG.prototype.reset;
const events=[];
const owner={
  unknownCache:new Map(),
  measureTextNode(callback){events.push('measure');return callback();},
  clearFontCache(){events.push('clear-font-cache');}
};
const chars='same-key witness';
const variant='-same-key-witness';
let capturedMap;
let nestedResult;
const outerResult=Reflect.apply(measure,owner,[()=>{
  capturedMap=owner.unknownCache.get(variant);
  assert.ok(capturedMap instanceof Map,'Source creates the variant Map before measuring');
  nestedResult=Reflect.apply(measure,owner,[()=>({w:3,h:.75,d:.2}),chars,variant]);
  const other=Reflect.apply(measure,owner,[()=>({w:5,h:.75,d:.2}),'other text','-other-variant']);
  assert.equal(other.w,5);
  Reflect.apply(reset,owner,[]);
  assert.equal(owner.unknownCache.get(variant),capturedMap,
    'SVG reset clears font cache but retains the captured Common variant Map');
  return {w:7,h:.75,d:.2};
},chars,variant]);
const afterReset=Reflect.apply(measure,owner,[()=>{throw Error('cached Source result should be returned');},chars,variant]);
assert.equal(nestedResult.w,3);
assert.equal(outerResult.w,7,'outer Map.set follows and replaces nested same-key Map.set');
assert.equal(afterReset.w,7,'public SVG reset does not clear Common unknown-text cache');
assert.deepEqual(events,['measure','measure','measure','clear-font-cache']);
process.stdout.write(JSON.stringify({capturedBeforeMeasure:true,nestedSameKey:3,outerStored:7,
  afterSvgReset:7,events})+'\n');
