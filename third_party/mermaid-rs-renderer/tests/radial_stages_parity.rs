// Root independent actual Source stage observations, never native-derived expectations.
use mermaid_rs_renderer::scene::prepare::{radial_stages::prepare_radial_stages,radial_types::RadialKind};
use serde::Deserialize;
#[derive(Deserialize)]
struct Input { tag:u8, parameters:[u32;2], swapped:bool }
#[derive(Deserialize)]
struct Case { input:Input, p:[u32;2], stages:Vec<u8>, geometry_lowp_compatible:bool }
#[derive(Deserialize)]
struct Fixture { cases:Vec<Case> }
fn bits(value:f32,expected:u32,index:usize) {
 if f32::from_bits(expected).is_nan(){assert!(value.is_nan(),"case {index}: NaN");}
 else{assert_eq!(value.to_bits(),expected,"case {index}: parameter");}
}
#[test]
fn complete_source_radial_stage_projection() {
 let fixture:Fixture=serde_json::from_str(include_str!("fixtures/radial-stages.json")).unwrap();
 assert_eq!(fixture.cases.len(),3266);let mut assertions=1;
 for(index,case)in fixture.cases.iter().enumerate(){
  let p=case.input.parameters.map(f32::from_bits);
  let kind=match case.input.tag{2=>RadialKind::Radial{radius1:p[0],radius2:p[1]},3=>RadialKind::Strip{scaled_r0:p[0]},4=>RadialKind::Focal{r1:p[0],focal_x:p[1],is_swapped:case.input.swapped},_=>panic!("invalid Source fixture")};
  let plan=prepare_radial_stages(kind);bits(plan.p0,case.p[0],index);bits(plan.p1,case.p[1],index);assertions+=2;
  let split=case.stages.iter().position(|s|*s==13).unwrap_or(case.stages.len());
  assert_eq!(plan.before_color.iter().map(|s|*s as u8).collect::<Vec<_>>(),case.stages[..split],"case {index}: before color");
  assert_eq!(plan.after_color.iter().map(|s|*s as u8).collect::<Vec<_>>(),case.stages[split..],"case {index}: after color");
  assert_eq!(plan.geometry_lowp_compatible,case.geometry_lowp_compatible,"case {index}: eligibility");assertions+=3;
 }println!("{} Source cases, {assertions} assertions",fixture.cases.len());
}
