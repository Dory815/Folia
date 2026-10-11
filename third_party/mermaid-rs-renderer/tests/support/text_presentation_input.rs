// Root input construction uses only public source data types, never render code.
use serde_json::Value;
use source::{config::LayoutConfig,layout::TextBlock,theme::Theme};
fn scalar(v:&Value)->f32 {v.as_str().unwrap().parse().unwrap()}
struct Input {
    x:f32,y:f32,size:f32,lh:f32,text:String,lines:Vec<String>,
    family:String,fill:String,weight:Option<String>,anchor:String,
    baseline:bool,edge:bool,italic:bool,override_color:Option<String>,
    theme:Theme,config:LayoutConfig,label:TextBlock,
}
fn input(row:&Value)->Input {
    let lines:Vec<String>=row["lines"].as_array().unwrap().iter().map(|s|s.as_str().unwrap().to_owned()).collect();
    let mut theme=Theme::modern();theme.font_family=row["family"].as_str().unwrap().to_owned();theme.font_size=scalar(&row["theme_size"]);theme.primary_text_color=row["theme_fill"].as_str().unwrap().to_owned();
    let mut config=LayoutConfig::default();config.label_line_height=scalar(&row["lh"]);
    Input{x:scalar(&row["x"]),y:scalar(&row["y"]),size:scalar(&row["size"]),lh:scalar(&row["lh"]),text:row["text"].as_str().unwrap().to_owned(),lines:lines.clone(),family:theme.font_family.clone(),fill:row["fill"].as_str().unwrap().to_owned(),weight:row["weight"].as_str().map(str::to_owned),anchor:row["anchor"].as_str().unwrap().to_owned(),baseline:row["baseline"].as_bool().unwrap(),edge:row["edge"].as_bool().unwrap(),italic:row["italic"].as_bool().unwrap(),override_color:row["override_color"].as_str().map(str::to_owned),theme,config,label:TextBlock{lines,width:f32::NAN,height:f32::INFINITY}}
}
