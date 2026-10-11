// Root adapter observes original Source stages; no native preparation linked.
use serde::{Deserialize,Serialize};
use std::io::{self,Read};
#[derive(Deserialize,Serialize)]
struct Input { tag:u8, parameters:[u32;2], swapped:bool }
#[derive(Serialize)]
struct Output { input:Input, p:[u32;2], stages:Vec<u8>, geometry_lowp_compatible:bool }
fn main() {
 let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();
 let input:Vec<Input>=serde_json::from_str(&text).unwrap();
 let out:Vec<Output>=input.into_iter().map(|input|{
  let(p,stages,geometry_lowp_compatible)=tiny_skia::RadialGradient::folia_source_stage_snapshot(input.tag,input.parameters.map(f32::from_bits),input.swapped);
  Output{input,p,stages,geometry_lowp_compatible}
 }).collect();println!("{}",serde_json::to_string(&out).unwrap());
}
