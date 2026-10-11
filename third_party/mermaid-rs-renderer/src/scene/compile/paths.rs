//! Typed extraction of the pinned renderer's polyline presentation helpers.
//!
//! Arithmetic stays f32 until the corresponding source emission site. Only a
//! scalar is formatted/parsed to reproduce its three-decimal quantization;
//! no SVG, XML, or path command string is produced or parsed.

use crate::scene::{FillRule, Path, PathSegment, Point, SceneError};

/// Deduplicate against the last retained point, using the source's strict epsilon.
/// Retained coordinates remain the original f32 values (including signed zero).
pub fn dedupe_points(points: &[(f32, f32)]) -> Result<Vec<(f32, f32)>, SceneError> {
    for (index, point) in points.iter().copied().enumerate() {
        finite_pair(point, &format!("dedupe_points.input[{index}]"))?;
    }
    let mut out = Vec::with_capacity(points.len());
    for point in points.iter().copied() {
        if out
            .last()
            .is_some_and(|prev: &(f32, f32)| {
                (prev.0 - point.0).abs() < 1e-3 && (prev.1 - point.1).abs() < 1e-3
            })
        {
            continue;
        }
        out.push(point);
    }
    Ok(out)
}

pub fn points_to_path(points: &[(f32, f32)]) -> Result<Path, SceneError> {
    let deduped = dedupe_points(points)?;
    if deduped.is_empty() {
        return Ok(Path::new(Vec::new(), FillRule::NonZero));
    }
    let mut segments = Vec::with_capacity(deduped.len());
    segments.push(PathSegment::MoveTo(quantized_point(
        deduped[0], "points_to_path.move",
    )?));
    if deduped.len() == 1 {
        return Ok(Path::new(segments, FillRule::NonZero));
    }
    for (index, point) in deduped.iter().copied().enumerate().skip(1) {
        segments.push(PathSegment::LineTo(quantized_point(
            point, &format!("points_to_path.line[{index}]"),
        )?));
    }
    Ok(Path::new(segments, FillRule::NonZero))
}

/// Trim and curve source-approved interior corners; endpoints remain straight.
pub fn rounded_polyline_path(
    points: &[(f32, f32)], radius: f32,
) -> Result<Path, SceneError> {
    finite(radius, "rounded_polyline_path.radius")?;
    let pts = dedupe_points(points)?;
    if pts.len() <= 2 || radius <= 0.0 {
        // Preserve the original second dedupe in its straight-path fallback.
        return points_to_path(&pts);
    }

    let mut segments = Vec::new();
    segments.push(PathSegment::MoveTo(quantized_point(
        pts[0], "rounded_polyline_path.move",
    )?));
    for idx in 1..pts.len() - 1 {
        let context = format!("rounded_polyline_path.corner[{idx}]");
        let prev = pts[idx - 1];
        let cur = pts[idx];
        let next = pts[idx + 1];
        let in_vec = (cur.0 - prev.0, cur.1 - prev.1);
        let out_vec = (next.0 - cur.0, next.1 - cur.1);
        let in_len = in_vec.0.hypot(in_vec.1);
        let out_len = out_vec.0.hypot(out_vec.1);
        if in_len <= 1e-3 || out_len <= 1e-3 {
            continue;
        }

        let in_unit = (in_vec.0 / in_len, in_vec.1 / in_len);
        let out_unit = (out_vec.0 / out_len, out_vec.1 / out_len);
        let cross = in_unit.0 * out_unit.1 - in_unit.1 * out_unit.0;
        let dot = in_unit.0 * out_unit.0 + in_unit.1 * out_unit.1;

        if cross.abs() <= 1e-3 || dot < -0.95 {
            segments.push(PathSegment::LineTo(quantized_point(
                cur, &format!("{context}.straight"),
            )?));
            continue;
        }

        let trim = radius.min(in_len * 0.45).min(out_len * 0.45);
        if trim <= 0.5 {
            segments.push(PathSegment::LineTo(quantized_point(
                cur, &format!("{context}.short_trim"),
            )?));
            continue;
        }

        let before = (cur.0 - in_unit.0 * trim, cur.1 - in_unit.1 * trim);
        let after = (cur.0 + out_unit.0 * trim, cur.1 + out_unit.1 * trim);
        segments.push(PathSegment::LineTo(quantized_point(
            before, &format!("{context}.before"),
        )?));
        segments.push(PathSegment::QuadraticTo {
            control: quantized_point(cur, &format!("{context}.control"))?,
            end: quantized_point(after, &format!("{context}.after"))?,
        });
    }

    let last = pts[pts.len() - 1];
    segments.push(PathSegment::LineTo(quantized_point(
        last, "rounded_polyline_path.last",
    )?));
    Ok(Path::new(segments, FillRule::NonZero))
}

