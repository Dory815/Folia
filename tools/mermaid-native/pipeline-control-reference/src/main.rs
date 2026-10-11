// Root adapter links original tiny-skia only, not the native Mermaid crate.
use serde::{Deserialize,Serialize};
use std::io::{self,Read};
#[derive(Deserialize,Serialize)]
struct Input { stages:Vec<u8>,force:bool,marker:u32 }
type Observed=(bool,usize,Vec<(u8,bool)>,Vec<(u8,bool)>,[u32;13]);
#[derive(Serialize)]
struct Case { input:Input,result:Option<Observed> }
fn main(){
 std::panic::set_hook(Box::new(|_|{}));
 let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();
 let input:Vec<Input>=serde_json::from_str(&text).unwrap();
 let cases:Vec<_>=input.into_iter().map(|input|{let result=tiny_skia::folia_compile_control(&input.stages,input.force,input.marker);Case{input,result}}).collect();
 println!("{}",serde_json::to_string(&cases).unwrap());
}
