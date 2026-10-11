//! Construction and final structural validation; no diagram layout algorithms.
use std::collections::HashMap;
use std::ops::Range;
use ttf_parser::Face;

use super::*;

/// Top-level nodes have depth 1. This bounds accepted consumer recursion too.
pub const MAX_SCENE_DEPTH: usize = 128;

pub struct SceneBuilder {
    dimensions: SceneDimensions,
    viewport: RootViewport,
    nodes: Vec<SceneNode>,
    gradients: Vec<LinearGradient>,
    fonts: Vec<FontResource>,
    images: Vec<ImageResource>,
    metadata: ExportMetadata,
}

impl SceneBuilder {
    /// Both the policy and its completed transform are supplied by compilation.
    pub fn new(dimensions: SceneDimensions, viewport: RootViewport) -> Self {
        Self { dimensions, viewport, nodes: Vec::new(), gradients: Vec::new(),
            fonts: Vec::new(), images: Vec::new(), metadata: ExportMetadata::default() }
    }

    pub fn push_node(&mut self, node: SceneNode) { self.nodes.push(node); }
    pub fn set_metadata(&mut self, metadata: ExportMetadata) { self.metadata = metadata; }
    pub fn add_gradient(&mut self, gradient: LinearGradient) -> GradientId {
        let id = GradientId(self.gradients.len());
        self.gradients.push(gradient);
        id
    }
    pub fn add_font(&mut self, font: FontResource) -> FontId {
        let id = FontId(self.fonts.len());
        self.fonts.push(font);
        id
    }
    pub fn add_image(&mut self, image: ImageResource) -> ImageId {
        let id = ImageId(self.images.len());
        self.images.push(image);
        id
    }

    /// On failure self is disposed iteratively; no partially valid scene escapes.
    pub fn finish(mut self) -> Result<DiagramScene, SceneError> {
        self.validate()?;
        Ok(DiagramScene {
            dimensions: self.dimensions,
            viewport: self.viewport,
            nodes: std::mem::take(&mut self.nodes),
            gradients: std::mem::take(&mut self.gradients),
            fonts: std::mem::take(&mut self.fonts),
            images: std::mem::take(&mut self.images),
            metadata: std::mem::take(&mut self.metadata),
        })
    }

