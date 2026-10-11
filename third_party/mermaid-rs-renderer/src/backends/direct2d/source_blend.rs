//! Private two-input GPU effects for Source nonseparable blend modes.
use std::sync::{Mutex, OnceLock};
use windows::core::{implement, ComObjectInterface, GUID, HRESULT, IUnknown, Interface, OutRef, PCSTR, PCWSTR, Ref};
use windows::Win32::Foundation::RECT;
use windows::Win32::Graphics::Direct2D::*;
use windows::Win32::Graphics::Direct3D::{D3D_SHADER_MACRO, ID3DBlob, ID3DInclude};
use windows::Win32::Graphics::Direct3D::Fxc::{D3DCompile, D3DCOMPILE_ENABLE_STRICTNESS,
    D3DCOMPILE_IEEE_STRICTNESS, D3DCOMPILE_OPTIMIZATION_LEVEL0};
use crate::scene::BlendMode;
use super::{win, NativeErrorKind, NativeRenderError};

const EFFECT_IDS: [GUID; 4] = [
    GUID::from_u128(0xe3e05271_5946_4ea9_9432_abcd00000011),
    GUID::from_u128(0xe3e05271_5946_4ea9_9432_abcd00000012),
    GUID::from_u128(0xe3e05271_5946_4ea9_9432_abcd00000013),
    GUID::from_u128(0xe3e05271_5946_4ea9_9432_abcd00000014),
];
const SHADER_IDS: [GUID; 4] = [
    GUID::from_u128(0xe3e05271_5946_4ea9_9432_abcd00000021),
    GUID::from_u128(0xe3e05271_5946_4ea9_9432_abcd00000022),
    GUID::from_u128(0xe3e05271_5946_4ea9_9432_abcd00000023),
    GUID::from_u128(0xe3e05271_5946_4ea9_9432_abcd00000024),
];
const NAMES: [&str; 4] = ["Hue", "Saturation", "Color", "Luminosity"];
const CALLBACKS: [PD2D1_EFFECT_FACTORY; 4] = [
    Some(create_hue), Some(create_saturation), Some(create_color), Some(create_luminosity),
];
const E_INVALIDARG: HRESULT = HRESULT(0x80070057_u32 as i32);
const E_NOTIMPL: HRESULT = HRESULT(0x80004001_u32 as i32);
const E_POINTER: HRESULT = HRESULT(0x80004003_u32 as i32);
static REGISTRATION: Mutex<()> = Mutex::new(());
static SHADERS: OnceLock<Result<[Vec<u8>; 4], NativeRenderError>> = OnceLock::new();

fn shader_bytes() -> Result<&'static [Vec<u8>; 4], NativeRenderError> {
    SHADERS.get_or_init(|| {
        Ok([compile_shader(0)?, compile_shader(1)?, compile_shader(2)?, compile_shader(3)?])
    }).as_ref().map_err(Clone::clone)
}

fn compile_shader(kernel: usize) -> Result<Vec<u8>, NativeRenderError> {
    let source = include_str!("source_blend.hlsl");
    let definitions: [&[u8]; 4] = [b"0\0", b"1\0", b"2\0", b"3\0"];
    let macros = [
        D3D_SHADER_MACRO { Name: PCSTR(b"SOURCE_BLEND_KERNEL\0".as_ptr()),
            Definition: PCSTR(definitions[kernel].as_ptr()) },
        D3D_SHADER_MACRO { Name: PCSTR::null(), Definition: PCSTR::null() },
    ];
    let mut code: Option<ID3DBlob> = None;
    let mut diagnostics: Option<ID3DBlob> = None;
    let result = unsafe { D3DCompile(source.as_ptr().cast(), source.len(), PCSTR::null(),
        Some(macros.as_ptr()), None::<&ID3DInclude>, PCSTR(b"main\0".as_ptr()),
        PCSTR(b"ps_4_0\0".as_ptr()), D3DCOMPILE_ENABLE_STRICTNESS |
        D3DCOMPILE_IEEE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL0, 0,
        &mut code, Some(&mut diagnostics)) };
    if let Err(error) = result {
        let bytes = diagnostics.as_ref().map(blob_bytes).unwrap_or_default();
        let hex = bytes.iter().map(|byte| format!("{byte:02x}")).collect::<String>();
        return Err(NativeRenderError { kind: NativeErrorKind::NativeFailure,
            hresult: Some(error.code()), operation: format!("D3DCompile {}", NAMES[kernel]),
            context: format!("{}; diagnostic bytes (hex): {hex}", error), cleanup: Vec::new() });
    }
    let code = code.ok_or_else(|| NativeRenderError::invalid_resource(
        format!("D3DCompile {} returned no bytecode", NAMES[kernel])))?;
    Ok(blob_bytes(&code))
}

