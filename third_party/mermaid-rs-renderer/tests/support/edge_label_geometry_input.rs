fn observe_geometry(input: &serde_json::Value) -> serde_json::Value {
    use serde_json::json;
    fn scalar(v: &serde_json::Value) -> f32 { v.as_str().unwrap().parse().unwrap() }
    fn point(v: &serde_json::Value) -> (f32,f32) { (scalar(&v[0]),scalar(&v[1])) }
    fn float(v: f32) -> serde_json::Value {
        if v.is_nan() { json!("NaN") } else { json!(format!("{:08x}",v.to_bits())) }
    }
    fn rect(v: &serde_json::Value) -> LabelRect {
        LabelRect { x:scalar(&v[0]), y:scalar(&v[1]), width:scalar(&v[2]), height:scalar(&v[3]) }
    }
    let a=point(&input["a"]); let b=point(&input["b"]);
    let c=point(&input["c"]); let d=point(&input["d"]);
    let r=rect(&input["rect"]);
    let result=match input["method"].as_str().unwrap() {
        "center" => {
            let v=&input["center"];
            let r=LabelRect::from_center(scalar(&v[0]),scalar(&v[1]),scalar(&v[2]),scalar(&v[3]),scalar(&v[4]),scalar(&v[5]));
            json!([float(r.x),float(r.y),float(r.width),float(r.height)])
        },
        "visible" => {
            let kind=match input["diagram"].as_str().unwrap() {
                "Flowchart"=>crate::ir::DiagramKind::Flowchart,"Class"=>crate::ir::DiagramKind::Class,
                "State"=>crate::ir::DiagramKind::State,"Sequence"=>crate::ir::DiagramKind::Sequence,
                "Er"=>crate::ir::DiagramKind::Er,"Pie"=>crate::ir::DiagramKind::Pie,
                "Mindmap"=>crate::ir::DiagramKind::Mindmap,"Journey"=>crate::ir::DiagramKind::Journey,
                "Timeline"=>crate::ir::DiagramKind::Timeline,"Gantt"=>crate::ir::DiagramKind::Gantt,
                "Requirement"=>crate::ir::DiagramKind::Requirement,"GitGraph"=>crate::ir::DiagramKind::GitGraph,
                "C4"=>crate::ir::DiagramKind::C4,"Sankey"=>crate::ir::DiagramKind::Sankey,
                "Quadrant"=>crate::ir::DiagramKind::Quadrant,"ZenUML"=>crate::ir::DiagramKind::ZenUML,
                "Block"=>crate::ir::DiagramKind::Block,"Packet"=>crate::ir::DiagramKind::Packet,
                "Kanban"=>crate::ir::DiagramKind::Kanban,"Architecture"=>crate::ir::DiagramKind::Architecture,
                "Radar"=>crate::ir::DiagramKind::Radar,"Treemap"=>crate::ir::DiagramKind::Treemap,
                "XYChart"=>crate::ir::DiagramKind::XYChart,_=>panic!("Unknown source diagram")
            };
            let label=match input["label"].as_str().unwrap() {
                "Center"=>EdgeLabelKind::Center,"Start"=>EdgeLabelKind::Start,"End"=>EdgeLabelKind::End,_=>panic!("Unknown label kind")
            };
            let p:Vec<_>=input["points"].as_array().unwrap().iter().map(point).collect();
            json!(edge_label_background_visible(kind,label,&p,r))
        },
        "polyline" => {
            let p:Vec<_>=input["points"].as_array().unwrap().iter().map(point).collect();
            float(polyline_rect_gap(&p,&r))
        },
        "segment_rect"=>float(segment_rect_gap(a,b,&r)),
        "point_rect"=>float(point_rect_distance(a,&r)),
        "point_segment"=>float(point_segment_distance(c,a,b)),
        "segment_intersects"=>json!(segment_intersects_rect(a,b,&r)),
        "point_in"=>json!(point_in_rect(a,&r)),
        "segments"=>json!(segments_intersect(a,b,c,d)),
        "orient"=>float(orient(a,b,c)),
        "on_segment"=>json!(on_segment(a,b,c,scalar(&input["eps"]))),
        _=>panic!("Unknown helper")
    };
    json!({"input":input,"result":result})
}
