// Copyright 2016 Google Inc.
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Resource-bound lowp stages of the original tiny-skia raster pipeline.
use super::{ResourceContext, ResourceCursor, ResourceProgram, ResourceStageError, StageControl};
use super::super::{LowRegisters, wide::u16x16};
use super::super::super::pipeline_control_types::{PipelinePrecision, PipelineStage};

const WIDTH: usize = 16;

pub fn resource_low_stage(
    stage: PipelineStage,
    registers: &mut LowRegisters,
    context: &mut ResourceContext<'_, '_>,
    cursor: ResourceCursor,
) -> Result<StageControl, ResourceStageError> {
    use PipelineStage::*;
    if !matches!(stage, UniformColor | LoadDestination | Store | LoadDestinationU8 | StoreU8
        | LoadMaskU8 | MaskU8 | ScaleU8 | LerpU8 | Scale1Float | Lerp1Float | SourceOverRgba) {
        return Err(ResourceStageError::Unsupported { precision: PipelinePrecision::Low, stage });
    }

    let mut channels = registers.channels.map(u16x16);
    let [r, g, b, a, dr, dg, db, da] = &mut channels;
    match stage {
        UniformColor => {
            let color = &context.uniform_color.rgba;
            *r = u16x16::splat(color[0]);
            *g = u16x16::splat(color[1]);
            *b = u16x16::splat(color[2]);
            *a = u16x16::splat(color[3]);
        }
        LoadDestination => {
            let pixels = read_rgba(context.destination.data, context.destination.real_width, cursor);
            load_8888(&pixels, dr, dg, db, da);
        }
        Store => store_8888(r, g, b, a, context.destination.data, context.destination.real_width, cursor),
        LoadDestinationU8 => {
            let data = read_u8(context.destination.data, context.destination.real_width, cursor);
            *da = u16x16(core::array::from_fn(|i| data[i] as u16));
        }
        StoreU8 => {
            let start = context.destination.real_width * cursor.dy + cursor.dx;
            let count = lane_count(cursor);
            for i in 0..count {
                context.destination.data[start + i] = a.0[i] as u8;
            }
        }
        LoadMaskU8 => {
            let offset = context.mask.real_width as usize * cursor.dy + cursor.dx;
            let mut c = u16x16::default();
            for i in 0..cursor.tail { c.0[i] = u16::from(context.mask.data[offset + i]); }
            *r = u16x16::splat(0);
            *g = u16x16::splat(0);
            *b = u16x16::splat(0);
            *a = c;
        }
        MaskU8 => {
            let offset = context.mask.real_width as usize * cursor.dy + cursor.dx;
            let mut c = u16x16::default();
            for i in 0..cursor.tail { c.0[i] = u16::from(context.mask.data[offset + i]); }
            if c == u16x16::default() { return Ok(StageControl::Stop); }
            *r = div255(*r * c);
            *g = div255(*g * c);
            *b = div255(*b * c);
            *a = div255(*a * c);
        }
        ScaleU8 => {
            let data = copy_aa_at_xy(context, cursor);
            let c = aa_lanes(data);
            *r = div255(*r * c);
            *g = div255(*g * c);
            *b = div255(*b * c);
            *a = div255(*a * c);
        }
        LerpU8 => {
            let data = copy_aa_at_xy(context, cursor);
            let c = aa_lanes(data);
            *r = lerp(*dr, *r, c);
            *g = lerp(*dg, *g, c);
            *b = lerp(*db, *b, c);
            *a = lerp(*da, *a, c);
        }
        Scale1Float => {
            let c = from_float(context.current_coverage);
            *r = div255(*r * c);
            *g = div255(*g * c);
            *b = div255(*b * c);
            *a = div255(*a * c);
        }
        Lerp1Float => {
            let c = from_float(context.current_coverage);
            *r = lerp(*dr, *r, c);
            *g = lerp(*dg, *g, c);
            *b = lerp(*db, *b, c);
            *a = lerp(*da, *a, c);
        }
        SourceOverRgba => {
            let pixels = read_rgba(context.destination.data, context.destination.real_width, cursor);
            load_8888(&pixels, dr, dg, db, da);
            *r = *r + div255(*dr * inv(*a));
            *g = *g + div255(*dg * inv(*a));
            *b = *b + div255(*db * inv(*a));
            *a = *a + div255(*da * inv(*a));
            store_8888(r, g, b, a, context.destination.data, context.destination.real_width, cursor);
        }
        _ => unreachable!("unsupported stages are rejected before state conversion"),
    }
    registers.channels = channels.map(|value| value.0);
    Ok(StageControl::Continue)
}

#[inline(always)]
fn lane_count(cursor: ResourceCursor) -> usize {
    match cursor.program { ResourceProgram::Full => WIDTH, ResourceProgram::Tail => cursor.tail }
}

#[inline(always)]
fn copy_aa_at_xy(context: &ResourceContext<'_, '_>, cursor: ResourceCursor) -> [u8; 2] {
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
fn aa_lanes(data: [u8; 2]) -> u16x16 {
    u16x16([u16::from(data[0]), u16::from(data[1]), 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0])
}

#[inline(always)]
fn read_rgba(data: &[u8], real_width: usize, cursor: ResourceCursor) -> [[u8; 4]; WIDTH] {
    let start = (real_width * cursor.dy + cursor.dx) * 4;
    let mut pixels = [[0u8; 4]; WIDTH];
    for (i, pixel) in pixels.iter_mut().enumerate().take(lane_count(cursor)) {
        pixel.copy_from_slice(&data[start + i * 4..start + (i + 1) * 4]);
    }
    pixels
}

#[inline(always)]
fn read_u8(data: &[u8], real_width: usize, cursor: ResourceCursor) -> [u8; WIDTH] {
    let start = real_width * cursor.dy + cursor.dx;
    let mut pixels = [0u8; WIDTH];
    let count = lane_count(cursor);
    pixels[..count].copy_from_slice(&data[start..start + count]);
    pixels
}

#[inline(always)]
fn load_8888(data: &[[u8; 4]; WIDTH], r: &mut u16x16, g: &mut u16x16,
    b: &mut u16x16, a: &mut u16x16) {
    *r = u16x16(core::array::from_fn(|i| data[i][0] as u16));
    *g = u16x16(core::array::from_fn(|i| data[i][1] as u16));
    *b = u16x16(core::array::from_fn(|i| data[i][2] as u16));
    *a = u16x16(core::array::from_fn(|i| data[i][3] as u16));
}

#[inline(always)]
fn store_8888(r: &u16x16, g: &u16x16, b: &u16x16, a: &u16x16,
    data: &mut [u8], real_width: usize, cursor: ResourceCursor) {
    let start = (real_width * cursor.dy + cursor.dx) * 4;
    for i in 0..lane_count(cursor) {
        data[start + i * 4..start + (i + 1) * 4]
            .copy_from_slice(&[r.0[i] as u8, g.0[i] as u8, b.0[i] as u8, a.0[i] as u8]);
    }
}

#[inline(always)]
fn div255(v: u16x16) -> u16x16 { (v + u16x16::splat(255)) >> u16x16::splat(8) }

#[inline(always)]
fn inv(v: u16x16) -> u16x16 { u16x16::splat(255) - v }

#[inline(always)]
fn from_float(f: f32) -> u16x16 { u16x16::splat((f * 255.0 + 0.5) as u16) }

#[inline(always)]
fn lerp(from: u16x16, to: u16x16, t: u16x16) -> u16x16 {
    div255(from * inv(t) + to * t)
}
