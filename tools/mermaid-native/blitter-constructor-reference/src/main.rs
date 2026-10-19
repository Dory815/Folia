// Transport only: actual public factories, unchanged original blitter constructors and read-only appended projections.
use serde::{Deserialize,Serialize};
use serde_json::{Value,json};
use std::io::{self,Read};
use tiny_skia::{BlendMode,Color,ColorSpace,FilterQuality,GradientStop,IntRect,IntSize,LinearGradient,Mask,Paint,Pattern,PixmapRef,Point,RadialGradient,Shader,SpreadMode,SweepGradient,Transform};

static WARNINGS:std::sync::Mutex<Vec<String>>=std::sync::Mutex::new(Vec::new());
struct Observer;
static OBSERVER:Observer=Observer;
impl log::Log for Observer{
 fn enabled(&self,m:&log::Metadata)->bool{m.level()<=log::Level::Warn}
 fn log(&self,r:&log::Record){if self.enabled(r.metadata()){WARNINGS.lock().unwrap().push(r.args().to_string());}}
 fn flush(&self){}
}

#[derive(Clone,Deserialize,Serialize)]struct Rect{xy:[i32;2],wh:[u32;2]}
#[derive(Clone,Deserialize,Serialize)]struct MaskInput{bytes:Vec<u8>,size:[u32;2],rect:Option<Rect>}
#[derive(Clone,Deserialize,Serialize)]struct Input{
 label:String,kind:u8,blend:u8,color_space:u8,anti_alias:bool,force_hq:bool,
 color:[u32;4],stops:Vec<[u32;5]>,geometry:[u32;6],spread:u8,transform:[u32;6],
 pattern:[u32;4],image:Vec<u8>,transforms:Vec<[u32;6]>,opacities:Vec<u32>,
 target:Vec<u8>,target_size:[u32;2],target_rect:Option<Rect>,mask:Option<MaskInput>,
}
#[derive(Serialize)]struct Output{input:Input,factory:String,actual_variant:Option<String>,constructor:String,warnings:Vec<String>,snapshot:Option<Value>,target_after:Vec<u8>}

