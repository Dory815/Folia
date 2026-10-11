//! Generic typed primitive conversion, never diagram placement or routing.
use windows::core::Interface;
use windows::Win32::Graphics::Direct2D::*;
use windows::Win32::Graphics::Direct2D::Common::*;
use windows_numerics::{Matrix3x2, Vector2};
use crate::scene::{Affine, FillRule, PathSegment, Point, Rect, ShapeGeometry};
use super::{win, NativeRenderError};

pub(super) fn number(value: f64, context: &str) -> Result<f32, NativeRenderError> {
    let result = value as f32;
    if !value.is_finite() || !result.is_finite() {
        return Err(NativeRenderError::invalid_number(context));
    }
    Ok(result)
}

pub(super) fn point(value: Point, context: &str) -> Result<Vector2, NativeRenderError> {
    Ok(Vector2 { X: number(value.x, &format!("{context}.x"))?,
        Y: number(value.y, &format!("{context}.y"))? })
}

pub(super) fn rect(value: Rect, context: &str) -> Result<D2D_RECT_F, NativeRenderError> {
    number(value.width, &format!("{context}.width"))?;
    number(value.height, &format!("{context}.height"))?;
    if value.width < 0.0 || value.height < 0.0 {
        return Err(NativeRenderError::invalid_resource(format!("{context}: negative size")));
    }
    Ok(D2D_RECT_F { left: number(value.x, &format!("{context}.left"))?,
        top: number(value.y, &format!("{context}.top"))?,
        right: number(value.x + value.width, &format!("{context}.right"))?,
        bottom: number(value.y + value.height, &format!("{context}.bottom"))? })
}

pub(super) fn matrix(value: Affine, context: &str) -> Result<Matrix3x2, NativeRenderError> {
    Ok(Matrix3x2 { M11: number(value.a, &format!("{context}.a"))?,
        M12: number(value.b, &format!("{context}.b"))?,
        M21: number(value.c, &format!("{context}.c"))?,
        M22: number(value.d, &format!("{context}.d"))?,
        M31: number(value.e, &format!("{context}.e"))?,
        M32: number(value.f, &format!("{context}.f"))? })
}

pub(super) fn affine(value: Matrix3x2) -> Affine {
    Affine { a: value.M11 as f64, b: value.M12 as f64,
        c: value.M21 as f64, d: value.M22 as f64, e: value.M31 as f64, f: value.M32 as f64 }
}

pub(super) fn transformed(factory: &ID2D1Factory, geometry: ID2D1Geometry,
    transform: Affine, context: &str) -> Result<ID2D1Geometry, NativeRenderError> {
    let transform = matrix(transform, context)?;
    let transformed = win(unsafe { factory.CreateTransformedGeometry(&geometry, &transform) }, context)?;
    win(transformed.cast(), "transformed geometry interface")
}

pub(super) fn bounds(geometry: &ID2D1Geometry, context: &str) -> Result<Rect, NativeRenderError> {
    let bounds = win(unsafe { geometry.GetBounds(None) }, &format!("{context}: GetBounds"))?;
    // D2D's empty-geometry sentinel is an inverted rectangle.
    if bounds.right < bounds.left || bounds.bottom < bounds.top {
        return Ok(Rect::default());
    }
    let result = Rect { x: bounds.left as f64, y: bounds.top as f64,
        width: bounds.right as f64 - bounds.left as f64,
        height: bounds.bottom as f64 - bounds.top as f64 };
    rect(result, context)?;
    Ok(result)
}

