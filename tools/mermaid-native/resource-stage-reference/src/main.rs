// Root input transport only; actual original compiler/start/stages produce gold.
use serde::{Deserialize,Serialize};
use std::io::{self,Read};
#[derive(Deserialize,Serialize)]
struct Input {
 ids:Vec<u8>,force:bool,high:[[u32;8];8],low:[[u16;16];8],
 uniform:[u32;4],uniform_bytes:[u16;4],coverage:u32,
 destination:Vec<u8>,real_width:usize,mask:Vec<u8>,mask_width:u32,
 aa_pixels:[u8;2],aa_stride:u32,aa_shift:usize,origin:[u32;2],size:[u32;2],
}
#[derive(Serialize)]
struct Case {input:Input,result:(bool,usize,Vec<(Vec<Vec<u32>>,[usize;3],Vec<u8>)>,Vec<u8>)}
fn main(){
 let mut input=String::new();io::stdin().read_to_string(&mut input).unwrap();
 let cases:Vec<_>=serde_json::from_str::<Vec<Input>>(&input).unwrap().into_iter().map(|input|{
  let result=tiny_skia::folia_resources(&input.ids,input.force,input.high,input.low,input.uniform,input.uniform_bytes,input.coverage,&input.destination,input.real_width,&input.mask,input.mask_width,input.aa_pixels,input.aa_stride,input.aa_shift,input.origin,input.size);
  Case{input,result}
 }).collect();println!("{}",serde_json::to_string(&cases).unwrap());
}
