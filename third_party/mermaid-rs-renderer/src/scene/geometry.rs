//! Portable numeric geometry. Matrices use SVG's column-vector convention.

#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Point {
    pub x: f64,
    pub y: f64,
}

#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Rect {
    pub x: f64,
    pub y: f64,
    pub width: f64,
    pub height: f64,
}

/// x' = a*x + c*y + e; y' = b*x + d*y + f.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Affine {
    pub a: f64,
    pub b: f64,
    pub c: f64,
    pub d: f64,
    pub e: f64,
    pub f: f64,
}

impl Affine {
    pub const IDENTITY: Self = Self {
        a: 1.0, b: 0.0, c: 0.0, d: 1.0, e: 0.0, f: 0.0,
    };

    pub fn transform_point(self, point: Point) -> Point {
        Point {
            x: self.a * point.x + self.c * point.y + self.e,
            y: self.b * point.x + self.d * point.y + self.f,
        }
    }

    /// Returns self * inner: apply inner first, then self.
    pub fn compose(self, inner: Self) -> Self {
        Self {
            a: self.a * inner.a + self.c * inner.b,
            b: self.b * inner.a + self.d * inner.b,
            c: self.a * inner.c + self.c * inner.d,
            d: self.b * inner.c + self.d * inner.d,
            e: self.a * inner.e + self.c * inner.f + self.e,
            f: self.b * inner.e + self.d * inner.f + self.f,
        }
    }
}

impl Default for Affine {
    fn default() -> Self { Self::IDENTITY }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum FillRule {
    #[default]
    NonZero,
    EvenOdd,
}

#[derive(Debug, Clone, PartialEq)]
pub enum PathSegment {
    MoveTo(Point),
    LineTo(Point),
    QuadraticTo { control: Point, end: Point },
    CubicTo { control1: Point, control2: Point, end: Point },
    ArcTo {
        rx: f64,
        ry: f64,
        rotation_degrees: f64,
        large_arc: bool,
        sweep: bool,
        end: Point,
    },
    Close,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Path {
    segments: Vec<PathSegment>,
    fill_rule: FillRule,
}

impl Path {
    pub fn new(segments: Vec<PathSegment>, fill_rule: FillRule) -> Self {
        Self { segments, fill_rule }
    }
    pub fn segments(&self) -> &[PathSegment] { &self.segments }
    pub fn fill_rule(&self) -> FillRule { self.fill_rule }
}

#[derive(Debug, Clone, PartialEq)]
pub enum ShapeGeometry {
    Rect { rect: Rect, rx: f64, ry: f64 },
    Circle { center: Point, radius: f64 },
    Ellipse { center: Point, rx: f64, ry: f64 },
    Line { start: Point, end: Point },
    Polyline(Vec<Point>),
    Polygon(Vec<Point>),
    Path(Path),
}

#[derive(Debug, Clone, PartialEq)]
pub struct Clip {
    pub geometry: ShapeGeometry,
    pub transform: Affine,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AspectAlign {
    XMinYMin, XMidYMin, XMaxYMin,
    XMinYMid, XMidYMid, XMaxYMid,
    XMinYMax, XMidYMax, XMaxYMax,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AspectRatioPolicy {
    None,
    Meet(AspectAlign),
    Slice(AspectAlign),
}
