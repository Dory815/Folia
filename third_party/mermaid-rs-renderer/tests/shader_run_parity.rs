// Actual prepared shader -> sole builder -> compiled pipeline -> complete run.
// Expected values come exclusively from the unchanged original Source fixture.
use mermaid_rs_renderer::scene::{
    paint::SpreadMode,
    prepare::{
        gradient_program_types::ColorSpace,
        gradient_stop_types::{PreparedGradient, PreparedGradientStop, StopColor},
        linear_program::push_linear_gradient_stages,
        pattern_types::{FilterQuality, PreparedPattern},
        pipeline_builder::PipelineBuilder,
        pipeline_context_types::*,
        pipeline_control_types::{
            PipelineOperation, PipelinePrecision, PipelineStage, ALL_PIPELINE_STAGES,
        },
        pipeline_executor::run_pipeline,
        pipeline_executor_types::{PipelineResources, PipelineRunError},
        radial_program::push_radial_gradient_stages,
        radial_types::RadialKind,
        sampling::{
            pattern::SourcePixmap,
            resources::{AaMask, DestinationSurface, MaskSurface},
        },
    },
};
use serde::Deserialize;
use serde_json::Value;
use std::{collections::BTreeMap, panic::AssertUnwindSafe, sync::Mutex};
use tiny_skia_path::{IntRect, NormalizedF32, Transform};

#[derive(Deserialize)]
struct GradientInput {
    stops: Vec<[u32; 5]>,
    spread: u8,
    transform: [u32; 6],
    points_to_unit: [u32; 6],
    opacity: Vec<u32>,
    color_space: u8,
    kind: Option<(u8, [u32; 2], bool)>,
    prefix: usize,
    sentinel: bool,
    force: bool,
}

#[derive(Deserialize)]
struct PatternInput {
    width: u32,
    height: u32,
    spread: u8,
    quality: u8,
    opacity: u32,
    transform: [u32; 6],
    transforms: Vec<[u32; 6]>,
    opacities: Vec<u32>,
    color_space: u8,
    prefix: usize,
    sentinel: bool,
    force: bool,
}

#[derive(Deserialize)]
struct Input {
    family: String,
    source_index: usize,
    producer: Value,
    image: Vec<u8>,
    ids: Vec<u8>,
    destination: Vec<u8>,
    real_width: usize,
    mask: Vec<u8>,
    mask_width: u32,
    aa_pixels: [u8; 2],
    aa_stride: u32,
    aa_shift: usize,
    origin: [u32; 2],
    size: [u32; 2],
}

type RunRecord = (
    bool,
    usize,
    Vec<(u8, bool)>,
    Vec<(u8, bool)>,
    bool,
    Vec<u8>,
    Vec<Vec<u32>>,
    Vec<Vec<u32>>,
);

#[derive(Deserialize)]
struct Case {
    input: Input,
    result: (Value, Option<RunRecord>),
    warnings: Vec<String>,
}

#[derive(Deserialize)]
struct Fixture {
    cases: Vec<Case>,
}

static WARNINGS: Mutex<Vec<String>> = Mutex::new(Vec::new());
struct Observer;
static OBSERVER: Observer = Observer;

impl log::Log for Observer {
    fn enabled(&self, metadata: &log::Metadata) -> bool {
        metadata.level() <= log::Level::Warn
    }
    fn log(&self, record: &log::Record) {
        if self.enabled(record.metadata()) {
            WARNINGS.lock().unwrap().push(record.args().to_string());
        }
    }
    fn flush(&self) {}
}

fn transform(bits: [u32; 6]) -> Transform {
    let v = bits.map(f32::from_bits);
    Transform::from_row(v[0], v[1], v[2], v[3], v[4], v[5])
}

fn spread(value: u8) -> SpreadMode {
    match value {
        0 => SpreadMode::Pad,
        1 => SpreadMode::Reflect,
        _ => SpreadMode::Repeat,
    }
}

fn color_space(value: u8) -> ColorSpace {
    match value {
        0 => ColorSpace::Linear,
        1 => ColorSpace::Gamma2,
        2 => ColorSpace::SimpleSRGB,
        _ => ColorSpace::FullSRGBGamma,
    }
}

