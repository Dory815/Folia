// Root compiling execution mutations, independently pinned expected values.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-resource-stage-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const fd=fs.openSync(path.join(work,'run.lock'),'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(path.join(work,'run.lock'));});
const originals=new Map();function file(rel){const bytes=fs.readFileSync(path.join(vendor,rel)),dest=path.join(copy,rel);originals.set(path.normalize(rel),bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}
function tree(dir){for(const e of fs.readdirSync(path.join(vendor,dir),{withFileTypes:true})){const rel=path.join(dir,e.name);if(e.isDirectory())tree(rel);else file(rel);}}tree('src');
for(const f of ['Cargo.toml','Cargo.lock','tests/resource_stage_parity.rs','tests/fixtures/resource-stages.json','tests/fixtures/resource-terminal.json'])file(f);
function run(name){const args=['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','resource_stage_parity'];const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,name+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Must compile '+name);const r=cp.spawnSync('cargo',[...args,'--','--nocapture'],{encoding:'utf8',timeout:30000,maxBuffer:16*1024*1024});fs.writeFileSync(path.join(work,name+'-test.log'),(r.stdout||'')+(r.stderr||''));return r;}
const baseline=run('baseline');assert.equal(baseline.status,0);assert.match(baseline.stdout,/1276 original Source resource cases/);assert.match(baseline.stdout,/1683 original Source resource cases/);assert.equal((baseline.stdout.match(/209 original Source resource cases/g)||[]).length,2);assert.match(baseline.stdout,/685 unsupported resource assertions/);assert.match(baseline.stdout,/test result: ok\. 5 passed/);console.log('Baseline: 2959 resource cases + 418 terminal cases, five tests pass.');
const fixture=JSON.parse(fs.readFileSync(path.join(vendor,'tests/fixtures/resource-stages.json'))),mutants=[];
function edit(name,precision,from,to,count=1){const f=path.normalize(`src/scene/prepare/sampling/resources/${precision}.rs`),source=originals.get(f).toString().replaceAll('\r\n','\n');assert.equal(source.split(from).length-1,count,'Frozen anchor '+name);mutants.push({name,file:f,code:source.replaceAll(from,to)});}
for(const[precision,ids]of [['highp',fixture.high_stages],['lowp',fixture.low_stages]]){
 const f=path.normalize(`src/scene/prepare/sampling/resources/${precision}.rs`),source=originals.get(f).toString().replaceAll('\r\n','\n'),at=source.indexOf('pub fn resource_'),body=source.indexOf('{',at);assert.ok(body>at&&at>=0);
 for(const id of ids)mutants.push({name:precision+'-omit-'+fixture.names[id],file:f,code:source.slice(0,body+1)+`\n if stage as u8 == ${id} {return Ok(StageControl::Continue);}\n`+source.slice(body+1)});
 mutants.push({name:precision+'-accept-unbound-stage',file:f,code:source.slice(0,body+1)+'\n if stage as u8 == 0 {return Ok(StageControl::Continue);}\n'+source.slice(body+1)});
 edit(precision+'-rgba-offset-not-bytes',precision,'(real_width * cursor.dy + cursor.dx) * 4','real_width * cursor.dy + cursor.dx',2);
 edit(precision+'-aa-second-pixel-crossed',precision,'(1, 1) => [aa.pixels[1], 0]','(1, 1) => [aa.pixels[0], 0]');
 edit(precision+'-aa-tail2-collapsed',precision,'(0, 2) => aa.pixels,','(0, 2) => [aa.pixels[0],0],');
 edit(precision+'-aa-offset-shift-lost',precision,'(aa.stride as usize * cursor.dy + cursor.dx) - aa.shift','aa.stride as usize * cursor.dy + cursor.dx');
}
edit('highp-stop-modifies-register','highp','return Ok(StageControl::Stop);','registers.channels[0]=[0.0;8];return Ok(StageControl::Stop);');
edit('lowp-stop-modifies-register','lowp','return Ok(StageControl::Stop);','registers.channels[0]=[0;16];return Ok(StageControl::Stop);');
edit('highp-zero-mask-continues','highp','if c == f32x8::default() {','if false && c == f32x8::default() {');
edit('lowp-zero-mask-continues','lowp','if c == u16x16::default() {','if false && c == u16x16::default() {');
edit('highp-load-factor-wrong','highp','const FACTOR: f32 = 1.0 / 255.0;','const FACTOR: f32 = 1.0 / 256.0;');
edit('highp-store-round-truncated','highp','.round_int()','.trunc_int()');
edit('highp-tail-reads-inactive-pixels','highp','ResourceProgram::Tail => cursor.tail','ResourceProgram::Tail => WIDTH',2);
edit('lowp-tail-reads-inactive-pixels','lowp','ResourceProgram::Tail => cursor.tail','ResourceProgram::Tail => WIDTH');
edit('lowp-div255-rounding-wrong','lowp','v + u16x16::splat(255)','v + u16x16::splat(128)');
edit('lowp-float-coverage-truncated','lowp','f * 255.0 + 0.5','f * 255.0');
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutants.filter(m=>m.name===only):mutants,results=[];assert.ok(selected.length);
for(const m of selected){const dest=path.join(copy,m.file);try{fs.writeFileSync(dest,m.code);const r=run(m.name);assert.equal(r.status,101,'Assertion failure required '+m.name);assert.match(r.stdout,/test result: FAILED/);assert.match((r.stdout||'')+(r.stderr||''),/assertion .*failed/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation|attempt to .* with overflow|index out of bounds/);results.push({name:m.name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(dest,originals.get(m.file));}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working files changed '+rel);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify({baseline:{cases:2959,terminal_cases:418,tests:5},mutants:results,working_files_unchanged:true},null,2)+'\n');console.log(`All ${results.length} compiling semantic mutations detected; originals unchanged.`);
