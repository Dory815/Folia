//! Original LinearGradient stage dispatch, using the single common builder.
use super::{gradient_stop_types::PreparedGradient,gradient_program_types::ColorSpace,pipeline_builder::PipelineBuilder};
pub fn push_linear_gradient_stages(_gradient:&PreparedGradient,_color_space:ColorSpace,_builder:&mut PipelineBuilder)->bool{
 panic!("Unimplemented Root-frozen LinearGradient producer wrapper");
}
