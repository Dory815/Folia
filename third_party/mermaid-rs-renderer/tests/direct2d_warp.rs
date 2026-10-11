// Root native validation. No window, no GPU requirement, and no skipped WARP failures.
#![cfg(windows)]
use mermaid_rs_renderer::backends::direct2d::{DrawOptions, NativeErrorKind, NativeRenderer};
use mermaid_rs_renderer::scene::*;
use windows::Win32::Foundation::HMODULE;
use windows::Win32::Graphics::Direct2D::Common::*;
use windows::Win32::Graphics::Direct2D::*;
use windows::Win32::Graphics::Direct3D::*;
use windows::Win32::Graphics::Direct3D11::*;
use windows::Win32::Graphics::Dxgi::Common::DXGI_FORMAT_B8G8R8A8_UNORM;
use windows::Win32::Graphics::Dxgi::IDXGIDevice;
use windows::Win32::System::Com::*;
use windows::core::Interface;
use windows_numerics::Matrix3x2;

struct Com;
impl Com {
    fn new() -> Self {
        unsafe {
            CoInitializeEx(None, COINIT_MULTITHREADED)
                .ok()
                .expect("COM initialization");
        }
        Self
    }
}
impl Drop for Com {
    fn drop(&mut self) {
        unsafe { CoUninitialize() }
    }
}
fn warp_device() -> ID2D1Device {
    unsafe {
        let mut d3d = None;
        D3D11CreateDevice(
            None,
            D3D_DRIVER_TYPE_WARP,
            HMODULE::default(),
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            Some(&[D3D_FEATURE_LEVEL_11_0]),
            D3D11_SDK_VERSION,
            Some(&mut d3d),
            None,
            None,
        )
        .expect("Actual WARP device creation");
        let dxgi = d3d.unwrap().cast::<IDXGIDevice>().unwrap();
        let factory: ID2D1Factory1 =
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, None).unwrap();
        factory.CreateDevice(&dxgi).unwrap()
    }
}
fn bounds() -> Rect {
    Rect {
        x: 0.0,
        y: 0.0,
        width: 100.0,
        height: 80.0,
    }
}
fn builder() -> SceneBuilder {
    SceneBuilder::new(
        SceneDimensions {
            width: 100.0,
            height: 80.0,
            viewbox: bounds(),
        },
        RootViewport {
            clip: bounds(),
            aspect_ratio: AspectRatioPolicy::None,
            viewbox_to_viewport: Affine::IDENTITY,
        },
    )
}
fn rect(x: f64, y: f64, w: f64, h: f64, color: Rgba) -> SceneNode {
    SceneNode::Shape(ShapeNode {
        geometry: ShapeGeometry::Rect {
            rect: Rect {
                x,
                y,
                width: w,
                height: h,
            },
            rx: 0.0,
            ry: 0.0,
        },
        style: PaintStyle {
            fill: Paint::Solid(color),
            ..Default::default()
        },
        metadata: Default::default(),
    })
}
fn rgba(r: f64, g: f64, b: f64) -> Rgba {
    Rgba { r, g, b, a: 1.0 }
}
unsafe fn bitmap(ctx: &ID2D1DeviceContext, dpi: f32, cpu: bool) -> ID2D1Bitmap1 {
    unsafe {
        ctx.CreateBitmap(
            D2D_SIZE_U {
                width: (100.0 * dpi / 96.0).ceil() as u32,
                height: (80.0 * dpi / 96.0).ceil() as u32,
            },
            None,
            0,
            &D2D1_BITMAP_PROPERTIES1 {
                pixelFormat: D2D1_PIXEL_FORMAT {
                    format: DXGI_FORMAT_B8G8R8A8_UNORM,
                    alphaMode: D2D1_ALPHA_MODE_PREMULTIPLIED,
                },
                dpiX: dpi,
                dpiY: dpi,
                bitmapOptions: if cpu {
                    D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW
                } else {
                    D2D1_BITMAP_OPTIONS_TARGET
                },
                colorContext: Default::default(),
            },
        )
        .unwrap()
    }
}
unsafe fn pixels(ctx: &ID2D1DeviceContext, target: &ID2D1Bitmap1, dpi: f32) -> (Vec<u8>, usize) {
    unsafe {
        let saved = ctx.GetTarget().unwrap();
        ctx.SetTarget(None::<&ID2D1Image>);
        let read = bitmap(ctx, dpi, true);
        read.CopyFromBitmap(None, target, None).unwrap();
        let mapped = read.Map(D2D1_MAP_OPTIONS_READ).unwrap();
        let height = (80.0 * dpi / 96.0).ceil() as usize;
        let pitch = mapped.pitch as usize;
        let copied = std::slice::from_raw_parts(mapped.bits, pitch * height).to_vec();
        read.Unmap().unwrap();
        ctx.SetTarget(&saved);
        (copied, pitch)
    }
}
fn sample(data: &[u8], pitch: usize, dpi: f32, x: f32, y: f32) -> [u8; 4] {
    let x = (x * dpi / 96.0).floor() as usize;
    let y = (y * dpi / 96.0).floor() as usize;
    data[y * pitch + x * 4..y * pitch + x * 4 + 4]
        .try_into()
        .unwrap()
}
fn near(actual: [u8; 4], expected: [u8; 4], context: &str) {
    for (a, e) in actual.into_iter().zip(expected) {
        assert!(
            (i32::from(a) - i32::from(e)).abs() <= 2,
            "{context}: {actual:?} vs {expected:?}"
        );
    }
}

