//! Exact C4 style-field selection from the pinned renderer.

use crate::config::C4Config;
use crate::ir::C4ShapeKind;

pub fn c4_shape_colors(conf: &C4Config, kind: C4ShapeKind) -> (&str, &str) {
    match kind {
        C4ShapeKind::Person => (&conf.person_bg_color, &conf.person_border_color),
        C4ShapeKind::ExternalPerson => (
            &conf.external_person_bg_color,
            &conf.external_person_border_color,
        ),
        C4ShapeKind::System => (&conf.system_bg_color, &conf.system_border_color),
        C4ShapeKind::SystemDb => {
            (&conf.system_db_bg_color, &conf.system_db_border_color)
        }
        C4ShapeKind::SystemQueue => {
            (&conf.system_queue_bg_color, &conf.system_queue_border_color)
        }
        C4ShapeKind::ExternalSystem => (
            &conf.external_system_bg_color,
            &conf.external_system_border_color,
        ),
        C4ShapeKind::ExternalSystemDb => (
            &conf.external_system_db_bg_color,
            &conf.external_system_db_border_color,
        ),
        C4ShapeKind::ExternalSystemQueue => (
            &conf.external_system_queue_bg_color,
            &conf.external_system_queue_border_color,
        ),
        C4ShapeKind::Container => {
            (&conf.container_bg_color, &conf.container_border_color)
        }
        C4ShapeKind::ContainerDb => {
            (&conf.container_db_bg_color, &conf.container_db_border_color)
        }
        C4ShapeKind::ContainerQueue => (
            &conf.container_queue_bg_color,
            &conf.container_queue_border_color,
        ),
        C4ShapeKind::ExternalContainer => (
            &conf.external_container_bg_color,
            &conf.external_container_border_color,
        ),
        C4ShapeKind::ExternalContainerDb => (
            &conf.external_container_db_bg_color,
            &conf.external_container_db_border_color,
        ),
        C4ShapeKind::ExternalContainerQueue => (
            &conf.external_container_queue_bg_color,
            &conf.external_container_queue_border_color,
        ),
        C4ShapeKind::Component => {
            (&conf.component_bg_color, &conf.component_border_color)
        }
        C4ShapeKind::ComponentDb => {
            (&conf.component_db_bg_color, &conf.component_db_border_color)
        }
        C4ShapeKind::ComponentQueue => (
            &conf.component_queue_bg_color,
            &conf.component_queue_border_color,
        ),
        C4ShapeKind::ExternalComponent => (
            &conf.external_component_bg_color,
            &conf.external_component_border_color,
        ),
        C4ShapeKind::ExternalComponentDb => (
            &conf.external_component_db_bg_color,
            &conf.external_component_db_border_color,
        ),
        C4ShapeKind::ExternalComponentQueue => (
            &conf.external_component_queue_bg_color,
            &conf.external_component_queue_border_color,
        ),
    }
}

pub fn c4_shape_font_family(conf: &C4Config, kind: C4ShapeKind) -> &str {
    match kind {
        C4ShapeKind::Person => &conf.person_font_family,
        C4ShapeKind::ExternalPerson => &conf.external_person_font_family,
        C4ShapeKind::System => &conf.system_font_family,
        C4ShapeKind::SystemDb => &conf.system_db_font_family,
        C4ShapeKind::SystemQueue => &conf.system_queue_font_family,
        C4ShapeKind::ExternalSystem => &conf.external_system_font_family,
        C4ShapeKind::ExternalSystemDb => &conf.external_system_db_font_family,
        C4ShapeKind::ExternalSystemQueue => &conf.external_system_queue_font_family,
        C4ShapeKind::Container => &conf.container_font_family,
        C4ShapeKind::ContainerDb => &conf.container_db_font_family,
        C4ShapeKind::ContainerQueue => &conf.container_queue_font_family,
        C4ShapeKind::ExternalContainer => &conf.external_container_font_family,
        C4ShapeKind::ExternalContainerDb => &conf.external_container_db_font_family,
        C4ShapeKind::ExternalContainerQueue => {
            &conf.external_container_queue_font_family
        }
        C4ShapeKind::Component => &conf.component_font_family,
        C4ShapeKind::ComponentDb => &conf.component_db_font_family,
        C4ShapeKind::ComponentQueue => &conf.component_queue_font_family,
        C4ShapeKind::ExternalComponent => &conf.external_component_font_family,
        C4ShapeKind::ExternalComponentDb => &conf.external_component_db_font_family,
        C4ShapeKind::ExternalComponentQueue => {
            &conf.external_component_queue_font_family
        }
    }
}

