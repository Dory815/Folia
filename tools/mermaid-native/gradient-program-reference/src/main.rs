// Root observer adapter. No native Mermaid crate is linked.
use serde::{Deserialize,Serialize};
use std::io::{self,Read};
#[derive(Deserialize,Serialize)]
struct Input {stops:Vec<[u32;5]>,spread:u8,transform:[u32;6],points_to_unit:[u32;6],opacity:Vec<u32>,color_space:u8,radial:Option<([u32;2],Vec<u8>)>}
#[derive(Deserialize)]
struct Request{programs:Vec<Input>,colors:Vec<([u32;4],u8)>,powers:Vec<[u32;2]>}
#[derive(Serialize)]
struct Output{input:Input,accepted:bool,returned:bool,stages:Vec<String>,transform:[u32;6],two_stop:[[u32;4];2],len:usize,factors:Vec<[u32;4]>,biases:Vec<[u32;4]>,t_values:Vec<u32>,radial_p:[u32;2],radial_mask:[u32;8]}
#[derive(Serialize)]
struct ColorOutput{input:([u32;4],u8),expanded:Option<[u32;4]>}
#[derive(Serialize)]
struct PowerOutput{input:[u32;2],result:u32}
#[derive(Serialize)]
struct Response{programs:Vec<Output>,colors:Vec<ColorOutput>,powers:Vec<PowerOutput>}
fn main(){
 let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();let request:Request=serde_json::from_str(&text).unwrap();
 let programs=request.programs.into_iter().map(|input|{
  let observed=tiny_skia::folia_gradient_program(&input.stops,input.spread,input.transform,input.points_to_unit,&input.opacity,input.color_space,input.radial.clone());
  match observed{Some(p)=>Output{input,accepted:true,returned:p.returned,stages:p.stages,transform:p.transform,two_stop:p.two_stop,len:p.len,factors:p.factors,biases:p.biases,t_values:p.t_values,radial_p:p.radial_p,radial_mask:p.radial_mask},None=>Output{input,accepted:false,returned:false,stages:vec![],transform:[0;6],two_stop:[[0;4];2],len:0,factors:vec![],biases:vec![],t_values:vec![],radial_p:[0;2],radial_mask:[0;8]}}
 }).collect();
 let colors=request.colors.into_iter().map(|input|ColorOutput{expanded:tiny_skia::folia_color_expansion(input.0,input.1),input}).collect();
 let powers=request.powers.into_iter().map(|input|PowerOutput{result:tiny_skia::folia_approximate_power(input[0],input[1]),input}).collect();
 println!("{}",serde_json::to_string(&Response{programs,colors,powers}).unwrap());
}
