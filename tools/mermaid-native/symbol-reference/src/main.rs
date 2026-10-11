// Root-only oracle executes the two unchanged private source helpers.
pub use mermaid_pinned::ir;
include!(env!("FOLIA_MERMAID_SYMBOL_REFERENCE"));
use serde_json::{Value, json};
use std::str::FromStr;
fn number(v: f64) -> String { v.to_string() }
fn observe(node: roxmltree::Node<'_, '_>) -> Result<Value, Box<dyn std::error::Error>> {
    let tag=node.tag_name().name();
    if tag=="g" {
        let transform_text=node.attribute("transform").unwrap_or("");
        let transform=svgtypes::Transform::from_str(transform_text)?;
        let translation=match svgtypes::TransformListParser::from(transform_text).next().transpose()?{
            Some(svgtypes::TransformListToken::Translate{tx,ty})=>[number(tx),number(ty)],
            None=>[number(0.0),number(0.0)],
            _=>return Err("unexpected first source transform".into()),
        };
        let children=node.children().filter(|c|c.is_element()).map(observe).collect::<Result<Vec<_>,_>>()?;
        return Ok(json!({"kind":"group","matrix":([transform.a,transform.b,transform.c,transform.d,transform.e,transform.f].map(number)),"translation":translation,"children":children}));
    }
    let geometry=match tag {
        "circle"=>json!({"kind":"circle","values":(["cx","cy","r"].map(|a|node.attribute(a).unwrap_or("0")))}),
        "polygon"=>{
            let points=svgtypes::PointsParser::from(node.attribute("points").ok_or("missing points")?).map(|(x,y)|json!([number(x),number(y)])).collect::<Vec<_>>();
            json!({"kind":"polygon","points":points})
        },
        "path"=>{
            let mut segments=Vec::new();
            for segment in svgtypes::PathParser::from(node.attribute("d").ok_or("missing d")?) {
                segments.push(match segment?{
                    svgtypes::PathSegment::MoveTo{abs:true,x,y}=>json!({"command":"M","coordinates":[number(x),number(y)]}),
                    svgtypes::PathSegment::LineTo{abs:true,x,y}=>json!({"command":"L","coordinates":[number(x),number(y)]}),
                    _=>return Err("unexpected source symbol command".into()),
                });
            }
            json!({"kind":"path","segments":segments})
        },
        _=>return Err("unexpected source symbol element".into()),
    };
    Ok(json!({"kind":"shape","geometry":geometry,"fill":node.attribute("fill").unwrap_or("black"),"stroke":node.attribute("stroke").ok_or("missing stroke")?,"width":node.attribute("stroke-width").unwrap_or("1"),"cap":node.attribute("stroke-linecap").unwrap_or("butt"),"join":node.attribute("stroke-linejoin").unwrap_or("miter"),"miter_limit":"4"}))
}
fn value(v:&Value)->Result<f32,Box<dyn std::error::Error>>{Ok(v.as_str().ok_or("input scalar string")?.parse()?)}
fn main()->Result<(),Box<dyn std::error::Error>> {
    let args:Vec<_>=std::env::args().collect();let inputs:Value=serde_json::from_slice(&std::fs::read(&args[1])?)?;let mut output=Vec::new();
    for row in inputs.as_array().ok_or("input array")?{
        let point=(value(&row["point"][0])?,value(&row["point"][1])?);let angle=value(&row["angle"])?;let width=value(&row["width"])?;
        let svg=if row["kind"]=="arrowhead"{arrowhead_svg(point,angle,"#a14ec5",width)}else{
            let decoration=match row["kind"].as_str().ok_or("kind")?{
                "Circle"=>ir::EdgeDecoration::Circle,"Cross"=>ir::EdgeDecoration::Cross,"Diamond"=>ir::EdgeDecoration::Diamond,"DiamondFilled"=>ir::EdgeDecoration::DiamondFilled,
                "CrowsFootOne"=>ir::EdgeDecoration::CrowsFootOne,"CrowsFootZeroOne"=>ir::EdgeDecoration::CrowsFootZeroOne,"CrowsFootMany"=>ir::EdgeDecoration::CrowsFootMany,"CrowsFootZeroMany"=>ir::EdgeDecoration::CrowsFootZeroMany,_=>return Err("invalid kind".into()),
            };edge_decoration_svg(point,angle,decoration,"#a14ec5",width,row["at_start"].as_bool().ok_or("endpoint")?)
        };
        let document=roxmltree::Document::parse(&svg)?;let observation=observe(document.root_element())?;
        output.push(json!({"input":row,"svg":svg,"observation":observation}));
    }
    std::fs::write(&args[2],serde_json::to_vec_pretty(&output)?)?;Ok(())
}
