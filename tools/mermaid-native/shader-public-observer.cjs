// Root append-only original public constructors and five-variant dispatch.
'use strict';
module.exports={
'src/pipeline/mod.rs':require('./pattern-producer-observer.cjs')['src/pipeline/mod.rs'],
'src/shaders/gradient.rs':`
impl Gradient{pub(crate) fn folia_public_groups(&self)->Vec<Vec<u32>>{
 let t=self.transform;let u=self.points_to_unit;let mut v=alloc::vec![alloc::vec![match self.tile_mode{SpreadMode::Pad=>0,SpreadMode::Reflect=>1,SpreadMode::Repeat=>2},u32::from(self.colors_are_opaque),u32::from(self.has_uniform_stops),self.stops.len()as u32],
 [t.sx,t.ky,t.kx,t.sy,t.tx,t.ty].map(f32::to_bits).to_vec(),[u.sx,u.ky,u.kx,u.sy,u.tx,u.ty].map(f32::to_bits).to_vec()];
 for s in &self.stops{v.push([s.position.get(),s.color.red(),s.color.green(),s.color.blue(),s.color.alpha()].map(f32::to_bits).to_vec());}v
}}
`,
'src/shaders/radial_gradient.rs':`
impl RadialGradient{pub(crate) fn folia_public_kind(&self)->Vec<u32>{match self.gradient_type{
 GradientType::Radial{radius1,radius2}=>alloc::vec![2,radius1.to_bits(),radius2.to_bits()],
 GradientType::Strip{scaled_r0}=>alloc::vec![3,scaled_r0.to_bits()],
 GradientType::Focal(ref f)=>alloc::vec![4,f.r1.to_bits(),f.focal_x.to_bits(),u32::from(f.is_swapped)],
}}}
`,
'src/shaders/sweep_gradient.rs':`
impl SweepGradient{pub(crate) fn folia_public_angles(&self)->Vec<u32>{[self.t0,self.t1].map(f32::to_bits).to_vec()}}
`,
'src/shaders/pattern.rs':`
impl Pattern<'_>{pub(crate) fn folia_public_groups(&self,image:&[u8])->alloc::vec::Vec<alloc::vec::Vec<u32>>{
 let t=self.transform;alloc::vec![alloc::vec![4],alloc::vec![self.pixmap.width(),self.pixmap.height(),match self.spread_mode{crate::SpreadMode::Pad=>0,crate::SpreadMode::Reflect=>1,crate::SpreadMode::Repeat=>2},match self.quality{crate::FilterQuality::Nearest=>0,crate::FilterQuality::Bilinear=>1,crate::FilterQuality::Bicubic=>2},self.opacity.get().to_bits(),u32::from(self.pixmap.data().as_ptr()==image.as_ptr())],[t.sx,t.ky,t.kx,t.sy,t.tx,t.ty].map(f32::to_bits).to_vec()]
}}
`,
'src/shaders/mod.rs':`
fn folia_public_snapshot(s:&Shader,image:&[u8])->alloc::vec::Vec<alloc::vec::Vec<u32>>{
 match s{
 Shader::SolidColor(c)=>alloc::vec![alloc::vec![0],[c.red(),c.green(),c.blue(),c.alpha()].map(f32::to_bits).to_vec()],
 Shader::LinearGradient(g)=>{let mut v=alloc::vec![alloc::vec![1]];v.extend(g.base.folia_public_groups());v},
 Shader::RadialGradient(g)=>{let mut v=alloc::vec![alloc::vec![2],g.folia_public_kind()];v.extend(g.base.folia_public_groups());v},
 Shader::SweepGradient(g)=>{let mut v=alloc::vec![alloc::vec![3],g.folia_public_angles()];v.extend(g.base.folia_public_groups());v},
 Shader::Pattern(p)=>p.folia_public_groups(image),
 }
}
#[allow(missing_docs)]
pub fn folia_public_shader(kind:u8,raw:&[[u32;5]],color:[u32;4],geometry:[u32;6],mode:u8,transform:[u32;6],pattern:(u32,u32,u8,u32),image:&[u8],transforms:&[[u32;6]],opacities:&[u32],cs:u8,prefix:usize,sentinel:bool,force:bool)->(alloc::string::String,Option<alloc::vec::Vec<alloc::vec::Vec<u32>>>,Option<bool>,Option<alloc::vec::Vec<alloc::vec::Vec<u32>>>,Option<bool>,alloc::string::String,alloc::vec::Vec<alloc::string::String>,alloc::vec::Vec<alloc::vec::Vec<u32>>){
 fn ts(v:[u32;6])->crate::Transform{let a=v.map(f32::from_bits);crate::Transform::from_row(a[0],a[1],a[2],a[3],a[4],a[5])}
 let rejected=|name:&str|(alloc::string::String::from(name),None,None,None,None,alloc::string::String::from("not-called"),alloc::vec![],alloc::vec![]);
 let ctor=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||->Option<Shader>{
  let mut stops=alloc::vec![];for r in raw{let a=r.map(f32::from_bits);stops.push(crate::GradientStop::new(a[0],crate::Color::from_rgba(a[1],a[2],a[3],a[4])?));}
  let a=geometry.map(f32::from_bits);let spread=match mode{0=>crate::SpreadMode::Pad,1=>crate::SpreadMode::Reflect,_=>crate::SpreadMode::Repeat};let t=ts(transform);
  match kind{
  0=>{let c=color.map(f32::from_bits);Some(Shader::SolidColor(crate::Color::from_rgba(c[0],c[1],c[2],c[3])?))},
  1=>crate::LinearGradient::new(crate::Point::from_xy(a[0],a[1]),crate::Point::from_xy(a[2],a[3]),stops,spread,t),
  2=>crate::RadialGradient::new(crate::Point::from_xy(a[0],a[1]),a[4],crate::Point::from_xy(a[2],a[3]),a[5],stops,spread,t),
  3=>crate::SweepGradient::new(crate::Point::from_xy(a[0],a[1]),a[2],a[3],stops,spread,t),
  4=>Some(crate::Pattern::new(crate::PixmapRef::from_bytes(image,pattern.0,pattern.1)?,spread,match pattern.2{0=>crate::FilterQuality::Nearest,1=>crate::FilterQuality::Bilinear,_=>crate::FilterQuality::Bicubic},f32::from_bits(pattern.3),t)),
  _=>panic!("invalid Source constructor"),
  }
 }));
 let mut shader=match ctor{Ok(Some(s))=>s,Ok(None)=>return rejected("none"),Err(_)=>return rejected("panic")};
 let before=folia_public_snapshot(&shader,image);let opaque_before=shader.is_opaque();let mut b=RasterPipelineBuilder::new();b.set_force_hq_pipeline(force);if sentinel{b.folia_pattern_sentinel();}for _ in 0..prefix{b.push(crate::pipeline::Stage::Premultiply);}
 let result=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||{for v in transforms{shader.transform(ts(*v));}for o in opacities{shader.apply_opacity(f32::from_bits(*o));}shader.push_stages(match cs{0=>ColorSpace::Linear,1=>ColorSpace::Gamma2,2=>ColorSpace::SimpleSRGB,_=>ColorSpace::FullSRGBGamma},&mut b)}));
 (alloc::string::String::from("some"),Some(before),Some(opaque_before),Some(folia_public_snapshot(&shader,image)),Some(shader.is_opaque()),match result{Ok(v)=>alloc::format!("{}",v),Err(_)=>alloc::string::String::from("panic")},b.folia_pattern_stages(),b.folia_pattern_context())
}
`,
'src/lib.rs':'\n#[allow(missing_docs)] pub use shaders::folia_public_shader;\n'
};
