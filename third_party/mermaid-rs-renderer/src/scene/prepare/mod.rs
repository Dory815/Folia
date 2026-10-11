//! Shared paint preparation; consumers receive resolved geometry.
pub mod radial_types;
pub mod radial_geometry;
pub mod radial_stage_types;
pub mod radial_stages;
pub mod gradient_stop_types;
pub mod gradient_stops;
pub mod gradient_program_types;
pub mod gradient_color_space;
pub mod gradient_program;
pub mod pipeline_control_types;
pub mod pipeline_control;
pub mod pipeline_context_types;
pub mod pipeline_builder;
pub mod pattern_types;
pub mod pattern_program;

pub mod sampling;
pub mod linear_program;
pub mod radial_program;
pub mod pipeline_executor_types;
pub mod pipeline_executor;
