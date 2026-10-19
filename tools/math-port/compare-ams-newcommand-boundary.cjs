'use strict';
const fs=require('node:fs'),assert=require('node:assert/strict');
const source=JSON.parse(fs.readFileSync(process.argv[2]||'tools/math-port/ams-newcommand-reference/ams-newcommand-boundary-source.json','utf8'));
const native=JSON.parse(fs.readFileSync(process.argv[3]||'build/math-port/ams-newcommand-boundary-native.json','utf8'));
assert.equal(source.upstream?.commit,'fb987178c2d279a99b0db5ea02e751691703955e','unexpected pinned-source provenance');
assert(native.cases&&typeof native.cases==='object','native candidate has no cases object');
const fields=['result','error','dynamicMaps','packageData'];
const expected=source.cases.map(row=>row.name).sort();
assert.deepEqual(Object.keys(native.cases).sort(),expected,'supplemental native case set differs from Source');
for(const row of source.cases){
 const actual=native.cases[row.name];
 for(const field of fields){
  try{assert.deepStrictEqual(actual[field],row[field]);}
  catch(error){error.message=`${row.name}.${field}: ${error.message}`;throw error;}
 }
}
console.log(`AMS/Newcommand supplemental Source differential passed: ${source.cases.length} cases; ${fields.join(', ')}`);
