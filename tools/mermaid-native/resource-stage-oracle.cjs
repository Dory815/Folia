// Root: append-only observers execute actual original compile/start/pointers.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const terminal=process.argv.includes('--terminal'),blend=process.argv.includes('--blend'),pattern=process.argv.includes('--pattern'),stem=pattern?'pattern-stage':blend?'blend-stage':terminal?'resource-terminal':'resource-stage';assert.ok([terminal,blend,pattern].filter(Boolean).length<=1);
const root=path.resolve(__dirname,'../..'),source=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0'),copy=path.join(root,'build/mermaid-'+stem+'-source'),work=path.join(root,'build/mermaid-'+stem+'-reference');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
const pins={'src/pipeline/mod.rs':'fbbe3f276b7e95fdf501fe0420f60d18ca313395cdf79aa1a943e9ef996243a4','src/pipeline/highp.rs':'81388733345dbae3d29cbf758831809847bb1aa30a13f600d0672409bce38416','src/pipeline/lowp.rs':'3cc2fe168ae06273ff16bbc972b337e064b24c028a089e47aa14f5eb48752e88','src/pixmap.rs':'484d49d1f4c3c3c8fb3fab3e1b1535292de828c0f258c3cab9bda02c7d9626fc','src/color.rs':'3cea03557e9e7d8593230149052a11c2062a2439e6ebc2631dac6594e402bd8d'};
for(const[f,p]of Object.entries(pins))assert.equal(hash(fs.readFileSync(path.join(source,f))),p);
const names=fs.readFileSync(path.join(source,'src/pipeline/mod.rs'),'utf8').match(/pub enum Stage \{([\s\S]*?)\n\}/)[1].split('\n').map(s=>s.trim().replace(/ = 0/,'').replace(/,$/,'')).filter(Boolean);
const highStages=pattern?[11,47,48,49,50]:blend?Array.from({length:27},(_,i)=>i+18):[5,7,8,13,14,15,16,17,45],lowStages=pattern?[]:blend?highStages.filter(i=>![32,33,40,41,42,43,44].includes(i)):[5,7,8,9,10,12,13,14,15,16,17,45],append={};
for(const precision of ['highp','lowp']){
 const high=precision==='highp',width=high?8:16,type=high?'u32':'u16',dst=high?'pixmap_dst':'pixmap',channels=['r','g','b','a','dr','dg','db','da'];
 append[`src/pipeline/${precision}.rs`]=`
// Root observes register/cursor/destination bytes; original bodies are untouched.
type FoliaResourceCapture=(alloc::vec::Vec<alloc::vec::Vec<u32>>,[usize;3],alloc::vec::Vec<u8>);
std::thread_local! {static FOLIA_RESOURCES:std::cell::RefCell<Option<([[${type};${width}];8],alloc::vec::Vec<FoliaResourceCapture>)>>=const{std::cell::RefCell::new(None)};}
pub fn folia_resource_begin(input:[[${type};${width}];8]){FOLIA_RESOURCES.with(|s|{let mut s=s.borrow_mut();assert!(s.is_none());*s=Some((input,alloc::vec::Vec::new()));});}
pub fn folia_resource_end()->alloc::vec::Vec<FoliaResourceCapture>{FOLIA_RESOURCES.with(|s|s.borrow_mut().take().unwrap().1)}
pub fn folia_resource_setup(p:&mut Pipeline){
 FOLIA_RESOURCES.with(|s|{let s=s.borrow();let raw=&s.as_ref().unwrap().0;
 ${channels.map((c,i)=>`p.${c}=${high?`f32x8::from(raw[${i}].map(f32::from_bits))`:`u16x16(raw[${i}])`};`).join('\n')}
 });folia_resource_capture(p);
}
pub fn folia_resource_capture(p:&mut Pipeline){
 let registers=alloc::vec![${channels.map(c=>high?`<[f32;8]>::from(p.${c}).map(f32::to_bits).to_vec()`:`p.${c}.0.map(u32::from).to_vec()`).join(',')}];
 let data=p.${dst}.data.to_vec();
 FOLIA_RESOURCES.with(|s|s.borrow_mut().as_mut().unwrap().1.push((registers,[p.dx,p.dy,p.tail],data)));
 p.next_stage();
}
`;
}
append['src/pipeline/mod.rs']=`
// Root transport/binding only. The actual compiler selects precision/tail pointers.
pub fn folia_resources(ids:&[u8],force:bool,high:[[u32;8];8],low:[[u16;16];8],uniform:[u32;4],uniform_bytes:[u16;4],coverage:u32,dst:&[u8],real_width:usize,mask:&[u8],mask_width:u32,aa_pixels:[u8;2],aa_stride:u32,aa_shift:usize,origin:[u32;2],size:[u32;2])->(bool,usize,alloc::vec::Vec<(alloc::vec::Vec<alloc::vec::Vec<u32>>,[usize;3],alloc::vec::Vec<u8>)>,alloc::vec::Vec<u8>){
 let mut builder=RasterPipelineBuilder::new();builder.set_force_hq_pipeline(force);
 builder.ctx.uniform_color=UniformColorCtx{r:f32::from_bits(uniform[0]),g:f32::from_bits(uniform[1]),b:f32::from_bits(uniform[2]),a:f32::from_bits(uniform[3]),rgba:uniform_bytes};builder.ctx.current_coverage=f32::from_bits(coverage);
 for id in ids{builder.push(match *id{${names.map((n,i)=>`${i}=>Stage::${n}`).join(',')},_=>panic!("invalid stage")});}
 let mut compiled=builder.compile();let mut data=dst.to_vec();
 let result={let mut destination=SubPixmapMut{data:&mut data,size:tiny_skia_path::IntSize::from_wh(real_width as u32,4).unwrap(),real_width};
 let source=crate::Pixmap::new(1,1).unwrap();let rect=ScreenIntRect::from_xywh(origin[0],origin[1],size[0],size[1]).unwrap();
 let aa=AAMaskCtx{pixels:aa_pixels,stride:aa_stride,shift:aa_shift};let mask=MaskCtx{data:mask,real_width:mask_width};
 match &compiled.kind{
  RasterPipelineKind::High{functions,tail_functions}=>{
   let interleave=|f:&[highp::StageFn]|{let mut v=alloc::vec![highp::folia_resource_setup as highp::StageFn];for stage in &f[..f.len()-1]{v.push(*stage);v.push(highp::folia_resource_capture);}v.push(*f.last().unwrap());v};
   let full=interleave(functions);let tail=interleave(tail_functions);highp::folia_resource_begin(high);
   highp::start(&full,&tail,&rect,aa,mask,&mut compiled.ctx,source.as_ref(),&mut destination);(true,8,highp::folia_resource_end())
  },
  RasterPipelineKind::Low{functions,tail_functions}=>{
   let interleave=|f:&[lowp::StageFn]|{let mut v=alloc::vec![lowp::folia_resource_setup as lowp::StageFn];for stage in &f[..f.len()-1]{v.push(*stage);v.push(lowp::folia_resource_capture);}v.push(*f.last().unwrap());v};
   let full=interleave(functions);let tail=interleave(tail_functions);lowp::folia_resource_begin(low);
   lowp::start(&full,&tail,&rect,aa,mask,&mut compiled.ctx,&mut destination);(false,16,lowp::folia_resource_end())
  }
 }};(result.0,result.1,result.2,data)
}
`;
append['src/lib.rs']='\n#[allow(missing_docs)]\npub use pipeline::folia_resources;\n';
// The pattern observer uses its own append-only Source copy and transport.
// Original stage bodies, compiler, starts, pixmap validation and gathers stay intact.
if(pattern){append['src/pipeline/mod.rs']=append['src/pipeline/mod.rs']
 .replace('size:[u32;2])->','size:[u32;2],image:&[u8],dimensions:[u32;2],limits:[u32;4],spread:u8,sampler:[u32;2])->')
 .replace(' for id in ids{',' builder.ctx.limit_x=TileCtx{scale:f32::from_bits(limits[0]),inv_scale:f32::from_bits(limits[1])};builder.ctx.limit_y=TileCtx{scale:f32::from_bits(limits[2]),inv_scale:f32::from_bits(limits[3])};builder.ctx.sampler=SamplerCtx{spread_mode:match spread{0=>SpreadMode::Pad,1=>SpreadMode::Reflect,2=>SpreadMode::Repeat,_=>panic!("spread")},inv_width:f32::from_bits(sampler[0]),inv_height:f32::from_bits(sampler[1])};\n for id in ids{')
 .replace('let source=crate::Pixmap::new(1,1).unwrap();','let source=crate::PixmapRef::from_bytes(image,dimensions[0],dimensions[1]).unwrap();')
 .replace('source.as_ref(),&mut destination','source,&mut destination');}
