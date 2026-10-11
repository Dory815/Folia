//! Root-frozen invocation resources. The compiled pipeline remains the sole context owner.
use super::pipeline_control_types::{PipelinePrecision,PipelineStage};
use super::sampling::{pattern::SourcePixmap,resources::{DestinationSurface,MaskSurface,AaMask}};
#[derive(Debug)]
pub struct PipelineResources<'source,'dst,'mask>{
 pub source:SourcePixmap<'source>,pub destination:DestinationSurface<'dst>,
 pub mask:MaskSurface<'mask>,pub aa_mask:AaMask,
}
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub enum PipelineRunError{
 Unimplemented,
 Unsupported{precision:PipelinePrecision,stage:PipelineStage},
}
