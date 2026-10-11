//! Immutable completed geometry shared by all output backends.
//!
//! Only SceneBuilder::finish publishes a scene. Backends read positioned data;
//! they do not parse, shape, wrap, route, expand markers, or place diagram nodes.
pub mod geometry;
pub mod paint;
pub mod text;
pub mod builder;
pub mod compile;
pub mod prepare;

pub use builder::SceneBuilder;
pub use geometry::*;
pub use paint::*;
pub use text::*;

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct SceneDimensions {
    pub width: f64,
    pub height: f64,
    pub viewbox: Rect,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct RootViewport {
    pub clip: Rect,
    pub aspect_ratio: AspectRatioPolicy,
    /// Completed viewBox -> viewport transform, shared by both backends.
    pub viewbox_to_viewport: Affine,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct DataAttribute {
    /// Suffix after `data-`; never a paint/geometry attribute.
    pub name: String,
    pub value: String,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ExportLink {
    pub url: String,
    pub target: Option<String>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ExportDefinitionIdentity {
    pub id: String,
    pub kind: ExportDefinitionKind,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ExportDefinitionKind { Marker, Symbol, Gradient }

#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct ExportMetadata {
    pub id: Option<String>,
    pub classes: Vec<String>,
    pub data_attributes: Vec<DataAttribute>,
    pub title: Option<String>,
    pub link: Option<ExportLink>,
    pub semantic_text: Option<String>,
    pub definition: Option<ExportDefinitionIdentity>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct ShapeNode {
    pub geometry: ShapeGeometry,
    pub style: PaintStyle,
    pub metadata: ExportMetadata,
}

#[derive(Debug, Clone, PartialEq)]
pub struct GroupNode {
    pub children: Vec<SceneNode>,
    pub transform: Affine,
    pub clip: Option<Clip>,
    pub layer: LayerStyle,
    pub metadata: ExportMetadata,
}

#[derive(Debug, Clone, PartialEq)]
pub enum SceneNode {
    Shape(ShapeNode),
    Text(TextNode),
    Image(ImageNode),
    Group(GroupNode),
}

/// All storage is private and all access after finish is read-only.
#[derive(Debug)]
pub struct DiagramScene {
    dimensions: SceneDimensions,
    viewport: RootViewport,
    nodes: Vec<SceneNode>,
    gradients: Vec<LinearGradient>,
    fonts: Vec<FontResource>,
    images: Vec<ImageResource>,
    metadata: ExportMetadata,
}

impl DiagramScene {
    pub fn dimensions(&self) -> SceneDimensions { self.dimensions }
    pub fn viewport(&self) -> RootViewport { self.viewport }
    pub fn nodes(&self) -> &[SceneNode] { &self.nodes }
    pub fn gradients(&self) -> &[LinearGradient] { &self.gradients }
    pub fn fonts(&self) -> &[FontResource] { &self.fonts }
    pub fn images(&self) -> &[ImageResource] { &self.images }
    pub fn metadata(&self) -> &ExportMetadata { &self.metadata }
    pub fn gradient(&self, id: GradientId) -> Option<&LinearGradient> {
        self.gradients.get(id.0)
    }
    pub fn font(&self, id: FontId) -> Option<&FontResource> { self.fonts.get(id.0) }
    pub fn image(&self, id: ImageId) -> Option<&ImageResource> { self.images.get(id.0) }
}

impl Drop for DiagramScene {
    fn drop(&mut self) {
        builder::drop_nodes_iteratively(std::mem::take(&mut self.nodes));
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum SceneError {
    InvalidNumber { context: String },
    InvalidGeometry { context: String },
    InvalidPaint { context: String },
    InvalidResource { context: String },
    UnresolvedText { context: String },
    UnsupportedFeature { context: String },
    UnbalancedState { context: String },
}

impl std::fmt::Display for SceneError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let (kind, context) = match self {
            Self::InvalidNumber { context } => ("invalid number", context),
            Self::InvalidGeometry { context } => ("invalid geometry", context),
            Self::InvalidPaint { context } => ("invalid paint", context),
            Self::InvalidResource { context } => ("invalid resource", context),
            Self::UnresolvedText { context } => ("unresolved text", context),
            Self::UnsupportedFeature { context } => ("unsupported feature", context),
            Self::UnbalancedState { context } => ("unbalanced state", context),
        };
        write!(f, "{kind}: {context}")
    }
}

impl std::error::Error for SceneError {}