#[test]
fn native_painter_order_and_group_opacity_at_four_dpi() {
    let _com = Com::new();
    let device = warp_device();
    let mut renderer = NativeRenderer::new(&device).unwrap();
    let mut b = builder();
    b.push_node(rect(0.0, 0.0, 100.0, 80.0, rgba(1.0, 1.0, 1.0)));
    b.push_node(SceneNode::Group(GroupNode {
        children: vec![
            rect(10.0, 10.0, 30.0, 30.0, rgba(1.0, 0.0, 0.0)),
            rect(25.0, 10.0, 30.0, 30.0, rgba(0.0, 0.0, 1.0)),
        ],
        transform: Affine::IDENTITY,
        clip: None,
        layer: LayerStyle {
            opacity: 0.5,
            blend: BlendMode::Normal,
        },
        metadata: Default::default(),
    }));
    let scene = b.finish().unwrap();
    let recorded = renderer.record_scene(&scene).unwrap();
    drop(scene);
    drop(renderer);
    for dpi in [96.0, 120.0, 144.0, 192.0] {
        unsafe {
            let ctx = device
                .CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE)
                .unwrap();
            let target = bitmap(&ctx, dpi, false);
            ctx.SetDpi(dpi, dpi);
            ctx.SetTarget(&target);
            ctx.BeginDraw();
            ctx.Clear(Some(&D2D1_COLOR_F {
                r: 0.0,
                g: 0.0,
                b: 0.0,
                a: 0.0,
            }));
            recorded
                .draw(
                    &ctx,
                    &DrawOptions {
                        transform: Affine::IDENTITY,
                        clip: None,
                    },
                )
                .unwrap();
            ctx.EndDraw(None, None).unwrap();
            let (data, pitch) = pixels(&ctx, &target, dpi);
            near(
                sample(&data, pitch, dpi, 4.0, 4.0),
                [255, 255, 255, 255],
                "background must actually render",
            );
            near(
                sample(&data, pitch, dpi, 18.0, 20.0),
                [128, 128, 255, 255],
                "isolated half-opacity red",
            );
            near(
                sample(&data, pitch, dpi, 30.0, 20.0),
                [255, 128, 128, 255],
                "overlap applies group opacity once",
            );
        }
    }
}

