// Copyright 2018 Google Inc.
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Source highp gradient stages, with the continuation cursor owned by the caller.
use super::{HighRegisters, SamplingError};
use super::super::{
    pipeline_context_types::{GradientContext, PipelineContext},
    pipeline_control_types::{PipelinePrecision, PipelineStage},
};
use super::wide::{f32x8, u32x8};

pub fn sample_high_stage(
    stage: PipelineStage,
    registers: &mut HighRegisters,
    context: &mut PipelineContext,
    origin: [usize; 2],
) -> Result<(), SamplingError> {
    use PipelineStage::*;
    if !matches!(stage,
        MoveSourceToDestination | MoveDestinationToSource | Clamp0 | ClampA | Premultiply
        | SeedShader | Transform | PadX1 | ReflectX1 | RepeatX1 | Gradient
        | EvenlySpaced2StopGradient | XYToUnitAngle | XYToRadius
        | XYTo2PtConicalFocalOnCircle | XYTo2PtConicalWellBehaved
        | XYTo2PtConicalSmaller | XYTo2PtConicalGreater | XYTo2PtConicalStrip
        | Mask2PtConicalNan | Mask2PtConicalDegenerates | ApplyVectorMask
        | Alter2PtConicalCompensateFocal | Alter2PtConicalUnswap | NegateX
        | ApplyConcentricScaleBias | GammaExpand2 | GammaExpandDestination2
        | GammaCompress2 | GammaExpand22 | GammaExpandDestination22
        | GammaCompress22 | GammaExpandSrgb | GammaExpandDestinationSrgb
        | GammaCompressSrgb
    ) {
        return Err(SamplingError { precision: PipelinePrecision::High, stage });
    }

    let mut channels = registers.channels.map(f32x8::from);
    let [r, g, b, a, dr, dg, db, da] = &mut channels;
    match stage {
        MoveSourceToDestination => {
            *dr = *r; *dg = *g; *db = *b; *da = *a;
        }
        MoveDestinationToSource => {
            *r = *dr; *g = *dg; *b = *db; *a = *da;
        }
        Clamp0 => {
            *r = r.max(f32x8::default());
            *g = g.max(f32x8::default());
            *b = b.max(f32x8::default());
            *a = a.max(f32x8::default());
        }
        ClampA => {
            *r = r.min(f32x8::splat(1.0));
            *g = g.min(f32x8::splat(1.0));
            *b = b.min(f32x8::splat(1.0));
            *a = a.min(f32x8::splat(1.0));
        }
        Premultiply => {
            *r *= *a; *g *= *a; *b *= *a;
        }
        SeedShader => {
            let iota = f32x8::from([0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5]);
            *r = f32x8::splat(origin[0] as f32) + iota;
            *g = f32x8::splat(origin[1] as f32 + 0.5);
            *b = f32x8::splat(1.0);
            *a = f32x8::default();
            *dr = f32x8::default();
            *dg = f32x8::default();
            *db = f32x8::default();
            *da = f32x8::default();
        }
        Transform => {
            let ts = &context.transform;
            let tr = mad(*r, f32x8::splat(ts.sx), mad(*g, f32x8::splat(ts.kx), f32x8::splat(ts.tx)));
            let tg = mad(*r, f32x8::splat(ts.ky), mad(*g, f32x8::splat(ts.sy), f32x8::splat(ts.ty)));
            *r = tr; *g = tg;
        }
        PadX1 => *r = r.normalize(),
        ReflectX1 => {
            *r = ((*r - f32x8::splat(1.0))
                - two(((*r - f32x8::splat(1.0)) * f32x8::splat(0.5)).floor())
                - f32x8::splat(1.0)).abs().normalize();
        }
        RepeatX1 => *r = (*r - r.floor()).normalize(),
        Gradient => {
            let ctx = &context.gradient;
            let t: [f32; 8] = (*r).into();
            let mut idx = u32x8::default();
            for i in 1..ctx.len {
                let tt = ctx.t_values[i].get();
                let n: u32x8 = bytemuck::cast([
                    (t[0] >= tt) as u32, (t[1] >= tt) as u32,
                    (t[2] >= tt) as u32, (t[3] >= tt) as u32,
                    (t[4] >= tt) as u32, (t[5] >= tt) as u32,
                    (t[6] >= tt) as u32, (t[7] >= tt) as u32,
                ]);
                idx = idx + n;
            }
            let t = *r;
            gradient_lookup(ctx, &idx, t, r, g, b, a);
        }
        EvenlySpaced2StopGradient => {
            let ctx = &context.two_stop;
            let t = *r;
            *r = mad(t, f32x8::splat(ctx.factor.components[0]), f32x8::splat(ctx.bias.components[0]));
            *g = mad(t, f32x8::splat(ctx.factor.components[1]), f32x8::splat(ctx.bias.components[1]));
            *b = mad(t, f32x8::splat(ctx.factor.components[2]), f32x8::splat(ctx.bias.components[2]));
            *a = mad(t, f32x8::splat(ctx.factor.components[3]), f32x8::splat(ctx.bias.components[3]));
        }
        XYToUnitAngle => {
            let x = *r;
            let y = *g;
            let x_abs = x.abs();
            let y_abs = y.abs();
            let slope = x_abs.min(y_abs) / x_abs.max(y_abs);
            let s = slope * slope;
            let phi = slope
                * (f32x8::splat(0.15912117063999176025390625)
                    + s * (f32x8::splat(-5.185396969318389892578125e-2)
                        + s * (f32x8::splat(2.476101927459239959716796875e-2)
                            + s * f32x8::splat(-7.0547382347285747528076171875e-3))));
            let phi = x_abs.cmp_lt(y_abs).blend(f32x8::splat(0.25) - phi, phi);
            let phi = x.cmp_lt(f32x8::splat(0.0)).blend(f32x8::splat(0.5) - phi, phi);
            let phi = y.cmp_lt(f32x8::splat(0.0)).blend(f32x8::splat(1.0) - phi, phi);
            let phi = phi.cmp_ne(phi).blend(f32x8::splat(0.0), phi);
            *r = phi;
        }
        XYToRadius => {
            let x2 = *r * *r;
            let y2 = *g * *g;
            *r = (x2 + y2).sqrt();
        }
        XYTo2PtConicalFocalOnCircle => {
            let x = *r; let y = *g;
            *r = x + y * y / x;
        }
        XYTo2PtConicalWellBehaved => {
            let x = *r; let y = *g;
            *r = (x * x + y * y).sqrt() - x * f32x8::splat(context.radial_p[0]);
        }
        XYTo2PtConicalGreater => {
            let x = *r; let y = *g;
            *r = (x * x - y * y).sqrt() - x * f32x8::splat(context.radial_p[0]);
        }
        XYTo2PtConicalSmaller => {
            let x = *r; let y = *g;
            *r = -(x * x - y * y).sqrt() - x * f32x8::splat(context.radial_p[0]);
        }
        XYTo2PtConicalStrip => {
            let x = *r; let y = *g;
            *r = x + (f32x8::splat(context.radial_p[0]) - y * y).sqrt();
        }
        Mask2PtConicalNan => {
            let t = *r;
            let is_degenerate = t.cmp_ne(t);
            *r = is_degenerate.blend(f32x8::default(), t);
            context.radial_mask = bytemuck::cast(cond_to_mask(!is_degenerate.to_u32x8_bitcast()));
        }
        Mask2PtConicalDegenerates => {
            let t = *r;
            let is_degenerate = t.cmp_le(f32x8::default()) | t.cmp_ne(t);
            *r = is_degenerate.blend(f32x8::default(), t);
            context.radial_mask = bytemuck::cast(cond_to_mask(!is_degenerate.to_u32x8_bitcast()));
        }
        ApplyVectorMask => {
            let mask: u32x8 = bytemuck::cast(context.radial_mask);
            *r = (r.to_u32x8_bitcast() & mask).to_f32x8_bitcast();
            *g = (g.to_u32x8_bitcast() & mask).to_f32x8_bitcast();
            *b = (b.to_u32x8_bitcast() & mask).to_f32x8_bitcast();
            *a = (a.to_u32x8_bitcast() & mask).to_f32x8_bitcast();
        }
        Alter2PtConicalCompensateFocal => *r = *r + f32x8::splat(context.radial_p[1]),
        Alter2PtConicalUnswap => *r = f32x8::splat(1.0) - *r,
        NegateX => *r = -*r,
        ApplyConcentricScaleBias => {
            let x = *r;
            *r = x * f32x8::splat(context.radial_p[0]) + f32x8::splat(context.radial_p[1]);
        }
        GammaExpand2 => { *r = *r * *r; *g = *g * *g; *b = *b * *b; }
        GammaExpandDestination2 => { *dr = *dr * *dr; *dg = *dg * *dg; *db = *db * *db; }
        GammaCompress2 => { *r = r.sqrt(); *g = g.sqrt(); *b = b.sqrt(); }
        GammaExpand22 => { *r = r.powf(2.2); *g = g.powf(2.2); *b = b.powf(2.2); }
        GammaExpandDestination22 => { *dr = dr.powf(2.2); *dg = dg.powf(2.2); *db = db.powf(2.2); }
        GammaCompress22 => { *r = r.powf(0.45454545); *g = g.powf(0.45454545); *b = b.powf(0.45454545); }
        GammaExpandSrgb => { *r = srgb_expand(*r); *g = srgb_expand(*g); *b = srgb_expand(*b); }
        GammaExpandDestinationSrgb => { *dr = srgb_expand(*dr); *dg = srgb_expand(*dg); *db = srgb_expand(*db); }
        GammaCompressSrgb => { *r = srgb_compress(*r); *g = srgb_compress(*g); *b = srgb_compress(*b); }
        _ => unreachable!("unsupported stages were rejected before state conversion"),
    }
    registers.channels = channels.map(Into::into);
    Ok(())
}

