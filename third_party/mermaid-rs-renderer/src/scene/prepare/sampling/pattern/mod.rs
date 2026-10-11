//! Borrowed image sampling stages shared by the scene preparation pipeline.
mod types;
mod highp;
pub use types::{SourcePixmap,TileContext,SamplerContext,PatternContext};
pub use highp::pattern_high_stage;
use super::{LowRegisters,SamplingError};
use super::super::pipeline_control_types::{PipelineStage,PipelinePrecision};
/// The original lowp pointer table has no image sampling stages.
pub fn pattern_low_stage(stage:PipelineStage,_registers:&mut LowRegisters,_context:&PatternContext<'_>)->Result<(),SamplingError>{
 Err(SamplingError{precision:PipelinePrecision::Low,stage})
}
