// Copyright 2018 Google Inc.
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Source highp pure blend stages; the caller owns pipeline continuation.
use super::super::{HighRegisters, SamplingError, wide::f32x8};
use super::super::super::pipeline_control_types::{PipelineStage, PipelinePrecision};

#[derive(Clone, Copy)]
struct Pipeline { r: f32x8, g: f32x8, b: f32x8, a: f32x8,
    dr: f32x8, dg: f32x8, db: f32x8, da: f32x8 }
impl Pipeline {
    #[inline(always)]
    fn next_stage(&mut self) {} // Original continuation is returned to the caller.
}

pub fn blend_high_stage(stage: PipelineStage, registers: &mut HighRegisters) -> Result<(), SamplingError> {
    use PipelineStage::*;
    if !matches!(stage, DestinationAtop | DestinationIn | DestinationOut | DestinationOver
        | SourceAtop | SourceIn | SourceOut | SourceOver | Clear | Modulate | Multiply
        | Plus | Screen | Xor | ColorBurn | ColorDodge | Darken | Difference
        | Exclusion | HardLight | Lighten | Overlay | SoftLight | Hue | Saturation
        | Color | Luminosity) {
        return Err(SamplingError { precision: PipelinePrecision::High, stage });
    }
    let c = registers.channels.map(f32x8::from);
    let mut p = Pipeline { r:c[0],g:c[1],b:c[2],a:c[3],dr:c[4],dg:c[5],db:c[6],da:c[7] };
    match stage {
        DestinationAtop => destination_atop(&mut p),
        DestinationIn => destination_in(&mut p),
        DestinationOut => destination_out(&mut p),
        DestinationOver => destination_over(&mut p),
        SourceAtop => source_atop(&mut p),
        SourceIn => source_in(&mut p),
        SourceOut => source_out(&mut p),
        SourceOver => source_over(&mut p),
        Clear => clear(&mut p),
        Modulate => modulate(&mut p),
        Multiply => multiply(&mut p),
        Plus => plus(&mut p),
        Screen => screen(&mut p),
        Xor => xor(&mut p),
        ColorBurn => color_burn(&mut p),
        ColorDodge => color_dodge(&mut p),
        Darken => darken(&mut p),
        Difference => difference(&mut p),
        Exclusion => exclusion(&mut p),
        HardLight => hard_light(&mut p),
        Lighten => lighten(&mut p),
        Overlay => overlay(&mut p),
        SoftLight => soft_light(&mut p),
        Hue => hue(&mut p),
        Saturation => saturation(&mut p),
        Color => color(&mut p),
        Luminosity => luminosity(&mut p),
        _ => unreachable!("unsupported stages rejected before register conversion"),
    }
    registers.channels = [p.r,p.g,p.b,p.a,p.dr,p.dg,p.db,p.da].map(Into::into);
    Ok(())
}

// Original highp.rs:405-667, unchanged arithmetic and channel order.
macro_rules! blend_fn {
    ($name:ident, $f:expr) => {
        fn $name(p: &mut Pipeline) {
            p.r = $f(p.r, p.dr, p.a, p.da);
            p.g = $f(p.g, p.dg, p.a, p.da);
            p.b = $f(p.b, p.db, p.a, p.da);
            p.a = $f(p.a, p.da, p.a, p.da);

            p.next_stage();
        }
    };
}

blend_fn!(clear,            |_, _,  _,  _| f32x8::default());
blend_fn!(source_atop,      |s, d, sa, da| s * da + d * inv(sa));
blend_fn!(destination_atop, |s, d, sa, da| d * sa + s * inv(da));
blend_fn!(source_in,        |s, _,  _, da| s * da);
blend_fn!(destination_in,   |_, d, sa,  _| d * sa);
blend_fn!(source_out,       |s, _,  _, da| s * inv(da));
blend_fn!(destination_out,  |_, d, sa,  _| d * inv(sa));
blend_fn!(source_over,      |s, d, sa,  _| mad(d, inv(sa), s));
blend_fn!(destination_over, |s, d,  _, da| mad(s, inv(da), d));
blend_fn!(modulate,         |s, d,  _,  _| s * d);
blend_fn!(multiply,         |s, d, sa, da| s * inv(da) + d * inv(sa) + s * d);
blend_fn!(screen,           |s, d,  _,  _| s + d - s * d);
blend_fn!(xor,              |s, d, sa, da| s * inv(da) + d * inv(sa));

// Wants a type for some reason.
blend_fn!(plus, |s: f32x8, d: f32x8, _, _| (s + d).min(f32x8::splat(1.0)));

