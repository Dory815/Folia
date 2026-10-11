// Root actual-source fixtures; no implementation-generated expected geometry.
use mermaid_rs_renderer::scene::{Path,PathSegment,Point,FillRule,SceneError};
use mermaid_rs_renderer::scene::compile::paths::*;
use serde_json::Value;

fn numeric(x:f64,y:f64,context:&str) {assert_eq!(x.to_bits(),y.to_bits(),"{context}: {x:?} vs {y:?}");}
fn coordinates(segment:&PathSegment)->(&'static str,Vec<f64>){
    match segment {
        PathSegment::MoveTo(p)=>("M",vec![p.x,p.y]),PathSegment::LineTo(p)=>("L",vec![p.x,p.y]),
        PathSegment::QuadraticTo{control,end}=>("Q",vec![control.x,control.y,end.x,end.y]),
        PathSegment::CubicTo{control1,control2,end}=>("C",vec![control1.x,control1.y,control2.x,control2.y,end.x,end.y]),
        _=>panic!("Source helpers have only M/L/Q/C; extra native command")
    }
}
fn compare(actual:Result<Path,SceneError>,expected:&Value,context:&str){
    if expected.is_null(){assert!(matches!(actual,Err(SceneError::InvalidNumber{..})),"{context}: invalid source numeric path must fail");return;}
    let actual=actual.unwrap_or_else(|e|panic!("{context}: {e}"));assert_eq!(actual.fill_rule(),FillRule::NonZero);
    let expected=expected.as_array().unwrap();assert_eq!(actual.segments().len(),expected.len(),"{context}: complete command order/count");
    for (i,(a,e)) in actual.segments().iter().zip(expected).enumerate(){let (kind,coords)=coordinates(a);assert_eq!(kind,e["command"].as_str().unwrap(),"{context}/{i}");let values=e["coordinates"].as_array().unwrap();assert_eq!(coords.len(),values.len());for (j,(x,y)) in coords.into_iter().zip(values).enumerate(){numeric(x,y.as_str().unwrap().parse().unwrap(),&format!("{context}/{i}/{j}"));}}
}
#[test]fn every_command_and_coordinate_matches_unchanged_upstream(){
    let fixtures:Value=serde_json::from_str(include_str!("fixtures/paths.json")).unwrap();
    let rows=fixtures["cases"].as_array().unwrap();assert_eq!(rows.len(),164);
    for (i,row) in rows.iter().enumerate(){
        let points:Vec<(f32,f32)>=row["points"].as_array().unwrap().iter().map(|p|(p[0].as_f64().unwrap() as f32,p[1].as_f64().unwrap() as f32)).collect();
        let radius=row["radius"].as_f64().unwrap() as f32;
        let deduped=dedupe_points(&points).unwrap();let expected=row["deduped"].as_array().unwrap();assert_eq!(deduped.len(),expected.len(),"dedupe/{i}");for(a,e)in deduped.iter().zip(expected){assert_eq!(a.0.to_bits(),(e[0].as_f64().unwrap() as f32).to_bits());assert_eq!(a.1.to_bits(),(e[1].as_f64().unwrap() as f32).to_bits());}
        compare(points_to_path(&points),&row["geometry"]["plain"],&format!("plain/{i}"));compare(rounded_polyline_path(&points,radius),&row["geometry"]["rounded"],&format!("rounded/{i}"));compare(basis_curve_path(&points),&row["geometry"]["basis"],&format!("basis/{i}"));
    }
}
#[test]fn invalid_inputs_fail_and_finite_predicate_overflow_preserves_source(){
    for invalid in [f32::NAN,f32::INFINITY,f32::NEG_INFINITY]{let points=[(invalid,0.0)];assert!(dedupe_points(&points).is_err());assert!(points_to_path(&points).is_err());assert!(rounded_polyline_path(&points,1.0).is_err());assert!(basis_curve_path(&points).is_err());assert!(rounded_polyline_path(&[],invalid).is_err());}
    let points=[(f32::MAX,0.0),(-f32::MAX,0.0)];assert_eq!(dedupe_points(&points).unwrap(),points);assert_eq!(points_to_path(&points).unwrap().segments().len(),2);
    let PathSegment::MoveTo(Point{x,..})=points_to_path(&[(-0.00001,0.0)]).unwrap().segments()[0] else {panic!()};assert_eq!(x.to_bits(),(-0.0f64).to_bits());
}