pub(super) fn convert(factory: &ID2D1Factory, shape: &ShapeGeometry,
    context: &str) -> Result<ID2D1Geometry, NativeRenderError> {
    match shape {
        ShapeGeometry::Rect { rect: bounds, rx, ry } => {
            let rectangle = rect(*bounds, context)?;
            number(*rx, &format!("{context}.rx"))?;
            number(*ry, &format!("{context}.ry"))?;
            if *rx < 0.0 || *ry < 0.0 {
                return Err(NativeRenderError::invalid_resource(format!("{context}: negative radius")));
            }
            if *rx == 0.0 || *ry == 0.0 {
                let geometry = win(unsafe { factory.CreateRectangleGeometry(&rectangle) }, context)?;
                win(geometry.cast(), "rectangle geometry interface")
            } else {
                let rounded = D2D1_ROUNDED_RECT { rect: rectangle,
                    radiusX: number(rx.min(bounds.width / 2.0), &format!("{context}.clamped_rx"))?,
                    radiusY: number(ry.min(bounds.height / 2.0), &format!("{context}.clamped_ry"))? };
                let geometry = win(unsafe { factory.CreateRoundedRectangleGeometry(&rounded) }, context)?;
                win(geometry.cast(), "rounded rectangle geometry interface")
            }
        }
        ShapeGeometry::Circle { center, radius } => ellipse(factory, *center, *radius, *radius, context),
        ShapeGeometry::Ellipse { center, rx, ry } => ellipse(factory, *center, *rx, *ry, context),
        ShapeGeometry::Line { start, end } => {
            make_path(factory, vec![NativeSegment::Move(point(*start, context)?),
                NativeSegment::Line(point(*end, context)?)], FillRule::NonZero, false, context)
        }
        ShapeGeometry::Polyline(points) | ShapeGeometry::Polygon(points) => {
            let mut segments = Vec::with_capacity(points.len() + 1);
            for (index, value) in points.iter().enumerate() {
                let value = point(*value, &format!("{context}.points[{index}]"))?;
                segments.push(if index == 0 { NativeSegment::Move(value) } else { NativeSegment::Line(value) });
            }
            if matches!(shape, ShapeGeometry::Polygon(_)) && !segments.is_empty() {
                segments.push(NativeSegment::Close);
            }
            make_path(factory, segments, FillRule::NonZero, true, context)
        }
        ShapeGeometry::Path(path) => {
            let mut segments = Vec::with_capacity(path.segments().len());
            for (index, segment) in path.segments().iter().enumerate() {
                let location = format!("{context}.segments[{index}]");
                segments.push(match segment {
                    PathSegment::MoveTo(value) => NativeSegment::Move(point(*value, &location)?),
                    PathSegment::LineTo(value) => NativeSegment::Line(point(*value, &location)?),
                    PathSegment::QuadraticTo { control, end } => NativeSegment::Quadratic(D2D1_QUADRATIC_BEZIER_SEGMENT {
                        point1: point(*control, &location)?, point2: point(*end, &location)? }),
                    PathSegment::CubicTo { control1, control2, end } => NativeSegment::Cubic(D2D1_BEZIER_SEGMENT {
                        point1: point(*control1, &location)?, point2: point(*control2, &location)?, point3: point(*end, &location)? }),
                    PathSegment::ArcTo { rx, ry, rotation_degrees, large_arc, sweep, end } => {
                        let rx = number(*rx, &format!("{location}.rx"))?;
                        let ry = number(*ry, &format!("{location}.ry"))?;
                        if rx < 0.0 || ry < 0.0 {
                            return Err(NativeRenderError::invalid_resource(format!("{location}: negative arc radius")));
                        }
                        NativeSegment::Arc(D2D1_ARC_SEGMENT {
                            point: point(*end, &location)?, size: D2D_SIZE_F { width: rx, height: ry },
                            rotationAngle: number(*rotation_degrees, &format!("{location}.rotation"))?,
                            sweepDirection: if *sweep { D2D1_SWEEP_DIRECTION_CLOCKWISE } else { D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE },
                            arcSize: if *large_arc { D2D1_ARC_SIZE_LARGE } else { D2D1_ARC_SIZE_SMALL },
                        })
                    }
                    PathSegment::Close => NativeSegment::Close,
                });
            }
            make_path(factory, segments, path.fill_rule(), true, context)
        }
    }
}