#[test]
fn invalid_resource_and_conversion_fail_without_poisoning_recorder() {
    let _com = Com::new();
    let device = warp_device();
    let mut renderer = NativeRenderer::new(&device).unwrap();
    let mut b = builder();
    let image = b.add_image(ImageResource {
        encoded: std::sync::Arc::from([137u8, 80, 78, 71]),
        mime: ImageMime::Png,
        pixel_width: 1,
        pixel_height: 1,
    });
    b.push_node(SceneNode::Image(ImageNode {
        image,
        destination: Rect {
            x: 1.0,
            y: 2.0,
            width: 4.0,
            height: 5.0,
        },
        source_to_destination: Affine {
            a: 4.0,
            d: 5.0,
            e: 1.0,
            f: 2.0,
            ..Affine::IDENTITY
        },
        aspect_ratio: AspectRatioPolicy::None,
        style: Default::default(),
        metadata: Default::default(),
    }));
    let malformed = b.finish().unwrap();
    assert_eq!(
        renderer.record_scene(&malformed).unwrap_err().kind,
        NativeErrorKind::InvalidResource
    );
    let mut b = builder();
    b.push_node(SceneNode::Shape(ShapeNode {
        geometry: ShapeGeometry::Circle {
            center: Point {
                x: f64::MAX,
                y: 0.0,
            },
            radius: 1.0,
        },
        style: Default::default(),
        metadata: Default::default(),
    }));
    let overflow = b.finish().unwrap();
    assert_eq!(
        renderer.record_scene(&overflow).unwrap_err().kind,
        NativeErrorKind::InvalidNumber
    );
    let mut b = builder();
    b.push_node(rect(0.0, 0.0, 100.0, 80.0, rgba(0.0, 1.0, 0.0)));
    let good = b.finish().unwrap();
    let recorded = renderer
        .record_scene(&good)
        .expect("recording context reusable after diagnosed failure");
    unsafe {
        let ctx = device
            .CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE)
            .unwrap();
        let target = bitmap(&ctx, 96.0, false);
        ctx.SetTarget(&target);
        ctx.BeginDraw();
        recorded
            .draw(
                &ctx,
                &DrawOptions {
                    transform: Affine::IDENTITY,
                    clip: None,
                },
            )
            .unwrap();
        ctx.EndDraw(None, None).unwrap();
        let (data, pitch) = pixels(&ctx, &target, 96.0);
        near(
            sample(&data, pitch, 96.0, 20.0, 20.0),
            [0, 255, 0, 255],
            "failure recovery must really draw",
        );
    }
}

