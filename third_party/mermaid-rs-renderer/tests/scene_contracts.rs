// Root-owned behavior checks; implementation agents do not edit expectations.
use mermaid_rs_renderer::scene::*;
use std::sync::Arc;

fn builder() -> SceneBuilder {
    let bounds=Rect{x:0.0,y:0.0,width:100.0,height:80.0};
    SceneBuilder::new(SceneDimensions{width:100.0,height:80.0,viewbox:bounds},
        RootViewport{clip:bounds,aspect_ratio:AspectRatioPolicy::Meet(AspectAlign::XMidYMid),viewbox_to_viewport:Affine::IDENTITY})
}
fn shape(g:ShapeGeometry) -> SceneNode {
    SceneNode::Shape(ShapeNode{geometry:g,style:PaintStyle::default(),metadata:ExportMetadata::default()})
}
fn group(children:Vec<SceneNode>) -> SceneNode {
    SceneNode::Group(GroupNode{children,transform:Affine::IDENTITY,clip:None,layer:LayerStyle::default(),metadata:ExportMetadata::default()})
}
fn solid() -> Paint {Paint::Solid(Rgba{r:1.0,g:0.0,b:0.0,a:1.0})}
fn text(value:&str) -> TextNode {
    TextNode{text:value.into(),font:FontSpec{families:vec!["Arial".into()],size:16.0,weight:400,stretch:100.0,style:FontStyle::Normal},origin:Point{x:12.0,y:30.0},baseline:TextBaseline::Alphabetic,anchor:TextAnchor::Start,runs:vec![],style:PaintStyle{fill:solid(),..Default::default()},metadata:Default::default()}
}
fn font() -> Arc<[u8]> {std::fs::read(std::env::var("FOLIA_SCENE_TEST_FONT").expect("root supplies fixed local font fixture")).expect("font fixture").into()}
fn with_run(t:&mut TextNode,id:FontId,source:std::ops::Range<usize>) {
    t.runs.push(GlyphRun{font:id,origin:t.origin,em_size:16.0,glyphs:vec![PositionedGlyph{glyph_id:0,advance:8.0,offset:Point::default(),cluster:source.clone(),appearance:GlyphAppearance::FontOutline}],source_range:source,bidi_level:0,sideways:false});
}