fn gradient_lookup(ctx: &GradientContext, idx: &u32x8, t: f32x8,
    r: &mut f32x8, g: &mut f32x8, b: &mut f32x8, a: &mut f32x8) {
    let idx: [u32; 8] = bytemuck::cast(*idx);
    macro_rules! gather {
        ($values:expr, $channel:expr) => {
            f32x8::from([
                $values[idx[0] as usize].components[$channel], $values[idx[1] as usize].components[$channel],
                $values[idx[2] as usize].components[$channel], $values[idx[3] as usize].components[$channel],
                $values[idx[4] as usize].components[$channel], $values[idx[5] as usize].components[$channel],
                $values[idx[6] as usize].components[$channel], $values[idx[7] as usize].components[$channel],
            ])
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
    *r = mad(t, fr, br);
    *g = mad(t, fg, bg);
    *b = mad(t, fb, bb);
    *a = mad(t, fa, ba);
}

#[inline(always)]
fn cond_to_mask(cond: u32x8) -> u32x8 {
    let cond: [u32; 8] = bytemuck::cast(cond);
    bytemuck::cast([
        if cond[0] != 0 { !0 } else { 0 }, if cond[1] != 0 { !0 } else { 0 },
        if cond[2] != 0 { !0 } else { 0 }, if cond[3] != 0 { !0 } else { 0 },
        if cond[4] != 0 { !0 } else { 0 }, if cond[5] != 0 { !0 } else { 0 },
        if cond[6] != 0 { !0 } else { 0 }, if cond[7] != 0 { !0 } else { 0 },
    ])
}

#[inline(always)]
fn two(v: f32x8) -> f32x8 { v + v }

#[inline(always)]
fn mad(f: f32x8, m: f32x8, a: f32x8) -> f32x8 { f * m + a }

fn srgb_expand(x: f32x8) -> f32x8 {
    let small = x.cmp_le(f32x8::splat(0.04045));
    let linear = x / f32x8::splat(12.92);
    let exp = ((x + f32x8::splat(0.055)) / f32x8::splat(1.055)).powf(2.4);
    small.blend(linear, exp)
}

fn srgb_compress(x: f32x8) -> f32x8 {
    let small = x.cmp_le(f32x8::splat(0.0031308));
    let linear = x * f32x8::splat(12.92);
    let exp = x.powf(0.416666666) * f32x8::splat(1.055) - f32x8::splat(0.055);
    small.blend(linear, exp)
}
