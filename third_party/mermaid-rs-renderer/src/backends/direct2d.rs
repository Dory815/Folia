//! Windows-only consumer of completed scene geometry; no layout or shaping.
use std::marker::PhantomData;
use std::rc::Rc;

use windows::core::{HRESULT, IUnknown, Interface};
use windows::Win32::Graphics::Direct2D::*;
use windows::Win32::Graphics::Direct2D::Common::*;
use windows_numerics::Matrix3x2;

use crate::scene::{Affine, BlendMode, DiagramScene, Rect, SceneDimensions, SceneNode};

mod geometry;
mod paint;
mod text;
mod image;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NativeErrorKind {
    InvalidNumber, InvalidResource, UnsupportedFeature, DeviceMismatch,
    DeviceLost, NativeFailure,
}

#[derive(Debug, Clone)]
pub struct NativeRenderError {
    pub kind: NativeErrorKind,
    pub hresult: Option<HRESULT>,
    pub operation: String,
    pub context: String,
    /// Errors from mandatory cleanup are retained alongside a primary failure.
    pub cleanup: Vec<NativeRenderError>,
}

impl NativeRenderError {
    pub(super) fn invalid_number(context: impl Into<String>) -> Self {
        Self::diagnostic(NativeErrorKind::InvalidNumber, "numeric conversion", context)
    }
    pub(super) fn invalid_resource(context: impl Into<String>) -> Self {
        Self::diagnostic(NativeErrorKind::InvalidResource, "resource preparation", context)
    }
    pub(super) fn unsupported(context: impl Into<String>) -> Self {
        Self::diagnostic(NativeErrorKind::UnsupportedFeature, "preflight", context)
    }
    fn diagnostic(kind: NativeErrorKind, operation: impl Into<String>, context: impl Into<String>) -> Self {
        Self { kind, hresult: None, operation: operation.into(), context: context.into(), cleanup: Vec::new() }
    }
}

impl std::fmt::Display for NativeRenderError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}: {}: {}", self.kind, self.operation, self.context)?;
        if let Some(hr) = self.hresult { write!(f, " ({hr:?})")?; }
        for error in &self.cleanup { write!(f, "; cleanup: {error}")?; }
        Ok(())
    }
}
impl std::error::Error for NativeRenderError {}

pub(super) fn native_error(error: windows::core::Error, operation: &str) -> NativeRenderError {
    let hr = error.code();
    let kind = match hr.0 as u32 {
        0x8899000c | 0x887a0005 | 0x887a0006 | 0x887a0007 | 0x887a0020 => NativeErrorKind::DeviceLost,
        _ => NativeErrorKind::NativeFailure,
    };
    NativeRenderError { kind, hresult: Some(hr), operation: operation.to_owned(),
        context: error.to_string(), cleanup: Vec::new() }
}

