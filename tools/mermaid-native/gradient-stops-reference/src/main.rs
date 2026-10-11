// Root adapter executes original Source stops/color/gradient; no native crate linked.
use serde::{Deserialize,Serialize};
use std::io::{self,Read};
#[derive(Deserialize,Serialize)]
struct Input { stops:Vec<[u32;5]>, spread:u8, transform:[u32;6], points_to_unit:[u32;6], opacity:Vec<u32> }
#[derive(Serialize)]
struct Output { input:Input, accepted:bool, constructed:Vec<[u32;5]>, states:Vec<Vec<[u32;5]>>, flags:Vec<[bool;2]>, spread:u8, transform:[u32;6], points_to_unit:[u32;6] }
fn main(){
 let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();
 let input:Vec<Input>=serde_json::from_str(&text).unwrap();
 let output:Vec<Output>=input.into_iter().map(|input|{
  let result=tiny_skia::folia_gradient_stop_snapshot(&input.stops,input.spread,input.transform,input.points_to_unit,&input.opacity);
  let(accepted,constructed,states,flags,spread,transform,points_to_unit)=match result{Some((a,b,c,d,e,f))=>(true,a,b,c,d,e,f),None=>(false,vec![],vec![],vec![],0,[0;6],[0;6])};
  Output{input,accepted,constructed,states,flags,spread,transform,points_to_unit}
 }).collect();println!("{}",serde_json::to_string(&output).unwrap());
}
