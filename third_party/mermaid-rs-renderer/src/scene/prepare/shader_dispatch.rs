//! Source Shader variant methods; one shared builder passed by its caller.
// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
use tiny_skia_path::Transform;
use super::{prepared_shader_types::PreparedShader,gradient_program_types::ColorSpace,pipeline_builder::PipelineBuilder};
impl PreparedShader<'_>{
 pub fn is_opaque(&self)->bool{
  match self{
   PreparedShader::SolidColor(color)=>color.is_opaque(),
   PreparedShader::LinearGradient(base)=>base.colors_are_opaque(),
   PreparedShader::RadialGradient{..}=>false,
   PreparedShader::SweepGradient{base,..}=>base.colors_are_opaque(),
   PreparedShader::Pattern(_)=>false,
  }
 }
 pub fn transform(&mut self,transform:Transform){
  match self{
   PreparedShader::SolidColor(_)=>{},
   PreparedShader::LinearGradient(base)=>base.transform=base.transform.post_concat(transform),
   PreparedShader::RadialGradient{base,..}=>base.transform=base.transform.post_concat(transform),
   PreparedShader::SweepGradient{base,..}=>base.transform=base.transform.post_concat(transform),
   PreparedShader::Pattern(pattern)=>pattern.transform=pattern.transform.post_concat(transform),
  }
 }
 pub fn apply_opacity(&mut self,opacity:f32){
  match self{
   PreparedShader::SolidColor(color)=>color.apply_opacity(opacity),
   PreparedShader::LinearGradient(base)=>base.apply_opacity(opacity),
   PreparedShader::RadialGradient{base,..}=>base.apply_opacity(opacity),
   PreparedShader::SweepGradient{base,..}=>base.apply_opacity(opacity),
   PreparedShader::Pattern(pattern)=>pattern.apply_opacity(opacity),
  }
 }
 pub fn push_stages(&self,color_space:ColorSpace,builder:&mut PipelineBuilder)->bool{
  match self{
   PreparedShader::SolidColor(color)=>{
    let color=color_space.expand_color(*color).premultiply();
    builder.push_uniform_color(color);
    true
   },
   PreparedShader::LinearGradient(base)=>super::linear_program::push_linear_gradient_stages(base,color_space,builder),
   PreparedShader::RadialGradient{base,kind}=>super::radial_program::push_radial_gradient_stages(base,*kind,color_space,builder),
   PreparedShader::SweepGradient{base,t0,t1}=>super::sweep_program::push_sweep_gradient_stages(base,*t0,*t1,color_space,builder),
   PreparedShader::Pattern(pattern)=>pattern.push_stages(color_space,builder),
  }
 }
}
