// Root bounded whole-run comparisons and isolated control-flow faults.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-pipeline-executor-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
fs.cpSync(vendor,copy,{recursive:true,filter:p=>!p.split(path.sep).some(part=>['target','.git'].includes(part))});const originals=new Map();function remember(dir=''){for(const e of fs.readdirSync(path.join(vendor,dir),{withFileTypes:true})){if(['target','.git'].includes(e.name))continue;const rel=path.join(dir,e.name);if(e.isDirectory())remember(rel);else originals.set(rel,fs.readFileSync(path.join(vendor,rel)));}}remember();
const env={...process.env,CARGO_TARGET_DIR:path.join(work,'target')};function run(label){const r=cp.spawnSync('cargo',['test','--locked','--offline','--manifest-path',path.join(copy,'Cargo.toml'),'--no-default-features','--test','pipeline_executor_parity','--','--nocapture'],{env,encoding:'utf8',maxBuffer:16*1024*1024});fs.writeFileSync(path.join(work,label+'.log'),(r.stdout||'')+(r.stderr||''));return r;}
const baseline=run('baseline');assert.equal(baseline.status,0);assert.match(baseline.stdout,/4608 actual Source complete run cases \/ 0 mismatches \/ 0 actual stub calls \/ 0 errors/);assert.match(baseline.stdout,/test result: ok\. 1 passed/);console.log('Baseline: all 4608 original whole-run records pass.');
const mutants=[];function edit(name,precision,from,to,count=1){const file=path.normalize('src/scene/prepare/pipeline_executor/'+precision+'.rs'),source=originals.get(file).toString().replaceAll('\r\n','\n');assert.equal(source.split(from).length-1,count,'Frozen control anchor '+name);mutants.push({name,file,code:source.replaceAll(from,to)});}
for(const precision of ['highp','lowp']){const bank=precision==='highp'?'HighRegisters':'LowRegisters';
 edit(precision+'-register-reset-per-tile',precision,'while x + STAGE_WIDTH <= end {','while x + STAGE_WIDTH <= end {\n registers='+bank+'::default();');
 edit(precision+'-register-reset-per-row',precision,'for y in rect.y()..rect.bottom() {','for y in rect.y()..rect.bottom() {\n registers='+bank+'::default();');
 edit(precision+'-tail-skipped',precision,'if x != end {','if false && x != end {');
 edit(precision+'-tail-uses-full-list',precision,'run_tile(tail,','run_tile(full,');
 edit(precision+'-mask-stop-ignored',precision,'Ok(StageControl::Stop) => return Ok(()),','Ok(StageControl::Stop) => {},');
 edit(precision+'-tail-resource-uses-full',precision,'ResourceCursor { program, ..cursor }','ResourceCursor { program: ResourceProgram::Full, ..cursor }');
 edit(precision+'-blend-dispatch-source-over-to-plus',precision,'blend_'+precision.replace('p','')+'_stage(stage, registers)','blend_'+precision.replace('p','')+'_stage(if stage == SourceOver { Plus } else { stage }, registers)');
 edit(precision+'-row-cursor-stuck',precision,'dy: y as usize','dy: rect.y() as usize',2);
 edit(precision+'-tile-overlaps',precision,'x += STAGE_WIDTH;','x += STAGE_WIDTH - 1;');
}
edit('highp-A8-continues', 'highp','LoadDestinationU8 | StoreU8 | LoadMaskU8 => return Ok(()),','LoadDestinationU8 | StoreU8 | LoadMaskU8 => {},');
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutants.filter(m=>m.name===only):mutants,results=[];assert.ok(selected.length,'Known control fault');
for(const m of selected){const dest=path.join(copy,m.file);try{fs.writeFileSync(dest,m.code);const r=run(m.name);assert.equal(r.status,101,'A compiling assertion failure is required '+m.name);assert.match(r.stdout,/test result: FAILED/);assert.match((r.stdout||'')+(r.stderr||''),/assertion .*failed/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation/);results.push({name:m.name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(dest,originals.get(m.file));}}
for(const[rel,b]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(b),'Working file changed '+rel);fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify({baseline:{original_run_records:4608,tests:1},mutants:results,working_files_unchanged:true},null,2)+'\n');console.log(`All ${results.length} compiling control mutations detected; originals unchanged.`);
