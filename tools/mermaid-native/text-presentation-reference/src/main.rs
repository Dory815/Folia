// Actual unchanged helpers only. No native text or scene implementation imported.
use mermaid_pinned as source;
include!(concat!(env!("CARGO_MANIFEST_DIR"),"/../../../third_party/mermaid-rs-renderer/tests/support/text_presentation_input.rs"));
include!(env!("FOLIA_TEXT_PRESENTATION_REFERENCE"));
use serde_json::json;
fn number(s:&str)->Result<String,Box<dyn std::error::Error>> {let v:f64=s.parse()?;if !v.is_finite(){return Err("nonfinite source emission".into());}Ok(v.to_string())}
fn observe(n:roxmltree::Node<'_,'_>)->Result<Value,Box<dyn std::error::Error>> {
    let mut attrs=serde_json::Map::new();
    for a in n.attributes(){let value=if matches!(a.name(),"x"|"y"|"dy"|"font-size"){number(a.value())?}else{a.value().to_owned()};attrs.insert(a.name().to_owned(),json!(value));}
    if let Some(style)=n.attribute("style"){
        let style=style.strip_prefix("text-anchor: middle; font-size: ").ok_or("unexpected source style prefix")?;
        let (size,suffix)=style.split_once("px; ").ok_or("source style size")?;
        attrs.remove("style");attrs.insert("style_size_px".into(),json!(number(size)?));attrs.insert("style_suffix".into(),json!(suffix));
    }
    let children=n.children().filter(|c|c.is_element()).map(observe).collect::<Result<Vec<_>,_>>()?;
    let text=n.children().filter(|c|c.is_text()).map(|c|c.text().unwrap_or("")).collect::<String>();
    Ok(json!({"tag":n.tag_name().name(),"attrs":attrs,"text":text,"children":children}))
}
fn main()->Result<(),Box<dyn std::error::Error>> {
    let args:Vec<_>=std::env::args().collect();let rows:Value=serde_json::from_slice(&std::fs::read(&args[1])?)?;let mut out=Vec::new();
    for row in rows.as_array().unwrap(){let i=input(row);let svg=match row["method"].as_str().unwrap(){
        "git"=>render_gitgraph_multiline_text(i.x,i.y,&i.text,&i.family,i.size,i.lh,&i.fill),
        "block"=>text_block_svg(i.x,i.y,&i.label,&i.theme,&i.config,i.edge,i.override_color.as_deref()),
        "anchor"=>text_block_svg_anchor(i.x,i.y,&i.label,&i.theme,&i.config,&i.anchor,i.override_color.as_deref()),
        "size"=>text_block_svg_with_font_size(i.x,i.y,&i.label,&i.theme,&i.config,i.size,&i.anchor,i.override_color.as_deref(),i.baseline),
        "weight"=>text_block_svg_with_font_size_weight(i.x,i.y,&i.label,&i.theme,&i.config,i.size,&i.anchor,i.override_color.as_deref(),i.weight.as_deref(),i.baseline),
        "line_size"=>text_line_svg_with_font_size(i.x,i.y,&i.text,&i.theme,i.size,&i.fill,&i.anchor),
        "line"=>text_line_svg(i.x,i.y,&i.text,&i.theme,&i.fill,&i.anchor),
        "c4"=>c4_text_svg(i.x,i.y,&i.lines,&i.family,i.size,i.weight.as_deref().unwrap_or(""),&i.fill,i.italic),
        _=>panic!("root input method")};
        let wrapped=format!("<svg>{svg}</svg>");let observed=(||->Result<Vec<Value>,Box<dyn std::error::Error>>{let doc=roxmltree::Document::parse(&wrapped)?;doc.root_element().children().filter(|n|n.is_element()).map(observe).collect()})();
        let families=normalize_font_family(&i.family);let dividers:Vec<_>=i.lines.iter().map(|s|is_divider_line(s)).collect();
        match observed {Ok(elements)=>out.push(json!({"input":row,"svg":svg,"valid":true,"elements":elements,"family":families,"dividers":dividers})),Err(e)=>out.push(json!({"input":row,"svg":svg,"valid":false,"diagnostic":e.to_string(),"family":families,"dividers":dividers}))};
    }
    std::fs::write(&args[2],serde_json::to_vec_pretty(&out)?)?;Ok(())
}
