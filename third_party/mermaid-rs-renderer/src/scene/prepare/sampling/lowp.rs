// Copyright 2018 Google Inc.
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Source lowp gradient stages, retaining coordinate bit packing.
use super::{LowRegisters, SamplingError};
use super::super::{
    gradient_program_types::{GradientContext, GradientProgram},
    pipeline_control_types::{PipelinePrecision, PipelineStage},
};
use super::wide::{f32x8, f32x16, u16x16};

pub fn sample_low_stage(
    stage: PipelineStage,
    registers: &mut LowRegisters,
    program: &mut GradientProgram,
    origin: [usize; 2],
) -> Result<(), SamplingError> {
    use PipelineStage::*;
    if !matches!(stage,
        MoveSourceToDestination | MoveDestinationToSource | Premultiply | SeedShader
        | Transform | PadX1 | ReflectX1 | RepeatX1 | Gradient
        | EvenlySpaced2StopGradient | XYToRadius
    ) {
        return Err(SamplingError { precision: PipelinePrecision::Low, stage });
    }

    let mut channels = registers.channels.map(u16x16);
    let [r, g, b, a, dr, dg, db, da] = &mut channels;
    match stage {
        MoveSourceToDestination => {
            *dr = *r; *dg = *g; *db = *b; *da = *a;
        }
        MoveDestinationToSource => {
            *r = *dr; *g = *dg; *b = *db; *a = *da;
        }
        Premultiply => {
            *r = div255(*r * *a);
            *g = div255(*g * *a);
            *b = div255(*b * *a);
        }
        SeedShader => {
            let iota = f32x16(
                f32x8::from([0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5]),
                f32x8::from([8.5, 9.5, 10.5, 11.5, 12.5, 13.5, 14.5, 15.5]),
            );
            let x = f32x16::splat(origin[0] as f32) + iota;
            let y = f32x16::splat(origin[1] as f32 + 0.5);
            split(&x, r, g);
            split(&y, b, a);
        }
        Transform => {
            let ts = &program.transform;
            let x = join(r, g);
            let y = join(b, a);
            let nx = mad(x, f32x16::splat(ts.sx), mad(y, f32x16::splat(ts.kx), f32x16::splat(ts.tx)));
            let ny = mad(x, f32x16::splat(ts.ky), mad(y, f32x16::splat(ts.sy), f32x16::splat(ts.ty)));
            split(&nx, r, g);
            split(&ny, b, a);
        }
        PadX1 => {
            let x = join(r, g);
            let x = x.normalize();
            split(&x, r, g);
        }
        ReflectX1 => {
            let x = join(r, g);
            let two = |x| x + x;
            let x = ((x - f32x16::splat(1.0))
                - two(((x - f32x16::splat(1.0)) * f32x16::splat(0.5)).floor())
                - f32x16::splat(1.0)).abs().normalize();
            split(&x, r, g);
        }
        RepeatX1 => {
            let x = join(r, g);
            let x = (x - x.floor()).normalize();
            split(&x, r, g);
        }
        Gradient => {
            let ctx = &program.gradient;
            let t = join(r, g);
            let mut idx = u16x16::splat(0);
            for i in 1..ctx.len {
                let tt = ctx.t_values[i].get();
                let t0: [f32; 8] = t.0.into();
                let t1: [f32; 8] = t.1.into();
                idx.0[0] += (t0[0] >= tt) as u16;
                idx.0[1] += (t0[1] >= tt) as u16;
                idx.0[2] += (t0[2] >= tt) as u16;
                idx.0[3] += (t0[3] >= tt) as u16;
                idx.0[4] += (t0[4] >= tt) as u16;
                idx.0[5] += (t0[5] >= tt) as u16;
                idx.0[6] += (t0[6] >= tt) as u16;
                idx.0[7] += (t0[7] >= tt) as u16;
                idx.0[8] += (t1[0] >= tt) as u16;
                idx.0[9] += (t1[1] >= tt) as u16;
                idx.0[10] += (t1[2] >= tt) as u16;
                idx.0[11] += (t1[3] >= tt) as u16;
                idx.0[12] += (t1[4] >= tt) as u16;
                idx.0[13] += (t1[5] >= tt) as u16;
                idx.0[14] += (t1[6] >= tt) as u16;
                idx.0[15] += (t1[7] >= tt) as u16;
            }
            gradient_lookup(ctx, &idx, t, r, g, b, a);
        }
        EvenlySpaced2StopGradient => {
            let ctx = &program.two_stop;
            let t = join(r, g);
            round_f32_to_u16(
                mad(t, f32x16::splat(ctx.factor.components[0]), f32x16::splat(ctx.bias.components[0])),
                mad(t, f32x16::splat(ctx.factor.components[1]), f32x16::splat(ctx.bias.components[1])),
                mad(t, f32x16::splat(ctx.factor.components[2]), f32x16::splat(ctx.bias.components[2])),
                mad(t, f32x16::splat(ctx.factor.components[3]), f32x16::splat(ctx.bias.components[3])),
                r, g, b, a,
            );
        }
        XYToRadius => {
            let x = join(r, g);
            let y = join(b, a);
            let x = (x*x + y*y).sqrt();
            split(&x, r, g);
            split(&y, b, a);
        }
        _ => unreachable!("unsupported stages were rejected before state conversion"),
    }
    registers.channels = channels.map(|value| value.0);
    Ok(())
}

