//! Shared paint resource stages; SVG and Direct2D consume this preparation.
mod types;
mod highp;
mod lowp;
pub use types::*;
pub use highp::resource_high_stage;
pub use lowp::resource_low_stage;