fn sentinel(c: &mut PipelineContext) {
    c.current_coverage = 0.25;
    c.sampler = SamplerContext {
        spread_mode: SpreadMode::Reflect,
        inv_width: 0.125,
        inv_height: 0.375,
    };
    c.uniform_color = UniformColor {
        components: [0.1, 0.2, 0.3, 0.4],
        rgba: [1, 2, 3, 4],
    };
    c.two_stop = TwoStopContext {
        factor: GradientColor {
            components: [2.0, 3.0, 4.0, 5.0],
        },
        bias: GradientColor {
            components: [6.0, 7.0, 8.0, 9.0],
        },
    };
    c.gradient = GradientContext {
        len: 1,
        factors: vec![GradientColor {
            components: [10.0, 11.0, 12.0, 13.0],
        }],
        biases: vec![GradientColor {
            components: [14.0, 15.0, 16.0, 17.0],
        }],
        t_values: vec![NormalizedF32::new(0.5).unwrap()],
    };
    c.radial_p = [12.0, 13.0];
    c.radial_mask = [1, 2, 3, 4, 5, 6, 7, 8];
    c.limit_x = TileContext {
        scale: 14.0,
        inv_scale: 15.0,
    };
    c.limit_y = TileContext {
        scale: 16.0,
        inv_scale: 17.0,
    };
    c.transform = Transform::from_row(2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
}

// Exactly the first fifteen Source producer groups. Force is builder metadata.
fn context_groups(c: &PipelineContext) -> Vec<Vec<u32>> {
    vec![
        vec![c.current_coverage.to_bits()],
        vec![
            match c.sampler.spread_mode {
                SpreadMode::Pad => 0,
                SpreadMode::Reflect => 1,
                SpreadMode::Repeat => 2,
            },
            c.sampler.inv_width.to_bits(),
            c.sampler.inv_height.to_bits(),
        ],
        c.uniform_color.components.map(f32::to_bits).to_vec(),
        c.uniform_color.rgba.map(u32::from).to_vec(),
        c.two_stop.factor.components.map(f32::to_bits).to_vec(),
        c.two_stop.bias.components.map(f32::to_bits).to_vec(),
        vec![
            c.gradient.len as u32,
            c.gradient.factors.len() as u32,
            c.gradient.biases.len() as u32,
            c.gradient.t_values.len() as u32,
        ],
        c.gradient
            .factors
            .iter()
            .flat_map(|v| v.components.map(f32::to_bits))
            .collect(),
        c.gradient
            .biases
            .iter()
            .flat_map(|v| v.components.map(f32::to_bits))
            .collect(),
        c.gradient
            .t_values
            .iter()
            .map(|v| v.get().to_bits())
            .collect(),
        c.radial_p.map(f32::to_bits).to_vec(),
        c.radial_mask.to_vec(),
        [c.limit_x.scale, c.limit_x.inv_scale]
            .map(f32::to_bits)
            .to_vec(),
        [c.limit_y.scale, c.limit_y.inv_scale]
            .map(f32::to_bits)
            .to_vec(),
        [
            c.transform.sx,
            c.transform.ky,
            c.transform.kx,
            c.transform.sy,
            c.transform.tx,
            c.transform.ty,
        ]
        .map(f32::to_bits)
        .to_vec(),
    ]
}

fn producer_groups(builder: &PipelineBuilder) -> Vec<Vec<u32>> {
    let mut groups = context_groups(&builder.context);
    groups.push(vec![u32::from(builder.force_hq_pipeline())]);
    groups
}

fn operations(values: &[PipelineOperation]) -> Vec<(u8, bool)> {
    values
        .iter()
        .map(|op| match op {
            PipelineOperation::Stage(stage) => (*stage as u8, false),
            PipelineOperation::Tail(stage) => (*stage as u8, true),
            PipelineOperation::Return => (79, false),
        })
        .collect()
}

#[derive(Default)]
struct NativeObservation {
    producer: Option<Value>,
    run: Option<RunRecord>,
    warnings: Vec<String>,
    producer_panic: bool,
    compile_panic: bool,
    run_error: Option<PipelineRunError>,
}

fn run_builder(
    mut builder: PipelineBuilder,
    input: &Input,
    source: SourcePixmap<'_>,
    observation: &mut NativeObservation,
) {
    for id in &input.ids {
        builder.push(ALL_PIPELINE_STAGES[*id as usize]);
    }
    let compiled = std::panic::catch_unwind(AssertUnwindSafe(|| builder.compile()));
    let Ok(mut compiled) = compiled else {
        observation.compile_panic = true;
        return;
    };
    let before = context_groups(&compiled.context);
    let mut destination = input.destination.clone();
    let rect = IntRect::from_xywh(
        input.origin[0] as i32,
        input.origin[1] as i32,
        input.size[0],
        input.size[1],
    )
    .unwrap();
    let result = std::panic::catch_unwind(AssertUnwindSafe(|| {
        run_pipeline(
            &mut compiled,
            &rect,
            PipelineResources {
                source,
                destination: DestinationSurface {
                    data: &mut destination,
                    real_width: input.real_width,
                },
                mask: MaskSurface {
                    data: &input.mask,
                    real_width: input.mask_width,
                },
                aa_mask: AaMask {
                    pixels: input.aa_pixels,
                    stride: input.aa_stride,
                    shift: input.aa_shift,
                },
            },
        )
    }));
    if let Ok(Err(error)) = &result {
        observation.run_error = Some(*error);
    }
    observation.run = Some((
        compiled.precision == PipelinePrecision::High,
        compiled.width,
        operations(&compiled.full),
        operations(&compiled.tail),
        result.is_err(),
        destination,
        before,
        context_groups(&compiled.context),
    ));
}

fn observe_gradient(input: &Input, g: GradientInput) -> NativeObservation {
    let mut observation = NativeObservation::default();
    let stops: Option<Vec<_>> = g
        .stops
        .iter()
        .map(|v| {
            let a = v.map(f32::from_bits);
            StopColor::from_rgba(a[1], a[2], a[3], a[4])
                .map(|color| PreparedGradientStop::new(a[0], color))
        })
        .collect();
    let Some(stops) = stops else {
        observation.warnings = std::mem::take(&mut *WARNINGS.lock().unwrap());
        return observation;
    };
    let mut gradient = PreparedGradient::new(
        stops,
        spread(g.spread),
        transform(g.transform),
        transform(g.points_to_unit),
    );
    for opacity in &g.opacity {
        gradient.apply_opacity(f32::from_bits(*opacity));
    }
    let kind = g.kind.map(|(tag, values, swapped)| {
        let values = values.map(f32::from_bits);
        match tag {
            2 => RadialKind::Radial {
                radius1: values[0],
                radius2: values[1],
            },
            3 => RadialKind::Strip {
                scaled_r0: values[0],
            },
            4 => RadialKind::Focal {
                r1: values[0],
                focal_x: values[1],
                is_swapped: swapped,
            },
            _ => panic!("bad Source radial kind"),
        }
    });
    let mut builder = PipelineBuilder::new();
    builder.set_force_hq_pipeline(g.force);
    if g.sentinel {
        sentinel(&mut builder.context);
    }
    for _ in 0..g.prefix {
        builder.push(PipelineStage::Premultiply);
    }
    let result = std::panic::catch_unwind(AssertUnwindSafe(|| match kind {
        None => push_linear_gradient_stages(&gradient, color_space(g.color_space), &mut builder),
        Some(kind) => push_radial_gradient_stages(
            &gradient,
            kind,
            color_space(g.color_space),
            &mut builder,
        ),
    }));
    let accepted = matches!(&result, Ok(true));
    observation.producer_panic = result.is_err();
    observation.producer = Some(
        serde_json::to_value((
            match result {
                Ok(value) => value.to_string(),
                Err(_) => "panic".to_owned(),
            },
            builder
                .stages()
                .iter()
                .map(|stage| format!("{stage:?}"))
                .collect::<Vec<_>>(),
            producer_groups(&builder),
        ))
        .unwrap(),
    );
    if accepted {
        let dummy = [0u8; 4];
        let source = SourcePixmap::from_bytes(&dummy, 1, 1).unwrap();
        run_builder(builder, input, source, &mut observation);
    }
    observation.warnings = std::mem::take(&mut *WARNINGS.lock().unwrap());
    observation
}

fn observe_pattern(input: &Input, p: PatternInput) -> NativeObservation {
    let source = SourcePixmap::from_bytes(&input.image, p.width, p.height).unwrap();
    let mut pattern = PreparedPattern::new(
        source,
        spread(p.spread),
        match p.quality {
            0 => FilterQuality::Nearest,
            1 => FilterQuality::Bilinear,
            _ => FilterQuality::Bicubic,
        },
        f32::from_bits(p.opacity),
        transform(p.transform),
    );
    let mut builder = PipelineBuilder::new();
    builder.set_force_hq_pipeline(p.force);
    if p.sentinel {
        sentinel(&mut builder.context);
    }
    for _ in 0..p.prefix {
        builder.push(PipelineStage::Premultiply);
    }
    let result = std::panic::catch_unwind(AssertUnwindSafe(|| {
        for t in &p.transforms {
            pattern.transform(transform(*t));
        }
        for opacity in &p.opacities {
            pattern.apply_opacity(f32::from_bits(*opacity));
        }
        pattern.push_stages(color_space(p.color_space), &mut builder)
    }));
    let mut observation = NativeObservation::default();
    let accepted = matches!(&result, Ok(true));
    observation.producer_panic = result.is_err();
    let t = pattern.transform;
    observation.producer = Some(
        serde_json::to_value((
            match result {
                Ok(value) => value.to_string(),
                Err(_) => "panic".to_owned(),
            },
            vec![
                pattern.source.width(),
                pattern.source.height(),
                match pattern.spread {
                    SpreadMode::Pad => 0,
                    SpreadMode::Reflect => 1,
                    SpreadMode::Repeat => 2,
                },
                pattern.quality as u32,
                pattern.opacity.get().to_bits(),
                t.sx.to_bits(),
                t.ky.to_bits(),
                t.kx.to_bits(),
                t.sy.to_bits(),
                t.tx.to_bits(),
                t.ty.to_bits(),
            ],
            builder
                .stages()
                .iter()
                .map(|stage| format!("{stage:?}"))
                .collect::<Vec<_>>(),
            producer_groups(&builder),
            pattern.source.data().as_ptr() == input.image.as_ptr(),
        ))
        .unwrap(),
    );
    if accepted {
        run_builder(builder, input, pattern.source, &mut observation);
    }
    observation.warnings = std::mem::take(&mut *WARNINGS.lock().unwrap());
    observation
}

#[test]
fn actual_original_shader_producer_then_complete_run() {
    log::set_logger(&OBSERVER).unwrap();
    log::set_max_level(log::LevelFilter::Warn);
    let previous_hook = std::panic::take_hook();
    std::panic::set_hook(Box::new(|_| {}));

    let mut fixture: Fixture = serde_json::from_str(include_str!("fixtures/shader-run.json")).unwrap();
    assert_eq!(fixture.cases.len(), 10_016);
    let boundary: Fixture = serde_json::from_str(include_str!("fixtures/shader-run-sampling-boundary.json")).unwrap();
    assert_eq!(boundary.cases.len(), 3_564);
    fixture.cases.extend(boundary.cases);
    assert_eq!(fixture.cases.len(), 13_580);
    let mut mismatches = 0usize;
    let mut categories = BTreeMap::<&'static str, usize>::new();
    let mut first = Vec::new();
    for (index, case) in fixture.cases.iter().enumerate() {
        let input = &case.input;
        let observation = match input.family.as_str() {
            "gradient" => observe_gradient(
                input,
                serde_json::from_value(input.producer.clone()).unwrap(),
            ),
            "pattern" => observe_pattern(
                input,
                serde_json::from_value(input.producer.clone()).unwrap(),
            ),
            _ => panic!("unknown original shader family"),
        };
        let mut differences = Vec::new();
        if observation.producer.as_ref() != Some(&case.result.0) {
            differences.push("producer");
        }
        if observation.warnings != case.warnings {
            differences.push("warnings");
        }
        if observation.producer_panic {
            differences.push("producer_panic");
        }
        if observation.compile_panic {
            differences.push("compile_panic");
        }
        if let Some(error) = observation.run_error {
            differences.push(match error {
                PipelineRunError::Unimplemented => "unimplemented",
                _ => "run_error",
            });
        }
        match (&observation.run, &case.result.1) {
            (Some(actual), Some(expected)) => {
                if actual.0 != expected.0 {
                    differences.push("precision");
                }
                if actual.1 != expected.1 {
                    differences.push("width");
                }
                if actual.2 != expected.2 {
                    differences.push("full_operations");
                }
                if actual.3 != expected.3 {
                    differences.push("tail_operations");
                }
                if actual.4 != expected.4 {
                    differences.push("panic");
                }
                if actual.5 != expected.5 {
                    differences.push("destination_bytes");
                    if first.len() < 5 {
                        let bytes: Vec<_> = actual.5.iter().zip(&expected.5).enumerate()
                            .filter_map(|(i,(a,e))| (a!=e).then_some((i,*a,*e))).take(8).collect();
                        println!("case {index} first differing destination bytes (offset,native,Source): {bytes:?}");
                    }
                }
                if actual.6 != expected.6 {
                    differences.push("context_before");
                }
                if actual.7 != expected.7 {
                    differences.push("context_after");
                }
            }
            (None, None) => {}
            _ => differences.push("missing_run"),
        }
        if !differences.is_empty() {
            mismatches += 1;
            for category in &differences {
                *categories.entry(*category).or_default() += 1;
            }
            if first.len() < 5 {
                first.push(format!(
                    "case {index} {} source_index={} categories={}",
                    input.family,
                    input.source_index,
                    differences.join(",")
                ));
            }
        }
    }
    std::panic::set_hook(previous_hook);
    let stubs = categories.get("unimplemented").copied().unwrap_or(0);
    let errors = stubs + categories.get("run_error").copied().unwrap_or(0);
    println!(
        "13580 actual Source shader complete run cases / {mismatches} mismatches / {stubs} actual stub calls / {errors} errors / categories={categories:?}"
    );
    assert_eq!(mismatches, 0, "{}", first.join("\n"));
}
