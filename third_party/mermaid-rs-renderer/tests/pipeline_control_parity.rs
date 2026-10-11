// Root observations from actual original compiler pointers and context, not native golds.
use mermaid_rs_renderer::scene::prepare::{pipeline_control::*,pipeline_control_types::*};
use serde::Deserialize;
#[derive(Deserialize)]
struct Input{stages:Vec<u8>,force:bool,marker:u32}
type ResultProjection=(bool,usize,Vec<(u8,bool)>,Vec<(u8,bool)>,[u32;13]);
#[derive(Deserialize)]
struct Case{input:Input,result:Option<ResultProjection>}
#[derive(Deserialize)]
struct Fixture{names:Vec<String>,cases:Vec<Case>}
#[derive(Debug,PartialEq)]
struct ContextProjection([u32;13]);
impl Default for ContextProjection{
 fn default()->Self{let t=tiny_skia_path::Transform::default();Self([0,t.sx.to_bits(),t.ky.to_bits(),t.kx.to_bits(),t.sy.to_bits(),t.tx.to_bits(),t.ty.to_bits(),0,0,0,0,0,0])}
}
fn supplied_context(m:u32)->ContextProjection{ContextProjection([m,2f32.to_bits(),3f32.to_bits(),4f32.to_bits(),5f32.to_bits(),6f32.to_bits(),7f32.to_bits(),m,7,m,m,m,m])}
fn operations(v:&[PipelineOperation])->Vec<(u8,bool)>{v.iter().map(|o|match o{PipelineOperation::Stage(s)=>(*s as u8,false),PipelineOperation::Tail(s)=>(*s as u8,true),PipelineOperation::Return=>(79,false)}).collect()}
fn fixture()->Fixture{serde_json::from_str(include_str!("fixtures/pipeline-control.json")).unwrap()}
#[test]
fn complete_original_compiler_control_projection(){
 let f=fixture();assert_eq!(f.cases.len(),12872);assert_eq!(f.names.len(),79);let mut assertions=2;let mut panics=0;
 for (i,c) in f.cases.iter().enumerate(){
  let stages:Vec<_>=c.input.stages.iter().map(|s|ALL_PIPELINE_STAGES[*s as usize]).collect();
  let result=std::panic::catch_unwind(||compile_pipeline(&stages,c.input.force,supplied_context(c.input.marker)));
  match &c.result{
   None=>{assert!(result.is_err(),"case {i}: actual Source capacity panic required");assertions+=1;panics+=1;},
   Some((high,width,full,tail,context))=>{let result=result.unwrap_or_else(|_|panic!("case {i}: unexpected native panic"));
    assert_eq!(result.precision==PipelinePrecision::High,*high,"case {i}: precision");
    assert_eq!(result.width,*width,"case {i}: stage width");
    assert_eq!(operations(&result.full),*full,"case {i}: full function projection");
    assert_eq!(operations(&result.tail),*tail,"case {i}: tail function projection");
    assert_eq!(result.context.0,*context,"case {i}: default/moved raw context");assertions+=5;
   }
  }
 }
 assert_eq!(panics,12);assertions+=1;println!("12872 Source cases, {assertions} assertions");
}
#[test]
fn original_lowp_function_table_and_stage_identity(){
 let f=fixture();let mut assertions=0;
 for (id,name) in f.names.iter().enumerate(){let s=ALL_PIPELINE_STAGES[id];assert_eq!(s as usize,id);assert_eq!(format!("{s:?}"),*name);assertions+=2;
  let c=f.cases.iter().find(|c|!c.input.force&&c.input.stages==[id as u8]).unwrap();
  assert_eq!(pipeline_stage_is_lowp(s),!c.result.as_ref().unwrap().0,"Source function-pointer eligibility for {name}");assertions+=1;
 }println!("79 Source stages, {assertions} assertions");
}
