// Root isolated semantic compiler-control mutations. No expected values change.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer'),work=path.join(root,'build/mermaid-pipeline-control-mutations'),copy=path.join(work,'crate');fs.mkdirSync(work,{recursive:true});
const lock=path.join(work,'run.lock'),fd=fs.openSync(lock,'wx');fs.writeFileSync(fd,String(process.pid));process.on('exit',()=>{fs.closeSync(fd);fs.unlinkSync(lock);});
const originals=new Map();function tree(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())tree(f);else if(e.isFile()){const rel=path.relative(vendor,f),bytes=fs.readFileSync(f),dest=path.join(copy,rel);originals.set(rel,bytes);fs.mkdirSync(path.dirname(dest),{recursive:true});if(!fs.existsSync(dest)||!fs.readFileSync(dest).equals(bytes))fs.writeFileSync(dest,bytes);}}}tree(path.join(vendor,'src'));tree(path.join(vendor,'tests'));for(const name of ['Cargo.toml','Cargo.lock']){const bytes=fs.readFileSync(path.join(vendor,name));originals.set(name,bytes);fs.writeFileSync(path.join(copy,name),bytes);}
function run(label){const args=['test','--locked','--offline','--no-default-features','--manifest-path',path.join(copy,'Cargo.toml'),'--target-dir',path.join(work,'target'),'--test','pipeline_control_parity'];const built=cp.spawnSync('cargo',[...args,'--no-run'],{encoding:'utf8'});fs.writeFileSync(path.join(work,label+'-build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Mutant must compile '+label);const result=cp.spawnSync('cargo',[...args,'--','--nocapture'],{encoding:'utf8'});fs.writeFileSync(path.join(work,label+'-test.log'),(result.stdout||'')+(result.stderr||''));return result;}
const mutants=[],add=(name,from,to)=>mutants.push({name,from,to});
add('empty-low-precision','precision: PipelinePrecision::High,','precision: PipelinePrecision::Low,');
add('empty-wrong-width','width: 8,','width: 16,');
add('empty-context-retained','context: C::default(),','context,');
add('empty-terminal-added','full: Vec::new(),','full: vec![PipelineOperation::Return],');
add('force-ignored','force_hq_pipeline || !is_lowp_compatible','!is_lowp_compatible');
add('eligibility-any-stage','stages.iter().all(|&stage| pipeline_stage_is_lowp(stage))','stages.iter().any(|&stage| pipeline_stage_is_lowp(stage))');
add('eligibility-first-only','stages.iter().all(|&stage| pipeline_stage_is_lowp(stage))','pipeline_stage_is_lowp(stages[0])');
add('eligibility-last-only','stages.iter().all(|&stage| pipeline_stage_is_lowp(stage))','pipeline_stage_is_lowp(*stages.last().unwrap())');
add('lowp-null-table-accepted','PipelineStage::GammaCompressSrgb => false,','PipelineStage::GammaCompressSrgb => true,');
add('lowp-supported-table-rejected','PipelineStage::XYToRadius => true,','PipelineStage::XYToRadius => false,');
add('high-width-sixteen','PipelinePrecision::High => 8,','PipelinePrecision::High => 16,');
add('low-width-eight','PipelinePrecision::Low => 16,','PipelinePrecision::Low => 8,');
add('full-order-reversed','full.extend(stages.iter().copied().map(PipelineOperation::Stage));','full.extend(stages.iter().copied().map(PipelineOperation::Stage));full.reverse();');
add('repeated-stages-deduplicated','full.extend(stages.iter().copied().map(PipelineOperation::Stage));','full.extend(stages.iter().copied().map(PipelineOperation::Stage));full.dedup();');
add('terminal-omitted','full.push(PipelineOperation::Return);','// Terminal omitted.');
add('nonempty-context-reset','tail,\n        context,','tail,\n        context:C::default(),');
add('tail-replacements-omitted',') => PipelineOperation::Tail(stage),',') => PipelineOperation::Stage(stage),');
add('tail-load-destination-missing','PipelineStage::LoadDestination\n                        |','PipelineStage::SeedShader\n                        |');
add('tail-store-missing','| PipelineStage::Store\n                        |','| PipelineStage::UniformColor\n                        |');
add('tail-load-u8-missing','| PipelineStage::LoadDestinationU8\n                        |','| PipelineStage::LoadMaskU8\n                        |');
add('tail-store-u8-missing','| PipelineStage::StoreU8\n                        |','| PipelineStage::ScaleU8\n                        |');
add('tail-source-over-missing','| PipelineStage::SourceOverRgba\n                )','| PipelineStage::SourceOver\n                )');
add('tail-first-only','for operation in &mut tail {','for operation in tail.iter_mut().take(1) {');
add('full-replaced-instead-of-tail','for operation in &mut tail {','for operation in &mut full {');
add('capacity-31-rejected','if full.len() == MAX_STAGES {','if full.len() >= MAX_STAGES-1 {');
add('capacity-32-accepted','if full.len() == MAX_STAGES {','if false {');
add('capacity-silently-truncated','full.extend(stages.iter().copied().map(PipelineOperation::Stage));','full.extend(stages.iter().take(31).copied().map(PipelineOperation::Stage));');
const file=path.normalize('src/scene/prepare/pipeline_control.rs'),original=originals.get(file),source=original.toString().replaceAll('\r\n','\n'),target=path.join(copy,file);
const baseline=run('baseline');assert.equal(baseline.status,0);assert.match(baseline.stdout,/64315 assertions/);assert.match(baseline.stdout,/237 assertions/);console.log('Baseline: 12872 Source cases / 64552 assertions / 2 tests pass.');
const only=process.argv.find(a=>a.startsWith('--only='))?.slice(7),selected=only?mutants.filter(m=>m.name===only):mutants,results=[];assert.ok(selected.length);
for(const m of selected){assert.equal(source.split(m.from).length-1,1,'Unique anchor '+m.name);try{fs.writeFileSync(target,source.replace(m.from,m.to));const r=run(m.name);assert.equal(r.status,101,'Assertion failure required '+m.name);assert.match(r.stdout,/test result: FAILED/);assert.doesNotMatch((r.stdout||'')+(r.stderr||''),/process didn't exit successfully|stack overflow|access violation/);results.push({name:m.name,compiles:true,test_exit:r.status,assertions_detected:true});console.log('Detected '+m.name);}finally{fs.writeFileSync(target,original);}}
for(const[rel,bytes]of originals)assert.ok(fs.readFileSync(path.join(vendor,rel)).equals(bytes),'Working files changed '+rel);
fs.writeFileSync(path.join(work,only?'results-'+only+'.json':'results.json'),JSON.stringify({baseline:{cases:12872,assertions:64552,tests:2},mutants:results,working_files_unchanged:true},null,2)+'\n');console.log(`All ${results.length} compiling semantic mutations detected; originals unchanged.`);
