// Root source expectations come from complete unchanged upstream selectors.
use mermaid_rs_renderer::scene::compile::c4_style::*;
mod config {pub use mermaid_rs_renderer::config::*;}
mod ir {pub use mermaid_rs_renderer::ir::*;}
include!("fixtures/c4-style-stimuli.rs");
#[test]
fn complete_c4_style_source_parity(){
    let fixture:serde_json::Value=serde_json::from_str(include_str!("fixtures/c4-style.json")).unwrap();
    let cases=fixture["cases"].as_array().unwrap();assert_eq!(cases.len(),2380);
    for (index,case) in cases.iter().enumerate(){
        let mut conf=stimulus(&case["input"]);let kind=kind(case["input"]["kind"].as_str().unwrap());let expected=&case["expected"];
        let(bg,border)=c4_shape_colors(&conf,kind);let family=c4_shape_font_family(&conf,kind);let weight=c4_shape_font_weight(&conf,kind);
        assert_eq!(bg,expected["colors"][0].as_str().unwrap(),"case {index} background");
        assert_eq!(border,expected["colors"][1].as_str().unwrap(),"case {index} border");
        assert_eq!(family,expected["family"].as_str().unwrap(),"case {index} family");
        assert_eq!(weight,expected["weight"].as_str().unwrap(),"case {index} weight");
        assert_eq!(c4_shape_font_size(&conf,kind).to_bits(),expected["size_bits"].as_u64().unwrap() as u32,"case {index} original f32 bits");
        if let Some(bindings)=expected["bindings"].as_array(){for (value,key) in [bg,border,family,weight].iter().zip(bindings){let field=string_field(&conf,key.as_str().unwrap());assert_eq!(value.as_ptr(),field.as_ptr(),"case {index} borrowed config identity");assert_eq!(value.len(),field.len());}}
        // Source selectors read current configuration each time, including
        // updates which produce a nonfinite numeric payload.
        conf.person_font_family="later family".into();conf.person_font_size=f32::from_bits(0x7fc00071);
        assert_eq!(c4_shape_font_family(&conf,ir::C4ShapeKind::Person),"later family");
        assert_eq!(c4_shape_font_size(&conf,ir::C4ShapeKind::Person).to_bits(),0x7fc00071);
    }
}