macro_rules! blend_fn2 {
    ($name:ident, $f:expr) => {
        fn $name(p: &mut Pipeline) {
            // The same logic applied to color, and source_over for alpha.
            p.r = $f(p.r, p.dr, p.a, p.da);
            p.g = $f(p.g, p.dg, p.a, p.da);
            p.b = $f(p.b, p.db, p.a, p.da);
            p.a = mad(p.da, inv(p.a), p.a);

            p.next_stage();
        }
    };
}

blend_fn2!(darken,      |s: f32x8, d, sa, da: f32x8| s + d - (s * da).max(d * sa));
blend_fn2!(lighten,     |s: f32x8, d, sa, da: f32x8| s + d - (s * da).min(d * sa));
blend_fn2!(difference,  |s: f32x8, d, sa, da: f32x8| s + d - two((s * da).min(d * sa)));
blend_fn2!(exclusion,   |s: f32x8, d,  _,  _| s + d - two(s * d));

blend_fn2!(color_burn, |s: f32x8, d: f32x8, sa: f32x8, da: f32x8|
    d.cmp_eq(da).blend(
        d + s * inv(da),
        s.cmp_eq(f32x8::default()).blend(
            d * inv(sa),
            sa * (da - da.min((da - d) * sa * s.recip_fast())) + s * inv(da) + d * inv(sa)
        )
    )
);

blend_fn2!(color_dodge, |s: f32x8, d: f32x8, sa: f32x8, da: f32x8|
    d.cmp_eq(f32x8::default()).blend(
        s * inv(da),
        s.cmp_eq(sa).blend(
            s + d * inv(sa),
            sa * da.min((d * sa) * (sa - s).recip_fast()) + s * inv(da) + d * inv(sa)
        )
    )
);

blend_fn2!(hard_light, |s: f32x8, d: f32x8, sa, da|
    s * inv(da) + d * inv(sa) + two(s).cmp_le(sa).blend(
        two(s * d),
        sa * da - two((da - d) * (sa - s))
    )
);

blend_fn2!(overlay, |s: f32x8, d: f32x8, sa, da|
    s * inv(da) + d * inv(sa) + two(d).cmp_le(da).blend(
        two(s * d),
        sa * da - two((da - d) * (sa - s))
    )
);

blend_fn2!(soft_light, |s: f32x8, d: f32x8, sa: f32x8, da: f32x8| {
    let m  = da.cmp_gt(f32x8::default()).blend(d / da, f32x8::default());
    let s2 = two(s);
    let m4 = two(two(m));

    // The logic forks three ways:
    //    1. dark src?
    //    2. light src, dark dst?
    //    3. light src, light dst?
    let dark_src = d * (sa + (s2 - sa) * (f32x8::splat(1.0) - m));
    let dark_dst = (m4 * m4 + m4) * (m - f32x8::splat(1.0)) + f32x8::splat(7.0) * m;
    let lite_dst = m.sqrt() - m;
    let lite_src = d * sa + da * (s2 - sa)
        * two(two(d)).cmp_le(da).blend(dark_dst, lite_dst); // 2 or 3?

    s * inv(da) + d * inv(sa) + s2.cmp_le(sa).blend(dark_src, lite_src) // 1 or (2 or 3)?
});

// We're basing our implementation of non-separable blend modes on
//   https://www.w3.org/TR/compositing-1/#blendingnonseparable.
// and
//   https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
// They're equivalent, but ES' math has been better simplified.
//
// Anything extra we add beyond that is to make the math work with premul inputs.

macro_rules! blend_fn3 {
    ($name:ident, $f:expr) => {
        fn $name(p: &mut Pipeline) {
            let (tr, tg, tb, ta) = $f(p.r, p.g, p.b, p.a, p.dr, p.dg, p.db, p.da);
            p.r = tr;
            p.g = tg;
            p.b = tb;
            p.a = ta;

            p.next_stage();
        }
    };
}

blend_fn3!(hue, hue_k);

#[inline(always)]
fn hue_k(
    r: f32x8, g: f32x8, b: f32x8, a: f32x8,
    dr: f32x8, dg: f32x8, db: f32x8, da: f32x8,
) -> (f32x8, f32x8, f32x8, f32x8) {
    let rr = &mut (r * a);
    let gg = &mut (g * a);
    let bb = &mut (b * a);

    set_sat(rr, gg, bb, sat(dr, dg, db) * a);
    set_lum(rr, gg, bb, lum(dr, dg, db) * a);
    clip_color(rr, gg, bb, a * da);

    let r = r * inv(da) + dr * inv(a) + *rr;
    let g = g * inv(da) + dg * inv(a) + *gg;
    let b = b * inv(da) + db * inv(a) + *bb;
    let a = a + da - a * da;

    (r, g, b, a)
}

blend_fn3!(saturation, saturation_k);

