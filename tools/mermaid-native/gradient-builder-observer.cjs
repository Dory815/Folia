// Root append-only original generic Gradient producer, with complete builder state.
'use strict';
const pipeline=require('./pattern-producer-observer.cjs')['src/pipeline/mod.rs'];
module.exports={
 'src/pipeline/mod.rs':pipeline,
 'src/shaders/gradient.rs':`
#[allow(missing_docs)]
pub fn folia_gradient_builder(raw:&[[u32;5]],mode:u8,transform:[u32;6],points:[u32;6],opacity:&[u32],color_space:u8,radial:Option<([u32;2],Vec<u8>)>,prefix:usize,sentinel:bool,force:bool)->Option<(alloc::string::String,Vec<alloc::string::String>,Vec<Vec<u32>>)>{
 fn ts(v:[u32;6])->Transform{let a=v.map(f32::from_bits);Transform::from_row(a[0],a[1],a[2],a[3],a[4],a[5])}
 fn stage(v:u8)->pipeline::Stage{use pipeline::Stage::*;match v{1=>XYToRadius,2=>ApplyConcentricScaleBias,3=>XYTo2PtConicalStrip,4=>Mask2PtConicalNan,5=>XYTo2PtConicalFocalOnCircle,6=>XYTo2PtConicalWellBehaved,7=>XYTo2PtConicalSmaller,8=>XYTo2PtConicalGreater,9=>Mask2PtConicalDegenerates,10=>NegateX,11=>Alter2PtConicalCompensateFocal,12=>Alter2PtConicalUnswap,13=>ApplyVectorMask,_=>panic!("bad radial stage")}}
 let mut stops=Vec::new();for values in raw{let a=values.map(f32::from_bits);let c=Color::from_rgba(a[1],a[2],a[3],a[4])?;stops.push(GradientStop::new(a[0],c));}
 let mut g=Gradient::new(stops,match mode{0=>SpreadMode::Pad,1=>SpreadMode::Reflect,_=>SpreadMode::Repeat},ts(transform),ts(points));for o in opacity{g.apply_opacity(f32::from_bits(*o));}
 let mut p=RasterPipelineBuilder::new();p.set_force_hq_pipeline(force);if sentinel{p.folia_pattern_sentinel();}for _ in 0..prefix{p.push(pipeline::Stage::Premultiply);}
 // The original generic producer receives only pre/post callbacks. Radial
 // parameter setup belongs to RadialGradient::push_stages, outside this packet.
 let pre=|p:&mut RasterPipelineBuilder|{if let Some((_,stages))=&radial{for code in stages.iter().filter(|v|**v!=13){p.push(stage(*code));}}};
 let post=|p:&mut RasterPipelineBuilder|{if let Some((_,stages))=&radial{for code in stages.iter().filter(|v|**v==13){p.push(stage(*code));}}};
 let outcome=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||g.push_stages(&mut p,match color_space{0=>ColorSpace::Linear,1=>ColorSpace::Gamma2,2=>ColorSpace::SimpleSRGB,_=>ColorSpace::FullSRGBGamma},&pre,&post)));
 let result=match outcome{Ok(v)=>alloc::format!("{}",v),Err(_)=>alloc::string::String::from("panic")};Some((result,p.folia_pattern_stages(),p.folia_pattern_context()))
}
`,
 'src/shaders/mod.rs':'\n#[allow(missing_docs)] pub use gradient::folia_gradient_builder;\n',
 'src/lib.rs':'\n#[allow(missing_docs)] pub use shaders::folia_gradient_builder;\n',
};
