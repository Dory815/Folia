//! Root-frozen projection of tiny-skia's two-circle geometry responsibility.
use tiny_skia_path::Transform;

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum RadialKind {
    Radial { radius1: f32, radius2: f32 },
    Strip { scaled_r0: f32 },
    Focal { r1: f32, focal_x: f32, is_swapped: bool },
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum RadialGeometry {
    None,
    SolidFirstStop,
    Conical { kind: RadialKind, points_to_unit: Transform, thin_ring: bool },
}
