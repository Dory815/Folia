//! Original raster blend modes, separate from retained scene CSS paint modes.
#[repr(u8)]
#[derive(Debug,Clone,Copy,PartialEq,Eq,Default)]
pub enum RasterBlendMode{
 Clear,
 Source,
 Destination,
 #[default]
 SourceOver,
 DestinationOver,
 SourceIn,
 DestinationIn,
 SourceOut,
 DestinationOut,
 SourceAtop,
 DestinationAtop,
 Xor,
 Plus,
 Modulate,
 Screen,
 Overlay,
 Darken,
 Lighten,
 ColorDodge,
 ColorBurn,
 HardLight,
 SoftLight,
 Difference,
 Exclusion,
 Multiply,
 Hue,
 Saturation,
 Color,
 Luminosity,
}
pub const ALL_RASTER_BLEND_MODES:[RasterBlendMode;29]=[RasterBlendMode::Clear,RasterBlendMode::Source,RasterBlendMode::Destination,RasterBlendMode::SourceOver,RasterBlendMode::DestinationOver,RasterBlendMode::SourceIn,RasterBlendMode::DestinationIn,RasterBlendMode::SourceOut,RasterBlendMode::DestinationOut,RasterBlendMode::SourceAtop,RasterBlendMode::DestinationAtop,RasterBlendMode::Xor,RasterBlendMode::Plus,RasterBlendMode::Modulate,RasterBlendMode::Screen,RasterBlendMode::Overlay,RasterBlendMode::Darken,RasterBlendMode::Lighten,RasterBlendMode::ColorDodge,RasterBlendMode::ColorBurn,RasterBlendMode::HardLight,RasterBlendMode::SoftLight,RasterBlendMode::Difference,RasterBlendMode::Exclusion,RasterBlendMode::Multiply,RasterBlendMode::Hue,RasterBlendMode::Saturation,RasterBlendMode::Color,RasterBlendMode::Luminosity];
