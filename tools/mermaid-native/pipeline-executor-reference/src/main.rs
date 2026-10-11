// Root transport only: original compile and RasterPipeline::run supply results.
use serde::{Deserialize,Serialize};
use std::io::{self,Read};
#[derive(Deserialize,Serialize)]
struct Input{
 ids:Vec<u8>,force:bool,uniform:[u32;4],uniform_bytes:[u16;4],coverage:u32,
 destination:Vec<u8>,real_width:usize,mask:Vec<u8>,mask_width:u32,
 aa_pixels:[u8;2],aa_stride:u32,aa_shift:usize,origin:[u32;2],size:[u32;2],
}
#[derive(Serialize)]
struct Case{input:Input,result:(bool,usize,Vec<(u8,bool)>,Vec<(u8,bool)>,bool,Vec<u8>,Vec<u32>,Vec<u32>)}
fn main(){
 let mut s=String::new();io::stdin().read_to_string(&mut s).unwrap();
 let cases:Vec<_>=serde_json::from_str::<Vec<Input>>(&s).unwrap().into_iter().map(|input|{
  let result=tiny_skia::folia_executor(&input.ids,input.force,input.uniform,input.uniform_bytes,input.coverage,&input.destination,input.real_width,&input.mask,input.mask_width,input.aa_pixels,input.aa_stride,input.aa_shift,input.origin,input.size);
  Case{input,result}
 }).collect();println!("{}",serde_json::to_string(&cases).unwrap());
}
