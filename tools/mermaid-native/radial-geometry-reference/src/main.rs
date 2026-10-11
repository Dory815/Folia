// Root-only actual Source observations. No native preparation module is linked.
use serde::{Deserialize, Serialize};
use std::io::{self, Read};
#[derive(Deserialize, Serialize)]
struct Input { circles: [u32; 6], transform: [u32; 6], stops: usize, spread: u8 }
#[derive(Serialize)]
struct Output { input: Input, tag: u8, matrix: [u32; 6], parameters: [u32; 3], swapped: bool, thin_ring: bool }
fn main() {
    let mut text=String::new(); io::stdin().read_to_string(&mut text).unwrap();
    let input: Vec<Input>=serde_json::from_str(&text).unwrap();
    let out: Vec<Output>=input.into_iter().map(|input| {
        let c=input.circles.map(f32::from_bits); let t=input.transform.map(f32::from_bits);
        let colors=[tiny_skia::Color::from_rgba8(255,0,0,255),tiny_skia::Color::from_rgba8(0,255,0,128),tiny_skia::Color::from_rgba8(0,0,255,255)];
        let stops=(0..input.stops).map(|i| tiny_skia::GradientStop::new(if input.stops<2 {0.0} else {i as f32/(input.stops-1) as f32},colors[i.min(2)])).collect();
        let spread=match input.spread {0=>tiny_skia::SpreadMode::Pad,1=>tiny_skia::SpreadMode::Reflect,2=>tiny_skia::SpreadMode::Repeat,_=>panic!("invalid stimulus")};
        let shader=tiny_skia::RadialGradient::new(tiny_skia::Point::from_xy(c[0],c[1]),c[2],tiny_skia::Point::from_xy(c[3],c[4]),c[5],stops,spread,tiny_skia::Transform::from_row(t[0],t[1],t[2],t[3],t[4],t[5]));
        let (tag,matrix,parameters,swapped,thin_ring)=match shader {None=>(0,[0;6],[0;3],false,false),Some(tiny_skia::Shader::SolidColor(_))=>(1,[0;6],[0;3],false,false),Some(tiny_skia::Shader::RadialGradient(g))=>g.folia_source_snapshot(),_=>panic!("unexpected Source shader")};
        Output {input,tag,matrix,parameters,swapped,thin_ring}
    }).collect();
    println!("{}",serde_json::to_string(&out).unwrap());
}
