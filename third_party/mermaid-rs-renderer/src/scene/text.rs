//! Positioned text and retained resources. No font lookup or shaping happens here.
use std::ops::Range;
use std::sync::Arc;

use super::geometry::{Affine, AspectRatioPolicy, Point, Rect, ShapeGeometry};
use super::paint::PaintStyle;
use super::ExportMetadata;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct FontId(pub usize);

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct ImageId(pub usize);

#[derive(Debug, Clone, PartialEq)]
pub struct FontResource {
    pub bytes: Arc<[u8]>,
    pub face_index: u32,
    pub identity: String,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ImageMime { Png, Jpeg, Gif, WebP }

#[derive(Debug, Clone, PartialEq)]
pub struct ImageResource {
    pub encoded: Arc<[u8]>,
    pub mime: ImageMime,
    pub pixel_width: u32,
    pub pixel_height: u32,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FontStyle { Normal, Italic, Oblique }

#[derive(Debug, Clone, PartialEq)]
pub struct FontSpec {
    pub families: Vec<String>,
    pub size: f64,
    pub weight: u16,
    /// CSS percentage, 100 for normal width.
    pub stretch: f64,
    pub style: FontStyle,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TextAnchor { Start, Middle, End }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TextBaseline { Alphabetic, Middle, Central, Hanging, Mathematical }

#[derive(Debug, Clone, PartialEq)]
pub struct GlyphLayer {
    pub geometry: ShapeGeometry,
    pub transform: Affine,
    pub style: PaintStyle,
}

/// Ordinary font outlines and retained color/fallback appearances are explicit.
/// Geometry/image placement is already resolved in the enclosing run's space.
#[derive(Debug, Clone, PartialEq)]
pub enum GlyphAppearance {
    FontOutline,
    Geometry(Vec<GlyphLayer>),
    Image { image: ImageId, destination: Rect, transform: Affine },
}

#[derive(Debug, Clone, PartialEq)]
pub struct PositionedGlyph {
    pub glyph_id: u16,
    pub advance: f64,
    pub offset: Point,
    /// UTF-8 bytes in TextNode::text; boundaries may overlap for a cluster.
    pub cluster: Range<usize>,
    pub appearance: GlyphAppearance,
}

#[derive(Debug, Clone, PartialEq)]
pub struct GlyphRun {
    pub font: FontId,
    /// Final baseline origin; the backend must not apply an anchor adjustment.
    pub origin: Point,
    pub em_size: f64,
    pub glyphs: Vec<PositionedGlyph>,
    pub source_range: Range<usize>,
    pub bidi_level: u32,
    pub sideways: bool,
}

#[derive(Debug, Clone, PartialEq)]
pub struct TextNode {
    pub text: String,
    pub font: FontSpec,
    /// Resolved text origin; anchor/baseline also retain export semantics.
    pub origin: Point,
    pub baseline: TextBaseline,
    pub anchor: TextAnchor,
    pub runs: Vec<GlyphRun>,
    pub style: PaintStyle,
    pub metadata: ExportMetadata,
}

#[derive(Debug, Clone, PartialEq)]
pub struct ImageNode {
    pub image: ImageId,
    pub destination: Rect,
    /// Completed intrinsic-pixel -> scene transform, not a request to fit again.
    pub source_to_destination: Affine,
    pub aspect_ratio: AspectRatioPolicy,
    pub style: PaintStyle,
    pub metadata: ExportMetadata,
}
