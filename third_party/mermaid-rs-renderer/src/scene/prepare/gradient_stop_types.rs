//! Root-frozen f32 stop-preparation values; shared by both later consumers.
use tiny_skia_path::{NormalizedF32, Transform};
use crate::scene::paint::SpreadMode;

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct StopColor { pub(crate) components: [NormalizedF32; 4] }

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct PreparedGradientStop {
    pub(crate) position: NormalizedF32,
    pub(crate) color: StopColor,
}

#[derive(Debug, Clone, PartialEq)]
pub struct PreparedGradient {
    pub(crate) stops: Vec<PreparedGradientStop>,
    pub(crate) spread: SpreadMode,
    pub(crate) transform: Transform,
    pub(crate) points_to_unit: Transform,
    pub(crate) colors_are_opaque: bool,
    pub(crate) has_uniform_stops: bool,
}

/// Normalized premultiplied components; constructed only through StopColor.
#[derive(Debug,Clone,Copy,PartialEq)]
pub struct PremultipliedStopColor{pub(crate) components:[NormalizedF32;4]}
impl PremultipliedStopColor{pub fn components(&self)->[f32;4]{self.components.map(|v|v.get())}}
