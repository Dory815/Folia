//! Original RadialGradient context setup and stage dispatch to the common builder.
use super::{gradient_stop_types::PreparedGradient,gradient_program_types::ColorSpace,radial_types::RadialKind,pipeline_builder::PipelineBuilder};
pub fn push_radial_gradient_stages(gradient:&PreparedGradient,kind:RadialKind,color_space:ColorSpace,builder:&mut PipelineBuilder)->bool{
 let plan=super::radial_stages::prepare_radial_stages(kind);
 builder.context.radial_mask=[0;8];
 builder.context.radial_p=[plan.p0,plan.p1];
 super::gradient_program::push_gradient_stages(gradient,color_space,Some(&plan),builder)
}

pub fn new_radial_gradient(_start:tiny_skia_path::Point,_start_radius:f32,_end:tiny_skia_path::Point,_end_radius:f32,_stops:Vec<super::gradient_stop_types::PreparedGradientStop>,_spread:crate::scene::paint::SpreadMode,_transform:tiny_skia_path::Transform)->Option<super::prepared_shader_types::PreparedShader<'static>>{panic!("Unimplemented Root-frozen new_radial_gradient")}
