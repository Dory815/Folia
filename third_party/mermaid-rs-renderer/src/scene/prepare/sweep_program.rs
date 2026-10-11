//! Source Sweep factory and producer; no separate stage or context owner.
use tiny_skia_path::{Point,Transform};
use crate::scene::paint::SpreadMode;
use super::{prepared_shader_types::PreparedShader,gradient_stop_types::{PreparedGradient,PreparedGradientStop},gradient_program_types::ColorSpace,pipeline_builder::PipelineBuilder};
pub fn new_sweep_gradient(_center:Point,_start:f32,_end:f32,_stops:Vec<PreparedGradientStop>,_spread:SpreadMode,_transform:Transform)->Option<PreparedShader<'static>>{panic!("Unimplemented Root-frozen SweepGradient::new")}
pub fn push_sweep_gradient_stages(_base:&PreparedGradient,_t0:f32,_t1:f32,_color_space:ColorSpace,_builder:&mut PipelineBuilder)->bool{panic!("Unimplemented Root-frozen SweepGradient::push_stages")}
