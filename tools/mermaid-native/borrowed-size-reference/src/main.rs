use serde::{Deserialize,Serialize};use std::io::{self,Read};
#[derive(Deserialize,Serialize)]struct Input{width:u32,height:u32,row_bytes:usize}
#[derive(Serialize)]struct Case{input:Input,result:(bool,Option<usize>,Option<usize>,Option<usize>)}
fn main(){let mut data=String::new();io::stdin().read_to_string(&mut data).unwrap();let cases:Vec<_>=serde_json::from_str::<Vec<Input>>(&data).unwrap().into_iter().map(|input|Case{result:tiny_skia::folia_borrowed_size(input.width,input.height,input.row_bytes),input}).collect();println!("{}",serde_json::to_string(&cases).unwrap());}