    fn validate(&self) -> Result<(), SceneError> {
        positive(self.dimensions.width, "dimensions.width")?;
        positive(self.dimensions.height, "dimensions.height")?;
        rect(self.dimensions.viewbox, "dimensions.viewbox")?;
        positive(self.dimensions.viewbox.width, "dimensions.viewbox.width")?;
        positive(self.dimensions.viewbox.height, "dimensions.viewbox.height")?;
        rect(self.viewport.clip, "viewport.clip")?;
        affine(self.viewport.viewbox_to_viewport, "viewport.transform")?;
        metadata(&self.metadata, "scene.metadata")?;

        for (index, gradient) in self.gradients.iter().enumerate() {
            let context = format!("gradients[{index}]");
            affine(gradient.transform, &format!("{context}.transform"))?;
            point(gradient.start, &format!("{context}.start"))?;
            point(gradient.end, &format!("{context}.end"))?;
            let mut previous = 0.0;
            for (stop_index, stop) in gradient.stops.iter().enumerate() {
                let location = format!("{context}.stops[{stop_index}]");
                unit(stop.offset, &format!("{location}.offset"))?;
                rgba(stop.color, &format!("{location}.color"))?;
                unit(stop.opacity, &format!("{location}.opacity"))?;
                if stop.offset < previous {
                    return Err(SceneError::InvalidPaint {
                        context: format!("{location}.offset is before the previous stop"),
                    });
                }
                previous = stop.offset;
            }
        }

        let mut identities: HashMap<&str, (&[u8], u32)> = HashMap::new();
        let mut font_glyph_counts = Vec::with_capacity(self.fonts.len());
        for (index, font) in self.fonts.iter().enumerate() {
            if font.bytes.is_empty() || font.identity.trim().is_empty() {
                return Err(SceneError::InvalidResource {
                    context: format!("fonts[{index}] requires bytes and a stable identity"),
                });
            }
            let face = Face::parse(&font.bytes, font.face_index).map_err(|error| {
                SceneError::InvalidResource {
                    context: format!("fonts[{index}] has invalid font data/face index: {error:?}"),
                }
            })?;
            font_glyph_counts.push(face.number_of_glyphs());
            if let Some((bytes, face)) = identities.get(font.identity.as_str()) {
                if *bytes != font.bytes.as_ref() || *face != font.face_index {
                    return Err(SceneError::InvalidResource {
                        context: format!("fonts[{index}] identity refers to different font data"),
                    });
                }
            } else {
                identities.insert(&font.identity, (&font.bytes, font.face_index));
            }
        }
        for (index, image) in self.images.iter().enumerate() {
            if image.encoded.is_empty() || image.pixel_width == 0 || image.pixel_height == 0 {
                return Err(SceneError::InvalidResource {
                    context: format!("images[{index}] requires encoded bytes and nonzero intrinsic dimensions"),
                });
            }
        }

        // References, transforms, clips and depth are checked without recursion.
        let mut pending = Vec::new();
        for (index, node) in self.nodes.iter().enumerate().rev() {
            pending.push((node, 1usize, format!("nodes[{index}]"),
                self.viewport.viewbox_to_viewport, true));
        }
        while let Some((node, depth, context, parent_transform, visible)) = pending.pop() {
            if depth > MAX_SCENE_DEPTH {
                return Err(SceneError::UnbalancedState {
                    context: format!("{context} exceeds depth budget {MAX_SCENE_DEPTH}"),
                });
            }
            match node {
                SceneNode::Shape(shape) => {
                    geometry(&shape.geometry, &context)?;
                    self.style(&shape.style, &context)?;
                    metadata(&shape.metadata, &format!("{context}.metadata"))?;
                }
                SceneNode::Text(text) => self.text(text, &context, visible, &font_glyph_counts)?,
                SceneNode::Image(image) => {
                    self.image_ref(image.image, &context)?;
                    rect(image.destination, &format!("{context}.destination"))?;
                    affine(image.source_to_destination, &format!("{context}.image_mapping"))?;
                    affine(parent_transform.compose(image.source_to_destination),
                        &format!("{context}.composed_image_mapping"))?;
                    self.style(&image.style, &context)?;
                    metadata(&image.metadata, &format!("{context}.metadata"))?;
                }
                SceneNode::Group(group) => {
                    affine(group.transform, &format!("{context}.transform"))?;
                    let transform = parent_transform.compose(group.transform);
                    affine(transform, &format!("{context}.composed_transform"))?;
                    unit(group.layer.opacity, &format!("{context}.layer.opacity"))?;
                    metadata(&group.metadata, &format!("{context}.metadata"))?;
                    if let Some(clip) = &group.clip {
                        geometry(&clip.geometry, &format!("{context}.clip"))?;
                        affine(clip.transform, &format!("{context}.clip.transform"))?;
                        affine(transform.compose(clip.transform),
                            &format!("{context}.clip.composed_transform"))?;
                    }
                    for (index, child) in group.children.iter().enumerate().rev() {
                        pending.push((child, depth + 1,
                            format!("{context}.children[{index}]"), transform,
                            visible && group.layer.opacity > 0.0));
                    }
                }
            }
        }
        Ok(())
    }

    fn paint(&self, paint: &Paint, context: &str) -> Result<(), SceneError> {
        match paint {
            Paint::None => Ok(()),
            Paint::Solid(color) => rgba(*color, context),
            Paint::LinearGradient(id) => {
                if self.gradients.get(id.0).is_none() {
                    return Err(SceneError::InvalidResource {
                        context: format!("{context} references missing gradient {}", id.0),
                    });
                }
                Ok(())
            }
        }
    }

    fn style(&self, style: &PaintStyle, context: &str) -> Result<(), SceneError> {
        self.paint(&style.fill, &format!("{context}.fill"))?;
        unit(style.fill_opacity, &format!("{context}.fill_opacity"))?;
        unit(style.stroke_opacity, &format!("{context}.stroke_opacity"))?;
        unit(style.opacity, &format!("{context}.opacity"))?;
        if let Some(stroke) = &style.stroke {
            self.paint(&stroke.paint, &format!("{context}.stroke.paint"))?;
            paint_nonnegative(stroke.width, &format!("{context}.stroke.width"))?;
            finite(stroke.dash_offset, &format!("{context}.stroke.dash_offset"))?;
            finite(stroke.miter_limit, &format!("{context}.stroke.miter_limit"))?;
            if stroke.miter_limit < 1.0 {
                return Err(SceneError::InvalidPaint {
                    context: format!("{context}.stroke.miter_limit must be at least 1"),
                });
            }
            for (index, dash) in stroke.dash.iter().enumerate() {
                paint_nonnegative(*dash, &format!("{context}.stroke.dash[{index}]"))?;
            }
        }
        Ok(())
    }

