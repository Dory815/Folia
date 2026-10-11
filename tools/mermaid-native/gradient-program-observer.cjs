// Root append-only observers. All original Source bodies remain byte-identical.
'use strict';
module.exports={
 'src/pipeline/mod.rs':`
// Root observation of original private stage list only.
impl RasterPipelineBuilder {
 pub fn folia_stage_names(&self)->alloc::vec::Vec<alloc::string::String>{
  self.stages.iter().map(|s|alloc::format!("{:?}",s)).collect()
 }
}
`,
 'src/shaders/gradient.rs':`
// Root observations: construct original inputs and execute original methods.
#[allow(missing_docs)]
pub struct FoliaGradientProgramSnapshot {
 pub returned:bool,pub stages:Vec<alloc::string::String>,pub transform:[u32;6],
 pub two_stop:[[u32;4];2],pub len:usize,pub factors:Vec<[u32;4]>,pub biases:Vec<[u32;4]>,pub t_values:Vec<u32>,pub radial_p:[u32;2],pub radial_mask:[u32;8],
}
fn folia_color_space(v:u8)->ColorSpace{match v{0=>ColorSpace::Linear,1=>ColorSpace::Gamma2,2=>ColorSpace::SimpleSRGB,3=>ColorSpace::FullSRGBGamma,_=>panic!("bad color space")}}
#[allow(missing_docs)]
pub fn folia_color_expansion(v:[u32;4],mode:u8)->Option<[u32;4]>{let a=v.map(f32::from_bits);let c=Color::from_rgba(a[0],a[1],a[2],a[3])?;let c=folia_color_space(mode).expand_color(c);Some([c.red(),c.green(),c.blue(),c.alpha()].map(f32::to_bits))}
#[allow(missing_docs)]
pub fn folia_approximate_power(x:u32,y:u32)->u32{crate::math::approx_powf(f32::from_bits(x),f32::from_bits(y)).to_bits()}
#[allow(missing_docs)]
pub fn folia_gradient_program(raw:&[[u32;5]],mode:u8,transform:[u32;6],points:[u32;6],opacity:&[u32],color_space:u8,radial:Option<([u32;2],Vec<u8>)>)->Option<FoliaGradientProgramSnapshot>{
 fn ts(v:[u32;6])->Transform{let a=v.map(f32::from_bits);Transform::from_row(a[0],a[1],a[2],a[3],a[4],a[5])}
 fn tb(t:Transform)->[u32;6]{[t.sx,t.ky,t.kx,t.sy,t.tx,t.ty].map(f32::to_bits)}
 fn cb(c:GradientColor)->[u32;4]{[c.r,c.g,c.b,c.a].map(f32::to_bits)}
 fn stage(v:u8)->pipeline::Stage{use pipeline::Stage::*;match v{1=>XYToRadius,2=>ApplyConcentricScaleBias,3=>XYTo2PtConicalStrip,4=>Mask2PtConicalNan,5=>XYTo2PtConicalFocalOnCircle,6=>XYTo2PtConicalWellBehaved,7=>XYTo2PtConicalSmaller,8=>XYTo2PtConicalGreater,9=>Mask2PtConicalDegenerates,10=>NegateX,11=>Alter2PtConicalCompensateFocal,12=>Alter2PtConicalUnswap,13=>ApplyVectorMask,_=>panic!("bad radial stage")}}
 let mut stops=Vec::new();for values in raw{let a=values.map(f32::from_bits);let c=Color::from_rgba(a[1],a[2],a[3],a[4])?;stops.push(GradientStop::new(a[0],c));}
 let spread=match mode{0=>SpreadMode::Pad,1=>SpreadMode::Reflect,2=>SpreadMode::Repeat,_=>panic!("bad spread")};
 let mut g=Gradient::new(stops,spread,ts(transform),ts(points));for o in opacity{g.apply_opacity(f32::from_bits(*o));}
 let mut p=RasterPipelineBuilder::new();if let Some((values,_))=&radial{p.ctx.two_point_conical_gradient.p0=f32::from_bits(values[0]);p.ctx.two_point_conical_gradient.p1=f32::from_bits(values[1]);}
 let pre=|p:&mut RasterPipelineBuilder|{if let Some((_,stages))=&radial{for code in stages.iter().filter(|v|**v!=13){p.push(stage(*code));}}};
 let post=|p:&mut RasterPipelineBuilder|{if let Some((_,stages))=&radial{for code in stages.iter().filter(|v|**v==13){p.push(stage(*code));}}};
 let returned=g.push_stages(&mut p,folia_color_space(color_space),&pre,&post);
 Some(FoliaGradientProgramSnapshot{returned,stages:p.folia_stage_names(),transform:tb(p.ctx.transform),two_stop:[cb(p.ctx.evenly_spaced_2_stop_gradient.factor),cb(p.ctx.evenly_spaced_2_stop_gradient.bias)],len:p.ctx.gradient.len,factors:p.ctx.gradient.factors.iter().map(|c|cb(*c)).collect(),biases:p.ctx.gradient.biases.iter().map(|c|cb(*c)).collect(),t_values:p.ctx.gradient.t_values.iter().map(|t|t.get().to_bits()).collect(),radial_p:[p.ctx.two_point_conical_gradient.p0,p.ctx.two_point_conical_gradient.p1].map(f32::to_bits),radial_mask:bytemuck::cast(p.ctx.two_point_conical_gradient.mask)})
}
`,
 'src/shaders/mod.rs':'\n// Root observer exports only.\npub use gradient::{folia_gradient_program,folia_color_expansion,folia_approximate_power,FoliaGradientProgramSnapshot};\n',
 'src/lib.rs':'\n// Root observer exports only.\npub use shaders::{folia_gradient_program,folia_color_expansion,folia_approximate_power,FoliaGradientProgramSnapshot};\n'
};
