// Root isolated mutation checks for the two frozen source-symbol helpers.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-symbol-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map();function copyTree(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())copyTree(f);else if(e.isFile()){const rel=path.relative(vendor,f),bytes=fs.readFileSync(f),dest=path.join(copy,rel);originals.set(rel,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}}}
copyTree(path.join(vendor,'src'));copyTree(path.join(vendor,'tests'));for(const name of ['Cargo.toml','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name));originals.set(name,bytes);fs.writeFileSync(path.join(copy,name),bytes);}
function run(name){const args=['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','edge_symbols_parity'];const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Mutant must compile: '+name);const result=cp.spawnSync('cargo',args,{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-test.log'),(result.stdout||'')+(result.stderr||''));return result;}
assert.equal(run('baseline').status,0,'Unchanged baseline must pass');console.log('Baseline compiles and passes.');
const mutations=[
 ['arrow-half','size * 0.52','size * 0.50'],
 ['arrow-minimum','clamp(5.5, 10.0)','clamp(4.5, 10.0)'],
 ['arrow-maximum','clamp(5.5, 10.0)','clamp(5.5, 9.0)'],
 ['diamond-end-rotation','&& !at_start','&& at_start'],
 ['circle-radius','circle_node(0.0, 5.0','circle_node(0.0, 4.0'],
 ['crowsfoot-circle-position','circle_node(-4.0, 4.0','circle_node(-3.0, 4.0',2],
 ['cross-contour-omitted','                PathSegment::MoveTo(local_point(-5.0, 5.0)),\n                PathSegment::LineTo(local_point(5.0, -5.0)),',''],
 ['crowsfoot-branch','local_point(8.0, 6.0)','local_point(6.0, 8.0)'],
 ['filled-diamond-paint','let fill = if decoration == EdgeDecoration::DiamondFilled','let fill = if false'],
 ['round-caps','LineCap::Round','LineCap::Butt'],
 ['translation-order','e: x, f: y','e: y, f: x'],
 ['rotation-precision','let cos = radians.cos();','let cos = (radians as f32).cos() as f64;'],
 ['width-binary-widening','parse_decimal(&value.to_string(), context)','Ok(f64::from(value))'],
 ['source-decimal-precision','format!("{value:.2}")','format!("{value:.3}")'],
 ['negative-width-check','if width < 0.0','if false'],
 // Removing only the raw guard is equivalent: parsed-decimal validation still
 // rejects every nonfinite input. Omit both validation stages for a real bug.
 ['nonfinite-guards-omitted',['if !value.is_finite()','if !number.is_finite()'],['if false','if false']],
];
const file=path.join('src','scene','compile','edge_symbols.rs'),target=path.join(copy,file),original=originals.get(file),source=original.toString().replaceAll('\r\n','\n'),results=[];
for(const[name,from,to,count=1]of mutations){const froms=Array.isArray(from)?from:[from],tos=Array.isArray(to)?to:[to];let mutated=source;for(let i=0;i<froms.length;i++){assert.equal(source.split(froms[i]).length-1,count,'Mutation anchor '+name);mutated=mutated.replaceAll(froms[i],tos[i]);}try{fs.writeFileSync(target,mutated);const r=run(name);assert.equal(r.status,101,'Must fail assertions, not crash: '+name);assert.match(r.stdout||'',/test result: FAILED/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation/);results.push({name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+name);}finally{fs.writeFileSync(target,original);}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working source or expected fixture changed: '+rel);
fs.writeFileSync(path.join(work,'results.json'),JSON.stringify(results,null,2)+'\n');console.log('All '+results.length+' compiling mutants detected; originals unchanged.');
