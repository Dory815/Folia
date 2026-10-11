// Root independent semantic mutants; Source fixture remains unchanged.
'use strict';
const file='src/scene/prepare/radial_geometry.rs';
module.exports=[
 ['negative-radii-dropped','if start_radius < 0.0 || end_radius < 0.0 {','if false {'],
 ['single-before-negative','if start_radius < 0.0 || end_radius < 0.0 {','if stop_count == 1 { return RadialGeometry::SolidFirstStop; }\n    if start_radius < 0.0 || end_radius < 0.0 {'],
 ['empty-solid','0 => return RadialGeometry::None,','0 => return RadialGeometry::SolidFirstStop,'],
 ['single-transform-order','1 => return RadialGeometry::SolidFirstStop,','1 => { if transform.invert().is_none() { return RadialGeometry::None; } return RadialGeometry::SolidFirstStop; },'],
 ['singular-check-dropped','if transform.invert().is_none() {','if false {'],
 ['threshold-widened','1.0 / (1 << 15) as f32','1.0 / (1 << 12) as f32'],
 ['threshold-exclusive','if length.is_nearly_zero_within_tolerance(DEGENERATE_THRESHOLD) {','if length.abs() < DEGENERATE_THRESHOLD {'],
 ['thin-ring-spread','mode == SpreadMode::Pad && end_radius > DEGENERATE_THRESHOLD','end_radius > DEGENERATE_THRESHOLD'],
 ['thin-ring-flag','new_radial_unchecked(start_point, end_radius, true)','new_radial_unchecked(start_point, end_radius, false)'],
 ['simple-center','new_radial_unchecked(start_point, end_radius, false)','new_radial_unchecked(end_point, end_radius, false)'],
 ['simple-radius','let inv = radius.invert();','let inv = radius;'],
 ['concentric-scale','let scale = 1.0 / r0.max(r1);','let scale = 1.0 / r0.min(r1);'],
 ['strip-scale','let scaled_r0 = r0 / d_center;','let scaled_r0 = r0;'],
 ['focal-point','self.focal_x = r0 / (r0 - r1);','self.focal_x = r0 / (r0 + r1);'],
 ['swap-disabled','if (self.focal_x - 1.0).is_nearly_zero() {','if false {'],
 ['swap-flag','self.is_swapped = true;','self.is_swapped = false;'],
 ['focal-radius','self.r1 = r1 / (1.0 - self.focal_x).abs();','self.r1 = r1;'],
 ['circle-scale','matrix.post_scale(0.5, 0.5)','matrix.post_scale(1.0, 1.0)'],
 ['focal-final-scale','*matrix = matrix.post_scale((1.0 - self.focal_x).abs(), (1.0 - self.focal_x).abs());','*matrix = matrix.post_scale(1.0, 1.0);'],
 ['matrix-order','Some(tmp.pre_concat(res))','Some(tmp.post_concat(res))'],
 ['basis-sign','p0.x - p1.x,','p1.x - p0.x,'],
].map(([name,from,to])=>[name,file,from,to]);
