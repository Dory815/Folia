// Root reproducible ignored Source copy, preserving both previous parser repairs.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),source=path.join(root,'.reference/regress'),copy=path.join(root,'build/math-port-regexp-casefold');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
assert.equal(cp.execFileSync('git',['-C',source,'rev-parse','HEAD'],{encoding:'utf8'}).trim(),'771e6c8fa1e472ef86c45b5206b37fa4e4494e25');
const pins={'src/unicode.rs':['df896bd48e36ba4d08812bc68a378b8e1ad32a069ad67f5cf68bcc2d6ba26e12','4aebcc04392ddf6820b74bc470e3e9bfc7ea7bf486e0b203e3f589136c741635'],'src/parse.rs':['591243258a598d58e9ed9c4998bff3d75004ceb5dc15bb40e2f35b9c2e813833','901843e501c545b8b9c4a479797c3222650919b64f22ad6f3c35f9bf556a219b']};
for(const[name,[before]]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(source,name))),before);
function prepare(copy){
if(!fs.existsSync(copy)){
 const names=cp.execFileSync('git',['-C',source,'ls-files','-z'],{encoding:'utf8'}).split('\0').filter(Boolean);
 for(const name of names){const dest=path.resolve(copy,name);assert.ok(dest.startsWith(copy+path.sep));fs.mkdirSync(path.dirname(dest),{recursive:true});fs.copyFileSync(path.join(source,name),dest);}
 for(const name of ['parse.rs','unicode.rs'])cp.execFileSync('git',['apply','-p3','--directory='+path.relative(root,copy).split(path.sep).join('/'),path.join(__dirname,'regexp-legacy-experiment',name+'.patch')],{cwd:root,stdio:'pipe'});
 const lib=path.join(copy,'src/lib.rs');fs.writeFileSync(lib,fs.readFileSync(lib,'utf8').replace('mod unicode;','mod legacy_case;\nmod unicode;'));
 if(copy===path.join(root,'build/math-port-regexp-casefold'))cp.execFileSync(process.execPath,[path.join(__dirname,'regexp-legacy-data.cjs')]);
 else fs.copyFileSync(path.join(root,'build/math-port-regexp-casefold/src/legacy_case.rs'),path.join(copy,'src/legacy_case.rs'));
 fs.copyFileSync(path.join(__dirname,'regexp-legacy-experiment/upstream.Cargo.lock'),path.join(copy,'Cargo.lock'));
}
for(const[name,[,after]]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(copy,name),'utf8').replaceAll('\r\n','\n')),after);
assert.equal(hash(fs.readFileSync(path.join(copy,'src/lib.rs'))),'1bcbdb8e6890f51bcb56ba3a8431900e1c44124fb215f2d974a3938904431a23');
assert.ok(fs.readFileSync(path.join(copy,'Cargo.lock')).equals(fs.readFileSync(path.join(__dirname,'regexp-legacy-experiment/upstream.Cargo.lock'))));
}
prepare(copy);
cp.execFileSync(process.execPath,[path.join(__dirname,'regexp-legacy-data.cjs'),'--check']);
if(process.argv.includes('--fresh-check'))prepare(path.join(root,'build/math-port-regexp-casefold-fresh-check')); 
console.log('Pinned ignored legacy experiment source/data verified; existing files never reset.');