pub fn c4_shape_font_size(conf: &C4Config, kind: C4ShapeKind) -> f32 {
    match kind {
        C4ShapeKind::Person => conf.person_font_size,
        C4ShapeKind::ExternalPerson => conf.external_person_font_size,
        C4ShapeKind::System => conf.system_font_size,
        C4ShapeKind::SystemDb => conf.system_db_font_size,
        C4ShapeKind::SystemQueue => conf.system_queue_font_size,
        C4ShapeKind::ExternalSystem => conf.external_system_font_size,
        C4ShapeKind::ExternalSystemDb => conf.external_system_db_font_size,
        C4ShapeKind::ExternalSystemQueue => conf.external_system_queue_font_size,
        C4ShapeKind::Container => conf.container_font_size,
        C4ShapeKind::ContainerDb => conf.container_db_font_size,
        C4ShapeKind::ContainerQueue => conf.container_queue_font_size,
        C4ShapeKind::ExternalContainer => conf.external_container_font_size,
        C4ShapeKind::ExternalContainerDb => conf.external_container_db_font_size,
        C4ShapeKind::ExternalContainerQueue => conf.external_container_queue_font_size,
        C4ShapeKind::Component => conf.component_font_size,
        C4ShapeKind::ComponentDb => conf.component_db_font_size,
        C4ShapeKind::ComponentQueue => conf.component_queue_font_size,
        C4ShapeKind::ExternalComponent => conf.external_component_font_size,
        C4ShapeKind::ExternalComponentDb => conf.external_component_db_font_size,
        C4ShapeKind::ExternalComponentQueue => conf.external_component_queue_font_size,
    }
}

pub fn c4_shape_font_weight(conf: &C4Config, kind: C4ShapeKind) -> &str {
    match kind {
        C4ShapeKind::Person => &conf.person_font_weight,
        C4ShapeKind::ExternalPerson => &conf.external_person_font_weight,
        C4ShapeKind::System => &conf.system_font_weight,
        C4ShapeKind::SystemDb => &conf.system_db_font_weight,
        C4ShapeKind::SystemQueue => &conf.system_queue_font_weight,
        C4ShapeKind::ExternalSystem => &conf.external_system_font_weight,
        C4ShapeKind::ExternalSystemDb => &conf.external_system_db_font_weight,
        C4ShapeKind::ExternalSystemQueue => &conf.external_system_queue_font_weight,
        C4ShapeKind::Container => &conf.container_font_weight,
        C4ShapeKind::ContainerDb => &conf.container_db_font_weight,
        C4ShapeKind::ContainerQueue => &conf.container_queue_font_weight,
        C4ShapeKind::ExternalContainer => &conf.external_container_font_weight,
        C4ShapeKind::ExternalContainerDb => &conf.external_container_db_font_weight,
        C4ShapeKind::ExternalContainerQueue => {
            &conf.external_container_queue_font_weight
        }
        C4ShapeKind::Component => &conf.component_font_weight,
        C4ShapeKind::ComponentDb => &conf.component_db_font_weight,
        C4ShapeKind::ComponentQueue => &conf.component_queue_font_weight,
        C4ShapeKind::ExternalComponent => &conf.external_component_font_weight,
        C4ShapeKind::ExternalComponentDb => &conf.external_component_db_font_weight,
        C4ShapeKind::ExternalComponentQueue => {
            &conf.external_component_queue_font_weight
        }
    }
}
