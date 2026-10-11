//! Typed extraction of the pinned renderer's complete ordinary node shapes.
//!
//! Numeric presentation stays in f32 until each source scalar emission site.
//! Paints and dashes are resolved by the shared caller; metadata belongs there.

use crate::config::LayoutConfig;
use crate::ir::NodeShape;
use crate::layout::NodeLayout;
use crate::scene::{
    Affine, ExportMetadata, FillRule, GroupNode, LayerStyle, LineCap, LineJoin,
    Paint, PaintStyle, Path, PathSegment, Point, Rect, SceneError, SceneNode,
    ShapeGeometry, ShapeNode, Stroke,
};

#[derive(Debug, Clone, PartialEq)]
pub struct NodeShapePaints {
    pub fill: Paint,
    pub stroke: Paint,
    pub background: Paint,
    pub primary_text: Paint,
    pub primary_border: Paint,
    pub line: Paint,
    pub divider_line: Paint,
}

/// Compile all shape_svg branches, including its supported default rectangle.
pub fn node_shape(
    node: &NodeLayout,
    config: &LayoutConfig,
    paints: &NodeShapePaints,
    dash: &[f64],
) -> Result<SceneNode, SceneError> {
    let x = node.x;
    let y = node.y;
    let w = node.width;
    let h = node.height;
    finite(x, "node_shape.x")?;
    finite(y, "node_shape.y")?;
    finite(w, "node_shape.width")?;
    finite(h, "node_shape.height")?;
    for (index, value) in dash.iter().copied().enumerate() {
        if !value.is_finite() || value < 0.0 {
            return Err(SceneError::InvalidPaint {
                context: format!("node_shape.dash[{index}] must be finite and nonnegative"),
            });
        }
    }
    let raw_width = node.style.stroke_width.unwrap_or(1.0);
    let width = display_width(raw_width, "node_shape.stroke_width")?;
    let ordinary = style(&paints.fill, &paints.stroke, width, dash, true, 1.0);
    let mut children = Vec::new();
    match node.shape {
        NodeShape::Rectangle | NodeShape::ActorBox => {
            children.push(shape(rectangle(x, y, w, h, 3.0, 3.0)?, &ordinary));
        }
        NodeShape::ForkJoin => {
            children.push(shape(rectangle(x, y, w, h, 2.0, 2.0)?, &ordinary));
        }
        NodeShape::Diamond => {
            let cx = x + w / 2.0;
            let cy = y + h / 2.0;
            children.push(shape(polygon(&[(cx, y), (x + w, cy), (cx, y + h), (x, cy)])?,
                &ordinary));
        }
        NodeShape::Circle | NodeShape::DoubleCircle => {
            let label_empty = node.label.lines.iter().all(|line| line.trim().is_empty());
            let is_state_start = node.id.starts_with("__start_");
            let is_state_end = node.id.starts_with("__end_");
            let (fill, stroke) = if is_state_start {
                (&paints.line, &paints.line)
            } else if is_state_end {
                (&paints.primary_border, &paints.primary_border)
            } else if label_empty {
                if node.shape == NodeShape::Circle {
                    (&paints.primary_text, &paints.primary_text)
                } else {
                    (&paints.primary_border, &paints.background)
                }
            } else {
                (&paints.fill, &paints.stroke)
            };
            let cx = x + w / 2.0;
            let cy = y + h / 2.0;
            let r = w.min(h) / 2.0;
            children.push(shape(circle(cx, cy, r)?,
                &style(fill, stroke, width, dash, true, 1.0)));
            if node.shape == NodeShape::DoubleCircle {
                let r2 = r - 4.0;
                // The predicate is evaluated before source radius quantization.
                if r2 > 0.0 {
                    let inner_fill = if label_empty || is_state_end {
                        paints.background.clone()
                    } else {
                        Paint::None
                    };
                    let inner_stroke = if label_empty || is_state_end {
                        &paints.background
                    } else {
                        stroke
                    };
                    let inner_width = if label_empty || is_state_end { 1.2 } else { 1.0 };
                    children.push(shape(circle(cx, cy, r2)?,
                        &style(&inner_fill, inner_stroke,
                            display_width(inner_width, "node_shape.inner_circle.stroke_width")?,
                            &[], true, 1.0)));
                }
            }
        }
        NodeShape::Stadium => {
            children.push(shape(rectangle(x, y, w, h, h / 2.0, h / 2.0)?, &ordinary));
        }
        NodeShape::RoundRect => {
            children.push(shape(rectangle(x, y, w, h, 10.0, 10.0)?, &ordinary));
        }
        NodeShape::Cylinder => {
            let cx = x + w / 2.0;
            let ry = (h * 0.12).clamp(6.0, 14.0);
            let rx = w / 2.0;
            children.push(shape(ellipse(cx, y + ry, rx, ry)?, &ordinary));
            children.push(shape(rectangle(x, y + ry, w, (h - 2.0 * ry).max(0.0),
                0.0, 0.0)?, &ordinary));
            children.push(shape(ellipse(cx, y + h - ry, rx, ry)?,
                &style(&Paint::None, &paints.stroke, width, dash, true, 1.0)));
        }
        NodeShape::Subroutine => {
            let inset = 6.0;
            children.push(shape(rectangle(x, y, w, h, 6.0, 6.0)?, &ordinary));
            let y1 = y + 2.0;
            let y2 = y + h - 2.0;
            let x1 = x + inset;
            let x2 = x + w - inset;
            let line_style = style(&Paint::None, &paints.stroke, width, &[], true, 1.0);
            children.push(shape(line((x1, y1), (x1, y2))?, &line_style));
            children.push(shape(line((x2, y1), (x2, y2))?, &line_style));
        }
        NodeShape::Hexagon => {
            let x1 = x + w * 0.25;
            let x2 = x + w * 0.75;
            let y_mid = y + h / 2.0;
            children.push(shape(polygon(&[
                (x1, y), (x2, y), (x + w, y_mid),
                (x2, y + h), (x1, y + h), (x, y_mid),
            ])?, &ordinary));
        }
        NodeShape::Parallelogram | NodeShape::ParallelogramAlt => {
            let offset = w * 0.18;
            let points = if node.shape == NodeShape::Parallelogram {
                [(x + offset, y), (x + w, y), (x + w - offset, y + h), (x, y + h)]
            } else {
                [(x, y), (x + w - offset, y), (x + w, y + h), (x + offset, y + h)]
            };
            children.push(shape(polygon(&points)?, &ordinary));
        }
        NodeShape::Trapezoid | NodeShape::TrapezoidAlt => {
            let offset = w * 0.18;
            let points = if node.shape == NodeShape::Trapezoid {
                [(x + offset, y), (x + w - offset, y), (x + w, y + h), (x, y + h)]
            } else {
                [(x, y), (x + w, y), (x + w - offset, y + h), (x + offset, y + h)]
            };
            children.push(shape(polygon(&points)?, &ordinary));
        }
        NodeShape::Asymmetric => {
            let slant = w * 0.22;
            children.push(shape(polygon(&[
                (x, y), (x + w - slant, y), (x + w, y + h / 2.0),
                (x + w - slant, y + h), (x, y + h),
            ])?, &ordinary));
        }
        NodeShape::MindmapDefault => {
            finite(config.mindmap.default_corner_radius, "node_shape.mindmap.corner_radius")?;
            finite(config.mindmap.divider_line_width, "node_shape.mindmap.divider_line_width")?;
            let rd = config.mindmap.default_corner_radius.max(0.0);
            let inner_h = (h - 2.0 * rd).max(0.0);
            let inner_w = (w - 2.0 * rd).max(0.0);
            children.push(shape(mindmap_path(x, y + h - rd, rd, inner_h, inner_w)?,
                &ordinary));
            let line_y = y + h - raw_width.max(0.8);
            let line_width = two_decimals(config.mindmap.divider_line_width,
                "node_shape.mindmap.divider_line_width")?;
            nonnegative_paint(line_width, "node_shape.mindmap.divider_line_width")?;
            children.push(shape(line((x, line_y), (x + w, line_y))?,
                &style(&Paint::None, &paints.divider_line, line_width, &[], false, 0.35)));
        }
        _ => {
            // The pinned source intentionally uses this rectangle for Text and
            // any remaining NodeShape values; it is not an unsupported branch.
            children.push(shape(rectangle(x, y, w, h, 6.0, 6.0)?, &ordinary));
        }
    }
    Ok(SceneNode::Group(GroupNode {
        children, transform: Affine::IDENTITY, clip: None,
        layer: LayerStyle::default(), metadata: ExportMetadata::default(),
    }))
}

