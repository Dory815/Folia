// Root invokes three unchanged Source functions, including accepted divider.
use serde_json::{Value,json};
include!(env!("FOLIA_LABEL_DATA_SOURCE"));
fn main()->Result<(),Box<dyn std::error::Error>> {
    let args:Vec<_>=std::env::args().collect();
    let inputs:Vec<Value>=serde_json::from_slice(&std::fs::read(&args[1])?)?;
    let mut output=Vec::new();
    for input in inputs {
        let expected=if input["kind"]=="sankey" {
            json!(format_sankey_value(f32::from_bits(input["bits"].as_u64().unwrap() as u32)))
        } else {
            let lines:Vec<String>=serde_json::from_value(input["lines"].clone())?;
            let (title,attributes)=parse_er_attributes(&lines);
            json!({"title":title,"attributes":attributes.iter().map(|a|json!({"name":a.name,"data_type":a.data_type,"keys":a.keys})).collect::<Vec<_>>()})
        };
        output.push(json!({"input":input,"expected":expected}));
    }
    std::fs::write(&args[2],serde_json::to_vec(&output)?)?;Ok(())
}
