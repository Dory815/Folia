// Root executes byte-extracted, unchanged private upstream path functions.
include!(env!("FOLIA_MERMAID_PATH_REFERENCE"));
use serde_json::{Value,json};
use std::fs;
fn main()->Result<(),Box<dyn std::error::Error>> {
    let args:Vec<_>=std::env::args().collect();
    let inputs:Value=serde_json::from_str(&fs::read_to_string(&args[1])?)?;
    let mut results=Vec::new();
    for row in inputs.as_array().ok_or("input array")? {
        let points:Vec<(f32,f32)>=row["points"].as_array().ok_or("points")?.iter()
            .map(|v|Ok((v[0].as_f64().ok_or("x")? as f32,v[1].as_f64().ok_or("y")? as f32)))
            .collect::<Result<_,Box<dyn std::error::Error>>>()?;
        let radius=row["radius"].as_f64().ok_or("radius")? as f32;
        results.push(json!({"points":points,"radius":radius,"deduped":dedupe_points(&points),
            "plain":points_to_path(&points),"rounded":rounded_polyline_path(&points,radius),"basis":basis_curve_path(&points)}));
    }
    fs::write(&args[2],serde_json::to_vec_pretty(&results)?)?;Ok(())
}
