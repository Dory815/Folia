// Root isolated successor copy; previous experiment and original Source untouched.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),prior=path.join(root,'build/math-port-regexp-casefold'),copy=path.join(root,'build/math-port-regexp-lookahead-source');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
cp.execFileSync(process.execPath,[path.join(__dirname,'regexp-legacy-prepare.cjs')]);
const original=fs.readFileSync(path.join(prior,'src/parse.rs'),'utf8').replaceAll('\r\n','\n');
assert.equal(hash(original),'901843e501c545b8b9c4a479797c3222650919b64f22ad6f3c35f9bf556a219b');
const before=String.raw`                        if !self.try_consume_str("\\u") {
                            return Some(u as u32);
                        }
                        orig_input = self.input.clone();`;
const after=String.raw`                        // A failed low-surrogate trial must restore its escape prefix.
                        orig_input = self.input.clone();
                        if !self.try_consume_str("\\u") {
                            return Some(u as u32);
                        }`;
assert.equal(original.split(before).length,2);
const changed=original.replace(before,after);assert.equal(hash(changed),'3c2a911d8b65389d1a657f371784777393b2d374f7b4de4b0c3c5f2f31f2967e');
if(!fs.existsSync(copy)){
 const source=path.join(root,'.reference/regress'),names=cp.execFileSync('git',['-C',source,'ls-files','-z'],{encoding:'utf8'}).split('\0').filter(Boolean);
 for(const name of names){const dest=path.resolve(copy,name);assert.ok(dest.startsWith(copy+path.sep));fs.mkdirSync(path.dirname(dest),{recursive:true});fs.copyFileSync(path.join(source,name),dest);}
 for(const name of ['src/lib.rs','src/unicode.rs','src/legacy_case.rs','Cargo.lock'])fs.copyFileSync(path.join(prior,name),path.join(copy,name));
 fs.writeFileSync(path.join(copy,'src/parse.rs'),changed);
}
assert.equal(fs.readFileSync(path.join(copy,'src/parse.rs'),'utf8').replaceAll('\r\n','\n'),changed);
for(const name of ['src/lib.rs','src/unicode.rs','src/legacy_case.rs','Cargo.lock','Cargo.toml','src/api.rs','src/indexing.rs','src/unicodetables.rs'])assert.equal(fs.readFileSync(path.join(copy,name),'utf8').replaceAll('\r\n','\n'),fs.readFileSync(path.join(prior,name),'utf8').replaceAll('\r\n','\n'),'Protected boundary '+name);
console.log('Speculative-escape rollback copy verified; original and predecessor unchanged.');
