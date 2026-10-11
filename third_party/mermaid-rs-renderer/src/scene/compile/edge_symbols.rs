//! Typed extraction of the pinned renderer's explicit edge-symbol helpers.
//!
//! Callers supply the already selected endpoint, tangent and resolved paint.
//! Source arithmetic remains f32 until scalar decimal quantization. These are
//! ordinary scene groups, not SVG marker definitions or routed edge geometry.

use crate::ir::EdgeDecoration;
use crate::scene::{
    Affine, ExportMetadata, FillRule, GroupNode, LayerStyle, LineCap, LineJoin,
    Paint, PaintStyle, Path, PathSegment, Point, SceneError, SceneNode,
    ShapeGeometry, ShapeNode, Stroke,
};

/// Compile the source's explicitly drawn flowchart arrowhead.
/// Start-arrow reversal is a caller responsibility, as in the source.
pub fn arrowhead(
    point: (f32, f32),
    angle_deg: f32,
    stroke: Paint,
    stroke_width: f32,
) -> Result<SceneNode, SceneError> {
    validate_inputs(point, angle_deg, stroke_width, "arrowhead")?;
    // A finite width may overflow this expression before clamping to 10.
    // That intermediate infinity is part of the source's valid behavior.
    let size = (stroke_width * 1.5 + 4.5).clamp(5.5, 10.0);
    let half = size * 0.52;
    let polygon = ShapeGeometry::Polygon(vec![
        local_point(0.0, 0.0),
        Point {
            x: two_decimals(-size, "arrowhead.neg_size")?,
            y: two_decimals(half, "arrowhead.half")?,
        },
        Point {
            x: two_decimals(-size, "arrowhead.neg_size")?,
            y: two_decimals(-half, "arrowhead.neg_half")?,
        },
    ]);
    let width = display_width(stroke_width, "arrowhead.stroke_width")?;
    let symbol = shape_node(polygon, stroke.clone(), &stroke, width, true);
    Ok(group_node(
        vec![symbol],
        symbol_transform(point, angle_deg, "arrowhead")?,
    ))
}

/// Compile all eight source edge decorations, retaining their local contours.
pub fn edge_decoration(
    point: (f32, f32),
    angle_deg: f32,
    decoration: EdgeDecoration,
    stroke: Paint,
    stroke_width: f32,
    at_start: bool,
) -> Result<SceneNode, SceneError> {
    validate_inputs(point, angle_deg, stroke_width, "edge_decoration")?;
    let mut angle = angle_deg;
    if matches!(decoration, EdgeDecoration::Diamond | EdgeDecoration::DiamondFilled)
        && !at_start
    {
        angle += 180.0;
    }
    let width = display_width(stroke_width, "edge_decoration.stroke_width")?;
    let symbol = match decoration {
        EdgeDecoration::Circle => circle_node(0.0, 5.0, &stroke, width),
        EdgeDecoration::Cross => path_node(
            vec![
                PathSegment::MoveTo(local_point(-5.0, -5.0)),
                PathSegment::LineTo(local_point(5.0, 5.0)),
                PathSegment::MoveTo(local_point(-5.0, 5.0)),
                PathSegment::LineTo(local_point(5.0, -5.0)),
            ],
            &stroke, width,
        ),
        EdgeDecoration::Diamond | EdgeDecoration::DiamondFilled => {
            let fill = if decoration == EdgeDecoration::DiamondFilled {
                stroke.clone()
            } else {
                Paint::None
            };
            shape_node(
                ShapeGeometry::Polygon(vec![
                    local_point(0.0, 0.0), local_point(9.0, 6.0),
                    local_point(18.0, 0.0), local_point(9.0, -6.0),
                ]),
                fill, &stroke, width, true,
            )
        }
        EdgeDecoration::CrowsFootOne => path_node(
            vec![
                PathSegment::MoveTo(local_point(0.0, -6.0)),
                PathSegment::LineTo(local_point(0.0, 6.0)),
                PathSegment::MoveTo(local_point(5.0, -6.0)),
                PathSegment::LineTo(local_point(5.0, 6.0)),
            ],
            &stroke, width,
        ),
        EdgeDecoration::CrowsFootZeroOne => group_node(
            vec![
                circle_node(-4.0, 4.0, &stroke, width),
                path_node(
                    vec![
                        PathSegment::MoveTo(local_point(4.0, -6.0)),
                        PathSegment::LineTo(local_point(4.0, 6.0)),
                    ],
                    &stroke, width,
                ),
            ],
            Affine::IDENTITY,
        ),
        EdgeDecoration::CrowsFootMany => path_node(
            vec![
                PathSegment::MoveTo(local_point(0.0, -6.0)),
                PathSegment::LineTo(local_point(0.0, 6.0)),
                PathSegment::MoveTo(local_point(0.0, 0.0)),
                PathSegment::LineTo(local_point(8.0, -6.0)),
                PathSegment::MoveTo(local_point(0.0, 0.0)),
                PathSegment::LineTo(local_point(8.0, 6.0)),
            ],
            &stroke, width,
        ),
        EdgeDecoration::CrowsFootZeroMany => group_node(
            vec![
                circle_node(-4.0, 4.0, &stroke, width),
                path_node(
                    vec![
                        PathSegment::MoveTo(local_point(4.0, 0.0)),
                        PathSegment::LineTo(local_point(12.0, -6.0)),
                        PathSegment::MoveTo(local_point(4.0, 0.0)),
                        PathSegment::LineTo(local_point(12.0, 6.0)),
                    ],
                    &stroke, width,
                ),
            ],
            Affine::IDENTITY,
        ),
    };
    Ok(group_node(
        vec![symbol],
        symbol_transform(point, angle, "edge_decoration")?,
    ))
}

