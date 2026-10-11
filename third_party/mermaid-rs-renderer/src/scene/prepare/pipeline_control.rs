//! Root stubs: original compile control must fail the independent tests first.
use super::pipeline_control_types::*;
pub fn pipeline_stage_is_lowp(_:PipelineStage)->bool{panic!("M3i1 eligibility stub")}
pub fn compile_pipeline<C:Default>(_:&[PipelineStage],_:bool,_:C)->CompiledPipeline<C>{panic!("M3i1 compiler stub")}
