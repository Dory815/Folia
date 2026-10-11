// Root native observations against immutable actual-original policy records.
use mermaid_rs_renderer::scene::prepare::{blend_policy_types::ALL_RASTER_BLEND_MODES,gradient_program_types::ColorSpace,gradient_stop_types::StopColor};
use serde::Deserialize;use std::panic::AssertUnwindSafe;
#[derive(Debug,Deserialize)]struct Input{blend:u8,color_space:u8,color:[u32;4]}
type Record=(String,bool,Option<String>,Option<String>,Option<String>,Option<([u32;4],[u8;4])>);
#[derive(Deserialize)]struct Case{input:Input,result:Record}
#[derive(Deserialize)]struct Fixture{cases:Vec<Case>}
fn count_stub(payload:&(dyn std::any::Any+Send),stubs:&mut usize){let text=payload.downcast_ref::<&str>().copied().or_else(||payload.downcast_ref::<String>().map(String::as_str)).unwrap_or("");if text.contains("Unimplemented Root-frozen"){*stubs+=1;}}
fn observe(i:&Input)->Record{
 let blend=ALL_RASTER_BLEND_MODES[i.blend as usize];let cs=match i.color_space{0=>ColorSpace::Linear,1=>ColorSpace::Gamma2,2=>ColorSpace::SimpleSRGB,_=>ColorSpace::FullSRGBGamma};let c=i.color.map(f32::from_bits);let color=StopColor::from_rgba(c[0],c[1],c[2],c[3]).map(|c|{let c=c.premultiply();(c.components().map(f32::to_bits),c.to_color_u8())});
 (format!("{blend:?}"),blend.should_pre_scale_coverage(),blend.to_stage().map(|s|format!("{s:?}")),cs.expand_dest_stage().map(|s|format!("{s:?}")),cs.compress_stage().map(|s|format!("{s:?}")),color)
}
#[test]fn actual_original_blitter_policy_records(){let hook=std::panic::take_hook();std::panic::set_hook(Box::new(|_|{}));let f:Fixture=serde_json::from_str(include_str!("fixtures/blitter-policy.json")).unwrap();assert_eq!(f.cases.len(),3445);let(mut mismatches,mut stubs,mut errors)=(0,0,0);let mut first=vec![];
 for(index,c)in f.cases.iter().enumerate(){let result=std::panic::catch_unwind(||observe(&c.input));if let Err(e)=&result{count_stub(&**e,&mut stubs);errors+=1;}let actual=result.ok();if actual.as_ref()!=Some(&c.result){mismatches+=1;if first.len()<3{first.push(format!("case {index} input={:?} actual={actual:?} Source={:?}",c.input,c.result));}}}
 std::panic::set_hook(hook);println!("3445 actual original blitter policy records / {mismatches} mismatches / {stubs} actual stub calls / {errors} errors");assert_eq!(mismatches,0,"{}",first.join("\n"));assert_eq!(stubs,0);assert_eq!(errors,0);
}
#[test]fn all_five_policy_dependency_methods_are_implemented(){let color=StopColor::from_rgba(0.2,0.4,0.7,0.5).unwrap().premultiply();let blend=ALL_RASTER_BLEND_MODES[3];let mut calls:Vec<Box<dyn FnMut()>>=vec![Box::new(||{let _=blend.should_pre_scale_coverage();}),Box::new(||{let _=blend.to_stage();}),Box::new(||{let _=ColorSpace::Gamma2.expand_dest_stage();}),Box::new(||{let _=ColorSpace::Gamma2.compress_stage();}),Box::new(||{let _=color.to_color_u8();})];let hook=std::panic::take_hook();std::panic::set_hook(Box::new(|_|{}));let mut stubs=0;for call in &mut calls{if let Err(e)=std::panic::catch_unwind(AssertUnwindSafe(call)){count_stub(&*e,&mut stubs);}}std::panic::set_hook(hook);println!("Five direct policy dependency witnesses / {stubs} actual stub calls");assert_eq!(stubs,0);}