fn ellipse(factory: &ID2D1Factory, center: Point, rx: f64, ry: f64,
    context: &str) -> Result<ID2D1Geometry, NativeRenderError> {
    let ellipse = D2D1_ELLIPSE { point: point(center, context)?,
        radiusX: number(rx, &format!("{context}.rx"))?, radiusY: number(ry, &format!("{context}.ry"))? };
    if rx < 0.0 || ry < 0.0 { return Err(NativeRenderError::invalid_resource("negative ellipse radius")); }
    let geometry = win(unsafe { factory.CreateEllipseGeometry(&ellipse) }, context)?;
    win(geometry.cast(), "ellipse geometry interface")
}

enum NativeSegment {
    Move(Vector2), Line(Vector2), Quadratic(D2D1_QUADRATIC_BEZIER_SEGMENT),
    Cubic(D2D1_BEZIER_SEGMENT), Arc(D2D1_ARC_SEGMENT), Close,
}

fn make_path(factory: &ID2D1Factory, segments: Vec<NativeSegment>, fill: FillRule,
    filled: bool, context: &str) -> Result<ID2D1Geometry, NativeRenderError> {
    if !segments.is_empty() && !matches!(segments.first(), Some(NativeSegment::Move(_))) {
        return Err(NativeRenderError::invalid_resource(format!("{context}: path must begin with MoveTo")));
    }
    let path = win(unsafe { factory.CreatePathGeometry() }, &format!("{context}: CreatePathGeometry"))?;
    let sink = win(unsafe { path.Open() }, &format!("{context}: geometry Open"))?;
    let begin = if filled { D2D1_FIGURE_BEGIN_FILLED } else { D2D1_FIGURE_BEGIN_HOLLOW };
    let mut open = false;
    let mut current = Vector2::default();
    let mut start = Vector2::default();
    unsafe {
        sink.SetFillMode(match fill { FillRule::NonZero => D2D1_FILL_MODE_WINDING, FillRule::EvenOdd => D2D1_FILL_MODE_ALTERNATE });
        for segment in segments {
            match segment {
                NativeSegment::Move(value) => {
                    if open { sink.EndFigure(D2D1_FIGURE_END_OPEN); }
                    sink.BeginFigure(value, begin);
                    open = true; current = value; start = value;
                }
                NativeSegment::Close => {
                    if open { sink.EndFigure(D2D1_FIGURE_END_CLOSED); open = false; }
                    current = start;
                }
                segment => {
                    if !open { sink.BeginFigure(current, begin); start = current; open = true; }
                    match segment {
                        NativeSegment::Line(value) => { sink.AddLine(value); current = value; }
                        NativeSegment::Quadratic(value) => { sink.AddQuadraticBezier(&value); current = value.point2; }
                        NativeSegment::Cubic(value) => { sink.AddBezier(&value); current = value.point3; }
                        NativeSegment::Arc(value) => {
                            if value.point != current {
                                if value.size.width == 0.0 || value.size.height == 0.0 { sink.AddLine(value.point); }
                                else { sink.AddArc(&value); }
                            }
                            current = value.point;
                        }
                        NativeSegment::Move(_) | NativeSegment::Close => {
                            // Defensive exhaustiveness without a caller-data panic;
                            // normal dispatch handles these before this branch.
                            if open { sink.EndFigure(D2D1_FIGURE_END_OPEN); }
                            let mut error = NativeRenderError::invalid_resource(
                                format!("{context}: unexpected contour state"));
                            if let Err(cleanup) = win(sink.Close(), &format!("{context}: failed contour Close")) {
                                error.cleanup.push(cleanup);
                            }
                            return Err(error);
                        }
                    }
                }
            }
        }
        if open { sink.EndFigure(D2D1_FIGURE_END_OPEN); }
    }
    win(unsafe { sink.Close() }, &format!("{context}: geometry Close"))?;
    win(path.cast(), "path geometry interface")
}
