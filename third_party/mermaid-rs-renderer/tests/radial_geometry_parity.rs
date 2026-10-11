// Root tests over unchanged actual-Source constructor observations.
use mermaid_rs_renderer::scene::{paint::SpreadMode,prepare::{radial_geometry::prepare_radial_geometry,radial_types::{RadialGeometry,RadialKind}}};
use tiny_skia_path::{Point,Transform};
use serde::Deserialize;

#[derive(Deserialize)]
struct Input { circles: [u32;6], transform: [u32;6], stops: usize, spread: u8 }
#[derive(Deserialize)]
struct Case { input: Input, tag: u8, matrix: [u32;6], parameters: [u32;3], swapped: bool, thin_ring: bool }
#[derive(Deserialize)]
struct Fixture { cases: Vec<Case> }
fn assert_bits(value:f32,expected:u32,index:usize,field:&str) {
    if f32::from_bits(expected).is_nan() {assert!(value.is_nan(),"case {index}: {field} expected NaN");}
    else {assert_eq!(value.to_bits(),expected,"case {index}: {field}");}
}
#[test]
fn complete_source_radial_geometry_projection() {
    let fixture:Fixture=serde_json::from_str(include_str!("fixtures/radial-geometry.json")).unwrap();
    assert_eq!(fixture.cases.len(),4956);
    let mut assertions=1;
    for (index,case) in fixture.cases.iter().enumerate() {
        let c=case.input.circles.map(f32::from_bits);let t=case.input.transform.map(f32::from_bits);
        let spread=match case.input.spread {0=>SpreadMode::Pad,1=>SpreadMode::Reflect,2=>SpreadMode::Repeat,_=>panic!("invalid fixture")};
        let native=prepare_radial_geometry(Point::from_xy(c[0],c[1]),c[2],Point::from_xy(c[3],c[4]),c[5],case.input.stops,spread,Transform::from_row(t[0],t[1],t[2],t[3],t[4],t[5]));
        let tag=match native {RadialGeometry::None=>0,RadialGeometry::SolidFirstStop=>1,RadialGeometry::Conical{kind:RadialKind::Radial{..},..}=>2,RadialGeometry::Conical{kind:RadialKind::Strip{..},..}=>3,RadialGeometry::Conical{kind:RadialKind::Focal{..},..}=>4};
        assert_eq!(tag,case.tag,"case {index}: constructor tag");assertions+=1;
        if let RadialGeometry::Conical{kind,points_to_unit:m,thin_ring}=native {
            for(value,bits)in [m.sx,m.ky,m.kx,m.sy,m.tx,m.ty].into_iter().zip(case.matrix) {assert_bits(value,bits,index,"matrix");assertions+=1;}
            let (parameters,swapped)=match kind {RadialKind::Radial{radius1,radius2}=>([radius1,radius2,0.0],false),RadialKind::Strip{scaled_r0}=>([scaled_r0,0.0,0.0],false),RadialKind::Focal{r1,focal_x,is_swapped}=>([r1,focal_x,0.0],is_swapped)};
            for(value,bits)in parameters.into_iter().zip(case.parameters) {assert_bits(value,bits,index,"parameter");assertions+=1;}
            assert_eq!(swapped,case.swapped,"case {index}: swapped");assert_eq!(thin_ring,case.thin_ring,"case {index}: thin ring");assertions+=2;
        }
    }
    println!("{} Source cases, {assertions} successful assertions",fixture.cases.len());
}
