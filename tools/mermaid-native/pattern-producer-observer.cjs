// Root append-only original observers; no native renderer is linked.
'use strict';
module.exports={
'src/pipeline/mod.rs':`
impl RasterPipelineBuilder {
 pub fn folia_pattern_stages(&self)->alloc::vec::Vec<alloc::string::String>{self.stages.iter().map(|s|alloc::format!("{:?}",s)).collect()}
 pub fn folia_pattern_context(&self)->alloc::vec::Vec<alloc::vec::Vec<u32>>{
  let c=&self.ctx;let color=|v:GradientColor|[v.r,v.g,v.b,v.a].map(f32::to_bits).to_vec();let mask:[u32;8]=bytemuck::cast(c.two_point_conical_gradient.mask);
  alloc::vec![alloc::vec![c.current_coverage.to_bits()],alloc::vec![match c.sampler.spread_mode{crate::SpreadMode::Pad=>0,crate::SpreadMode::Reflect=>1,crate::SpreadMode::Repeat=>2},c.sampler.inv_width.to_bits(),c.sampler.inv_height.to_bits()],
  [c.uniform_color.r,c.uniform_color.g,c.uniform_color.b,c.uniform_color.a].map(f32::to_bits).to_vec(),c.uniform_color.rgba.map(u32::from).to_vec(),color(c.evenly_spaced_2_stop_gradient.factor),color(c.evenly_spaced_2_stop_gradient.bias),
  alloc::vec![c.gradient.len as u32,c.gradient.factors.len()as u32,c.gradient.biases.len()as u32,c.gradient.t_values.len()as u32],c.gradient.factors.iter().copied().flat_map(color).collect(),c.gradient.biases.iter().copied().flat_map(color).collect(),c.gradient.t_values.iter().map(|v|v.get().to_bits()).collect(),
  [c.two_point_conical_gradient.p0,c.two_point_conical_gradient.p1].map(f32::to_bits).to_vec(),mask.to_vec(),[c.limit_x.scale,c.limit_x.inv_scale].map(f32::to_bits).to_vec(),[c.limit_y.scale,c.limit_y.inv_scale].map(f32::to_bits).to_vec(),
  [c.transform.sx,c.transform.ky,c.transform.kx,c.transform.sy,c.transform.tx,c.transform.ty].map(f32::to_bits).to_vec(),alloc::vec![u32::from(self.force_hq_pipeline)]]
 }
 pub fn folia_pattern_sentinel(&mut self){
  let c=&mut self.ctx;c.current_coverage=0.25;c.sampler=SamplerCtx{spread_mode:crate::SpreadMode::Reflect,inv_width:0.125,inv_height:0.375};c.uniform_color=UniformColorCtx{r:0.1,g:0.2,b:0.3,a:0.4,rgba:[1,2,3,4]};
  c.evenly_spaced_2_stop_gradient=EvenlySpaced2StopGradientCtx{factor:GradientColor::new(2.0,3.0,4.0,5.0),bias:GradientColor::new(6.0,7.0,8.0,9.0)};c.gradient.len=1;c.gradient.factors=alloc::vec![GradientColor::new(10.0,11.0,12.0,13.0)];c.gradient.biases=alloc::vec![GradientColor::new(14.0,15.0,16.0,17.0)];c.gradient.t_values=alloc::vec![NormalizedF32::new(0.5).unwrap()];
  c.two_point_conical_gradient.p0=12.0;c.two_point_conical_gradient.p1=13.0;c.two_point_conical_gradient.mask=bytemuck::cast([1u32,2,3,4,5,6,7,8]);c.limit_x=TileCtx{scale:14.0,inv_scale:15.0};c.limit_y=TileCtx{scale:16.0,inv_scale:17.0};c.transform=Transform::from_row(2.0,3.0,4.0,5.0,6.0,7.0);
 }
}
`,
'src/shaders/pattern.rs':`
#[allow(missing_docs)]
pub fn folia_pattern_producer(width:u32,height:u32,spread:u8,quality:u8,opacity:u32,transform:[u32;6],transforms:&[[u32;6]],opacities:&[u32],color_space:u8,prefix:usize,sentinel:bool,force:bool)->(alloc::string::String,alloc::vec::Vec<u32>,alloc::vec::Vec<alloc::string::String>,alloc::vec::Vec<alloc::vec::Vec<u32>>,bool){
 fn ts(v:[u32;6])->Transform{let a=v.map(f32::from_bits);Transform::from_row(a[0],a[1],a[2],a[3],a[4],a[5])}
 let data=alloc::vec![0u8;(width as usize)*(height as usize)*4];let pixmap=PixmapRef::from_bytes(&data,width,height).unwrap();
 let mut shader=Pattern::new(pixmap,match spread{0=>SpreadMode::Pad,1=>SpreadMode::Reflect,_=>SpreadMode::Repeat},match quality{0=>FilterQuality::Nearest,1=>FilterQuality::Bilinear,_=>FilterQuality::Bicubic},f32::from_bits(opacity),ts(transform));
 let mut p=RasterPipelineBuilder::new();p.set_force_hq_pipeline(force);if sentinel{p.folia_pattern_sentinel();}for _ in 0..prefix{p.push(pipeline::Stage::Premultiply);}
 let outcome=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||{for v in transforms{shader.transform(ts(*v));}for v in opacities{shader.apply_opacity(f32::from_bits(*v));}shader.push_stages(match color_space{0=>ColorSpace::Linear,1=>ColorSpace::Gamma2,2=>ColorSpace::SimpleSRGB,_=>ColorSpace::FullSRGBGamma},&mut p)}));
 let result=match outcome{Ok(v)=>alloc::format!("{}",v),Err(_)=>alloc::string::String::from("panic")};let Shader::Pattern(pattern)=shader else{unreachable!()};let t=pattern.transform;
 let params=alloc::vec![pattern.pixmap.width(),pattern.pixmap.height(),match pattern.spread_mode{SpreadMode::Pad=>0,SpreadMode::Reflect=>1,SpreadMode::Repeat=>2},match pattern.quality{FilterQuality::Nearest=>0,FilterQuality::Bilinear=>1,FilterQuality::Bicubic=>2},pattern.opacity.get().to_bits(),t.sx.to_bits(),t.ky.to_bits(),t.kx.to_bits(),t.sy.to_bits(),t.tx.to_bits(),t.ty.to_bits()];
 (result,params,p.folia_pattern_stages(),p.folia_pattern_context(),pattern.pixmap.data().as_ptr()==data.as_ptr())
}
`,
'src/shaders/mod.rs':'\n#[allow(missing_docs)] pub use pattern::folia_pattern_producer;\n',
'src/lib.rs':'\n#[allow(missing_docs)] pub use shaders::folia_pattern_producer;\n'
};
