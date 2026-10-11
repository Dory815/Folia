//! Source text presentation, before the single shared font preparation stage.
//!
//! Scalar decimal conversion preserves emission precision. Character content
//! remains literal Unicode; lexical attributes/CSS are not resolved here.

use crate::config::LayoutConfig;
use crate::layout::TextBlock;
use crate::scene::SceneError;
use crate::theme::Theme;

use super::text_presentation::{
    FontPresentation, TextContentPresentation, TextPresentation,
    TextSpanPresentation, TextValue,
};

pub fn normalize_font_family(font_family: &str) -> String {
    let normalized = font_family
        .split(',')
        .map(|part| part.trim().trim_matches('\'').trim_matches('"'))
        .filter(|part| !part.is_empty())
        .collect::<Vec<_>>()
        .join(",");
    if normalized.is_empty() {
        "sans-serif".to_string()
    } else {
        normalized
    }
}

pub fn is_divider_line(line: &str) -> bool { line.trim() == "---" }

pub fn gitgraph_multiline_text(
    x: f32,
    y: f32,
    text: &str,
    font_family: &str,
    font_size: f32,
    line_height: f32,
    color: &str,
) -> Result<Vec<TextPresentation>, SceneError> {
    let lines: Vec<&str> = text.split('\n').collect();
    if lines.is_empty() {
        return Ok(Vec::new());
    }
    let start_y = y + font_size;
    let mut presentation = attribute_text(
        x, start_y, font_family, font_size, None,
        TextValue::Literal("start".to_string()),
        TextValue::Literal(color.to_string()),
        TextContentPresentation::Spans(Vec::new()),
    )?;
    let line_height_px = font_size * line_height;
    let mut spans = Vec::with_capacity(lines.len());
    for (idx, line) in lines.iter().enumerate() {
        let dy = if idx == 0 { 0.0 } else { line_height_px };
        spans.push(TextSpanPresentation {
            text: (*line).to_string(),
            x: Some(presentation.x),
            dy: two_decimals(dy, "gitgraph_multiline_text.span.dy")?,
            alignment_baseline: None,
        });
    }
    presentation.content = TextContentPresentation::Spans(spans);
    Ok(vec![presentation])
}

pub fn text_block(
    x: f32,
    y: f32,
    label: &TextBlock,
    theme: &Theme,
    config: &LayoutConfig,
    _edge: bool,
    override_color: Option<&str>,
) -> Result<Vec<TextPresentation>, SceneError> {
    text_block_with_font_size(
        x, y, label, theme, config, theme.font_size, "middle", override_color, false,
    )
}

pub fn text_block_anchor(
    x: f32,
    y: f32,
    label: &TextBlock,
    theme: &Theme,
    config: &LayoutConfig,
    anchor: &str,
    override_color: Option<&str>,
) -> Result<Vec<TextPresentation>, SceneError> {
    text_block_with_font_size(
        x, y, label, theme, config, theme.font_size, anchor, override_color, false,
    )
}

pub fn text_block_with_font_size(
    x: f32,
    y: f32,
    label: &TextBlock,
    theme: &Theme,
    config: &LayoutConfig,
    font_size: f32,
    anchor: &str,
    override_color: Option<&str>,
    baseline: bool,
) -> Result<Vec<TextPresentation>, SceneError> {
    block_presentation(
        x, y, label, theme, config, font_size, anchor, override_color, None, baseline,
    )
}

pub fn text_block_with_font_size_weight(
    x: f32,
    y: f32,
    label: &TextBlock,
    theme: &Theme,
    config: &LayoutConfig,
    font_size: f32,
    anchor: &str,
    override_color: Option<&str>,
    font_weight: Option<&str>,
    baseline: bool,
) -> Result<Vec<TextPresentation>, SceneError> {
    let weight = font_weight
        .filter(|weight| !weight.trim().is_empty())
        .map(|weight| TextValue::AttributeLexeme(weight.to_string()));
    block_presentation(
        x, y, label, theme, config, font_size, anchor, override_color, weight, baseline,
    )
}

pub fn text_line_with_font_size(
    x: f32,
    y: f32,
    text: &str,
    theme: &Theme,
    font_size: f32,
    fill: &str,
    anchor: &str,
) -> Result<Vec<TextPresentation>, SceneError> {
    Ok(vec![attribute_text(
        x, y, &theme.font_family, font_size, None,
        TextValue::AttributeLexeme(anchor.to_string()),
        TextValue::AttributeLexeme(fill.to_string()),
        TextContentPresentation::Direct(text.to_string()),
    )?])
}