#[test]
fn retained_font_contours_use_completed_run_origin_and_em_size() {
    let _com = Com::new();
    let device = warp_device();
    let mut renderer = NativeRenderer::new(&device).unwrap();
    let bytes: std::sync::Arc<[u8]> =
        std::fs::read(std::env::var("FOLIA_SCENE_TEST_FONT").expect("root fixed font fixture"))
            .unwrap()
            .into();
    let face = ttf_parser::Face::parse(&bytes, 0).unwrap();
    let glyph = face.glyph_index('A').unwrap();
    let glyph_bounds = face.glyph_bounding_box(glyph).unwrap();
    let scale = 24.0 / f64::from(face.units_per_em());
    let expected = [
        40.0 + f64::from(glyph_bounds.x_min) * scale,
        50.0 - f64::from(glyph_bounds.y_max) * scale,
        40.0 + f64::from(glyph_bounds.x_max) * scale,
        50.0 - f64::from(glyph_bounds.y_min) * scale,
    ];
    let mut b = builder();
    b.push_node(rect(0.0, 0.0, 100.0, 80.0, rgba(1.0, 1.0, 1.0)));
    let id = b.add_font(FontResource {
        bytes: bytes.clone(),
        face_index: 0,
        identity: "root-fixed-arial".into(),
    });
    b.push_node(SceneNode::Text(TextNode {
        text: "A".into(),
        font: FontSpec {
            families: vec!["Arial".into()],
            size: 48.0,
            weight: 400,
            stretch: 100.0,
            style: FontStyle::Normal,
        },
        origin: Point { x: 12.0, y: 50.0 },
        baseline: TextBaseline::Alphabetic,
        anchor: TextAnchor::Middle,
        runs: vec![GlyphRun {
            font: id,
            origin: Point { x: 40.0, y: 50.0 },
            em_size: 24.0,
            glyphs: vec![PositionedGlyph {
                glyph_id: glyph.0,
                advance: 16.0,
                offset: Point::default(),
                cluster: 0..1,
                appearance: GlyphAppearance::FontOutline,
            }],
            source_range: 0..1,
            bidi_level: 0,
            sideways: false,
        }],
        style: PaintStyle {
            fill: Paint::Solid(rgba(0.0, 0.0, 0.0)),
            ..Default::default()
        },
        metadata: Default::default(),
    }));
    let scene = b.finish().unwrap();
    let recorded = renderer.record_scene(&scene).unwrap();
    drop(scene);
    drop(renderer);
    drop(bytes);
    for dpi in [96.0, 120.0, 144.0, 192.0] {
        unsafe {
            let ctx = device
                .CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE)
                .unwrap();
            let target = bitmap(&ctx, dpi, false);
            ctx.SetDpi(dpi, dpi);
            ctx.SetTarget(&target);
            ctx.BeginDraw();
            recorded
                .draw(
                    &ctx,
                    &DrawOptions {
                        transform: Affine::IDENTITY,
                        clip: None,
                    },
                )
                .unwrap();
            ctx.EndDraw(None, None).unwrap();
            let (data, pitch) = pixels(&ctx, &target, dpi);
            let width = (100.0 * dpi / 96.0).ceil() as usize;
            let height = (80.0 * dpi / 96.0).ceil() as usize;
            let mut found = [usize::MAX, usize::MAX, 0, 0];
            let mut count = 0;
            for y in 0..height {
                for x in 0..width {
                    let p = &data[y * pitch + x * 4..y * pitch + x * 4 + 4];
                    if p[0] < 240 && p[1] < 240 && p[2] < 240 && p[3] > 240 {
                        found[0] = found[0].min(x);
                        found[1] = found[1].min(y);
                        found[2] = found[2].max(x + 1);
                        found[3] = found[3].max(y + 1);
                        count += 1;
                    }
                }
            }
            assert!(count > 10, "actual retained font outline must render");
            for (i, (actual, expected)) in found.into_iter().zip(expected).enumerate() {
                assert!(
                    (actual as f64 - expected * f64::from(dpi) / 96.0).abs() <= 2.0,
                    "glyph bound {i} at {dpi}dpi: {actual} vs {expected}"
                );
            }
        }
    }
}
#[test]
fn native_multiply_composites_completed_background() {
    let _com = Com::new();
    let device = warp_device();
    let mut renderer = NativeRenderer::new(&device).unwrap();
    let mut b = builder();
    b.push_node(rect(0.0, 0.0, 100.0, 80.0, rgba(0.2, 0.4, 0.8)));
    b.push_node(SceneNode::Group(GroupNode {
        children: vec![rect(10.0, 10.0, 30.0, 30.0, rgba(0.8, 0.6, 0.2))],
        transform: Affine::IDENTITY,
        clip: None,
        layer: LayerStyle {
            opacity: 1.0,
            blend: BlendMode::Multiply,
        },
        metadata: Default::default(),
    }));
    let scene = b.finish().unwrap();
    let recorded = renderer.record_scene(&scene).unwrap();
    unsafe {
        let ctx = device
            .CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE)
            .unwrap();
        let target = bitmap(&ctx, 96.0, false);
        ctx.SetTarget(&target);
        ctx.BeginDraw();
        ctx.Clear(Some(&D2D1_COLOR_F {
            r: 0.0,
            g: 0.0,
            b: 0.0,
            a: 0.0,
        }));
        recorded
            .draw(
                &ctx,
                &DrawOptions {
                    transform: Affine::IDENTITY,
                    clip: None,
                },
            )
            .unwrap();
        ctx.EndDraw(None, None).unwrap();
        let (data, pitch) = pixels(&ctx, &target, 96.0);
        near(
            sample(&data, pitch, 96.0, 20.0, 20.0),
            [41, 61, 41, 255],
            "actual multiply effect",
        );
    }
}
#[test]
fn caller_state_and_device_mismatch_are_preserved() {
    let _com = Com::new();
    let device = warp_device();
    let scene = builder().finish().unwrap();
    let mut renderer = NativeRenderer::new(&device).unwrap();
    let recorded = renderer.record_scene(&scene).unwrap();
    unsafe {
        let ctx = device
            .CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE)
            .unwrap();
        let target = bitmap(&ctx, 96.0, false);
        ctx.SetTarget(&target);
        let transform = Matrix3x2 {
            M11: 1.0,
            M12: 0.2,
            M21: 0.3,
            M22: 1.0,
            M31: 7.0,
            M32: 4.0,
        };
        ctx.SetTransform(&transform);
        ctx.BeginDraw();
        recorded
            .draw(
                &ctx,
                &DrawOptions {
                    transform: Affine {
                        e: 3.0,
                        f: 5.0,
                        ..Affine::IDENTITY
                    },
                    clip: Some(Rect {
                        x: 2.0,
                        y: 3.0,
                        width: 40.0,
                        height: 30.0,
                    }),
                },
            )
            .unwrap();
        let mut actual = Matrix3x2::default();
        ctx.GetTransform(&mut actual);
        assert_eq!(actual, transform);
        assert_eq!(
            ctx.GetTarget()
                .unwrap()
                .cast::<windows::core::IUnknown>()
                .unwrap()
                .as_raw(),
            target.cast::<windows::core::IUnknown>().unwrap().as_raw()
        );
        ctx.EndDraw(None, None).unwrap();
        let other = warp_device();
        let ctx = other
            .CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE)
            .unwrap();
        let target = bitmap(&ctx, 96.0, false);
        ctx.SetTarget(&target);
        ctx.BeginDraw();
        assert_eq!(
            recorded
                .draw(
                    &ctx,
                    &DrawOptions {
                        transform: Affine::IDENTITY,
                        clip: None
                    }
                )
                .unwrap_err()
                .kind,
            NativeErrorKind::DeviceMismatch
        );
        ctx.EndDraw(None, None).unwrap();
    }
}

