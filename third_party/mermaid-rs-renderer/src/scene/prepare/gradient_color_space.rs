// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Source color-space expansion and approximate power for shared gradients.

use tiny_skia_path::NormalizedF32;

use super::{gradient_program_types::ColorSpace, gradient_stop_types::StopColor};

pub fn approximate_power(x: f32, y: f32) -> f32 {
    if x == 0.0 || x == 1.0 {
        return x;
    }

    let e = x.to_bits() as f32 * (1.0f32 / ((1 << 23) as f32));
    let m = f32::from_bits((x.to_bits() & 0x007fffff) | 0x3f000000);

    let log2_x = e - 124.225514990f32 - 1.498030302f32 * m
        - 1.725879990f32 / (0.3520887068f32 + m);

    let x = log2_x * y;
    let f = x - x.floor();

    let mut a = x + 121.274057500f32;
    a -= f * 1.490129070f32;
    a += 27.728023300f32 / (4.84252568f32 - f);
    a *= (1 << 23) as f32;

    if a < f32::INFINITY.to_bits() as f32 {
        if a > 0.0 {
            f32::from_bits(a.round() as u32)
        } else {
            0.0
        }
    } else {
        f32::INFINITY
    }
}

impl ColorSpace {
    fn expand_channel(self, x: NormalizedF32) -> NormalizedF32 {
        match self {
            ColorSpace::Linear => x,
            ColorSpace::Gamma2 => x * x,
            ColorSpace::SimpleSRGB => {
                NormalizedF32::new_clamped(approximate_power(x.get(), 2.2))
            }
            ColorSpace::FullSRGBGamma => {
                let x = x.get();
                let x = if x <= 0.04045 {
                    x / 12.92
                } else {
                    approximate_power((x + 0.055) / 1.055, 2.4)
                };
                NormalizedF32::new_clamped(x)
            }
        }
    }

    pub fn expand_color(self, mut color: StopColor) -> StopColor {
        color.components[0] = self.expand_channel(color.components[0]);
        color.components[1] = self.expand_channel(color.components[1]);
        color.components[2] = self.expand_channel(color.components[2]);
        color
    }
}

impl ColorSpace{
 pub fn expand_dest_stage(self)->Option<super::pipeline_control_types::PipelineStage>{panic!("Unimplemented Root-frozen ColorSpace::expand_dest_stage")}
 pub fn compress_stage(self)->Option<super::pipeline_control_types::PipelineStage>{panic!("Unimplemented Root-frozen ColorSpace::compress_stage")}
}
