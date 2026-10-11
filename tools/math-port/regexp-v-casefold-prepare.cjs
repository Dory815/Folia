// Root reproducible two-file successor; original and predecessor never reset.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),prior=path.join(root,'build/math-port-regexp-unicode16-source'),copy=path.join(root,'build/math-port-regexp-v-casefold-source');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex'),read=f=>fs.readFileSync(f);
cp.execFileSync(process.execPath,[path.join(__dirname,'regexp-unicode16-prepare.cjs'),'--check']);
const source=path.join(root,'.reference/regress'),names=cp.execFileSync('git',['-C',source,'ls-files','-z'],{encoding:'utf8'}).split('\0').filter(Boolean),all=[...names,'src/legacy_case.rs','Cargo.lock'];
// Patches and these successor pins use LF; replay also records raw file hashes.
const pins={'src/parse.rs':'db9a0588975f072671dfecbe309b156bb480dff6bb842e495b5c6b5a42c3e54a','src/unicode.rs':'27a4c473cbbf7990d55f2a8aedc06dc845d0bd165b87b3c3ea6370eaf67b3892'},protectedBytes=new Map();
for(const name of all)protectedBytes.set(path.join(prior,name),read(path.join(prior,name)));
function prepare(dest){
 if(!fs.existsSync(dest)){
  for(const name of all){const file=path.resolve(dest,name);assert.ok(file.startsWith(dest+path.sep));fs.mkdirSync(path.dirname(file),{recursive:true});fs.copyFileSync(path.join(prior,name),file);}
  for(const name of Object.keys(pins))cp.execFileSync('git',['apply','-p3','--directory='+path.relative(root,dest).split(path.sep).join('/'),path.join(__dirname,'regexp-v-casefold-experiment',path.basename(name)+'.patch')],{cwd:root,stdio:'pipe'});
 }
 for(const name of all){const bytes=read(path.join(dest,name));if(pins[name])assert.equal(hash(bytes.toString('utf8').replaceAll('\r\n','\n')),pins[name],name+' successor identity');else assert.ok(bytes.equals(read(path.join(prior,name))),'Unchanged boundary '+name);}
}
prepare(copy);if(process.argv.includes('--fresh-check'))prepare(path.join(root,'build/math-port-regexp-v-casefold-fresh-check'));
for(const[file,bytes]of protectedBytes)assert.ok(bytes.equals(read(file)),'Protected predecessor '+file);
console.log('v point-set preparation successor reproduced; predecessor and all other source files unchanged.');
