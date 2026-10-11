// Root actual Source bit observations; expected data never comes from native.
use mermaid_rs_renderer::scene::{paint::SpreadMode,prepare::gradient_stop_types::{StopColor,PreparedGradientStop,PreparedGradient}};
use serde::Deserialize;
use tiny_skia_path::Transform;
#[derive(Deserialize)]
struct Input { stops:Vec<[u32;5]>, spread:u8, transform:[u32;6], points_to_unit:[u32;6], opacity:Vec<u32> }
#[derive(Deserialize)]
struct Case { input:Input, accepted:bool, constructed:Vec<[u32;5]>, states:Vec<Vec<[u32;5]>>, flags:Vec<[bool;2]>, spread:u8, transform:[u32;6], points_to_unit:[u32;6] }
#[derive(Deserialize)]
struct Fixture { cases:Vec<Case> }
fn transform(v:[u32;6])->Transform{let a=v.map(f32::from_bits);Transform::from_row(a[0],a[1],a[2],a[3],a[4],a[5])}
fn stop_bits(stops:&[PreparedGradientStop],expected:&[[u32;5]],index:usize,count:&mut usize){
 assert_eq!(stops.len(),expected.len(),"case {index}: stop count");*count+=1;
 for(stop,values)in stops.iter().zip(expected){let c=stop.color().components();let actual=[stop.position(),c[0],c[1],c[2],c[3]];
  for(value,expected)in actual.into_iter().zip(values){assert_eq!(value.to_bits(),*expected,"case {index}: stop bits");*count+=1;}
 }
}
#[test]
fn complete_source_gradient_stop_preparation(){
 let fixture:Fixture=serde_json::from_str(include_str!("fixtures/gradient-stops.json")).unwrap();assert_eq!(fixture.cases.len(),3242);let mut assertions=1;
 for(index,case)in fixture.cases.iter().enumerate(){
  let stops:Option<Vec<_>>=case.input.stops.iter().map(|v|{let a=v.map(f32::from_bits);StopColor::from_rgba(a[1],a[2],a[3],a[4]).map(|c|PreparedGradientStop::new(a[0],c))}).collect();
  assert_eq!(stops.is_some(),case.accepted,"case {index}: color construction");assertions+=1;let Some(stops)=stops else{continue};stop_bits(&stops,&case.constructed,index,&mut assertions);
  let spread=match case.input.spread{0=>SpreadMode::Pad,1=>SpreadMode::Reflect,2=>SpreadMode::Repeat,_=>panic!("bad Source stimulus")};
  let mut prepared=PreparedGradient::new(stops,spread,transform(case.input.transform),transform(case.input.points_to_unit));
  assert_eq!(prepared.spread() as u8,case.spread,"case {index}: spread");assertions+=1;
  for(t,expected)in[(prepared.transform(),case.transform),(prepared.points_to_unit(),case.points_to_unit)]{for(value,bits)in[t.sx,t.ky,t.kx,t.sy,t.tx,t.ty].into_iter().zip(expected){assert_eq!(value.to_bits(),bits,"case {index}: transform");assertions+=1;}}
  assert_eq!(case.states.len(),case.input.opacity.len()+1);assert_eq!(case.flags.len(),case.states.len());assertions+=2;
  for state in 0..case.states.len(){if state>0{prepared.apply_opacity(f32::from_bits(case.input.opacity[state-1]));}stop_bits(prepared.stops(),&case.states[state],index,&mut assertions);
   assert_eq!(prepared.colors_are_opaque(),case.flags[state][0],"case {index}: opaque");assert_eq!(prepared.has_uniform_stops(),case.flags[state][1],"case {index}: uniform");assertions+=2;
  }
 }println!("{} Source cases, {assertions} assertions",fixture.cases.len());
}