fn native_pixels(scene: &DiagramScene, dpi: f32) -> (Vec<u8>, usize) {
    let device = warp_device();
    let mut renderer = NativeRenderer::new(&device).unwrap();
    let recorded = renderer.record_scene(scene).unwrap();
    unsafe {
        let ctx = device
            .CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE)
            .unwrap();
        let target = bitmap(&ctx, dpi, false);
        ctx.SetDpi(dpi, dpi);
        ctx.SetTarget(&target);
        ctx.BeginDraw();
        ctx.Clear(Some(&D2D1_COLOR_F {
            r: 0.0,
            g: 0.0,
            b: 0.0,
            a: 0.0,
        }));
        recorded.draw(&ctx, &DrawOptions::default()).unwrap();
        ctx.EndDraw(None, None).unwrap();
        pixels(&ctx, &target, dpi)
    }
}
fn white_builder() -> SceneBuilder {
    let mut b = builder();
    b.push_node(rect(0.0, 0.0, 100.0, 80.0, rgba(1.0, 1.0, 1.0)));
    b
}
fn shape(geometry: ShapeGeometry, style: PaintStyle) -> SceneNode {
    SceneNode::Shape(ShapeNode {
        geometry,
        style,
        metadata: Default::default(),
    })
}
fn red() -> PaintStyle {
    PaintStyle {
        fill: Paint::Solid(rgba(1.0, 0.0, 0.0)),
        ..Default::default()
    }
}
fn point(x: f64, y: f64) -> Point {
    Point { x, y }
}
fn hollow_path(rule: FillRule) -> ShapeGeometry {
    let mut segments = Vec::new();
    // Same winding direction: only EvenOdd may make the inner contour a hole.
    for (a, b) in [(10.0, 40.0), (20.0, 30.0)] {
        segments.extend([
            PathSegment::MoveTo(point(a, a)),
            PathSegment::LineTo(point(b, a)),
            PathSegment::LineTo(point(b, b)),
            PathSegment::LineTo(point(a, b)),
            PathSegment::Close,
        ]);
    }
    ShapeGeometry::Path(Path::new(segments, rule))
}

