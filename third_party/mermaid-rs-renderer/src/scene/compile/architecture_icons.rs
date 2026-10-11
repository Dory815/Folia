//! Typed presentation of the pinned Architecture icon renderer.
//! Lambda remains a source text request for the shared text preparation stage.

use crate::scene::compile::text_presentation::{
    FontPresentation, TextContentPresentation, TextPresentation, TextValue,
};
use crate::scene::{
    ExportMetadata, FillRule, LineCap, LineJoin, Paint, PaintStyle, Path,
    PathSegment, Point, Rect, SceneError, SceneNode, ShapeGeometry, ShapeNode,
    Stroke,
};

#[derive(Debug, Clone, PartialEq)]
pub enum ArchitectureIconItem {
    Shape(SceneNode),
    Text(TextPresentation),
}

#[derive(Debug, Clone, PartialEq)]
pub struct ArchitectureIconPresentation {
    pub items: Vec<ArchitectureIconItem>,
}

/// Preserve all branches of render.rs::architecture_icon_svg in source order.
pub fn architecture_icon(
    icon_type: Option<&str>,
    w: f32,
    h: f32,
    fill: &Paint,
    fill_lexeme: &str,
) -> Result<ArchitectureIconPresentation, SceneError> {
    let cx = w / 2.0;
    let cy = h / 2.0;
    let r = w.min(h) * 0.35;
    let sw = (w * 0.02).max(1.5);
    let width = one_decimal(sw, "architecture_icon.stroke_width")?;
    if width < 0.0 {
        return Err(SceneError::InvalidPaint {
            context: "architecture_icon.stroke_width: negative emitted width".to_owned(),
        });
    }
    let outline = outline_style(fill, width);
    let indicator = PaintStyle {
        fill: fill.clone(), ..PaintStyle::default()
    };
    let icon = icon_type.unwrap_or_default().to_ascii_lowercase();
    let mut items = Vec::new();
    match icon.as_str() {
        "internet" | "globe" => {
            items.push(shape(circle(cx, cy, r)?, &outline));
            items.push(shape(ellipse(cx, cy, r * 0.5, r)?, &outline));
            items.push(shape(line((cx - r, cy), (cx + r, cy))?, &outline));
            items.push(shape(line((cx, cy - r), (cx, cy + r))?, &outline));
        }
        name if name.contains("internet")
            || name.contains("gateway")
            || name.contains("api-gateway") =>
        {
            items.push(shape(circle(cx, cy, r)?, &outline));
            items.push(shape(ellipse(cx, cy, r * 0.5, r)?, &outline));
            items.push(shape(line((cx - r, cy), (cx + r, cy))?, &outline));
        }
        "server" => server_icon(cx, cy, r, &outline, &indicator, &mut items)?,
        name if name.contains("server") || name.contains("ec2") || name.contains("compute") => {
            server_icon(cx, cy, r, &outline, &indicator, &mut items)?;
        }
        "database" | "disk" => database_icon(cx, cy, r, true, &outline, &mut items)?,
        name if name.contains("database")
            || name.contains("aurora")
            || name.contains("rds")
            || name.contains("dynamodb")
            || name.contains("db")
            || name.contains("disk")
            || name.contains("storage")
            || name.contains("s3")
            || name.contains("glacier") =>
        {
            database_icon(cx, cy, r, false, &outline, &mut items)?;
        }
        "cloud" => items.push(shape(cloud_icon(cx, cy, r)?, &outline)),
        name if name.contains("cloud") => {
            items.push(shape(cloud_icon(cx, cy, r)?, &outline));
        }
        name if name.contains("lambda") => {
            let fs = w * 0.72;
            items.push(ArchitectureIconItem::Text(TextPresentation {
                x: one_decimal(cx, "architecture_icon.lambda.x")?,
                y: one_decimal(cy, "architecture_icon.lambda.y")?,
                anchor: TextValue::Literal("middle".to_owned()),
                font: FontPresentation::Attributes {
                    family: TextValue::Literal("serif".to_owned()),
                    size: font_size_zero_decimals(fs)?,
                    weight: None,
                },
                fill: TextValue::AttributeLexeme(fill_lexeme.to_owned()),
                italic: None,
                dominant_baseline: Some("central".to_owned()),
                content: TextContentPresentation::Direct("λ".to_owned()),
            }));
        }
        _ => {
            let s = r * 0.9;
            let x = cx - s;
            let y = cy - s;
            let inner = s * 0.35;
            items.push(shape(rectangle(x, y, s * 2.0, s * 2.0, 4.0)?, &outline));
            items.push(shape(line((cx - inner, cy), (cx + inner, cy))?, &outline));
            items.push(shape(line((cx, cy - inner), (cx, cy + inner))?, &outline));
        }
    }
    Ok(ArchitectureIconPresentation { items })
}

