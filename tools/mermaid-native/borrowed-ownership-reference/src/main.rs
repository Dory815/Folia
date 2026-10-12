use serde::{Deserialize,Serialize};use std::io::{self,Read};
#[derive(Deserialize,Serialize)]struct Input{data:Vec<u8>,capacity:usize,width:u32,height:u32}
#[derive(Serialize)]struct Case{input:Input,result:(String,bool,usize,usize)}
fn main(){let mut data=String::new();io::stdin().read_to_string(&mut data).unwrap();let cases:Vec<_>=serde_json::from_str::<Vec<Input>>(&data).unwrap().into_iter().map(|input|Case{result:tiny_skia::folia_mask_ownership(&input.data,input.capacity,input.width,input.height),input}).collect();println!("{}",serde_json::to_string(&cases).unwrap());}
