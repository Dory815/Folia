use crate::config::LayoutConfig;
use crate::layout::{DiagramData, Layout};
use serde::Serialize;

#[derive(Debug, Clone, Copy, PartialEq, Serialize)]
pub struct DiagramDimensions {
    pub width: f32,
    pub height: f32,
    pub viewbox_x: f32,
    pub viewbox_y: f32,
    pub viewbox_width: f32,
    pub viewbox_height: f32,
}

impl DiagramDimensions {
    pub fn aspect_ratio(self) -> f32 {
        self.width / self.height.max(1.0)
    }

    pub fn viewbox_aspect_ratio(self) -> f32 {
        self.viewbox_width / self.viewbox_height.max(1.0)
    }
}

fn fit_dimensions_to_preferred_ratio(
    width: f32,
    height: f32,
    preferred_ratio: Option<f32>,
) -> (f32, f32) {
    let mut width = width.max(1.0);
    let mut height = height.max(1.0);
    let Some(target_ratio) = preferred_ratio else {
        return (width, height);
    };
    if !target_ratio.is_finite() || target_ratio <= 0.0 {
        return (width, height);
    }
    let current_ratio = width / height;
    if (current_ratio - target_ratio).abs() < 1e-6 {
        return (width, height);
    }
    if current_ratio < target_ratio {
        width = height * target_ratio;
    } else {
        height = width / target_ratio;
    }
    (width.max(1.0), height.max(1.0))
}

pub(crate) const SEQUENCE_VIEWBOX_PAD_LEFT: f32 = 50.0;
pub(crate) const SEQUENCE_VIEWBOX_PAD_RIGHT: f32 = 50.0;
pub(crate) const SEQUENCE_VIEWBOX_PAD_TOP: f32 = 10.0;
pub(crate) const SEQUENCE_VIEWBOX_PAD_BOTTOM: f32 = 11.0;

pub fn measure_layout_dimensions(
    layout: &Layout,
    config: &LayoutConfig,
    dimensions: Option<(f32, f32)>,
) -> DiagramDimensions {
    let is_sequence = matches!(layout.diagram, DiagramData::Sequence(_));
    let (width, height, viewbox_x, viewbox_y, viewbox_width, viewbox_height) =
        if let DiagramData::Error(error) = &layout.diagram {
            (
                error.render_width,
                error.render_height,
                0.0,
                0.0,
                error.viewbox_width,
                error.viewbox_height,
            )
        } else if layout.kind == crate::ir::DiagramKind::Requirement {
            let pad_x = config.requirement.render_padding_x;
            let pad_y = config.requirement.render_padding_y;
            let mut width = layout.width + pad_x * 2.0;
            let mut height = layout.height + pad_y * 2.0;
            width = width.max(1.0);
            height = height.max(1.0);
            (width, height, 0.0, 0.0, width, height)
        } else if let DiagramData::C4(c4) = &layout.diagram {
            let width = layout.width.max(1.0);
            let height = layout.height.max(1.0);
            (
                width,
                height,
                c4.viewbox_x,
                c4.viewbox_y,
                c4.viewbox_width,
                c4.viewbox_height,
            )
        } else if let DiagramData::GitGraph(gitgraph) = &layout.diagram {
            let width = layout.width.max(1.0);
            let height = layout.height.max(1.0);
            let viewbox_x = -gitgraph.offset_x;
            let viewbox_y = -gitgraph.offset_y;
            (
                width,
                height,
                viewbox_x,
                viewbox_y,
                gitgraph.width,
                gitgraph.height,
            )
        } else if layout.kind == crate::ir::DiagramKind::Mindmap {
            let pad = config.mindmap.padding;
            let mut min_x = f32::MAX;
            let mut min_y = f32::MAX;
            let mut max_x = f32::MIN;
            let mut max_y = f32::MIN;
            for node in layout.nodes.values() {
                min_x = min_x.min(node.x);
                min_y = min_y.min(node.y);
                max_x = max_x.max(node.x + node.width);
                max_y = max_y.max(node.y + node.height);
            }
            if min_x == f32::MAX {
                min_x = 0.0;
                max_x = 1.0;
            }
            if min_y == f32::MAX {
                min_y = 0.0;
                max_y = 1.0;
            }
            let width = (max_x - min_x + pad * 2.0).max(1.0);
            let height = (max_y - min_y + pad * 2.0).max(1.0);
            let viewbox_x = min_x - pad;
            let viewbox_y = min_y - pad;
            (width, height, viewbox_x, viewbox_y, width, height)
        } else if is_sequence {
            let width =
                (layout.width + SEQUENCE_VIEWBOX_PAD_LEFT + SEQUENCE_VIEWBOX_PAD_RIGHT).max(1.0);
            let height =
                (layout.height + SEQUENCE_VIEWBOX_PAD_TOP + SEQUENCE_VIEWBOX_PAD_BOTTOM).max(1.0);
            (
                width,
                height,
                -SEQUENCE_VIEWBOX_PAD_LEFT,
                -SEQUENCE_VIEWBOX_PAD_TOP,
                width,
                height,
            )
        } else {
            let width = layout.width.max(1.0);
            let height = layout.height.max(1.0);
            (width, height, 0.0, 0.0, width, height)
        };

    let preferred_ratio = config
        .preferred_aspect_ratio
        .filter(|ratio| ratio.is_finite() && *ratio > 0.0);
    let (mut target_width, mut target_height) =
        fit_dimensions_to_preferred_ratio(width, height, preferred_ratio);
    if let Some((width, height)) = dimensions
        && width.is_finite()
        && height.is_finite()
        && width > 0.0
        && height > 0.0
    {
        target_width = width;
        target_height = height;
    }

    DiagramDimensions {
        width: target_width,
        height: target_height,
        viewbox_x,
        viewbox_y,
        viewbox_width,
        viewbox_height,
    }
}
