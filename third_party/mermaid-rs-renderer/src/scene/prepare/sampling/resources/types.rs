//! Root-frozen borrowed resource projection of tiny-skia pipeline Context.
use super::super::super::pipeline_control_types::{PipelinePrecision,PipelineStage};
pub use super::super::super::pipeline_context_types::UniformColor;
#[derive(Debug)]
pub struct DestinationSurface<'a> {pub data:&'a mut [u8],pub real_width:usize}
#[derive(Debug,Clone,Copy,Default)]
pub struct MaskSurface<'a> {pub data:&'a [u8],pub real_width:u32}
#[derive(Debug,Clone,Copy,Default,PartialEq,Eq)]
pub struct AaMask {pub pixels:[u8;2],pub stride:u32,pub shift:usize}
#[derive(Debug)]
pub struct ResourceContext<'dst,'mask> {
 pub destination:DestinationSurface<'dst>,pub mask:MaskSurface<'mask>,
 pub aa_mask:AaMask,pub uniform_color:UniformColor,pub current_coverage:f32,
}
/// Selects the actual full/tail function family emitted by the shared compiler.
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub enum ResourceProgram {Full,Tail}
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub struct ResourceCursor {pub dx:usize,pub dy:usize,pub tail:usize,pub program:ResourceProgram}
/// MaskU8 may terminate a pipeline invocation before its continuation executes.
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub enum StageControl {Continue,Stop}
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub enum ResourceStageError {
 Unimplemented{precision:PipelinePrecision,stage:PipelineStage},
 Unsupported{precision:PipelinePrecision,stage:PipelineStage},
}
