//! Original LinearGradient stage dispatch, using the single common builder.
use super::{gradient_stop_types::PreparedGradient,gradient_program_types::ColorSpace,pipeline_builder::PipelineBuilder};
pub fn push_linear_gradient_stages(gradient:&PreparedGradient,color_space:ColorSpace,builder:&mut PipelineBuilder)->bool{
 super::gradient_program::push_gradient_stages(gradient,color_space,None,builder)
}

pub fn new_linear_gradient(_start:tiny_skia_path::Point,_end:tiny_skia_path::Point,_stops:Vec<super::gradient_stop_types::PreparedGradientStop>,_spread:crate::scene::paint::SpreadMode,_transform:tiny_skia_path::Transform)->Option<super::prepared_shader_types::PreparedShader<'static>>{panic!("Unimplemented Root-frozen new_linear_gradient")}
