// Root expected fields are observed from actual original Context::default.
use mermaid_rs_renderer::scene::{paint::SpreadMode,prepare::pipeline_context_types::PipelineContext};
#[test]fn actual_source_persistent_context_defaults(){let c=PipelineContext::default();let groups=vec![
 vec![c.current_coverage.to_bits()],vec![match c.sampler.spread_mode{SpreadMode::Pad=>0,SpreadMode::Reflect=>1,SpreadMode::Repeat=>2},c.sampler.inv_width.to_bits(),c.sampler.inv_height.to_bits()],
 c.uniform_color.components.map(f32::to_bits).to_vec(),c.uniform_color.rgba.map(u32::from).to_vec(),c.two_stop.factor.components.map(f32::to_bits).to_vec(),c.two_stop.bias.components.map(f32::to_bits).to_vec(),
 vec![c.gradient.len as u32,c.gradient.factors.len()as u32,c.gradient.biases.len()as u32,c.gradient.t_values.len()as u32],c.gradient.factors.iter().flat_map(|v|v.components.map(f32::to_bits)).collect(),c.gradient.biases.iter().flat_map(|v|v.components.map(f32::to_bits)).collect(),c.gradient.t_values.iter().map(|v|v.get().to_bits()).collect(),
 c.radial_p.map(f32::to_bits).to_vec(),c.radial_mask.to_vec(),[c.limit_x.scale,c.limit_x.inv_scale].map(f32::to_bits).to_vec(),[c.limit_y.scale,c.limit_y.inv_scale].map(f32::to_bits).to_vec(),[c.transform.sx,c.transform.ky,c.transform.kx,c.transform.sy,c.transform.tx,c.transform.ty].map(f32::to_bits).to_vec()];
 let fixture:serde_json::Value=serde_json::from_str(include_str!("fixtures/context-default.json")).unwrap();let expected:Vec<Vec<u32>>=serde_json::from_value(fixture["groups"].clone()).unwrap();assert_eq!(groups.len(),15);for(i,(a,e))in groups.iter().zip(expected).enumerate(){assert_eq!(*a,e,"Source default context group {i}");}println!("15 actual Source default groups / 16 assertions");
}
