//! Root frozen entry point; Source red tests precede implementation.
use super::PatternContext;
use super::super::{HighRegisters,SamplingError};
use super::super::super::pipeline_control_types::{PipelineStage,PipelinePrecision};
pub fn pattern_high_stage(stage:PipelineStage,_registers:&mut HighRegisters,_context:&PatternContext<'_>)->Result<(),SamplingError>{
 Err(SamplingError{precision:PipelinePrecision::High,stage})
}
