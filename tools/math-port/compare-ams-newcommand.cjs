'use strict';
// Strictly compares the real native candidate with the independent pinned CJS
// observer. Run the source oracle first; it validates all TS/CJS locks.
const fs=require('node:fs'),assert=require('node:assert/strict');
const sourcePath=process.argv[2]||'tools/math-port/ams-newcommand-reference/ams-newcommand-source.json';
const nativePath=process.argv[3]||'build/math-port/ams-newcommand-native.json';
const source=JSON.parse(fs.readFileSync(sourcePath,'utf8'));
const native=JSON.parse(fs.readFileSync(nativePath,'utf8'));
assert.equal(source.upstream?.commit,'fb987178c2d279a99b0db5ea02e751691703955e','unexpected source oracle provenance');
assert(native.cases&&typeof native.cases==='object','native candidate has no cases object');
const fields=['result','error','dynamicMaps','packageData'];
const expectedNames=source.cases.map(x=>x.name).sort();
assert.deepEqual(Object.keys(native.cases).sort(),expectedNames,'native case set differs from locked Source cases');
for(const row of source.cases){
  const actual=native.cases[row.name];
  for(const field of fields){
    try{assert.deepStrictEqual(actual[field],row[field]);}
    catch(error){error.message=`${row.name}.${field}: ${error.message}`;throw error;}
  }
}
console.log(`AMS/Newcommand Source differential passed: ${source.cases.length} complete cases; ${fields.join(', ')}`);
