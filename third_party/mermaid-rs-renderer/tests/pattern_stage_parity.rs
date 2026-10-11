// Root gold executes pinned original compile/start/stage pointers and borrowed PixmapRef.
use mermaid_rs_renderer::scene::{paint::SpreadMode,prepare::{
 sampling::{HighRegisters,LowRegisters,SamplingError,sample_high_stage,pattern::*},
 gradient_program_types::{GradientProgram,GradientContext,TwoStopContext},
 pipeline_control::compile_pipeline,pipeline_control_types::{ALL_PIPELINE_STAGES,PipelinePrecision},
}};
use serde::Deserialize;
#[derive(Deserialize)]struct Fixture{high_stages:Vec<u8>,low_stages:Vec<u8>,cases:Vec<Case>}
#[derive(Deserialize)]struct Case{input:Input,result:(bool,usize,Vec<Capture>)}
type Capture=(Vec<Vec<u32>>,[usize;3]);
#[derive(Deserialize)]struct Input{ids:Vec<u8>,force:bool,high:[[u32;8];8],low:[[u16;16];8],origin:[u32;2],size:[u32;2],image:Vec<u8>,dimensions:[u32;2],limits:[u32;4],spread:u8,sampler:[u32;2]}
fn fixture()->Fixture{let mut f:Fixture=serde_json::from_str(include_str!("fixtures/pattern-stages.json")).unwrap();let tall:Fixture=serde_json::from_str(include_str!("fixtures/pattern-tall.json")).unwrap();assert_eq!(f.high_stages,tall.high_stages);assert_eq!(f.low_stages,tall.low_stages);f.cases.extend(tall.cases);f}
fn context(i:&Input)->PatternContext<'_>{PatternContext{
 source:SourcePixmap::from_bytes(&i.image,i.dimensions[0],i.dimensions[1]).unwrap(),
 limit_x:TileContext{scale:f32::from_bits(i.limits[0]),inv_scale:f32::from_bits(i.limits[1])},
 limit_y:TileContext{scale:f32::from_bits(i.limits[2]),inv_scale:f32::from_bits(i.limits[3])},
 sampler:SamplerContext{spread_mode:match i.spread{0=>SpreadMode::Pad,1=>SpreadMode::Reflect,2=>SpreadMode::Repeat,_=>panic!("bad Source input")},inv_width:f32::from_bits(i.sampler[0]),inv_height:f32::from_bits(i.sampler[1])},
}}
#[test]fn actual_source_pattern_registers(){let f=fixture();let mut count=0;let mut assertions=0;let mut rejected=Vec::new();
 'cases:for(case_id,c)in f.cases.iter().enumerate(){count+=1;let i=&c.input;let context=context(i);let image_before=i.image.clone();
  assert_eq!(context.source.data().as_ptr(),i.image.as_ptr());assert_eq!(context.source.width(),i.dimensions[0]);assert_eq!(context.source.height(),i.dimensions[1]);assertions+=3;
  let stages:Vec<_>=i.ids.iter().map(|id|ALL_PIPELINE_STAGES[*id as usize]).collect();let plan=compile_pipeline(&stages,i.force,());
  assert!(c.result.0);assert_eq!(plan.precision,PipelinePrecision::High);assert_eq!(plan.width,c.result.1);assertions+=3;
  let mut observed=Vec::new();
  for dy in i.origin[1]as usize..(i.origin[1]+i.size[1])as usize{let mut dx=i.origin[0]as usize;let end=dx+i.size[0]as usize;while dx<end{
   let tail=(end-dx).min(plan.width);let mut h=HighRegisters{channels:i.high.map(|v|v.map(f32::from_bits))};
   let mut program=GradientProgram{returned:true,stages:Vec::new(),transform:tiny_skia_path::Transform::identity(),two_stop:TwoStopContext::default(),gradient:GradientContext::default(),radial_p:[0.0;2],radial_mask:[0;8]};
   let capture=|h:&HighRegisters|->Capture{(h.channels.iter().map(|v|v.map(f32::to_bits).to_vec()).collect(),[dx,dy,tail])};
   observed.push(capture(&h));for stage in stages.iter().copied(){let result=if f.high_stages.contains(&(stage as u8)){pattern_high_stage(stage,&mut h,&context)}else{sample_high_stage(stage,&mut h,&mut program,[dx,dy])};if let Err(error)=result{rejected.push((case_id,error));continue 'cases;}assertions+=1;observed.push(capture(&h));}dx+=tail;
  }}
  assert_eq!(observed.len(),c.result.2.len(),"case {case_id} continuation count");assertions+=1;
  for(step,(a,e))in observed.iter().zip(&c.result.2).enumerate(){assert_eq!(a.1,e.1,"case {case_id} capture {step} cursor");assertions+=1;
   for channel in 0..8{for lane in 0..plan.width{assert_eq!(a.0[channel][lane],e.0[channel][lane],"case {case_id} capture {step} channel {channel} lane {lane}");assertions+=1;}}
  }
  assert_eq!(i.image,image_before);assertions+=1;
 }println!("{} actual pattern cases rejected",rejected.len());assert!(rejected.is_empty(),"Source-bound actual entries rejected: {:?}",&rejected[..rejected.len().min(8)]);assert_eq!(count,7056);assertions+=2;println!("{count} original Source pattern cases / {assertions} assertions");
}
#[test]fn unbound_pattern_stages_return_error_without_mutation(){let f=fixture();let i=&f.cases[0].input;let context=context(i);let mut count=0;
 for(id,stage)in ALL_PIPELINE_STAGES.iter().copied().enumerate(){for high in [false,true]{if (if high{&f.high_stages}else{&f.low_stages}).contains(&(id as u8)){continue;}
  let mut h=HighRegisters{channels:i.high.map(|v|v.map(f32::from_bits))};let mut l=LowRegisters{channels:i.low};let before=h.channels.map(|v|v.map(f32::to_bits));let low_before=l;
  let result=if high{pattern_high_stage(stage,&mut h,&context)}else{pattern_low_stage(stage,&mut l,&context)};
  assert_eq!(result,Err(SamplingError{precision:if high{PipelinePrecision::High}else{PipelinePrecision::Low},stage}));assert_eq!(h.channels.map(|v|v.map(f32::to_bits)),before);assert_eq!(l,low_before);count+=3;
 }}println!("{count} unsupported pattern assertions");
}
#[test]fn actual_source_pixmap_storage_and_borrow(){
 #[derive(Deserialize)]struct PixmapInput{width:u32,height:u32,data:Vec<u8>}
 #[derive(Deserialize)]struct PixmapCase{input:PixmapInput,result:Option<(u32,u32,Vec<u8>,bool)>}
 #[derive(Deserialize)]struct PixmapFixture{cases:Vec<PixmapCase>}
 let f:PixmapFixture=serde_json::from_str(include_str!("fixtures/pattern-pixmap.json")).unwrap();let mut assertions=1;assert_eq!(f.cases.len(),1694);
 for(case_id,c)in f.cases.iter().enumerate(){let i=&c.input;let actual=SourcePixmap::from_bytes(&i.data,i.width,i.height).map(|p|(p.width(),p.height(),p.data().to_vec(),p.data().as_ptr()==i.data.as_ptr()));assert_eq!(actual,c.result,"original PixmapRef case {case_id}");assertions+=1;}
 println!("1694 original Source pixmap cases / {assertions} assertions");
}
