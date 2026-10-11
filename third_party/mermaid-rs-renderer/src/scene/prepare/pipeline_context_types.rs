//! Persistent tiny-skia Context fields; one owner per builder/compiled pipeline.
use tiny_skia_path::{NormalizedF32,Transform};
use crate::scene::paint::SpreadMode;
#[derive(Debug,Clone,Copy,PartialEq,Default)]
pub struct GradientColor{pub components:[f32;4]}
#[derive(Debug,Clone,Copy,PartialEq,Default)]
pub struct TwoStopContext{pub factor:GradientColor,pub bias:GradientColor}
#[derive(Debug,Clone,PartialEq,Default)]
pub struct GradientContext{pub len:usize,pub factors:Vec<GradientColor>,pub biases:Vec<GradientColor>,pub t_values:Vec<NormalizedF32>}
#[derive(Debug,Clone,Copy,Default,PartialEq)]
pub struct UniformColor{pub components:[f32;4],pub rgba:[u16;4]}
#[derive(Debug,Clone,Copy,Default,PartialEq)]
pub struct TileContext{pub scale:f32,pub inv_scale:f32}
#[derive(Debug,Clone,Copy,PartialEq)]
pub struct SamplerContext{pub spread_mode:SpreadMode,pub inv_width:f32,pub inv_height:f32}
impl Default for SamplerContext{fn default()->Self{Self{spread_mode:SpreadMode::Pad,inv_width:0.0,inv_height:0.0}}}
#[derive(Debug,Clone,PartialEq,Default)]
pub struct PipelineContext{
 pub current_coverage:f32,pub sampler:SamplerContext,pub uniform_color:UniformColor,
 pub two_stop:TwoStopContext,pub gradient:GradientContext,
 pub radial_p:[f32;2],pub radial_mask:[u32;8],pub limit_x:TileContext,pub limit_y:TileContext,pub transform:Transform,
}
