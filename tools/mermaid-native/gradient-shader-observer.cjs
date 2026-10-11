// Root actual LinearGradient/RadialGradient producer wrappers; original bodies untouched.
'use strict';
module.exports={
 'src/pipeline/mod.rs':require('./pattern-producer-observer.cjs')['src/pipeline/mod.rs'],
 'src/shaders/gradient.rs':`
pub(super) fn folia_gradient_base(raw:&[[u32;5]],mode:u8,transform:[u32;6],points:[u32;6],opacity:&[u32])->Option<Gradient>{
 fn ts(v:[u32;6])->Transform{let a=v.map(f32::from_bits);Transform::from_row(a[0],a[1],a[2],a[3],a[4],a[5])}
 let mut stops=Vec::new();for values in raw{let a=values.map(f32::from_bits);let c=Color::from_rgba(a[1],a[2],a[3],a[4])?;stops.push(GradientStop::new(a[0],c));}
 let mut base=Gradient::new(stops,match mode{0=>SpreadMode::Pad,1=>SpreadMode::Reflect,_=>SpreadMode::Repeat},ts(transform),ts(points));for o in opacity{base.apply_opacity(f32::from_bits(*o));}Some(base)
}
`,
 'src/shaders/linear_gradient.rs':`
#[allow(missing_docs)] pub fn folia_linear_producer(raw:&[[u32;5]],mode:u8,transform:[u32;6],points:[u32;6],opacity:&[u32],color_space:u8,prefix:usize,sentinel:bool,force:bool)->Option<(alloc::string::String,Vec<alloc::string::String>,Vec<Vec<u32>>)>{
 let g=LinearGradient{base:super::gradient::folia_gradient_base(raw,mode,transform,points,opacity)?};
 let mut p=RasterPipelineBuilder::new();p.set_force_hq_pipeline(force);if sentinel{p.folia_pattern_sentinel();}for _ in 0..prefix{p.push(crate::pipeline::Stage::Premultiply);}
 let outcome=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||g.push_stages(match color_space{0=>ColorSpace::Linear,1=>ColorSpace::Gamma2,2=>ColorSpace::SimpleSRGB,_=>ColorSpace::FullSRGBGamma},&mut p)));
 Some((match outcome{Ok(v)=>alloc::format!("{}",v),Err(_)=>alloc::string::String::from("panic")},p.folia_pattern_stages(),p.folia_pattern_context()))
}
`,
 'src/shaders/radial_gradient.rs':`
#[allow(missing_docs)] pub fn folia_radial_producer(raw:&[[u32;5]],mode:u8,transform:[u32;6],points:[u32;6],opacity:&[u32],color_space:u8,kind:(u8,[u32;2],bool),prefix:usize,sentinel:bool,force:bool)->Option<(alloc::string::String,Vec<alloc::string::String>,Vec<Vec<u32>>)>{
 let v=kind.1.map(f32::from_bits);let gradient_type=match kind.0{2=>GradientType::Radial{radius1:v[0],radius2:v[1]},3=>GradientType::Strip{scaled_r0:v[0]},4=>GradientType::Focal(FocalData{r1:v[0],focal_x:v[1],is_swapped:kind.2}),_=>panic!("bad Source kind")};
 let g=RadialGradient{base:super::gradient::folia_gradient_base(raw,mode,transform,points,opacity)?,gradient_type};
 let mut p=RasterPipelineBuilder::new();p.set_force_hq_pipeline(force);if sentinel{p.folia_pattern_sentinel();}for _ in 0..prefix{p.push(pipeline::Stage::Premultiply);}
 let outcome=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||g.push_stages(match color_space{0=>ColorSpace::Linear,1=>ColorSpace::Gamma2,2=>ColorSpace::SimpleSRGB,_=>ColorSpace::FullSRGBGamma},&mut p)));
 Some((match outcome{Ok(v)=>alloc::format!("{}",v),Err(_)=>alloc::string::String::from("panic")},p.folia_pattern_stages(),p.folia_pattern_context()))
}
`,
 'src/shaders/mod.rs':'\n#[allow(missing_docs)] pub use linear_gradient::folia_linear_producer;\n#[allow(missing_docs)] pub use radial_gradient::folia_radial_producer;\n',
 'src/lib.rs':'\n#[allow(missing_docs)] pub use shaders::{folia_linear_producer,folia_radial_producer};\n',
};
