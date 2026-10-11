//! Consume positioned glyphs; extract actual retained font contours, never shape.
use std::collections::HashMap;
use ttf_parser::{Face, GlyphId, OutlineBuilder};
use windows::Win32::Graphics::Direct2D::*;
use windows_numerics::Matrix3x2;
use crate::scene::{Affine, DiagramScene, FillRule, GlyphAppearance, PaintStyle,
    Path, PathSegment, Point, Rect, ShapeGeometry, TextNode};
use super::{geometry, image, paint, NativeRenderError, TransformGuard};

enum RawDrawable {
    Shape { geometry: ID2D1Geometry, style: PaintStyle, local_transform: Option<Affine> },
    Image { drawing: image::PreparedImage },
}
enum Drawable {
    Shape { drawing: paint::PreparedShape, local_transform: Option<Affine> },
    Image(image::PreparedImage),
}
pub(super) struct PreparedText { drawables: Vec<Drawable>, opacity: f32 }

impl PreparedText {
    pub(super) fn draw(&self, context: &ID2D1DeviceContext) -> Result<(), NativeRenderError> {
        let _layer = paint::LayerGuard::push(context, self.opacity, None);
        for drawable in &self.drawables {
            match drawable {
                Drawable::Shape { drawing, local_transform } => {
                    let mut original = Matrix3x2::identity();
                    unsafe { context.GetTransform(&mut original); }
                    let _restore = TransformGuard { context, original };
                    if let Some(transform) = local_transform {
                        let combined = geometry::matrix(geometry::affine(original).compose(*transform),
                            "retained glyph layer transform")?;
                        unsafe { context.SetTransform(&combined); }
                    }
                    drawing.draw(context);
                }
                Drawable::Image(image) => image.draw(context)?,
            }
        }
        Ok(())
    }
}

