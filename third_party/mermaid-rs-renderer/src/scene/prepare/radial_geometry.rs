// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Exact portable projection of tiny-skia 0.12.0 two-circle geometry preparation.
use tiny_skia_path::{Point, Scalar, Transform};

use crate::scene::paint::SpreadMode;
use super::radial_types::{RadialGeometry, RadialKind};

const DEGENERATE_THRESHOLD: f32 = 1.0 / (1 << 15) as f32;

#[derive(Copy, Clone, PartialEq, Debug, Default)]
struct FocalData {
    r1: f32,
    focal_x: f32,
    is_swapped: bool,
}

impl FocalData {
    fn set(&mut self, mut r0: f32, mut r1: f32, matrix: &mut Transform) -> bool {
        self.is_swapped = false;
        self.focal_x = r0 / (r0 - r1);

        if (self.focal_x - 1.0).is_nearly_zero() {
            *matrix = matrix.post_translate(-1.0, 0.0).post_scale(-1.0, 1.0);
            core::mem::swap(&mut r0, &mut r1);
            self.focal_x = 0.0;
            self.is_swapped = true;
        }

        let from = [Point::from_xy(self.focal_x, 0.0), Point::from_xy(1.0, 0.0)];
        let to = [Point::from_xy(0.0, 0.0), Point::from_xy(1.0, 0.0)];
        let focal_matrix = match ts_from_poly_to_poly(from[0], from[1], to[0], to[1]) {
            Some(m) => m,
            None => return false,
        };

        *matrix = matrix.post_concat(focal_matrix);
        self.r1 = r1 / (1.0 - self.focal_x).abs();

        if self.is_focal_on_circle() {
            *matrix = matrix.post_scale(0.5, 0.5);
        } else {
            *matrix = matrix.post_scale(
                self.r1 / (self.r1 * self.r1 - 1.0),
                1.0 / (self.r1 * self.r1 - 1.0).abs().sqrt(),
            );
        }

        *matrix = matrix.post_scale((1.0 - self.focal_x).abs(), (1.0 - self.focal_x).abs());
        true
    }

    fn is_focal_on_circle(&self) -> bool {
        (1.0 - self.r1).is_nearly_zero()
    }

}

pub fn prepare_radial_geometry(
    start_point: Point,
    start_radius: f32,
    end_point: Point,
    end_radius: f32,
    stop_count: usize,
    mode: SpreadMode,
    transform: Transform,
) -> RadialGeometry {
    if start_radius < 0.0 || end_radius < 0.0 {
        return RadialGeometry::None;
    }

    match stop_count {
        0 => return RadialGeometry::None,
        1 => return RadialGeometry::SolidFirstStop,
        _ => {}
    }

    if transform.invert().is_none() {
        return RadialGeometry::None;
    }

    let length = (start_point - end_point).length();
    if !length.is_finite() {
        return RadialGeometry::None;
    }
    if length.is_nearly_zero_within_tolerance(DEGENERATE_THRESHOLD) {
        if start_radius.is_nearly_equal_within_tolerance(end_radius, DEGENERATE_THRESHOLD) {
            if mode == SpreadMode::Pad && end_radius > DEGENERATE_THRESHOLD {
                return new_radial_unchecked(start_point, end_radius, true);
            }
            return RadialGeometry::None;
        }

        if start_radius.is_nearly_zero_within_tolerance(DEGENERATE_THRESHOLD) {
            return new_radial_unchecked(start_point, end_radius, false);
        }
    }

    create(start_point, start_radius, end_point, end_radius)
}

fn new_radial_unchecked(center: Point, radius: f32, thin_ring: bool) -> RadialGeometry {
    let inv = radius.invert();
    let points_to_unit = Transform::from_translate(-center.x, -center.y).post_scale(inv, inv);
    RadialGeometry::Conical {
        kind: RadialKind::Radial { radius1: 0.0, radius2: radius },
        points_to_unit,
        thin_ring,
    }
}

fn create(c0: Point, r0: f32, c1: Point, r1: f32) -> RadialGeometry {
    let mut gradient_matrix;
    let kind;

    if (c0 - c1).length().is_nearly_zero() {
        if r0.max(r1).is_nearly_zero() || r0.is_nearly_equal(r1) {
            return RadialGeometry::None;
        }

        let scale = 1.0 / r0.max(r1);
        gradient_matrix = Transform::from_translate(-c1.x, -c1.y).post_scale(scale, scale);
        kind = RadialKind::Radial { radius1: r0, radius2: r1 };
    } else {
        gradient_matrix = match map_to_unit_x(c0, c1) {
            Some(value) => value,
            None => return RadialGeometry::None,
        };
        let d_center = (c0 - c1).length();
        kind = if (r0 - r1).is_nearly_zero() {
            let scaled_r0 = r0 / d_center;
            RadialKind::Strip { scaled_r0 }
        } else {
            let mut focal_data = FocalData::default();
            let d_center = (c0 - c1).length();
            if !focal_data.set(r0 / d_center, r1 / d_center, &mut gradient_matrix) {
                return RadialGeometry::None;
            }
            RadialKind::Focal {
                r1: focal_data.r1,
                focal_x: focal_data.focal_x,
                is_swapped: focal_data.is_swapped,
            }
        };
    }

    RadialGeometry::Conical { kind, points_to_unit: gradient_matrix, thin_ring: false }
}

fn map_to_unit_x(origin: Point, x_is_one: Point) -> Option<Transform> {
    ts_from_poly_to_poly(
        origin,
        x_is_one,
        Point::from_xy(0.0, 0.0),
        Point::from_xy(1.0, 0.0),
    )
}

fn ts_from_poly_to_poly(src1: Point, src2: Point, dst1: Point, dst2: Point) -> Option<Transform> {
    let tmp = from_poly2(src1, src2);
    let res = tmp.invert()?;
    let tmp = from_poly2(dst1, dst2);
    Some(tmp.pre_concat(res))
}

fn from_poly2(p0: Point, p1: Point) -> Transform {
    Transform::from_row(
        p1.y - p0.y,
        p0.x - p1.x,
        p1.x - p0.x,
        p1.y - p0.y,
        p0.x,
        p0.y,
    )
}
