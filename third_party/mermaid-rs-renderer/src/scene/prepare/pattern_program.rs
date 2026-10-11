//! Selected original Pattern and Shader::Pattern preparation methods.
use tiny_skia_path::Transform;
use crate::scene::paint::SpreadMode;
use super::{pattern_types::{PreparedPattern,FilterQuality},pipeline_builder::PipelineBuilder,gradient_program_types::ColorSpace,sampling::pattern::SourcePixmap};
impl<'source> PreparedPattern<'source>{
    pub fn new(source:SourcePixmap<'source>,spread:SpreadMode,quality:FilterQuality,opacity:f32,transform:Transform)->Self{
        Self{source,spread,quality,opacity:tiny_skia_path::NormalizedF32::new_clamped(opacity),transform}
    }
    pub fn transform(&mut self,transform:Transform){
        self.transform=self.transform.post_concat(transform);
    }
    pub fn apply_opacity(&mut self,opacity:f32){
        self.opacity=tiny_skia_path::NormalizedF32::new(
            self.opacity.get()*<f32 as tiny_skia_path::Scalar>::bound(opacity,0.0,1.0)
        ).unwrap();
    }
    pub fn push_stages(&self,color_space:ColorSpace,builder:&mut PipelineBuilder)->bool{
        use super::pipeline_context_types::{SamplerContext,TileContext};
        use super::pipeline_control_types::PipelineStage;
        let ts=match self.transform.invert(){
            Some(value)=>value,
            None=>{
                log::warn!("failed to invert a pattern transform. Nothing will be rendered");
                return false;
            }
        };

        builder.push(PipelineStage::SeedShader);
        builder.push_transform(ts);

        let mut quality=self.quality;
        if ts.is_identity() || ts.is_translate(){
            quality=FilterQuality::Nearest;
        }
        if quality==FilterQuality::Bilinear{
            if ts.is_translate(){
                if ts.tx==ts.tx.trunc() && ts.ty==ts.ty.trunc(){
                    quality=FilterQuality::Nearest;
                }
            }
        }

        match quality{
            FilterQuality::Nearest=>{
                builder.context.limit_x=TileContext{
                    scale:self.source.width() as f32,
                    inv_scale:1.0/self.source.width() as f32,
                };
                builder.context.limit_y=TileContext{
                    scale:self.source.height() as f32,
                    inv_scale:1.0/self.source.height() as f32,
                };
                match self.spread{
                    SpreadMode::Pad=>{},
                    SpreadMode::Repeat=>builder.push(PipelineStage::Repeat),
                    SpreadMode::Reflect=>builder.push(PipelineStage::Reflect),
                }
                builder.push(PipelineStage::Gather);
            }
            FilterQuality::Bilinear=>{
                builder.context.sampler=SamplerContext{
                    spread_mode:self.spread,
                    inv_width:1.0/self.source.width() as f32,
                    inv_height:1.0/self.source.height() as f32,
                };
                builder.push(PipelineStage::Bilinear);
            }
            FilterQuality::Bicubic=>{
                builder.context.sampler=SamplerContext{
                    spread_mode:self.spread,
                    inv_width:1.0/self.source.width() as f32,
                    inv_height:1.0/self.source.height() as f32,
                };
                builder.push(PipelineStage::Bicubic);
                builder.push(PipelineStage::Clamp0);
                builder.push(PipelineStage::ClampA);
            }
        }

        if self.opacity!=tiny_skia_path::NormalizedF32::ONE{
            debug_assert_eq!(core::mem::size_of_val(&self.opacity),4,"alpha must be f32");
            builder.context.current_coverage=self.opacity.get();
            builder.push(PipelineStage::Scale1Float);
        }

        let expand_stage=match color_space{
            ColorSpace::Linear=>None,
            ColorSpace::Gamma2=>Some(PipelineStage::GammaExpand2),
            ColorSpace::SimpleSRGB=>Some(PipelineStage::GammaExpand22),
            ColorSpace::FullSRGBGamma=>Some(PipelineStage::GammaExpandSrgb),
        };
        if let Some(stage)=expand_stage{
            builder.push(stage);
        }

        true
    }
}
