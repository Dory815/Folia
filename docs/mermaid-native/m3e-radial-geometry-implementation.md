# M3e shared radial geometry implementation — awaiting root acceptance

Implemented only `src/scene/prepare/radial_geometry.rs` against the root-frozen
`radial_types.rs` projection and
`docs/mermaid-native/m3e-radial-geometry-contract.md`. The Source is
tiny-skia 0.12.0 `src/shaders/radial_gradient.rs`, SHA256
`3ae883526cead4d5361f8b6974a91dece8f62fe3740a1712be8ce6219efdd74c`.
The portable Point/Transform/Scalar operations use the locked tiny-skia-path
0.12.0 directly; their own f32/f64 matrix math is retained. This geometry
projection contains no stop colors, stop normalization, shader stages,
SVG/backend rendering, or Folia integration.

| Pinned Source method and lines | Target method and corresponding operation |
| --- | --- |
| `FocalData::set:29–68` | `FocalData::set:17–49`: same f32 `r0/(r0-r1)`, near-one swap, post-translate/post-scale, two-point mapping, `r1/abs(1-f)`, on-circle branch scales, final `abs(1-f)` scale, in Source order. A failed two-point inverse returns false. |
| `is_focal_on_circle:70–72` | `is_focal_on_circle:51–53`: same `(1-r1).is_nearly_zero()`. |
| `is_well_behaved:74–76`, `is_natively_focal:78–80` | Root review assigns these to the required later stage-selection packet and removed their unused private copies here. They are not accepted by this geometry projection. |
| `RadialGradient::new:120–200` | `prepare_radial_geometry:64–105`: negative radius rejection before stop count; zero/one stop returns None/SolidFirstStop before transform and distance; same transform inverse and finite distance checks; same inclusive `1/32768` center/radius threshold, Pad thin-ring branch, start-radius-near-zero optimization, then `create`. |
| `new_radial_unchecked:215–231` | `new_radial_unchecked:107–115`: `radius.invert()`, Source `Translate(-center).post_scale(inv)` and `Radial{radius1:0,radius2:radius}`. `thin_ring` records the earlier replacement branch. |
| `create:316–362` | `create:117–153`: repeats Source center-distance calculation, ordinary `1/4096` degeneracy check, concentric matrix/`Radial`, nonconcentric `map_to_unit_x`, `Strip{r0/d}`, or Focal set and field projection. No new finite guard or focal clamp. |
| `map_to_unit_x:365–372` | `map_to_unit_x:155–162`: identical source/destination point order, maps first center to origin and second to unit x. |
| `ts_from_poly_to_poly:374–379` | `ts_from_poly_to_poly:164–169`: source base, inverse failure, destination base and `pre_concat` in that order. |
| `from_poly2:381–390` | `from_poly2:171–180`: identical six `Transform::from_row` expressions and order. |

`DEGENERATE_THRESHOLD` comes from tiny-skia
`src/shaders/gradient.rs:18` as `1.0/(1<<15) as f32`. The ordinary near-zero
predicates come from tiny-skia-path 0.12.0 `Scalar` and retain its
`1/(1<<12)` threshold. No condition was rewritten as an exact comparison
or as a new finite check.

The frozen type contains `None`, `SolidFirstStop`, or
`Conical{kind,points_to_unit,thin_ring}`. `SolidFirstStop` leaves the
caller to use its actual first color. `thin_ring=true` leaves the later
stop preparer to construct `(0,first),(1,first),(1,last)`; this module does
not fabricate stop data. The input `transform` participates in Source's
early invertibility check, while the returned matrix is only the Source
`points_to_unit` projection. Combined paint transform and `Gradient::new`
remain later responsibilities.

I did not build, test, run the 4,284-observation Source fixture, or declare
acceptance. Root owns independent bit-pattern comparison, compiling mutants,
regressions, paint preparation and later WARP/backend acceptance. No private
API gap was encountered within the frozen signature. Root subsequently removed
the two unused stage-selection predicates and retains that later responsibility.
