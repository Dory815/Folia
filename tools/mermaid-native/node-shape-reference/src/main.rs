// Root oracle executes exactly one unchanged private source function.
use mermaid_pinned as source;
pub use mermaid_pinned::{ir, layout};
include!(concat!(
    env!("CARGO_MANIFEST_DIR"),
    "/../../../third_party/mermaid-rs-renderer/tests/support/node_shape_input.rs"
));
include!(env!("FOLIA_NODE_SHAPE_REFERENCE"));
use serde_json::json;
fn decimal(value: f64) -> Result<String, Box<dyn std::error::Error>> {
    if !value.is_finite() {
        return Err("nonfinite source output".into());
    }
    Ok(value.to_string())
}
fn numbers(
    node: roxmltree::Node<'_, '_>,
    keys: &[&str],
) -> Result<Vec<String>, Box<dyn std::error::Error>> {
    keys.iter()
        .map(|k| decimal(node.attribute(*k).unwrap_or("0").parse()?))
        .collect()
}
fn observe(node: roxmltree::Node<'_, '_>) -> Result<Value, Box<dyn std::error::Error>> {
    let tag = node.tag_name().name();
    let geometry = match tag {
        "rect" => {
            let v = numbers(node, &["x", "y", "width", "height", "rx", "ry"])?;
            for n in &v[2..] {
                if n.parse::<f64>()? < 0.0 {
                    return Err("negative source rect size".into());
                }
            }
            json!({"kind":tag,"values":v})
        }
        "circle" => {
            let v = numbers(node, &["cx", "cy", "r"])?;
            if v[2].parse::<f64>()? < 0.0 {
                return Err("negative source radius".into());
            }
            json!({"kind":tag,"values":v})
        }
        "ellipse" => {
            let v = numbers(node, &["cx", "cy", "rx", "ry"])?;
            for n in &v[2..] {
                if n.parse::<f64>()? < 0.0 {
                    return Err("negative source ellipse radius".into());
                }
            }
            json!({"kind":tag,"values":v})
        }
        "line" => json!({"kind":tag,"values":numbers(node,&["x1","y1","x2","y2"])?}),
        "polygon" => {
            let text = node.attribute("points").ok_or("missing polygon")?;
            let mut count = 0;
            for n in svgtypes::NumberListParser::from(text) {
                decimal(n?)?;
                count += 1;
            }
            if count % 2 != 0 {
                return Err("unpaired source polygon coordinate".into());
            }
            let mut points = Vec::new();
            for (x, y) in svgtypes::PointsParser::from(text) {
                points.push(json!([decimal(x)?, decimal(y)?]));
            }
            json!({"kind":tag,"points":points})
        }
        "path" => {
            let mut segments = Vec::new();
            let (mut x, mut y) = (0.0, 0.0);
            let (mut sx, mut sy) = (0.0, 0.0);
            for token in svgtypes::PathParser::from(node.attribute("d").ok_or("missing path")?) {
                use svgtypes::PathSegment::*;
                let command;
                let mut coords = Vec::new();
                match token? {
                    MoveTo { abs, x: a, y: b } => {
                        if abs {
                            x = a;
                            y = b;
                        } else {
                            x += a;
                            y += b;
                        }
                        sx = x;
                        sy = y;
                        command = "M";
                        coords = vec![decimal(x)?, decimal(y)?];
                    }
                    LineTo { abs, x: a, y: b } => {
                        if abs {
                            x = a;
                            y = b;
                        } else {
                            x += a;
                            y += b;
                        }
                        command = "L";
                        coords = vec![decimal(x)?, decimal(y)?];
                    }
                    HorizontalLineTo { abs, x: a } => {
                        if abs {
                            x = a;
                        } else {
                            x += a;
                        }
                        command = "L";
                        coords = vec![decimal(x)?, decimal(y)?];
                    }
                    VerticalLineTo { abs, y: b } => {
                        if abs {
                            y = b;
                        } else {
                            y += b;
                        }
                        command = "L";
                        coords = vec![decimal(x)?, decimal(y)?];
                    }
                    Quadratic {
                        abs,
                        x1,
                        y1,
                        x: a,
                        y: b,
                    } => {
                        let (cx, cy) = if abs { (x1, y1) } else { (x + x1, y + y1) };
                        if abs {
                            x = a;
                            y = b;
                        } else {
                            x += a;
                            y += b;
                        }
                        command = "Q";
                        coords = vec![decimal(cx)?, decimal(cy)?, decimal(x)?, decimal(y)?];
                    }
                    ClosePath { .. } => {
                        x = sx;
                        y = sy;
                        command = "Z";
                    }
                    _ => return Err("unexpected source node path command".into()),
                }
                segments.push(json!({"command":command,"coordinates":coords}));
            }
            json!({"kind":tag,"segments":segments})
        }
        _ => return Err("unexpected source node element".into()),
    };
    let width = node
        .attribute("stroke-width")
        .unwrap_or("1")
        .parse::<f64>()?;
    if width < 0.0 {
        return Err("negative source stroke width".into());
    }
    let mut dash = Vec::new();
    if let Some(d) = node.attribute("stroke-dasharray") {
        for part in d
            .split(|c: char| c == ',' || c.is_ascii_whitespace())
            .filter(|p| !p.is_empty())
        {
            let n = part.parse::<f64>()?;
            if n < 0.0 {
                return Err("negative source dash".into());
            }
            dash.push(decimal(n)?);
        }
    }
    let stroke_opacity = node
        .attribute("stroke-opacity")
        .unwrap_or("1")
        .parse::<f64>()?;
    Ok(
        json!({"geometry":geometry,"fill":if tag=="line"{"none"}else{node.attribute("fill").unwrap_or("black")},"stroke":node.attribute("stroke").ok_or("missing stroke")?,"width":decimal(width)?,"cap":node.attribute("stroke-linecap").unwrap_or("butt"),"join":node.attribute("stroke-linejoin").unwrap_or("miter"),"dash":dash,"stroke_opacity":decimal(stroke_opacity)?}),
    )
}
fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<_> = std::env::args().collect();
    let inputs: Value = serde_json::from_slice(&std::fs::read(&args[1])?)?;
    let mut output = Vec::new();
    for row in inputs.as_array().ok_or("input array")? {
        let (node, theme, config) = input(row);
        let svg = shape_svg(&node, &theme, &config);
        let wrapper = format!("<svg>{svg}</svg>");
        let observed = (|| -> Result<Vec<Value>, Box<dyn std::error::Error>> {
            let doc = roxmltree::Document::parse(&wrapper)?;
            doc.root_element()
                .children()
                .filter(|n| n.is_element())
                .map(observe)
                .collect()
        })();
        match observed {
            Ok(shapes) => output.push(json!({"input":row,"svg":svg,"valid":true,"shapes":shapes})),
            Err(e) => {
                output.push(json!({"input":row,"svg":svg,"valid":false,"diagnostic":e.to_string()}))
            }
        }
    }
    std::fs::write(&args[2], serde_json::to_vec_pretty(&output)?)?;
    Ok(())
}
