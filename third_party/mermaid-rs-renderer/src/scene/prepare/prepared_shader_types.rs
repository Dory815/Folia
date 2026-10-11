//! One prepared f32 Shader value; Pattern keeps its source borrow.
use super::{gradient_stop_types::{StopColor,PreparedGradient},radial_types::RadialKind,pattern_types::PreparedPattern};
#[derive(Debug,Clone)]
pub enum PreparedShader<'source>{
 SolidColor(StopColor),LinearGradient(PreparedGradient),
 RadialGradient{base:PreparedGradient,kind:RadialKind},
 SweepGradient{base:PreparedGradient,t0:f32,t1:f32},Pattern(PreparedPattern<'source>),
}
