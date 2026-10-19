// Append-only readers beside original private blitter and compiled pipeline fields.
'use strict';
const assert=require('node:assert/strict');
const run=require('./shader-run-observer.cjs');
const pipeline=run['src/pipeline/mod.rs'];
const begin=pipeline.indexOf('fn folia_shader_context('),end=pipeline.indexOf('pub fn folia_shader_run',begin);
assert.ok(begin>=0&&end>begin);
const contextFunction=pipeline.slice(begin,end);
assert.ok(contextFunction.includes('assert_eq!(result.pop(),Some(alloc::vec![0]))'));
module.exports={
 'src/pipeline/highp.rs':run['src/pipeline/highp.rs'],
 'src/pipeline/lowp.rs':run['src/pipeline/lowp.rs'],
 'src/pipeline/mod.rs':`
${contextFunction}
#[allow(missing_docs)] pub struct FoliaConstructorPipeline{pub high:bool,pub width:usize,pub full:alloc::vec::Vec<(u8,bool)>,pub tail:alloc::vec::Vec<(u8,bool)>,pub context:alloc::vec::Vec<alloc::vec::Vec<u32>>}
impl RasterPipeline{pub fn folia_constructor_snapshot(&self)->FoliaConstructorPipeline{
 let (high,width,full,tail)=match &self.kind{
 RasterPipelineKind::High{functions,tail_functions}=>(true,8,functions.iter().map(|f|highp::folia_shader_function_id(*f)).collect(),tail_functions.iter().map(|f|highp::folia_shader_function_id(*f)).collect()),
 RasterPipelineKind::Low{functions,tail_functions}=>(false,16,functions.iter().map(|f|lowp::folia_shader_function_id(*f)).collect(),tail_functions.iter().map(|f|lowp::folia_shader_function_id(*f)).collect()),
 };FoliaConstructorPipeline{high,width,full,tail,context:folia_shader_context(&self.ctx)}
}}
#[allow(missing_docs)] pub use blitter::{folia_blitter_constructor,folia_blitter_mask_constructor,FoliaConstructorSnapshot};
`,
 'src/pipeline/blitter.rs':`
#[allow(missing_docs)] pub struct FoliaConstructorSnapshot{
 pub is_mask:bool,
 pub mask:Option<(u32,u32,u32,usize,usize)>,
 pub target:(u32,u32,usize,usize,usize),
 pub source:(u32,u32,alloc::vec::Vec<u8>,bool),
 pub memset:Option<[u8;4]>,
 pub anti_h:super::FoliaConstructorPipeline,
 pub rect:super::FoliaConstructorPipeline,
 pub mask_pipeline:super::FoliaConstructorPipeline,
}
impl RasterPipelineBlitter<'_,'_>{
 fn folia_constructor_snapshot(&self,target_offset:usize,mask_offset:usize,image:&[u8])->FoliaConstructorSnapshot{
  let mask=self.mask.map(|m|(m.size.width(),m.size.height(),m.real_width,mask_offset,m.data.len()));
  let target=(self.pixmap.size.width(),self.pixmap.size.height(),self.pixmap.real_width,target_offset,self.pixmap.data.len());
  let source=(self.pixmap_src.width(),self.pixmap_src.height(),self.pixmap_src.data().to_vec(),self.pixmap_src.data().as_ptr()==image.as_ptr());
  let memset=self.memset2d_color.map(|c|[c.red(),c.green(),c.blue(),c.alpha()]);
  FoliaConstructorSnapshot{is_mask:self.is_mask,mask,target,source,memset,
   anti_h:self.blit_anti_h_rp.folia_constructor_snapshot(),rect:self.blit_rect_rp.folia_constructor_snapshot(),mask_pipeline:self.blit_mask_rp.folia_constructor_snapshot()}
 }
}
#[allow(missing_docs)] pub fn folia_blitter_constructor(paint:&Paint<'_>,target:&mut [u8],width:u32,height:u32,target_rect:Option<crate::IntRect>,mask:Option<(&crate::Mask,Option<crate::IntRect>)>,image:&[u8])->Result<Option<FoliaConstructorSnapshot>,&'static str>{
 let mut full=crate::PixmapMut::from_bytes(target,width,height).ok_or("target")?;
 let base=full.data_mut().as_ptr() as usize;
 let mut view=match target_rect{Some(r)=>full.subpixmap(r).ok_or("target_rect")?,None=>full.as_subpixmap()};
 let target_offset=view.data.as_ptr() as usize-base;
 let (mask_view,mask_offset)=match mask{Some((m,r))=>{let base=m.data().as_ptr() as usize;let view=match r{Some(r)=>m.submask(r).ok_or("mask_rect")?,None=>m.as_submask()};let offset=view.data.as_ptr() as usize-base;(Some(view),offset)},None=>(None,0)};
 let result=RasterPipelineBlitter::new(paint,mask_view,&mut view);
 Ok(result.map(|b|b.folia_constructor_snapshot(target_offset,mask_offset,image)))
}
#[allow(missing_docs)] pub fn folia_blitter_mask_constructor(target:&mut crate::Mask,target_rect:Option<crate::IntRect>)->Result<Option<FoliaConstructorSnapshot>,&'static str>{
 let base=target.data().as_ptr() as usize;
 let mut view=match target_rect{Some(r)=>target.subpixmap(r).ok_or("target_rect")?,None=>target.as_subpixmap()};
 let offset=view.data.as_ptr() as usize-base;
 let result=RasterPipelineBlitter::new_mask(&mut view);
 Ok(result.map(|b|b.folia_constructor_snapshot(offset,0,&[])))
}
`,
 'src/lib.rs':'\n#[allow(missing_docs)] pub use pipeline::{folia_blitter_constructor,folia_blitter_mask_constructor,FoliaConstructorSnapshot,FoliaConstructorPipeline};\n',
};