pub fn text_line(
    x: f32,
    y: f32,
    text: &str,
    theme: &Theme,
    fill: &str,
    anchor: &str,
) -> Result<Vec<TextPresentation>, SceneError> {
    text_line_with_font_size(x, y, text, theme, theme.font_size, fill, anchor)
}

pub fn c4_text(
    x: f32,
    y: f32,
    lines: &[String],
    font_family: &str,
    font_size: f32,
    font_weight: &str,
    fill: &str,
    italic: bool,
) -> Result<Vec<TextPresentation>, SceneError> {
    let mut out = Vec::with_capacity(lines.len());
    let line_count = lines.len() as f32;
    for (idx, line) in lines.iter().enumerate() {
        let dy = idx as f32 * font_size - font_size * (line_count - 1.0) / 2.0;
        out.push(TextPresentation {
            x: two_decimals(x, "c4_text.x")?,
            y: two_decimals(y, "c4_text.y")?,
            anchor: TextValue::Literal("middle".to_string()),
            font: FontPresentation::C4Style {
                size_px: display_number(font_size, "c4_text.font_size")?,
                weight: font_weight.to_string(),
                family: normalize_font_family(font_family),
            },
            fill: TextValue::Literal(fill.to_string()),
            italic: if italic { Some(true) } else { None },
            dominant_baseline: Some("middle".to_string()),
            content: TextContentPresentation::Spans(vec![TextSpanPresentation {
                text: line.clone(), x: None,
                dy: two_decimals(dy, "c4_text.span.dy")?,
                alignment_baseline: Some("mathematical".to_string()),
            }]),
        });
    }
    // Empty C4 emits no element and consequently no numeric input is checked.
    Ok(out)
}

fn block_presentation(
    x: f32,
    y: f32,
    label: &TextBlock,
    theme: &Theme,
    config: &LayoutConfig,
    font_size: f32,
    anchor: &str,
    override_color: Option<&str>,
    weight: Option<TextValue>,
    baseline: bool,
) -> Result<Vec<TextPresentation>, SceneError> {
    let total_height = label.lines.len() as f32 * font_size * config.label_line_height;
    let start_y = if baseline {
        y
    } else {
        y - total_height / 2.0 + font_size
    };
    let default_fill = theme.primary_text_color.as_str();
    let fill = override_color.unwrap_or(default_fill);
    let mut presentation = attribute_text(
        x, start_y, &theme.font_family, font_size, weight,
        TextValue::AttributeLexeme(anchor.to_string()),
        TextValue::AttributeLexeme(fill.to_string()),
        TextContentPresentation::Spans(Vec::new()),
    )?;
    let line_height = font_size * config.label_line_height;
    let mut spans = Vec::with_capacity(label.lines.len());
    for (idx, line) in label.lines.iter().enumerate() {
        let dy = if idx == 0 { 0.0 } else { line_height };
        let rendered = if is_divider_line(line) { String::new() } else { line.clone() };
        spans.push(TextSpanPresentation {
            text: rendered, x: Some(presentation.x),
            dy: two_decimals(dy, "text_block.span.dy")?,
            alignment_baseline: None,
        });
    }
    presentation.content = TextContentPresentation::Spans(spans);
    // The source has no zero-line guard: preserve its empty parent element.
    Ok(vec![presentation])
}

fn attribute_text(
    x: f32,
    y: f32,
    family: &str,
    size: f32,
    weight: Option<TextValue>,
    anchor: TextValue,
    fill: TextValue,
    content: TextContentPresentation,
) -> Result<TextPresentation, SceneError> {
    Ok(TextPresentation {
        x: two_decimals(x, "text_presentation.x")?,
        y: two_decimals(y, "text_presentation.y")?,
        anchor,
        font: FontPresentation::Attributes {
            family: TextValue::AttributeLexeme(normalize_font_family(family)),
            size: display_number(size, "text_presentation.font_size")?,
            weight,
        },
        fill, italic: None, dominant_baseline: None, content,
    })
}

fn two_decimals(value: f32, context: &str) -> Result<f64, SceneError> {
    finite_emitted(value, context)?;
    parse_decimal(&format!("{value:.2}"), context)
}

fn display_number(value: f32, context: &str) -> Result<f64, SceneError> {
    finite_emitted(value, context)?;
    parse_decimal(&value.to_string(), context)
}

fn finite_emitted(value: f32, context: &str) -> Result<(), SceneError> {
    if !value.is_finite() {
        return Err(SceneError::InvalidNumber { context: context.to_owned() });
    }
    Ok(())
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