fn style(
    fill: &Paint, stroke: &Paint, width: f64, dash: &[f64],
    round: bool, stroke_opacity: f64,
) -> PaintStyle {
    PaintStyle {
        fill: fill.clone(),
        stroke: Some(Stroke {
            paint: stroke.clone(), width, dash: dash.to_vec(), dash_offset: 0.0,
            cap: if round { LineCap::Round } else { LineCap::Butt },
            join: if round { LineJoin::Round } else { LineJoin::Miter },
            miter_limit: 4.0,
        }),
        fill_opacity: 1.0, stroke_opacity, opacity: 1.0,
    }
}

fn shape(geometry: ShapeGeometry, style: &PaintStyle) -> SceneNode {
    SceneNode::Shape(ShapeNode {
        geometry, style: style.clone(), metadata: ExportMetadata::default(),
    })
}

fn rectangle(x: f32, y: f32, w: f32, h: f32, rx: f32, ry: f32)
    -> Result<ShapeGeometry, SceneError> {
    Ok(ShapeGeometry::Rect {
        rect: Rect {
            x: two_decimals(x, "node_shape.rect.x")?,
            y: two_decimals(y, "node_shape.rect.y")?,
            width: size(w, "node_shape.rect.width")?,
            height: size(h, "node_shape.rect.height")?,
        },
        rx: size(rx, "node_shape.rect.rx")?,
        ry: size(ry, "node_shape.rect.ry")?,
    })
}

