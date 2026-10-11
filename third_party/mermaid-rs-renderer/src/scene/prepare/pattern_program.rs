//! Selected original Pattern and Shader::Pattern preparation methods.
use tiny_skia_path::Transform;
use crate::scene::paint::SpreadMode;
use super::{pattern_types::{PreparedPattern,FilterQuality},pipeline_builder::PipelineBuilder,gradient_program_types::ColorSpace,sampling::pattern::SourcePixmap};
impl<'source> PreparedPattern<'source>{
    pub fn new(_source:SourcePixmap<'source>,_spread:SpreadMode,_quality:FilterQuality,_opacity:f32,_transform:Transform)->Self{
        panic!("Unimplemented Root-frozen Pattern constructor");
    }
    pub fn transform(&mut self,_transform:Transform){panic!("Unimplemented Root-frozen Pattern transform");}
    pub fn apply_opacity(&mut self,_opacity:f32){panic!("Unimplemented Root-frozen Pattern opacity");}
    pub fn push_stages(&self,_color_space:ColorSpace,_builder:&mut PipelineBuilder)->bool{
        panic!("Unimplemented Root-frozen Pattern producer");
    }
}
