// Root geometry/style expectations come only from the unchanged source helper.
use mermaid_rs_renderer as source;
include!("support/node_shape_input.rs");
use source::scene::{compile::node_shapes::*, *};
fn num(value: &Value) -> f64 {
    value.as_str().unwrap().parse().unwrap()
}
fn exact(actual: f64, value: &Value) {
    let expected = num(value);
    assert_eq!(
        actual.to_bits(),
        expected.to_bits(),
        "source scalar {actual} != {expected}"
    );
}
fn paint(value: &str) -> Paint {
    if value == "none" {
        return Paint::None;
    }
    let rgb = u32::from_str_radix(value.strip_prefix('#').expect("source hex"), 16).unwrap();
    Paint::Solid(Rgba {
        r: ((rgb >> 16) & 255) as f64 / 255.0,
        g: ((rgb >> 8) & 255) as f64 / 255.0,
        b: (rgb & 255) as f64 / 255.0,
        a: 1.0,
    })
}
fn resolved(node: &NodeLayout, theme: &Theme) -> NodeShapePaints {
    let stroke = node
        .style
        .stroke
        .as_ref()
        .unwrap_or(&theme.primary_border_color);
    NodeShapePaints {
        fill: paint(node.style.fill.as_ref().unwrap_or(&theme.primary_color)),
        stroke: paint(stroke),
        background: paint(&theme.background),
        primary_text: paint(&theme.primary_text_color),
        primary_border: paint(&theme.primary_border_color),
        line: paint(&theme.line_color),
        divider_line: paint(node.style.line_color.as_ref().unwrap_or(stroke)),
    }
}
fn point(p: Point, w: &Value) {
    exact(p.x, &w[0]);
    exact(p.y, &w[1]);
}
fn geometry(actual: &ShapeGeometry, w: &Value) {
    let v = &w["values"];
    match actual {
        ShapeGeometry::Rect { rect, rx, ry } => {
            assert_eq!(w["kind"], "rect");
            for (a, b) in [rect.x, rect.y, rect.width, rect.height, *rx, *ry]
                .into_iter()
                .zip(v.as_array().unwrap())
            {
                exact(a, b);
            }
        }
        ShapeGeometry::Circle { center, radius } => {
            assert_eq!(w["kind"], "circle");
            for (a, b) in [center.x, center.y, *radius]
                .into_iter()
                .zip(v.as_array().unwrap())
            {
                exact(a, b);
            }
        }
        ShapeGeometry::Ellipse { center, rx, ry } => {
            assert_eq!(w["kind"], "ellipse");
            for (a, b) in [center.x, center.y, *rx, *ry]
                .into_iter()
                .zip(v.as_array().unwrap())
            {
                exact(a, b);
            }
        }
        ShapeGeometry::Line { start, end } => {
            assert_eq!(w["kind"], "line");
            for (a, b) in [start.x, start.y, end.x, end.y]
                .into_iter()
                .zip(v.as_array().unwrap())
            {
                exact(a, b);
            }
        }
        ShapeGeometry::Polygon(points) => {
            assert_eq!(w["kind"], "polygon");
            let wants = w["points"].as_array().unwrap();
            assert_eq!(points.len(), wants.len());
            for (p, w) in points.iter().zip(wants) {
                point(*p, w);
            }
        }
        ShapeGeometry::Path(path) => {
            assert_eq!(w["kind"], "path");
            assert_eq!(path.fill_rule(), FillRule::NonZero);
            let segments = w["segments"].as_array().unwrap();
            assert_eq!(path.segments().len(), segments.len());
            for (actual, w) in path.segments().iter().zip(segments) {
                let v = &w["coordinates"];
                match actual {
                    PathSegment::MoveTo(p) => {
                        assert_eq!(w["command"], "M");
                        point(*p, v);
                    }
                    PathSegment::LineTo(p) => {
                        assert_eq!(w["command"], "L");
                        point(*p, v);
                    }
                    PathSegment::QuadraticTo { control, end } => {
                        assert_eq!(w["command"], "Q");
                        for (a, b) in [control.x, control.y, end.x, end.y]
                            .into_iter()
                            .zip(v.as_array().unwrap())
                        {
                            exact(a, b);
                        }
                    }
                    PathSegment::Close => {
                        assert_eq!(w["command"], "Z");
                        assert!(v.as_array().unwrap().is_empty());
                    }
                    _ => panic!("unexpected node shape path segment"),
                }
            }
        }
        _ => panic!("unexpected node shape geometry"),
    }
}
fn check(actual: &SceneNode, rows: &Value) {
    let SceneNode::Group(group) = actual else {
        panic!("source helper returns a group")
    };
    assert_eq!(group.transform, Affine::IDENTITY);
    assert!(group.clip.is_none());
    assert_eq!(group.layer, LayerStyle::default());
    assert_eq!(group.metadata, ExportMetadata::default());
    let wants = rows.as_array().unwrap();
    assert_eq!(group.children.len(), wants.len());
    for (actual, w) in group.children.iter().zip(wants) {
        let SceneNode::Shape(shape) = actual else {
            panic!("ordinary source shape")
        };
        geometry(&shape.geometry, &w["geometry"]);
        assert_eq!(shape.metadata, ExportMetadata::default());
        assert_eq!(shape.style.fill, paint(w["fill"].as_str().unwrap()));
        assert_eq!(shape.style.fill_opacity, 1.0);
        assert_eq!(shape.style.opacity, 1.0);
        exact(shape.style.stroke_opacity, &w["stroke_opacity"]);
        let stroke = shape.style.stroke.as_ref().unwrap();
        assert_eq!(stroke.paint, paint(w["stroke"].as_str().unwrap()));
        exact(stroke.width, &w["width"]);
        assert_eq!(stroke.dash_offset, 0.0);
        assert_eq!(stroke.miter_limit, 4.0);
        assert_eq!(
            stroke.cap,
            match w["cap"].as_str().unwrap() {
                "round" => LineCap::Round,
                "butt" => LineCap::Butt,
                _ => panic!("source cap"),
            }
        );
        assert_eq!(
            stroke.join,
            match w["join"].as_str().unwrap() {
                "round" => LineJoin::Round,
                "miter" => LineJoin::Miter,
                _ => panic!("source join"),
            }
        );
        let dash = w["dash"].as_array().unwrap();
        assert_eq!(stroke.dash.len(), dash.len());
        for (a, b) in stroke.dash.iter().zip(dash) {
            exact(*a, b);
        }
    }
}
fn finish(node: SceneNode) -> Result<DiagramScene, SceneError> {
    let rect = Rect {
        x: 0.0,
        y: 0.0,
        width: 100.0,
        height: 100.0,
    };
    let mut builder = SceneBuilder::new(
        SceneDimensions {
            width: 100.0,
            height: 100.0,
            viewbox: rect,
        },
        RootViewport {
            clip: rect,
            viewbox_to_viewport: Affine::IDENTITY,
            aspect_ratio: AspectRatioPolicy::None,
        },
    );
    builder.push_node(node);
    builder.finish()
}
#[test]
fn every_node_shape_source_geometry_state_paint_order_and_precision() {
    let fixture: Value = serde_json::from_str(include_str!("fixtures/node-shapes.json")).unwrap();
    let cases = fixture["cases"].as_array().unwrap();
    assert_eq!(cases.len(), 508);
    let mut valid = 0;
    let mut invalid = 0;
    for (index, row) in cases.iter().enumerate() {
        let (node, theme, config) = input(&row["input"]);
        let dash = node
            .style
            .stroke_dasharray
            .as_ref()
            .map(|text| {
                text.split(|c: char| c == ',' || c.is_ascii_whitespace())
                    .filter(|s| !s.is_empty())
                    .map(|s| s.parse::<f64>().unwrap())
                    .collect::<Vec<_>>()
            })
            .unwrap_or_default();
        let result = node_shape(&node, &config, &resolved(&node, &theme), &dash);
        if row["valid"] == true {
            valid += 1;
            let result = result.unwrap_or_else(|e| panic!("source case {index}: {e}"));
            check(&result, &row["shapes"]);
            finish(result).unwrap_or_else(|e| panic!("source case {index} final scene: {e}"));
        } else {
            invalid += 1;
            assert!(
                result.is_err(),
                "invalid source case {index} must not publish a shape"
            );
        }
    }
    assert_eq!(valid, 480);
    assert_eq!(invalid, 28);
}
#[test]
fn node_shape_resource_validation_and_invalid_numbers_are_explicit() {
    let fixture: Value = serde_json::from_str(include_str!("fixtures/node-shapes.json")).unwrap();
    let (mut node, theme, mut config) = input(&fixture["cases"][0]["input"]);
    let paints = resolved(&node, &theme);
    for dash in [vec![f64::NAN], vec![f64::INFINITY], vec![-0.1]] {
        assert!(node_shape(&node, &config, &paints, &dash).is_err());
    }
    node.shape = NodeShape::MindmapDefault;
    config.mindmap.default_corner_radius = f32::NAN;
    assert!(node_shape(&node, &config, &paints, &[]).is_err());
    config.mindmap.default_corner_radius = 7.0;
    config.mindmap.divider_line_width = f32::INFINITY;
    assert!(node_shape(&node, &config, &paints, &[]).is_err());
    config.mindmap.divider_line_width = 1.0;
    let mut bad = paints;
    bad.stroke = Paint::LinearGradient(GradientId(999));
    let compiled = node_shape(&node, &config, &bad, &[]).unwrap();
    assert!(matches!(
        finish(compiled),
        Err(SceneError::InvalidResource { .. })
    ));
}