fn circle(cx: f32, cy: f32, radius: f32) -> Result<ShapeGeometry, SceneError> {
    Ok(ShapeGeometry::Circle {
        center: quantized_point((cx, cy), "node_shape.circle.center")?,
        radius: size(radius, "node_shape.circle.radius")?,
    })
}

fn ellipse(cx: f32, cy: f32, rx: f32, ry: f32) -> Result<ShapeGeometry, SceneError> {
    Ok(ShapeGeometry::Ellipse {
        center: quantized_point((cx, cy), "node_shape.ellipse.center")?,
        rx: size(rx, "node_shape.ellipse.rx")?,
        ry: size(ry, "node_shape.ellipse.ry")?,
    })
}

fn line(start: (f32, f32), end: (f32, f32)) -> Result<ShapeGeometry, SceneError> {
    Ok(ShapeGeometry::Line {
        start: quantized_point(start, "node_shape.line.start")?,
        end: quantized_point(end, "node_shape.line.end")?,
    })
}

fn polygon(points: &[(f32, f32)]) -> Result<ShapeGeometry, SceneError> {
    let mut out = Vec::with_capacity(points.len());
    for (index, point) in points.iter().copied().enumerate() {
        out.push(quantized_point(point, &format!("node_shape.polygon[{index}]"))?);
    }
    Ok(ShapeGeometry::Polygon(out))
}

