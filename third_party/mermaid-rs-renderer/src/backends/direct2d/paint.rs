//! Native brushes, exact stroke units, isolated layers and blend composition.
use std::mem::ManuallyDrop;
use windows::core::Interface;
use windows::Win32::Graphics::Direct2D::*;
use windows::Win32::Graphics::Direct2D::Common::*;
use windows_numerics::Matrix3x2;
use crate::scene::{Affine, BlendMode, DiagramScene, GradientUnits, LineCap, LineJoin,
    Paint, PaintStyle, Rect, Rgba, SpreadMode, Stroke};
use super::{draw_image, geometry, record, win, NativeRenderError};

pub(super) fn preflight_resources(scene: &DiagramScene) -> Result<(), NativeRenderError> {
    for (index, gradient) in scene.gradients().iter().enumerate() {
        let location = format!("gradients[{index}]");
        geometry::point(gradient.start, &format!("{location}.start"))?;
        geometry::point(gradient.end, &format!("{location}.end"))?;
        geometry::matrix(gradient.transform, &format!("{location}.transform"))?;
        for (index, stop) in gradient.stops.iter().enumerate() {
            geometry::number(stop.offset, &format!("{location}.stops[{index}].offset"))?;
            geometry::number(stop.opacity, &format!("{location}.stops[{index}].opacity"))?;
            color(stop.color, 1.0, &format!("{location}.stops[{index}].color"))?;
        }
    }
    Ok(())
}

pub(super) fn preflight_style(style: &PaintStyle, location: &str) -> Result<(), NativeRenderError> {
    geometry::number(style.opacity, &format!("{location}.opacity"))?;
    geometry::number(style.fill_opacity, &format!("{location}.fill_opacity"))?;
    geometry::number(style.stroke_opacity, &format!("{location}.stroke_opacity"))?;
    if let Paint::Solid(value) = &style.fill { color(*value, 1.0, location)?; }
    if let Some(stroke) = &style.stroke {
        geometry::number(stroke.width, &format!("{location}.stroke.width"))?;
        geometry::number(stroke.dash_offset, &format!("{location}.stroke.dash_offset"))?;
        geometry::number(stroke.miter_limit, &format!("{location}.stroke.miter_limit"))?;
        for (index, dash) in stroke.dash.iter().enumerate() {
            geometry::number(*dash, &format!("{location}.stroke.dash[{index}]"))?;
        }
        if let Paint::Solid(value) = &stroke.paint { color(*value, 1.0, location)?; }
    }
    Ok(())
}

pub(super) struct PreparedShape {
    geometry: ID2D1Geometry,
    fill: Option<ID2D1Brush>,
    stroke: Option<(ID2D1Brush, f32, ID2D1StrokeStyle)>,
    opacity: f32,
}

impl PreparedShape {
    pub(super) fn draw(&self, context: &ID2D1DeviceContext) {
        let _layer = LayerGuard::push(context, self.opacity, None);
        unsafe {
            if let Some(fill) = &self.fill { context.FillGeometry(&self.geometry, fill, None::<&ID2D1Brush>); }
            if let Some((brush, width, style)) = &self.stroke {
                context.DrawGeometry(&self.geometry, brush, *width, style);
            }
        }
    }
}

pub(super) fn prepare_shape(context: &ID2D1DeviceContext, factory: &ID2D1Factory,
    scene: &DiagramScene, geometry: ID2D1Geometry, style: &PaintStyle,
    full_drawable_bounds: Option<Rect>, location: &str) -> Result<PreparedShape, NativeRenderError> {
    preflight_style(style, location)?;
    let bounds = match full_drawable_bounds {
        Some(bounds) => bounds,
        None => geometry::bounds(&geometry, location)?,
    };
    let opacity = geometry::number(style.opacity, &format!("{location}.opacity"))?;
    let fill = brush(context, scene, &style.fill, style.fill_opacity, bounds, &format!("{location}.fill"))?;
    let stroke = match &style.stroke {
        Some(stroke) => {
            let native_style = stroke_style(factory, stroke, &format!("{location}.stroke"))?;
            let width = geometry::number(stroke.width, &format!("{location}.stroke.width"))?;
            let brush = brush(context, scene, &stroke.paint, style.stroke_opacity, bounds,
                &format!("{location}.stroke.paint"))?;
            match (brush, native_style) {
                (Some(brush), Some(native_style)) if width > 0.0 => Some((brush, width, native_style)),
                _ => None,
            }
        }
        None => None,
    };
    Ok(PreparedShape { geometry, fill, stroke, opacity })
}

fn color(value: Rgba, opacity: f64, location: &str) -> Result<D2D1_COLOR_F, NativeRenderError> {
    Ok(D2D1_COLOR_F { r: geometry::number(value.r, location)?,
        g: geometry::number(value.g, location)?, b: geometry::number(value.b, location)?,
        a: geometry::number(value.a * opacity, location)? })
}

