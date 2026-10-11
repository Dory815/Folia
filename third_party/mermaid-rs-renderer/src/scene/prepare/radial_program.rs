//! Original RadialGradient context setup and stage dispatch to the common builder.
// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
use super::{gradient_stop_types::PreparedGradient,gradient_program_types::ColorSpace,radial_types::RadialKind,pipeline_builder::PipelineBuilder};
pub fn push_radial_gradient_stages(gradient:&PreparedGradient,kind:RadialKind,color_space:ColorSpace,builder:&mut PipelineBuilder)->bool{
 let plan=super::radial_stages::prepare_radial_stages(kind);
 builder.context.radial_mask=[0;8];
 builder.context.radial_p=[plan.p0,plan.p1];
 super::gradient_program::push_gradient_stages(gradient,color_space,Some(&plan),builder)
}

pub fn new_radial_gradient(start:tiny_skia_path::Point,start_radius:f32,end:tiny_skia_path::Point,end_radius:f32,mut stops:Vec<super::gradient_stop_types::PreparedGradientStop>,spread:crate::scene::paint::SpreadMode,transform:tiny_skia_path::Transform)->Option<super::prepared_shader_types::PreparedShader<'static>>{
 use super::{gradient_stop_types::PreparedGradientStop,prepared_shader_types::PreparedShader,radial_geometry::prepare_radial_geometry,radial_types::RadialGeometry};
 if start_radius<0.0||end_radius<0.0{return None;}
 match stops.as_slice(){
  []=>return None,
  [stop]=>return Some(PreparedShader::SolidColor(stop.color())),
  _=>{}
 }
 let geometry=prepare_radial_geometry(start,start_radius,end,end_radius,stops.len(),spread,transform);
 match geometry{
  RadialGeometry::None=>None,
  RadialGeometry::SolidFirstStop=>Some(PreparedShader::SolidColor(stops[0].color())),
  RadialGeometry::Conical{kind,points_to_unit,thin_ring}=>{
   if thin_ring{
    let start_color=stops.first()?.color();
    let end_color=stops.last()?.color();
    stops.clear();
    stops.extend_from_slice(&[
     PreparedGradientStop::new(0.0,start_color),
     PreparedGradientStop::new(1.0,start_color),
     PreparedGradientStop::new(1.0,end_color),
    ]);
   }
   Some(PreparedShader::RadialGradient{
    base:PreparedGradient::new(stops,spread,transform,points_to_unit),kind,
   })
  }
 }
}
