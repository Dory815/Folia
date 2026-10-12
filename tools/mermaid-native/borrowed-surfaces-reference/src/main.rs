// Transport only; constructors/subviews are actual unchanged original methods.
use serde::{Deserialize,Serialize};use std::io::{self,Read};
#[derive(Deserialize,Serialize)]struct Input{kind:u8,width:u32,height:u32,data:Vec<u8>,rect:Option<[i32;4]>,write:Option<(usize,u8)>}
#[derive(Serialize)]struct Case{input:Input,result:serde_json::Value}
fn main(){let mut data=String::new();io::stdin().read_to_string(&mut data).unwrap();let cases:Vec<_>=serde_json::from_str::<Vec<Input>>(&data).unwrap().into_iter().map(|input|{let result=if input.kind==0{tiny_skia::folia_borrowed_pixmap(input.data.clone(),input.width,input.height,input.rect,input.write)}else{tiny_skia::folia_borrowed_mask(input.data.clone(),input.width,input.height,input.rect,input.kind==2,input.write)};Case{input,result:serde_json::to_value(result).unwrap()}}).collect();println!("{}",serde_json::to_string(&cases).unwrap());}
