// Copyright 2006 The Android Open Source Project
// Copyright 2020 Yevhenii Reizner
// BSD-style license: see LICENSE-tiny-skia-blend.
//! Complete Source stop preparation implementation packet; root-owned declarations.
use tiny_skia_path::Transform;
use crate::scene::paint::SpreadMode;
use super::gradient_stop_types::{StopColor, PreparedGradientStop, PreparedGradient};

impl StopColor {
    pub fn from_rgba(_r:f32,_g:f32,_b:f32,_a:f32)->Option<Self>{todo!("M3g implementation")}
    pub fn components(&self)->[f32;4]{self.components.map(|v|v.get())}
    pub fn is_opaque(&self)->bool{todo!("M3g implementation")}
    fn apply_opacity(&mut self,_opacity:f32){todo!("M3g implementation")}
}
impl PreparedGradientStop {
    pub fn new(_position:f32,_color:StopColor)->Self{todo!("M3g implementation")}
    pub fn position(&self)->f32{self.position.get()}
    pub fn color(&self)->StopColor{self.color}
}
impl PreparedGradient {
    /// Source precondition: at least two stops, after radial degenerate dispatch.
    pub fn new(_stops:Vec<PreparedGradientStop>,_spread:SpreadMode,_transform:Transform,_points_to_unit:Transform)->Self{todo!("M3g implementation")}
    pub fn apply_opacity(&mut self,_opacity:f32){todo!("M3g implementation")}
    pub fn stops(&self)->&[PreparedGradientStop]{&self.stops}
    pub fn spread(&self)->SpreadMode{self.spread}
    pub fn transform(&self)->Transform{self.transform}
    pub fn points_to_unit(&self)->Transform{self.points_to_unit}
    pub fn colors_are_opaque(&self)->bool{self.colors_are_opaque}
    pub fn has_uniform_stops(&self)->bool{self.has_uniform_stops}
}
