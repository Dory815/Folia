// Copyright 2018 Google Inc.
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Highp borrowed pattern stages; caller owns precision and continuation.
use super::{PatternContext, SamplerContext, SourcePixmap};
use super::super::{HighRegisters, SamplingError, wide::{f32x8, i32x8, u32x8}};
use super::super::super::pipeline_control_types::{PipelineStage, PipelinePrecision};
use crate::scene::paint::SpreadMode;

const STAGE_WIDTH: usize = 8;

struct Pipeline<'a, 'b> {
    pixmap_src: SourcePixmap<'a>,
    ctx: &'b PatternContext<'a>,
    r: f32x8, g: f32x8, b: f32x8, a: f32x8,
    dr: f32x8, dg: f32x8, db: f32x8, da: f32x8,
}
impl Pipeline<'_, '_> {
    #[inline(always)]
    fn next_stage(&mut self) {} // Single-stage boundary; continuation is caller-owned.
}

pub fn pattern_high_stage(stage: PipelineStage, registers: &mut HighRegisters,
    context: &PatternContext<'_>) -> Result<(), SamplingError> {
    use PipelineStage::*;
    if !matches!(stage, Gather | Reflect | Repeat | Bilinear | Bicubic) {
        return Err(SamplingError { precision: PipelinePrecision::High, stage });
    }
    let c = registers.channels.map(f32x8::from);
    let mut p = Pipeline { pixmap_src: context.source, ctx: context,
        r:c[0],g:c[1],b:c[2],a:c[3],dr:c[4],dg:c[5],db:c[6],da:c[7] };
    match stage {
        Gather => gather(&mut p),
        Reflect => reflect(&mut p),
        Repeat => repeat(&mut p),
        Bilinear => bilinear(&mut p),
        Bicubic => bicubic(&mut p),
        _ => unreachable!("unsupported stage rejected before state conversion"),
    }
    registers.channels = [p.r,p.g,p.b,p.a,p.dr,p.dg,p.db,p.da].map(Into::into);
    Ok(())
}

// The original PixmapRef::gather obtains a typed view of the entire byte slice,
// then indexes each lane in order. [u8;4] has the same RGBA representation.
#[inline(always)]
fn gather_pixels(pixmap: SourcePixmap<'_>, index: u32x8) -> [[u8; 4]; STAGE_WIDTH] {
    let index: [u32; 8] = bytemuck::cast(index);
    let pixels: &[[u8; 4]] = bytemuck::cast_slice(pixmap.data());
    [pixels[index[0] as usize], pixels[index[1] as usize],
     pixels[index[2] as usize], pixels[index[3] as usize],
     pixels[index[4] as usize], pixels[index[5] as usize],
     pixels[index[6] as usize], pixels[index[7] as usize]]
}

// Original highp.rs:309-331.
fn gather(p: &mut Pipeline) {
    let ix = gather_ix(p.pixmap_src, p.r, p.g);
    load_8888(&gather_pixels(p.pixmap_src, ix), &mut p.r, &mut p.g, &mut p.b, &mut p.a);

    p.next_stage();
}

#[inline(always)]
fn gather_ix(pixmap: SourcePixmap<'_>, mut x: f32x8, mut y: f32x8) -> u32x8 {
    // Exclusive -> inclusive.
    let w = ulp_sub(pixmap.width() as f32);
    let h = ulp_sub(pixmap.height() as f32);
    x = x.max(f32x8::default()).min(f32x8::splat(w));
    y = y.max(f32x8::default()).min(f32x8::splat(h));

    (y.trunc_int() * i32x8::splat(pixmap.width() as i32) + x.trunc_int()).to_u32x8_bitcast()
}

#[inline(always)]
fn ulp_sub(v: f32) -> f32 {
    // Somewhat similar to v - f32::EPSILON
    bytemuck::cast::<u32, f32>(bytemuck::cast::<f32, u32>(v) - 1)
}

// Original highp.rs:708-892.
fn reflect(p: &mut Pipeline) {
    let ctx = &p.ctx.limit_x;
    p.r = exclusive_reflect(p.r, ctx.scale, ctx.inv_scale);

    let ctx = &p.ctx.limit_y;
    p.g = exclusive_reflect(p.g, ctx.scale, ctx.inv_scale);

    p.next_stage();
}

