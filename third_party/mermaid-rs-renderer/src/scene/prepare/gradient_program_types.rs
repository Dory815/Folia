//! Root-frozen shared gradient wrapper context. Neither consumer recomputes it.
use tiny_skia_path::Transform;
use super::radial_stage_types::RadialStage;
pub use super::pipeline_context_types::{GradientColor,TwoStopContext,GradientContext};

#[derive(Debug,Clone,Copy,PartialEq,Eq,Default)]
pub enum ColorSpace { #[default] Linear, Gamma2, SimpleSRGB, FullSRGBGamma }

#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub enum GradientStage {
 SeedShader,Transform,Radial(RadialStage),PadX1,ReflectX1,RepeatX1,
 EvenlySpaced2StopGradient,Gradient,Premultiply,
}

#[derive(Debug,Clone,PartialEq)]
pub struct GradientProgram {
 pub returned:bool,pub stages:Vec<GradientStage>,pub transform:Transform,
 pub two_stop:TwoStopContext,pub gradient:GradientContext,
 pub radial_p:[f32;2],pub radial_mask:[u32;8],
}