fn blob_bytes(blob: &ID3DBlob) -> Vec<u8> {
    unsafe {
        let size = blob.GetBufferSize();
        if size == 0 { return Vec::new(); }
        let pointer = blob.GetBufferPointer();
        if pointer.is_null() { return Vec::new(); }
        std::slice::from_raw_parts(pointer.cast::<u8>(), size).to_vec()
    }
}

fn registered_effects(factory: &ID2D1Factory1) -> Result<Vec<GUID>, NativeRenderError> {
    let mut capacity = 0u32;
    win(unsafe { factory.GetRegisteredEffects(None, None, Some(&mut capacity)) },
        "GetRegisteredEffects count")?;
    loop {
        let mut effects = vec![GUID::from_u128(0); capacity as usize];
        let mut returned = 0u32;
        let mut total = 0u32;
        win(unsafe { factory.GetRegisteredEffects(Some(&mut effects), Some(&mut returned),
            Some(&mut total)) }, "GetRegisteredEffects list")?;
        if total > capacity { capacity = total; continue; }
        if returned as usize > effects.len() {
            return Err(NativeRenderError::invalid_resource(
                "GetRegisteredEffects returned more GUIDs than its buffer"));
        }
        effects.truncate(returned as usize);
        return Ok(effects);
    }
}

pub(super) fn register(factory: &ID2D1Factory) -> Result<(), NativeRenderError> {
    let _guard = REGISTRATION.lock().map_err(|_| NativeRenderError::invalid_resource(
        "Source blend registration mutex poisoned"))?;
    let factory1: ID2D1Factory1 = win(factory.cast(), "Source blend factory1 interface")?;
    let mut existing = registered_effects(&factory1)?;
    let _ = shader_bytes()?;
    for kernel in 0..4 {
        if existing.contains(&EFFECT_IDS[kernel]) { continue; }
        let xml = format!("<?xml version='1.0'?><Effect><Property name='DisplayName' type='string' value='Mermaid Source {}'/><Property name='Author' type='string' value='Folia'/><Property name='Category' type='string' value='Blend'/><Property name='Description' type='string' value='Pinned Source premultiplied blend'/><Inputs><Input name='Background'/><Input name='Foreground'/></Inputs></Effect>", NAMES[kernel]);
        let xml: Vec<u16> = xml.encode_utf16().chain(std::iter::once(0)).collect();
        win(unsafe { factory1.RegisterEffectFromString(&EFFECT_IDS[kernel],
            PCWSTR(xml.as_ptr()), None, CALLBACKS[kernel]) },
            &format!("RegisterEffectFromString {}", NAMES[kernel]))?;
        existing.push(EFFECT_IDS[kernel]);
    }
    Ok(())
}

pub(super) fn blend(context: &ID2D1DeviceContext, background: &ID2D1Image,
    foreground: &ID2D1Image, mode: BlendMode, location: &str)
    -> Result<ID2D1Image, NativeRenderError> {
    let kernel = match mode {
        BlendMode::Hue => 0, BlendMode::Saturation => 1,
        BlendMode::Color => 2, BlendMode::Luminosity => 3,
        _ => return Err(NativeRenderError::unsupported(
            format!("{location}: unsupported Source nonseparable blend mode {mode:?}"))),
    };
    let effect = win(unsafe { context.CreateEffect(&EFFECT_IDS[kernel]) },
        &format!("{location}: create {} Source blend", NAMES[kernel]))?;
    unsafe { effect.SetInput(0, background, true); effect.SetInput(1, foreground, true); }
    win(unsafe { effect.GetOutput() }, &format!("{location}: {} Source blend output", NAMES[kernel]))
}

