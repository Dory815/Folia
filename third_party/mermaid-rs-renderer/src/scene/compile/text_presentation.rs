//! Root-owned typed input to the shared text preparation stage.
//! These are source presentation requests, not backend text or final geometry.
//! The common resolver must interpret lexical attribute/CSS values once before
//! shaping; neither output backend consumes this intermediate representation.

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum TextValue {
    /// Source escaped the XML attribute; its semantic value is this string.
    Literal(String),
    /// Source inserted unescaped attribute text. The shared resolver owns
    /// entity decoding, whitespace normalization and invalid syntax errors.
    AttributeLexeme(String),
}

#[derive(Debug, Clone, PartialEq)]
pub struct TextSpanPresentation {
    pub text: String,
    pub x: Option<f64>,
    pub dy: f64,
    pub alignment_baseline: Option<String>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum TextContentPresentation {
    Direct(String),
    Spans(Vec<TextSpanPresentation>),
}

#[derive(Debug, Clone, PartialEq)]
pub enum FontPresentation {
    Attributes {
        family: TextValue,
        size: f64,
        weight: Option<TextValue>,
    },
    /// C4 emits one style declaration list. Preserve raw CSS values for the
    /// shared CSS parser, including source escapes and declaration order.
    C4Style { size_px: f64, weight: String, family: String },
}

#[derive(Debug, Clone, PartialEq)]
pub struct TextPresentation {
    pub x: f64,
    pub y: f64,
    pub anchor: TextValue,
    pub font: FontPresentation,
    pub fill: TextValue,
    pub italic: Option<bool>,
    pub dominant_baseline: Option<String>,
    pub content: TextContentPresentation,
}
