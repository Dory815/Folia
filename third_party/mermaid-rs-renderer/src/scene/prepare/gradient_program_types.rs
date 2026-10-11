//! Root-frozen shared gradient wrapper context. Neither consumer recomputes it.
pub use super::pipeline_context_types::{GradientColor,TwoStopContext,GradientContext};

#[derive(Debug,Clone,Copy,PartialEq,Eq,Default)]
pub enum ColorSpace { #[default] Linear, Gamma2, SimpleSRGB, FullSRGBGamma }

