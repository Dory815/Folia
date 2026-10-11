//! Root-frozen typed registers, with one mutable shared GradientProgram context.
use super::super::pipeline_control_types::{PipelineStage,PipelinePrecision};
/// Source register order r,g,b,a,dr,dg,db,da. Eight f32 lanes per channel.
#[derive(Debug,Clone,Copy,Default)]
pub struct HighRegisters {pub channels:[[f32;8];8]}
/// The same register order. Sixteen raw u16 lanes; coordinate phase is bit-packed.
#[derive(Debug,Clone,Copy,Default,PartialEq,Eq)]
pub struct LowRegisters {pub channels:[[u16;16];8]}
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub struct SamplingError {pub precision:PipelinePrecision,pub stage:PipelineStage}
