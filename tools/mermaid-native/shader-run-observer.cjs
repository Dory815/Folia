// Root append-only Source producer -> same builder -> unchanged compile/run witness.
'use strict';
const assert=require('node:assert/strict'),fs=require('node:fs'),path=require('node:path');
const source=path.join(process.env.USERPROFILE,'.cargo/registry/src/index.crates.io-1949cf8c6b5b557f/tiny-skia-0.12.0');
const pattern=require('./pattern-producer-observer.cjs'),gradient=require('./gradient-shader-observer.cjs');
const names=fs.readFileSync(path.join(source,'src/pipeline/mod.rs'),'utf8').match(/pub enum Stage \{([\s\S]*?)\n\}/)[1].split('\n').map(s=>s.trim().replace(/ = 0/,'').replace(/,$/,'')).filter(Boolean);assert.equal(names.length,79);
const append={...gradient};
// Reuse precisely the existing complete state projection; the sixteenth group
// is the builder force flag, so compiled context snapshots omit that group.
let context=pattern['src/pipeline/mod.rs'].split(' pub fn folia_pattern_context(&self)->alloc::vec::Vec<alloc::vec::Vec<u32>>{\n')[1].split('\n }\n pub fn folia_pattern_sentinel')[0];
assert.ok(context.includes('let c=&self.ctx;'));context=context.replace('let c=&self.ctx;','').replace('u32::from(self.force_hq_pipeline)','0');
append['src/pipeline/mod.rs']+=`
pub type FoliaShaderRun=(bool,usize,alloc::vec::Vec<(u8,bool)>,alloc::vec::Vec<(u8,bool)>,bool,alloc::vec::Vec<u8>,alloc::vec::Vec<alloc::vec::Vec<u32>>,alloc::vec::Vec<alloc::vec::Vec<u32>>);
fn folia_shader_context(c:&Context)->alloc::vec::Vec<alloc::vec::Vec<u32>>{let mut result={${context}};assert_eq!(result.pop(),Some(alloc::vec![0]));result}
pub fn folia_shader_run(mut b:RasterPipelineBuilder,ids:&[u8],source:PixmapRef,dst:&[u8],real_width:usize,mask:&[u8],mask_width:u32,aa_pixels:[u8;2],aa_stride:u32,aa_shift:usize,origin:[u32;2],size:[u32;2])->FoliaShaderRun{
 for id in ids{b.push(match *id{${names.map((n,i)=>`${i}=>Stage::${n}`).join(',')},_=>panic!("bad Source terminal")});}
 let mut compiled=b.compile();let (high,width,full,tail)=match &compiled.kind{
 RasterPipelineKind::High{functions,tail_functions}=>(true,8,functions.iter().map(|f|highp::folia_shader_function_id(*f)).collect(),tail_functions.iter().map(|f|highp::folia_shader_function_id(*f)).collect()),
 RasterPipelineKind::Low{functions,tail_functions}=>(false,16,functions.iter().map(|f|lowp::folia_shader_function_id(*f)).collect(),tail_functions.iter().map(|f|lowp::folia_shader_function_id(*f)).collect()),
 };
 let before=folia_shader_context(&compiled.ctx);let mut data=dst.to_vec();
 let panicked={let mut destination=SubPixmapMut{data:&mut data,size:tiny_skia_path::IntSize::from_wh(real_width as u32,4).unwrap(),real_width};
 let rect=ScreenIntRect::from_xywh(origin[0],origin[1],size[0],size[1]).unwrap();
 std::panic::catch_unwind(std::panic::AssertUnwindSafe(||compiled.run(&rect,AAMaskCtx{pixels:aa_pixels,stride:aa_stride,shift:aa_shift},MaskCtx{data:mask,real_width:mask_width},source,&mut destination))).is_err()};
 (high,width,full,tail,panicked,data,before,folia_shader_context(&compiled.ctx))
}
`;
const tails=[['LoadDestination','load_dst_tail'],['Store','store_tail'],['LoadDestinationU8','load_dst_u8_tail'],['StoreU8','store_u8_tail'],['SourceOverRgba','source_over_rgba_tail']];
for(const precision of ['highp','lowp'])append[`src/pipeline/${precision}.rs`]=`
pub fn folia_shader_function_id(fun:StageFn)->(u8,bool){
 if fn_ptr(fun)==fn_ptr(just_return){return(79,false);}
 ${tails.map(([name,fun])=>`if fn_ptr(fun)==fn_ptr(${fun}){return(${names.indexOf(name)},true);}`).join('\n')}
 for(i,f)in STAGES.iter().enumerate(){if fn_ptr(fun)==fn_ptr(*f){return(i as u8,false);}}panic!("unknown Source function pointer");
}
`;
const resourceArgs='ids:&[u8],dst:&[u8],real_width:usize,mask:&[u8],mask_width:u32,aa_pixels:[u8;2],aa_stride:u32,aa_shift:usize,origin:[u32;2],size:[u32;2]';
const runArgs='ids,source.as_ref(),dst,real_width,mask,mask_width,aa_pixels,aa_stride,aa_shift,origin,size';
for(const kind of ['linear','radial']){
 const file=`src/shaders/${kind}_gradient.rs`;let text=gradient[file];
 text=text.replace(`folia_${kind}_producer(`,`folia_${kind}_run(`);
 const old='force:bool)->Option<(alloc::string::String,Vec<alloc::string::String>,Vec<Vec<u32>>)>';
 assert.ok(text.includes(old));text=text.replace(old,`force:bool,${resourceArgs})->Option<((alloc::string::String,Vec<alloc::string::String>,Vec<Vec<u32>>),Option<crate::pipeline::FoliaShaderRun>)>`);
 const oldTail=' Some((match outcome{Ok(v)=>alloc::format!("{}",v),Err(_)=>alloc::string::String::from("panic")},p.folia_pattern_stages(),p.folia_pattern_context()))';assert.ok(text.includes(oldTail));
 text=text.replace(oldTail,` let accepted=matches!(&outcome,Ok(true));let record=(match outcome{Ok(v)=>alloc::format!("{}",v),Err(_)=>alloc::string::String::from("panic")},p.folia_pattern_stages(),p.folia_pattern_context());
 let run=if accepted{let source=crate::Pixmap::new(1,1).unwrap();Some(crate::pipeline::folia_shader_run(p,${runArgs}))}else{None};Some((record,run))`);
 append[file]=text;
}
let text=pattern['src/shaders/pattern.rs'].replace('folia_pattern_producer(','folia_pattern_run(');
const old='force:bool)->(alloc::string::String,alloc::vec::Vec<u32>,alloc::vec::Vec<alloc::string::String>,alloc::vec::Vec<alloc::vec::Vec<u32>>,bool)';assert.ok(text.includes(old));
text=text.replace(old,`force:bool,image:&[u8],${resourceArgs})->((alloc::string::String,alloc::vec::Vec<u32>,alloc::vec::Vec<alloc::string::String>,alloc::vec::Vec<alloc::vec::Vec<u32>>,bool),Option<crate::pipeline::FoliaShaderRun>)`);
text=text.replace('let data=alloc::vec![0u8;(width as usize)*(height as usize)*4];','let data=image;');
text=text.replace('let result=match outcome','let accepted=matches!(&outcome,Ok(true));let result=match outcome');
const oldTail=' (result,params,p.folia_pattern_stages(),p.folia_pattern_context(),pattern.pixmap.data().as_ptr()==data.as_ptr())';assert.ok(text.includes(oldTail));
text=text.replace(oldTail,` let record=(result,params,p.folia_pattern_stages(),p.folia_pattern_context(),pattern.pixmap.data().as_ptr()==data.as_ptr());
 let run=if accepted{Some(crate::pipeline::folia_shader_run(p,ids,pixmap,dst,real_width,mask,mask_width,aa_pixels,aa_stride,aa_shift,origin,size))}else{None};(record,run)`);
append['src/shaders/pattern.rs']=text;
append['src/shaders/mod.rs']='\n#[allow(missing_docs)] pub use linear_gradient::folia_linear_run;\n#[allow(missing_docs)] pub use radial_gradient::folia_radial_run;\n#[allow(missing_docs)] pub use pattern::folia_pattern_run;\n';
append['src/lib.rs']='\n#[allow(missing_docs)] pub use shaders::{folia_linear_run,folia_radial_run,folia_pattern_run};\n';
module.exports=append;
