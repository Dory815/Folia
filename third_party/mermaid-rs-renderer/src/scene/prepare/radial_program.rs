//! Original RadialGradient context setup and stage dispatch to the common builder.
use super::{gradient_stop_types::PreparedGradient,gradient_program_types::ColorSpace,radial_types::RadialKind,pipeline_builder::PipelineBuilder};
pub fn push_radial_gradient_stages(_gradient:&PreparedGradient,_kind:RadialKind,_color_space:ColorSpace,_builder:&mut PipelineBuilder)->bool{
 panic!("Unimplemented Root-frozen RadialGradient producer wrapper");
}
