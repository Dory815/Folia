//! Root-frozen immutable projection of PixmapRef, TileCtx and SamplerCtx.
pub use super::super::super::pipeline_context_types::{TileContext,SamplerContext};
use super::super::super::pipeline_context_types::PipelineContext;
#[derive(Debug,Clone,Copy)]
pub struct SourcePixmap<'a>{data:&'a[u8],width:u32,height:u32}
impl<'a> SourcePixmap<'a>{
 /// Source PixmapRef::from_bytes validates contiguous RGBA storage, permitting extra bytes.
 /// Bytes must already be premultiplied; this constructor does not transform them.
 pub fn from_bytes(data:&'a[u8],width:u32,height:u32)->Option<Self>{
  if width==0||height==0{return None;}
  let row_bytes=i32::try_from(width).ok()?.checked_mul(4)?;
  let size=((height-1)as usize).checked_mul(row_bytes as usize)?.checked_add((width as usize).checked_mul(4)?)?;
  if data.len()<size{return None;}
  Some(Self{data,width,height})
 }
 pub fn data(&self)->&'a[u8]{self.data}
 pub fn width(&self)->u32{self.width}
 pub fn height(&self)->u32{self.height}
}
#[derive(Debug,Clone,Copy)]
pub struct PatternContext<'source,'context>{pub source:SourcePixmap<'source>,pub pipeline:&'context PipelineContext}
