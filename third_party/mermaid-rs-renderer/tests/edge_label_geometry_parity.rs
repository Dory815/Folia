use mermaid_rs_renderer::scene::compile::edge_label_geometry::*;
pub use mermaid_rs_renderer::ir;
include!("support/edge_label_geometry_input.rs");

#[test]
fn complete_source_edge_label_geometry_matches_bits_and_branches() {
    let fixture:serde_json::Value=serde_json::from_str(include_str!("fixtures/edge-label-geometry.json")).unwrap();
    assert_eq!(fixture["source_sha256"],"026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4");
    assert_eq!(fixture["functions"].as_array().unwrap().len(),11);
    for (i,row) in fixture["cases"].as_array().unwrap().iter().enumerate() {
        assert_eq!(observe_geometry(&row["input"]),*row,"source observation {i}: {}",row["input"]["method"]);
    }
}