    fn image_ref(&self, id: ImageId, context: &str) -> Result<(), SceneError> {
        if self.images.get(id.0).is_none() {
            return Err(SceneError::InvalidResource {
                context: format!("{context} references missing image {}", id.0),
            });
        }
        Ok(())
    }

    fn paint_visible(&self, paint: &Paint) -> bool {
        match paint {
            Paint::None => false,
            Paint::Solid(color) => color.a > 0.0,
            Paint::LinearGradient(id) => self.gradients.get(id.0).is_some_and(|gradient| {
                gradient.stops.iter().any(|stop| stop.color.a > 0.0 && stop.opacity > 0.0)
            }),
        }
    }

    fn text(&self, text: &TextNode, context: &str, ancestor_visible: bool,
        font_glyph_counts: &[u16]) -> Result<(), SceneError> {
        self.style(&text.style, context)?;
        metadata(&text.metadata, &format!("{context}.metadata"))?;
        point(text.origin, &format!("{context}.origin"))?;
        positive(text.font.size, &format!("{context}.font.size"))?;
        positive(text.font.stretch, &format!("{context}.font.stretch"))?;
        if text.font.families.is_empty() || text.font.families.iter().any(|family| family.trim().is_empty())
            || !(1..=1000).contains(&text.font.weight) {
            return Err(SceneError::InvalidResource {
                context: format!("{context}.font requires nonempty families and weight 1..=1000"),
            });
        }
        let visible = ancestor_visible && text.style.opacity > 0.0
            && ((text.style.fill_opacity > 0.0 && self.paint_visible(&text.style.fill))
                || (text.style.stroke_opacity > 0.0 && text.style.stroke.as_ref()
                    .is_some_and(|stroke| stroke.width > 0.0 && self.paint_visible(&stroke.paint))));
        if text.text.chars().any(visible_scalar) && visible
            && !text.runs.iter().any(|run| !run.glyphs.is_empty()) {
            return Err(SceneError::UnresolvedText {
                context: format!("{context} has visible text without positioned glyphs"),
            });
        }
        let mut covered_runs = Vec::new();
        let mut covered_clusters = Vec::new();
        for (index, run) in text.runs.iter().enumerate() {
            let location = format!("{context}.runs[{index}]");
            if self.fonts.get(run.font.0).is_none() {
                return Err(SceneError::InvalidResource {
                    context: format!("{location} references missing font {}", run.font.0),
                });
            }
            point(run.origin, &format!("{location}.origin"))?;
            positive(run.em_size, &format!("{location}.em_size"))?;
            source_range(&text.text, &run.source_range, &format!("{location}.source_range"))?;
            if run.glyphs.is_empty() {
                if visible && text.text[run.source_range.clone()].chars().any(visible_scalar) {
                    return Err(SceneError::UnresolvedText {
                        context: format!("{location} covers visible source but has no positioned glyphs"),
                    });
                }
            } else {
                covered_runs.push(run.source_range.clone());
            }
            for (glyph_index, glyph) in run.glyphs.iter().enumerate() {
                let glyph_location = format!("{location}.glyphs[{glyph_index}]");
                finite(glyph.advance, &format!("{glyph_location}.advance"))?;
                point(glyph.offset, &format!("{glyph_location}.offset"))?;
                source_range(&text.text, &glyph.cluster, &format!("{glyph_location}.cluster"))?;
                if !text.text.is_empty() && glyph.cluster.is_empty() {
                    return Err(SceneError::InvalidResource {
                        context: format!("{glyph_location}.cluster is empty in a nonempty text source"),
                    });
                }
                if glyph.cluster.start < run.source_range.start || glyph.cluster.end > run.source_range.end {
                    return Err(SceneError::InvalidResource {
                        context: format!("{glyph_location}.cluster is outside its run"),
                    });
                }
                covered_clusters.push(glyph.cluster.clone());
                match &glyph.appearance {
                    GlyphAppearance::FontOutline => {
                        if !font_glyph_counts.get(run.font.0)
                            .is_some_and(|count| glyph.glyph_id < *count) {
                            return Err(SceneError::InvalidResource {
                                context: format!("{glyph_location}.glyph_id is outside its font face"),
                            });
                        }
                    }
                    GlyphAppearance::Geometry(layers) => {
                        if layers.is_empty() {
                            return Err(SceneError::InvalidResource {
                                context: format!("{glyph_location} has no retained geometry layers"),
                            });
                        }
                        for (layer_index, layer) in layers.iter().enumerate() {
                            let layer_location = format!("{glyph_location}.layers[{layer_index}]");
                            geometry(&layer.geometry, &layer_location)?;
                            affine(layer.transform, &format!("{layer_location}.transform"))?;
                            self.style(&layer.style, &layer_location)?;
                        }
                    }
                    GlyphAppearance::Image { image, destination, transform } => {
                        self.image_ref(*image, &glyph_location)?;
                        rect(*destination, &format!("{glyph_location}.destination"))?;
                        affine(*transform, &format!("{glyph_location}.transform"))?;
                    }
                }
            }
        }
        if visible {
            require_visible_coverage(&text.text, &mut covered_runs,
                &format!("{context}.run source coverage"))?;
            require_visible_coverage(&text.text, &mut covered_clusters,
                &format!("{context}.glyph cluster coverage"))?;
        }
        Ok(())
    }
}

