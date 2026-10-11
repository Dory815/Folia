// Root golden values execute original compile/start/stage bodies, never native algorithms.
use mermaid_rs_renderer::scene::prepare::{sampling::{sample_high_stage,sample_low_stage,HighRegisters,LowRegisters},gradient_program_types::{GradientProgram,GradientColor,TwoStopContext,GradientContext},pipeline_control::{compile_pipeline},pipeline_control_types::{ALL_PIPELINE_STAGES,PipelinePrecision}};
use serde::Deserialize;
use tiny_skia_path::{NormalizedF32,Transform};
#[derive(Deserialize)]struct Fixture {high_stages:Vec<u8>,low_stages:Vec<u8>,cases:Vec<Case>}
#[derive(Deserialize)]struct Case {input:Input,result:(bool,usize,Vec<(Vec<Vec<u32>>,[u32;8],[usize;3])>)}
#[derive(Deserialize)]struct Input {
 ids:Vec<u8>,force:bool,high:[[u32;8];8],low:[[u16;16];8],transform:[u32;6],two_stop:[[u32;4];2],len:usize,
 factors:Vec<[u32;4]>,biases:Vec<[u32;4]>,t:Vec<u32>,radial:[u32;2],mask:[u32;8],origin:[u32;2],length:u32,
}
fn fixture()->Fixture{serde_json::from_str(include_str!("fixtures/sampling.json")).unwrap()}
fn context(i:&Input)->GradientProgram{
 let color=|v:[u32;4]|GradientColor{components:v.map(f32::from_bits)};let t=i.transform.map(f32::from_bits);
 GradientProgram{returned:true,stages:Vec::new(),transform:Transform::from_row(t[0],t[1],t[2],t[3],t[4],t[5]),two_stop:TwoStopContext{factor:color(i.two_stop[0]),bias:color(i.two_stop[1])},gradient:GradientContext{len:i.len,factors:i.factors.iter().copied().map(color).collect(),biases:i.biases.iter().copied().map(color).collect(),t_values:i.t.iter().map(|b|NormalizedF32::new(f32::from_bits(*b)).unwrap()).collect()},radial_p:i.radial.map(f32::from_bits),radial_mask:i.mask}
}
fn context_bits(p:&GradientProgram)->Vec<u64>{
 let t=p.transform;let mut v=vec![p.returned as u64,p.stages.len() as u64,p.gradient.len as u64,p.gradient.factors.len() as u64,p.gradient.biases.len() as u64,p.gradient.t_values.len() as u64];
 for x in [t.sx,t.ky,t.kx,t.sy,t.tx,t.ty].into_iter().chain(p.two_stop.factor.components).chain(p.two_stop.bias.components).chain(p.radial_p){v.push(x.to_bits() as u64);}
 v.extend(p.radial_mask.map(u64::from));
 for c in p.gradient.factors.iter().chain(&p.gradient.biases){v.extend(c.components.map(|x|x.to_bits() as u64));}
 v.extend(p.gradient.t_values.iter().map(|x|x.get().to_bits() as u64));v
}
fn check_cases(high_precision:bool){
 let f=fixture();let mut assertions=0usize;
 let mut cases=0;for (case_id,c) in f.cases.iter().enumerate().filter(|(_,c)|c.result.0==high_precision){cases+=1;let i=&c.input;let stages:Vec<_>=i.ids.iter().map(|id|ALL_PIPELINE_STAGES[*id as usize]).collect();let plan=compile_pipeline(&stages,i.force,());
  assert_eq!(plan.precision,if c.result.0{PipelinePrecision::High}else{PipelinePrecision::Low},"case {case_id} precision");assert_eq!(plan.width,c.result.1);assertions+=2;
  let mut ctx=context(i);let mut high=HighRegisters{channels:i.high.map(|v|v.map(f32::from_bits))};let mut low=LowRegisters{channels:i.low};
  for (step,stage) in stages.into_iter().enumerate(){let expected=&c.result.2[step];let origin=i.origin.map(|v|v as usize);
   let actual=if c.result.0{sample_high_stage(stage,&mut high,&mut ctx,origin).unwrap();high.channels.iter().map(|v|v.map(f32::to_bits).to_vec()).collect::<Vec<_>>()}else{sample_low_stage(stage,&mut low,&mut ctx,origin).unwrap();low.channels.iter().map(|v|v.map(u32::from).to_vec()).collect::<Vec<_>>()};
   for channel in 0..8{for lane in 0..c.result.1{assert_eq!(actual[channel][lane],expected.0[channel][lane],"case {case_id} step {step} {stage:?} channel {channel} lane {lane}");assertions+=1;}}
   for lane in 0..8{assert_eq!(ctx.radial_mask[lane],expected.1[lane],"case {case_id} step {step} mask lane {lane}");assertions+=1;}
   assert_eq!(expected.2,[origin[0],origin[1],i.length.min(c.result.1 as u32) as usize]);assertions+=1;
  }
  assert_eq!(ctx.radial_p.map(f32::to_bits),i.radial);assert_eq!(ctx.two_stop.factor.components.map(f32::to_bits),i.two_stop[0]);assert_eq!(ctx.two_stop.bias.components.map(f32::to_bits),i.two_stop[1]);assert_eq!(ctx.gradient.len,i.len);assert_eq!(ctx.gradient.factors.iter().map(|c|c.components.map(f32::to_bits)).collect::<Vec<_>>(),i.factors);assert_eq!(ctx.gradient.biases.iter().map(|c|c.components.map(f32::to_bits)).collect::<Vec<_>>(),i.biases);assert_eq!(ctx.gradient.t_values.iter().map(|v|v.get().to_bits()).collect::<Vec<_>>(),i.t);let t=ctx.transform;assert_eq!([t.sx,t.ky,t.kx,t.sy,t.tx,t.ty].map(f32::to_bits),i.transform);assertions+=8;
 }
 println!("{cases} Source cases / {assertions} assertions");
}
#[test]fn actual_source_high_sampling_registers_and_masks(){check_cases(true);}
#[test]fn actual_source_low_sampling_registers_and_masks(){check_cases(false);}
#[test]fn stages_outside_sampling_packet_return_explicit_error_without_mutation(){
 let f=fixture();let i=&f.cases[0].input;let mut assertions=0;
 for (id,stage) in ALL_PIPELINE_STAGES.iter().copied().enumerate(){
  if !f.high_stages.contains(&(id as u8)){let mut h=HighRegisters{channels:i.high.map(|v|v.map(f32::from_bits))};let before=h.channels.map(|v|v.map(f32::to_bits));let mut ctx=context(i);let ctx_before=context_bits(&ctx);let result=sample_high_stage(stage,&mut h,&mut ctx,[0,0]);assert_eq!(result.is_err(),true,"highp accepted unsupported {stage:?}");let e=result.unwrap_err();assert_eq!(e.stage,stage);assert_eq!(e.precision,PipelinePrecision::High);assert_eq!(h.channels.map(|v|v.map(f32::to_bits)),before);assert_eq!(ctx.radial_mask,i.mask);assert_eq!(context_bits(&ctx),ctx_before);assertions+=6;}
  if !f.low_stages.contains(&(id as u8)){let mut l=LowRegisters{channels:i.low};let before=l;let mut ctx=context(i);let ctx_before=context_bits(&ctx);let result=sample_low_stage(stage,&mut l,&mut ctx,[0,0]);assert_eq!(result.is_err(),true,"lowp accepted unsupported {stage:?}");let e=result.unwrap_err();assert_eq!(e.stage,stage);assert_eq!(e.precision,PipelinePrecision::Low);assert_eq!(l,before);assert_eq!(ctx.radial_mask,i.mask);assert_eq!(context_bits(&ctx),ctx_before);assertions+=6;}
 }
 println!("{assertions} explicit unsupported-stage assertions");
}
