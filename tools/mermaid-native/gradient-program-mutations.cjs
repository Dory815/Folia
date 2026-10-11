// Root isolated compiling/assertion-failing mutations; originals byte guarded.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-gradient-program-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map();function copyTree(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())copyTree(f);else if(e.isFile()){const rel=path.relative(vendor,f),bytes=fs.readFileSync(f),dest=path.join(copy,rel);originals.set(rel,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}}}
copyTree(path.join(vendor,'src'));copyTree(path.join(vendor,'tests'));for(const name of ['Cargo.toml','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name));originals.set(name,bytes);fs.writeFileSync(path.join(copy,name),bytes);}
function run(name){
 const args=['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','gradient_program_parity'];
 const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8',timeout:180000});fs.writeFileSync(path.join(work,name+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Mutant compiles '+name);
 const result=cp.spawnSync('cargo',[...args,'--','--nocapture'],{encoding:'utf8',timeout:30000});fs.writeFileSync(path.join(work,name+'-test.log'),(result.stdout||'')+(result.stderr||''));return result;
}
const mutations=require('./gradient-program-mutation-specs.cjs'),only=process.argv.find(x=>x.startsWith('--only='))?.slice(7),selected=only?mutations.filter(m=>m[0]===only):mutations;
assert.ok(selected.length,'Unknown mutation');const baseline=run('baseline');assert.equal(baseline.status,0,'Complete frozen baseline must pass');assert.match(baseline.stdout,/3 passed; 0 failed/);console.log('Baseline passes all three Source tests.');const results=[];
for(const[name,file,from,to]of selected){const normalized=path.normalize(file),target=path.join(copy,normalized),original=originals.get(normalized),source=original.toString().replaceAll('\r\n','\n');assert.equal(source.split(from).length-1,1,'Unique anchor '+name);
 try{fs.writeFileSync(target,source.replace(from,to));const r=run(name);assert.equal(r.status,101,'Assertions fail, not crash '+name);assert.match(r.stdout||'',/test result: FAILED/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation|not yet implemented/);assert.match((r.stdout||'')+(r.stderr||''),/assertion `left == right` failed/);results.push({name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+name);}
 finally{fs.writeFileSync(target,original);}
}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Source/expected protected '+rel);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify({baseline_passes:true,results,working_files_unchanged:true},null,2)+'\n');
console.log(`All ${selected.length} compiling semantic mutations detected.`);