#[test]
fn path_fill_rules_curves_and_arc_direction_produce_real_interiors() {
    let _com = Com::new();
    let cases = [
        (
            hollow_path(FillRule::EvenOdd),
            vec![(15.0, 15.0, true), (25.0, 25.0, false)],
        ),
        (
            hollow_path(FillRule::NonZero),
            vec![(15.0, 15.0, true), (25.0, 25.0, true)],
        ),
        (
            ShapeGeometry::Path(Path::new(
                vec![
                    PathSegment::MoveTo(point(10.0, 40.0)),
                    PathSegment::ArcTo {
                        rx: 15.0,
                        ry: 15.0,
                        rotation_degrees: 0.0,
                        large_arc: false,
                        sweep: true,
                        end: point(40.0, 40.0),
                    },
                    PathSegment::Close,
                ],
                FillRule::NonZero,
            )),
            vec![(25.0, 30.0, true), (25.0, 50.0, false)],
        ),
        (
            ShapeGeometry::Path(Path::new(
                vec![
                    PathSegment::MoveTo(point(10.0, 70.0)),
                    PathSegment::QuadraticTo {
                        control: point(25.0, 40.0),
                        end: point(40.0, 70.0),
                    },
                    PathSegment::Close,
                ],
                FillRule::NonZero,
            )),
            vec![(25.0, 63.0, true), (25.0, 50.0, false)],
        ),
        (
            ShapeGeometry::Path(Path::new(
                vec![
                    PathSegment::MoveTo(point(10.0, 70.0)),
                    PathSegment::CubicTo {
                        control1: point(10.0, 40.0),
                        control2: point(40.0, 40.0),
                        end: point(40.0, 70.0),
                    },
                    PathSegment::Close,
                ],
                FillRule::NonZero,
            )),
            vec![(25.0, 55.0, true), (25.0, 40.0, false)],
        ),
        (
            ShapeGeometry::Circle {
                center: point(25.0, 25.0),
                radius: 10.0,
            },
            vec![(25.0, 25.0, true), (37.0, 25.0, false)],
        ),
        (
            ShapeGeometry::Ellipse {
                center: point(25.0, 25.0),
                rx: 15.0,
                ry: 5.0,
            },
            vec![(37.0, 25.0, true), (25.0, 33.0, false)],
        ),
        (
            ShapeGeometry::Rect {
                rect: Rect {
                    x: 10.0,
                    y: 10.0,
                    width: 30.0,
                    height: 30.0,
                },
                rx: 6.0,
                ry: 10.0,
            },
            vec![(11.0, 11.0, false), (25.0, 11.0, true)],
        ),
        (
            ShapeGeometry::Polygon(vec![
                point(10.0, 40.0),
                point(25.0, 10.0),
                point(40.0, 40.0),
            ]),
            vec![(25.0, 30.0, true), (12.0, 15.0, false)],
        ),
        // SVG polyline fill implicitly closes, while its stroke remains open.
        (
            ShapeGeometry::Polyline(vec![
                point(10.0, 40.0),
                point(25.0, 10.0),
                point(40.0, 40.0),
            ]),
            vec![(25.0, 30.0, true), (12.0, 15.0, false)],
        ),
    ];
    for (index, (geometry, checks)) in cases.into_iter().enumerate() {
        let mut b = white_builder();
        b.push_node(shape(geometry, red()));
        let scene = b.finish().unwrap();
        for dpi in [96.0, 120.0, 144.0, 192.0] {
            let (data, pitch) = native_pixels(&scene, dpi);
            for &(x, y, inside) in &checks {
                near(
                    sample(&data, pitch, dpi, x, y),
                    if inside {
                        [0, 0, 255, 255]
                    } else {
                        [255, 255, 255, 255]
                    },
                    &format!("primitive {index} {dpi}dpi ({x},{y})"),
                );
            }
        }
    }
}

#[test]
fn absolute_dash_units_odd_repetition_and_zero_stroke_are_preserved() {
    let _com = Com::new();
    for (dash, width, checks) in [
        (
            vec![8.0, 4.0],
            4.0,
            vec![(14.0, true), (20.0, false), (26.0, true)],
        ),
        (
            vec![8.0],
            4.0,
            vec![(14.0, true), (22.0, false), (30.0, true)],
        ),
        (
            vec![8.0, 4.0, 2.0],
            4.0,
            vec![
                (14.0, true),
                (20.0, false),
                (23.0, true),
                (28.0, false),
                (34.0, true),
                (37.0, false),
                (42.0, true),
            ],
        ),
        (
            vec![0.0, 0.0],
            4.0,
            vec![(14.0, true), (22.0, true), (30.0, true)],
        ),
        (
            vec![8.0, 4.0],
            0.0,
            vec![(14.0, false), (22.0, false), (30.0, false)],
        ),
    ] {
        let mut b = white_builder();
        b.push_node(shape(
            ShapeGeometry::Line {
                start: point(10.0, 40.0),
                end: point(80.0, 40.0),
            },
            PaintStyle {
                stroke: Some(Stroke {
                    paint: Paint::Solid(rgba(1.0, 0.0, 0.0)),
                    width,
                    dash,
                    dash_offset: 0.0,
                    cap: LineCap::Butt,
                    join: LineJoin::Miter,
                    miter_limit: 4.0,
                }),
                ..Default::default()
            },
        ));
        let scene = b.finish().unwrap();
        for dpi in [96.0, 120.0, 144.0, 192.0] {
            let (data, pitch) = native_pixels(&scene, dpi);
            for &(x, on) in &checks {
                near(
                    sample(&data, pitch, dpi, x, 40.0),
                    if on {
                        [0, 0, 255, 255]
                    } else {
                        [255, 255, 255, 255]
                    },
                    "absolute dash/zero stroke",
                );
            }
        }
    }
}

