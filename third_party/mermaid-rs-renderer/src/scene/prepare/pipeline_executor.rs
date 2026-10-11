//! Root-frozen real stub for the original RasterPipeline::run/start boundary.
use tiny_skia_path::IntRect;
use super::{pipeline_control_types::{CompiledPipeline,PipelinePrecision},pipeline_context_types::PipelineContext,pipeline_executor_types::{PipelineResources,PipelineRunError}};
mod highp;
mod lowp;
pub fn run_pipeline(pipeline:&mut CompiledPipeline<PipelineContext>,rect:&IntRect,resources:PipelineResources<'_,'_,'_>)->Result<(),PipelineRunError>{
 match pipeline.precision{
  PipelinePrecision::High=>highp::start(pipeline,rect,resources),
  PipelinePrecision::Low=>lowp::start(pipeline,rect,resources),
 }
}
