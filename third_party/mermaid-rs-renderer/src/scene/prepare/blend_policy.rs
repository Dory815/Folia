//! Original BlendMode policy used by blitter composition.
use super::{blend_policy_types::RasterBlendMode,pipeline_control_types::PipelineStage};
impl RasterBlendMode{
 pub fn should_pre_scale_coverage(self)->bool{panic!("Unimplemented Root-frozen BlendMode::should_pre_scale_coverage")}
 pub fn to_stage(self)->Option<PipelineStage>{panic!("Unimplemented Root-frozen BlendMode::to_stage")}
}