#[implement(ID2D1EffectImpl, ID2D1DrawTransform)]
struct SourceBlendEffect { kernel: usize }
impl ID2D1EffectImpl_Impl for SourceBlendEffect_Impl {
    fn Initialize(&self, effectcontext: Ref<'_, ID2D1EffectContext>,
        transformgraph: Ref<'_, ID2D1TransformGraph>) -> windows::core::Result<()> {
        let context = effectcontext.ok()?;
        let graph = transformgraph.ok()?;
        let bytes = shader_bytes().map_err(|error| windows::core::Error::new(
            error.hresult.unwrap_or(E_INVALIDARG), error.to_string()))?;
        unsafe { context.LoadPixelShader(&SHADER_IDS[self.kernel], &bytes[self.kernel])?; }
        let draw = <Self as ComObjectInterface<ID2D1DrawTransform>>::as_interface_ref(self);
        let node: ID2D1TransformNode = draw.to_owned().cast()?;
        unsafe { graph.SetSingleTransformNode(&node)?; }
        Ok(())
    }
    fn PrepareForRender(&self, _changetype: D2D1_CHANGE_TYPE) -> windows::core::Result<()> { Ok(()) }
    fn SetGraph(&self, _transformgraph: Ref<'_, ID2D1TransformGraph>) -> windows::core::Result<()> {
        Err(windows::core::Error::from_hresult(E_NOTIMPL))
    }
}
impl ID2D1TransformNode_Impl for SourceBlendEffect_Impl {
    fn GetInputCount(&self) -> u32 { 2 }
}
impl ID2D1Transform_Impl for SourceBlendEffect_Impl {
    fn MapOutputRectToInputRects(&self, outputrect: *const RECT, inputrects: *mut RECT,
        inputrectscount: u32) -> windows::core::Result<()> {
        if outputrect.is_null() || inputrects.is_null() { return Err(windows::core::Error::from_hresult(E_POINTER)); }
        if inputrectscount != 2 { return Err(windows::core::Error::from_hresult(E_INVALIDARG)); }
        unsafe { *inputrects = *outputrect; *inputrects.add(1) = *outputrect; }
        Ok(())
    }
    fn MapInputRectsToOutputRect(&self, inputrects: *const RECT, inputopaquesubrects: *const RECT,
        inputrectcount: u32, outputrect: *mut RECT, outputopaquesubrect: *mut RECT)
        -> windows::core::Result<()> {
        if inputrects.is_null() || inputopaquesubrects.is_null() || outputrect.is_null() ||
            outputopaquesubrect.is_null() { return Err(windows::core::Error::from_hresult(E_POINTER)); }
        if inputrectcount != 2 { return Err(windows::core::Error::from_hresult(E_INVALIDARG)); }
        unsafe {
            let a = *inputrects;
            let b = *inputrects.add(1);
            *outputrect = RECT { left: a.left.min(b.left), top: a.top.min(b.top),
                right: a.right.max(b.right), bottom: a.bottom.max(b.bottom) };
            *outputopaquesubrect = RECT { left: 0, top: 0, right: 0, bottom: 0 };
        }
        Ok(())
    }
    fn MapInvalidRect(&self, inputindex: u32, invalidinputrect: &RECT)
        -> windows::core::Result<RECT> {
        if inputindex >= 2 { return Err(windows::core::Error::from_hresult(E_INVALIDARG)); }
        Ok(*invalidinputrect)
    }
}
impl ID2D1DrawTransform_Impl for SourceBlendEffect_Impl {
    fn SetDrawInfo(&self, drawinfo: Ref<'_, ID2D1DrawInfo>) -> windows::core::Result<()> {
        unsafe { drawinfo.ok()?.SetPixelShader(&SHADER_IDS[self.kernel], D2D1_PIXEL_OPTIONS_NONE) }
    }
}
fn make_effect(kernel: usize, output: OutRef<'_, IUnknown>) -> HRESULT {
    if output.is_null() { return E_POINTER; }
    let effect: IUnknown = SourceBlendEffect { kernel }.into();
    match output.write(Some(effect)) { Ok(()) => HRESULT(0), Err(error) => error.code() }
}
unsafe extern "system" fn create_hue(output: OutRef<'_, IUnknown>) -> HRESULT { make_effect(0, output) }
unsafe extern "system" fn create_saturation(output: OutRef<'_, IUnknown>) -> HRESULT { make_effect(1, output) }
unsafe extern "system" fn create_color(output: OutRef<'_, IUnknown>) -> HRESULT { make_effect(2, output) }
unsafe extern "system" fn create_luminosity(output: OutRef<'_, IUnknown>) -> HRESULT { make_effect(3, output) }