impl Drop for SceneBuilder {
    fn drop(&mut self) { drop_nodes_iteratively(std::mem::take(&mut self.nodes)); }
}

pub(super) fn drop_nodes_iteratively(mut pending: Vec<SceneNode>) {
    while let Some(node) = pending.pop() {
        if let SceneNode::Group(mut group) = node {
            pending.append(&mut group.children);
        }
    }
}

fn finite(value: f64, context: &str) -> Result<(), SceneError> {
    if !value.is_finite() {
        return Err(SceneError::InvalidNumber { context: context.to_owned() });
    }
    Ok(())
}

fn nonnegative(value: f64, context: &str) -> Result<(), SceneError> {
    finite(value, context)?;
    if value < 0.0 {
        return Err(SceneError::InvalidGeometry { context: format!("{context} must be nonnegative") });
    }
    Ok(())
}

fn positive(value: f64, context: &str) -> Result<(), SceneError> {
    finite(value, context)?;
    if value <= 0.0 {
        return Err(SceneError::InvalidGeometry { context: format!("{context} must be positive") });
    }
    Ok(())
}

fn paint_nonnegative(value: f64, context: &str) -> Result<(), SceneError> {
    finite(value, context)?;
    if value < 0.0 {
        return Err(SceneError::InvalidPaint { context: format!("{context} must be nonnegative") });
    }
    Ok(())
}

fn unit(value: f64, context: &str) -> Result<(), SceneError> {
    finite(value, context)?;
    if !(0.0..=1.0).contains(&value) {
        return Err(SceneError::InvalidPaint { context: format!("{context} must be in 0..=1") });
    }
    Ok(())
}

fn point(value: Point, context: &str) -> Result<(), SceneError> {
    finite(value.x, &format!("{context}.x"))?;
    finite(value.y, &format!("{context}.y"))
}

fn rect(value: Rect, context: &str) -> Result<(), SceneError> {
    finite(value.x, &format!("{context}.x"))?;
    finite(value.y, &format!("{context}.y"))?;
    nonnegative(value.width, &format!("{context}.width"))?;
    nonnegative(value.height, &format!("{context}.height"))?;
    finite(value.x + value.width, &format!("{context}.right"))?;
    finite(value.y + value.height, &format!("{context}.bottom"))
}

fn affine(value: Affine, context: &str) -> Result<(), SceneError> {
    for (name, coordinate) in [("a", value.a), ("b", value.b), ("c", value.c),
        ("d", value.d), ("e", value.e), ("f", value.f)] {
        finite(coordinate, &format!("{context}.{name}"))?;
    }
    Ok(())
}

fn rgba(value: Rgba, context: &str) -> Result<(), SceneError> {
    for (name, component) in [("r", value.r), ("g", value.g), ("b", value.b), ("a", value.a)] {
        unit(component, &format!("{context}.{name}"))?;
    }
    Ok(())
}

fn source_range(text: &str, range: &Range<usize>, context: &str) -> Result<(), SceneError> {
    if range.start > range.end || range.end > text.len()
        || !text.is_char_boundary(range.start) || !text.is_char_boundary(range.end) {
        return Err(SceneError::InvalidResource {
            context: format!("{context} is not an ordered UTF-8 boundary range"),
        });
    }
    Ok(())
}

