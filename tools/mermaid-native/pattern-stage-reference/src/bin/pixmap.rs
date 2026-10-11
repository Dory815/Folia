// Root calls the actual original public PixmapRef constructor and accessors.
use serde::{Deserialize,Serialize};use std::io::{self,Read};
#[derive(Deserialize,Serialize)]struct Input{width:u32,height:u32,data:Vec<u8>}
#[derive(Serialize)]struct Case{input:Input,result:Option<(u32,u32,Vec<u8>,bool)>}
fn main(){let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();let inputs:Vec<Input>=serde_json::from_str(&text).unwrap();let cases:Vec<_>=inputs.into_iter().map(|input|{let result=tiny_skia::PixmapRef::from_bytes(&input.data,input.width,input.height).map(|p|(p.width(),p.height(),p.data().to_vec(),p.data().as_ptr()==input.data.as_ptr()));Case{input,result}}).collect();println!("{}",serde_json::to_string(&cases).unwrap());}
