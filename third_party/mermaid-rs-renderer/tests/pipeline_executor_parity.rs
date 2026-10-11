// Root full original run observations. Native output never supplies expected values.
use mermaid_rs_renderer::scene::{paint::SpreadMode,prepare::{pipeline_builder::PipelineBuilder,pipeline_context_types::PipelineContext,pipeline_control_types::{PipelineOperation,PipelinePrecision,ALL_PIPELINE_STAGES},pipeline_executor::run_pipeline,pipeline_executor_types::{PipelineResources,PipelineRunError},sampling::{pattern::SourcePixmap,resources::{DestinationSurface,MaskSurface,AaMask}}}};
use serde::Deserialize;
#[derive(Deserialize)]struct Input{
 ids:Vec<u8>,force:bool,uniform:[u32;4],uniform_bytes:[u16;4],coverage:u32,
 destination:Vec<u8>,real_width:usize,mask:Vec<u8>,mask_width:u32,
 aa_pixels:[u8;2],aa_stride:u32,aa_shift:usize,origin:[u32;2],size:[u32;2],
}
type Record=(bool,usize,Vec<(u8,bool)>,Vec<(u8,bool)>,bool,Vec<u8>,Vec<u32>,Vec<u32>);
#[derive(Deserialize)]struct Case{input:Input,result:Record}
#[derive(Deserialize)]struct Fixture{cases:Vec<Case>}
fn context(c:&PipelineContext)->Vec<u32>{
 let mut v=vec![c.current_coverage.to_bits()];v.extend(c.uniform_color.components.map(f32::to_bits));v.extend(c.uniform_color.rgba.map(u32::from));
 v.extend(c.two_stop.factor.components.map(f32::to_bits));v.extend(c.two_stop.bias.components.map(f32::to_bits));v.push(c.gradient.len as u32);
 v.extend(c.radial_p.map(f32::to_bits));v.extend(c.radial_mask);let t=c.transform;
 v.extend([c.limit_x.scale,c.limit_x.inv_scale,c.limit_y.scale,c.limit_y.inv_scale,t.sx,t.ky,t.kx,t.sy,t.tx,t.ty,c.sampler.inv_width,c.sampler.inv_height].map(f32::to_bits));
 v.push(match c.sampler.spread_mode{SpreadMode::Pad=>0,SpreadMode::Reflect=>1,SpreadMode::Repeat=>2});
 v.push(c.gradient.factors.len()as u32);for x in &c.gradient.factors{v.extend(x.components.map(f32::to_bits));}
 v.push(c.gradient.biases.len()as u32);for x in &c.gradient.biases{v.extend(x.components.map(f32::to_bits));}
 v.push(c.gradient.t_values.len()as u32);for x in &c.gradient.t_values{v.push(x.get().to_bits());}v
}
fn operations(ops:&[PipelineOperation])->Vec<(u8,bool)>{ops.iter().map(|x|match x{PipelineOperation::Stage(s)=>(*s as u8,false),PipelineOperation::Tail(s)=>(*s as u8,true),PipelineOperation::Return=>(79,false)}).collect()}
#[test]fn actual_original_complete_pipeline_run(){
 let fixture:Fixture=serde_json::from_str(include_str!("fixtures/pipeline-executor.json")).unwrap();assert_eq!(fixture.cases.len(),4608);
 let previous=std::panic::take_hook();std::panic::set_hook(Box::new(|_|{}));let mut mismatches=0;let mut stubs=0;let mut errors=0;let mut first=Vec::new();
 for(index,c)in fixture.cases.iter().enumerate(){let i=&c.input;
  let mut builder=PipelineBuilder::new();builder.set_force_hq_pipeline(i.force);builder.context.uniform_color.components=i.uniform.map(f32::from_bits);builder.context.uniform_color.rgba=i.uniform_bytes;builder.context.current_coverage=f32::from_bits(i.coverage);
  for id in &i.ids{builder.push(ALL_PIPELINE_STAGES[*id as usize]);}let mut p=builder.compile();let before=context(&p.context);
  let mut data=i.destination.clone();let dummy=[0u8;4];let source=SourcePixmap::from_bytes(&dummy,1,1).unwrap();let rect=tiny_skia_path::IntRect::from_xywh(i.origin[0]as i32,i.origin[1]as i32,i.size[0],i.size[1]).unwrap();
  let result=std::panic::catch_unwind(std::panic::AssertUnwindSafe(||run_pipeline(&mut p,&rect,PipelineResources{source,destination:DestinationSurface{data:&mut data,real_width:i.real_width},mask:MaskSurface{data:&i.mask,real_width:i.mask_width},aa_mask:AaMask{pixels:i.aa_pixels,stride:i.aa_stride,shift:i.aa_shift}})));
  if matches!(result,Ok(Err(PipelineRunError::Unimplemented))){stubs+=1;}let error=matches!(result,Ok(Err(_)));if error{errors+=1;}
  let actual=(p.precision==PipelinePrecision::High,p.width,operations(&p.full),operations(&p.tail),result.is_err(),data,before,context(&p.context));
  if error||actual!=c.result{mismatches+=1;if first.len()<4{first.push(format!("case {index} ids={:?} force={} error={error} panic={}/{}, destination equal={}, before equal={}, after equal={}",i.ids,i.force,actual.4,c.result.4,actual.5==c.result.5,actual.6==c.result.6,actual.7==c.result.7));}}
 }
 std::panic::set_hook(previous);println!("4608 actual Source complete run cases / {mismatches} mismatches / {stubs} actual stub calls / {errors} errors");assert_eq!(mismatches,0,"{}",first.join("\n"));
}
