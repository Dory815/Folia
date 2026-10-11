// Root copies existing Folia dependency resolutions before local path builds.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),source=fs.readFileSync(path.join(root,'src/app-winui/third_party/usvg-normalizer/Cargo.lock'),'utf8').replaceAll('\r\n','\n');
const blocks=source.split('[[package]]\n'),prefix=blocks.shift();
for(const [file,reference]of [['tools/mermaid-native/reference/Cargo.lock',true],['third_party/mermaid-rs-renderer/Cargo.lock',false]]){
 const output=blocks.flatMap(block=>{
  if(block.startsWith('name = "folia-native-svg"'))return reference?['name = "folia-mermaid-pinned-reference"\nversion = "0.1.0"\ndependencies = [\n "mermaid-rs-renderer",\n "serde_json",\n]\n\n']:[];
  if(block.startsWith('name = "mermaid-rs-renderer"')){assert.ok(block.includes('version = "0.3.1"'));return[block.replace(/^source = .*\n/m,'').replace(/^checksum = .*\n/m,'')];}
  return[block];
 });fs.writeFileSync(path.join(root,file),prefix+output.map(b=>'[[package]]\n'+b).join(''));
}
console.log('Seeded path-package locks from current Folia registry resolutions; run cargo metadata offline once, then locked builds.');
