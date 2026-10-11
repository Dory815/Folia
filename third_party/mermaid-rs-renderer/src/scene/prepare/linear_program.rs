//! Original LinearGradient stage dispatch, using the single common builder.
// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
use super::{gradient_stop_types::PreparedGradient,gradient_program_types::ColorSpace,pipeline_builder::PipelineBuilder};
pub fn push_linear_gradient_stages(gradient:&PreparedGradient,color_space:ColorSpace,builder:&mut PipelineBuilder)->bool{
 super::gradient_program::push_gradient_stages(gradient,color_space,None,builder)
}

pub fn new_linear_gradient(start:tiny_skia_path::Point,end:tiny_skia_path::Point,stops:Vec<super::gradient_stop_types::PreparedGradientStop>,spread:crate::scene::paint::SpreadMode,transform:tiny_skia_path::Transform)->Option<super::prepared_shader_types::PreparedShader<'static>>{
 use tiny_skia_path::Scalar;
 use super::prepared_shader_types::PreparedShader;
 use crate::scene::paint::SpreadMode;
 const DEGENERATE_THRESHOLD:f32=1.0/(1<<15)as f32;
 if stops.is_empty(){return None;}
 if stops.len()==1{return Some(PreparedShader::SolidColor(stops[0].color()));}
 let length=(end-start).length();
 if !length.is_finite(){return None;}
 if length.is_nearly_zero_within_tolerance(DEGENERATE_THRESHOLD){
  match spread{
   SpreadMode::Pad=>return Some(PreparedShader::SolidColor(stops.last().unwrap().color())),
   SpreadMode::Reflect|SpreadMode::Repeat=>return Some(PreparedShader::SolidColor(average_gradient_color(&stops))),
  }
 }
 transform.invert()?;
 let points_to_unit=points_to_unit_ts(start,end)?;
 Some(PreparedShader::LinearGradient(PreparedGradient::new(stops,spread,transform,points_to_unit)))
}

fn points_to_unit_ts(start:tiny_skia_path::Point,end:tiny_skia_path::Point)->Option<tiny_skia_path::Transform>{
 use tiny_skia_path::Scalar;
 let mut vector=end-start;
 let magnitude=vector.length();
 let inverse=if magnitude!=0.0{magnitude.invert()}else{0.0};
 vector.scale(inverse);
 let mut transform=ts_from_sin_cos_at(-vector.y,vector.x,start.x,start.y);
 transform=transform.post_translate(-start.x,-start.y);
 transform=transform.post_scale(inverse,inverse);
 Some(transform)
}

fn average_gradient_color(stops:&[super::gradient_stop_types::PreparedGradientStop])->super::gradient_stop_types::StopColor{
 use super::{gradient_stop_types::StopColor,sampling::wide::f32x4};
 fn load_color(color:StopColor)->f32x4{f32x4::from(color.components())}
 fn store_color(color:f32x4)->StopColor{
  let color:[f32;4]=color.into();
  StopColor::from_rgba(color[0],color[1],color[2],color[3]).unwrap()
 }
 assert!(!stops.is_empty());
 let mut blend=f32x4::splat(0.0);
 let weight_scale=f32x4::splat(0.5);
 for index in 0..stops.len()-1{
  let c0=load_color(stops[index].color());
  let c1=load_color(stops[index+1].color());
  let weight=stops[index+1].position()-stops[index].position();
  blend+=weight_scale*f32x4::splat(weight)*(c1+c0);
 }
 if stops[0].position()>0.0{
  let color=load_color(stops[0].color());
  blend+=f32x4::splat(stops[0].position())*color;
 }
 let last_index=stops.len()-1;
 if stops[last_index].position()<1.0{
  let color=load_color(stops[last_index].color());
  blend+=(f32x4::splat(1.0)-f32x4::splat(stops[last_index].position()))*color;
 }
 store_color(blend)
}

fn ts_from_sin_cos_at(sin:f32,cos:f32,px:f32,py:f32)->tiny_skia_path::Transform{
 let cos_inv=1.0-cos;
 tiny_skia_path::Transform::from_row(cos,sin,-sin,cos,sdot(sin,py,cos_inv,px),sdot(-sin,px,cos_inv,py))
}

fn sdot(a:f32,b:f32,c:f32,d:f32)->f32{a*b+c*d}
