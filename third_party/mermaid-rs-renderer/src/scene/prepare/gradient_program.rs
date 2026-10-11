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
    _gradient: &PreparedGradient,
    _color_space: ColorSpace,
    _radial: Option<&RadialStagePlan>,
    _builder: &mut PipelineBuilder,
) -> bool {
    panic!("Unimplemented Root-frozen common Gradient producer");
}
