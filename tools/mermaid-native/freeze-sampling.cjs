// Root-only declaration and Source-based test freeze, before implementation delegation.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),vendor=path.join(root,'third_party/mermaid-rs-renderer');
const files={
 'src/scene/prepare/sampling/mod.rs':`//! Shared resource-independent gradient sampling; no SVG or platform APIs.
mod wide;
mod highp;
mod lowp;
mod types;
pub use highp::sample_high_stage;
pub use lowp::sample_low_stage;
pub use types::{HighRegisters,LowRegisters,SamplingError};
`,
 'src/scene/prepare/sampling/types.rs':`//! Root-frozen typed registers, with one mutable shared GradientProgram context.
use super::super::pipeline_control_types::{PipelineStage,PipelinePrecision};
/// Source register order r,g,b,a,dr,dg,db,da. Eight f32 lanes per channel.
#[derive(Debug,Clone,Copy,Default)]
pub struct HighRegisters {pub channels:[[f32;8];8]}
/// The same register order. Sixteen raw u16 lanes; coordinate phase is bit-packed.
#[derive(Debug,Clone,Copy,Default,PartialEq,Eq)]
pub struct LowRegisters {pub channels:[[u16;16];8]}
#[derive(Debug,Clone,Copy,PartialEq,Eq)]
pub struct SamplingError {pub precision:PipelinePrecision,pub stage:PipelineStage}
`,
 'src/scene/prepare/sampling/highp.rs':`// Root stub: must fail the independent actual Source register corpus.
use super::{HighRegisters,SamplingError};
use super::super::{gradient_program_types::GradientProgram,pipeline_control_types::PipelineStage};
pub fn sample_high_stage(_:PipelineStage,_:&mut HighRegisters,_:&mut GradientProgram,_:[usize;2])->Result<(),SamplingError>{unimplemented!("M3i2 highp Source stage stub")}
`,
 'src/scene/prepare/sampling/lowp.rs':`// Root stub: must fail the independent actual Source register corpus.
use super::{LowRegisters,SamplingError};
use super::super::{gradient_program_types::GradientProgram,pipeline_control_types::PipelineStage};
pub fn sample_low_stage(_:PipelineStage,_:&mut LowRegisters,_:&mut GradientProgram,_:[usize;2])->Result<(),SamplingError>{unimplemented!("M3i2 lowp Source stage stub")}
`,
 'tests/sampling_parity.rs':`// Root golden values execute original compile/start/stage bodies, never native algorithms.
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
  if !f.high_stages.contains(&(id as u8)){let mut h=HighRegisters{channels:i.high.map(|v|v.map(f32::from_bits))};let before=h.channels.map(|v|v.map(f32::to_bits));let mut ctx=context(i);let e=sample_high_stage(stage,&mut h,&mut ctx,[0,0]).unwrap_err();assert_eq!(e.stage,stage);assert_eq!(e.precision,PipelinePrecision::High);assert_eq!(h.channels.map(|v|v.map(f32::to_bits)),before);assert_eq!(ctx.radial_mask,i.mask);assertions+=4;}
  if !f.low_stages.contains(&(id as u8)){let mut l=LowRegisters{channels:i.low};let before=l;let mut ctx=context(i);let e=sample_low_stage(stage,&mut l,&mut ctx,[0,0]).unwrap_err();assert_eq!(e.stage,stage);assert_eq!(e.precision,PipelinePrecision::Low);assert_eq!(l,before);assert_eq!(ctx.radial_mask,i.mask);assertions+=4;}
 }
 println!("{assertions} explicit unsupported-stage assertions");
}
`
};
for(const[rel,text]of Object.entries(files)){const file=path.join(vendor,rel);assert.ok(!fs.existsSync(file),'Do not overwrite frozen files '+rel);fs.mkdirSync(path.dirname(file),{recursive:true});fs.writeFileSync(file,text);}
const mod=path.join(vendor,'src/scene/prepare/mod.rs');assert.ok(!fs.readFileSync(mod,'utf8').includes('pub mod sampling;'));fs.appendFileSync(mod,'\npub mod sampling;\n');
const manifest=path.join(vendor,'Cargo.toml');let text=fs.readFileSync(manifest,'utf8');assert.ok(!text.includes('name = "sampling_parity"'));text=text.replace('log = "=0.4.33"','log = "=0.4.33"\nbytemuck = "=1.25.0"\ncfg-if = "=1.0.4"');text+='\n[[test]]\nname = "sampling_parity"\npath = "tests/sampling_parity.rs"\n';fs.writeFileSync(manifest,text);
const lock=path.join(vendor,'Cargo.lock');text=fs.readFileSync(lock,'utf8');const start=text.indexOf('name = "mermaid-rs-renderer"'),end=text.indexOf('[[package]]',start);let section=text.slice(start,end);section=section.replace(' "anyhow",',' "anyhow",\n "bytemuck",\n "cfg-if",');fs.writeFileSync(lock,text.slice(0,start)+section+text.slice(end));
console.log('Root frozen sampling interfaces, independent tests and named stubs created.');
