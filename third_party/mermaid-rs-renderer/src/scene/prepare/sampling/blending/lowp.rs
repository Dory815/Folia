// Copyright 2018 Google Inc.
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Source lowp pure blend stages; the caller owns pipeline continuation.
use super::super::{LowRegisters, SamplingError, wide::u16x16};
use super::super::super::pipeline_control_types::{PipelineStage, PipelinePrecision};

#[derive(Clone, Copy)]
struct Pipeline { r: u16x16, g: u16x16, b: u16x16, a: u16x16,
    dr: u16x16, dg: u16x16, db: u16x16, da: u16x16 }
impl Pipeline {
    #[inline(always)]
    fn next_stage(&mut self) {} // Original continuation is returned to the caller.
}

pub fn blend_low_stage(stage: PipelineStage, registers: &mut LowRegisters) -> Result<(), SamplingError> {
    use PipelineStage::*;
    if !matches!(stage, DestinationAtop | DestinationIn | DestinationOut | DestinationOver
        | SourceAtop | SourceIn | SourceOut | SourceOver | Clear | Modulate | Multiply
        | Plus | Screen | Xor | Darken | Difference | Exclusion | HardLight
        | Lighten | Overlay) {
        return Err(SamplingError { precision: PipelinePrecision::Low, stage });
    }
    let c = registers.channels.map(u16x16);
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
        Darken => darken(&mut p),
        Difference => difference(&mut p),
        Exclusion => exclusion(&mut p),
        HardLight => hard_light(&mut p),
        Lighten => lighten(&mut p),
        Overlay => overlay(&mut p),
        _ => unreachable!("unsupported stages rejected before register conversion"),
    }
    registers.channels = [p.r,p.g,p.b,p.a,p.dr,p.dg,p.db,p.da].map(|v| v.0);
    Ok(())
}

// Original lowp.rs:464-532, unchanged arithmetic and channel order.
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

blend_fn!(clear,            |_, _,  _,  _| u16x16::splat(0));
blend_fn!(source_atop,      |s, d, sa, da| div255(s * da + d * inv(sa)));
blend_fn!(destination_atop, |s, d, sa, da| div255(d * sa + s * inv(da)));
blend_fn!(source_in,        |s, _,  _, da| div255(s * da));
blend_fn!(destination_in,   |_, d, sa,  _| div255(d * sa));
blend_fn!(source_out,       |s, _,  _, da| div255(s * inv(da)));
blend_fn!(destination_out,  |_, d, sa,  _| div255(d * inv(sa)));
blend_fn!(source_over,      |s, d, sa,  _| s + div255(d * inv(sa)));
blend_fn!(destination_over, |s, d,  _, da| d + div255(s * inv(da)));
blend_fn!(modulate,         |s, d,  _,  _| div255(s * d));
blend_fn!(multiply,         |s, d, sa, da| div255(s * inv(da) + d * inv(sa) + s * d));
blend_fn!(screen,           |s, d,  _,  _| s + d - div255(s * d));
blend_fn!(xor,              |s, d, sa, da| div255(s * inv(da) + d * inv(sa)));

// Wants a type for some reason.
blend_fn!(plus, |s: u16x16, d, _, _| (s + d).min(&u16x16::splat(255)));


macro_rules! blend_fn2 {
    ($name:ident, $f:expr) => {
        fn $name(p: &mut Pipeline) {
            // The same logic applied to color, and source_over for alpha.
            p.r = $f(p.r, p.dr, p.a, p.da);
            p.g = $f(p.g, p.dg, p.a, p.da);
            p.b = $f(p.b, p.db, p.a, p.da);
            p.a = p.a + div255(p.da * inv(p.a));

            p.next_stage();
        }
    };
}

blend_fn2!(darken,      |s: u16x16, d, sa, da| s + d - div255((s * da).max(&(d * sa))));
blend_fn2!(lighten,     |s: u16x16, d, sa, da| s + d - div255((s * da).min(&(d * sa))));
blend_fn2!(exclusion,   |s: u16x16, d,  _,  _| s + d - u16x16::splat(2) * div255(s * d));

blend_fn2!(difference,  |s: u16x16, d, sa, da|
    s + d - u16x16::splat(2) * div255((s * da).min(&(d * sa))));

blend_fn2!(hard_light, |s: u16x16, d: u16x16, sa, da| {
    div255(s * inv(da) + d * inv(sa)
        + (s+s).cmp_le(&sa).blend(
            u16x16::splat(2) * s * d,
            sa * da - u16x16::splat(2) * (sa-s)*(da-d)
        )
    )
});

blend_fn2!(overlay, |s: u16x16, d: u16x16, sa, da| {
    div255(s * inv(da) + d * inv(sa)
        + (d+d).cmp_le(&da).blend(
            u16x16::splat(2) * s * d,
            sa * da - u16x16::splat(2) * (sa-s)*(da-d)
        )
    )
});

// Original lowp.rs:848-859.
#[inline(always)]
fn div255(v: u16x16) -> u16x16 {
    // Skia uses `vrshrq_n_u16(vrsraq_n_u16(v, v, 8), 8)` here when NEON is available,
    // but it doesn't affect performance much and breaks reproducible result. Ignore it.
    // NOTE: the compiler does not replace the division with a shift.
    (v + u16x16::splat(255)) >> u16x16::splat(8) // / u16x16::splat(256)
}

#[inline(always)]
fn inv(v: u16x16) -> u16x16 {
    u16x16::splat(255) - v
}
