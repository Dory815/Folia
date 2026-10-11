// Root transport: invokes actual original Context::default and reads its fields.
fn main(){println!("{}",serde_json::to_string(&tiny_skia::folia_context_default()).unwrap());}
