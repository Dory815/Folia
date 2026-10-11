// Root executes the original compiler and observes actual function pointers.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),registry=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f'),source=path.join(registry,'tiny-skia-0.12.0'),copy=path.join(root,'build/mermaid-pipeline-control-source'),work=path.join(root,'build/mermaid-pipeline-control-reference');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex'),pins={'src/pipeline/mod.rs':'fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4','src/pipeline/highp.rs':'81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416','src/pipeline/lowp.rs':'3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88'};
for(const[f,p]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(source,f))),p);
const text=fs.readFileSync(path.join(source,'src/pipeline/mod.rs'),'utf8'),names=text.match(/pub enum Stage \{([\s\S]*?)\n\}/)[1].split('\n').map(l=>l.trim().replace(/ = 0/,'').replace(/,$/,'')).filter(Boolean);assert.equal(names.length,79);
const tails=[['LoadDestination','load_dst_tail'],['Store','store_tail'],['LoadDestinationU8','load_dst_u8_tail'],['StoreU8','store_u8_tail'],['SourceOverRgba','source_over_rgba_tail']];
const append={};
for(const precision of ['highp','lowp'])append[`src/pipeline/${precision}.rs`]=`
// Root observes actual compiled pointers; never computes the compiler result.
pub fn folia_function_id(fun:StageFn)->(u8,bool){
 if fn_ptr(fun)==fn_ptr(just_return){return (79,false);}
 ${tails.map(([name,fun])=>`if fn_ptr(fun)==fn_ptr(${fun}){return (${names.indexOf(name)},true);}`).join('\n')}
 for (i,original) in STAGES.iter().enumerate(){if fn_ptr(fun)==fn_ptr(*original){return (i as u8,false);}}
 panic!("Unidentified actual Source function");
}
`;
append['src/pipeline/mod.rs']=`
// Root context projection is an observer, not a new Source implementation.
pub fn folia_compile_control(ids:&[u8],force:bool,marker:u32)->Option<(bool,usize,alloc::vec::Vec<(u8,bool)>,alloc::vec::Vec<(u8,bool)>,[u32;13])>{
 std::panic::catch_unwind(||{
  let mut builder=RasterPipelineBuilder::new();builder.set_force_hq_pipeline(force);
  builder.ctx.current_coverage=f32::from_bits(marker);
  builder.ctx.transform=Transform::from_row(2.0,3.0,4.0,5.0,6.0,7.0);
  builder.ctx.uniform_color.r=f32::from_bits(marker);
  builder.ctx.gradient.len=7;builder.ctx.two_point_conical_gradient.p0=f32::from_bits(marker);
  builder.ctx.two_point_conical_gradient.mask=u32x8::splat(marker);
  builder.ctx.limit_x.scale=f32::from_bits(marker);builder.ctx.sampler.inv_width=f32::from_bits(marker);
  for id in ids {builder.push(match *id{${names.map((n,i)=>`${i}=>Stage::${n}`).join(',')},_=>panic!("invalid Source stage")});}
  let pipeline=builder.compile();
  let (high,width,full,tail)=match pipeline.kind{
   RasterPipelineKind::High{functions,tail_functions}=>(true,highp::STAGE_WIDTH,functions.iter().map(|f|highp::folia_function_id(*f)).collect(),tail_functions.iter().map(|f|highp::folia_function_id(*f)).collect()),
   RasterPipelineKind::Low{functions,tail_functions}=>(false,lowp::STAGE_WIDTH,functions.iter().map(|f|lowp::folia_function_id(*f)).collect(),tail_functions.iter().map(|f|lowp::folia_function_id(*f)).collect()),
  };
  let c=pipeline.ctx;let t=c.transform;
  (high,width,full,tail,[c.current_coverage.to_bits(),t.sx.to_bits(),t.ky.to_bits(),t.kx.to_bits(),t.sy.to_bits(),t.tx.to_bits(),t.ty.to_bits(),c.uniform_color.r.to_bits(),c.gradient.len as u32,c.two_point_conical_gradient.p0.to_bits(),bytemuck::cast::<_,[u32;8]>(c.two_point_conical_gradient.mask)[0],c.limit_x.scale.to_bits(),c.sampler.inv_width.to_bits()])
 }).ok()
}
`;
append['src/lib.rs']='\n#[allow(missing_docs)]\npub use pipeline::folia_compile_control;\n';
if(!fs.existsSync(copy)){fs.cpSync(source,copy,{recursive:true});for(const[f,a]of Object.entries(append))fs.appendFileSync(path.join(copy,f),a);}
function guard(dir=''){for(const e of fs.readdirSync(path.join(source,dir),{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())guard(f);else if(e.name.endsWith('.rs'))assert.equal(fs.readFileSync(path.join(copy,f),'utf8'),fs.readFileSync(path.join(source,f),'utf8')+(append[f.split(path.sep).join('/')]??''),'append-only '+f);}}guard();
const cases=[],markers=[0,0x80000000,0x3eaaaaab,0x7fc12345,0x7f800000];
function add(stages,force){cases.push({stages,force,marker:markers[cases.length%markers.length]});}
for(const force of [false,true]){add([],force);for(let i=0;i<79;i++){add([i],force);for(let j=0;j<79;j++)add([i,j],force);}}
for(let len=1;len<=33;len++)for(const force of [false,true])for(const stage of [6,58,7])add(Array(len).fill(stage),force);
const transformed=[6,46,57,51,54,4],suffixes=[[8],[7,25,8],[45],[13,14,7,25,8],[16,7,28,8],[7,71,25,72,8],[7,74,25,75,8],[7,77,25,78,8]];
for(const stages of [transformed,[6,46,62,63,51,54,4,65]])for(const suffix of suffixes)for(const force of [false,true])add([...stages,...suffix],force);
const manifest=path.join(__dirname,'pipeline-control-reference/Cargo.toml'),env={...process.env,CARGO_TARGET_DIR:path.join(root,'build/mermaid-pipeline-control-reference-target')};fs.mkdirSync(work,{recursive:true});
if(!fs.existsSync(path.join(path.dirname(manifest),'Cargo.lock')))cp.execFileSync('cargo',['generate-lockfile','--offline','--manifest-path',manifest],{env,stdio:'pipe'});
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest],{env,encoding:'utf8',timeout:180000});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Original Source must compile');
const run=cp.spawnSync(path.join(env.CARGO_TARGET_DIR,'debug/folia-pipeline-control-source-reference.exe'),[],{input:JSON.stringify(cases),encoding:'utf8',maxBuffer:64*1024*1024,timeout:60000});fs.writeFileSync(path.join(work,'stderr.log'),run.stderr||'');assert.equal(run.status,0,'Source observer must complete');
const observed=JSON.parse(run.stdout);assert.deepEqual(observed.map(c=>c.input),cases);guard();
const data=JSON.stringify({schema:1,scope:'actual Source compile control and pointer/context projection; no stage sampling',names,cases:observed})+'\n',out=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/pipeline-control.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else fs.writeFileSync(out,data);
console.log(JSON.stringify({cases:cases.length,panics:observed.filter(c=>c.result===null).length,stages:names.length,fixture_sha256:hash(data)}));