fn server_icon(
    cx: f32, cy: f32, r: f32,
    outline: &PaintStyle, indicator: &PaintStyle,
    items: &mut Vec<ArchitectureIconItem>,
) -> Result<(), SceneError> {
    let bx = cx - r;
    let by = cy - r;
    let bw = r * 2.0;
    let bh = r * 2.0;
    let rows = 3;
    let row_h = bh / rows as f32;
    for i in 0..rows {
        let ry = by + i as f32 * row_h;
        items.push(shape(rectangle(bx, ry, bw, row_h, 2.0)?, outline));
        let dot_x = bx + bw - row_h * 0.35;
        let dot_y = ry + row_h * 0.5;
        let dot_r = row_h * 0.12;
        items.push(shape(circle(dot_x, dot_y, dot_r)?, indicator));
    }
    Ok(())
}

fn database_icon(
    cx: f32, cy: f32, r: f32, include_lines: bool,
    outline: &PaintStyle, items: &mut Vec<ArchitectureIconItem>,
) -> Result<(), SceneError> {
    let bx = cx - r;
    let bw = r * 2.0;
    let ell_ry = r * 0.3;
    let body_top = cy - r + ell_ry;
    let body_bot = cy + r - ell_ry;
    let body_h = body_bot - body_top;
    items.push(shape(rectangle(bx, body_top, bw, body_h, 0.0)?, outline));
    items.push(shape(ellipse(cx, body_top, r, ell_ry)?, outline));
    items.push(shape(ellipse(cx, body_bot, r, ell_ry)?, outline));
    if include_lines {
        items.push(shape(line((bx, body_top), (bx, body_bot))?, outline));
        items.push(shape(line((bx + bw, body_top), (bx + bw, body_bot))?, outline));
    }
    Ok(())
}

fn cloud_icon(cx: f32, cy: f32, r: f32) -> Result<ShapeGeometry, SceneError> {
    let s = r * 0.7;
    let x1 = cx - s;
    let x2 = cx + s;
    let y_mid = cy;
    let y_top = cy - s * 0.8;
    let y_bot = cy + s * 0.6;
    let segments = vec![
        PathSegment::MoveTo(point(x1, y_mid, "architecture_icon.cloud.move")?),
        PathSegment::QuadraticTo {
            control: point(x1, y_top, "architecture_icon.cloud.top_left_control")?,
            end: point(cx, y_top, "architecture_icon.cloud.top_left_end")?,
        },
        PathSegment::QuadraticTo {
            control: point(x2, y_top, "architecture_icon.cloud.top_right_control")?,
            end: point(x2, y_mid, "architecture_icon.cloud.top_right_end")?,
        },
        PathSegment::QuadraticTo {
            control: point(x2, y_bot, "architecture_icon.cloud.bottom_right_control")?,
            end: point(cx, y_bot, "architecture_icon.cloud.bottom_right_end")?,
        },
        PathSegment::QuadraticTo {
            control: point(x1, y_bot, "architecture_icon.cloud.bottom_left_control")?,
            end: point(x1, y_mid, "architecture_icon.cloud.bottom_left_end")?,
        },
        PathSegment::Close,
    ];
    Ok(ShapeGeometry::Path(Path::new(segments, FillRule::NonZero)))
}

fn outline_style(fill: &Paint, width: f64) -> PaintStyle {
    PaintStyle {
        fill: Paint::None,
        stroke: Some(Stroke {
            paint: fill.clone(), width, dash: Vec::new(), dash_offset: 0.0,
            cap: LineCap::Round, join: LineJoin::Round, miter_limit: 4.0,
        }),
        fill_opacity: 1.0, stroke_opacity: 1.0, opacity: 1.0,
    }
}

