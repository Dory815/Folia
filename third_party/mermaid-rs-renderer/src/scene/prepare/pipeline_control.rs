//! Portable projection of tiny-skia's whole-pipeline compile control.
use super::pipeline_control_types::*;

const MAX_STAGES: usize = 32;

/// Whether the original lowp STAGES entry is an executable function, not null_fn.
pub fn pipeline_stage_is_lowp(stage: PipelineStage) -> bool {
    match stage {
        PipelineStage::Clamp0
        | PipelineStage::ClampA
        | PipelineStage::Gather
        | PipelineStage::ColorBurn
        | PipelineStage::ColorDodge
        | PipelineStage::SoftLight
        | PipelineStage::Hue
        | PipelineStage::Saturation
        | PipelineStage::Color
        | PipelineStage::Luminosity
        | PipelineStage::Reflect
        | PipelineStage::Repeat
        | PipelineStage::Bilinear
        | PipelineStage::Bicubic
        | PipelineStage::XYToUnitAngle
        | PipelineStage::XYTo2PtConicalFocalOnCircle
        | PipelineStage::XYTo2PtConicalWellBehaved
        | PipelineStage::XYTo2PtConicalSmaller
        | PipelineStage::XYTo2PtConicalGreater
        | PipelineStage::XYTo2PtConicalStrip
        | PipelineStage::Mask2PtConicalNan
        | PipelineStage::Mask2PtConicalDegenerates
        | PipelineStage::ApplyVectorMask
        | PipelineStage::Alter2PtConicalCompensateFocal
        | PipelineStage::Alter2PtConicalUnswap
        | PipelineStage::NegateX
        | PipelineStage::ApplyConcentricScaleBias
        | PipelineStage::GammaExpand2
        | PipelineStage::GammaExpandDestination2
        | PipelineStage::GammaCompress2
        | PipelineStage::GammaExpand22
        | PipelineStage::GammaExpandDestination22
        | PipelineStage::GammaCompress22
        | PipelineStage::GammaExpandSrgb
        | PipelineStage::GammaExpandDestinationSrgb
        | PipelineStage::GammaCompressSrgb => false,

        PipelineStage::MoveSourceToDestination
        | PipelineStage::MoveDestinationToSource
        | PipelineStage::Premultiply
        | PipelineStage::UniformColor
        | PipelineStage::SeedShader
        | PipelineStage::LoadDestination
        | PipelineStage::Store
        | PipelineStage::LoadDestinationU8
        | PipelineStage::StoreU8
        | PipelineStage::LoadMaskU8
        | PipelineStage::MaskU8
        | PipelineStage::ScaleU8
        | PipelineStage::LerpU8
        | PipelineStage::Scale1Float
        | PipelineStage::Lerp1Float
        | PipelineStage::DestinationAtop
        | PipelineStage::DestinationIn
        | PipelineStage::DestinationOut
        | PipelineStage::DestinationOver
        | PipelineStage::SourceAtop
        | PipelineStage::SourceIn
        | PipelineStage::SourceOut
        | PipelineStage::SourceOver
        | PipelineStage::Clear
        | PipelineStage::Modulate
        | PipelineStage::Multiply
        | PipelineStage::Plus
        | PipelineStage::Screen
        | PipelineStage::Xor
        | PipelineStage::Darken
        | PipelineStage::Difference
        | PipelineStage::Exclusion
        | PipelineStage::HardLight
        | PipelineStage::Lighten
        | PipelineStage::Overlay
        | PipelineStage::SourceOverRgba
        | PipelineStage::Transform
        | PipelineStage::PadX1
        | PipelineStage::ReflectX1
        | PipelineStage::RepeatX1
        | PipelineStage::Gradient
        | PipelineStage::EvenlySpaced2StopGradient
        | PipelineStage::XYToRadius => true,
    }
}

/// Compile the complete final stage list, preserving Source order and capacity failures.
pub fn compile_pipeline<C: Default>(
    stages: &[PipelineStage],
    force_hq_pipeline: bool,
    context: C,
) -> CompiledPipeline<C> {
    // Source's builder.push uses ArrayVec<Stage, 32> before compile is called.
    if stages.len() > MAX_STAGES {
        panic!("pipeline stage push exceeds Source ArrayVec capacity");
    }

    // Source returns before eligibility and drops the builder's context.
    if stages.is_empty() {
        return CompiledPipeline {
            precision: PipelinePrecision::High,
            width: 8,
            full: Vec::new(),
            tail: Vec::new(),
            context: C::default(),
        };
    }

    let is_lowp_compatible = stages.iter().all(|&stage| pipeline_stage_is_lowp(stage));
    let precision = if force_hq_pipeline || !is_lowp_compatible {
        PipelinePrecision::High
    } else {
        PipelinePrecision::Low
    };

    let mut full = Vec::with_capacity(MAX_STAGES);
    full.extend(stages.iter().copied().map(PipelineOperation::Stage));
    // The original ArrayVec has no slot left to append just_return at 32 stages.
    if full.len() == MAX_STAGES {
        panic!("pipeline Return exceeds Source ArrayVec capacity");
    }
    full.push(PipelineOperation::Return);

    // Source clones the compiled function list, then replaces only five pointers.
    let mut tail = full.clone();
    for operation in &mut tail {
        *operation = match *operation {
            PipelineOperation::Stage(stage)
                if matches!(
                    stage,
                    PipelineStage::LoadDestination
                        | PipelineStage::Store
                        | PipelineStage::LoadDestinationU8
                        | PipelineStage::StoreU8
                        | PipelineStage::SourceOverRgba
                ) => PipelineOperation::Tail(stage),
            other => other,
        };
    }

    CompiledPipeline {
        precision,
        width: match precision {
            PipelinePrecision::High => 8,
            PipelinePrecision::Low => 16,
        },
        full,
        tail,
        context,
    }
}
