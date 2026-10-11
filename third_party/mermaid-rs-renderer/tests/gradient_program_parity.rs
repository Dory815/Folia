// Root actual Source observations; no native-derived expected constants.
use mermaid_rs_renderer::scene::{paint::SpreadMode,prepare::{
 gradient_stop_types::{StopColor,PreparedGradientStop,PreparedGradient},
 gradient_program_types::{ColorSpace,GradientStage},gradient_color_space::approximate_power,
 gradient_program::prepare_gradient_program,radial_stage_types::{RadialStage,RadialStagePlan},
}};
use serde::Deserialize;
use tiny_skia_path::Transform;
#[derive(Deserialize)]
struct Input{stops:Vec<[u32;5]>,spread:u8,transform:[u32;6],points_to_unit:[u32;6],opacity:Vec<u32>,color_space:u8,radial:Option<([u32;2],Vec<u8>)>}
#[derive(Deserialize)]
struct Case{input:Input,accepted:bool,returned:bool,stages:Vec<String>,transform:[u32;6],two_stop:[[u32;4];2],len:usize,factors:Vec<[u32;4]>,biases:Vec<[u32;4]>,t_values:Vec<u32>,radial_p:[u32;2],radial_mask:[u32;8],warnings:Vec<String>}
#[derive(Deserialize)]
struct ColorCase{input:([u32;4],u8),expanded:Option<[u32;4]>}
#[derive(Deserialize)]
struct PowerCase{input:[u32;2],result:u32}
#[derive(Deserialize)]
struct Fixture{programs:Vec<Case>,colors:Vec<ColorCase>,powers:Vec<PowerCase>}
fn transform(v:[u32;6])->Transform{let a=v.map(f32::from_bits);Transform::from_row(a[0],a[1],a[2],a[3],a[4],a[5])}
fn color_space(v:u8)->ColorSpace{match v{0=>ColorSpace::Linear,1=>ColorSpace::Gamma2,2=>ColorSpace::SimpleSRGB,3=>ColorSpace::FullSRGBGamma,_=>panic!("bad Source stimulus")}}
fn radial_stage(v:u8)->RadialStage{use RadialStage::*;match v{1=>XYToRadius,2=>ApplyConcentricScaleBias,3=>XYTo2PtConicalStrip,4=>Mask2PtConicalNan,5=>XYTo2PtConicalFocalOnCircle,6=>XYTo2PtConicalWellBehaved,7=>XYTo2PtConicalSmaller,8=>XYTo2PtConicalGreater,9=>Mask2PtConicalDegenerates,10=>NegateX,11=>Alter2PtConicalCompensateFocal,12=>Alter2PtConicalUnswap,13=>ApplyVectorMask,_=>panic!("bad Source stimulus")}}
fn stage_name(s:&GradientStage)->String{match s{GradientStage::Radial(r)=>format!("{r:?}"),other=>format!("{other:?}")}}
fn assert_bits(actual:f32,expected:u32,index:usize,count:&mut usize){assert_eq!(actual.to_bits(),expected,"Source case {index}: f32 bits");*count+=1;}
fn fixture()->Fixture{serde_json::from_str(include_str!("fixtures/gradient-program.json")).unwrap()}
static WARNINGS:std::sync::Mutex<Vec<String>>=std::sync::Mutex::new(Vec::new());
struct Observer;
impl log::Log for Observer{
 fn enabled(&self,m:&log::Metadata)->bool{m.level()<=log::Level::Warn}
 fn log(&self,r:&log::Record){if self.enabled(r.metadata()){WARNINGS.lock().unwrap().push(r.args().to_string());}}
 fn flush(&self){}
}
static OBSERVER:Observer=Observer;
#[test]
fn complete_source_gradient_program(){
 log::set_logger(&OBSERVER).unwrap();log::set_max_level(log::LevelFilter::Warn);
 let fixture=fixture();assert_eq!(fixture.programs.len(),13988);let mut assertions=1;
 for(index,c)in fixture.programs.iter().enumerate(){
  let stops:Option<Vec<_>>=c.input.stops.iter().map(|v|{let a=v.map(f32::from_bits);StopColor::from_rgba(a[1],a[2],a[3],a[4]).map(|color|PreparedGradientStop::new(a[0],color))}).collect();
  assert_eq!(stops.is_some(),c.accepted,"case {index}: color input");assertions+=1;let Some(stops)=stops else{continue};
  let spread=match c.input.spread{0=>SpreadMode::Pad,1=>SpreadMode::Reflect,2=>SpreadMode::Repeat,_=>panic!("bad Source stimulus")};
  let mut gradient=PreparedGradient::new(stops,spread,transform(c.input.transform),transform(c.input.points_to_unit));for o in &c.input.opacity{gradient.apply_opacity(f32::from_bits(*o));}
  let radial=c.input.radial.as_ref().map(|(p,stages)|RadialStagePlan{p0:f32::from_bits(p[0]),p1:f32::from_bits(p[1]),before_color:stages.iter().filter(|v|**v!=13).map(|v|radial_stage(*v)).collect(),after_color:stages.iter().filter(|v|**v==13).map(|v|radial_stage(*v)).collect(),geometry_lowp_compatible:false});
  let program=prepare_gradient_program(&gradient,color_space(c.input.color_space),radial.as_ref());
  assert_eq!(std::mem::take(&mut *WARNINGS.lock().unwrap()),c.warnings,"case {index}: Source warnings");assertions+=1;
  assert_eq!(program.returned,c.returned,"case {index}: Source returned flag");assert_eq!(program.stages.iter().map(stage_name).collect::<Vec<_>>(),c.stages,"case {index}: ordered stages");assertions+=2;
  let t=program.transform;for(v,e)in[t.sx,t.ky,t.kx,t.sy,t.tx,t.ty].into_iter().zip(c.transform){assert_bits(v,e,index,&mut assertions);}
  for(v,e)in program.two_stop.factor.components.into_iter().zip(c.two_stop[0]){assert_bits(v,e,index,&mut assertions);}for(v,e)in program.two_stop.bias.components.into_iter().zip(c.two_stop[1]){assert_bits(v,e,index,&mut assertions);}
  assert_eq!(program.gradient.len,c.len);assert_eq!(program.gradient.factors.len(),c.factors.len());assert_eq!(program.gradient.biases.len(),c.biases.len());assert_eq!(program.gradient.t_values.len(),c.t_values.len());assertions+=4;
  for(actual,expected)in[(&program.gradient.factors,&c.factors),(&program.gradient.biases,&c.biases)]{for(a,e)in actual.iter().zip(expected){for(v,bits)in a.components.into_iter().zip(e){assert_bits(v,*bits,index,&mut assertions);}}}
  for(v,e)in program.gradient.t_values.iter().zip(&c.t_values){assert_bits(v.get(),*e,index,&mut assertions);}for(v,e)in program.radial_p.into_iter().zip(c.radial_p){assert_bits(v,e,index,&mut assertions);}assert_eq!(program.radial_mask,c.radial_mask);assertions+=1;
 }
 println!("{} Source programs, {assertions} assertions",fixture.programs.len());
}
#[test]
fn complete_source_color_expansion(){
 let fixture=fixture();assert_eq!(fixture.colors.len(),1296);let mut assertions=1;
 for(index,c)in fixture.colors.iter().enumerate(){let a=c.input.0.map(f32::from_bits);let input=StopColor::from_rgba(a[0],a[1],a[2],a[3]);assert_eq!(input.is_some(),c.expanded.is_some());assertions+=1;if let(Some(input),Some(expected))=(input,c.expanded){for(v,e)in color_space(c.input.1).expand_color(input).components().into_iter().zip(expected){assert_bits(v,e,index,&mut assertions);}}}
 println!("{} Source colors, {assertions} assertions",fixture.colors.len());
}
#[test]
fn complete_source_approximate_power(){
 let fixture=fixture();assert_eq!(fixture.powers.len(),8010);let mut assertions=1;
 for(index,c)in fixture.powers.iter().enumerate(){assert_bits(approximate_power(f32::from_bits(c.input[0]),f32::from_bits(c.input[1])),c.result,index,&mut assertions);}
 println!("{} Source powers, {assertions} assertions",fixture.powers.len());
}