#[test]
fn gradient_spaces_transform_order_and_spread_use_completed_local_bounds() {
    let _com = Com::new();
    for (units, transform, spread, start_x, end_x) in [
        (
            GradientUnits::ObjectBoundingBox,
            Affine {
                e: 0.25,
                ..Affine::IDENTITY
            },
            SpreadMode::Pad,
            20.0,
            60.0,
        ),
        (
            GradientUnits::UserSpace,
            Affine {
                a: 20.0,
                e: 10.0,
                ..Affine::IDENTITY
            },
            SpreadMode::Pad,
            10.0,
            30.0,
        ),
        (
            GradientUnits::UserSpace,
            Affine {
                a: 20.0,
                e: 10.0,
                ..Affine::IDENTITY
            },
            SpreadMode::Repeat,
            10.0,
            30.0,
        ),
        (
            GradientUnits::UserSpace,
            Affine {
                a: 20.0,
                e: 10.0,
                ..Affine::IDENTITY
            },
            SpreadMode::Reflect,
            10.0,
            30.0,
        ),
    ] {
        let mut b = white_builder();
        let id = b.add_gradient(LinearGradient {
            units,
            transform,
            start: point(0.0, 0.0),
            end: point(1.0, 0.0),
            stops: vec![
                GradientStop {
                    offset: 0.0,
                    color: rgba(1.0, 0.0, 0.0),
                    opacity: 1.0,
                },
                GradientStop {
                    offset: 1.0,
                    color: rgba(0.0, 0.0, 1.0),
                    opacity: 1.0,
                },
            ],
            spread,
        });
        b.push_node(shape(
            ShapeGeometry::Rect {
                rect: Rect {
                    x: 10.0,
                    y: 10.0,
                    width: 40.0,
                    height: 30.0,
                },
                rx: 0.0,
                ry: 0.0,
            },
            PaintStyle {
                fill: Paint::LinearGradient(id),
                ..Default::default()
            },
        ));
        let scene = b.finish().unwrap();
        for dpi in [96.0, 120.0, 144.0, 192.0] {
            let (data, pitch) = native_pixels(&scene, dpi);
            for x in [15.0f32, 25.0, 40.0] {
                let local_x = (f64::from((x * dpi / 96.0).floor()) + 0.5) * 96.0 / f64::from(dpi);
                let t = (local_x - start_x) / (end_x - start_x);
                let t = match spread {
                    SpreadMode::Pad => t.clamp(0.0, 1.0),
                    SpreadMode::Repeat => t.rem_euclid(1.0),
                    SpreadMode::Reflect => {
                        let r = t.rem_euclid(2.0);
                        if r <= 1.0 { r } else { 2.0 - r }
                    }
                };
                near(
                    sample(&data, pitch, dpi, x, 20.0),
                    [
                        (255.0 * t).round() as u8,
                        0,
                        (255.0 * (1.0 - t)).round() as u8,
                        255,
                    ],
                    "gradient mapping/spread",
                );
            }
        }
    }
}

