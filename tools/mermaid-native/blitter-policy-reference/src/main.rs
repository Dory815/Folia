// Transport only: original policy methods and original normalized color conversion.
use serde::{Deserialize,Serialize};use std::io::{self,Read};
#[derive(Deserialize,Serialize)]struct Input{blend:u8,color_space:u8,color:[u32;4]}
#[derive(Serialize)]struct Case{input:Input,result:serde_json::Value}
fn main(){let mut data=String::new();io::stdin().read_to_string(&mut data).unwrap();let out:Vec<_>=serde_json::from_str::<Vec<Input>>(&data).unwrap().into_iter().map(|input|{let result=serde_json::to_value(tiny_skia::folia_blitter_policy(input.blend,input.color_space,input.color)).unwrap();Case{input,result}}).collect();println!("{}",serde_json::to_string(&out).unwrap());}
