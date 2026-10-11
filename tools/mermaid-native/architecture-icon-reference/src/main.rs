// Root oracle executes the unchanged pinned function; no native scene code.
use serde_json::{Value,json};
include!(env!("FOLIA_ARCHITECTURE_ICON_REFERENCE"));
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
            let mut v = numbers(node, &["x", "y", "width", "height", "rx", "ry"])?;
            if node.attribute("ry").is_none(){v[5]=v[4].clone();}
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
    let paint = node.attribute("stroke").ok_or("missing stroke")?;
    let stroke = if paint == "none" { json!(null) } else { json!({"paint":paint,"width":decimal(width)?,"cap":node.attribute("stroke-linecap").unwrap_or("butt"),"join":node.attribute("stroke-linejoin").unwrap_or("miter"),"dash":dash}) };
    Ok(json!({"kind":"shape","geometry":geometry,"fill":if tag=="line"{"none"}else{node.attribute("fill").unwrap_or("black")},"stroke":stroke,"stroke_opacity":decimal(stroke_opacity)?}))
}
fn text_request(svg:&str)->Result<Value,Box<dyn std::error::Error>> {
    // Recover the actual raw source text attributes, including invalid XML fill.
    let rest=svg.strip_prefix("<text x=\"").ok_or("source text prefix")?;
    let (x,rest)=rest.split_once("\" y=\"").ok_or("source text x")?;
    let (y,rest)=rest.split_once("\" text-anchor=\"").ok_or("source text y")?;
    let (anchor,rest)=rest.split_once("\" dominant-baseline=\"").ok_or("source text anchor")?;
    let (baseline,rest)=rest.split_once("\" fill=\"").ok_or("source text baseline")?;
    let (fill,rest)=rest.rsplit_once("\" font-size=\"").ok_or("source text fill")?;
    let (size,rest)=rest.split_once("\" font-family=\"").ok_or("source text size")?;
    let (family,text)=rest.split_once("\">").ok_or("source text family")?;
    let text=text.strip_suffix("</text>").ok_or("source text suffix")?;
    let size:f64=size.parse()?;if size<0.0{return Err("negative source font size".into());}
    Ok(json!({"kind":"text","x":decimal(x.parse()?)?,"y":decimal(y.parse()?)?,"anchor":anchor,"baseline":baseline,"fill_lexeme":fill,"size":decimal(size)?,"family":family,"text":text}))
}
fn main()->Result<(),Box<dyn std::error::Error>> {
    let args:Vec<_>=std::env::args().collect();let rows:Value=serde_json::from_slice(&std::fs::read(&args[1])?)?;let mut out=Vec::new();
    for row in rows.as_array().unwrap(){let w:f32=row["w"].as_str().unwrap().parse()?;let h:f32=row["h"].as_str().unwrap().parse()?;let svg=architecture_icon_svg(row["icon"].as_str(),w,h,row["fill"].as_str().unwrap());
    let observed=(||->Result<Vec<Value>,Box<dyn std::error::Error>>{if svg.starts_with("<text "){return Ok(vec![text_request(&svg)?]);}let wrapper=format!("<svg>{svg}</svg>");let doc=roxmltree::Document::parse(&wrapper)?;doc.root_element().children().filter(|n|n.is_element()).map(observe).collect()})();
    match observed {Ok(items)=>out.push(json!({"input":row,"svg":svg,"valid":true,"items":items})),Err(e)=>out.push(json!({"input":row,"svg":svg,"valid":false,"diagnostic":e.to_string()}))};}
    std::fs::write(&args[2],serde_json::to_vec(&out)?)?;Ok(())
}
