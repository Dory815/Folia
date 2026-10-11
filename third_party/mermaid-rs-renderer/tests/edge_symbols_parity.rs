// Root expected scene observations come from actual unchanged upstream helpers.
use mermaid_rs_renderer::{
    ir::EdgeDecoration,
    scene::{compile::edge_symbols::*, *},
};
use serde_json::Value;
fn num(v: &Value) -> f64 {
    v.as_str().unwrap().parse().unwrap()
}
fn scalar(v: &Value) -> f32 {
    v.as_str().unwrap().parse().unwrap()
}
fn exact(actual: f64, expected: f64) {
    assert_eq!(
        actual.to_bits(),
        expected.to_bits(),
        "source decimal scalar {actual} != {expected}"
    );
}
fn color() -> Paint {
    Paint::Solid(Rgba {
        r: 161.0 / 255.0,
        g: 78.0 / 255.0,
        b: 197.0 / 255.0,
        a: 1.0,
    })
}
fn point(actual: Point, expected: &Value) {
    exact(actual.x, num(&expected[0]));
    exact(actual.y, num(&expected[1]));
}
fn check(node: &SceneNode, expected: &Value) {
    match node {
        SceneNode::Group(group) => {
            assert_eq!(expected["kind"], "group");
            let a = group.transform;
            for (actual, want) in [a.a, a.b, a.c, a.d, a.e, a.f]
                .into_iter()
                .zip(expected["matrix"].as_array().unwrap())
            {
                let want = num(want);
                assert!(
                    (actual - want).abs() <= 1e-12,
                    "source matrix {actual} != {want}"
                );
            }
            // Matrix multiplication may discard signed zero. Check source's
            // translate tokens separately to retain the emitted decimal value.
            exact(a.e, num(&expected["translation"][0]));
            exact(a.f, num(&expected["translation"][1]));
            assert!(group.clip.is_none());
            assert_eq!(group.layer, LayerStyle::default());
            assert_eq!(group.metadata, ExportMetadata::default());
            let children = expected["children"].as_array().unwrap();
            assert_eq!(group.children.len(), children.len());
            for (child, expected) in group.children.iter().zip(children) {
                check(child, expected);
            }
        }
        SceneNode::Shape(shape) => {
            assert_eq!(expected["kind"], "shape");
            let geometry = &expected["geometry"];
            match &shape.geometry {
                ShapeGeometry::Circle { center, radius } => {
                    assert_eq!(geometry["kind"], "circle");
                    exact(center.x, num(&geometry["values"][0]));
                    exact(center.y, num(&geometry["values"][1]));
                    exact(*radius, num(&geometry["values"][2]));
                }
                ShapeGeometry::Polygon(points) => {
                    assert_eq!(geometry["kind"], "polygon");
                    let wanted = geometry["points"].as_array().unwrap();
                    assert_eq!(points.len(), wanted.len());
                    for (&p, w) in points.iter().zip(wanted) {
                        point(p, w);
                    }
                }
                ShapeGeometry::Path(path) => {
                    assert_eq!(geometry["kind"], "path");
                    assert_eq!(path.fill_rule(), FillRule::NonZero);
                    let wanted = geometry["segments"].as_array().unwrap();
                    assert_eq!(path.segments().len(), wanted.len());
                    for (p, w) in path.segments().iter().zip(wanted) {
                        match p {
                            PathSegment::MoveTo(p) => {
                                assert_eq!(w["command"], "M");
                                point(*p, &w["coordinates"]);
                            }
                            PathSegment::LineTo(p) => {
                                assert_eq!(w["command"], "L");
                                point(*p, &w["coordinates"]);
                            }
                            _ => panic!("unexpected extra symbol segment"),
                        }
                    }
                }
                _ => panic!("unexpected symbol shape"),
            }
            assert_eq!(shape.metadata, ExportMetadata::default());
            let style = &shape.style;
            assert_eq!(
                style.fill,
                if expected["fill"] == "none" {
                    Paint::None
                } else {
                    assert_eq!(expected["fill"], "#a14ec5");
                    color()
                }
            );
            assert_eq!(
                (style.fill_opacity, style.stroke_opacity, style.opacity),
                (1.0, 1.0, 1.0)
            );
            let stroke = style.stroke.as_ref().unwrap();
            assert_eq!(expected["stroke"], "#a14ec5");
            assert_eq!(stroke.paint, color());
            exact(stroke.width, num(&expected["width"]));
            assert!(stroke.dash.is_empty());
            exact(stroke.dash_offset, 0.0);
            exact(stroke.miter_limit, num(&expected["miter_limit"]));
            assert_eq!(
                stroke.cap,
                match expected["cap"].as_str().unwrap() {
                    "round" => LineCap::Round,
                    "butt" => LineCap::Butt,
                    _ => panic!("unexpected source cap"),
                }
            );
            assert_eq!(
                stroke.join,
                match expected["join"].as_str().unwrap() {
                    "round" => LineJoin::Round,
                    "miter" => LineJoin::Miter,
                    _ => panic!("unexpected source join"),
                }
            );
        }
        _ => panic!("unexpected symbol node"),
    }
}
fn kind(value: &str) -> EdgeDecoration {
    match value {
        "Circle" => EdgeDecoration::Circle,
        "Cross" => EdgeDecoration::Cross,
        "Diamond" => EdgeDecoration::Diamond,
        "DiamondFilled" => EdgeDecoration::DiamondFilled,
        "CrowsFootOne" => EdgeDecoration::CrowsFootOne,
        "CrowsFootZeroOne" => EdgeDecoration::CrowsFootZeroOne,
        "CrowsFootMany" => EdgeDecoration::CrowsFootMany,
        "CrowsFootZeroMany" => EdgeDecoration::CrowsFootZeroMany,
        _ => panic!("unexpected fixture kind"),
    }
}
#[test]
fn every_symbol_contour_style_hierarchy_and_transform_matches_source() {
    let fixture: Value = serde_json::from_str(include_str!("fixtures/edge-symbols.json")).unwrap();
    let cases = fixture["cases"].as_array().unwrap();
    assert_eq!(cases.len(), 286);
    for (index, row) in cases.iter().enumerate() {
        let input = &row["input"];
        let p = (scalar(&input["point"][0]), scalar(&input["point"][1]));
        let angle = scalar(&input["angle"]);
        let width = scalar(&input["width"]);
        let symbol = if input["kind"] == "arrowhead" {
            arrowhead(p, angle, color(), width)
        } else {
            edge_decoration(
                p,
                angle,
                kind(input["kind"].as_str().unwrap()),
                color(),
                width,
                input["at_start"].as_bool().unwrap(),
            )
        }
        .unwrap_or_else(|e| panic!("source case {index}: {e}"));
        check(&symbol, &row["observation"]);
        let bounds = Rect {
            x: 0.0,
            y: 0.0,
            width: 1.0,
            height: 1.0,
        };
        let mut b = SceneBuilder::new(
            SceneDimensions {
                width: 1.0,
                height: 1.0,
                viewbox: bounds,
            },
            RootViewport {
                clip: bounds,
                aspect_ratio: AspectRatioPolicy::None,
                viewbox_to_viewport: Affine::IDENTITY,
            },
        );
        b.push_node(symbol);
        b.finish()
            .unwrap_or_else(|e| panic!("source case {index} fails final scene: {e}"));
    }
}
#[test]
fn invalid_inputs_fail_before_a_partial_symbol_is_published() {
    for (p, angle, width) in [
        ((f32::NAN, 0.0), 0.0, 1.0),
        ((0.0, f32::INFINITY), 0.0, 1.0),
        ((0.0, 0.0), f32::NEG_INFINITY, 1.0),
        ((0.0, 0.0), 0.0, f32::NAN),
        ((0.0, 0.0), 0.0, f32::INFINITY),
    ] {
        assert!(matches!(
            arrowhead(p, angle, color(), width),
            Err(SceneError::InvalidNumber { .. })
        ));
        for decoration in [
            EdgeDecoration::Circle,
            EdgeDecoration::Cross,
            EdgeDecoration::Diamond,
            EdgeDecoration::DiamondFilled,
            EdgeDecoration::CrowsFootOne,
            EdgeDecoration::CrowsFootZeroOne,
            EdgeDecoration::CrowsFootMany,
            EdgeDecoration::CrowsFootZeroMany,
        ] {
            assert!(matches!(
                edge_decoration(p, angle, decoration, color(), width, false),
                Err(SceneError::InvalidNumber { .. })
            ));
        }
    }
    assert!(matches!(
        arrowhead((0.0, 0.0), 0.0, color(), -1.0),
        Err(SceneError::InvalidPaint { .. })
    ));
    assert!(matches!(
        edge_decoration((0.0, 0.0), 0.0, EdgeDecoration::Circle, color(), -1.0, true),
        Err(SceneError::InvalidPaint { .. })
    ));
}