pub(super) fn win<T>(result: windows::core::Result<T>, operation: &str) -> Result<T, NativeRenderError> {
    result.map_err(|error| native_error(error, operation))
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct DrawOptions {
    pub transform: Affine,
    /// Clip in caller's original coordinate space, before destination mapping.
    pub clip: Option<Rect>,
}
impl Default for DrawOptions {
    fn default() -> Self { Self { transform: Affine::IDENTITY, clip: None } }
}

#[derive(Debug)]
pub struct NativeRenderer {
    context: ID2D1DeviceContext,
    factory: ID2D1Factory,
    device_identity: IUnknown,
    _thread_confined: PhantomData<Rc<()>>,
}

#[derive(Debug)]
pub struct RecordedScene {
    image: ID2D1Image,
    dimensions: SceneDimensions,
    device_identity: IUnknown,
    _thread_confined: PhantomData<Rc<()>>,
}

struct NativeNode { image: ID2D1Image, blend: BlendMode }

impl NativeRenderer {
    pub fn new(device: &ID2D1Device) -> Result<Self, NativeRenderError> {
        let context = win(unsafe { device.CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE) }, "CreateDeviceContext")?;
        let factory = win(unsafe { context.GetFactory() }, "GetFactory")?;
        let device_identity = win(device.cast::<IUnknown>(), "device canonical IUnknown")?;
        unsafe {
            context.SetDpi(96.0, 96.0);
            context.SetUnitMode(D2D1_UNIT_MODE_DIPS);
            context.SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
            context.SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
        Ok(Self { context, factory, device_identity, _thread_confined: PhantomData })
    }

    pub fn record_scene(&mut self, scene: &DiagramScene) -> Result<RecordedScene, NativeRenderError> {
        // Each attempt owns a fresh resource table; indices are scene-local.
        let dimensions = scene.dimensions();
        geometry::number(dimensions.width, "scene.width")?;
        geometry::number(dimensions.height, "scene.height")?;
        geometry::rect(dimensions.viewbox, "scene.viewbox")?;
        let viewport = scene.viewport();
        let clip = geometry::rect(viewport.clip, "scene.viewport.clip")?;
        let transform = geometry::matrix(viewport.viewbox_to_viewport, "scene.viewport.transform")?;
        paint::preflight_resources(scene)?;
        let images = image::prepare_resources(&self.context, scene.images())?;
        let mut nodes = Vec::with_capacity(scene.nodes().len());
        for (index, node) in scene.nodes().iter().enumerate() {
            nodes.push(self.prepare_node(scene, node, &images, viewport.viewbox_to_viewport,
                &format!("nodes[{index}]"))?);
        }
        let contents = self.compose(nodes, "scene ordered contents")?;
        let final_image = record(&self.context, "scene root viewport", |context| {
            let _clip = AxisClipGuard::push(context, &clip);
            unsafe { context.SetTransform(&transform); }
            draw_image(context, &contents);
            Ok(())
        })?;
        Ok(RecordedScene { image: final_image, dimensions,
            device_identity: self.device_identity.clone(), _thread_confined: PhantomData })
    }

    fn prepare_node(&self, scene: &DiagramScene, node: &SceneNode,
        images: &[ID2D1Bitmap1], parent_mapping: Affine,
        context: &str) -> Result<NativeNode, NativeRenderError> {
        geometry::matrix(parent_mapping, &format!("{context}.parent_mapping"))?;
        match node {
            SceneNode::Shape(shape) => {
                let geometry = geometry::convert(&self.factory, &shape.geometry, context)?;
                let drawing = paint::prepare_shape(&self.context, &self.factory, scene,
                    geometry, &shape.style, None, context)?;
                let image = record(&self.context, context, |ctx| { drawing.draw(ctx); Ok(()) })?;
                Ok(NativeNode { image, blend: BlendMode::Normal })
            }
            SceneNode::Text(text_node) => {
                let drawing = text::prepare(&self.context, &self.factory, scene, text_node, images, parent_mapping, context)?;
                let image = record(&self.context, context, |ctx| drawing.draw(ctx))?;
                Ok(NativeNode { image, blend: BlendMode::Normal })
            }
            SceneNode::Image(image_node) => {
                geometry::matrix(parent_mapping.compose(image_node.source_to_destination),
                    &format!("{context}.completed_image_mapping"))?;
                paint::preflight_style(&image_node.style, context)?;
                let drawing = image::prepare_node(image_node, images, context)?;
                let image = record(&self.context, context, |ctx| drawing.draw(ctx))?;
                Ok(NativeNode { image, blend: BlendMode::Normal })
            }
            SceneNode::Group(group) => {
                let transform = geometry::matrix(group.transform, &format!("{context}.transform"))?;
                let completed_mapping = parent_mapping.compose(group.transform);
                geometry::matrix(completed_mapping, &format!("{context}.completed_mapping"))?;
                let opacity = geometry::number(group.layer.opacity, &format!("{context}.opacity"))?;
                let clip = match &group.clip {
                    Some(clip) => {
                        geometry::matrix(completed_mapping.compose(clip.transform),
                            &format!("{context}.clip.completed_transform"))?;
                        Some(geometry::transformed(&self.factory,
                            geometry::convert(&self.factory, &clip.geometry, &format!("{context}.clip"))?,
                            clip.transform, &format!("{context}.clip.transform"))?)
                    }
                    None => None,
                };
                let mut children = Vec::with_capacity(group.children.len());
                for (index, child) in group.children.iter().enumerate() {
                    children.push(self.prepare_node(scene, child, images, completed_mapping,
                        &format!("{context}.children[{index}]"))?);
                }
                let contents = self.compose(children, context)?;
                let image = record(&self.context, context, |ctx| {
                    unsafe { ctx.SetTransform(&transform); }
                    let _layer = paint::LayerGuard::push(ctx, opacity, clip.as_ref());
                    draw_image(ctx, &contents);
                    Ok(())
                })?;
                Ok(NativeNode { image, blend: group.layer.blend })
            }
        }
    }

    fn compose(&self, nodes: Vec<NativeNode>, operation: &str) -> Result<ID2D1Image, NativeRenderError> {
        let mut background: Option<ID2D1Image> = None;
        let mut normal = Vec::new();
        for node in nodes {
            match node.blend {
                BlendMode::Normal => normal.push(node.image),
                BlendMode::Multiply => {
                    let completed = self.normal_batch(background.as_ref(), &normal, operation)?;
                    normal.clear();
                    background = Some(paint::multiply(&self.context, &completed, &node.image, operation)?);
                }
            }
        }
        self.normal_batch(background.as_ref(), &normal, operation)
    }

    fn normal_batch(&self, background: Option<&ID2D1Image>, normal: &[ID2D1Image],
        operation: &str) -> Result<ID2D1Image, NativeRenderError> {
        record(&self.context, operation, |ctx| {
            if let Some(background) = background { draw_image(ctx, background); }
            for image in normal { draw_image(ctx, image); }
            Ok(())
        })
    }
}

impl RecordedScene {
    pub fn dimensions(&self) -> SceneDimensions { self.dimensions }

    pub fn draw(&self, context: &ID2D1DeviceContext, options: &DrawOptions) -> Result<(), NativeRenderError> {
        let device = win(unsafe { context.GetDevice() }, "draw GetDevice")?;
        let identity = win(device.cast::<IUnknown>(), "draw device canonical IUnknown")?;
        if identity != self.device_identity {
            return Err(NativeRenderError::diagnostic(NativeErrorKind::DeviceMismatch,
                "RecordedScene::draw", "recording belongs to a different Direct2D device"));
        }
        geometry::matrix(options.transform, "draw.transform")?;
        let clip = options.clip.map(|clip| geometry::rect(clip, "draw.clip")).transpose()?;
        let mut original = Matrix3x2::identity();
        unsafe { context.GetTransform(&mut original); }
        let combined = geometry::matrix(geometry::affine(original).compose(options.transform), "draw.combined_transform")?;
        let _restore = TransformGuard { context, original };
        let _clip = clip.as_ref().map(|clip| AxisClipGuard::push(context, clip));
        unsafe { context.SetTransform(&combined); }
        draw_image(context, &self.image);
        win(unsafe { context.Flush(None, None) }, "RecordedScene::draw Flush")
    }
}

struct TransformGuard<'a> { context: &'a ID2D1DeviceContext, original: Matrix3x2 }
impl Drop for TransformGuard<'_> {
    fn drop(&mut self) { unsafe { self.context.SetTransform(&self.original); } }
}

