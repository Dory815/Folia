// Root expected values execute actual pinned Source compile/start/stage pointers.
use mermaid_rs_renderer::scene::prepare::{sampling::{HighRegisters,LowRegisters,sample_high_stage,sample_low_stage,resources::*},pipeline_control::compile_pipeline,pipeline_control_types::{ALL_PIPELINE_STAGES,PipelinePrecision},pipeline_context_types::PipelineContext};
use serde::Deserialize;

#[derive(Deserialize)]struct Fixture{high_stages:Vec<u8>,low_stages:Vec<u8>,cases:Vec<Case>,#[serde(default)]terminal:bool}
#[derive(Deserialize)]struct Case{input:Input,result:(bool,usize,Vec<Capture>,Vec<u8>)}
type Capture=(Vec<Vec<u32>>,[usize;3],Vec<u8>);
#[derive(Deserialize)]struct Input{
 ids:Vec<u8>,force:bool,high:[[u32;8];8],low:[[u16;16];8],uniform:[u32;4],uniform_bytes:[u16;4],coverage:u32,
 destination:Vec<u8>,real_width:usize,mask:Vec<u8>,mask_width:u32,aa_pixels:[u8;2],aa_stride:u32,aa_shift:usize,origin:[u32;2],size:[u32;2],
}
fn fixture()->Fixture{serde_json::from_str(include_str!("fixtures/resource-stages.json")).unwrap()}
fn context<'p,'d,'m>(i:&'m Input,d:&'d mut[u8],p:&'p mut PipelineContext)->ResourceContext<'p,'d,'m>{ResourceContext{pipeline:p,
 destination:DestinationSurface{data:d,real_width:i.real_width},mask:MaskSurface{data:&i.mask,real_width:i.mask_width},
 aa_mask:AaMask{pixels:i.aa_pixels,stride:i.aa_stride,shift:i.aa_shift},
}}
fn pipeline(i:&Input)->PipelineContext{PipelineContext{uniform_color:UniformColor{components:i.uniform.map(f32::from_bits),rgba:i.uniform_bytes},current_coverage:f32::from_bits(i.coverage),..PipelineContext::default()}}
fn capture(h:&HighRegisters,l:&LowRegisters,high:bool,c:ResourceCursor,ctx:&ResourceContext<'_,'_,'_>)->Capture{
 (if high{h.channels.iter().map(|v|v.map(f32::to_bits).to_vec()).collect()}else{l.channels.iter().map(|v|v.map(u32::from).to_vec()).collect()},[c.dx,c.dy,c.tail],ctx.destination.data.to_vec())
}
fn context_bits(ctx:&ResourceContext<'_,'_,'_>)->Vec<u64>{let mut v=ctx.pipeline.uniform_color.components.map(|x|x.to_bits()as u64).to_vec();v.extend(ctx.pipeline.uniform_color.rgba.map(u64::from));v.extend([ctx.pipeline.current_coverage.to_bits()as u64,ctx.destination.real_width as u64,ctx.mask.real_width as u64,ctx.aa_mask.pixels[0]as u64,ctx.aa_mask.pixels[1]as u64,ctx.aa_mask.stride as u64,ctx.aa_mask.shift as u64]);v.extend(ctx.mask.data.iter().map(|x|*x as u64));v}
fn cases(high:bool,f:Fixture){let mut count=0;let mut assertions=0;
 for(case_id,c)in f.cases.iter().enumerate().filter(|(_,c)|c.result.0==high){count+=1;let i=&c.input;let stages:Vec<_>=i.ids.iter().map(|id|ALL_PIPELINE_STAGES[*id as usize]).collect();let plan=compile_pipeline(&stages,i.force,());assert_eq!(plan.precision,if high{PipelinePrecision::High}else{PipelinePrecision::Low});assert_eq!(plan.width,c.result.1);assertions+=2;
  let mut data=i.destination.clone();let mut p=pipeline(i);let mut ctx=context(i,&mut data,&mut p);let before=context_bits(&ctx);let mut observed=Vec::new();
  for dy in i.origin[1]as usize..(i.origin[1]+i.size[1])as usize{let mut dx=i.origin[0]as usize;let end=dx+i.size[0]as usize;while dx<end{
   let tail=(end-dx).min(plan.width);let cursor=ResourceCursor{dx,dy,tail,program:if tail==plan.width{ResourceProgram::Full}else{ResourceProgram::Tail}};
   let mut h=HighRegisters{channels:i.high.map(|v|v.map(f32::from_bits))};let mut l=LowRegisters{channels:i.low};observed.push(capture(&h,&l,high,cursor,&ctx));
   for stage in stages.iter().copied(){let action=if (stage as u8)<=1{if high{sample_high_stage(stage,&mut h,ctx.pipeline,[dx,dy]).unwrap();}else{sample_low_stage(stage,&mut l,ctx.pipeline,[dx,dy]).unwrap();}StageControl::Continue}else if high{resource_high_stage(stage,&mut h,&mut ctx,cursor).unwrap()}else{resource_low_stage(stage,&mut l,&mut ctx,cursor).unwrap()};
    if action==StageControl::Stop{break;}observed.push(capture(&h,&l,high,cursor,&ctx));
   }if f.terminal{observed.push(capture(&h,&l,high,cursor,&ctx));}dx+=tail;
  }}
  assert_eq!(observed.len(),c.result.2.len(),"case {case_id} continuation count");assertions+=1;
  for(step,(a,e))in observed.iter().zip(&c.result.2).enumerate(){assert_eq!(a.1,e.1,"case {case_id} capture {step} cursor");assertions+=1;
   for channel in 0..8{for lane in 0..plan.width{assert_eq!(a.0[channel][lane],e.0[channel][lane],"case {case_id} capture {step} channel {channel} lane {lane}");assertions+=1;}}
   assert_eq!(a.2.len(),e.2.len());assertions+=1;for(offset,(actual,expected))in a.2.iter().zip(&e.2).enumerate(){assert_eq!(actual,expected,"case {case_id} capture {step} destination offset {offset}");assertions+=1;}
  }
  assert_eq!(ctx.destination.data,c.result.3,"case {case_id} final destination");assert_eq!(context_bits(&ctx),before,"case {case_id} immutable borrowed context");assertions+=2;
 }
 println!("{count} original Source resource cases / {assertions} assertions");
}
#[test]fn actual_source_high_resource_registers_and_destination(){cases(true,fixture());}
#[test]fn actual_source_low_resource_registers_and_destination(){cases(false,fixture());}
#[test]fn actual_source_high_mask_terminal_state(){cases(true,serde_json::from_str(include_str!("fixtures/resource-terminal.json")).unwrap());}
#[test]fn actual_source_low_mask_terminal_state(){cases(false,serde_json::from_str(include_str!("fixtures/resource-terminal.json")).unwrap());}
#[test]fn unbound_resource_stages_return_error_without_mutation(){let f=fixture();let i=&f.cases[0].input;let mut count=0;
 for(id,stage)in ALL_PIPELINE_STAGES.iter().copied().enumerate(){for high in [false,true]{if (if high{&f.high_stages}else{&f.low_stages}).contains(&(id as u8)){continue;}
  let mut data=i.destination.clone();let mut p=pipeline(i);let mut ctx=context(i,&mut data,&mut p);let before=context_bits(&ctx);let mut h=HighRegisters{channels:i.high.map(|v|v.map(f32::from_bits))};let mut l=LowRegisters{channels:i.low};let h_before=h.channels.map(|v|v.map(f32::to_bits));let l_before=l;
  let cursor=ResourceCursor{dx:3,dy:1,tail:1,program:ResourceProgram::Tail};let result=if high{resource_high_stage(stage,&mut h,&mut ctx,cursor)}else{resource_low_stage(stage,&mut l,&mut ctx,cursor)};
  assert_eq!(result,Err(ResourceStageError::Unsupported{precision:if high{PipelinePrecision::High}else{PipelinePrecision::Low},stage}));assert_eq!(h.channels.map(|v|v.map(f32::to_bits)),h_before);assert_eq!(l,l_before);assert_eq!(ctx.destination.data,i.destination);assert_eq!(context_bits(&ctx),before);count+=5;
 }}println!("{count} unsupported resource assertions");}
