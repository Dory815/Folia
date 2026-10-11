//! Fully resolved paints. Color/style parsing is outside this module.
use super::geometry::{Affine, Point};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct GradientId(pub usize);

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Rgba {
    pub r: f64,
    pub g: f64,
    pub b: f64,
    pub a: f64,
}

#[derive(Debug, Clone, PartialEq, Default)]
pub enum Paint {
    #[default]
    None,
    Solid(Rgba),
    LinearGradient(GradientId),
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum GradientUnits { UserSpace, ObjectBoundingBox }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SpreadMode { Pad, Reflect, Repeat }

#[derive(Debug, Clone, PartialEq)]
pub struct GradientStop {
    pub offset: f64,
    pub color: Rgba,
    pub opacity: f64,
}

#[derive(Debug, Clone, PartialEq)]
pub struct LinearGradient {
    pub units: GradientUnits,
    pub transform: Affine,
    pub start: Point,
    pub end: Point,
    pub stops: Vec<GradientStop>,
    pub spread: SpreadMode,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineCap { Butt, Round, Square }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineJoin { Miter, Round, Bevel }

#[derive(Debug, Clone, PartialEq)]
pub struct Stroke {
    pub paint: Paint,
    pub width: f64,
    pub dash: Vec<f64>,
    pub dash_offset: f64,
    pub cap: LineCap,
    pub join: LineJoin,
    pub miter_limit: f64,
}

#[derive(Debug, Clone, PartialEq)]
pub struct PaintStyle {
    pub fill: Paint,
    pub stroke: Option<Stroke>,
    pub fill_opacity: f64,
    pub stroke_opacity: f64,
    pub opacity: f64,
}

impl Default for PaintStyle {
    fn default() -> Self {
        Self { fill: Paint::None, stroke: None, fill_opacity: 1.0,
            stroke_opacity: 1.0, opacity: 1.0 }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum BlendMode {
    #[default]
    Normal,
    Multiply,
    Screen,
    Overlay,
    Darken,
    Lighten,
    ColorDodge,
    ColorBurn,
    HardLight,
    SoftLight,
    Difference,
    Exclusion,
    Hue,
    Saturation,
    Color,
    Luminosity,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct LayerStyle {
    pub opacity: f64,
    pub blend: BlendMode,
}

impl Default for LayerStyle {
    fn default() -> Self { Self { opacity: 1.0, blend: BlendMode::Normal } }
}