#[test] fn completed_scene_is_portable_and_empty_does_not_invent_drawing() {
    fn portable<T:Send+Sync>(){} portable::<DiagramScene>();
    let scene=builder().finish().unwrap();assert!(scene.nodes().is_empty());assert!(scene.images().is_empty());
    assert_eq!(scene.dimensions().viewbox.width,100.0);
}
#[test] fn affine_order_matches_column_vector_coordinates() {
    let translation=Affine{e:10.0,f:20.0,..Affine::IDENTITY};
    let scaling=Affine{a:2.0,d:3.0,..Affine::IDENTITY};
    assert_eq!(translation.compose(scaling).transform_point(Point{x:4.0,y:5.0}),Point{x:18.0,y:35.0});
    assert_eq!(scaling.compose(translation).transform_point(Point{x:4.0,y:5.0}),Point{x:28.0,y:75.0});
}
#[test] fn painter_order_and_zero_length_geometry_survive_finish() {
    let mut b=builder();b.push_node(shape(ShapeGeometry::Circle{center:Point::default(),radius:0.0}));
    b.push_node(shape(ShapeGeometry::Line{start:Point{x:3.0,y:4.0},end:Point{x:3.0,y:4.0}}));
    let scene=b.finish().unwrap();assert_eq!(scene.nodes().len(),2);
    assert!(matches!(&scene.nodes()[0],SceneNode::Shape(ShapeNode{geometry:ShapeGeometry::Circle{radius:0.0,..},..})));
    assert!(matches!(&scene.nodes()[1],SceneNode::Shape(ShapeNode{geometry:ShapeGeometry::Line{..},..})));
}
#[test] fn invalid_numbers_negative_radii_and_unstarted_paths_are_rejected() {
    for g in [ShapeGeometry::Circle{center:Point::default(),radius:-1.0},ShapeGeometry::Line{start:Point{x:f64::NAN,y:0.0},end:Point::default()},ShapeGeometry::Path(Path::new(vec![PathSegment::LineTo(Point::default())],FillRule::NonZero))] {
        let mut b=builder();b.push_node(shape(g));assert!(b.finish().is_err());
    }
    let mut b=builder();b.push_node(group(vec![shape(ShapeGeometry::Path(Path::new(vec![PathSegment::MoveTo(Point::default()),PathSegment::ArcTo{rx:-2.0,ry:1.0,rotation_degrees:0.0,large_arc:true,sweep:false,end:Point::default()}],FillRule::EvenOdd)))]));assert!(b.finish().is_err());
}
#[test] fn valid_curve_and_arc_geometry_retains_all_segments() {
    let parts=vec![PathSegment::MoveTo(Point::default()),PathSegment::QuadraticTo{control:Point{x:1.0,y:2.0},end:Point{x:3.0,y:4.0}},PathSegment::CubicTo{control1:Point{x:4.0,y:5.0},control2:Point{x:6.0,y:7.0},end:Point{x:8.0,y:9.0}},PathSegment::ArcTo{rx:10.0,ry:12.0,rotation_degrees:25.0,large_arc:true,sweep:false,end:Point{x:20.0,y:30.0}},PathSegment::Close];
    let mut b=builder();b.push_node(shape(ShapeGeometry::Path(Path::new(parts.clone(),FillRule::EvenOdd))));
    let scene=b.finish().unwrap();let SceneNode::Shape(n)=&scene.nodes()[0] else{panic!()};let ShapeGeometry::Path(p)=&n.geometry else{panic!()};assert_eq!(p.segments(),parts.as_slice());assert_eq!(p.fill_rule(),FillRule::EvenOdd);
}
#[test] fn bad_paint_reference_stops_and_opacity_fail_before_publication() {
    let mut b=builder();b.push_node(SceneNode::Shape(ShapeNode{geometry:ShapeGeometry::Polygon(vec![]),style:PaintStyle{fill:Paint::LinearGradient(GradientId(3)),..Default::default()},metadata:Default::default()}));assert!(matches!(b.finish(),Err(SceneError::InvalidResource{..})));
    let mut b=builder();b.add_gradient(LinearGradient{units:GradientUnits::UserSpace,transform:Affine::IDENTITY,start:Point::default(),end:Point{x:1.0,y:1.0},stops:vec![GradientStop{offset:0.8,color:Rgba{r:0.0,g:0.0,b:0.0,a:1.0},opacity:1.0},GradientStop{offset:0.2,color:Rgba{r:0.0,g:0.0,b:0.0,a:1.0},opacity:1.0}],spread:SpreadMode::Pad});assert!(matches!(b.finish(),Err(SceneError::InvalidPaint{..})));
    let mut n=group(vec![]);if let SceneNode::Group(g)=&mut n{g.layer.opacity=1.01;}let mut b=builder();b.push_node(n);assert!(b.finish().is_err());
}
#[test] fn clip_transform_and_metadata_cannot_bypass_geometry_validation() {
    let mut n=group(vec![]);if let SceneNode::Group(g)=&mut n{g.clip=Some(Clip{geometry:ShapeGeometry::Circle{center:Point::default(),radius:1.0},transform:Affine{a:f64::INFINITY,..Affine::IDENTITY}});}let mut b=builder();b.push_node(n);assert!(matches!(b.finish(),Err(SceneError::InvalidNumber{..})));
    let mut b=builder();b.set_metadata(ExportMetadata{data_attributes:vec![DataAttribute{name:"x\" fill".into(),value:"red".into()}],..Default::default()});assert!(b.finish().is_err());
}
#[test] fn visible_text_without_glyphs_is_an_error() {
    let mut b=builder();b.push_node(SceneNode::Text(text("数学")));assert!(matches!(b.finish(),Err(SceneError::UnresolvedText{..})));
}
#[test] fn hidden_and_empty_text_need_no_invented_glyphs() {
    let mut b=builder();b.push_node(SceneNode::Text(text("")));let mut hidden=text("hidden");hidden.style.opacity=0.0;b.push_node(SceneNode::Text(hidden));assert!(b.finish().is_ok());
}
#[test] fn font_identity_ownership_face_and_glyph_bounds_are_checked() {
    let mut b=builder();b.add_font(FontResource{bytes:Arc::from([1u8,2,3]),face_index:0,identity:"bad".into()});assert!(matches!(b.finish(),Err(SceneError::InvalidResource{..})));
    let mut b=builder();b.add_font(FontResource{bytes:font(),face_index:999,identity:"bad-face".into()});assert!(b.finish().is_err());
    let mut b=builder();let id=b.add_font(FontResource{bytes:font(),face_index:0,identity:"fixed-font".into()});let mut t=text("x");with_run(&mut t,id,0..1);t.runs[0].glyphs[0].glyph_id=u16::MAX;b.push_node(SceneNode::Text(t));assert!(b.finish().is_err());
}
#[test] fn positioned_text_requires_complete_utf8_source_coverage() {
    let mut b=builder();let bytes=font();let id=b.add_font(FontResource{bytes:bytes.clone(),face_index:0,identity:"fixed-font".into()});let mut t=text("AB");with_run(&mut t,id,0..1);b.push_node(SceneNode::Text(t));assert!(matches!(b.finish(),Err(SceneError::UnresolvedText{..})));
    let mut b=builder();let id=b.add_font(FontResource{bytes,face_index:0,identity:"fixed-font".into()});let mut t=text("中e\u{301}");let length=t.text.len();with_run(&mut t,id,0..length);b.push_node(SceneNode::Text(t));let scene=b.finish().unwrap();assert_eq!(scene.fonts().len(),1);assert_eq!(scene.fonts()[0].identity,"fixed-font");
    let mut b=builder();let id=b.add_font(FontResource{bytes:font(),face_index:0,identity:"fixed-font".into()});let mut t=text("中");with_run(&mut t,id,0..1);b.push_node(SceneNode::Text(t));assert!(b.finish().is_err());
}
#[test] fn image_refs_and_retained_source_bytes_are_owned() {
    let mut b=builder();let bytes:Arc<[u8]>=Arc::from([137u8,80,78,71]);let weak=Arc::downgrade(&bytes);let id=b.add_image(ImageResource{encoded:bytes,mime:ImageMime::Png,pixel_width:1,pixel_height:1});b.push_node(SceneNode::Image(ImageNode{image:id,destination:Rect{x:1.0,y:2.0,width:4.0,height:5.0},source_to_destination:Affine{a:4.0,d:5.0,e:1.0,f:2.0,..Affine::IDENTITY},aspect_ratio:AspectRatioPolicy::None,style:Default::default(),metadata:Default::default()}));let scene=b.finish().unwrap();assert!(weak.upgrade().is_some());drop(scene);assert!(weak.upgrade().is_none());
    let mut b=builder();b.push_node(SceneNode::Image(ImageNode{image:ImageId(999),destination:Rect::default(),source_to_destination:Affine::IDENTITY,aspect_ratio:AspectRatioPolicy::None,style:Default::default(),metadata:Default::default()}));assert!(b.finish().is_err());
}
#[test] fn rejected_or_abandoned_deep_trees_drop_without_recursive_overflow() {
    std::thread::Builder::new().stack_size(128*1024).spawn(||{
        let mut n=group(vec![]);for _ in 0..20_000{n=group(vec![n]);}let mut b=builder();b.push_node(n);assert!(matches!(b.finish(),Err(SceneError::UnbalancedState{..})));
        let mut n=group(vec![]);for _ in 0..20_000{n=group(vec![n]);}let mut b=builder();b.push_node(n);drop(b);
    }).unwrap().join().unwrap();
}