pub(super) fn prepare(context: &ID2D1DeviceContext, factory: &ID2D1Factory,
    scene: &DiagramScene, text: &TextNode, images: &[ID2D1Bitmap1],
    parent_mapping: Affine, location: &str) -> Result<PreparedText, NativeRenderError> {
    geometry::point(text.origin, &format!("{location}.origin"))?;
    geometry::number(text.font.size, &format!("{location}.font.size"))?;
    geometry::number(text.font.stretch, &format!("{location}.font.stretch"))?;
    let opacity = geometry::number(text.style.opacity, &format!("{location}.opacity"))?;
    paint::preflight_style(&text.style, location)?;
    let mut raw = Vec::new();
    let mut full_bounds: Option<Rect> = None;
    // Scope is this preparation, so font/glyph indices cannot cross scenes.
    let mut outlines: HashMap<(usize, u16), Option<Path>> = HashMap::new();
    for (run_index, run) in text.runs.iter().enumerate() {
        let run_location = format!("{location}.runs[{run_index}]");
        if run.sideways { return Err(NativeRenderError::unsupported(format!("{run_location}: sideways glyph run"))); }
        geometry::point(run.origin, &format!("{run_location}.origin"))?;
        geometry::number(run.em_size, &format!("{run_location}.em_size"))?;
        let font = scene.font(run.font).ok_or_else(|| NativeRenderError::invalid_resource(
            format!("{run_location}: missing font {}", run.font.0)))?;
        let face = Face::parse(&font.bytes, font.face_index).map_err(|error| NativeRenderError::invalid_resource(
            format!("{run_location}: retained font face: {error:?}")))?;
        let scale = run.em_size / f64::from(face.units_per_em());
        geometry::number(scale, &format!("{run_location}.font_scale"))?;
        let mut advance = 0.0;
        for (glyph_index, glyph) in run.glyphs.iter().enumerate() {
            let glyph_location = format!("{run_location}.glyphs[{glyph_index}]");
            geometry::number(glyph.advance, &format!("{glyph_location}.advance"))?;
            geometry::point(glyph.offset, &format!("{glyph_location}.offset"))?;
            let origin = Point { x: run.origin.x + advance + glyph.offset.x,
                y: run.origin.y + glyph.offset.y };
            geometry::point(origin, &format!("{glyph_location}.final_origin"))?;
            let translation = Affine { e: origin.x, f: origin.y, ..Affine::IDENTITY };
            match &glyph.appearance {
                GlyphAppearance::FontOutline => {
                    if glyph.glyph_id >= face.number_of_glyphs() {
                        return Err(NativeRenderError::invalid_resource(format!("{glyph_location}: glyph id outside retained face")));
                    }
                    let id = GlyphId(glyph.glyph_id);
                    // A declared ordinary outline must not become a monochrome
                    // approximation of a retained color/bitmap/SVG glyph.
                    if face.is_color_glyph(id) || face.glyph_raster_image(id, u16::MAX).is_some()
                        || face.glyph_svg_image(id).is_some() {
                        return Err(NativeRenderError::unsupported(format!("{glyph_location}: color glyph requires retained Geometry/Image appearance")));
                    }
                    let key = (run.font.0, glyph.glyph_id);
                    if !outlines.contains_key(&key) {
                        let mut collector = Contours { segments: Vec::new() };
                        let outlined = face.outline_glyph(id, &mut collector);
                        outlines.insert(key, if outlined.is_some() {
                            Some(Path::new(collector.segments, FillRule::NonZero))
                        } else { None });
                    }
                    let outline = outlines.get(&key).and_then(Option::as_ref);
                    if let Some(outline) = outline {
                        let shape = geometry::convert(factory, &ShapeGeometry::Path(outline.clone()), &glyph_location)?;
                        let mapping = translation.compose(Affine { a: scale, d: -scale, ..Affine::IDENTITY });
                        geometry::matrix(parent_mapping.compose(mapping), &format!("{glyph_location}.completed_mapping"))?;
                        let shape = geometry::transformed(factory, shape, mapping, &glyph_location)?;
                        include_bounds(&mut full_bounds, geometry::bounds(&shape, &glyph_location)?)?;
                        raw.push(RawDrawable::Shape { geometry: shape,
                            style: PaintStyle { opacity: 1.0, ..text.style.clone() }, local_transform: None });
                    } else {
                        let cluster = text.text.get(glyph.cluster.clone()).ok_or_else(||
                            NativeRenderError::invalid_resource(format!("{glyph_location}: invalid UTF-8 cluster")))?;
                        if cluster.chars().any(|ch| !ch.is_whitespace() && !ch.is_control()) {
                            return Err(NativeRenderError::unsupported(format!("{glyph_location}: visible glyph has no supported outline")));
                        }
                    }
                }
                GlyphAppearance::Geometry(layers) => {
                    if layers.is_empty() { return Err(NativeRenderError::invalid_resource(format!("{glyph_location}: empty retained layers"))); }
                    for (layer_index, layer) in layers.iter().enumerate() {
                        let layer_location = format!("{glyph_location}.layers[{layer_index}]");
                        let shape = geometry::convert(factory, &layer.geometry, &layer_location)?;
                        let transform = translation.compose(layer.transform);
                        geometry::matrix(parent_mapping.compose(transform), &format!("{layer_location}.completed_mapping"))?;
                        let positioned = geometry::transformed(factory, shape.clone(), transform, &layer_location)?;
                        include_bounds(&mut full_bounds, geometry::bounds(&positioned, &layer_location)?)?;
                        raw.push(RawDrawable::Shape { geometry: shape, style: layer.style.clone(),
                            local_transform: Some(transform) });
                    }
                }
                GlyphAppearance::Image { image: id, destination, transform } => {
                    let destination = Rect { x: destination.x + origin.x, y: destination.y + origin.y, ..*destination };
                    geometry::rect(destination, &glyph_location)?;
                    geometry::matrix(parent_mapping.compose(translation.compose(*transform)),
                        &format!("{glyph_location}.completed_image_mapping"))?;
                    include_bounds(&mut full_bounds, destination)?;
                    let drawing = image::prepare(*id, destination, translation.compose(*transform), 1.0,
                        images, &glyph_location)?;
                    raw.push(RawDrawable::Image { drawing });
                }
            }
            advance += glyph.advance;
            geometry::number(advance, &format!("{glyph_location}.accumulated_advance"))?;
        }
    }
    let full_bounds = full_bounds.unwrap_or_default();
    let mut drawables = Vec::with_capacity(raw.len());
    for (index, drawable) in raw.into_iter().enumerate() {
        let drawable_location = format!("{location}.drawable[{index}]");
        drawables.push(match drawable {
            RawDrawable::Shape { geometry, style, local_transform } => Drawable::Shape {
                drawing: paint::prepare_shape(context, factory, scene, geometry, &style,
                    if local_transform.is_some() { None } else { Some(full_bounds) }, &drawable_location)?,
                local_transform,
            },
            RawDrawable::Image { drawing } => Drawable::Image(drawing),
        });
    }
    Ok(PreparedText { drawables, opacity })
}

fn include_bounds(bounds: &mut Option<Rect>, value: Rect) -> Result<(), NativeRenderError> {
    let merged = match *bounds {
        None => value,
        Some(previous) => {
            let x = previous.x.min(value.x); let y = previous.y.min(value.y);
            Rect { x, y, width: (previous.x + previous.width).max(value.x + value.width) - x,
                height: (previous.y + previous.height).max(value.y + value.height) - y }
        }
    };
    geometry::rect(merged, "full text drawable bounds")?;
    *bounds = Some(merged);
    Ok(())
}

struct Contours { segments: Vec<PathSegment> }
fn font_point(x: f32, y: f32) -> Point { Point { x: f64::from(x), y: f64::from(y) } }
impl OutlineBuilder for Contours {
    fn move_to(&mut self, x: f32, y: f32) { self.segments.push(PathSegment::MoveTo(font_point(x, y))); }
    fn line_to(&mut self, x: f32, y: f32) { self.segments.push(PathSegment::LineTo(font_point(x, y))); }
    fn quad_to(&mut self, x1: f32, y1: f32, x: f32, y: f32) {
        self.segments.push(PathSegment::QuadraticTo { control: font_point(x1, y1), end: font_point(x, y) });
    }
    fn curve_to(&mut self, x1: f32, y1: f32, x2: f32, y2: f32, x: f32, y: f32) {
        self.segments.push(PathSegment::CubicTo { control1: font_point(x1, y1),
            control2: font_point(x2, y2), end: font_point(x, y) });
    }
    fn close(&mut self) { self.segments.push(PathSegment::Close); }
}