fn gradient_lookup(ctx: &GradientContext, idx: &u16x16, t: f32x16,
    r: &mut u16x16, g: &mut u16x16, b: &mut u16x16, a: &mut u16x16) {
    macro_rules! gather {
        ($values:expr, $channel:expr) => {
            f32x16(
                f32x8::from([
                    $values[idx.0[0] as usize].components[$channel], $values[idx.0[1] as usize].components[$channel],
                    $values[idx.0[2] as usize].components[$channel], $values[idx.0[3] as usize].components[$channel],
                    $values[idx.0[4] as usize].components[$channel], $values[idx.0[5] as usize].components[$channel],
                    $values[idx.0[6] as usize].components[$channel], $values[idx.0[7] as usize].components[$channel],
                ]),
                f32x8::from([
                    $values[idx.0[8] as usize].components[$channel], $values[idx.0[9] as usize].components[$channel],
                    $values[idx.0[10] as usize].components[$channel], $values[idx.0[11] as usize].components[$channel],
                    $values[idx.0[12] as usize].components[$channel], $values[idx.0[13] as usize].components[$channel],
                    $values[idx.0[14] as usize].components[$channel], $values[idx.0[15] as usize].components[$channel],
                ]),
            )
        };
    }
    let fr = gather!(&ctx.factors, 0);
    let fg = gather!(&ctx.factors, 1);
    let fb = gather!(&ctx.factors, 2);
    let fa = gather!(&ctx.factors, 3);
    let br = gather!(&ctx.biases, 0);
    let bg = gather!(&ctx.biases, 1);
    let bb = gather!(&ctx.biases, 2);
    let ba = gather!(&ctx.biases, 3);
    round_f32_to_u16(
        mad(t, fr, br), mad(t, fg, bg), mad(t, fb, bb), mad(t, fa, ba),
        r, g, b, a,
    );
}

#[inline(always)]
fn round_f32_to_u16(rf: f32x16, gf: f32x16, bf: f32x16, af: f32x16,
    r: &mut u16x16, g: &mut u16x16, b: &mut u16x16, a: &mut u16x16) {
    let rf = rf.normalize() * f32x16::splat(255.0) + f32x16::splat(0.5);
    let gf = gf.normalize() * f32x16::splat(255.0) + f32x16::splat(0.5);
    let bf = bf.normalize() * f32x16::splat(255.0) + f32x16::splat(0.5);
    let af = af * f32x16::splat(255.0) + f32x16::splat(0.5);
    rf.save_to_u16x16(r);
    gf.save_to_u16x16(g);
    bf.save_to_u16x16(b);
    af.save_to_u16x16(a);
}

#[inline(always)]
fn div255(v: u16x16) -> u16x16 {
    (v + u16x16::splat(255)) >> u16x16::splat(8)
}

#[inline(always)]
fn split(v: &f32x16, lo: &mut u16x16, hi: &mut u16x16) {
    let data: [u8; 64] = bytemuck::cast(*v);
    let d0: &mut [u8; 32] = bytemuck::cast_mut(&mut lo.0);
    let d1: &mut [u8; 32] = bytemuck::cast_mut(&mut hi.0);
    d0.copy_from_slice(&data[0..32]);
    d1.copy_from_slice(&data[32..64]);
}

#[inline(always)]
fn join(lo: &u16x16, hi: &u16x16) -> f32x16 {
    let d0: [u8; 32] = bytemuck::cast(lo.0);
    let d1: [u8; 32] = bytemuck::cast(hi.0);
    let mut v = f32x16::default();
    let data: &mut [u8; 64] = bytemuck::cast_mut(&mut v);
    data[0..32].copy_from_slice(&d0);
    data[32..64].copy_from_slice(&d1);
    v
}

#[inline(always)]
fn mad(f: f32x16, m: f32x16, a: f32x16) -> f32x16 { f * m + a }
