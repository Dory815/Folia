//! Original BlendMode policy used by blitter composition.
use super::{blend_policy_types::RasterBlendMode,pipeline_control_types::PipelineStage};
impl RasterBlendMode{
 pub fn should_pre_scale_coverage(self)->bool{
  matches!(self,
   RasterBlendMode::Destination |
   RasterBlendMode::DestinationOver |
   RasterBlendMode::Plus |
   RasterBlendMode::DestinationOut |
   RasterBlendMode::SourceAtop |
   RasterBlendMode::SourceOver |
   RasterBlendMode::Xor
  )
 }
 pub fn to_stage(self)->Option<PipelineStage>{
  match self{
   RasterBlendMode::Clear=>Some(PipelineStage::Clear),
   RasterBlendMode::Source=>None,
   RasterBlendMode::Destination=>Some(PipelineStage::MoveDestinationToSource),
   RasterBlendMode::SourceOver=>Some(PipelineStage::SourceOver),
   RasterBlendMode::DestinationOver=>Some(PipelineStage::DestinationOver),
   RasterBlendMode::SourceIn=>Some(PipelineStage::SourceIn),
   RasterBlendMode::DestinationIn=>Some(PipelineStage::DestinationIn),
   RasterBlendMode::SourceOut=>Some(PipelineStage::SourceOut),
   RasterBlendMode::DestinationOut=>Some(PipelineStage::DestinationOut),
   RasterBlendMode::SourceAtop=>Some(PipelineStage::SourceAtop),
   RasterBlendMode::DestinationAtop=>Some(PipelineStage::DestinationAtop),
   RasterBlendMode::Xor=>Some(PipelineStage::Xor),
   RasterBlendMode::Plus=>Some(PipelineStage::Plus),
   RasterBlendMode::Modulate=>Some(PipelineStage::Modulate),
   RasterBlendMode::Screen=>Some(PipelineStage::Screen),
   RasterBlendMode::Overlay=>Some(PipelineStage::Overlay),
   RasterBlendMode::Darken=>Some(PipelineStage::Darken),
   RasterBlendMode::Lighten=>Some(PipelineStage::Lighten),
   RasterBlendMode::ColorDodge=>Some(PipelineStage::ColorDodge),
   RasterBlendMode::ColorBurn=>Some(PipelineStage::ColorBurn),
   RasterBlendMode::HardLight=>Some(PipelineStage::HardLight),
   RasterBlendMode::SoftLight=>Some(PipelineStage::SoftLight),
   RasterBlendMode::Difference=>Some(PipelineStage::Difference),
   RasterBlendMode::Exclusion=>Some(PipelineStage::Exclusion),
   RasterBlendMode::Multiply=>Some(PipelineStage::Multiply),
   RasterBlendMode::Hue=>Some(PipelineStage::Hue),
   RasterBlendMode::Saturation=>Some(PipelineStage::Saturation),
   RasterBlendMode::Color=>Some(PipelineStage::Color),
   RasterBlendMode::Luminosity=>Some(PipelineStage::Luminosity),
  }
 }
}
