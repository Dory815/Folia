// Root isolated compiling-mutant acceptance; working sources/fixtures untouched.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-path-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map();function copyTree(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())copyTree(f);else if(e.isFile()){const rel=path.relative(vendor,f),bytes=fs.readFileSync(f),dest=path.join(copy,rel);originals.set(rel,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}}}
copyTree(path.join(vendor,'src'));copyTree(path.join(vendor,'tests'));for(const name of ['Cargo.toml','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name));originals.set(name,bytes);fs.writeFileSync(path.join(copy,name),bytes);}
function run(name){const built=cp.spawnSync('cargo',['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','path_parity','--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Mutant must compile: '+name);const result=cp.spawnSync('cargo',['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','path_parity'],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-test.log'),(result.stdout||'')+(result.stderr||''));return result;}
assert.equal(run('baseline').status,0,'Unchanged baseline must pass');console.log('Baseline compiles and passes.');
const mutations=[
 ['dedupe-first-point','            .last()','            .first()'],
 ['dedupe-epsilon-inclusive','< 1e-3 && (prev.1 - point.1).abs() < 1e-3','<= 1e-3 && (prev.1 - point.1).abs() <= 1e-3'],
 ['dedupe-tolerance-increased','< 1e-3 && (prev.1 - point.1).abs() < 1e-3','< 1e-2 && (prev.1 - point.1).abs() < 1e-2'],
 ['rounded-degenerate-threshold','in_len <= 1e-3 || out_len <= 1e-3','in_len <= 1e-2 || out_len <= 1e-2'],
 ['rounded-trim-threshold','if trim <= 0.5','if trim <= 0.55'],
 ['rounded-curvature-omitted','end: quantized_point(after,','end: quantized_point(cur,'],
 ['rounded-radius-ignored','radius.min(in_len * 0.45).min(out_len * 0.45)','in_len.min(out_len) * 0.45'],
 ['basis-initial-weight','5.0 * p0.0 + p1.0','4.0 * p0.0 + p1.0'],
 ['basis-interior-weight','x0 + 4.0 * x1 + x','x0 + 3.0 * x1 + x'],
 ['basis-final-weight','x0 + 5.0 * x1','x0 + 4.0 * x1'],
 ['quantization-coarsened','format!("{value:.3}")','format!("{value:.2}")'],
 ['nonfinite-radius-ignored','finite(radius, "rounded_polyline_path.radius")?;','// omitted finite radius check'],
];
const file=path.join('src','scene','compile','paths.rs'),target=path.join(copy,file),source=originals.get(file).toString().replaceAll('\r\n','\n'),results=[];
for(const[name,from,to]of mutations){assert.ok(source.includes(from),'Missing mutation anchor '+name);try{fs.writeFileSync(target,source.replace(from,to));const r=run(name);assert.equal(r.status,101,'Must fail Rust assertions, not crash: '+name);assert.match(r.stdout||'',/test result: FAILED/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation/);results.push({name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+name);}finally{fs.writeFileSync(target,originals.get(file));}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working source or expected fixture changed: '+rel);
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');console.log('All '+results.length+' compiling mutants detected; originals unchanged.');