// Whitespace and Unicode control scalars may have no visible glyph. Formatting
// scalars such as ZWJ are not silently exempt: their shaped cluster must retain
// the source range. This is coverage validation, not shaping or grapheme lookup.
fn visible_scalar(ch: char) -> bool { !ch.is_whitespace() && !ch.is_control() }

fn require_visible_coverage(text: &str, ranges: &mut [Range<usize>],
    context: &str) -> Result<(), SceneError> {
    ranges.sort_by_key(|range| (range.start, range.end));
    let mut merged: Vec<Range<usize>> = Vec::new();
    for range in ranges.iter() {
        if let Some(previous) = merged.last_mut() {
            if range.start <= previous.end {
                previous.end = previous.end.max(range.end);
                continue;
            }
        }
        merged.push(range.clone());
    }
    let mut range_index = 0;
    for (start, ch) in text.char_indices() {
        if !visible_scalar(ch) { continue; }
        let end = start + ch.len_utf8();
        while merged.get(range_index).is_some_and(|range| range.end <= start) {
            range_index += 1;
        }
        if !merged.get(range_index).is_some_and(|range| range.start <= start && range.end >= end) {
            return Err(SceneError::UnresolvedText {
                context: format!("{context} omits visible UTF-8 source at {start}..{end}"),
            });
        }
    }
    Ok(())
}

fn metadata(value: &ExportMetadata, context: &str) -> Result<(), SceneError> {
    for (index, attribute) in value.data_attributes.iter().enumerate() {
        if attribute.name.is_empty() || !attribute.name.chars()
            .all(|ch| ch.is_ascii_alphanumeric() || matches!(ch, '-' | '_' | '.')) {
            return Err(SceneError::UnsupportedFeature {
                context: format!("{context}.data_attributes[{index}] has an unsupported data- suffix"),
            });
        }
    }
    Ok(())
}

fn geometry(value: &ShapeGeometry, context: &str) -> Result<(), SceneError> {
    match value {
        ShapeGeometry::Rect { rect: bounds, rx, ry } => {
            rect(*bounds, context)?;
            nonnegative(*rx, &format!("{context}.rx"))?;
            nonnegative(*ry, &format!("{context}.ry"))?;
        }
        ShapeGeometry::Circle { center, radius } => {
            point(*center, &format!("{context}.center"))?;
            nonnegative(*radius, &format!("{context}.radius"))?;
        }
        ShapeGeometry::Ellipse { center, rx, ry } => {
            point(*center, &format!("{context}.center"))?;
            nonnegative(*rx, &format!("{context}.rx"))?;
            nonnegative(*ry, &format!("{context}.ry"))?;
        }
        ShapeGeometry::Line { start, end } => {
            point(*start, &format!("{context}.start"))?;
            point(*end, &format!("{context}.end"))?;
        }
        ShapeGeometry::Polyline(points) | ShapeGeometry::Polygon(points) => {
            for (index, coordinate) in points.iter().enumerate() {
                point(*coordinate, &format!("{context}.points[{index}]"))?;
            }
        }
        ShapeGeometry::Path(path) => {
            let mut has_subpath = false;
            for (index, segment) in path.segments().iter().enumerate() {
                let location = format!("{context}.segments[{index}]");
                if !has_subpath && !matches!(segment, PathSegment::MoveTo(_)) {
                    return Err(SceneError::InvalidGeometry {
                        context: format!("{location} must start with MoveTo"),
                    });
                }
                match segment {
                    PathSegment::MoveTo(end) => { point(*end, &location)?; has_subpath = true; }
                    PathSegment::LineTo(end) => point(*end, &location)?,
                    PathSegment::QuadraticTo { control, end } => {
                        point(*control, &format!("{location}.control"))?;
                        point(*end, &format!("{location}.end"))?;
                    }
                    PathSegment::CubicTo { control1, control2, end } => {
                        point(*control1, &format!("{location}.control1"))?;
                        point(*control2, &format!("{location}.control2"))?;
                        point(*end, &format!("{location}.end"))?;
                    }
                    PathSegment::ArcTo { rx, ry, rotation_degrees, end, .. } => {
                        nonnegative(*rx, &format!("{location}.rx"))?;
                        nonnegative(*ry, &format!("{location}.ry"))?;
                        finite(*rotation_degrees, &format!("{location}.rotation"))?;
                        point(*end, &format!("{location}.end"))?;
                    }
                    PathSegment::Close => {},
                }
            }
        }
    }
    Ok(())
}
