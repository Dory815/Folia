//! Root-frozen declarations. Stubs establish a failing baseline before delegation.
use super::{gradient_program_types::ColorSpace,gradient_stop_types::StopColor};
pub fn approximate_power(_x:f32,_y:f32)->f32 {unimplemented!("M3h Source approx_powf")}
impl ColorSpace {
 pub fn expand_color(self,_color:StopColor)->StopColor {unimplemented!("M3h Source expand_color")}
}
