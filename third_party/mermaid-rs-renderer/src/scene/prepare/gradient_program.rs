// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Complete generic gradient stage and color-context preparation.

use tiny_skia_path::NormalizedF32;

use crate::scene::paint::SpreadMode;

use super::gradient_program_types::{
    ColorSpace, GradientColor, GradientContext, TwoStopContext,
};
use super::gradient_stop_types::{PreparedGradient, StopColor};
use super::pipeline_builder::PipelineBuilder;
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

/// Original generic Gradient producer: supplied pre/post plan owns no context.
pub fn push_gradient_stages(
    gradient: &PreparedGradient,
    color_space: ColorSpace,
    radial: Option<&RadialStagePlan>,
    builder: &mut PipelineBuilder,
) -> bool {
    use super::pipeline_control_types::PipelineStage;
    use super::radial_stage_types::RadialStage;

    let push_radial_stages = |stages: &[RadialStage], builder: &mut PipelineBuilder| {
        for stage in stages {
            let stage = match stage {
                RadialStage::XYToRadius => PipelineStage::XYToRadius,
                RadialStage::ApplyConcentricScaleBias => PipelineStage::ApplyConcentricScaleBias,
                RadialStage::XYTo2PtConicalStrip => PipelineStage::XYTo2PtConicalStrip,
                RadialStage::Mask2PtConicalNan => PipelineStage::Mask2PtConicalNan,
                RadialStage::XYTo2PtConicalFocalOnCircle => PipelineStage::XYTo2PtConicalFocalOnCircle,
                RadialStage::XYTo2PtConicalWellBehaved => PipelineStage::XYTo2PtConicalWellBehaved,
                RadialStage::XYTo2PtConicalSmaller => PipelineStage::XYTo2PtConicalSmaller,
                RadialStage::XYTo2PtConicalGreater => PipelineStage::XYTo2PtConicalGreater,
                RadialStage::Mask2PtConicalDegenerates => PipelineStage::Mask2PtConicalDegenerates,
                RadialStage::NegateX => PipelineStage::NegateX,
                RadialStage::Alter2PtConicalCompensateFocal => PipelineStage::Alter2PtConicalCompensateFocal,
                RadialStage::Alter2PtConicalUnswap => PipelineStage::Alter2PtConicalUnswap,
                RadialStage::ApplyVectorMask => PipelineStage::ApplyVectorMask,
            };
            builder.push(stage);
        }
    };

    builder.push(PipelineStage::SeedShader);

    let ts = match gradient.transform.invert() {
        Some(value) => value,
        None => {
            log::warn!("failed to invert a gradient transform. Nothing will be rendered");
            return false;
        }
    };
    let ts = ts.post_concat(gradient.points_to_unit);
    builder.push_transform(ts);

    if let Some(radial) = radial {
        push_radial_stages(&radial.before_color, builder);
    }

    match gradient.spread {
        SpreadMode::Reflect => builder.push(PipelineStage::ReflectX1),
        SpreadMode::Repeat => builder.push(PipelineStage::RepeatX1),
        SpreadMode::Pad => {
            if gradient.has_uniform_stops {
                builder.push(PipelineStage::PadX1);
            }
        }
    }

    if gradient.stops.len() == 2 {
        debug_assert!(gradient.has_uniform_stops);

        let c0 = color_space.expand_color(gradient.stops[0].color);
        let c1 = color_space.expand_color(gradient.stops[1].color);
        let c0 = GradientColor::from_color(c0);
        let c1 = GradientColor::from_color(c1);

        builder.context.two_stop = TwoStopContext {
            factor: GradientColor::new(
                c1.components[0] - c0.components[0],
                c1.components[1] - c0.components[1],
                c1.components[2] - c0.components[2],
                c1.components[3] - c0.components[3],
            ),
            bias: c0,
        };
        builder.push(PipelineStage::EvenlySpaced2StopGradient);
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

        builder.push(PipelineStage::Gradient);
        builder.context.gradient = ctx;
    }

    if !gradient.colors_are_opaque {
        builder.push(PipelineStage::Premultiply);
    }

    if let Some(radial) = radial {
        push_radial_stages(&radial.after_color, builder);
    }

    true
}