#[test]
fn nested_group_geometric_clip_and_viewport_mapping_preserve_clip_spaces() {
    let _com = Com::new();
    let mut b = white_builder();
    b.push_node(SceneNode::Group(GroupNode {
        children: vec![rect(0.0, 0.0, 30.0, 30.0, rgba(1.0, 0.0, 0.0))],
        transform: Affine {
            a: 2.0,
            d: 1.0,
            e: 10.0,
            f: 10.0,
            ..Affine::IDENTITY
        },
        clip: Some(Clip {
            geometry: ShapeGeometry::Circle {
                center: point(10.0, 10.0),
                radius: 5.0,
            },
            transform: Affine {
                e: 5.0,
                ..Affine::IDENTITY
            },
        }),
        layer: Default::default(),
        metadata: Default::default(),
    }));
    let scene = b.finish().unwrap();
    for dpi in [96.0, 120.0, 144.0, 192.0] {
        let (data, pitch) = native_pixels(&scene, dpi);
        near(
            sample(&data, pitch, dpi, 40.0, 20.0),
            [0, 0, 255, 255],
            "nested clip translated then group scale",
        );
        near(
            sample(&data, pitch, dpi, 20.0, 20.0),
            [255, 255, 255, 255],
            "clip must not lose local translation",
        );
        near(
            sample(&data, pitch, dpi, 40.0, 30.0),
            [255, 255, 255, 255],
            "clip y radius preserved",
        );
    }
    let mut b = SceneBuilder::new(
        SceneDimensions {
            width: 100.0,
            height: 80.0,
            viewbox: bounds(),
        },
        RootViewport {
            clip: Rect {
                x: 20.0,
                y: 10.0,
                width: 30.0,
                height: 25.0,
            },
            aspect_ratio: AspectRatioPolicy::None,
            viewbox_to_viewport: Affine {
                a: 2.0,
                d: 2.0,
                e: 10.0,
                f: 5.0,
                ..Affine::IDENTITY
            },
        },
    );
    b.push_node(rect(0.0, 0.0, 100.0, 80.0, rgba(0.0, 1.0, 0.0)));
    let scene = b.finish().unwrap();
    let (data, pitch) = native_pixels(&scene, 96.0);
    near(
        sample(&data, pitch, 96.0, 30.0, 20.0),
        [0, 255, 0, 255],
        "root clip inside",
    );
    near(
        sample(&data, pitch, 96.0, 15.0, 20.0),
        [0, 0, 0, 0],
        "root clip applied before viewBox mapping",
    );
    near(
        sample(&data, pitch, 96.0, 60.0, 20.0),
        [0, 0, 0, 0],
        "root clip remains viewport sized",
    );
}

#[test]
fn both_source_c4_images_decode_owned_bytes_and_clip_without_refitting() {
    let _com = Com::new();
    for (encoded, rgb) in [
        (
            &include_bytes!("fixtures/c4_person_icon.png")[..],
            &include_bytes!("fixtures/c4_person_icon.rgb")[..],
        ),
        (
            &include_bytes!("fixtures/c4_external_person_icon.png")[..],
            &include_bytes!("fixtures/c4_external_person_icon.rgb")[..],
        ),
    ] {
        let mut b = builder();
        let id = b.add_image(ImageResource {
            encoded: encoded.into(),
            mime: ImageMime::Png,
            pixel_width: 48,
            pixel_height: 48,
        });
        b.push_node(SceneNode::Image(ImageNode {
            image: id,
            destination: Rect {
                x: 10.0,
                y: 10.0,
                width: 20.0,
                height: 20.0,
            },
            source_to_destination: Affine {
                e: 10.0,
                f: 10.0,
                ..Affine::IDENTITY
            },
            aspect_ratio: AspectRatioPolicy::Slice(AspectAlign::XMaxYMax),
            style: Default::default(),
            metadata: Default::default(),
        }));
        let scene = b.finish().unwrap();
        for dpi in [96.0, 120.0, 144.0, 192.0] {
            let (data, pitch) = native_pixels(&scene, dpi);
            near(
                sample(&data, pitch, dpi, 35.0, 20.0),
                [0, 0, 0, 0],
                "image destination clip must truncate original mapping",
            );
            for y in [12.0f32, 17.0, 22.0, 27.0] {
                for x in [12.0f32, 17.0, 22.0, 27.0] {
                    // Independent source PNG RGB pixels, bilinear interpolation at native pixel centers.
                    let sx = (f64::from((x * dpi / 96.0).floor()) + 0.5) * 96.0 / f64::from(dpi)
                        - 10.0
                        - 0.5;
                    let sy = (f64::from((y * dpi / 96.0).floor()) + 0.5) * 96.0 / f64::from(dpi)
                        - 10.0
                        - 0.5;
                    let ix = sx.floor() as usize;
                    let iy = sy.floor() as usize;
                    let fx = sx - ix as f64;
                    let fy = sy - iy as f64;
                    let mut expected = [0u8, 0, 0, 255];
                    for c in 0..3 {
                        let get = |dx, dy| f64::from(rgb[((iy + dy) * 48 + ix + dx) * 3 + c]);
                        let value = get(0, 0) * (1.0 - fx) * (1.0 - fy)
                            + get(1, 0) * fx * (1.0 - fy)
                            + get(0, 1) * (1.0 - fx) * fy
                            + get(1, 1) * fx * fy;
                        expected[2 - c] = value.round() as u8;
                    }
                    near(
                        sample(&data, pitch, dpi, x, y),
                        expected,
                        "source C4 RGB pixel and completed mapping",
                    );
                }
            }
        }
    }
}