fn ts(bits:[u32;6])->Transform{let b=bits.map(f32::from_bits);Transform::from_row(b[0],b[1],b[2],b[3],b[4],b[5])}
fn rect(r:&Rect)->IntRect{IntRect::from_xywh(r.xy[0],r.xy[1],r.wh[0],r.wh[1]).unwrap()}
fn spread(id:u8)->SpreadMode{match id{0=>SpreadMode::Pad,1=>SpreadMode::Reflect,2=>SpreadMode::Repeat,_=>panic!("bad spread")}}
fn blend(id:u8)->BlendMode{
 const ALL:[BlendMode;29]=[BlendMode::Clear,BlendMode::Source,BlendMode::Destination,BlendMode::SourceOver,BlendMode::DestinationOver,BlendMode::SourceIn,BlendMode::DestinationIn,BlendMode::SourceOut,BlendMode::DestinationOut,BlendMode::SourceAtop,BlendMode::DestinationAtop,BlendMode::Xor,BlendMode::Plus,BlendMode::Modulate,BlendMode::Screen,BlendMode::Overlay,BlendMode::Darken,BlendMode::Lighten,BlendMode::ColorDodge,BlendMode::ColorBurn,BlendMode::HardLight,BlendMode::SoftLight,BlendMode::Difference,BlendMode::Exclusion,BlendMode::Multiply,BlendMode::Hue,BlendMode::Saturation,BlendMode::Color,BlendMode::Luminosity];
 ALL[id as usize]
}
fn color_space(id:u8)->ColorSpace{match id{0=>ColorSpace::Linear,1=>ColorSpace::Gamma2,2=>ColorSpace::SimpleSRGB,3=>ColorSpace::FullSRGBGamma,_=>panic!("bad color space")}}
fn variant(s:&Shader)->String{match s{Shader::SolidColor(_)=>"SolidColor",Shader::LinearGradient(_)=>"LinearGradient",Shader::RadialGradient(_)=>"RadialGradient",Shader::SweepGradient(_)=>"SweepGradient",Shader::Pattern(_)=>"Pattern"}.into()}
fn factory(i:&Input)->Option<Shader<'_>>{
 let c=i.color.map(f32::from_bits);
 let t=ts(i.transform);
 let spread=spread(i.spread);
 let mut stops=Vec::new();
 if (1..=3).contains(&i.kind){for r in &i.stops{let a=r.map(f32::from_bits);stops.push(GradientStop::new(a[0],Color::from_rgba(a[1],a[2],a[3],a[4])?));}}
 let g=i.geometry.map(f32::from_bits);
 let mut shader=match i.kind{
  0=>Shader::SolidColor(Color::from_rgba(c[0],c[1],c[2],c[3])?),
  1=>LinearGradient::new(Point::from_xy(g[0],g[1]),Point::from_xy(g[2],g[3]),stops,spread,t)?,
  2=>RadialGradient::new(Point::from_xy(g[0],g[1]),g[4],Point::from_xy(g[2],g[3]),g[5],stops,spread,t)?,
  3=>SweepGradient::new(Point::from_xy(g[0],g[1]),g[2],g[3],stops,spread,t)?,
  4=>Pattern::new(PixmapRef::from_bytes(&i.image,i.pattern[0],i.pattern[1])?,spread,match i.pattern[2]{0=>FilterQuality::Nearest,1=>FilterQuality::Bilinear,2=>FilterQuality::Bicubic,_=>panic!("bad quality")},f32::from_bits(i.pattern[3]),t),
  _=>panic!("bad shader kind"),
 };
 for tr in &i.transforms{shader.transform(ts(*tr));}
 for o in &i.opacities{shader.apply_opacity(f32::from_bits(*o));}
 Some(shader)
}
fn pipe(p:tiny_skia::FoliaConstructorSnapshot)->Value{
 fn one(p:tiny_skia::FoliaConstructorPipeline)->Value{json!({"high":p.high,"width":p.width,"full":p.full,"tail":p.tail,"context":p.context})}
 let (mw,mh,mrw,mo,ml)=match p.mask{Some((w,h,rw,o,l))=>(Some(w),Some(h),Some(rw),Some(o),Some(l)),None=>(None,None,None,None,None)};
 json!({"is_mask":p.is_mask,"logical_mask":if let(Some(w),Some(h),Some(rw),Some(o),Some(l))=(mw,mh,mrw,mo,ml){Some(json!({"width":w,"height":h,"real_width":rw,"offset":o,"slice_len":l}))}else{None},
  "target":{"width":p.target.0,"height":p.target.1,"real_width":p.target.2,"offset":p.target.3,"slice_len":p.target.4},
  "source":{"width":p.source.0,"height":p.source.1,"bytes":p.source.2,"pattern_input_identity":p.source.3},
  "memset":p.memset,"anti_h":one(p.anti_h),"rect":one(p.rect),"mask":one(p.mask_pipeline)})
}
fn observe(i:Input)->Output{
 let mut target=i.target.clone();
 let result=if i.kind==5{
  let size=IntSize::from_wh(i.target_size[0],i.target_size[1]).unwrap();
  let mut mask_target=Mask::from_vec(target,size).unwrap();
  let call=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||tiny_skia::folia_blitter_mask_constructor(&mut mask_target,i.target_rect.as_ref().map(rect))));
  target=mask_target.take();
  ("not-called".to_string(),None,call)
 }else{
  let made=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||factory(&i)));
  match made{
   Ok(Some(shader))=>{
    let actual=variant(&shader);
    let paint=Paint{shader,blend_mode:blend(i.blend),anti_alias:i.anti_alias,colorspace:color_space(i.color_space),force_hq_pipeline:i.force_hq};
    let built_mask=i.mask.as_ref().map(|m|Mask::from_vec(m.bytes.clone(),IntSize::from_wh(m.size[0],m.size[1]).unwrap()).unwrap());
    let mask_arg=built_mask.as_ref().zip(i.mask.as_ref()).map(|(m,s)|(m,s.rect.as_ref().map(rect)));
    let call=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||tiny_skia::folia_blitter_constructor(&paint,&mut target,i.target_size[0],i.target_size[1],i.target_rect.as_ref().map(rect),mask_arg,&i.image)));
    ("Some".to_string(),Some(actual),call)
   },
   Ok(None)=>return Output{input:i,factory:"None".into(),actual_variant:None,constructor:"not-called".into(),warnings:std::mem::take(&mut*WARNINGS.lock().unwrap()),snapshot:None,target_after:target},
   Err(_)=>return Output{input:i,factory:"panic".into(),actual_variant:None,constructor:"not-called".into(),warnings:std::mem::take(&mut*WARNINGS.lock().unwrap()),snapshot:None,target_after:target},
  }
 };
 let (factory,actual_variant,call)=result;
 let (constructor,snapshot)=match call{Ok(Ok(Some(s)))=>("Some".into(),Some(pipe(s))),Ok(Ok(None))=>("None".into(),None),Ok(Err(_))=>("not-called".into(),None),Err(_)=>("panic".into(),None)};
 let snapshot=snapshot.map(|mut s|{s["target_before"]=json!(&i.target);s["target_after"]=json!(&target);s});
 Output{input:i,factory,actual_variant,constructor,warnings:std::mem::take(&mut*WARNINGS.lock().unwrap()),snapshot,target_after:target}
}
fn main(){log::set_logger(&OBSERVER).unwrap();log::set_max_level(log::LevelFilter::Warn);std::panic::set_hook(Box::new(|_|{}));let mut text=String::new();io::stdin().read_to_string(&mut text).unwrap();let inputs:Vec<Input>=serde_json::from_str(&text).unwrap();let outputs:Vec<Output>=inputs.into_iter().map(observe).collect();println!("{}",serde_json::to_string(&outputs).unwrap());}
