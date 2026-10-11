//! Original LinearGradient stage dispatch, using the single common builder.
use super::{gradient_stop_types::PreparedGradient,gradient_program_types::ColorSpace,pipeline_builder::PipelineBuilder};
pub fn push_linear_gradient_stages(gradient:&PreparedGradient,color_space:ColorSpace,builder:&mut PipelineBuilder)->bool{
 super::gradient_program::push_gradient_stages(gradient,color_space,None,builder)
}