fn local_point(x: f64, y: f64) -> Point { Point { x, y } }

fn circle_node(x: f64, radius: f64, stroke: &Paint, width: f64) -> SceneNode {
    shape_node(
        ShapeGeometry::Circle { center: local_point(x, 0.0), radius },
        Paint::None, stroke, width, false,
    )
}

fn path_node(segments: Vec<PathSegment>, stroke: &Paint, width: f64) -> SceneNode {
    shape_node(
        ShapeGeometry::Path(Path::new(segments, FillRule::NonZero)),
        Paint::None, stroke, width, true,
    )
}

fn shape_node(
    geometry: ShapeGeometry, fill: Paint, stroke: &Paint, width: f64,
    round_stroke: bool,
) -> SceneNode {
    SceneNode::Shape(ShapeNode {
        geometry,
        style: PaintStyle {
            fill,
            stroke: Some(Stroke {
                paint: stroke.clone(), width, dash: Vec::new(), dash_offset: 0.0,
                cap: if round_stroke { LineCap::Round } else { LineCap::Butt },
                join: if round_stroke { LineJoin::Round } else { LineJoin::Miter },
                miter_limit: 4.0,
            }),
            fill_opacity: 1.0, stroke_opacity: 1.0, opacity: 1.0,
        },
        metadata: ExportMetadata::default(),
    })
}

fn group_node(children: Vec<SceneNode>, transform: Affine) -> SceneNode {
    SceneNode::Group(GroupNode {
        children, transform, clip: None, layer: LayerStyle::default(),
        metadata: ExportMetadata::default(),
    })
}

fn validate_inputs(
    point: (f32, f32), angle: f32, width: f32, context: &str,
) -> Result<(), SceneError> {
    finite(point.0, &format!("{context}.point.x"))?;
    finite(point.1, &format!("{context}.point.y"))?;
    finite(angle, &format!("{context}.angle"))?;
    finite(width, &format!("{context}.stroke_width"))?;
    if width < 0.0 {
        return Err(SceneError::InvalidPaint {
            context: format!("{context}.stroke_width: negative width"),
        });
    }
    Ok(())
}

fn symbol_transform(
    point: (f32, f32), angle: f32, context: &str,
) -> Result<Affine, SceneError> {
    let x = two_decimals(point.0, &format!("{context}.translate.x"))?;
    let y = two_decimals(point.1, &format!("{context}.translate.y"))?;
    let radians = two_decimals(angle, &format!("{context}.rotation"))?.to_radians();
    // Match svgtypes' separate f64 trigonometric calls, including huge angles.
    let cos = radians.cos();
    let sin = radians.sin();
    Ok(Affine { a: cos, b: sin, c: -sin, d: cos, e: x, f: y })
}

fn finite(value: f32, context: &str) -> Result<(), SceneError> {
    if !value.is_finite() {
        return Err(SceneError::InvalidNumber { context: context.to_owned() });
    }
    Ok(())
}

fn two_decimals(value: f32, context: &str) -> Result<f64, SceneError> {
    finite(value, context)?;
    parse_decimal(&format!("{value:.2}"), context)
}

fn display_width(value: f32, context: &str) -> Result<f64, SceneError> {
    finite(value, context)?;
    parse_decimal(&value.to_string(), context)
}

fn parse_decimal(value: &str, context: &str) -> Result<f64, SceneError> {
    let number = value.parse::<f64>().map_err(|error| SceneError::InvalidNumber {
        context: format!("{context}: numeric quantization failed: {error}"),
    })?;
    if !number.is_finite() {
        return Err(SceneError::InvalidNumber {
            context: format!("{context}: nonfinite quantized number"),
        });
    }
    Ok(number)
}
