//! Root-frozen shared gradient wrapper context. Neither consumer recomputes it.
use tiny_skia_path::{NormalizedF32,Transform};
use super::radial_stage_types::RadialStage;

#[derive(Debug,Clone,Copy,PartialEq,Eq,Default)]
pub enum ColorSpace { #[default] Linear, Gamma2, SimpleSRGB, FullSRGBGamma }

/// Unpremultiplied Source context values; deliberately not normalized.
#[derive(Debug,Clone,Copy,PartialEq,Default)]
pub struct GradientColor { pub components:[f32;4] }

#[derive(Debug,Clone,Copy,PartialEq,Default)]
pub struct TwoStopContext { pub factor:GradientColor,pub bias:GradientColor }

#[derive(Debug,Clone,PartialEq,Default)]
pub struct GradientContext {
 pub len:usize,pub factors:Vec<GradientColor>,pub biases:Vec<GradientColor>,pub t_values:Vec<NormalizedF32>,
}

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
