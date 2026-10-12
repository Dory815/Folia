// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Complete Source stop preparation implementation packet; root-owned declarations.
use tiny_skia_path::{NormalizedF32, Scalar, Transform};
use crate::scene::paint::SpreadMode;
use super::gradient_stop_types::{StopColor, PreparedGradientStop, PreparedGradient};

impl StopColor {
    pub fn from_rgba(r: f32, g: f32, b: f32, a: f32) -> Option<Self> {
        Some(Self {
            components: [
                NormalizedF32::new(r)?,
                NormalizedF32::new(g)?,
                NormalizedF32::new(b)?,
                NormalizedF32::new(a)?,
            ],
        })
    }
    pub fn components(&self)->[f32;4]{self.components.map(|v|v.get())}
    pub fn premultiply(&self)->super::gradient_stop_types::PremultipliedStopColor{
        use super::gradient_stop_types::PremultipliedStopColor;
        if self.is_opaque(){
            PremultipliedStopColor{components:self.components}
        }else{
            PremultipliedStopColor{components:[
                NormalizedF32::new_clamped(self.components[0].get()*self.components[3].get()),
                NormalizedF32::new_clamped(self.components[1].get()*self.components[3].get()),
                NormalizedF32::new_clamped(self.components[2].get()*self.components[3].get()),
                self.components[3],
            ]}
        }
    }
    pub fn is_opaque(&self) -> bool { self.components[3] == NormalizedF32::ONE }
    pub(crate) fn apply_opacity(&mut self, opacity: f32) {
        self.components[3] = NormalizedF32::new_clamped(
            self.components[3].get() * opacity.bound(0.0, 1.0),
        );
    }
}
impl PreparedGradientStop {
    pub fn new(position: f32, color: StopColor) -> Self {
        Self { position: NormalizedF32::new_clamped(position), color }
    }
    pub fn position(&self)->f32{self.position.get()}
    pub fn color(&self)->StopColor{self.color}
}
impl PreparedGradient {
    /// Source precondition: at least two stops, after radial degenerate dispatch.
    pub fn new(mut stops: Vec<PreparedGradientStop>, spread: SpreadMode,
        transform: Transform, points_to_unit: Transform) -> Self {
        debug_assert!(stops.len() > 1);

        let dummy_first = stops[0].position.get() != 0.0;
        let dummy_last = stops[stops.len() - 1].position.get() != 1.0;

        if dummy_first {
            stops.insert(0, PreparedGradientStop::new(0.0, stops[0].color));
        }
        if dummy_last {
            stops.push(PreparedGradientStop::new(1.0, stops[stops.len() - 1].color));
        }

        let colors_are_opaque = stops.iter().all(|p| p.color.is_opaque());

        let start_index = if dummy_first { 0 } else { 1 };
        let mut prev = 0.0;
        let mut has_uniform_stops = true;
        let uniform_step = stops[start_index].position.get() - prev;
        for i in start_index..stops.len() {
            let curr = if i + 1 == stops.len() {
                1.0
            } else {
                stops[i].position.get().bound(prev, 1.0)
            };

            has_uniform_stops &= uniform_step.is_nearly_equal(curr - prev);
            stops[i].position = NormalizedF32::new_clamped(curr);
            prev = curr;
        }

        Self { stops, spread, transform, points_to_unit,
            colors_are_opaque, has_uniform_stops }
    }
    pub fn apply_opacity(&mut self, opacity: f32) {
        for stop in &mut self.stops {
            stop.color.apply_opacity(opacity);
        }

        self.colors_are_opaque = self.stops.iter().all(|p| p.color.is_opaque());
    }
    pub fn stops(&self)->&[PreparedGradientStop]{&self.stops}
    pub fn spread(&self)->SpreadMode{self.spread}
    pub fn transform(&self)->Transform{self.transform}
    pub fn points_to_unit(&self)->Transform{self.points_to_unit}
    pub fn colors_are_opaque(&self)->bool{self.colors_are_opaque}
    pub fn has_uniform_stops(&self)->bool{self.has_uniform_stops}
}

impl super::gradient_stop_types::PremultipliedStopColor{
 pub fn to_color_u8(&self)->[u8;4]{
  [
   (self.components[0].get()*255.0+0.5) as u8,
   (self.components[1].get()*255.0+0.5) as u8,
   (self.components[2].get()*255.0+0.5) as u8,
   (self.components[3].get()*255.0+0.5) as u8,
  ]
 }
}
