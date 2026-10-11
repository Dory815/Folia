// Root actual callable stub, to be replaced only after Source red execution.
use super::super::{HighRegisters,SamplingError};
use super::super::super::pipeline_control_types::{PipelineStage,PipelinePrecision};
pub fn blend_high_stage(stage:PipelineStage,_registers:&mut HighRegisters)->Result<(),SamplingError>{
 Err(SamplingError{precision:PipelinePrecision::High,stage})
}
