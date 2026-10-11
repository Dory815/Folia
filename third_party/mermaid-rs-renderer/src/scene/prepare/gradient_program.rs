// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Complete generic gradient stage and color-context preparation.

use tiny_skia_path::NormalizedF32;

use crate::scene::paint::SpreadMode;

use super::gradient_program_types::{
    ColorSpace, GradientColor, GradientContext, GradientProgram, GradientStage, TwoStopContext,
};
use super::gradient_stop_types::{PreparedGradient, StopColor};
use super::pipeline_context_types::PipelineContext;
use super::radial_stage_types::RadialStagePlan;

impl GradientColor {
    fn new(r: f32, g: f32, b: f32, a: f32) -> Self {
        Self { components: [r, g, b, a] }
    }

    fn from_color(color: StopColor) -> Self {
        let [r, g, b, a] = color.components();
        Self::new(r, g, b, a)
    }
}

impl GradientContext {
    fn push_const_color(&mut self, color: GradientColor) {
        self.factors.push(GradientColor::new(0.0, 0.0, 0.0, 0.0));
        self.biases.push(color);
    }
}

pub fn prepare_gradient_program(
    gradient: &PreparedGradient,
    color_space: ColorSpace,
    radial: Option<&RadialStagePlan>,
) -> GradientProgram {
    let mut program = GradientProgram {
        returned: false,
        stages: Vec::new(),
        context: PipelineContext::default(),
    };

    if let Some(radial) = radial {
        program.context.radial_p = [radial.p0, radial.p1];
    }

    program.stages.push(GradientStage::SeedShader);

    let ts = match gradient.transform.invert() {
        Some(value) => value,
        None => {
            log::warn!("failed to invert a gradient transform. Nothing will be rendered");
            return program;
        }
    };
    let ts = ts.post_concat(gradient.points_to_unit);
    if ts.is_finite() && !ts.is_identity() {
        program.stages.push(GradientStage::Transform);
        program.context.transform = ts;
    }

    if let Some(radial) = radial {
        program.stages.extend(radial.before_color.iter().copied().map(GradientStage::Radial));
    }

    match gradient.spread {
        SpreadMode::Reflect => program.stages.push(GradientStage::ReflectX1),
        SpreadMode::Repeat => program.stages.push(GradientStage::RepeatX1),
        SpreadMode::Pad => {
            if gradient.has_uniform_stops {
                program.stages.push(GradientStage::PadX1);
            }
        }
    }

    if gradient.stops.len() == 2 {
        debug_assert!(gradient.has_uniform_stops);

        let c0 = color_space.expand_color(gradient.stops[0].color);
        let c1 = color_space.expand_color(gradient.stops[1].color);
        let c0 = GradientColor::from_color(c0);
        let c1 = GradientColor::from_color(c1);

        program.context.two_stop = TwoStopContext {
            factor: GradientColor::new(
                c1.components[0] - c0.components[0],
                c1.components[1] - c0.components[1],
                c1.components[2] - c0.components[2],
                c1.components[3] - c0.components[3],
            ),
            bias: c0,
        };
        program.stages.push(GradientStage::EvenlySpaced2StopGradient);
    } else {
        let mut ctx = GradientContext::default();
        ctx.factors.reserve((gradient.stops.len() + 1).max(16));
        ctx.biases.reserve((gradient.stops.len() + 1).max(16));
        ctx.t_values.reserve(gradient.stops.len() + 1);

        let (first_stop, last_stop) = if gradient.stops.len() > 2 {
            let first = if gradient.stops[0].color != gradient.stops[1].color {
                0
            } else {
                1
            };

            let len = gradient.stops.len();
            let last = if gradient.stops[len - 2].color != gradient.stops[len - 1].color {
                len - 1
            } else {
                len - 2
            };
            (first, last)
        } else {
            (0, 1)
        };

        let mut t_l = gradient.stops[first_stop].position.get();
        let mut c_l = GradientColor::from_color(
            color_space.expand_color(gradient.stops[first_stop].color),
        );
        ctx.push_const_color(c_l);
        ctx.t_values.push(NormalizedF32::ZERO);

        for i in first_stop..last_stop {
            let t_r = gradient.stops[i + 1].position.get();
            let c_r = GradientColor::from_color(
                color_space.expand_color(gradient.stops[i + 1].color),
            );
            debug_assert!(t_l <= t_r);
            if t_l < t_r {
                let f = GradientColor::new(
                    (c_r.components[0] - c_l.components[0]) / (t_r - t_l),
                    (c_r.components[1] - c_l.components[1]) / (t_r - t_l),
                    (c_r.components[2] - c_l.components[2]) / (t_r - t_l),
                    (c_r.components[3] - c_l.components[3]) / (t_r - t_l),
                );
                ctx.factors.push(f);

                ctx.biases.push(GradientColor::new(
                    c_l.components[0] - f.components[0] * t_l,
                    c_l.components[1] - f.components[1] * t_l,
                    c_l.components[2] - f.components[2] * t_l,
                    c_l.components[3] - f.components[3] * t_l,
                ));

                ctx.t_values.push(NormalizedF32::new_clamped(t_l));
            }

            t_l = t_r;
            c_l = c_r;
        }

        ctx.push_const_color(c_l);
        ctx.t_values.push(NormalizedF32::new_clamped(t_l));
        ctx.len = ctx.factors.len();

        debug_assert_eq!(ctx.factors.len(), ctx.t_values.len());
        debug_assert_eq!(ctx.biases.len(), ctx.t_values.len());

        while ctx.factors.len() < 16 {
            ctx.factors.push(GradientColor::default());
            ctx.biases.push(GradientColor::default());
        }

        program.stages.push(GradientStage::Gradient);
        program.context.gradient = ctx;
    }

    if !gradient.colors_are_opaque {
        program.stages.push(GradientStage::Premultiply);
    }

    if let Some(radial) = radial {
        program.stages.extend(radial.after_color.iter().copied().map(GradientStage::Radial));
    }

    program.returned = true;
    program
}