/// Preserve the original open uniform cubic basis formulas and command order.
pub fn basis_curve_path(points: &[(f32, f32)]) -> Result<Path, SceneError> {
    let pts = dedupe_points(points)?;
    let n = pts.len();
    if n == 0 {
        return Ok(Path::new(Vec::new(), FillRule::NonZero));
    }
    let mut segments = Vec::new();
    segments.push(PathSegment::MoveTo(quantized_point(
        pts[0], "basis_curve_path.move",
    )?));
    if n == 1 {
        return Ok(Path::new(segments, FillRule::NonZero));
    }
    if n == 2 {
        segments.push(PathSegment::LineTo(quantized_point(
            pts[1], "basis_curve_path.two_point_line",
        )?));
        return Ok(Path::new(segments, FillRule::NonZero));
    }
    let p0 = pts[0];
    let p1 = pts[1];
    segments.push(PathSegment::LineTo(quantized_point(
        ((5.0 * p0.0 + p1.0) / 6.0, (5.0 * p0.1 + p1.1) / 6.0),
        "basis_curve_path.initial_line",
    )?));
    let mut x0 = p0.0;
    let mut y0 = p0.1;
    let mut x1 = p1.0;
    let mut y1 = p1.1;
    for i in 2..n {
        let (x, y) = pts[i];
        let context = format!("basis_curve_path.interior[{i}]");
        segments.push(PathSegment::CubicTo {
            control1: quantized_point(
                ((2.0 * x0 + x1) / 3.0, (2.0 * y0 + y1) / 3.0),
                &format!("{context}.control1"),
            )?,
            control2: quantized_point(
                ((x0 + 2.0 * x1) / 3.0, (y0 + 2.0 * y1) / 3.0),
                &format!("{context}.control2"),
            )?,
            end: quantized_point(
                ((x0 + 4.0 * x1 + x) / 6.0, (y0 + 4.0 * y1 + y) / 6.0),
                &format!("{context}.end"),
            )?,
        });
        x0 = x1;
        y0 = y1;
        x1 = x;
        y1 = y;
    }
    // Repeat the final point for the source's closing cubic, then line to it.
    segments.push(PathSegment::CubicTo {
        control1: quantized_point(
            ((2.0 * x0 + x1) / 3.0, (2.0 * y0 + y1) / 3.0),
            "basis_curve_path.final.control1",
        )?,
        control2: quantized_point(
            ((x0 + 2.0 * x1) / 3.0, (y0 + 2.0 * y1) / 3.0),
            "basis_curve_path.final.control2",
        )?,
        end: quantized_point(
            ((x0 + 5.0 * x1) / 6.0, (y0 + 5.0 * y1) / 6.0),
            "basis_curve_path.final.end",
        )?,
    });
    segments.push(PathSegment::LineTo(quantized_point(
        (x1, y1), "basis_curve_path.last",
    )?));
    Ok(Path::new(segments, FillRule::NonZero))
}

fn finite(value: f32, context: &str) -> Result<(), SceneError> {
    if !value.is_finite() {
        return Err(SceneError::InvalidNumber { context: context.to_owned() });
    }
    Ok(())
}

fn finite_pair(value: (f32, f32), context: &str) -> Result<(), SceneError> {
    finite(value.0, &format!("{context}.x"))?;
    finite(value.1, &format!("{context}.y"))
}

fn quantized_point(value: (f32, f32), context: &str) -> Result<Point, SceneError> {
    Ok(Point {
        x: quantized_coordinate(value.0, &format!("{context}.x"))?,
        y: quantized_coordinate(value.1, &format!("{context}.y"))?,
    })
}

fn quantized_coordinate(value: f32, context: &str) -> Result<f64, SceneError> {
    finite(value, context)?;
    let coordinate = format!("{value:.3}").parse::<f64>().map_err(|error| {
        SceneError::InvalidNumber { context: format!("{context}: numeric quantization failed: {error}") }
    })?;
    if !coordinate.is_finite() {
        return Err(SceneError::InvalidNumber {
            context: format!("{context}: nonfinite quantized coordinate"),
        });
    }
    Ok(coordinate)
}
