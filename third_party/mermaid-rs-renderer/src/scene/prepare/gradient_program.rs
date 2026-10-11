//! Root-frozen declarations. Stub establishes a failing baseline before delegation.
use super::{gradient_program_types::{ColorSpace,GradientProgram},gradient_stop_types::PreparedGradient,radial_stage_types::RadialStagePlan};
pub fn prepare_gradient_program(_gradient:&PreparedGradient,_color_space:ColorSpace,_radial:Option<&RadialStagePlan>)->GradientProgram {unimplemented!("M3h complete Source Gradient::push_stages")}
