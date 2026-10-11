//! Root-frozen stub of original lowp::start; numerical stages remain separate.
use tiny_skia_path::IntRect;
use super::super::{pipeline_control_types::CompiledPipeline,pipeline_context_types::PipelineContext,pipeline_executor_types::{PipelineResources,PipelineRunError}};
pub(super) fn start(_pipeline:&mut CompiledPipeline<PipelineContext>,_rect:&IntRect,_resources:PipelineResources<'_,'_,'_>)->Result<(),PipelineRunError>{
 Err(PipelineRunError::Unimplemented)
}