#[inline(always)]
fn exclusive_reflect(v: f32x8, limit: f32, inv_limit: f32) -> f32x8 {
    let limit = f32x8::splat(limit);
    let inv_limit = f32x8::splat(inv_limit);
    ((v - limit) - (limit + limit)
        * ((v - limit) * (inv_limit * f32x8::splat(0.5))).floor() - limit).abs()
}

fn repeat(p: &mut Pipeline) {
    let ctx = &p.ctx.limit_x;
    p.r = exclusive_repeat(p.r, ctx.scale, ctx.inv_scale);

    let ctx = &p.ctx.limit_y;
    p.g = exclusive_repeat(p.g, ctx.scale, ctx.inv_scale);

    p.next_stage();
}

#[inline(always)]
fn exclusive_repeat(v: f32x8, limit: f32, inv_limit: f32) -> f32x8 {
    v - (v * f32x8::splat(inv_limit)).floor() * f32x8::splat(limit)
}

fn bilinear(p: &mut Pipeline) {
    let x = p.r;
    let fx = (x + f32x8::splat(0.5)).fract();
    let y = p.g;
    let fy = (y + f32x8::splat(0.5)).fract();
    let one = f32x8::splat(1.0);
    let wx = [one - fx, fx];
    let wy = [one - fy, fy];

    sampler_2x2(p.pixmap_src, &p.ctx.sampler, x, y, &wx, &wy, &mut p.r, &mut p.g, &mut p.b, &mut p.a);

    p.next_stage();
}

fn bicubic(p: &mut Pipeline) {
    let x = p.r;
    let fx = (x + f32x8::splat(0.5)).fract();
    let y = p.g;
    let fy = (y + f32x8::splat(0.5)).fract();
    let one = f32x8::splat(1.0);
    let wx = [bicubic_far(one - fx), bicubic_near(one - fx), bicubic_near(fx), bicubic_far(fx)];
    let wy = [bicubic_far(one - fy), bicubic_near(one - fy), bicubic_near(fy), bicubic_far(fy)];

    sampler_4x4(p.pixmap_src, &p.ctx.sampler, x, y, &wx, &wy, &mut p.r, &mut p.g, &mut p.b, &mut p.a);

    p.next_stage();
}

// In bicubic interpolation, the 16 pixels and +/- 0.5 and +/- 1.5 offsets from the sample
// pixel center are combined with a non-uniform cubic filter, with higher values near the center.
//
// We break this function into two parts, one for near 0.5 offsets and one for far 1.5 offsets.

#[inline(always)]
fn bicubic_near(t: f32x8) -> f32x8 {
    // 1/18 + 9/18t + 27/18t^2 - 21/18t^3 == t ( t ( -21/18t + 27/18) + 9/18) + 1/18
    mad(
        t,
        mad(t,
            mad(
                f32x8::splat(-21.0/18.0),
                t,
                f32x8::splat(27.0/18.0),
            ),
            f32x8::splat(9.0/18.0),
        ),
        f32x8::splat(1.0/18.0),
    )
}

#[inline(always)]
fn bicubic_far(t: f32x8) -> f32x8 {
    // 0/18 + 0/18*t - 6/18t^2 + 7/18t^3 == t^2 (7/18t - 6/18)
    (t * t) * mad(f32x8::splat(7.0/18.0), t, f32x8::splat(-6.0/18.0))
}

#[inline(always)]
fn sampler_2x2(
    pixmap: SourcePixmap<'_>,
    ctx: &SamplerContext,
    cx: f32x8, cy: f32x8,
    wx: &[f32x8; 2], wy: &[f32x8; 2],
    r: &mut f32x8, g: &mut f32x8, b: &mut f32x8, a: &mut f32x8,
) {
    *r = f32x8::default();
    *g = f32x8::default();
    *b = f32x8::default();
    *a = f32x8::default();

    let one = f32x8::splat(1.0);
    let start = -0.5;
    let mut y = cy + f32x8::splat(start);
    for j in 0..2 {
        let mut x = cx + f32x8::splat(start);
        for i in 0..2 {
            let mut rr = f32x8::default();
            let mut gg = f32x8::default();
            let mut bb = f32x8::default();
            let mut aa = f32x8::default();
            sample(pixmap, ctx, x,y, &mut rr, &mut gg, &mut bb, &mut aa);

            let w = wx[i] * wy[j];
            *r = mad(w, rr, *r);
            *g = mad(w, gg, *g);
            *b = mad(w, bb, *b);
            *a = mad(w, aa, *a);

            x += one;
        }

        y += one;
    }
}

