// Root actual callable stub, to be replaced only after Source red execution.
use super::super::{LowRegisters,SamplingError};
use super::super::super::pipeline_control_types::{PipelineStage,PipelinePrecision};
pub fn blend_low_stage(stage:PipelineStage,_registers:&mut LowRegisters)->Result<(),SamplingError>{
 Err(SamplingError{precision:PipelinePrecision::Low,stage})
}
