// Only the byte-guarded original Source executes to create expected values.
use serde::{Deserialize,Serialize};
use std::io::{self,Read};
#[derive(Deserialize,Serialize)]
struct Input {
 ids:Vec<u8>,force:bool,high:[[u32;8];8],low:[[u16;16];8],
 transform:[u32;6],two_stop:[[u32;4];2],len:usize,
 factors:Vec<[u32;4]>,biases:Vec<[u32;4]>,t:Vec<u32>,radial:[u32;2],mask:[u32;8],
 origin:[u32;2],length:u32,
}
#[derive(Serialize)]
struct Case {input:Input,result:(bool,usize,Vec<(Vec<Vec<u32>>,[u32;8],[usize;3])>)}
fn main(){
 let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();
 let cases:Vec<_>=serde_json::from_str::<Vec<Input>>(&text).unwrap().into_iter().map(|input|{
  let result=tiny_skia::folia_sampling(&input.ids,input.force,input.high,input.low,input.transform,input.two_stop,input.len,&input.factors,&input.biases,&input.t,input.radial,input.mask,input.origin,input.length);
  Case{input,result}
 }).collect();
 println!("{}",serde_json::to_string(&cases).unwrap());
}