fn solid(context: &ID2D1DeviceContext, value: Rgba, opacity: f64,
    location: &str) -> Result<Option<ID2D1Brush>, NativeRenderError> {
    let color = color(value, 1.0, location)?;
    let properties = D2D1_BRUSH_PROPERTIES { opacity: geometry::number(opacity, location)?,
        transform: Matrix3x2::identity() };
    let brush = win(unsafe { context.CreateSolidColorBrush(&color, Some(&properties)) }, location)?;
    Ok(Some(win(brush.cast(), "solid brush interface")?))
}

fn brush(context: &ID2D1DeviceContext, scene: &DiagramScene, paint: &Paint,
    opacity: f64, bounds: Rect, location: &str) -> Result<Option<ID2D1Brush>, NativeRenderError> {
    geometry::number(opacity, location)?;
    match paint {
        Paint::None => Ok(None),
        Paint::Solid(value) => solid(context, *value, opacity, location),
        Paint::LinearGradient(id) => {
            let gradient = scene.gradient(*id).ok_or_else(|| NativeRenderError::invalid_resource(
                format!("{location}: missing gradient {}", id.0)))?;
            let start = geometry::point(gradient.start, &format!("{location}.start"))?;
            let end = geometry::point(gradient.end, &format!("{location}.end"))?;
            geometry::matrix(gradient.transform, &format!("{location}.transform"))?;
            if gradient.stops.is_empty() { return Ok(None); }
            // SVG objectBoundingBox paint is disabled for a zero-width/height box.
            if gradient.units == GradientUnits::ObjectBoundingBox && (bounds.width == 0.0 || bounds.height == 0.0) {
                return Ok(None);
            }
            let last = &gradient.stops[gradient.stops.len() - 1];
            if gradient.stops.len() == 1 || gradient.start == gradient.end {
                return solid(context, Rgba { a: last.color.a * last.opacity, ..last.color }, opacity, location);
            }
            if gradient.stops.len() > u32::MAX as usize {
                return Err(NativeRenderError::invalid_resource(format!("{location}: too many gradient stops")));
            }
            let mut stops = Vec::with_capacity(gradient.stops.len());
            for (index, stop) in gradient.stops.iter().enumerate() {
                stops.push(D2D1_GRADIENT_STOP { position: geometry::number(stop.offset,
                    &format!("{location}.stops[{index}].offset"))?,
                    color: color(stop.color, stop.opacity, &format!("{location}.stops[{index}].color"))? });
            }
            let extend = match gradient.spread { SpreadMode::Pad => D2D1_EXTEND_MODE_CLAMP,
                SpreadMode::Reflect => D2D1_EXTEND_MODE_MIRROR, SpreadMode::Repeat => D2D1_EXTEND_MODE_WRAP };
            let collection = win(unsafe { context.CreateGradientStopCollection(&stops,
                D2D1_COLOR_SPACE_SRGB, D2D1_COLOR_SPACE_SRGB, D2D1_BUFFER_PRECISION_8BPC_UNORM,
                extend, D2D1_COLOR_INTERPOLATION_MODE_STRAIGHT) }, &format!("{location}: gradient stops"))?;
            let collection: ID2D1GradientStopCollection = win(collection.cast(), "gradient stop interface")?;
            let mapping = match gradient.units {
                GradientUnits::UserSpace => gradient.transform,
                GradientUnits::ObjectBoundingBox => Affine { a: bounds.width, b: 0.0, c: 0.0,
                    d: bounds.height, e: bounds.x, f: bounds.y }.compose(gradient.transform),
            };
            let properties = D2D1_BRUSH_PROPERTIES { opacity: geometry::number(opacity, location)?,
                transform: geometry::matrix(mapping, &format!("{location}.mapping"))? };
            let linear = D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES { startPoint: start, endPoint: end };
            let brush = win(unsafe { context.CreateLinearGradientBrush(&linear, Some(&properties), &collection) }, location)?;
            Ok(Some(win(brush.cast(), "gradient brush interface")?))
        }
    }
}

