// Root read-only classification of complete independent Source observations.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),crypto=require('node:crypto'),{isDeepStrictEqual:eq}=require('node:util');
const root=path.resolve(__dirname,'../..'),reports=[],unicode16=process.argv.includes('--unicode16'),dataOnly=process.argv.includes('--data');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
for(const [name,file,sha] of (dataOnly?[['properties','build/math-port-unicode16/node-reference.json','03d1dd3cc23955f107d1a2265f04720497743bdb30b83d5ea804ba8adbb8094e']]:[
 ['original','build/math-port-regexp-candidate/node-reference.json','5b03d68995413da826ebb481d5e8057fa1f248eaf240913ffeacc7087e1d1195'],
 ['legacy','build/math-port-regexp-legacy-oracle/node-reference.json','ac9fbadad5d6da06de36de3458df1912bdebc9bf2d0810013d5c6edada7d7376'],
 ['lookahead','build/math-port-regexp-lookahead/node-reference.json','9bedc9186f90a3f4eddfcd01fbdbe8cee6a097cc3d229639743d8cb1a801323a'],
 ['unicode_escape','build/math-port-regexp-unicode-escape/node-reference.json','cf090d64ed483a82a37740784a3c97b388c1a7c75335d9a6da8e82deced67bc6']])){
 const bytes=fs.readFileSync(path.join(root,file));assert.equal(hash(bytes),sha);const cases=JSON.parse(bytes),values=dataOnly?Array.from({length:Math.ceil(cases.length/16384)},(_,i)=>JSON.parse(fs.readFileSync(path.join(root,'build/math-port-regexp-unicode16-source-harness/properties-'+(i*16384)+'-unicode16-native-results.json')))).flat():JSON.parse(fs.readFileSync(path.join(root,'build/math-port-regexp-'+(unicode16?'unicode16':'unicode-escape')+'-source-harness/'+name+'-native-results.json')));assert.equal(values.length,cases.length);
 const patterns=new Map();let different=0,constructorDifferent=0;
 for(let i=0;i<cases.length;i++){const c=cases[i],{diagnostic,...native}=values[i].v_implies_unicode;if(eq(native,c.expected))continue;++different;const constructorDiff=native.constructor!==c.expected.constructor;if(constructorDiff)++constructorDifferent;const pattern=String.fromCharCode(...c.pattern),key=JSON.stringify([pattern,c.flags]),entry=patterns.get(key)||{pattern,flags:c.flags,count:0,constructor_differences:0,samples:[]};entry.count++;if(constructorDiff)entry.constructor_differences++;if(entry.samples.length<2)entry.samples.push({text:c.text,start:c.start,expected:c.expected,native,diagnostic});patterns.set(key,entry);}
 reports.push({corpus:name,source_case_sha256:sha,cases:cases.length,different,constructor_differences:constructorDifferent,distinct_pattern_flags:patterns.size,patterns:[...patterns.values()].sort((a,b)=>b.count-a.count||a.pattern.localeCompare(b.pattern)||a.flags.localeCompare(b.flags))});
}
const report={stage:'root remaining-difference inventory; no engine or host accepted',adapter:'v_implies_unicode',reports},file=path.join(root,'docs/math-port/evidence/'+(dataOnly?'p21c-data':unicode16?'p21c':'p21b')+'-regexp-remaining-patterns.json');
if(process.argv.includes('--check'))assert.deepEqual(JSON.parse(fs.readFileSync(file)),JSON.parse(JSON.stringify(report)));else fs.writeFileSync(file,JSON.stringify(report,null,2)+'\n');
console.log(JSON.stringify(reports.map(({patterns,...summary})=>summary)));
