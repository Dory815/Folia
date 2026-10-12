// Root: independently check a move, including unchanged surrounding rendering.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),file='third_party/mermaid-rs-renderer/src/render.rs';
const norm=s=>s.replaceAll('\r\n','\n').trimEnd()+'\n';
const before=norm(cp.execFileSync('git',['show','c3cdf17:'+file],{cwd:root,encoding:'utf8'}));
assert.equal(crypto.createHash('sha256').update(before).digest('hex'),'026efceab06b86752789ea2b5c37ecac569b8cd020f97c776f444bec0a2166b4');
const start=before.indexOf('#[derive(Debug, Clone, Copy, PartialEq, Serialize)]'),end=before.indexOf('fn edge_dom_id(',start),types=before.slice(start,end);
const cs=before.indexOf('const SEQUENCE_VIEWBOX_PAD_LEFT:'),ce=before.indexOf('pub fn render_svg(',cs),constants=before.slice(cs,ce);
const ms=before.indexOf('pub fn measure_svg_dimensions('),me=before.indexOf('pub fn render_svg_with_dimensions(',ms),measurement=before.slice(ms,me);
const shared='use crate::config::LayoutConfig;\nuse crate::layout::{DiagramData, Layout};\nuse serde::Serialize;\n\n'+types.replaceAll('SvgDimensions','DiagramDimensions')+constants.replaceAll('const SEQUENCE_','pub(crate) const SEQUENCE_')+measurement.replaceAll('measure_svg_dimensions','measure_layout_dimensions').replaceAll('SvgDimensions','DiagramDimensions');
assert.equal(norm(fs.readFileSync(path.join(root,'third_party/mermaid-rs-renderer/src/scene/compile/dimensions.rs'),'utf8')),norm(shared));
const wrapper=measurement.slice(0,measurement.indexOf('{')+1)+'\n    crate::scene::compile::dimensions::measure_layout_dimensions(layout, config, dimensions)\n}\n\n';
const expected=before.replace('use serde::Serialize;\n','').replace(types,'pub use crate::scene::compile::dimensions::DiagramDimensions as SvgDimensions;\n\n').replace(constants,'').replace(measurement,wrapper);
assert.equal(norm(fs.readFileSync(path.join(root,file),'utf8')),norm(expected));
console.log('Original type, methods, ratio helper, constants and measurement moved unchanged; all other render source unchanged.');
