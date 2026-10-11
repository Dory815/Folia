//! Pure blend stages operate on the shared register banks; caller owns continuation.
mod highp;
mod lowp;
pub use highp::blend_high_stage;
pub use lowp::blend_low_stage;