fn shape(geometry: ShapeGeometry, style: &PaintStyle) -> ArchitectureIconItem {
    ArchitectureIconItem::Shape(SceneNode::Shape(ShapeNode {
        geometry, style: style.clone(), metadata: ExportMetadata::default(),
    }))
}

fn rectangle(x: f32, y: f32, w: f32, h: f32, radius: f32)
    -> Result<ShapeGeometry, SceneError> {
    Ok(ShapeGeometry::Rect {
        rect: Rect {
            x: one_decimal(x, "architecture_icon.rect.x")?,
            y: one_decimal(y, "architecture_icon.rect.y")?,
            width: size_one_decimal(w, "architecture_icon.rect.width")?,
            height: size_one_decimal(h, "architecture_icon.rect.height")?,
        },
        rx: size_literal(radius, "architecture_icon.rect.rx")?,
        ry: size_literal(radius, "architecture_icon.rect.ry")?,
    })
}

fn circle(cx: f32, cy: f32, radius: f32) -> Result<ShapeGeometry, SceneError> {
    Ok(ShapeGeometry::Circle {
        center: point(cx, cy, "architecture_icon.circle.center")?,
        radius: size_one_decimal(radius, "architecture_icon.circle.radius")?,
    })
}

fn ellipse(cx: f32, cy: f32, rx: f32, ry: f32) -> Result<ShapeGeometry, SceneError> {
    Ok(ShapeGeometry::Ellipse {
        center: point(cx, cy, "architecture_icon.ellipse.center")?,
        rx: size_one_decimal(rx, "architecture_icon.ellipse.rx")?,
        ry: size_one_decimal(ry, "architecture_icon.ellipse.ry")?,
    })
}

fn line(start: (f32, f32), end: (f32, f32)) -> Result<ShapeGeometry, SceneError> {
    Ok(ShapeGeometry::Line {
        start: point(start.0, start.1, "architecture_icon.line.start")?,
        end: point(end.0, end.1, "architecture_icon.line.end")?,
    })
}

fn point(x: f32, y: f32, context: &str) -> Result<Point, SceneError> {
    Ok(Point {
        x: one_decimal(x, &format!("{context}.x"))?,
        y: one_decimal(y, &format!("{context}.y"))?,
    })
}

fn size_one_decimal(value: f32, context: &str) -> Result<f64, SceneError> {
    let emitted = one_decimal(value, context)?;
    if emitted < 0.0 {
        return Err(SceneError::InvalidGeometry {
            context: format!("{context}: negative emitted size or radius"),
        });
    }
    Ok(emitted)
}

fn size_literal(value: f32, context: &str) -> Result<f64, SceneError> {
    if !value.is_finite() {
        return Err(SceneError::InvalidNumber { context: context.to_owned() });
    }
    if value < 0.0 {
        return Err(SceneError::InvalidGeometry {
            context: format!("{context}: negative emitted size or radius"),
        });
    }
    Ok(value as f64)
}

fn font_size_zero_decimals(value: f32) -> Result<f64, SceneError> {
    let size = decimal(value, 0, "architecture_icon.lambda.font_size")?;
    if size < 0.0 {
        return Err(SceneError::InvalidGeometry {
            context: "architecture_icon.lambda.font_size: negative emitted font size".to_owned(),
        });
    }
    Ok(size)
}

fn one_decimal(value: f32, context: &str) -> Result<f64, SceneError> {
    decimal(value, 1, context)
}

fn decimal(value: f32, places: usize, context: &str) -> Result<f64, SceneError> {
    if !value.is_finite() {
        return Err(SceneError::InvalidNumber { context: context.to_owned() });
    }
    let emitted = format!("{value:.places$}");
    let number = emitted.parse::<f64>().map_err(|error| SceneError::InvalidNumber {
        context: format!("{context}: scalar quantization failed: {error}"),
    })?;
    if !number.is_finite() {
        return Err(SceneError::InvalidNumber {
            context: format!("{context}: nonfinite emitted number"),
        });
    }
    Ok(number)
}
