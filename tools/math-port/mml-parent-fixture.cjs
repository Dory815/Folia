// Root selects unchanged Native tests from the already frozen Source observer.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),source=fs.readFileSync(path.join(root,'tests/math/reference_multiscript.cpp'),'utf8').replaceAll('\r\n','\n');
const starts=[...source.matchAll(/^"(math_port_multiscript_[^"]+)"_test=\[\] \{/gm)],selected=[];
assert.equal(starts.length,226);
for(let j=0;j<starts.length;j++){const row=starts[j];if(!/^math_port_multiscript_(parent_verification|real_parent_partial)_/.test(row[1]))continue;let block=source.slice(row.index,j+1<starts.length?starts[j+1].index:source.lastIndexOf('};'));selected.push(block);}
assert.equal(selected.length,88);
const output=source.slice(0,starts[0].index)+selected.join('')+'};\n',target=path.join(root,'tests/math/reference_mml_verification_parents.inc');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
console.log('88 unchanged Source-derived parent verification tests selected; expectations untouched.');
