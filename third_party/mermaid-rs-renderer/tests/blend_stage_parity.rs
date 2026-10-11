// Root gold executes original pinned compile/start/stage pointers, not native code.
use mermaid_rs_renderer::scene::prepare::{sampling::{HighRegisters,LowRegisters,SamplingError,blending::*},pipeline_control::compile_pipeline,pipeline_control_types::{ALL_PIPELINE_STAGES,PipelinePrecision}};
use serde::Deserialize;
#[derive(Deserialize)]struct Fixture{high_stages:Vec<u8>,low_stages:Vec<u8>,cases:Vec<Case>}
#[derive(Deserialize)]struct Case{input:Input,result:(bool,usize,Vec<Capture>)}
type Capture=(Vec<Vec<u32>>,[usize;3]);
#[derive(Deserialize)]struct Input{ids:Vec<u8>,force:bool,high:[[u32;8];8],low:[[u16;16];8],origin:[u32;2],size:[u32;2]}
fn fixture()->Fixture{serde_json::from_str(include_str!("fixtures/blend-stages-compact.json")).unwrap()}
fn cases(high:bool){let f=fixture();let mut count=0;let mut assertions=0;
 for(case_id,c)in f.cases.iter().enumerate().filter(|(_,c)|c.result.0==high){count+=1;let i=&c.input;let stages:Vec<_>=i.ids.iter().map(|id|ALL_PIPELINE_STAGES[*id as usize]).collect();let plan=compile_pipeline(&stages,i.force,());
  assert_eq!(plan.precision,if high{PipelinePrecision::High}else{PipelinePrecision::Low});assert_eq!(plan.width,c.result.1);assertions+=2;
  let mut observed=Vec::new();
  for dy in i.origin[1]as usize..(i.origin[1]+i.size[1])as usize{let mut dx=i.origin[0]as usize;let end=dx+i.size[0]as usize;while dx<end{
   let tail=(end-dx).min(plan.width);let mut h=HighRegisters{channels:i.high.map(|v|v.map(f32::from_bits))};let mut l=LowRegisters{channels:i.low};
   let capture=|h:&HighRegisters,l:&LowRegisters|->Capture{(if high{h.channels.iter().map(|v|v.map(f32::to_bits).to_vec()).collect()}else{l.channels.iter().map(|v|v.map(u32::from).to_vec()).collect()},[dx,dy,tail])};
   observed.push(capture(&h,&l));for stage in stages.iter().copied(){let result=if high{blend_high_stage(stage,&mut h)}else{blend_low_stage(stage,&mut l)};assert!(result.is_ok(),"actual blend entry: {result:?}");assertions+=1;observed.push(capture(&h,&l));}dx+=tail;
  }}
  assert_eq!(observed.len(),c.result.2.len(),"case {case_id} continuation count");assertions+=1;
  for(step,(a,e))in observed.iter().zip(&c.result.2).enumerate(){assert_eq!(a.1,e.1,"case {case_id} capture {step} cursor");assertions+=1;
   for channel in 0..8{for lane in 0..plan.width{assert_eq!(a.0[channel][lane],e.0[channel][lane],"case {case_id} capture {step} channel {channel} lane {lane}");assertions+=1;}}
  }
 }println!("{count} original Source blend cases / {assertions} assertions");
}
#[test]fn actual_source_high_blend_registers(){cases(true);}
#[test]fn actual_source_low_blend_registers(){cases(false);}
#[test]fn unbound_blend_stages_return_error_without_mutation(){let f=fixture();let i=&f.cases[0].input;let mut count=0;
 for(id,stage)in ALL_PIPELINE_STAGES.iter().copied().enumerate(){for high in [false,true]{if (if high{&f.high_stages}else{&f.low_stages}).contains(&(id as u8)){continue;}
  let mut h=HighRegisters{channels:i.high.map(|v|v.map(f32::from_bits))};let mut l=LowRegisters{channels:i.low};let before=h.channels.map(|v|v.map(f32::to_bits));let low_before=l;
  let result=if high{blend_high_stage(stage,&mut h)}else{blend_low_stage(stage,&mut l)};
  assert_eq!(result,Err(SamplingError{precision:if high{PipelinePrecision::High}else{PipelinePrecision::Low},stage}));assert_eq!(h.channels.map(|v|v.map(f32::to_bits)),before);assert_eq!(l,low_before);count+=3;
 }}println!("{count} unsupported blend assertions");
}