fn stroke_style(factory: &ID2D1Factory, stroke: &Stroke,
    location: &str) -> Result<Option<ID2D1StrokeStyle>, NativeRenderError> {
    let width = geometry::number(stroke.width, &format!("{location}.width"))?;
    geometry::number(stroke.dash_offset, &format!("{location}.dash_offset"))?;
    let miter_limit = geometry::number(stroke.miter_limit, &format!("{location}.miter_limit"))?;
    for (index, dash) in stroke.dash.iter().enumerate() {
        geometry::number(*dash, &format!("{location}.dash[{index}]"))?;
    }
    if width <= 0.0 { return Ok(None); }
    let patterned = stroke.dash.iter().any(|dash| *dash > 0.0);
    let mut dashes = Vec::new();
    if patterned {
        let count = stroke.dash.len().checked_mul(if stroke.dash.len() % 2 == 1 { 2 } else { 1 })
            .ok_or_else(|| NativeRenderError::invalid_resource("dash pattern length overflow"))?;
        if count > u32::MAX as usize { return Err(NativeRenderError::invalid_resource("too many dashes")); }
        for (index, value) in stroke.dash.iter().cycle().take(count).enumerate() {
            dashes.push(geometry::number(*value / stroke.width, &format!("{location}.dash_units[{index}]"))?);
        }
    }
    let cap = match stroke.cap { LineCap::Butt => D2D1_CAP_STYLE_FLAT,
        LineCap::Round => D2D1_CAP_STYLE_ROUND, LineCap::Square => D2D1_CAP_STYLE_SQUARE };
    let properties = D2D1_STROKE_STYLE_PROPERTIES { startCap: cap, endCap: cap, dashCap: cap,
        // SVG miter is D2D's miter-or-bevel behavior at the miter limit.
        lineJoin: match stroke.join { LineJoin::Miter => D2D1_LINE_JOIN_MITER_OR_BEVEL,
            LineJoin::Round => D2D1_LINE_JOIN_ROUND, LineJoin::Bevel => D2D1_LINE_JOIN_BEVEL },
        miterLimit: miter_limit, dashStyle: if patterned { D2D1_DASH_STYLE_CUSTOM } else { D2D1_DASH_STYLE_SOLID },
        dashOffset: if patterned { geometry::number(stroke.dash_offset / stroke.width,
            &format!("{location}.dash_offset_units"))? } else { 0.0 } };
    Ok(Some(win(unsafe { factory.CreateStrokeStyle(&properties,
        if patterned { Some(dashes.as_slice()) } else { None }) }, location)?))
}

pub(super) struct LayerGuard<'a> { context: &'a ID2D1DeviceContext }
impl<'a> LayerGuard<'a> {
    pub(super) fn push(context: &'a ID2D1DeviceContext, opacity: f32,
        clip: Option<&ID2D1Geometry>) -> Self {
        let mut parameters = D2D1_LAYER_PARAMETERS1 {
            contentBounds: D2D_RECT_F { left: -f32::MAX, top: -f32::MAX, right: f32::MAX, bottom: f32::MAX },
            geometricMask: ManuallyDrop::new(clip.cloned()),
            maskAntialiasMode: D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
            maskTransform: Matrix3x2::identity(), opacity,
            opacityBrush: ManuallyDrop::new(None), layerOptions: D2D1_LAYER_OPTIONS1_NONE,
        };
        unsafe {
            context.PushLayer(&parameters, None::<&ID2D1Layer>);
            // The API borrows the mask; release our temporary reference exactly once.
            ManuallyDrop::drop(&mut parameters.geometricMask);
        }
        Self { context }
    }
}
impl Drop for LayerGuard<'_> {
    fn drop(&mut self) { unsafe { self.context.PopLayer(); } }
}

pub(super) fn blend(context: &ID2D1DeviceContext, background: &ID2D1Image,
    foreground: &ID2D1Image, mode: BlendMode, location: &str) -> Result<ID2D1Image, NativeRenderError> {
    let mode = match mode {
        BlendMode::Normal => return record(context, location, |context| {
            draw_image(context, background);
            draw_image(context, foreground);
            Ok(())
        }),
        BlendMode::Multiply => D2D1_BLEND_MODE_MULTIPLY,
        BlendMode::Screen => D2D1_BLEND_MODE_SCREEN,
        BlendMode::Overlay => D2D1_BLEND_MODE_OVERLAY,
        BlendMode::Darken => D2D1_BLEND_MODE_DARKEN,
        BlendMode::Lighten => D2D1_BLEND_MODE_LIGHTEN,
        BlendMode::ColorDodge => D2D1_BLEND_MODE_COLOR_DODGE,
        BlendMode::ColorBurn => D2D1_BLEND_MODE_COLOR_BURN,
        BlendMode::HardLight => D2D1_BLEND_MODE_HARD_LIGHT,
        BlendMode::SoftLight => D2D1_BLEND_MODE_SOFT_LIGHT,
        BlendMode::Difference => D2D1_BLEND_MODE_DIFFERENCE,
        BlendMode::Exclusion => D2D1_BLEND_MODE_EXCLUSION,
        BlendMode::Hue | BlendMode::Saturation | BlendMode::Color | BlendMode::Luminosity =>
            return super::source_blend::blend(context, background, foreground, mode, location),
    };
    let effect = win(unsafe { context.CreateEffect(&CLSID_D2D1Blend) }, &format!("{location}: Blend effect"))?;
    let mode_bytes = mode.0.to_ne_bytes();
    win(unsafe { effect.SetValue(D2D1_BLEND_PROP_MODE.0 as u32, D2D1_PROPERTY_TYPE_ENUM, &mode_bytes) },
        &format!("{location}: Blend mode property"))?;
    unsafe { effect.SetInput(0, background, true); effect.SetInput(1, foreground, true); }
    win(unsafe { effect.GetOutput() }, &format!("{location}: Blend output"))
}
