// Independent Source SVG oracle only; never linked into the native scene consumer.
use serde::{Deserialize, Serialize};
use std::io::{self, Read};

#[derive(Deserialize, Serialize)]
struct Input {
    mode: String,
    background: [u8; 4],
    foreground: [u8; 4],
    opacity: f64,
}
#[derive(Serialize)]
struct Sample { dpi: u32, background_bgra: [u8; 4], overlap_bgra: [u8; 4] }
#[derive(Serialize)]
struct Output { input: Input, samples: Vec<Sample> }
fn color(c: &[u8; 4]) -> String { format!("#{:02x}{:02x}{:02x}",c[0],c[1],c[2]) }
fn pixel(p: &resvg::tiny_skia::Pixmap, dpi: u32, x: u32, y: u32) -> [u8; 4] {
    let c = p.pixel(x * dpi / 96, y * dpi / 96).unwrap();
    [c.blue(),c.green(),c.red(),c.alpha()]
}
fn main() {
    let mut source = String::new();io::stdin().read_to_string(&mut source).unwrap();
    let inputs: Vec<Input> = serde_json::from_str(&source).unwrap();
    let outputs: Vec<Output> = inputs.into_iter().map(|input| {
        let svg = format!(r#"<svg xmlns="http://www.w3.org/2000/svg" width="100" height="80" viewBox="0 0 100 80"><rect width="100" height="80" fill="{}" fill-opacity="{}"/><g style="mix-blend-mode:{}" opacity="{}"><rect x="24" y="16" width="40" height="32" fill="{}" fill-opacity="{}"/></g></svg>"#,
            color(&input.background),f64::from(input.background[3])/255.0,input.mode,input.opacity,
            color(&input.foreground),f64::from(input.foreground[3])/255.0);
        let tree = usvg::Tree::from_str(&svg,&usvg::Options::default()).unwrap();
        let samples = [96,120,144,192].into_iter().map(|dpi| {
            let mut image = resvg::tiny_skia::Pixmap::new(100*dpi/96,80*dpi/96).unwrap();
            resvg::render(&tree,resvg::tiny_skia::Transform::from_scale(dpi as f32/96.0,dpi as f32/96.0),&mut image.as_mut());
            Sample {dpi,background_bgra:pixel(&image,dpi,5,5),overlap_bgra:pixel(&image,dpi,40,32)}
        }).collect();
        Output {input,samples}
    }).collect();
    println!("{}",serde_json::to_string(&outputs).unwrap());
}
