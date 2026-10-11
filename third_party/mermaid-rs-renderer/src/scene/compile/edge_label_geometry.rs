//! Exact edge-label background geometry from the pinned renderer.

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct LabelRect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

impl LabelRect {
    pub fn from_center(
        center_x: f32,
        center_y: f32,
        label_w: f32,
        label_h: f32,
        pad_x: f32,
        pad_y: f32,
    ) -> Self {
        let width = (label_w + pad_x * 2.0).max(0.0);
        let height = (label_h + pad_y * 2.0).max(0.0);
        Self {
            x: center_x - width * 0.5,
            y: center_y - height * 0.5,
            width,
            height,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EdgeLabelKind {
    Center,
    Start,
    End,
}

pub fn edge_label_background_visible(
    diagram_kind: crate::ir::DiagramKind,
    label_kind: EdgeLabelKind,
    edge_points: &[(f32, f32)],
    rect: LabelRect,
) -> bool {
    if edge_points.len() < 2 || rect.width <= 0.0 || rect.height <= 0.0 {
        return true;
    }
    let gap = polyline_rect_gap(edge_points, &rect);
    match label_kind {
        EdgeLabelKind::Center => {
            let gap_limit = match diagram_kind {
                crate::ir::DiagramKind::Flowchart => 1.2,
                crate::ir::DiagramKind::Sequence => (rect.height * 0.16).clamp(1.2, 2.4),
                crate::ir::DiagramKind::Requirement => 1.0,
                _ => 0.9,
            };
            gap <= gap_limit
        }
        EdgeLabelKind::Start | EdgeLabelKind::End => match diagram_kind {
            crate::ir::DiagramKind::Sequence => gap <= (rect.height * 0.12).clamp(0.6, 1.4),
            crate::ir::DiagramKind::Flowchart | crate::ir::DiagramKind::Requirement => gap <= 0.35,
            _ => false,
        },
    }
}

pub fn polyline_rect_gap(points: &[(f32, f32)], rect: &LabelRect) -> f32 {
    if points.len() < 2 {
        return f32::INFINITY;
    }
    let mut best = f32::INFINITY;
    for segment in points.windows(2) {
        let dist = segment_rect_gap(segment[0], segment[1], rect);
        best = best.min(dist);
        if best <= 1e-6 {
            return 0.0;
        }
    }
    best
}

pub fn segment_rect_gap(a: (f32, f32), b: (f32, f32), rect: &LabelRect) -> f32 {
    if segment_intersects_rect(a, b, rect) {
        return 0.0;
    }
    let mut best = point_rect_distance(a, rect).min(point_rect_distance(b, rect));
    let corners = [
        (rect.x, rect.y),
        (rect.x + rect.width, rect.y),
        (rect.x + rect.width, rect.y + rect.height),
        (rect.x, rect.y + rect.height),
    ];
    for corner in corners {
        best = best.min(point_segment_distance(corner, a, b));
    }
    best
}

pub fn point_rect_distance(point: (f32, f32), rect: &LabelRect) -> f32 {
    let (px, py) = point;
    let x1 = rect.x;
    let y1 = rect.y;
    let x2 = rect.x + rect.width;
    let y2 = rect.y + rect.height;
    let dx = if px < x1 {
        x1 - px
    } else if px > x2 {
        px - x2
    } else {
        0.0
    };
    let dy = if py < y1 {
        y1 - py
    } else if py > y2 {
        py - y2
    } else {
        0.0
    };
    (dx * dx + dy * dy).sqrt()
}

pub fn point_segment_distance(point: (f32, f32), a: (f32, f32), b: (f32, f32)) -> f32 {
    let ab_x = b.0 - a.0;
    let ab_y = b.1 - a.1;
    let len_sq = ab_x * ab_x + ab_y * ab_y;
    if len_sq <= 1e-9 {
        let dx = point.0 - a.0;
        let dy = point.1 - a.1;
        return (dx * dx + dy * dy).sqrt();
    }
    let t = ((point.0 - a.0) * ab_x + (point.1 - a.1) * ab_y) / len_sq;
    let t = t.clamp(0.0, 1.0);
    let proj_x = a.0 + ab_x * t;
    let proj_y = a.1 + ab_y * t;
    let dx = point.0 - proj_x;
    let dy = point.1 - proj_y;
    (dx * dx + dy * dy).sqrt()
}

pub fn segment_intersects_rect(a: (f32, f32), b: (f32, f32), rect: &LabelRect) -> bool {
    if point_in_rect(a, rect) || point_in_rect(b, rect) {
        return true;
    }
    let corners = [
        (rect.x, rect.y),
        (rect.x + rect.width, rect.y),
        (rect.x + rect.width, rect.y + rect.height),
        (rect.x, rect.y + rect.height),
    ];
    let edges = [
        (corners[0], corners[1]),
        (corners[1], corners[2]),
        (corners[2], corners[3]),
        (corners[3], corners[0]),
    ];
    edges
        .iter()
        .any(|(c0, c1)| segments_intersect(a, b, *c0, *c1))
}

pub fn point_in_rect(point: (f32, f32), rect: &LabelRect) -> bool {
    point.0 >= rect.x
        && point.0 <= rect.x + rect.width
        && point.1 >= rect.y
        && point.1 <= rect.y + rect.height
}

pub fn segments_intersect(a: (f32, f32), b: (f32, f32), c: (f32, f32), d: (f32, f32)) -> bool {
    let eps = 1e-6;
    let o1 = orient(a, b, c);
    let o2 = orient(a, b, d);
    let o3 = orient(c, d, a);
    let o4 = orient(c, d, b);

    if o1.abs() < eps && on_segment(a, b, c, eps) {
        return true;
    }
    if o2.abs() < eps && on_segment(a, b, d, eps) {
        return true;
    }
    if o3.abs() < eps && on_segment(c, d, a, eps) {
        return true;
    }
    if o4.abs() < eps && on_segment(c, d, b, eps) {
        return true;
    }

    (o1 > 0.0) != (o2 > 0.0) && (o3 > 0.0) != (o4 > 0.0)
}

pub fn orient(a: (f32, f32), b: (f32, f32), c: (f32, f32)) -> f32 {
    (b.0 - a.0) * (c.1 - a.1) - (b.1 - a.1) * (c.0 - a.0)
}

pub fn on_segment(a: (f32, f32), b: (f32, f32), c: (f32, f32), eps: f32) -> bool {
    c.0 >= a.0.min(b.0) - eps
        && c.0 <= a.0.max(b.0) + eps
        && c.1 >= a.1.min(b.1) - eps
        && c.1 <= a.1.max(b.1) + eps
}
