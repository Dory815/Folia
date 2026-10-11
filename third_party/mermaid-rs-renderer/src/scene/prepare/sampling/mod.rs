//! Shared resource-independent gradient sampling; no SVG or platform APIs.
mod wide;
mod highp;
mod lowp;
mod types;
pub mod resources;
pub mod blending;
pub use highp::sample_high_stage;
pub use lowp::sample_low_stage;
pub use types::{HighRegisters,LowRegisters,SamplingError};
