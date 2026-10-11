// Root transport only. Original shader and original compile/run provide values.
use serde::{Deserialize,Serialize};use serde_json::Value;use std::io::{self,Read};
static WARNINGS:std::sync::Mutex<Vec<String>>=std::sync::Mutex::new(Vec::new());struct Observer;static OBSERVER:Observer=Observer;
impl log::Log for Observer{fn enabled(&self,m:&log::Metadata)->bool{m.level()<=log::Level::Warn}fn log(&self,r:&log::Record){if self.enabled(r.metadata()){WARNINGS.lock().unwrap().push(r.args().to_string());}}fn flush(&self){}}
#[derive(Deserialize)]struct Gradient{stops:Vec<[u32;5]>,spread:u8,transform:[u32;6],points_to_unit:[u32;6],opacity:Vec<u32>,color_space:u8,kind:Option<(u8,[u32;2],bool)>,prefix:usize,sentinel:bool,force:bool}
#[derive(Deserialize)]struct Pattern{width:u32,height:u32,spread:u8,quality:u8,opacity:u32,transform:[u32;6],transforms:Vec<[u32;6]>,opacities:Vec<u32>,color_space:u8,prefix:usize,sentinel:bool,force:bool}
#[derive(Deserialize,Serialize)]struct Input{family:String,source_index:usize,producer:Value,image:Vec<u8>,ids:Vec<u8>,destination:Vec<u8>,real_width:usize,mask:Vec<u8>,mask_width:u32,aa_pixels:[u8;2],aa_stride:u32,aa_shift:usize,origin:[u32;2],size:[u32;2]}
#[derive(Serialize)]struct Output{input:Input,result:Value,warnings:Vec<String>}
fn main(){log::set_logger(&OBSERVER).unwrap();log::set_max_level(log::LevelFilter::Warn);std::panic::set_hook(Box::new(|_|{}));let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();let inputs:Vec<Input>=serde_json::from_str(&text).unwrap();let out:Vec<_>=inputs.into_iter().map(|input|{
 let i=&input;let result=if i.family=="gradient"{let g:Gradient=serde_json::from_value(i.producer.clone()).unwrap();match g.kind{
 None=>serde_json::to_value(tiny_skia::folia_linear_run(&g.stops,g.spread,g.transform,g.points_to_unit,&g.opacity,g.color_space,g.prefix,g.sentinel,g.force,&i.ids,&i.destination,i.real_width,&i.mask,i.mask_width,i.aa_pixels,i.aa_stride,i.aa_shift,i.origin,i.size)).unwrap(),
 Some(kind)=>serde_json::to_value(tiny_skia::folia_radial_run(&g.stops,g.spread,g.transform,g.points_to_unit,&g.opacity,g.color_space,kind,g.prefix,g.sentinel,g.force,&i.ids,&i.destination,i.real_width,&i.mask,i.mask_width,i.aa_pixels,i.aa_stride,i.aa_shift,i.origin,i.size)).unwrap(),
 }}else{assert_eq!(i.family,"pattern");let p:Pattern=serde_json::from_value(i.producer.clone()).unwrap();serde_json::to_value(tiny_skia::folia_pattern_run(p.width,p.height,p.spread,p.quality,p.opacity,p.transform,&p.transforms,&p.opacities,p.color_space,p.prefix,p.sentinel,p.force,&i.image,&i.ids,&i.destination,i.real_width,&i.mask,i.mask_width,i.aa_pixels,i.aa_stride,i.aa_shift,i.origin,i.size)).unwrap()};
 let warnings=std::mem::take(&mut*WARNINGS.lock().unwrap());Output{input,result,warnings}
 }).collect();println!("{}",serde_json::to_string(&out).unwrap());}
