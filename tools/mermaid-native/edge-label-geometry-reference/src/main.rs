pub use mermaid_pinned::ir;
include!(env!("FOLIA_EDGE_LABEL_GEOMETRY_REFERENCE"));
include!("../../../../third_party/mermaid-rs-renderer/tests/support/edge_label_geometry_input.rs");
fn main() {
    let args: Vec<_> = std::env::args().collect();
    let inputs: serde_json::Value = serde_json::from_slice(&std::fs::read(&args[1]).unwrap()).unwrap();
    let observations: Vec<_> = inputs.as_array().unwrap().iter().map(observe_geometry).collect();
    std::fs::write(&args[2], serde_json::to_vec(&observations).unwrap()).unwrap();
}