pub(super) struct AxisClipGuard<'a> { context: &'a ID2D1DeviceContext }
impl<'a> AxisClipGuard<'a> {
    pub(super) fn push(context: &'a ID2D1DeviceContext, clip: &D2D_RECT_F) -> Self {
        unsafe { context.PushAxisAlignedClip(clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE); }
        Self { context }
    }
}
impl Drop for AxisClipGuard<'_> {
    fn drop(&mut self) { unsafe { self.context.PopAxisAlignedClip(); } }
}

pub(super) fn draw_image(context: &ID2D1DeviceContext, image: &ID2D1Image) {
    unsafe { context.DrawImage(image, None, None, D2D1_INTERPOLATION_MODE_LINEAR, D2D1_COMPOSITE_MODE_SOURCE_OVER); }
}

pub(super) fn record<F>(context: &ID2D1DeviceContext, operation: &str, draw: F) -> Result<ID2D1Image, NativeRenderError>
where F: FnOnce(&ID2D1DeviceContext) -> Result<(), NativeRenderError> {
    let commands = win(unsafe { context.CreateCommandList() }, &format!("{operation}: CreateCommandList"))?;
    unsafe {
        context.SetTransform(&Matrix3x2::identity());
        context.SetTarget(&commands);
        context.BeginDraw();
    }
    // All scoped layer/clip guards unwind before EndDraw, including on Err.
    let result = draw(context);
    let ended = win(unsafe { context.EndDraw(None, None) }, &format!("{operation}: EndDraw"));
    unsafe {
        context.SetTarget(None::<&ID2D1Image>);
        context.SetTransform(&Matrix3x2::identity());
    }
    match (result, ended) {
        (Err(mut primary), Err(cleanup)) => { primary.cleanup.push(cleanup); return Err(primary); }
        (Err(primary), Ok(())) => return Err(primary),
        (Ok(()), Err(error)) => return Err(error),
        (Ok(()), Ok(())) => {},
    }
    win(unsafe { commands.Close() }, &format!("{operation}: command list Close"))?;
    win(commands.cast::<ID2D1Image>(), &format!("{operation}: command list image"))
}
