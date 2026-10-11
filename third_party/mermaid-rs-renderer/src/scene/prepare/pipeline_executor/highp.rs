//! Original highp::start control flow; numerical stages remain separate.
use tiny_skia_path::IntRect;
use super::super::{
    pipeline_control_types::{CompiledPipeline, PipelineOperation, PipelineStage},
    pipeline_context_types::PipelineContext,
    pipeline_executor_types::{PipelineResources, PipelineRunError},
    sampling::{
        HighRegisters,
        sample_high_stage,
        blending::blend_high_stage,
        pattern::{pattern_high_stage, PatternContext, SourcePixmap},
        resources::{
            resource_high_stage, AaMask, DestinationSurface, MaskSurface, ResourceContext,
            ResourceCursor, ResourceProgram, ResourceStageError, StageControl,
        },
    },
};

const STAGE_WIDTH: usize = 8;

pub(super) fn start(
    pipeline: &mut CompiledPipeline<PipelineContext>,
    rect: &IntRect,
    resources: PipelineResources<'_, '_, '_>,
) -> Result<(), PipelineRunError> {
    let PipelineResources { source, mut destination, mask, aa_mask } = resources;
    let mut registers = HighRegisters::default();
    let full = &pipeline.full;
    let tail = &pipeline.tail;
    let context = &mut pipeline.context;

    for y in rect.y()..rect.bottom() {
        let mut x = rect.x() as usize;
        let end = rect.right() as usize;

        while x + STAGE_WIDTH <= end {
            run_tile(full, &mut registers, context, source, &mut destination, mask, aa_mask,
                ResourceCursor { dx: x, dy: y as usize, tail: STAGE_WIDTH, program: ResourceProgram::Full })?;
            x += STAGE_WIDTH;
        }

        if x != end {
            run_tile(tail, &mut registers, context, source, &mut destination, mask, aa_mask,
                ResourceCursor { dx: x, dy: y as usize, tail: end - x, program: ResourceProgram::Tail })?;
        }
    }
    Ok(())
}

fn run_tile(
    operations: &[PipelineOperation],
    registers: &mut HighRegisters,
    context: &mut PipelineContext,
    source: SourcePixmap<'_>,
    destination: &mut DestinationSurface<'_>,
    mask: MaskSurface<'_>,
    aa_mask: AaMask,
    cursor: ResourceCursor,
) -> Result<(), PipelineRunError> {
    use PipelineStage::*;
    let mut index = 0;
    loop {
        let operation = operations[index];
        index += 1;
        let (stage, program) = match operation {
            PipelineOperation::Return => return Ok(()),
            PipelineOperation::Stage(stage) => (stage, ResourceProgram::Full),
            PipelineOperation::Tail(stage) => (stage, ResourceProgram::Tail),
        };

        match stage {
            // The original highp A8 functions have no next_stage call.
            LoadDestinationU8 | StoreU8 | LoadMaskU8 => return Ok(()),

            MoveSourceToDestination | MoveDestinationToSource | Clamp0 | ClampA
            | Premultiply | SeedShader | Transform | PadX1 | ReflectX1 | RepeatX1
            | Gradient | EvenlySpaced2StopGradient | XYToUnitAngle | XYToRadius
            | XYTo2PtConicalFocalOnCircle | XYTo2PtConicalWellBehaved
            | XYTo2PtConicalSmaller | XYTo2PtConicalGreater | XYTo2PtConicalStrip
            | Mask2PtConicalNan | Mask2PtConicalDegenerates | ApplyVectorMask
            | Alter2PtConicalCompensateFocal | Alter2PtConicalUnswap | NegateX
            | ApplyConcentricScaleBias | GammaExpand2 | GammaExpandDestination2
            | GammaCompress2 | GammaExpand22 | GammaExpandDestination22
            | GammaCompress22 | GammaExpandSrgb | GammaExpandDestinationSrgb
            | GammaCompressSrgb => {
                sample_high_stage(stage, registers, &mut *context, [cursor.dx, cursor.dy])
                    .map_err(|error| PipelineRunError::Unsupported {
                        precision: error.precision, stage: error.stage,
                    })?;
            }

            Gather | Reflect | Repeat | Bilinear | Bicubic => {
                let view = PatternContext { source, pipeline: &*context };
                pattern_high_stage(stage, registers, &view)
                    .map_err(|error| PipelineRunError::Unsupported {
                        precision: error.precision, stage: error.stage,
                    })?;
            }

            DestinationAtop | DestinationIn | DestinationOut | DestinationOver
            | SourceAtop | SourceIn | SourceOut | SourceOver | Clear | Modulate
            | Multiply | Plus | Screen | Xor | ColorBurn | ColorDodge | Darken
            | Difference | Exclusion | HardLight | Lighten | Overlay | SoftLight
            | Hue | Saturation | Color | Luminosity => {
                blend_high_stage(stage, registers)
                    .map_err(|error| PipelineRunError::Unsupported {
                        precision: error.precision, stage: error.stage,
                    })?;
            }

            UniformColor | LoadDestination | Store | MaskU8 | ScaleU8
            | LerpU8 | Scale1Float | Lerp1Float | SourceOverRgba => {
                let real_width = destination.real_width;
                let mut view = ResourceContext {
                    pipeline: &mut *context,
                    destination: DestinationSurface {
                        data: &mut *destination.data,
                        real_width,
                    },
                    mask,
                    aa_mask,
                };
                let stage_cursor = ResourceCursor { program, ..cursor };
                match resource_high_stage(stage, registers, &mut view, stage_cursor) {
                    Ok(StageControl::Continue) => {}
                    Ok(StageControl::Stop) => return Ok(()),
                    Err(ResourceStageError::Unimplemented { .. }) => return Err(PipelineRunError::Unimplemented),
                    Err(ResourceStageError::Unsupported { precision, stage }) => {
                        return Err(PipelineRunError::Unsupported { precision, stage });
                    }
                }
            }
        }
    }
}