#[inline(always)]
fn saturation_k(
    r: f32x8, g: f32x8, b: f32x8, a: f32x8,
    dr: f32x8, dg: f32x8, db: f32x8, da: f32x8,
) -> (f32x8, f32x8, f32x8, f32x8) {
    let rr = &mut (dr * a);
    let gg = &mut (dg * a);
    let bb = &mut (db * a);

    set_sat(rr, gg, bb, sat(r, g, b) * da);
    set_lum(rr, gg, bb, lum(dr, dg, db) * a); // (This is not redundant.)
    clip_color(rr, gg, bb, a * da);

    let r = r * inv(da) + dr * inv(a) + *rr;
    let g = g * inv(da) + dg * inv(a) + *gg;
    let b = b * inv(da) + db * inv(a) + *bb;
    let a = a + da - a * da;

    (r, g, b, a)
}

blend_fn3!(color, color_k);

#[inline(always)]
fn color_k(
    r: f32x8, g: f32x8, b: f32x8, a: f32x8,
    dr: f32x8, dg: f32x8, db: f32x8, da: f32x8,
) -> (f32x8, f32x8, f32x8, f32x8) {
    let rr = &mut (r * da);
    let gg = &mut (g * da);
    let bb = &mut (b * da);

    set_lum(rr, gg, bb, lum(dr, dg, db) * a);
    clip_color(rr, gg, bb, a * da);

    let r = r * inv(da) + dr * inv(a) + *rr;
    let g = g * inv(da) + dg * inv(a) + *gg;
    let b = b * inv(da) + db * inv(a) + *bb;
    let a = a + da - a * da;

    (r, g, b, a)
}

blend_fn3!(luminosity, luminosity_k);

#[inline(always)]
fn luminosity_k(
    r: f32x8, g: f32x8, b: f32x8, a: f32x8,
    dr: f32x8, dg: f32x8, db: f32x8, da: f32x8,
) -> (f32x8, f32x8, f32x8, f32x8) {
    let rr = &mut (dr * a);
    let gg = &mut (dg * a);
    let bb = &mut (db * a);

    set_lum(rr, gg, bb, lum(r, g, b) * da);
    clip_color(rr, gg, bb, a * da);

    let r = r * inv(da) + dr * inv(a) + *rr;
    let g = g * inv(da) + dg * inv(a) + *gg;
    let b = b * inv(da) + db * inv(a) + *bb;
    let a = a + da - a * da;

    (r, g, b, a)
}

#[inline(always)]
fn sat(r: f32x8, g: f32x8, b: f32x8) -> f32x8 {
    r.max(g.max(b)) - r.min(g.min(b))
}

#[inline(always)]
fn lum(r: f32x8, g: f32x8, b: f32x8) -> f32x8 {
    r * f32x8::splat(0.30) + g * f32x8::splat(0.59) + b * f32x8::splat(0.11)
}

#[inline(always)]
fn set_sat(r: &mut f32x8, g: &mut f32x8, b: &mut f32x8, s: f32x8) {
    let mn  = r.min(g.min(*b));
    let mx  = r.max(g.max(*b));
    let sat = mx - mn;

    // Map min channel to 0, max channel to s, and scale the middle proportionally.
    let scale = |c| sat.cmp_eq(f32x8::default())
                       .blend(f32x8::default(), (c - mn) * s / sat);

    *r = scale(*r);
    *g = scale(*g);
    *b = scale(*b);
}

#[inline(always)]
fn set_lum(r: &mut f32x8, g: &mut f32x8, b: &mut f32x8, l: f32x8) {
    let diff = l - lum(*r, *g, *b);
    *r += diff;
    *g += diff;
    *b += diff;
}

#[inline(always)]
fn clip_color(r: &mut f32x8, g: &mut f32x8, b: &mut f32x8, a: f32x8) {
    let mn = r.min(g.min(*b));
    let mx = r.max(g.max(*b));
    let l  = lum(*r, *g, *b);

    let clip = |mut c| {
        c = mx.cmp_ge(f32x8::default()).blend(c, l + (c - l) * l / (l - mn));
        c = mx.cmp_gt(a).blend(l + (c - l) * (a - l) / (mx - l), c);
        c = c.max(f32x8::default()); // Sometimes without this we may dip just a little negative.
        c
    };

    *r = clip(*r);
    *g = clip(*g);
    *b = clip(*b);
}

// Original highp.rs:1347-1359.
#[inline(always)]
fn inv(v: f32x8) -> f32x8 {
    f32x8::splat(1.0) - v
}

#[inline(always)]
fn two(v: f32x8) -> f32x8 {
    v + v
}

#[inline(always)]
fn mad(f: f32x8, m: f32x8, a: f32x8) -> f32x8 {
    f * m + a
}