#[inline(always)]
fn sampler_4x4(
    pixmap: SourcePixmap<'_>,
    ctx: &SamplerContext,
    cx: f32x8, cy: f32x8,
    wx: &[f32x8; 4], wy: &[f32x8; 4],
    r: &mut f32x8, g: &mut f32x8, b: &mut f32x8, a: &mut f32x8,
) {
    *r = f32x8::default();
    *g = f32x8::default();
    *b = f32x8::default();
    *a = f32x8::default();

    let one = f32x8::splat(1.0);
    let start = -1.5;
    let mut y = cy + f32x8::splat(start);
    for j in 0..4 {
        let mut x = cx + f32x8::splat(start);
        for i in 0..4 {
            let mut rr = f32x8::default();
            let mut gg = f32x8::default();
            let mut bb = f32x8::default();
            let mut aa = f32x8::default();
            sample(pixmap, ctx, x,y, &mut rr, &mut gg, &mut bb, &mut aa);

            let w = wx[i] * wy[j];
            *r = mad(w, rr, *r);
            *g = mad(w, gg, *g);
            *b = mad(w, bb, *b);
            *a = mad(w, aa, *a);

            x += one;
        }

        y += one;
    }
}

#[inline(always)]
fn sample(
    pixmap: SourcePixmap<'_>, ctx: &SamplerContext, mut x: f32x8, mut y: f32x8,
    r: &mut f32x8, g: &mut f32x8, b: &mut f32x8, a: &mut f32x8,
) {
    x = tile(x, ctx.spread_mode, pixmap.width() as f32, ctx.inv_width);
    y = tile(y, ctx.spread_mode, pixmap.height() as f32, ctx.inv_height);

    let ix = gather_ix(pixmap, x, y);
    load_8888(&gather_pixels(pixmap, ix), r, g, b, a);
}

#[inline(always)]
fn tile(v: f32x8, mode: SpreadMode, limit: f32, inv_limit: f32) -> f32x8 {
    match mode {
        SpreadMode::Pad => v,
        SpreadMode::Repeat => exclusive_repeat(v, limit, inv_limit),
        SpreadMode::Reflect => exclusive_reflect(v, limit, inv_limit),
    }
}

// Original highp.rs:1245-1280, byte-view adaptation only.
fn load_8888(
    data: &[[u8; 4]; STAGE_WIDTH],
    r: &mut f32x8, g: &mut f32x8, b: &mut f32x8, a: &mut f32x8,
) {
    // Surprisingly, `f32 * FACTOR` is way faster than `f32x8 * f32x8::splat(FACTOR)`.

    const FACTOR: f32 = 1.0 / 255.0;

    *r = f32x8::from([
        data[0][0] as f32 * FACTOR, data[1][0] as f32 * FACTOR,
        data[2][0] as f32 * FACTOR, data[3][0] as f32 * FACTOR,
        data[4][0] as f32 * FACTOR, data[5][0] as f32 * FACTOR,
        data[6][0] as f32 * FACTOR, data[7][0] as f32 * FACTOR,
    ]);

    *g = f32x8::from([
        data[0][1] as f32 * FACTOR, data[1][1] as f32 * FACTOR,
        data[2][1] as f32 * FACTOR, data[3][1] as f32 * FACTOR,
        data[4][1] as f32 * FACTOR, data[5][1] as f32 * FACTOR,
        data[6][1] as f32 * FACTOR, data[7][1] as f32 * FACTOR,
    ]);

    *b = f32x8::from([
        data[0][2] as f32 * FACTOR, data[1][2] as f32 * FACTOR,
        data[2][2] as f32 * FACTOR, data[3][2] as f32 * FACTOR,
        data[4][2] as f32 * FACTOR, data[5][2] as f32 * FACTOR,
        data[6][2] as f32 * FACTOR, data[7][2] as f32 * FACTOR,
    ]);

    *a = f32x8::from([
        data[0][3] as f32 * FACTOR, data[1][3] as f32 * FACTOR,
        data[2][3] as f32 * FACTOR, data[3][3] as f32 * FACTOR,
        data[4][3] as f32 * FACTOR, data[5][3] as f32 * FACTOR,
        data[6][3] as f32 * FACTOR, data[7][3] as f32 * FACTOR,
    ]);
}

// Original highp.rs:1356-1359.
#[inline(always)]
fn mad(f: f32x8, m: f32x8, a: f32x8) -> f32x8 {
    f * m + a
}
