// Root append-only observation of actual compile + RasterPipeline::run. No rewritten start.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),source=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0'),copy=path.join(root,'build/mermaid-pipeline-executor-source'),work=path.join(root,'build/mermaid-pipeline-executor-reference'),hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const pins={'src/pipeline/mod.rs':'fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4','src/pipeline/highp.rs':'81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416','src/pipeline/lowp.rs':'3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88','src/pixmap.rs':'484d49d1f4c3c3c8fb3fab3e1b1535292de828c0f258c3cab9bda02c7d9626fc'};
for(const[f,p]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(source,f))),p);
const names=fs.readFileSync(path.join(source,'src/pipeline/mod.rs'),'utf8').match(/pub enum Stage \{([\s\S]*?)\n\}/)[1].split('\n').map(s=>s.trim().replace(/ = 0/,'').replace(/,$/,'')).filter(Boolean);assert.equal(names.length,79);
const tails=[['LoadDestination','load_dst_tail'],['Store','store_tail'],['LoadDestinationU8','load_dst_u8_tail'],['StoreU8','store_u8_tail'],['SourceOverRgba','source_over_rgba_tail']],append={};
for(const precision of ['highp','lowp'])append[`src/pipeline/${precision}.rs`]=`
// Root maps actual compiled function pointers without inserting any stage.
pub fn folia_executor_function_id(fun:StageFn)->(u8,bool){
 if fn_ptr(fun)==fn_ptr(just_return){return (79,false);}
 ${tails.map(([name,fun])=>`if fn_ptr(fun)==fn_ptr(${fun}){return (${names.indexOf(name)},true);}`).join('\n')}
 for(i,original)in STAGES.iter().enumerate(){if fn_ptr(fun)==fn_ptr(*original){return(i as u8,false);}}
 panic!("Unknown original compiled pointer");
}
`;
append['src/pipeline/mod.rs']=`
// Root snapshots original persistent state and destination after the actual run.
fn folia_executor_context(c:&Context)->alloc::vec::Vec<u32>{
 let u=&c.uniform_color;let t=c.transform;let g=&c.evenly_spaced_2_stop_gradient;let p=&c.two_point_conical_gradient;
 let mut v=alloc::vec![c.current_coverage.to_bits(),u.r.to_bits(),u.g.to_bits(),u.b.to_bits(),u.a.to_bits()];
 v.extend(u.rgba.map(u32::from));for x in [g.factor.r,g.factor.g,g.factor.b,g.factor.a,g.bias.r,g.bias.g,g.bias.b,g.bias.a]{v.push(x.to_bits());}
 v.push(c.gradient.len as u32);v.extend([p.p0.to_bits(),p.p1.to_bits()]);v.extend(bytemuck::cast::<_,[u32;8]>(p.mask));
 for x in [c.limit_x.scale,c.limit_x.inv_scale,c.limit_y.scale,c.limit_y.inv_scale,t.sx,t.ky,t.kx,t.sy,t.tx,t.ty,c.sampler.inv_width,c.sampler.inv_height]{v.push(x.to_bits());}
 v.push(match c.sampler.spread_mode{SpreadMode::Pad=>0,SpreadMode::Reflect=>1,SpreadMode::Repeat=>2});
 v.push(c.gradient.factors.len()as u32);for x in &c.gradient.factors{v.extend([x.r.to_bits(),x.g.to_bits(),x.b.to_bits(),x.a.to_bits()]);}
 v.push(c.gradient.biases.len()as u32);for x in &c.gradient.biases{v.extend([x.r.to_bits(),x.g.to_bits(),x.b.to_bits(),x.a.to_bits()]);}
 v.push(c.gradient.t_values.len()as u32);for x in &c.gradient.t_values{v.push(x.get().to_bits());}v
}
pub fn folia_executor(ids:&[u8],force:bool,uniform:[u32;4],uniform_bytes:[u16;4],coverage:u32,dst:&[u8],real_width:usize,mask:&[u8],mask_width:u32,aa_pixels:[u8;2],aa_stride:u32,aa_shift:usize,origin:[u32;2],size:[u32;2])->(bool,usize,alloc::vec::Vec<(u8,bool)>,alloc::vec::Vec<(u8,bool)>,bool,alloc::vec::Vec<u8>,alloc::vec::Vec<u32>,alloc::vec::Vec<u32>){
 let mut b=RasterPipelineBuilder::new();b.set_force_hq_pipeline(force);
 b.ctx.uniform_color=UniformColorCtx{r:f32::from_bits(uniform[0]),g:f32::from_bits(uniform[1]),b:f32::from_bits(uniform[2]),a:f32::from_bits(uniform[3]),rgba:uniform_bytes};b.ctx.current_coverage=f32::from_bits(coverage);
 for id in ids{b.push(match *id{${names.map((n,i)=>`${i}=>Stage::${n}`).join(',')},_=>panic!("invalid Source stage")});}
 let mut compiled=b.compile();let (high,width,full,tail)=match &compiled.kind{
 RasterPipelineKind::High{functions,tail_functions}=>(true,8,functions.iter().map(|f|highp::folia_executor_function_id(*f)).collect(),tail_functions.iter().map(|f|highp::folia_executor_function_id(*f)).collect()),
 RasterPipelineKind::Low{functions,tail_functions}=>(false,16,functions.iter().map(|f|lowp::folia_executor_function_id(*f)).collect(),tail_functions.iter().map(|f|lowp::folia_executor_function_id(*f)).collect()),
 };
 let before=folia_executor_context(&compiled.ctx);let mut data=dst.to_vec();
 let panicked={let mut destination=SubPixmapMut{data:&mut data,size:tiny_skia_path::IntSize::from_wh(real_width as u32,4).unwrap(),real_width};
 let source=crate::Pixmap::new(1,1).unwrap();let rect=ScreenIntRect::from_xywh(origin[0],origin[1],size[0],size[1]).unwrap();
 std::panic::catch_unwind(std::panic::AssertUnwindSafe(||compiled.run(&rect,AAMaskCtx{pixels:aa_pixels,stride:aa_stride,shift:aa_shift},MaskCtx{data:mask,real_width:mask_width},source.as_ref(),&mut destination))).is_err()};
 (high,width,full,tail,panicked,data,before,folia_executor_context(&compiled.ctx))
}
`;
append['src/lib.rs']='\n#[allow(missing_docs)]\npub use pipeline::folia_executor;\n';
if(!fs.existsSync(copy)){fs.cpSync(source,copy,{recursive:true});for(const[f,a]of Object.entries(append))fs.appendFileSync(path.join(copy,f),a);}
function guard(dir=''){for(const e of fs.readdirSync(path.join(source,dir),{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())guard(f);else if(e.name.endsWith('.rs'))assert.equal(fs.readFileSync(path.join(copy,f),'utf8'),fs.readFileSync(path.join(source,f),'utf8')+(append[f.split(path.sep).join('/')]??''),'append-only '+f);}}guard();
const buffer=new ArrayBuffer(4),dv=new DataView(buffer),bits=x=>{dv.setFloat32(0,x,true);return dv.getUint32(0,true);},inputs=[];
function add(ids,force,shift,length,height=1,maskMode=0){
 const real_width=40,origin=[3,1],alpha=[0,1,127,128,254,255][shift%6],rgba=[Math.floor(alpha*.2),Math.floor(alpha*.4),Math.floor(alpha*.7),alpha];
 inputs.push({ids,force,uniform:rgba.map(x=>bits(x/255)),uniform_bytes:rgba,coverage:bits([0,1/255,.49999,.5,1][shift%5]),destination:Array.from({length:real_width*4*4},(_,i)=>{const a=(Math.floor(i/4)*31+shift*7)%256;return i%4===3?a:Math.floor(a*(i%4+1)/4);}),real_width,mask:Array.from({length:real_width*4},(_,i)=>maskMode===1?0:maskMode===2?(i<real_width+19?0:255):[0,1,127,128,254,255][(i+shift)%6]),mask_width:real_width,aa_pixels:[31,223],aa_stride:0,aa_shift:origin[0],origin,size:[length,height]});
}
const programs=[[],[5,8],[5,45],[5,13,45],[5,7,17,8],[5,16,45],[7,1,8],[7,25,8],[5,13,8],[5,9,8],[5,10,8],[5,12,8],[9,1,10],[12,9,15,10],[5,14,45],[5,7,15,8],[26,8],[5,0,1,8]];
for(let blend=18;blend<=44;blend++)programs.push([5,7,blend,8]);
for(const ids of programs)for(const force of [false,true])for(let shift=0;shift<6;shift++)for(const length of [1,2,7,8,9,15,16,17,33]){
 if(ids.some(id=>[14,15].includes(id))&&length>2)continue;
 add(ids,force,shift,length,shift%2?2:1,ids.includes(13)?shift%3:0);
}
const manifest=path.join(__dirname,'pipeline-executor-reference/Cargo.toml');fs.mkdirSync(work,{recursive:true});
if(!fs.existsSync(path.join(path.dirname(manifest),'Cargo.lock')))cp.execFileSync('cargo',['generate-lockfile','--offline','--manifest-path',manifest],{stdio:'pipe'});
const build=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest,'--target-dir',path.join(work,'target')],{encoding:'utf8'});fs.writeFileSync(path.join(work,'build.log'),(build.stdout||'')+(build.stderr||''));assert.equal(build.status,0,'Actual Source compiles');
const run=cp.spawnSync(path.join(work,'target/debug/folia-pipeline-executor-source-reference.exe'),[],{input:JSON.stringify(inputs),encoding:'utf8',maxBuffer:128*1024*1024});fs.writeFileSync(path.join(work,'stderr.log'),run.stderr||'');assert.equal(run.status,0,'Actual RasterPipeline::run execution');const cases=JSON.parse(run.stdout);assert.deepEqual(cases.map(c=>c.input),inputs);guard();
const data=JSON.stringify({schema:1,scope:'actual original compile + unchanged RasterPipeline::run; resource/blend compositions, complete destination and persistent context; no per-stage capture or register seeding',pins,cases})+'\n',out=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/pipeline-executor.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}
console.log(JSON.stringify({cases:cases.length,programs:programs.length,panics:cases.filter(c=>c.result[4]).length,sha256:hash(data)}));
