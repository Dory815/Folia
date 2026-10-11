// Root independent semantic branch, precision, ordering and compatibility mutants.
'use strict';
const file='src/scene/prepare/radial_stages.rs';
module.exports=[
 ['radius-zero-order',file,'if radius1 == 0.0 {','if radius1 <= 0.0 {'],
 ['radius-max-lost',file,'radius1.max(radius2) / d_radius','radius2 / d_radius'],
 ['radius-difference-reversed',file,'let d_radius = radius2 - radius1;','let d_radius = radius1 - radius2;'],
 ['radius-bias-sign',file,'let p1 = -radius1 / d_radius;','let p1 = radius1 / d_radius;'],
 ['strip-square-lost',file,'(scaled_r0 * scaled_r0, 0.0)','(scaled_r0, 0.0)'],
 ['focal-reciprocal-lost',file,'=> (1.0 / r1, focal_x),','=> (r1, focal_x),'],
 ['on-circle-inclusive-lost',file,'(1.0 - self.r1).is_nearly_zero()','(1.0 - self.r1).abs() < 1.0 / 4096.0'],
 ['well-behaved-direction',file,'!self.is_focal_on_circle() && self.r1 > 1.0','!self.is_focal_on_circle() && self.r1 < 1.0'],
 ['native-focal-threshold-lost',file,'self.focal_x.is_nearly_zero()','self.focal_x == 0.0'],
 ['focal-on-circle-priority',file,'if fd.is_focal_on_circle() {','if fd.is_focal_on_circle() && !fd.is_swapped {'],
 ['smaller-conjunction',file,'fd.is_swapped || (1.0 - fd.focal_x) < 0.0','fd.is_swapped && (1.0 - fd.focal_x) < 0.0'],
 ['strip-nan-mask-lost',file,'before_color.push(RadialStage::Mask2PtConicalNan);',''],
 ['focal-degenerate-mask-lost',file,'before_color.push(RadialStage::Mask2PtConicalDegenerates);',''],
 ['negate-strict-sign',file,'if (1.0 - fd.focal_x) < 0.0 {','if (1.0 - fd.focal_x) <= 0.0 {'],
 ['compensate-predicate',file,'if !fd.is_natively_focal() {','if fd.is_natively_focal() {'],
 ['unswap-predicate',file,'if fd.is_swapped {','if !fd.is_swapped {'],
 ['strip-mask-before-color',file,'before_color.push(RadialStage::Mask2PtConicalNan);\n            after_color.push(RadialStage::ApplyVectorMask);','before_color.push(RadialStage::Mask2PtConicalNan);\n            before_color.push(RadialStage::ApplyVectorMask);'],
 ['focal-mask-before-color',file,'if !fd.is_well_behaved() {\n                after_color.push(RadialStage::ApplyVectorMask);','if !fd.is_well_behaved() {\n                before_color.push(RadialStage::ApplyVectorMask);'],
 ['lowp-always-compatible',file,'before_color.len() == 1\n        && before_color[0] == RadialStage::XYToRadius\n        && after_color.is_empty()','true'],
 ['lowp-never-compatible',file,'before_color.len() == 1\n        && before_color[0] == RadialStage::XYToRadius\n        && after_color.is_empty()','false'],
];
