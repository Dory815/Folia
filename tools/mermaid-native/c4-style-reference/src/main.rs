// Root invokes unchanged source selectors; no native scene code is imported.
use serde_json::{Value,json};
mod config {pub use mermaid_pinned::config::*;}
mod ir {pub use mermaid_pinned::ir::*;}
include!(env!("FOLIA_C4_STYLE_SOURCE"));
include!(env!("FOLIA_C4_STYLE_STIMULI"));
fn binding(conf:&config::C4Config,value:&str)->String {
    for key in STRING_FIELDS {
        let field=string_field(conf,key);
        if field.as_ptr()==value.as_ptr()&&field.len()==value.len(){return (*key).to_owned();}
    }
    panic!("source selector did not return a borrowed config field");
}
fn main()->Result<(),Box<dyn std::error::Error>> {
    let args:Vec<_>=std::env::args().collect();
    let inputs:Vec<Value>=serde_json::from_slice(&std::fs::read(&args[1])?)?;
    let mut output=Vec::new();
    for input in inputs {
        let conf=stimulus(&input);let kind=kind(input["kind"].as_str().unwrap());
        let(bg,border)=c4_shape_colors(&conf,kind);
        let family=c4_shape_font_family(&conf,kind);let weight=c4_shape_font_weight(&conf,kind);
        let bindings=if input["preset"]!="raw" {json!([binding(&conf,bg),binding(&conf,border),binding(&conf,family),binding(&conf,weight)])}else{Value::Null};
        output.push(json!({"input":input,"expected":{"colors":[bg,border],"family":family,"weight":weight,"size_bits":c4_shape_font_size(&conf,kind).to_bits(),"bindings":bindings}}));
    }
    std::fs::write(&args[2],serde_json::to_vec(&output)?)?;Ok(())
}
