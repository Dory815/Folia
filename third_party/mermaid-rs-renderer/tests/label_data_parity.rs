// Root independent assertions over unchanged Source observations.
use mermaid_rs_renderer::scene::compile::{er_attributes::parse_er_attributes,sankey_labels::format_sankey_value};
use serde_json::{Value,json};
#[test]
fn complete_source_label_data() {
    let fixture:Value=serde_json::from_str(include_str!("fixtures/label-data.json")).unwrap();
    let cases=fixture["cases"].as_array().unwrap();
    assert!(cases.len()>2500);
    let mut assertions=1;
    for (index,case) in cases.iter().enumerate() {
        let input=&case["input"];
        if input["kind"]=="sankey" {
            let native=format_sankey_value(f32::from_bits(input["bits"].as_u64().unwrap() as u32));
            assert_eq!(native,case["expected"].as_str().unwrap(),"case {index}: {input}");
            assertions+=1;
        } else {
            let lines:Vec<String>=serde_json::from_value(input["lines"].clone()).unwrap();
            let (title,attributes)=parse_er_attributes(&lines);
            assert_eq!(title,case["expected"]["title"].as_str().unwrap(),"case {index}: title");
            let expected=case["expected"]["attributes"].as_array().unwrap();
            assert_eq!(attributes.len(),expected.len(),"case {index}: attribute count");
            assertions+=2;
            for (a,e) in attributes.iter().zip(expected) {
                assert_eq!(a.name,e["name"].as_str().unwrap(),"case {index}: name");
                assert_eq!(a.data_type,e["data_type"].as_str().unwrap(),"case {index}: type");
                assert_eq!(json!(a.keys),e["keys"],"case {index}: key order/duplicates");
                assertions+=3;
            }
        }
    }
    println!("{} source cases, {assertions} successful assertions",cases.len());
}
