// Root independent compiling mutants; frozen original gold is never rewritten.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-pattern-stage-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const fd=fs.openSync(path.join(work,'run.lock'),'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(path.join(work,'run.lock'));});
const originals=new Map();function file(rel){const bytes=fs.readFileSync(path.join(vendor,rel)),dest=path.join(copy,rel);originals.set(path.normalize(rel),bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}
function tree(dir){for(const e of fs.readdirSync(path.join(vendor,dir),{withFileTypes:true})){const rel=path.join(dir,e.name);if(e.isDirectory())tree(rel);else file(rel);}}tree('src');
for(const f of ['Cargo.toml','Cargo.lock','tests/pattern_stage_parity.rs','tests/fixtures/pattern-stages.json','tests/fixtures/pattern-tall.json','tests/fixtures/pattern-pixmap.json'])file(f);
function run(name){const args=['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','pattern_stage_parity'];const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Must compile '+name);const r=cp.spawnSync('cargo',[...args,'--','--nocapture'],{encoding:'utf8',timeout:30000,maxBuffer:16*1024*1024});fs.writeFileSync(path.join(work,name+'-test.log'),(r.stdout||'')+(r.stderr||''));return r;}
const baseline=run('baseline');assert.equal(baseline.status,0);assert.match(baseline.stdout,/7056 original Source pattern cases \/ 2776370 assertions/);assert.match(baseline.stdout,/1694 original Source pixmap cases \/ 1695 assertions/);assert.match(baseline.stdout,/459 unsupported pattern assertions/);assert.match(baseline.stdout,/test result: ok\. 3 passed/);console.log('Baseline: 7056 pattern / 1694 storage cases / 2778524 assertions / three tests.');
const mutants=[];
function edit(name,file,from,to,count=1){const f=path.normalize('src/scene/prepare/sampling/pattern/'+file+'.rs'),source=originals.get(f).toString().replaceAll('\r\n','\n');assert.equal(source.split(from).length-1,count,'Frozen anchor '+name);mutants.push({name,file:f,code:source.replaceAll(from,to)});}
const high=path.normalize('src/scene/prepare/sampling/pattern/highp.rs'),source=originals.get(high).toString().replaceAll('\r\n','\n'),at=source.indexOf('pub fn pattern_high_stage'),body=source.indexOf('{',at);assert.ok(at>=0&&body>at);
for(const id of [11,47,48,49,50])mutants.push({name:'omit-stage-'+id,file:high,code:source.slice(0,body+1)+`\n if stage as u8 == ${id} {return Ok(());}\n`+source.slice(body+1)});
mutants.push({name:'accept-unsupported-high',file:high,code:source.slice(0,body+1)+'\n if stage as u8 == 0 {return Ok(());}\n'+source.slice(body+1)});
edit('accept-unsupported-low','mod','Err(SamplingError{precision:PipelinePrecision::Low,stage})','if stage as u8 == 0 {Ok(())}else{Err(SamplingError{precision:PipelinePrecision::Low,stage})}');
edit('swap-output-r-g','highp','[p.r,p.g,p.b,p.a,p.dr,p.dg,p.db,p.da]','[p.g,p.r,p.b,p.a,p.dr,p.dg,p.db,p.da]');
edit('gather-y-limit-uses-width','highp','ulp_sub(pixmap.height() as f32)','ulp_sub(pixmap.height().min(pixmap.width()) as f32)');
edit('gather-x-clamp-loses-last-column','highp','ulp_sub(pixmap.width() as f32)','ulp_sub((pixmap.width().max(2)-1) as f32)');
edit('gather-lane-seven-reuses-zero','highp','pixels[index[7] as usize]','pixels[index[0] as usize]');
edit('byte-scale-256','highp','const FACTOR: f32 = 1.0 / 255.0;','const FACTOR: f32 = 1.0 / 256.0;');
edit('nearest-recompute-inverse-x','highp','exclusive_repeat(p.r, ctx.scale, ctx.inv_scale)','exclusive_repeat(p.r, ctx.scale, 1.0/ctx.scale)');
edit('reflect-half-period-wrong','highp','inv_limit * f32x8::splat(0.5)','inv_limit * f32x8::splat(0.25)');
edit('repeat-truncates-negative','highp','(v * f32x8::splat(inv_limit)).floor()','(v * f32x8::splat(inv_limit)).trunc_int().to_f32x8()');
edit('filter-x-half-offset','highp','x + f32x8::splat(0.5)','x + f32x8::splat(0.25)',2);
edit('bilinear-first-tap-offset','highp','let start = -0.5;','let start = 0.0;');
edit('bicubic-first-tap-offset','highp','let start = -1.5;','let start = -1.0;');
edit('bilinear-reverse-x-tap-order','highp','for i in 0..2 {','for i in (0..2).rev() {');
edit('bicubic-reverse-y-tap-order','highp','for j in 0..4 {','for j in (0..4).rev() {');
edit('filter-weight-swap-axis','highp','wx[i] * wy[j]','wx[j] * wy[i]',2);
edit('bicubic-near-polynomial','highp','f32x8::splat(-21.0/18.0)','f32x8::splat(-20.0/18.0)');
edit('bicubic-far-polynomial','highp','f32x8::splat(7.0/18.0)','f32x8::splat(6.0/18.0)');
edit('per-tap-spread-forced-pad','highp','tile(x, ctx.spread_mode, pixmap.width() as f32, ctx.inv_width)','tile(x, SpreadMode::Pad, pixmap.width() as f32, ctx.inv_width)');
edit('per-tap-inverse-recomputed','highp','pixmap.width() as f32, ctx.inv_width','pixmap.width() as f32, 1.0/(pixmap.width() as f32)');
edit('reflect-route-uses-repeat','highp','SpreadMode::Reflect => exclusive_reflect(v, limit, inv_limit)','SpreadMode::Reflect => exclusive_repeat(v, limit, inv_limit)');
edit('constructor-allows-zero-width','types','if width==0||height==0','if height==0');
edit('constructor-rejects-excess-bytes','types','if data.len()<size','if data.len()!=size');
edit('constructor-ignores-short-storage','types','if data.len()<size{return None;}','if data.len()<size{ /* omitted validation */ }');
edit('constructor-truncates-excess-storage','types','Some(Self{data,width,height})','Some(Self{data:&data[..size],width,height})');
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutants.filter(m=>m.name===only):mutants,results=[];assert.ok(selected.length);
for(const m of selected){const dest=path.join(copy,m.file);try{fs.writeFileSync(dest,m.code);const r=run(m.name);assert.equal(r.status,101,'Assertion failure required '+m.name);assert.match(r.stdout,/test result: FAILED/);assert.match((r.stdout||'')+(r.stderr||''),/assertion .*failed/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation|attempt to .* with overflow|index out of bounds|OutputSliceWouldHaveSlop/);results.push({name:m.name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(dest,originals.get(m.file));}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working files changed '+rel);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify({baseline:{pattern_cases:7056,pixmap_cases:1694,assertions:2778524,tests:3},mutants:results,working_files_unchanged:true},null,2)+'\n');console.log(`All ${results.length} compiling semantic mutations detected; originals unchanged.`);
