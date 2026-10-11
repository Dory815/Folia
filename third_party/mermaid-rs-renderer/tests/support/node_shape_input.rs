// Root fixture construction; only uses source public data types, never render code.
use serde_json::Value;
use source::{
    config::LayoutConfig,
    ir::{NodeShape, NodeStyle},
    layout::{NodeLayout, TextBlock},
    theme::Theme,
};
fn scalar(v: &Value) -> f32 {
    v.as_str()
        .expect("scalar string")
        .parse()
        .expect("f32 input")
}
fn input(row: &Value) -> (NodeLayout, Theme, LayoutConfig) {
    let shape = match row["shape"].as_str().unwrap() {
        "Rectangle" => NodeShape::Rectangle,
        "ForkJoin" => NodeShape::ForkJoin,
        "RoundRect" => NodeShape::RoundRect,
        "Stadium" => NodeShape::Stadium,
        "Subroutine" => NodeShape::Subroutine,
        "Cylinder" => NodeShape::Cylinder,
        "ActorBox" => NodeShape::ActorBox,
        "Circle" => NodeShape::Circle,
        "DoubleCircle" => NodeShape::DoubleCircle,
        "Diamond" => NodeShape::Diamond,
        "Hexagon" => NodeShape::Hexagon,
        "Parallelogram" => NodeShape::Parallelogram,
        "ParallelogramAlt" => NodeShape::ParallelogramAlt,
        "Trapezoid" => NodeShape::Trapezoid,
        "TrapezoidAlt" => NodeShape::TrapezoidAlt,
        "Asymmetric" => NodeShape::Asymmetric,
        "MindmapDefault" => NodeShape::MindmapDefault,
        "Text" => NodeShape::Text,
        _ => panic!("unknown source shape"),
    };
    let overridden = row["overrides"].as_bool().unwrap();
    let mut style = NodeStyle::default();
    if overridden {
        style.fill = Some("#101112".into());
        style.stroke = Some("#131415".into());
        style.line_color = Some("#161718".into());
    }
    if !row["width"].is_null() {
        style.stroke_width = Some(scalar(&row["width"]));
    }
    if !row["dash"].is_null() {
        style.stroke_dasharray = Some(row["dash"].as_str().unwrap().into());
    }
    let node = NodeLayout {
        id: row["id"].as_str().unwrap().into(),
        x: scalar(&row["box"][0]),
        y: scalar(&row["box"][1]),
        width: scalar(&row["box"][2]),
        height: scalar(&row["box"][3]),
        label: TextBlock {
            lines: row["lines"]
                .as_array()
                .unwrap()
                .iter()
                .map(|v| v.as_str().unwrap().into())
                .collect(),
            width: 0.0,
            height: 0.0,
        },
        shape,
        style,
        link: None,
        anchor_subgraph: None,
        hidden: false,
        icon: None,
    };
    let mut theme = Theme::modern();
    theme.primary_color = "#010203".into();
    theme.primary_border_color = "#040506".into();
    theme.background = "#070809".into();
    theme.primary_text_color = "#0a0b0c".into();
    theme.line_color = "#0d0e0f".into();
    let mut config = LayoutConfig::default();
    config.mindmap.default_corner_radius = scalar(&row["corner"]);
    config.mindmap.divider_line_width = scalar(&row["divider"]);
    (node, theme, config)
}