// Separate append-only copy: observe original registers after continuation
// returns too. This exposes MaskU8 Stop state without changing any Source body.
if(terminal)for(const precision of ['highp','lowp']){
 const key=`src/pipeline/${precision}.rs`;
 append[key]=append[key].replace('});folia_resource_capture(p);','});folia_resource_capture(p);folia_resource_snapshot(p);')
  .replace('pub fn folia_resource_capture(p:&mut Pipeline){','pub fn folia_resource_capture(p:&mut Pipeline){folia_resource_snapshot(p);p.next_stage();}\npub fn folia_resource_snapshot(p:&mut Pipeline){')
  .replace(' p.next_stage();\n}\n','}\n');
}
if(!fs.existsSync(copy)){fs.cpSync(source,copy,{recursive:true});for(const[f,a]of Object.entries(append))fs.appendFileSync(path.join(copy,f),a);}
function guard(dir=''){for(const e of fs.readdirSync(path.join(source,dir),{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())guard(f);else if(e.name.endsWith('.rs'))assert.equal(fs.readFileSync(path.join(copy,f),'utf8'),fs.readFileSync(path.join(source,f),'utf8')+(append[f.split(path.sep).join('/')]??''),'append-only '+f);}}guard();
const dv=new DataView(new ArrayBuffer(4)),bits=x=>{dv.setFloat32(0,x,true);return dv.getUint32(0,true);},cases=[];
function add(ids,force,shift,length,height=1,maskMode=0){
 const values=[0,0x80000000,bits(1/255),bits(.5),bits(1),bits(-.5),bits(1.5),0x7fc12345,bits(.5/255),bits(1.5/255),bits(254.5/255)];
 const bytes=[0,1,2,127,128,254,255],origin=[3,1],real_width=40;
 const input={ids,force,high:Array.from({length:8},(_,c)=>Array.from({length:8},(_,i)=>values[(i+c+shift)%values.length])),low:Array.from({length:8},(_,c)=>Array.from({length:16},(_,i)=>bytes[(i+c+shift)%bytes.length])),uniform:[.1,.3,.7,.9].map(x=>bits(x+(shift%3)*.01)),uniform_bytes:[11,67,113,229].map(x=>(x+shift)%256),coverage:bits([0,1/255,.49999,.5,1][shift%5]),destination:Array.from({length:real_width*4*4},(_,i)=>(i*37+shift*11)%256),real_width,mask:Array.from({length:real_width*4},(_,i)=>maskMode===1?0:maskMode===2?(i===real_width+3?255:0):bytes[(i+shift)%bytes.length]),mask_width:real_width,aa_pixels:[bytes[shift%7],bytes[(shift+3)%7]],aa_stride:shift%2?1:0,aa_shift:0,origin,size:[length,height]};
 input.aa_shift=input.aa_stride*origin[1]+origin[0]-(shift%3===2?1:0);
 // Compositions ending in fused source-over use valid premultiplied paints.
 if(ids.includes(45)){input.high=input.high.map((row,c)=>row.map((_,i)=>bits(c===3||c===7?1:(i+c+shift)%8/8)));}
 cases.push(input);
}
if(!blend&&!pattern){
for(const force of [false,true])for(const id of highStages)for(let shift=0;shift<11;shift++)for(const length of [1,2,7,8,9,15,16,17,33])add([id],force,shift,length,shift%4===0?2:1,id===13?shift%3:0);
for(const id of [9,10,12])for(let shift=0;shift<11;shift++)for(const length of [1,2,7,8,9,15,16,17,33])add([id],false,shift,length,shift%4===0?2:1);
const sequences=[[5,7,17,8],[5,13,45],[5,7,15,8],[5,14,45],[5,16,45],[7,1,8],[9,1,10],[12,9,15,10],[5,13,8]];
for(const force of [false,true])for(const ids of sequences){if(force&&ids.some(id=>[9,10,12].includes(id)))continue;for(let shift=0;shift<11;shift++)for(const length of [1,2,8,16,17])add(ids,force,shift,length,1,ids.includes(13)?shift%3:0);}
}else if(blend){
 function addBlend(ids,force,shift,length){add(ids,force,shift,length,shift%4===0?2:1);const input=cases.at(-1),restricted=ids.some(id=>id===37||id===39);
  const alphas=[0,1,2,64,127,128,254,255],ratios=[0,.125,.25,.375,.5,.625,.75,.875,1];
  input.low=Array.from({length:8},(_,c)=>Array.from({length:16},(_,lane)=>{const bank=c<4?0:1,a=restricted?64+((lane+shift+bank*3)%3)*32:alphas[(lane+shift+bank*3)%alphas.length];return c%4===3?a:Math.floor(a*(restricted?[.3125,.5,.6875][(lane+c+shift)%3]:ratios[(lane+c+shift)%ratios.length]));}));
  input.high=Array.from({length:8},(_,c)=>Array.from({length:8},(_,lane)=>{const bank=c<4?0:1,a=alphas[(lane+shift+bank*3)%alphas.length]/255;return bits(c%4===3?a:a*ratios[(lane+c+shift)%ratios.length]);}));
 }
 for(const force of [false,true])for(const id of highStages)for(let shift=0;shift<11;shift++)for(const length of [1,7,8,9,15,16,17])addBlend([id],force,shift,length);
 for(const force of [false,true])for(const ids of [[18,25,29],[27,28,30],[34,35,36],[37,39],[32,33,40],[41,42,43,44]])for(let shift=0;shift<11;shift++)for(const length of [1,8,16,17])addBlend(ids,force,shift,length);
}else{
 function addPattern(ids,force,shift,length,spread,dimensions){add(ids,force,shift,length,shift%4===0?2:1);const c=cases.at(-1),[w,h]=dimensions,coordinates=[0,0x80000000,bits(-2.75),bits(-.5),bits(.4999),bits(.5),bits(1.5),bits(w-.5),bits(w+1),bits(77.75),0x7fc12345,0x7f800000,0xff800000];
  c.high[0]=Array.from({length:8},(_,i)=>coordinates[(i+shift)%coordinates.length]);c.high[1]=Array.from({length:8},(_,i)=>coordinates[(i+shift*3+1)%coordinates.length]);
  c.image=Array.from({length:w*h*4},(_,i)=>{const pixel=i>>2,alpha=[0,1,63,127,128,254,255][(pixel+shift)%7];return i%4===3?alpha:Math.floor(alpha*((pixel+i+shift)%8)/7);});c.dimensions=dimensions;c.spread=spread;
  c.limits=[w,shift%3===1?0:1/(w+(shift%3===2?.25:0)),h,shift%3===1?0:1/(h+(shift%3===2?.25:0))].map(bits);c.sampler=[1/(w+(shift%3===2?1:0)),1/(h+(shift%3===2?1:0))].map(bits);
 }
 const dims=[[1,1],[3,2],[7,3],[17,5]];
 for(const force of [false,true])for(const id of highStages)for(const dimensions of dims)for(let shift=0;shift<8;shift++)for(const length of [1,8,9,17])for(const spread of id>=49?[0,1,2]:[0])addPattern([id],force,shift,length,spread,dimensions);
 for(const force of [false,true])for(const ids of [[47,11],[48,11],[49,2,3],[50,2,3],[6,46,11]])for(const dimensions of dims)for(let shift=0;shift<8;shift++)for(const length of [1,8,17])for(const spread of ids[0]>=49?[0,1,2]:[0])addPattern(ids,force,shift,length,spread,dimensions);
}
const activeCases=terminal?cases.filter(c=>c.ids.includes(13)):cases;
const manifest=path.join(__dirname,stem+'-reference/Cargo.toml'),env={...process.env,CARGO_TARGET_DIR:path.join(root,'build/mermaid-'+stem+'-reference-target')};assert.equal(env.RUSTFLAGS??'','');assert.equal(env.CARGO_ENCODED_RUSTFLAGS??'','');fs.mkdirSync(work,{recursive:true});
if(!fs.existsSync(path.join(path.dirname(manifest),'Cargo.lock')))cp.execFileSync('cargo',['generate-lockfile','--offline','--manifest-path',manifest],{env,stdio:'pipe'});
const built=cp.spawnSync('cargo',['build','--locked','--offline','--manifest-path',manifest],{env,encoding:'utf8',timeout:180000});fs.writeFileSync(path.join(work,'build.log'),(built.stdout||'')+(built.stderr||''));assert.equal(built.status,0,'Original Source must compile');
const run=cp.spawnSync(path.join(env.CARGO_TARGET_DIR,'debug/folia-resource-stage-source-reference.exe'),[],{input:JSON.stringify(activeCases),encoding:'utf8',maxBuffer:128*1024*1024,timeout:60000});fs.writeFileSync(path.join(work,'stderr.log'),run.stderr||'');assert.equal(run.status,0,'Source must finish');const observed=JSON.parse(run.stdout);assert.deepEqual(observed.map(c=>c.input),activeCases);guard();
// Pure blends have no resource API. Verify every original byte capture before
// removing repetitive byte buffers from their register-only golden projection.
let golden=observed;if(blend||pattern){golden=observed.map(c=>{for(const capture of c.result[2])assert.deepEqual(capture[2],c.input.destination);assert.deepEqual(c.result[3],c.input.destination);const {ids,force,high,low,origin,size,image,dimensions,limits,spread,sampler}=c.input;return {input:{ids,force,high,low,origin,size,...(pattern?{image,dimensions,limits,spread,sampler}:{})},result:[c.result[0],c.result[1],c.result[2].map(([r,cursor])=>[r,cursor])]};});}
const data=JSON.stringify({schema:1,scope:pattern?'actual original pattern compile/start/pixmap/gather; compact raw register/cursor projection after checking unchanged destination bytes; source program producer acceptance separate':blend?'actual original blend compile/start pointers; register-only projection after verifying all original destination captures unchanged; bounded premultiplied inputs; low HardLight/Overlay inputs avoid checked intermediate overflow':'actual original resource stage compile/start pointers; captures include invocation setup, stage continuation and destination bytes',pins,high_stages:highStages,low_stages:lowStages,names,...(terminal?{terminal:true}:{}),cases:golden})+'\n',out=path.join(root,'third_party/mermaid-rs-renderer/tests/fixtures/'+(pattern?'pattern-stages':blend?'blend-stages-compact':terminal?'resource-terminal':'resource-stages')+'.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}
console.log(JSON.stringify({cases:activeCases.length,terminal,high:observed.filter(c=>c.result[0]).length,low:observed.filter(c=>!c.result[0]).length,captures:observed.reduce((n,c)=>n+c.result[2].length,0),fixture_sha256:hash(data)}));
