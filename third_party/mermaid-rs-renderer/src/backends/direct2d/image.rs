//! Decode only owned encoded bytes; no URI/file/SVG resource resolver.
use std::mem::ManuallyDrop;
use windows::Win32::Graphics::Direct2D::*;
use windows::Win32::Graphics::Direct2D::Common::*;
use windows::Win32::Graphics::Dxgi::Common::DXGI_FORMAT_B8G8R8A8_UNORM;
use windows::Win32::Graphics::Imaging::*;
use windows::Win32::System::Com::{CoCreateInstance, CLSCTX_INPROC_SERVER};
use windows_numerics::Matrix3x2;
use crate::scene::{Affine, ImageId, ImageMime, ImageNode, ImageResource, Rect};
use super::{geometry, native_error, paint, win, AxisClipGuard, NativeErrorKind,
    NativeRenderError, TransformGuard};

fn resource_win<T>(result: windows::core::Result<T>, operation: &str) -> Result<T, NativeRenderError> {
    result.map_err(|error| {
        let mut result = native_error(error, operation);
        if result.kind != NativeErrorKind::DeviceLost { result.kind = NativeErrorKind::InvalidResource; }
        result
    })
}

pub(super) fn prepare_resources(context: &ID2D1DeviceContext,
    resources: &[ImageResource]) -> Result<Vec<ID2D1Bitmap1>, NativeRenderError> {
    if resources.is_empty() { return Ok(Vec::new()); }
    let factory: IWICImagingFactory = win(unsafe {
        CoCreateInstance(&CLSID_WICImagingFactory, None::<&windows::core::IUnknown>, CLSCTX_INPROC_SERVER)
    }, "WIC factory activation (caller owns COM apartment)")?;
    let mut bitmaps = Vec::with_capacity(resources.len());
    for (index, resource) in resources.iter().enumerate() {
        let location = format!("images[{index}]");
        if resource.encoded.is_empty() || resource.encoded.len() > u32::MAX as usize {
            return Err(NativeRenderError::invalid_resource(format!("{location}: invalid encoded byte count")));
        }
        // Keep private memory alive through synchronous WIC decode and upload.
        let bytes = resource.encoded.to_vec();
        let expected = match resource.mime {
            ImageMime::Png => GUID_ContainerFormatPng,
            ImageMime::Jpeg => GUID_ContainerFormatJpeg,
            ImageMime::Gif => GUID_ContainerFormatGif,
            ImageMime::WebP => GUID_ContainerFormatWebp,
        };
        let stream = resource_win(unsafe { factory.CreateStream() }, &format!("{location}: WIC stream"))?;
        resource_win(unsafe { stream.InitializeFromMemory(&bytes) }, &format!("{location}: WIC memory stream"))?;
        // Select only the declared raster decoder. An auto-detecting factory
        // must never activate an installed SVG decoder for disguised bytes.
        let decoder = resource_win(unsafe { factory.CreateDecoder(&expected, std::ptr::null()) },
            &format!("{location}: declared raster WIC decoder"))?;
        resource_win(unsafe { decoder.Initialize(&stream, WICDecodeMetadataCacheOnLoad) },
            &format!("{location}: raster decoder initialization"))?;
        let container = resource_win(unsafe { decoder.GetContainerFormat() }, &format!("{location}: WIC container"))?;
        if container != expected {
            return Err(NativeRenderError::invalid_resource(format!("{location}: encoded container disagrees with MIME")));
        }
        let frames = resource_win(unsafe { decoder.GetFrameCount() }, &format!("{location}: WIC frame count"))?;
        if frames != 1 {
            return Err(NativeRenderError::unsupported(format!("{location}: {frames} frames require an explicit scene frame policy")));
        }
        let frame = resource_win(unsafe { decoder.GetFrame(0) }, &format!("{location}: WIC frame"))?;
        let mut width = 0; let mut height = 0;
        resource_win(unsafe { frame.GetSize(&mut width, &mut height) }, &format!("{location}: WIC dimensions"))?;
        if width == 0 || height == 0 || width != resource.pixel_width || height != resource.pixel_height {
            return Err(NativeRenderError::invalid_resource(format!("{location}: decoded dimensions disagree with retained dimensions")));
        }
        let converter = resource_win(unsafe { factory.CreateFormatConverter() }, &format!("{location}: WIC converter"))?;
        resource_win(unsafe { converter.Initialize(&frame, &GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone, None::<&IWICPalette>, 0.0, WICBitmapPaletteTypeCustom) },
            &format!("{location}: WIC premultiplied BGRA conversion"))?;
        let properties = D2D1_BITMAP_PROPERTIES1 {
            pixelFormat: D2D1_PIXEL_FORMAT { format: DXGI_FORMAT_B8G8R8A8_UNORM,
                alphaMode: D2D1_ALPHA_MODE_PREMULTIPLIED }, dpiX: 96.0, dpiY: 96.0,
            bitmapOptions: D2D1_BITMAP_OPTIONS_NONE, colorContext: ManuallyDrop::new(None),
        };
        bitmaps.push(win(unsafe { context.CreateBitmapFromWicBitmap(&converter, Some(&properties)) },
            &format!("{location}: D2D bitmap upload"))?);
    }
    Ok(bitmaps)
}

pub(super) struct PreparedImage {
    bitmap: ID2D1Bitmap1,
    destination: D2D_RECT_F,
    source: D2D_RECT_F,
    mapping: Affine,
    opacity: f32,
    context: String,
}

pub(super) fn prepare_node(node: &ImageNode, bitmaps: &[ID2D1Bitmap1],
    location: &str) -> Result<PreparedImage, NativeRenderError> {
    prepare(node.image, node.destination, node.source_to_destination, node.style.opacity, bitmaps, location)
}

pub(super) fn prepare(id: ImageId, destination: Rect, mapping: Affine, opacity: f64,
    bitmaps: &[ID2D1Bitmap1], location: &str) -> Result<PreparedImage, NativeRenderError> {
    let bitmap = bitmaps.get(id.0).ok_or_else(|| NativeRenderError::invalid_resource(
        format!("{location}: missing image {}", id.0)))?.clone();
    let size = unsafe { bitmap.GetPixelSize() };
    geometry::matrix(mapping, &format!("{location}.mapping"))?;
    Ok(PreparedImage { bitmap, destination: geometry::rect(destination, &format!("{location}.destination"))?,
        source: D2D_RECT_F { left: 0.0, top: 0.0, right: size.width as f32, bottom: size.height as f32 },
        mapping, opacity: geometry::number(opacity, &format!("{location}.opacity"))?, context: location.to_owned() })
}

impl PreparedImage {
    pub(super) fn draw(&self, context: &ID2D1DeviceContext) -> Result<(), NativeRenderError> {
        let mut original = Matrix3x2::identity();
        unsafe { context.GetTransform(&mut original); }
        let mapping = geometry::matrix(geometry::affine(original).compose(self.mapping), &self.context)?;
        let _restore = TransformGuard { context, original };
        let _clip = AxisClipGuard::push(context, &self.destination);
        let _layer = paint::LayerGuard::push(context, self.opacity, None);
        unsafe {
            context.SetTransform(&mapping);
            context.DrawBitmap(&self.bitmap, Some(&self.source), 1.0,
                D2D1_INTERPOLATION_MODE_LINEAR, None, None);
        }
        Ok(())
    }
}
