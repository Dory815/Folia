// Root expected geometry/text attributes come from the unchanged source helper.
use mermaid_rs_renderer::scene::{compile::{architecture_icons::*,text_presentation::*},*};
use serde_json::Value;
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
fn check_shape(actual:&SceneNode,w:&Value){
    let SceneNode::Shape(shape)=actual else{panic!("source shape")};
    assert_eq!(w["kind"],"shape");geometry(&shape.geometry,&w["geometry"]);
    assert_eq!(shape.metadata,ExportMetadata::default());
    assert_eq!(shape.style.fill,paint(w["fill"].as_str().unwrap()));
    assert_eq!(shape.style.fill_opacity,1.0);assert_eq!(shape.style.opacity,1.0);
    exact(shape.style.stroke_opacity,&w["stroke_opacity"]);
    if w["stroke"].is_null(){assert!(shape.style.stroke.is_none());}else{
        let s=shape.style.stroke.as_ref().expect("source stroke");let w=&w["stroke"];
        assert_eq!(s.paint,paint(w["paint"].as_str().unwrap()));exact(s.width,&w["width"]);
        assert_eq!(s.cap,LineCap::Round);assert_eq!(w["cap"],"round");
        assert_eq!(s.join,LineJoin::Round);assert_eq!(w["join"],"round");
        assert_eq!(s.miter_limit,4.0);assert_eq!(s.dash_offset,0.0);assert!(s.dash.is_empty());assert_eq!(w["dash"].as_array().unwrap().len(),0);
    }
    finish(actual.clone()).expect("finished typed scene validates");
}
fn check_text(actual:&TextPresentation,w:&Value){
    assert_eq!(w["kind"],"text");exact(actual.x,&w["x"]);exact(actual.y,&w["y"]);
    assert_eq!(actual.anchor,TextValue::Literal(w["anchor"].as_str().unwrap().to_owned()));
    assert_eq!(actual.fill,TextValue::AttributeLexeme(w["fill_lexeme"].as_str().unwrap().to_owned()));
    assert_eq!(actual.dominant_baseline.as_deref(),w["baseline"].as_str());assert!(actual.italic.is_none());
    let FontPresentation::Attributes{family,size,weight}=&actual.font else{panic!("source font attributes")};
    assert_eq!(*family,TextValue::Literal(w["family"].as_str().unwrap().to_owned()));exact(*size,&w["size"]);assert!(weight.is_none());
    assert_eq!(actual.content,TextContentPresentation::Direct(w["text"].as_str().unwrap().to_owned()));
}
#[test]
fn complete_architecture_icon_source_branches_geometry_style_order_and_text_requests(){
    let fixture:Value=serde_json::from_str(include_str!("fixtures/architecture-icons.json")).unwrap();
    let rows=fixture["cases"].as_array().unwrap();assert_eq!(rows.len(),484);let(mut valid,mut invalid)=(0,0);
    for(index,row)in rows.iter().enumerate(){let i=&row["input"];let fill=i["fill"].as_str().unwrap();let resolved=if fill.starts_with('#'){paint(fill)}else{paint("#123456")};
        let result=architecture_icon(i["icon"].as_str(),i["w"].as_str().unwrap().parse().unwrap(),i["h"].as_str().unwrap().parse().unwrap(),&resolved,fill);
        if row["valid"]==true{valid+=1;let result=result.unwrap_or_else(|e|panic!("source case {index}: {e}"));let wants=row["items"].as_array().unwrap();assert_eq!(result.items.len(),wants.len(),"case {index}");
        for(item,w)in result.items.iter().zip(wants){match item{ArchitectureIconItem::Shape(n)=>check_shape(n,w),ArchitectureIconItem::Text(t)=>check_text(t,w)}}
        }else{invalid+=1;assert!(result.is_err(),"invalid source case {index} must diagnose malformed geometry");}
    }assert_eq!((valid,invalid),(432,52));
}
#[test]
fn architecture_icon_shared_scene_validates_resources(){
    let p=architecture_icon(Some("server"),80.0,60.0,&Paint::LinearGradient(GradientId(999)),"url(#x)").unwrap();
    for item in p.items{let ArchitectureIconItem::Shape(n)=item else{panic!("server geometry")};assert!(matches!(finish(n),Err(SceneError::InvalidResource{..})));}
}
