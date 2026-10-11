// Root independent DOM observations of actual pinned source emissions.
// XML is used only in this test, never by a native runtime text helper/backend.
use mermaid_rs_renderer as source;
include!("support/text_presentation_input.rs");
use source::scene::{compile::{text_helpers::*,text_presentation::*},SceneError};
use serde_json::json;
fn emit(row:&Value)->Result<Vec<TextPresentation>,SceneError>{let i=input(row);match row["method"].as_str().unwrap(){
"git"=>gitgraph_multiline_text(i.x,i.y,&i.text,&i.family,i.size,i.lh,&i.fill),
"block"=>text_block(i.x,i.y,&i.label,&i.theme,&i.config,i.edge,i.override_color.as_deref()),
"anchor"=>text_block_anchor(i.x,i.y,&i.label,&i.theme,&i.config,&i.anchor,i.override_color.as_deref()),
"size"=>text_block_with_font_size(i.x,i.y,&i.label,&i.theme,&i.config,i.size,&i.anchor,i.override_color.as_deref(),i.baseline),
"weight"=>text_block_with_font_size_weight(i.x,i.y,&i.label,&i.theme,&i.config,i.size,&i.anchor,i.override_color.as_deref(),i.weight.as_deref(),i.baseline),
"line_size"=>text_line_with_font_size(i.x,i.y,&i.text,&i.theme,i.size,&i.fill,&i.anchor),
"line"=>text_line(i.x,i.y,&i.text,&i.theme,&i.fill,&i.anchor),
"c4"=>c4_text(i.x,i.y,&i.lines,&i.family,i.size,i.weight.as_deref().unwrap_or(""),&i.fill,i.italic),_=>panic!("root input method")}}
fn escaped(s:&str)->String{s.replace('&',"&amp;").replace('<',"&lt;").replace('>',"&gt;").replace('"',"&quot;").replace('\'',"&apos;")}
fn attribute_lexeme(s:&str)->String{let xml=format!("<a v=\"{s}\"/>");let doc=roxmltree::Document::parse(&xml).expect("root valid lexical fixture");doc.root_element().attribute("v").unwrap().to_owned()}
fn value(v:&TextValue)->String{match v{TextValue::Literal(s)=>attribute_lexeme(&escaped(s)),TextValue::AttributeLexeme(s)=>attribute_lexeme(s)}}
fn observe(e:&TextPresentation)->Value{
let mut attrs=serde_json::Map::new();attrs.insert("x".into(),json!(e.x.to_string()));attrs.insert("y".into(),json!(e.y.to_string()));attrs.insert("fill".into(),json!(value(&e.fill)));
match &e.font{
FontPresentation::Attributes{family,size,weight}=>{attrs.insert("text-anchor".into(),json!(value(&e.anchor)));attrs.insert("font-family".into(),json!(value(family)));attrs.insert("font-size".into(),json!(size.to_string()));if let Some(w)=weight{attrs.insert("font-weight".into(),json!(value(w)));}},
FontPresentation::C4Style{size_px,weight,family}=>{assert_eq!(e.anchor,TextValue::Literal("middle".into()));attrs.insert("style_size_px".into(),json!(size_px.to_string()));attrs.insert("style_suffix".into(),json!(attribute_lexeme(&format!("font-weight: {}; font-family: {}",escaped(weight),family))));}}
if let Some(true)=e.italic{attrs.insert("font-style".into(),json!("italic"));}if let Some(b)=&e.dominant_baseline{attrs.insert("dominant-baseline".into(),json!(b));}
let mut children=Vec::new();let text=match &e.content{TextContentPresentation::Direct(s)=>s.replace("\r\n","\n").replace('\r',"\n"),TextContentPresentation::Spans(spans)=>{for s in spans{let mut a=serde_json::Map::new();if let Some(x)=s.x{a.insert("x".into(),json!(x.to_string()));}a.insert("dy".into(),json!(s.dy.to_string()));if let Some(b)=&s.alignment_baseline{a.insert("alignment-baseline".into(),json!(b));}children.push(json!({"tag":"tspan","attrs":a,"text":s.text.replace("\r\n","\n").replace('\r',"\n"),"children":[]}));}String::new()}};
json!({"tag":"text","attrs":attrs,"text":text,"children":children})}
fn provenance(e:&TextPresentation,row:&Value){let method=row["method"].as_str().unwrap();match method{
"git"=>{assert!(matches!(&e.anchor,TextValue::Literal(s)if s=="start"));assert!(matches!(&e.fill,TextValue::Literal(_)));},
"c4"=>{assert!(matches!(&e.anchor,TextValue::Literal(s)if s=="middle"));assert!(matches!(&e.fill,TextValue::Literal(_)));assert!(matches!(&e.font,FontPresentation::C4Style{..}));assert_eq!(e.italic,if row["italic"].as_bool().unwrap(){Some(true)}else{None});assert_eq!(e.dominant_baseline.as_deref(),Some("middle"));},
_=>{assert!(matches!(&e.anchor,TextValue::AttributeLexeme(_)));assert!(matches!(&e.fill,TextValue::AttributeLexeme(_)));}}
if let FontPresentation::Attributes{family,weight,..}=&e.font{assert!(matches!(family,TextValue::AttributeLexeme(_)));assert_eq!(e.italic,None);assert_eq!(e.dominant_baseline,None);if let Some(w)=weight{assert!(matches!(w,TextValue::AttributeLexeme(_)));assert_eq!(method,"weight");}}
match (&e.content,method){(TextContentPresentation::Direct(_),"line"|"line_size")=>{},(TextContentPresentation::Spans(spans),m)if m!="line"&&m!="line_size"=>{for s in spans{if m=="c4"{assert_eq!(s.x,None);assert_eq!(s.alignment_baseline.as_deref(),Some("mathematical"));}else{assert_eq!(s.x,Some(e.x));assert_eq!(s.alignment_baseline,None);}}},_=>panic!("wrong source content responsibility")}}
#[test]
fn all_source_text_helpers_match_ordered_source_observations(){
let fixture:Value=serde_json::from_str(include_str!("fixtures/text-presentation.json")).unwrap();let(mut valid,mut invalid)=(0,0);
for(index,row)in fixture["cases"].as_array().unwrap().iter().enumerate(){let input=&row["input"];let result=emit(input);if row["valid"].as_bool().unwrap(){let elements=result.unwrap_or_else(|e|panic!("case {index}: {e:?}"));for e in &elements{provenance(e,input);}let actual:Vec<_>=elements.iter().map(observe).collect();assert_eq!(json!(actual),row["elements"],"case {index}: {input}");valid+=1;}else{assert!(matches!(result,Err(SceneError::InvalidNumber{..})),"numeric source error case {index}: {input} -> {result:?}");invalid+=1;}
assert_eq!(normalize_font_family(input["family"].as_str().unwrap()),row["family"].as_str().unwrap());let dividers:Vec<_>=input["lines"].as_array().unwrap().iter().map(|s|is_divider_line(s.as_str().unwrap())).collect();assert_eq!(json!(dividers),row["dividers"]);}
assert!(valid>300&&invalid>10,"root matrix must exercise valid and invalid source emissions");}
#[test]
fn unresolved_lexical_text_requests_retain_source_values(){
let mut theme=Theme::modern();theme.font_family="A\"B &unknown;".into();let p=text_line_with_font_size(1.0,2.0,"\0&amp;\r\n",&theme,10.0,"&unknown;","end\" injection").unwrap();assert_eq!(p[0].fill,TextValue::AttributeLexeme("&unknown;".into()));assert_eq!(p[0].anchor,TextValue::AttributeLexeme("end\" injection".into()));assert_eq!(p[0].content,TextContentPresentation::Direct("\0&amp;\r\n".into()));assert!(matches!(&p[0].font,FontPresentation::Attributes{family:TextValue::AttributeLexeme(s),..}if s=="A\"B &unknown;"));
let p=c4_text(1.0,2.0,&["x".into()],"Arial",10.0," 600; opacity: .5 ","&unknown;\n",false).unwrap();assert_eq!(p[0].fill,TextValue::Literal("&unknown;\n".into()));assert!(matches!(&p[0].font,FontPresentation::C4Style{weight,..}if weight==" 600; opacity: .5 "));
// These are unresolved requests; this test makes no completed-scene/error claim.
}
