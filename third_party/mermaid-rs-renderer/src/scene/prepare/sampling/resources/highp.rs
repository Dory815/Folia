// Copyright 2016 Google Inc.
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Resource-bound highp stages of the original tiny-skia raster pipeline.
use super::{ResourceContext, ResourceCursor, ResourceProgram, ResourceStageError, StageControl};
use super::super::{HighRegisters, wide::{f32x8, i32x8}};
use super::super::super::pipeline_control_types::{PipelinePrecision, PipelineStage};

const WIDTH: usize = 8;

pub fn resource_high_stage(
    stage: PipelineStage,
    registers: &mut HighRegisters,
    context: &mut ResourceContext<'_, '_, '_>,
    cursor: ResourceCursor,
) -> Result<StageControl, ResourceStageError> {
    use PipelineStage::*;
    if !matches!(stage, UniformColor | LoadDestination | Store | MaskU8 | ScaleU8
        | LerpU8 | Scale1Float | Lerp1Float | SourceOverRgba) {
        return Err(ResourceStageError::Unsupported { precision: PipelinePrecision::High, stage });
    }

    let mut channels = registers.channels.map(f32x8::from);
    let [r, g, b, a, dr, dg, db, da] = &mut channels;
    match stage {
        UniformColor => {
            let color = &context.pipeline.uniform_color.components;
            *r = f32x8::splat(color[0]);
            *g = f32x8::splat(color[1]);
            *b = f32x8::splat(color[2]);
            *a = f32x8::splat(color[3]);
        }
        LoadDestination => {
            let pixels = read_rgba(context.destination.data, context.destination.real_width, cursor);
            load_8888(&pixels, dr, dg, db, da);
        }
        Store => {
            store_8888(r, g, b, a, context.destination.data, context.destination.real_width, cursor);
        }
        MaskU8 => {
            let offset = context.mask.real_width as usize * cursor.dy + cursor.dx;
            let mut values = [0.0; WIDTH];
            for (i, value) in values.iter_mut().enumerate().take(cursor.tail) {
                *value = context.mask.data[offset + i] as f32;
            }
            let c = f32x8::from(values) / f32x8::splat(255.0);
            if c == f32x8::default() {
                return Ok(StageControl::Stop);
            }
            *r *= c; *g *= c; *b *= c; *a *= c;
        }
        ScaleU8 => {
            let bytes = copy_aa_at_xy(context, cursor);
            let c = f32x8::from([bytes[0] as f32, bytes[1] as f32, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
                / f32x8::splat(255.0);
            *r *= c; *g *= c; *b *= c; *a *= c;
        }
        LerpU8 => {
            let bytes = copy_aa_at_xy(context, cursor);
            let c = f32x8::from([bytes[0] as f32, bytes[1] as f32, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
                / f32x8::splat(255.0);
            *r = lerp(*dr, *r, c);
            *g = lerp(*dg, *g, c);
            *b = lerp(*db, *b, c);
            *a = lerp(*da, *a, c);
        }
        Scale1Float => {
            let c = f32x8::splat(context.pipeline.current_coverage);
            *r *= c; *g *= c; *b *= c; *a *= c;
        }
        Lerp1Float => {
            let c = f32x8::splat(context.pipeline.current_coverage);
            *r = lerp(*dr, *r, c);
            *g = lerp(*dg, *g, c);
            *b = lerp(*db, *b, c);
            *a = lerp(*da, *a, c);
        }
        SourceOverRgba => {
            let pixels = read_rgba(context.destination.data, context.destination.real_width, cursor);
            load_8888(&pixels, dr, dg, db, da);
            *r = mad(*dr, inv(*a), *r);
            *g = mad(*dg, inv(*a), *g);
            *b = mad(*db, inv(*a), *b);
            *a = mad(*da, inv(*a), *a);
            store_8888(r, g, b, a, context.destination.data, context.destination.real_width, cursor);
        }
        _ => unreachable!("unsupported stages are rejected before state conversion"),
    }
    registers.channels = channels.map(Into::into);
    Ok(StageControl::Continue)
}

#[inline(always)]
fn copy_aa_at_xy(context: &ResourceContext<'_, '_, '_>, cursor: ResourceCursor) -> [u8; 2] {
    let aa = context.aa_mask;
    let offset = (aa.stride as usize * cursor.dy + cursor.dx) - aa.shift;
    match (offset, cursor.tail) {
        (0, 1) => [aa.pixels[0], 0],
        (0, 2) => aa.pixels,
        (1, 1) => [aa.pixels[1], 0],
        _ => [0, 0],
    }
}

#[inline(always)]
fn read_rgba(data: &[u8], real_width: usize, cursor: ResourceCursor) -> [[u8; 4]; WIDTH] {
    let start = (real_width * cursor.dy + cursor.dx) * 4;
    let count = match cursor.program { ResourceProgram::Full => WIDTH, ResourceProgram::Tail => cursor.tail };
    let mut pixels = [[0u8; 4]; WIDTH];
    for (i, pixel) in pixels.iter_mut().enumerate().take(count) {
        pixel.copy_from_slice(&data[start + i * 4..start + (i + 1) * 4]);
    }
    pixels
}

#[inline(always)]
fn load_8888(data: &[[u8; 4]; WIDTH], r: &mut f32x8, g: &mut f32x8, b: &mut f32x8, a: &mut f32x8) {
    const FACTOR: f32 = 1.0 / 255.0;
    *r = f32x8::from(core::array::from_fn(|i| data[i][0] as f32 * FACTOR));
    *g = f32x8::from(core::array::from_fn(|i| data[i][1] as f32 * FACTOR));
    *b = f32x8::from(core::array::from_fn(|i| data[i][2] as f32 * FACTOR));
    *a = f32x8::from(core::array::from_fn(|i| data[i][3] as f32 * FACTOR));
}

#[inline(always)]
fn store_8888(r: &f32x8, g: &f32x8, b: &f32x8, a: &f32x8,
    data: &mut [u8], real_width: usize, cursor: ResourceCursor) {
    let r: [i32; WIDTH] = unnorm(r).into();
    let g: [i32; WIDTH] = unnorm(g).into();
    let b: [i32; WIDTH] = unnorm(b).into();
    let a: [i32; WIDTH] = unnorm(a).into();
    let start = (real_width * cursor.dy + cursor.dx) * 4;
    let count = match cursor.program { ResourceProgram::Full => WIDTH, ResourceProgram::Tail => cursor.tail };
    for i in 0..count {
        data[start + i * 4..start + (i + 1) * 4]
            .copy_from_slice(&[r[i] as u8, g[i] as u8, b[i] as u8, a[i] as u8]);
    }
}

#[inline(always)]
fn unnorm(v: &f32x8) -> i32x8 {
    (v.max(f32x8::default()).min(f32x8::splat(1.0)) * f32x8::splat(255.0)).round_int()
}

#[inline(always)]
fn inv(v: f32x8) -> f32x8 { f32x8::splat(1.0) - v }

#[inline(always)]
fn mad(f: f32x8, m: f32x8, a: f32x8) -> f32x8 { f * m + a }

#[inline(always)]
fn lerp(from: f32x8, to: f32x8, t: f32x8) -> f32x8 { mad(to - from, t, from) }
