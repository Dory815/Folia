// Root isolated execution mutations. Original Source golds and working files stay unchanged.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-sampling-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map();function tree(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())tree(f);else if(e.isFile()){const rel=path.relative(vendor,f),bytes=fs.readFileSync(f),dest=path.join(copy,rel);originals.set(rel,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}}}tree(path.join(vendor,'src'));tree(path.join(vendor,'tests'));for(const name of ['Cargo.toml','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name));originals.set(name,bytes);fs.writeFileSync(path.join(copy,name),bytes);}
function run(label){const args=['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','sampling_parity'];const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,label+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Mutant must compile '+label);const result=cp.spawnSync('cargo',[...args,'--','--nocapture'],{encoding:'utf8'});fs.writeFileSync(path.join(work,label+'-test.log'),(result.stdout||'')+(result.stderr||''));return result;}
const fixture=JSON.parse(fs.readFileSync(path.join(vendor,'tests/fixtures/sampling.json'),'utf8')),mutants=[];
for(const [precision,ids] of [['highp',fixture.high_stages],['lowp',fixture.low_stages]]){
 const file=path.normalize(`src/scene/prepare/sampling/${precision}.rs`),source=originals.get(file).toString().replaceAll('\r\n','\n'),start=source.indexOf('pub fn sample_'+(precision==='highp'?'high':'low')+'_stage'),body=source.indexOf('{',start);assert.ok(start>=0&&body>start);
 for(const id of ids)mutants.push({name:precision+'-omit-'+fixture.names[id],file,code:source.slice(0,body+1)+`\n if stage as u8 == ${id} {return Ok(());}\n`+source.slice(body+1)});
 const unsupported=precision==='highp'?5:2;
 mutants.push({name:precision+'-accept-unsupported',file,code:source.slice(0,body+1)+`\n if stage as u8 == ${unsupported} {return Ok(());}\n`+source.slice(body+1)});
}
const baseline=run('baseline');assert.equal(baseline.status,0);assert.match(baseline.stdout,/5733 Source cases/);assert.match(baseline.stdout,/945 Source cases/);assert.match(baseline.stdout,/672 explicit unsupported-stage assertions/);assert.match(baseline.stdout,/test result: ok\. 3 passed/);console.log('Baseline: 6678 actual Source cases and three tests pass.');
function edit(name,precision,from,to,count=1){const file=path.normalize(`src/scene/prepare/sampling/${precision}.rs`),source=originals.get(file).toString().replaceAll('\r\n','\n');assert.equal(source.split(from).length-1,count,'Unique frozen mutation anchor '+name);mutants.push({name,file,code:source.replaceAll(from,to)});}
for(const precision of ['highp','lowp']){
 edit(precision+'-exclusive-stop-threshold',precision,'>= tt','> tt',precision==='highp'?8:16);
 edit(precision+'-last-threshold-omitted',precision,'for i in 1..ctx.len {','for i in 1..ctx.len-1 {');
 edit(precision+'-factor-from-bias',precision,'let fr = gather!(&ctx.factors, 0);','let fr = gather!(&ctx.biases, 0);');
 edit(precision+'-bias-channel-crossed',precision,'let ba = gather!(&ctx.biases, 3);','let ba = gather!(&ctx.biases, 2);');
}
edit('highp-seed-destination-retained','highp','*dr = f32x8::default();','// Source destination reset omitted.');
edit('highp-degenerate-zero-valid','highp','t.cmp_le(f32x8::default()) | t.cmp_ne(t)','t.cmp_lt(f32x8::default()) | t.cmp_ne(t)');
edit('highp-mask-signed-bits-lost','highp','let mask: u32x8 = bytemuck::cast(context.radial_mask);','let mask: u32x8 = bytemuck::cast(context.radial_mask.map(|m|m&0x7fffffff));');
edit('highp-conical-p0-p1-crossed','highp','*r = x * f32x8::splat(context.radial_p[0]) + f32x8::splat(context.radial_p[1]);','*r = x * f32x8::splat(context.radial_p[1]) + f32x8::splat(context.radial_p[0]);');
edit('highp-gamma-linear-branch-only','highp','small.blend(linear, exp)','linear',2);
edit('highp-gamma-power-branch-only','highp','small.blend(linear, exp)','exp',2);
edit('lowp-alpha-normalized','lowp','let af = af * f32x16::splat(255.0)','let af = af.normalize() * f32x16::splat(255.0)');
edit('lowp-lookup-rounding-truncated','lowp','* f32x16::splat(255.0) + f32x16::splat(0.5)','* f32x16::splat(255.0)',4);
edit('lowp-premultiply-rounding-changed','lowp','(v + u16x16::splat(255))','(v + u16x16::splat(128))');
edit('lowp-coordinate-halves-swapped','lowp','d0.copy_from_slice(&data[0..32]);\n    d1.copy_from_slice(&data[32..64]);','d0.copy_from_slice(&data[32..64]);\n    d1.copy_from_slice(&data[0..32]);');
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutants.filter(m=>m.name===only):mutants,results=[];assert.ok(selected.length);
for(const m of selected){const file=path.join(copy,m.file);try{fs.writeFileSync(file,m.code);const r=run(m.name);assert.equal(r.status,101,'Assertion failure required '+m.name);assert.match(r.stdout,/test result: FAILED/);assert.match((r.stdout||'')+(r.stderr||''),/assertion .*failed/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation|attempt to .* with overflow/);results.push({name:m.name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(file,originals.get(m.file));}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working files changed '+rel);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify({baseline:{cases:6678,tests:3},mutants:results,working_files_unchanged:true},null,2)+'\n');console.log(`All ${results.length} compiling semantic mutations detected; originals unchanged.`);
