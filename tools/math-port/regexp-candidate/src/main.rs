// Root development experiment, not a production regexp host or accepted API.
use regress::{Flags, Regex};
use serde::Deserialize;
use serde_json::{Value, json};
use std::collections::{HashMap, HashSet};
use std::io::{self, Read};
use std::panic::{AssertUnwindSafe, catch_unwind};

#[derive(Deserialize)]
struct Input {
    pattern: Vec<u16>,
    flags: String,
    text: Vec<u16>,
    start: usize,
}
type Cache = HashMap<(Vec<u16>, String, bool), Result<Regex, String>>;

fn checked_flags(text: &str, normalize_v: bool) -> Result<Flags, ()> {
    let mut seen = HashSet::new();
    for c in text.chars() {
        if !"dgimsuvy".contains(c) || !seen.insert(c) {
            return Err(());
        }
    }
    if seen.contains(&'u') && seen.contains(&'v') {
        return Err(());
    }
    let mut flags = Flags::from(text);
    if normalize_v && flags.unicode_sets {
        flags.unicode = true;
    }
    Ok(flags)
}

fn run(input: &Input, normalize_v: bool, cache: &mut Cache) -> Value {
    let flags = match checked_flags(&input.flags, normalize_v) {
        Ok(flags) => flags,
        Err(()) => return json!({"constructor":"SyntaxError"}),
    };
    let key = (input.pattern.clone(), input.flags.clone(), normalize_v);
    let compiled = cache.entry(key).or_insert_with(|| {
        // Legacy patterns are UTF-16 code units. Unicode patterns decode pairs
        // while retaining lone surrogates as code points.
        let pattern: Vec<u32> = if flags.unicode {
            char::decode_utf16(input.pattern.iter().copied())
                .map(|c| c.map(u32::from).unwrap_or_else(|e| u32::from(e.unpaired_surrogate())))
                .collect()
        } else {
            input.pattern.iter().copied().map(u32::from).collect()
        };
        Regex::from_unicode(pattern.into_iter(), flags).map_err(|e| e.to_string())
    });
    let regex = match compiled {
        Ok(regex) => regex,
        Err(message) => return json!({"constructor":"SyntaxError","diagnostic":message}),
    };
    let stateful = input.flags.contains('g') || input.flags.contains('y');
    let sticky = input.flags.contains('y');
    let mut start = if stateful { input.start } else { 0 };
    // This adapter stage is explicit experiment code, not library behavior.
    // Experimental alignment for low-half starts; not a proved general JS
    // protocol. Zero-width assertions inside pairs remain in the differences.
    if flags.unicode && start > 0 && start < input.text.len()
        && (0xdc00..=0xdfff).contains(&input.text[start])
        && (0xd800..=0xdbff).contains(&input.text[start - 1])
    {
        start -= 1;
    }
    let matched = if start > input.text.len() {
        None
    } else {
        let found = if flags.unicode {
            regex.find_from_utf16(&input.text, start).next()
        } else {
            regex.find_from_ucs2(&input.text, start).next()
        };
        found.filter(|m| !sticky || m.range.start == start)
    };
    let last_index = if stateful { matched.as_ref().map_or(0, |m| m.range.end) } else { input.start };
    let result = matched.map(|m| {
        let ranges: Vec<Option<[usize; 2]>> = m.groups()
            .map(|r| r.map(|r| [r.start, r.end])).collect();
        let named: Vec<Value> = m.named_groups()
            .map(|(name,r)| json!({"name":name,"range":r.map(|r|[r.start,r.end])})).collect();
        json!({"ranges":ranges,"named":named})
    });
    json!({"constructor":"ok","match":result,"last_index":last_index})
}

fn main() {
    let mut source = String::new();
    io::stdin().read_to_string(&mut source).unwrap();
    let inputs: Vec<Input> = serde_json::from_str(&source).unwrap();
    let mut cache = Cache::new();
    let outputs: Vec<Value> = inputs.iter().map(|input| {
        let mut modes = Vec::new();
        for normalize_v in [false, true] {
            let value = catch_unwind(AssertUnwindSafe(|| run(input,normalize_v,&mut cache)))
                .unwrap_or_else(|_| json!({"constructor":"native-panic"}));
            modes.push(value);
        }
        json!({"literal_flags":modes[0],"v_implies_unicode":modes[1]})
    }).collect();
    println!("{}", serde_json::to_string(&outputs).unwrap());
}
