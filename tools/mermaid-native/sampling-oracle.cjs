// Root actual original compile -> original start -> original stages, with append-only observers.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),source=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0'),copy=path.join(root,'build/mermaid-sampling-source'),work=path.join(root,'build/mermaid-sampling-reference');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const pins={'src/pipeline/mod.rs':'fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4','src/pipeline/highp.rs':'81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416','src/pipeline/lowp.rs':'3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88'};
for(const[f,p]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(source,f))),p);
const originalMod=fs.readFileSync(path.join(source,'src/pipeline/mod.rs'),'utf8');
const names=originalMod.match(/pub enum Stage \{([\s\S]*?)\n\}/)[1].split('\n').map(l=>l.trim().replace(/ = 0/,'').replace(/,$/,'')).filter(Boolean);
const highStages=[0,1,2,3,4,6,46,...Array.from({length:28},(_,i)=>51+i)],lowStages=[0,1,4,6,46,51,52,53,54,55,57];
assert.equal(highStages.length,35);assert.equal(lowStages.length,11);
const append={};
for(const precision of ['highp','lowp']){
 const high=precision==='highp',width=high?8:16,type=high?'u32':'u16',vtype=high?'f32x8':'u16x16',channels=['r','g','b','a','dr','dg','db','da'];
 append[`src/pipeline/${precision}.rs`]=`
// Root observation only: original start and stage bodies remain byte-identical.
type FoliaCapture=(alloc::vec::Vec<alloc::vec::Vec<u32>>,[u32;8],[usize;3]);
std::thread_local! {static FOLIA_OBSERVER:std::cell::RefCell<Option<([[${type};${width}];8],alloc::vec::Vec<FoliaCapture>)>>=const{std::cell::RefCell::new(None)};}
pub fn folia_begin(input:[[${type};${width}];8]){FOLIA_OBSERVER.with(|s|{let mut s=s.borrow_mut();assert!(s.is_none(),"nested observer");*s=Some((input,alloc::vec::Vec::new()));});}
pub fn folia_end()->alloc::vec::Vec<FoliaCapture>{FOLIA_OBSERVER.with(|s|s.borrow_mut().take().unwrap().1)}
pub fn folia_setup(p:&mut Pipeline){
 FOLIA_OBSERVER.with(|s|{let s=s.borrow();let raw=&s.as_ref().unwrap().0;
 ${channels.map((c,i)=>`p.${c}=${high?`f32x8::from(raw[${i}].map(f32::from_bits))`:`u16x16(raw[${i}])`};`).join('\n')}
 });p.next_stage();
}
pub fn folia_capture(p:&mut Pipeline){
 let registers=alloc::vec![${channels.map(c=>high?`<[f32;8]>::from(p.${c}).map(f32::to_bits).to_vec()`:`p.${c}.0.map(u32::from).to_vec()`).join(',')}];
 let mask=bytemuck::cast::<_,[u32;8]>(p.ctx.two_point_conical_gradient.mask);
 FOLIA_OBSERVER.with(|s|s.borrow_mut().as_mut().unwrap().1.push((registers,mask,[p.dx,p.dy,p.tail])));
 p.next_stage();
}
`;
}
append['src/pipeline/mod.rs']=`
// Observer owns no Source equations. The compiler's actual function slices execute.
pub fn folia_sampling(ids:&[u8],force:bool,high:[[u32;8];8],low:[[u16;16];8],transform:[u32;6],two_stop:[[u32;4];2],len:usize,factors:&[[u32;4]],biases:&[[u32;4]],t:&[u32],radial:[u32;2],mask:[u32;8],origin:[u32;2],length:u32)->(bool,usize,alloc::vec::Vec<(alloc::vec::Vec<alloc::vec::Vec<u32>>,[u32;8],[usize;3])>){
 assert!(!ids.is_empty());
 let mut builder=RasterPipelineBuilder::new();builder.set_force_hq_pipeline(force);
 builder.ctx.transform=Transform::from_row(f32::from_bits(transform[0]),f32::from_bits(transform[1]),f32::from_bits(transform[2]),f32::from_bits(transform[3]),f32::from_bits(transform[4]),f32::from_bits(transform[5]));
 let color=|c:[u32;4]|GradientColor::new(f32::from_bits(c[0]),f32::from_bits(c[1]),f32::from_bits(c[2]),f32::from_bits(c[3]));
 builder.ctx.evenly_spaced_2_stop_gradient.factor=color(two_stop[0]);builder.ctx.evenly_spaced_2_stop_gradient.bias=color(two_stop[1]);
 builder.ctx.gradient.len=len;builder.ctx.gradient.factors=factors.iter().copied().map(color).collect();builder.ctx.gradient.biases=biases.iter().copied().map(color).collect();builder.ctx.gradient.t_values=t.iter().map(|v|NormalizedF32::new(f32::from_bits(*v)).unwrap()).collect();
 builder.ctx.two_point_conical_gradient.p0=f32::from_bits(radial[0]);builder.ctx.two_point_conical_gradient.p1=f32::from_bits(radial[1]);builder.ctx.two_point_conical_gradient.mask=bytemuck::cast(mask);
 for id in ids{builder.push(match *id{${names.map((n,i)=>`${i}=>Stage::${n}`).join(',')},_=>panic!("invalid stage")});}
 let mut compiled=builder.compile();
 let mut destination=crate::Pixmap::new(64,8).unwrap();let source=crate::Pixmap::new(64,8).unwrap();let mut destination_mut=destination.as_mut();let mut destination=destination_mut.as_subpixmap();
 match &compiled.kind{
  RasterPipelineKind::High{functions,tail_functions}=>{
   let interleave=|original:&[highp::StageFn]|{let mut out=alloc::vec![highp::folia_setup as highp::StageFn];for f in &original[..original.len()-1]{out.push(*f);out.push(highp::folia_capture);}out.push(*original.last().unwrap());out};
   let full=interleave(functions);let tail=interleave(tail_functions);highp::folia_begin(high);
   struct Clear;impl Drop for Clear{fn drop(&mut self){if std::thread::panicking(){let _=highp::folia_end();}}}let _clear=Clear;
   let rect=ScreenIntRect::from_xywh(origin[0],origin[1],length.min(8),1).unwrap();
   highp::start(&full,&tail,&rect,AAMaskCtx::default(),MaskCtx::default(),&mut compiled.ctx,source.as_ref(),&mut destination);
   (true,8,highp::folia_end())
  },
  RasterPipelineKind::Low{functions,tail_functions}=>{
   let interleave=|original:&[lowp::StageFn]|{let mut out=alloc::vec![lowp::folia_setup as lowp::StageFn];for f in &original[..original.len()-1]{out.push(*f);out.push(lowp::folia_capture);}out.push(*original.last().unwrap());out};
   let full=interleave(functions);let tail=interleave(tail_functions);lowp::folia_begin(low);
   struct Clear;impl Drop for Clear{fn drop(&mut self){if std::thread::panicking(){let _=lowp::folia_end();}}}let _clear=Clear;
   let rect=ScreenIntRect::from_xywh(origin[0],origin[1],length.min(16),1).unwrap();
   lowp::start(&full,&tail,&rect,AAMaskCtx::default(),MaskCtx::default(),&mut compiled.ctx,&mut destination);
   (false,16,lowp::folia_end())
  }
 }
}
`;
append['src/lib.rs']='\n#[allow(missing_docs)]\npub use pipeline::folia_sampling;\n';
if(!fs.existsSync(copy)){fs.cpSync(source,copy,{recursive:true});for(const[f,a]of Object.entries(append))fs.appendFileSync(path.join(copy,f),a);}
function guard(dir=''){for(const e of fs.readdirSync(path.join(source,dir),{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())guard(f);else if(e.name.endsWith('.rs'))assert.equal(fs.readFileSync(path.join(copy,f),'utf8'),fs.readFileSync(path.join(source,f),'utf8')+(append[f.split(path.sep).join('/')]??''),'append-only '+f);}}guard();
const buffer=new ArrayBuffer(4),dv=new DataView(buffer),bits=x=>{dv.setFloat32(0,x,true);return dv.getUint32(0,true);};
const rows=(n,w,f)=>Array.from({length:n},(_,c)=>Array.from({length:w},(_,i)=>f(c,i)));
const values=[0,0x80000000,bits(-3),bits(-1),bits(-0.5),bits(0.0031308),bits(0.04045),bits(0.5),bits(1),bits(2),bits(3),1,0x80000001,0x7f800000,0xff800000,0x7fc12345,0xffc23456,bits(0.99999994),bits(1.00000012),bits(2147483648),bits(-2147483648)];
function packed(c,i,shift){const coord=c<2?0:1,index=(c%2)*8+Math.floor(i/2),b=values[(index+shift+coord*7)%values.length];return i%2===0?b&0xffff:b>>>16;}
const cases=[];
function add(ids,force,shift,mode=0){
 const n=[2,3,5,9][shift%4],t=Array.from({length:n},(_,i)=>bits(mode===2?Math.floor(i/2)/Math.max(1,Math.floor((n-1)/2)):i/(n-1)));
 const input={ids,force,high:rows(8,8,(c,i)=>values[(shift+c*3+i)%values.length]),low:rows(8,16,(c,i)=>c<4?packed(c,i,shift):[0,1,127,128,254,255,256,511,1023][(c+i+shift)%9]),transform:[1,0.125,-0.25,0.75,-3,2].map(bits),two_stop:[[0.2,-0.4,0.6,0.75],[-0.1,0.9,0.05,0.3]].map(a=>a.map(bits)),len:n,factors:rows(16,4,(c,i)=>bits(mode===1?0:(c+1)*(i%2?-0.15:0.11))),biases:rows(16,4,(c,i)=>bits((c+1)*(i+1)*0.07)),t,radial:[bits([0.25,1,2,-1][shift%4]),bits([-0.25,0,0.5,1][shift%4])],mask:Array.from({length:8},(_,i)=>[0,0xffffffff,0x7fffffff,0x80000000][(i+shift)%4]),origin:[shift%17,shift%7],length:[1,7,8,15,16][shift%5]};
 // Lowp premultiply's Source u16 arithmetic has a byte-channel input domain.
 // Coordinate-stage raw bits are a different phase; do not multiply packed coordinates.
 if(ids[0]===4)input.low=rows(8,16,(c,i)=>[0,1,127,128,254,255][(c+i+shift)%6]);
 // Keep alpha of generated color segments in the valid paint range for sequences
 // that reach premultiply. RGB factor/bias remain unnormalized Source contexts.
 if(ids.length>1&&ids.includes(4)){
  input.factors.forEach(c=>c[3]=bits(0));input.biases.forEach((c,i)=>c[3]=bits(i<n?(i+1)/(n+1):0));
 }
 cases.push(input);
}
for(const force of [false,true])for(const id of highStages)for(let shift=0;shift<values.length;shift++)for(let mode=0;mode<3;mode++)add([id],force,shift,mode);
const sequences=[[6,46,57,51,55,4],[6,46,57,52,54,4],[6,46,62,63,53,54,4,65],[6,46,58,64,51,55,4,65],[6,46,59,51,54,4],[6,46,60,64,68,66,67,52,54,4,65],[6,46,61,64,51,54,4,65],[57,69,53,54,4,70,72],[54,73,75],[54,76,78],[1,71,0],[1,74,0],[1,77,0],[2,3,4],[53,52,51],[63,64,65],[0,6,1],[56,51,55]];
for(const force of [false,true])for(const ids of sequences)for(let shift=0;shift<values.length;shift++)for(let mode=0;mode<3;mode++)add(ids,force,shift,mode);
const manifest=path.join(__dirname,'sampling-reference/Cargo.toml'),env={...process.env,CARGO_TARGET_DIR:path.join(root,'build/mermaid-sampling-reference-target')};assert.equal(env.RUSTFLAGS??'','');assert.equal(env.CARGO_ENCODED_RUSTFLAGS??'','');fs.mkdirSync(work,{recursive:true});
if(!fs.existsSync(path.join(path.dirname(manifest),'Cargo.lock')))cp.execFileSync('cargo',['generate-lockfile','--offline','--manifest-path',manifest],{env,stdio:'pipe'});
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest],{env,encoding:'utf8',timeout:180000});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Original Source must compile');
const run=cp.spawnSync(path.join(env.CARGO_TARGET_DIR,'debug/folia-sampling-source-reference.exe'),[],{input:JSON.stringify(cases),encoding:'utf8',maxBuffer:128*1024*1024,timeout:60000});fs.writeFileSync(path.join(work,'stderr.log'),run.stderr||'');assert.equal(run.status,0,'Source execution must finish');
const observed=JSON.parse(run.stdout);assert.deepEqual(observed.map(c=>c.input),cases);guard();
for(const c of observed)assert.equal(c.result[2].length,c.input.ids.length);
const data=JSON.stringify({schema:1,scope:'35 highp / 11 lowp resource-independent stages; actual Source compile/start execution and raw register/mask observations',high_stages:highStages,low_stages:lowStages,names,cases:observed})+'\n',out=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/sampling.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}
console.log(JSON.stringify({cases:cases.length,high:observed.filter(c=>c.result[0]).length,low:observed.filter(c=>!c.result[0]).length,captures:observed.reduce((n,c)=>n+c.result[2].length,0),fixture_sha256:hash(data)}));