fn mindmap_path(x: f32, start_y: f32, rd: f32, inner_h: f32, inner_w: f32)
    -> Result<ShapeGeometry, SceneError> {
    let mut current = quantized_point((x, start_y), "node_shape.mindmap.move")?;
    let mut segments = vec![PathSegment::MoveTo(current)];
    // Each relative operand is emitted separately by the source. Quantize the
    // operands before adding them to the current absolute scene coordinate.
    let radius = two_decimals(rd, "node_shape.mindmap.relative.radius")?;
    // Source spells this as a literal minus followed by the formatted rd.
    let neg_radius = parse_decimal(&format!("-{rd:.2}"),
        "node_shape.mindmap.relative.neg_radius")?;
    let up = two_decimals(-inner_h, "node_shape.mindmap.relative.up")?;
    let right = two_decimals(inner_w, "node_shape.mindmap.relative.right")?;
    let down = two_decimals(inner_h, "node_shape.mindmap.relative.down")?;
    let left = two_decimals(-inner_w, "node_shape.mindmap.relative.left")?;

    current = checked_point(Point { x: current.x, y: current.y + up })?;
    segments.push(PathSegment::LineTo(current));
    let control = relative(current, 0.0, neg_radius)?;
    current = relative(current, radius, neg_radius)?;
    segments.push(PathSegment::QuadraticTo { control, end: current });
    current = checked_point(Point { x: current.x + right, y: current.y })?;
    segments.push(PathSegment::LineTo(current));
    let control = relative(current, radius, 0.0)?;
    current = relative(current, radius, radius)?;
    segments.push(PathSegment::QuadraticTo { control, end: current });
    current = checked_point(Point { x: current.x, y: current.y + down })?;
    segments.push(PathSegment::LineTo(current));
    let control = relative(current, 0.0, radius)?;
    current = relative(current, neg_radius, radius)?;
    segments.push(PathSegment::QuadraticTo { control, end: current });
    current = checked_point(Point { x: current.x + left, y: current.y })?;
    segments.push(PathSegment::LineTo(current));
    let control = relative(current, neg_radius, 0.0)?;
    current = relative(current, neg_radius, neg_radius)?;
    segments.push(PathSegment::QuadraticTo { control, end: current });
    segments.push(PathSegment::Close);
    Ok(ShapeGeometry::Path(Path::new(segments, FillRule::NonZero)))
}

fn relative(current: Point, dx: f64, dy: f64) -> Result<Point, SceneError> {
    checked_point(Point { x: current.x + dx, y: current.y + dy })
}

fn checked_point(point: Point) -> Result<Point, SceneError> {
    if !point.x.is_finite() || !point.y.is_finite() {
        return Err(SceneError::InvalidNumber {
            context: "node_shape.mindmap absolute coordinate overflow".to_owned(),
        });
    }
    Ok(point)
}

fn finite(value: f32, context: &str) -> Result<(), SceneError> {
    if !value.is_finite() {
        return Err(SceneError::InvalidNumber { context: context.to_owned() });
    }
    Ok(())
}

fn quantized_point(value: (f32, f32), context: &str) -> Result<Point, SceneError> {
    Ok(Point {
        x: two_decimals(value.0, &format!("{context}.x"))?,
        y: two_decimals(value.1, &format!("{context}.y"))?,
    })
}

fn size(value: f32, context: &str) -> Result<f64, SceneError> {
    let number = two_decimals(value, context)?;
    if number < 0.0 {
        return Err(SceneError::InvalidGeometry {
            context: format!("{context}: negative emitted size or radius"),
        });
    }
    Ok(number)
}

fn display_width(value: f32, context: &str) -> Result<f64, SceneError> {
    finite(value, context)?;
    let number = parse_decimal(&value.to_string(), context)?;
    nonnegative_paint(number, context)?;
    Ok(number)
}

fn nonnegative_paint(value: f64, context: &str) -> Result<(), SceneError> {
    if value < 0.0 {
        return Err(SceneError::InvalidPaint {
            context: format!("{context}: negative emitted width"),
        });
    }
    Ok(())
}

fn two_decimals(value: f32, context: &str) -> Result<f64, SceneError> {
    finite(value, context)?;
    parse_decimal(&format!("{value:.2}"), context)
}

fn parse_decimal(value: &str, context: &str) -> Result<f64, SceneError> {
    let number = value.parse::<f64>().map_err(|error| SceneError::InvalidNumber {
        context: format!("{context}: scalar quantization failed: {error}"),
    })?;
    if !number.is_finite() {
        return Err(SceneError::InvalidNumber {
            context: format!("{context}: nonfinite emitted number"),
        });
    }
    Ok(number)
}
