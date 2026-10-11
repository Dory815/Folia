//! Source Shader variant methods; one shared builder passed by its caller.
use tiny_skia_path::Transform;
use super::{prepared_shader_types::PreparedShader,gradient_program_types::ColorSpace,pipeline_builder::PipelineBuilder};
impl PreparedShader<'_>{
 pub fn is_opaque(&self)->bool{panic!("Unimplemented Root-frozen Shader::is_opaque")}
 pub fn transform(&mut self,_transform:Transform){panic!("Unimplemented Root-frozen Shader::transform")}
 pub fn apply_opacity(&mut self,_opacity:f32){panic!("Unimplemented Root-frozen Shader::apply_opacity")}
 pub fn push_stages(&self,_color_space:ColorSpace,_builder:&mut PipelineBuilder)->bool{panic!("Unimplemented Root-frozen Shader::push_stages")}
}
