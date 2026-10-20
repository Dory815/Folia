'use strict';
// Generate the original AMS maps with direct compiled callbacks and owned values.
const fs = require('node:fs'), path = require('node:path');
const crypto = require('node:crypto'), assert = require('node:assert/strict');
const root = path.resolve(__dirname, '../..');
const args = new Map();
for (let i = 2; i < process.argv.length; ++i) {
  const name = process.argv[i];
  if (name === '--check') { args.set(name, true); continue; }
  assert.ok(name === '--output' && process.argv[i + 1], 'Expected --check or --output PATH');
  args.set(name, process.argv[++i]);
}
const output = path.resolve(args.get('--output') || path.join(root, 'build/math-port/ams-generated/mappings.cpp'));
const digest = file => crypto.createHash('sha256').update(fs.readFileSync(path.join(root, file))).digest('hex');
assert.equal(digest('.reference/MathJax-src/ts/input/tex/ams/AmsMappings.ts'), '1ff357a98ebf72b9fd0a4cc06401cc4b469dce6e76fec3dec4bbd7362f04f06e');
assert.equal(digest('tools/mathjax-bundle/node_modules/@mathjax/src/cjs/input/tex/ams/AmsMappings.js'), '83337b1aafc4e7431050696abc27a3392c26e5626d70d22595d0b250bc5f2af9');
const load = file => require(path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs', file));
load('input/tex/ams/AmsConfiguration.js');
const {MapHandler} = load('input/tex/MapHandler.js');
const {AmsMethods} = load('input/tex/ams/AmsMethods.js');
const {default: ParseMethods} = load('input/tex/ParseMethods.js');
const names = ['AMSmath-mathchar0mo', 'AMSmath-operatorLetter', 'AMSmath-macros',
  'AMSmath-environment', 'AMSmath-delimiter', 'AMSsymbols-mathchar0mi',
  'AMSsymbols-mathchar0mo', 'AMSsymbols-delimiter', 'AMSsymbols-macros'];
function method(fn, methods) {
  const name = Object.keys(methods).find(name => methods[name] === fn);
  assert.ok(name, 'Every map callback belongs to its original methods object');
  return name;
}
// Capture pristine registration values before constructing a profile or parsing.
const mappings = names.map(name => {
  const map = MapHandler.getMap(name);
  assert.ok(map, name);
  const type = map.constructor.name;
  const parser = map.parser ? method(map.parser, type === 'RegExpMap' ? AmsMethods : ParseMethods) : null;
  if (type === 'RegExpMap')
    return {name, type, parser, pattern: map._regExp.source, flags: map._regExp.flags};
  const entries = [...map.map].map(([key, entry]) => {
    assert.equal(entry.token, key, 'Pristine map tokens retain their registration keys');
    if (type === 'CommandMap' || type === 'EnvironmentMap')
      return [key, {token: entry.token, method: method(entry.func, AmsMethods), args: entry.args}];
    return [key, {token: entry.token, text: entry.char, attributes: entry.attributes}];
  });
  return JSON.parse(JSON.stringify({name, type, parser, entries}));
});
const reference = require('./reference-profile.cjs').createReference();
assert.equal(reference.provenance.compiled_source_sha256, 'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const snapshotHash = crypto.createHash('sha256').update(JSON.stringify(mappings)).digest('hex');
assert.equal(snapshotHash, 'ffad7e5d055ba41cec9827a7327c7dc886781aedad4172c94d73bbef07fe5aaf');
function quote(value) {
  assert.equal(typeof value, 'string');
  let out = 'u"';
  for (const c of value) {
    const code = c.codePointAt(0);
    if (c === '\\') out += '\\\\';
    else if (c === '"') out += '\\"';
    else if (c === '\n') out += '\\n';
    else if (c === '\r') out += '\\r';
    else if (c === '\t') out += '\\t';
    else if (code < 32 || code === 127) out += '\\u' + code.toString(16).padStart(4, '0');
    else if (code > 65535) out += '\\U' + code.toString(16).padStart(8, '0');
    else out += c;
  }
  return out + '"';
}
function argument(value) {
  if (value === null) return 'NullArgument{}';
  if (typeof value === 'string') return 'std::u16string(' + quote(value) + ')';
  if (typeof value === 'boolean') return String(value);
  if (typeof value === 'number' && Number.isFinite(value))
    return Number.isInteger(value) ? value + '.0' : String(value);
  throw Error('Unexpected mathematical argument ' + JSON.stringify(value));
}
function token(key, value) {
  const text = value.text === null ? 'std::nullopt' : 'std::u16string(' + quote(value.text) + ')';
  const attrs = value.attributes === null ? '' : ', attributes({' +
    Object.entries(value.attributes).map(([key, value]) => '{' + quote(key) + ',' +
      (value === null ? 'mml::AttributeNull{}' : argument(value)) + '}').join(',') + '})';
  return 'Token(' + quote(key) + ',' + text + attrs + ')';
}
function emitMap(map) {
  const name = quote(map.name);
  if (map.type === 'RegExpMap')
    return `    registry.registerMap(std::make_shared<RegExpMap>(RegExpMapInit{{${name},TokenMapParser{&AmsMethods::${map.parser}}},std::make_shared<regexp_host::RegExp>(${quote(map.pattern)},${quote(map.flags)})}));`;
  let init;
  if (map.type === 'CommandMap' || map.type === 'EnvironmentMap') {
    const entries = map.entries.map(([key, value]) => '        Macro(' + quote(key) +
      ',MacroMethod{AmsMethods::' + value.method + '},{' + value.args.map(argument).join(',') + '})').join(',\n');
    init = `MacroMapInit{{${name},std::nullopt},{\n${entries}\n    }}`;
    init = map.type === 'CommandMap' ? `CommandMapInit{${init}}` : `EnvironmentMapInit{${init},&ParseMethods::environment}`;
  } else {
    const entries = map.entries.map(([key, value]) => '        ' + token(key, value)).join(',\n');
    init = `CharacterMapInit{{${name},TokenMapParser{&ParseMethods::${map.parser}}},{\n${entries}\n    }}`;
  }
  return `    registry.registerMap(std::make_shared<${map.type}>(${init}));`;
}
const source = '// MathJax 4.1.3 input/tex/ams/AmsMappings.ts; Apache-2.0.\n' +
  '// Generated by tools/math-port/generate-ams-mapping-data.cjs; immutable table data.\n' +
  '// Pristine registration SHA256 ' + snapshotHash + '.\n' +
  'module folia.math.input.tex.ams.mappings;\nimport folia.math.input.tex.parse_methods;\nimport std;\n\nnamespace folia::math::tex {\nnamespace {\nmml::AttributeSet attributes(std::initializer_list<mml::AttributeSet::Entry> entries) {\n    mml::AttributeSet result;\n    for (auto const &[key,value] : entries) result.set(key,value);\n    return result;\n}\n}\nvoid AmsMappings::initialize(MapHandler &registry) {\n' +
  mappings.map(emitMap).join('\n') + '\n}\n}\n';
if (args.has('--check')) assert.equal(fs.readFileSync(output, 'utf8').replaceAll('\r\n', '\n'), source);
else { fs.mkdirSync(path.dirname(output), {recursive: true}); fs.writeFileSync(output, source); }
console.log(JSON.stringify({maps: mappings.length, entries: mappings.reduce((count, map) => count + (map.entries?.length || 0), 0),
  snapshot_sha256: snapshotHash, output, checked: args.has('--check')}));
