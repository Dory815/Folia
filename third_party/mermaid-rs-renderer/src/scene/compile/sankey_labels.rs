//! Sankey value label formatting from the pinned source renderer.

pub fn format_sankey_value(value: f32) -> String {
    let rounded_2 = (value * 100.0).round() / 100.0;
    if (rounded_2 - rounded_2.round()).abs() < 0.001 {
        return format!("{rounded_2:.0}");
    }
    let rounded_1 = (value * 10.0).round() / 10.0;
    if (rounded_1 - rounded_2).abs() < 0.001 {
        format!("{rounded_1:.1}")
    } else {
        format!("{rounded_2:.2}")
    }
}
