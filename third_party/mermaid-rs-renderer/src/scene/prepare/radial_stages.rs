// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Shared projection of tiny-skia 0.12.0 radial stage selection.

use tiny_skia_path::Scalar;

use super::radial_stage_types::{RadialStage, RadialStagePlan};
use super::radial_types::RadialKind;

#[derive(Clone, Copy)]
struct FocalData {
    r1: f32,
    focal_x: f32,
    is_swapped: bool,
}

impl FocalData {
    fn is_focal_on_circle(&self) -> bool {
        (1.0 - self.r1).is_nearly_zero()
    }

    fn is_well_behaved(&self) -> bool {
        !self.is_focal_on_circle() && self.r1 > 1.0
    }

    fn is_natively_focal(&self) -> bool {
        self.focal_x.is_nearly_zero()
    }
}

pub fn prepare_radial_stages(kind: RadialKind) -> RadialStagePlan {
    let (p0, p1) = match kind {
        RadialKind::Radial { radius1, radius2 } => {
            if radius1 == 0.0 {
                (1.0, 0.0)
            } else {
                let d_radius = radius2 - radius1;
                let p0 = radius1.max(radius2) / d_radius;
                let p1 = -radius1 / d_radius;
                (p0, p1)
            }
        }
        RadialKind::Strip { scaled_r0 } => (scaled_r0 * scaled_r0, 0.0),
        RadialKind::Focal { r1, focal_x, .. } => (1.0 / r1, focal_x),
    };

    let mut before_color = Vec::new();
    let mut after_color = Vec::new();

    match kind {
        RadialKind::Radial { .. } => {
            before_color.push(RadialStage::XYToRadius);
            if (p0, p1) != (1.0, 0.0) {
                before_color.push(RadialStage::ApplyConcentricScaleBias);
            }
        }
        RadialKind::Strip { .. } => {
            before_color.push(RadialStage::XYTo2PtConicalStrip);
            before_color.push(RadialStage::Mask2PtConicalNan);
            after_color.push(RadialStage::ApplyVectorMask);
        }
        RadialKind::Focal { r1, focal_x, is_swapped } => {
            let fd = FocalData { r1, focal_x, is_swapped };
            if fd.is_focal_on_circle() {
                before_color.push(RadialStage::XYTo2PtConicalFocalOnCircle);
            } else if fd.is_well_behaved() {
                before_color.push(RadialStage::XYTo2PtConicalWellBehaved);
            } else if fd.is_swapped || (1.0 - fd.focal_x) < 0.0 {
                before_color.push(RadialStage::XYTo2PtConicalSmaller);
            } else {
                before_color.push(RadialStage::XYTo2PtConicalGreater);
            }

            if !fd.is_well_behaved() {
                before_color.push(RadialStage::Mask2PtConicalDegenerates);
            }
            if (1.0 - fd.focal_x) < 0.0 {
                before_color.push(RadialStage::NegateX);
            }
            if !fd.is_natively_focal() {
                before_color.push(RadialStage::Alter2PtConicalCompensateFocal);
            }
            if fd.is_swapped {
                before_color.push(RadialStage::Alter2PtConicalUnswap);
            }
            if !fd.is_well_behaved() {
                after_color.push(RadialStage::ApplyVectorMask);
            }
        }
    }

    // lowp.rs implements XYToRadius; its other radial stage entries are null_fn.
    let geometry_lowp_compatible = before_color.len() == 1
        && before_color[0] == RadialStage::XYToRadius
        && after_color.is_empty();

    RadialStagePlan { p0, p1, before_color, after_color, geometry_lowp_compatible }
}
