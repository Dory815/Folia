// Root oracle: linked only against the unmodified, archive-verified baseline.
use mermaid_pinned::{LayoutConfig, Theme, compute_layout, parse_mermaid_strict};
use mermaid_pinned::config::{PieRenderMode, TreemapRenderMode};
use mermaid_pinned::render::{measure_svg_dimensions, render_svg_with_dimensions};
use serde_json::{Value, json};
use std::{fs, path::PathBuf};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<_> = std::env::args().collect();
    let inputs: Value = serde_json::from_str(&fs::read_to_string(&args[1])?)?;
    let reference = PathBuf::from(&args[2]);
    let mut result = Vec::new();
    for item in inputs.as_array().ok_or("input manifest must be an array")? {
        let name = item["name"].as_str().ok_or("missing name")?;
        let source = fs::read_to_string(reference.join(item["path"].as_str().ok_or("missing path")?))?;
        for theme_name in ["modern", "dark"] {
            let theme = if theme_name == "dark" {Theme::dark()} else {Theme::modern()};
            for variant in ["default", "ratio", "fast-text", "error-mode", "fixed-size"] {
                let mut config = LayoutConfig::default();
                if variant == "ratio" {config.preferred_aspect_ratio = Some(1.6);}
                if variant == "fast-text" {config.fast_text_metrics = true;}
                if variant == "error-mode" {
                    config.pie.render_mode = PieRenderMode::Error;
                    config.treemap.render_mode = TreemapRenderMode::Error;
                }
                let dimensions = if variant == "fixed-size" {Some((640.0, 360.0))} else {None};
                let parsed = match parse_mermaid_strict(&source) {
                    Ok(value) => value,
                    Err(error) => {result.push(json!({"name":name,"theme":theme_name,"variant":variant,"error":format!("{error:?}")}));continue;}
                };
                let layout = compute_layout(&parsed.graph, &theme, &config);
                let size = measure_svg_dimensions(&layout, &config, dimensions);
                let svg = render_svg_with_dimensions(&layout, &theme, &config, dimensions);
                result.push(json!({"name":name,"theme":theme_name,"variant":variant,
                    "kind":format!("{:?}", layout.kind),"dimensions":[size.width,size.height,size.viewbox_x,size.viewbox_y,size.viewbox_width,size.viewbox_height],
                    "svg":svg,"init_config":parsed.init_config}));
            }
        }
    }
    fs::write(&args[3], serde_json::to_vec_pretty(&result)?)?;
    println!("{} pinned-reference observations", result.len());
    Ok(())
}
