//! ER attribute label data parsing from the pinned source renderer.

use super::text_helpers::is_divider_line;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ErAttribute {
    pub name: String,
    pub data_type: String,
    pub keys: Vec<String>,
}

pub fn parse_er_attributes(lines: &[String]) -> (String, Vec<ErAttribute>) {
    let mut title = lines
        .first()
        .map(|s| s.trim().to_string())
        .unwrap_or_default();
    let mut attrs = Vec::new();
    let mut in_body = false;
    for line in lines.iter().skip(1) {
        if is_divider_line(line) {
            in_body = true;
            continue;
        }
        if !in_body {
            if !line.trim().is_empty() {
                title = line.trim().to_string();
            }
            continue;
        }
        let trimmed = line.trim();
        if trimmed.is_empty() {
            continue;
        }
        let mut keys = Vec::new();
        let mut parts: Vec<String> = Vec::new();
        for token in trimmed.split_whitespace() {
            let cleaned = token
                .trim_matches(|ch: char| ch == ',' || ch == ';')
                .to_ascii_uppercase();
            if cleaned == "PK" || cleaned == "FK" || cleaned == "UK" {
                keys.push(cleaned);
                continue;
            }
            if cleaned.contains(',') {
                let mut handled = false;
                for piece in cleaned.split(',') {
                    if piece == "PK" || piece == "FK" || piece == "UK" {
                        keys.push(piece.to_string());
                        handled = true;
                    }
                }
                if handled {
                    continue;
                }
            }
            parts.push(token.to_string());
        }
        if parts.is_empty() {
            continue;
        }
        let (data_type, name) = if parts.len() >= 2 {
            (parts[0].clone(), parts[1..].join(" "))
        } else {
            (String::new(), parts[0].clone())
        };
        attrs.push(ErAttribute {
            name,
            data_type,
            keys,
        });
    }
    (title, attrs)
}
