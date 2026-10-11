//! Source Sweep factory and producer; no separate stage or context owner.
// Copyright 2006 The Android Open Source Project
// BSD-style license: see LICENSE-tiny-skia-blend.
use tiny_skia_path::{Point,Transform};
use crate::scene::paint::SpreadMode;
use super::{prepared_shader_types::PreparedShader,gradient_stop_types::{PreparedGradient,PreparedGradientStop},gradient_program_types::ColorSpace,pipeline_builder::PipelineBuilder};
pub fn new_sweep_gradient(center:Point,start:f32,end:f32,stops:Vec<PreparedGradientStop>,mut spread:SpreadMode,transform:Transform)->Option<PreparedShader<'static>>{
 use tiny_skia_path::Scalar;
 const DEGENERATE_THRESHOLD:f32=1.0/(1<<15)as f32;
 if !start.is_finite()||!end.is_finite()||start>end{return None;}
 match stops.as_slice(){
  []=>return None,
  [stop]=>return Some(PreparedShader::SolidColor(stop.color())),
  _=>(),
 }
 transform.invert()?;
 if start.is_nearly_equal_within_tolerance(end,DEGENERATE_THRESHOLD){
  if spread==SpreadMode::Pad&&end>DEGENERATE_THRESHOLD{
   let front_color=stops.first().unwrap().color();
   let back_color=stops.last().unwrap().color();
   let mut new_stops=stops;
   new_stops.clear();
   new_stops.extend_from_slice(&[
    PreparedGradientStop::new(0.0,front_color),
    PreparedGradientStop::new(1.0,front_color),
    PreparedGradientStop::new(1.0,back_color),
   ]);
   return new_sweep_gradient(center,0.0,end,new_stops,spread,transform);
  }
  return None;
 }
 if start<=0.0&&end>=360.0{spread=SpreadMode::Pad;}
 let t0=start/360.0;
 let t1=end/360.0;
 Some(PreparedShader::SweepGradient{
  base:PreparedGradient::new(stops,spread,transform,Transform::from_translate(-center.x,-center.y)),
  t0,t1,
 })
}
pub fn push_sweep_gradient_stages(base:&PreparedGradient,t0:f32,t1:f32,color_space:ColorSpace,builder:&mut PipelineBuilder)->bool{
 use super::pipeline_control_types::PipelineStage;
 let scale=1.0/(t1-t0);
 let bias=-scale*t0;
 builder.context.radial_p[0]=scale;
 builder.context.radial_p[1]=bias;
 super::gradient_program::push_gradient_stages_with_callbacks(
  base,color_space,
  &|builder|{
   builder.push(PipelineStage::XYToUnitAngle);
   if scale!=1.0&&bias!=0.0{
    builder.push(PipelineStage::ApplyConcentricScaleBias);
   }
  },
  &|_|{},builder,
 )
}
