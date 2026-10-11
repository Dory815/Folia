//! Root real red boundary; Source bodies are delegated only after execution.
use super::{ResourceContext,ResourceCursor,StageControl,ResourceStageError};
use super::super::HighRegisters;
use super::super::super::pipeline_control_types::{PipelinePrecision,PipelineStage};
pub fn resource_high_stage(stage:PipelineStage,_registers:&mut HighRegisters,
 _context:&mut ResourceContext<'_,'_>,_cursor:ResourceCursor)->Result<StageControl,ResourceStageError>{
 Err(ResourceStageError::Unimplemented{precision:PipelinePrecision::High,stage})
}
