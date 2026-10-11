// Root reproducible successor; protected originals and predecessor remain immutable.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),prior=path.join(root,'build/math-port-regexp-lookahead-source'),copy=path.join(root,'build/math-port-regexp-unicode-escape-source');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
cp.execFileSync(process.execPath,[path.join(__dirname,'regexp-lookahead-prepare.cjs')]);
assert.equal(hash(fs.readFileSync(path.join(prior,'src/parse.rs'),'utf8').replaceAll('\r\n','\n')),'3c2a911d8b65389d1a657f371784777393b2d374f7b4de4b0c3c5f2f31f2967e');
const source=path.join(root,'.reference/regress'),names=cp.execFileSync('git',['-C',source,'ls-files','-z'],{encoding:'utf8'}).split('\0').filter(Boolean);
function prepare(dest){
 if(!fs.existsSync(dest)){
  for(const name of [...names,'src/legacy_case.rs','Cargo.lock']){const file=path.resolve(dest,name);assert.ok(file.startsWith(dest+path.sep));fs.mkdirSync(path.dirname(file),{recursive:true});fs.copyFileSync(path.join(prior,name),file);}
  cp.execFileSync('git',['apply','-p3','--directory='+path.relative(root,dest).split(path.sep).join('/'),path.join(__dirname,'regexp-unicode-escape-experiment/parse.rs.patch')],{cwd:root,stdio:'pipe'});
 }
 assert.equal(hash(fs.readFileSync(path.join(dest,'src/parse.rs'),'utf8').replaceAll('\r\n','\n')),'34a075eaf65c01f5c4f3fbe527018f7573cf3dc034072b419bcab82abcbd864a');
 for(const name of [...names,'src/legacy_case.rs','Cargo.lock'].filter(n=>n!=='src/parse.rs'))assert.ok(fs.readFileSync(path.join(dest,name)).equals(fs.readFileSync(path.join(prior,name))),'Protected boundary '+name);
}
prepare(copy);if(process.argv.includes('--fresh-check'))prepare(path.join(root,'build/math-port-regexp-unicode-escape-fresh-check'));
console.log('Unicode escape grammar copy and predecessor verified; existing files never reset.');
