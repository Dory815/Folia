'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');

function parseArgs(argv) {
  let repoRoot = process.cwd();
  let dumpArgument;
  let expectedArgument;
  for (let i = 0; i < argv.length; ++i) {
    const arg = argv[i];
    if (arg === '--repo-root') {
      if (!argv[i + 1]) throw new Error('--repo-root requires a directory');
      repoRoot = path.resolve(argv[++i]);
    } else if (arg === '--dump') {
      if (!argv[i + 1]) throw new Error('--dump requires a TSV path');
      dumpArgument = argv[++i];
    } else if (arg === '--expected') {
      if (!argv[i + 1]) throw new Error('--expected requires a JSON path');
      expectedArgument = argv[++i];
    } else if (arg.startsWith('-')) {
      throw new Error(`unknown option ${arg}`);
    } else if (!dumpArgument) {
      dumpArgument = arg; // Keep the old one-positional-path form usable.
    } else {
      throw new Error(`unexpected positional argument ${arg}`);
    }
  }
  if (!dumpArgument) {
    throw new Error('usage: node compare-svg-dump.cjs --repo-root <repo> --dump <native-svg-dump.tsv> [--expected <rows.json>]');
  }
  const resolveFromRoot = value => path.isAbsolute(value) ? path.normalize(value) : path.resolve(repoRoot, value);
  const dumpPath = resolveFromRoot(dumpArgument);
  const expectedPath = expectedArgument
    ? resolveFromRoot(expectedArgument)
    : path.join(repoRoot, 'tests/math/fixtures/common-source/menclose-geometry-source-expected.json');
  return { dumpPath, expectedPath };
}

function parseAttributes(source, allowedNames, context) {
  const result = Object.create(null);
  let offset = 0;
  while (offset < source.length) {
    if (/^\s*$/.test(source.slice(offset))) break;
    const match = /\s+([A-Za-z_:][A-Za-z0-9_.:-]*)="([^"<>]*)"/y;
    match.lastIndex = offset;
    const found = match.exec(source);
    if (!found) throw new Error(`${context}: unsupported or malformed attribute near ${source.slice(offset)}`);
    const [, name, value] = found;
    if (!allowedNames.has(name)) throw new Error(`${context}: unsupported attribute ${name}`);
    if (Object.hasOwn(result, name)) throw new Error(`${context}: duplicate attribute ${name}`);
    result[name] = value;
    offset = match.lastIndex;
  }
  return result;
}

// Geometry stays in exact base-ten rationals throughout. In particular, a
// relative Source segment such as -30.1 followed by +67 is exactly 36.9;
// doing that addition in binary doubles creates a spurious mismatch.
const gcd = (a, b) => { a = a < 0n ? -a : a; b = b < 0n ? -b : b;
  while (b) [a, b] = [b, a % b]; return a || 1n; };
function rat(n, d = 1n) { if (d < 0n) { n = -n; d = -d; } const g = gcd(n, d); return { n: n/g, d: d/g }; }
function decimal(token, context = 'number') {
  if (!/^[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?$/.test(token))
    throw new Error(`${context}: non-finite or invalid number ${token}`);
  const [mantissa, exponentText] = token.toLowerCase().split('e');
  const exponent = exponentText === undefined ? 0 : Number(exponentText);
  if (!Number.isSafeInteger(exponent) || Math.abs(exponent) > 10000) throw new Error(`${context}: invalid exponent ${token}`);
  const negative = mantissa.startsWith('-');
  const unsigned = mantissa.replace(/^[+-]/, '');
  const [whole, fraction = ''] = unsigned.split('.');
  let n = BigInt((whole || '0') + fraction); if (negative) n = -n;
  const scale = fraction.length - exponent;
  return scale >= 0 ? rat(n, 10n ** BigInt(scale)) : rat(n * 10n ** BigInt(-scale));
}
const add = (a,b) => rat(a.n*b.d+b.n*a.d,a.d*b.d);
const sub = (a,b) => rat(a.n*b.d-b.n*a.d,a.d*b.d);
const mul = (a,b) => rat(a.n*b.n,a.d*b.d);
const div = (a,b) => { if (!b.n) throw new Error('division by zero'); return rat(a.n*b.d,a.d*b.n); };
const cmp = (a,b) => { const d=a.n*b.d-b.n*a.d; return d<0n?-1:d>0n?1:0; };
const key = a => `${a.n}/${a.d}`;
const ZERO = rat(0n), ONE = rat(1n), THOUSAND = rat(1000n);
function finiteNumber(token, context) {
  if (!Number.isFinite(Number(token))) throw new Error(`${context}: outside finite SVG number range ${token}`);
  return decimal(token, context);
}

function tokenizePath(data) {
  const tokens = [];
  let offset = 0, previous = 'start', separated = false;
  while (offset < data.length) {
    if (/\s/.test(data[offset])) { ++offset; separated = true; continue; }
    if (data[offset] === ',') {
      if (previous !== 'number') throw new Error(`unexpected comma at ${offset} in path data: ${data}`);
      ++offset;
      while (offset < data.length && /\s/.test(data[offset])) ++offset;
      if (offset === data.length || /[A-Za-z,]/.test(data[offset]))
        throw new Error(`comma must separate numeric parameters in path data: ${data}`);
      previous = 'comma';
      separated = true;
      continue;
    }
    const token = /[A-Za-z]|[-+]?(?:\d*\.\d+|\d+\.?\d*)(?:[eE][-+]?\d+)?/y;
    token.lastIndex = offset;
    const match = token.exec(data);
    if (!match) throw new Error(`path data contains an invalid character at ${offset}: ${data.slice(offset, offset + 16)}`);
    const value = match[0];
    const isCommand = /^[A-Za-z]$/.test(value);
    if (isCommand && previous === 'comma')
      throw new Error(`comma cannot precede a path command in path data: ${data}`);
    if (!isCommand && previous === 'number' && !separated)
      throw new Error(`numeric parameters require a separator in path data: ${data}`);
    tokens.push(value);
    previous = isCommand ? 'command' : 'number';
    separated = false;
    offset = token.lastIndex;
  }
  if (previous === 'comma') throw new Error(`path data cannot end with a comma: ${data}`);
  return tokens;
}

function parsePath(data) {
  // Share exact relative/absolute normalization with the pinned Source path
  // observer. The native side still gates every number to finite SVG doubles.
  for (const token of tokenizePath(data))
    if (!/^[A-Za-z]$/.test(token)) finiteNumber(token, 'native path');
  const result = sourcePath(data);
  const rational = value => {
    const [numerator, denominator] = value.split('/').map(BigInt);
    return rat(numerator, denominator);
  };
  for (const segment of result) {
    if (segment[0] === 'A' && (cmp(rational(segment[1]), ZERO) < 0 || cmp(rational(segment[2]), ZERO) < 0))
      throw new Error('A arc radii must be nonnegative');
  }
  return result;
}

function transformOps(value) {
  if (value === undefined) return [];
  const ops = [];
  const pattern = /(translate|rotate)\(([^)]*)\)/g;
  for (const match of value.matchAll(pattern)) {
    const numsText = match[2].trim();
    const raw = numsText.split(/[ ,]+/);
    const nums = raw.map((token, i) => finiteNumber(token, `${match[1]} parameter ${i}`));
    if (match[1] === 'translate') {
      if (nums.length !== 2) throw new Error(`translate requires exactly two parameters: ${value}`);
      ops.push(['T', key(div(nums[0], THOUSAND)), key(div(nums[1], THOUSAND))]);
    } else {
      if (nums.length !== 3) throw new Error(`rotate requires exactly three parameters: ${value}`);
      ops.push(['R', key(nums[0]), key(div(nums[1], THOUSAND)), key(div(nums[2], THOUSAND))]);
    }
  }
  if (value.replace(pattern, '').trim()) throw new Error(`unsupported native transform: ${value}`);
  return ops;
}

function sourcePath(data) {
  const tokens = tokenizePath(data), out=[];
  let i=0, command='', x=ZERO, y=ZERO, first=true;
  while(i<tokens.length) {
    if (/^[A-Za-z]$/.test(tokens[i])) command=tokens[i++];
    else if (!command) throw new Error(`Source path starts without command: ${data}`);
    if (!'MmLlAaZz'.includes(command)) throw new Error(`unsupported Source path command ${command}`);
    if(first && command.toUpperCase()!=='M') throw new Error('Source path must start with M'); first=false;
    if(command.toUpperCase()==='Z') { out.push(['Z']); command=''; continue; }
    const relative=command===command.toLowerCase(), upper=command.toUpperCase();
    if(upper==='M'||upper==='L') {
      const start=i;
      while(i<tokens.length&&!/^[A-Za-z]$/.test(tokens[i])) {
        if(i+1>=tokens.length||/^[A-Za-z]$/.test(tokens[i+1])) throw new Error(`${upper} requires pairs`);
        let nx=decimal(tokens[i++]), ny=decimal(tokens[i++]);
        if(relative) { nx=add(x,nx); ny=add(y,ny); }
        x=nx; y=ny; out.push([upper==='M'&&i-start===2?'M':'L',key(div(x,THOUSAND)),key(div(y,THOUSAND))]);
        if(upper==='M') command=relative?'l':'L';
      }
      if(i===start) throw new Error(`${upper} requires coordinates`);
    } else {
      const start=i;
      while(i<tokens.length&&!/^[A-Za-z]$/.test(tokens[i])) {
        if(i+7>tokens.length||tokens.slice(i,i+7).some(t=>/^[A-Za-z]$/.test(t))) throw new Error('A requires seven parameters');
        const rx=decimal(tokens[i++]), ry=decimal(tokens[i++]), rot=decimal(tokens[i++]);
        const large=tokens[i++], sweep=tokens[i++];
        if(!/^[01]$/.test(large)||!/^[01]$/.test(sweep)) throw new Error('invalid Source arc flag');
        let nx=decimal(tokens[i++]), ny=decimal(tokens[i++]); if(relative){nx=add(x,nx);ny=add(y,ny);} x=nx;y=ny;
        out.push(['A',key(div(rx,THOUSAND)),key(div(ry,THOUSAND)),key(rot),large==='1',sweep==='1',key(div(x,THOUSAND)),key(div(y,THOUSAND))]);
      }
      if(i===start) throw new Error('A requires parameters');
    }
  }
  return out;
}
function sourceGeometry(elements) {
  return elements.map(({tag,attrs})=>{
    const allowed={line:['x1','y1','x2','y2','stroke-width'],
      rect:['x','y','width','height','rx','fill','stroke-width'],
      ellipse:['rx','ry','cx','cy','fill','stroke-width'],
      path:['d','style','stroke-linecap','stroke-linejoin','fill','transform']}[tag];
    if(!allowed) throw new Error(`unsupported Source shape ${tag}`);
    for(const name of Object.keys(attrs)) if(!allowed.includes(name))
      throw new Error(`Source ${tag} has unmodeled attribute ${name}`);
    if(attrs.style) for(const name of Object.keys(attrs.style))
      if(name!=='stroke-width') throw new Error(`Source style has unmodeled property ${name}`);
    let segments, fill=attrs.fill ?? 'inherited', width=ZERO;
    const val=(name, fallback) => attrs[name]===undefined ? fallback : decimal(String(attrs[name]),`Source ${name}`);
    if(tag==='line') {
      segments=[['M',key(div(val('x1',ZERO),THOUSAND)),key(div(val('y1',ZERO),THOUSAND))],['L',key(div(val('x2',ZERO),THOUSAND)),key(div(val('y2',ZERO),THOUSAND))]];
      width=div(val('stroke-width',ZERO),THOUSAND);
    } else if(tag==='rect') {
      const x=val('x',ZERO),y=val('y',ZERO),w=val('width',ZERO),h=val('height',ZERO);
      const x1=add(x,w),y1=add(y,h),rx=val('rx',ZERO),r=cmp(rx,ZERO)>0?[rx,div(w,rat(2n)),div(h,rat(2n))].reduce((a,b)=>cmp(a,b)<0?a:b):ZERO;
      const X=v=>key(div(v,THOUSAND)); width=div(val('stroke-width',ZERO),THOUSAND);
      if(!r.n) segments=[['M',X(x),X(y)],['L',X(x1),X(y)],['L',X(x1),X(y1)],['L',X(x),X(y1)],['Z']];
      else segments=[['M',X(add(x,r)),X(y)],['L',X(sub(x1,r)),X(y)],['A',X(r),X(r),key(ZERO),false,true,X(x1),X(add(y,r))],['L',X(x1),X(sub(y1,r))],['A',X(r),X(r),key(ZERO),false,true,X(sub(x1,r)),X(y1)],['L',X(add(x,r)),X(y1)],['A',X(r),X(r),key(ZERO),false,true,X(x),X(sub(y1,r))],['L',X(x),X(add(y,r))],['A',X(r),X(r),key(ZERO),false,true,X(add(x,r)),X(y)],['Z']];
    } else if(tag==='ellipse') {
      const rx=val('rx',ZERO),ry=val('ry',ZERO),cx=val('cx',ZERO),cy=val('cy',ZERO), X=v=>key(div(v,THOUSAND));
      width=div(val('stroke-width',ZERO),THOUSAND);
      segments=[['M',X(add(cx,rx)),X(cy)],['A',X(rx),X(ry),key(ZERO),false,true,X(sub(cx,rx)),X(cy)],['A',X(rx),X(ry),key(ZERO),false,true,X(add(cx,rx)),X(cy)],['Z']];
    } else if(tag==='path') {
      segments=sourcePath(attrs.d); if(attrs.style){width=div(decimal(String(attrs.style['stroke-width'])),THOUSAND);}
    } else throw new Error(`unsupported Source shape ${tag}`);
    const join=attrs['stroke-linejoin']??null;
    const paint={fill,stroke:'inherited',stroke_width:key(width),opacity:key(ONE),fill_rule:'nonzero',fill_opacity:null,stroke_opacity:null,line_cap:attrs['stroke-linecap']??null,line_join:join==='mitre'?'miter':join};
    return {segments,transform:transformOps(attrs.transform),paint};
  });
}

const SVG_PAINT_ATTRIBUTES = ['fill', 'stroke', 'opacity', 'fill-rule', 'fill-opacity', 'stroke-opacity',
  'stroke-linecap', 'stroke-linejoin', 'transform'];
const SVG_SHAPE_SCHEMA = {
  path: { geometry: ['d', 'style', 'stroke-width'], required: ['d'] },
  line: { geometry: ['x1', 'y1', 'x2', 'y2', 'stroke-width'], required: ['x1', 'y1', 'x2', 'y2'] },
  rect: { geometry: ['x', 'y', 'width', 'height', 'rx', 'stroke-width'], required: ['x', 'y', 'width', 'height'] },
  ellipse: { geometry: ['rx', 'ry', 'cx', 'cy', 'stroke-width'], required: ['rx', 'ry', 'cx', 'cy'] },
};

function nativeShape(tag, attrs, nestedTransforms) {
  const schema = SVG_SHAPE_SCHEMA[tag];
  if (!schema) throw new Error(`unsupported SVG element <${tag}>`);
  const { geometry } = schema;
  const allowed = new Set([...geometry, ...SVG_PAINT_ATTRIBUTES]);
  for (const name of Object.keys(attrs))
    if (!allowed.has(name)) throw new Error(`${tag}: unsupported attribute ${name}`);
  const values = new Map();
  for (const name of geometry) {
    if (name === 'd' || name === 'style' || !Object.hasOwn(attrs, name)) continue;
    values.set(name, finiteNumber(attrs[name], `${tag} ${name}`));
  }
  const nonnegative = names => {
    for (const name of names)
      if (values.has(name) && cmp(values.get(name), ZERO) < 0)
        throw new Error(`${tag} ${name} must be nonnegative`);
  };
  if (tag === 'rect') nonnegative(['width', 'height', 'rx']);
  if (tag === 'ellipse') nonnegative(['rx', 'ry']);
  let styleWidth, styleWidthText;
  if (attrs.style !== undefined) {
    const match = /^\s*stroke-width\s*:\s*([^;]+?)\s*;?\s*$/.exec(attrs.style);
    if (!match) throw new Error('path style only supports one stroke-width declaration');
    styleWidthText = match[1];
    styleWidth = finiteNumber(match[1], 'path style stroke-width');
  }

  const sourceAttrs = {};
  for (const name of geometry) {
    if (name === 'style' || name === 'stroke-width') continue;
    if (Object.hasOwn(attrs, name)) sourceAttrs[name] = attrs[name];
  }
  const width = styleWidth ?? values.get('stroke-width');
  if (tag === 'path' && width !== undefined)
    sourceAttrs.style = { 'stroke-width': styleWidthText ?? attrs['stroke-width'] };
  else if (tag !== 'path' && width !== undefined) sourceAttrs['stroke-width'] = attrs['stroke-width'];
  if (tag === 'path') {
    for (const name of ['stroke-linecap', 'stroke-linejoin'])
      if (Object.hasOwn(attrs, name)) sourceAttrs[name] = attrs[name];
    if (Object.hasOwn(attrs, 'fill')) sourceAttrs.fill = attrs.fill;
  }
  const normalized = sourceGeometry([{ tag, attrs: sourceAttrs }])[0];
  const cap = attrs['stroke-linecap'] ?? null;
  const join = attrs['stroke-linejoin'] ?? null;
  if (cap !== null && !['butt', 'round', 'square'].includes(cap)) throw new Error(`unsupported stroke-linecap ${cap}`);
  if (join !== null && !['miter', 'mitre', 'round', 'bevel'].includes(join)) throw new Error(`unsupported stroke-linejoin ${join}`);
  const numeric = (name, fallback) => Object.hasOwn(attrs, name)
    ? finiteNumber(attrs[name], `${tag} ${name}`) : fallback;
  return {
    segments: tag === 'path' ? parsePath(attrs.d) : normalized.segments,
    transform: nestedTransforms.concat(transformOps(attrs.transform)),
    paint: {
      fill: attrs.fill ?? 'inherited',
      stroke: attrs.stroke ?? 'inherited',
      stroke_width: normalized.paint.stroke_width,
      opacity: key(numeric('opacity', ONE)),
      fill_rule: attrs['fill-rule'] ?? 'nonzero',
      fill_opacity: Object.hasOwn(attrs, 'fill-opacity') ? key(numeric('fill-opacity', ONE)) : null,
      stroke_opacity: Object.hasOwn(attrs, 'stroke-opacity') ? key(numeric('stroke-opacity', ONE)) : null,
      line_cap: cap,
      line_join: join === 'mitre' ? 'miter' : join,
    },
  };
}

function parseSvg(svg) {
  const shapes = [];
  const stack = [];
  let coordinateGroupSeen = false, rootClosed = false, offset = 0;
  const tagPattern = /<(\/)?([A-Za-z][A-Za-z0-9:-]*)([^<>]*?)(\/?)>/y;
  while (offset < svg.length) {
    if (/\s/.test(svg[offset])) { ++offset; continue; }
    tagPattern.lastIndex = offset;
    const match = tagPattern.exec(svg);
    if (!match) throw new Error(`unparsed SVG content at ${offset}: ${svg.slice(offset, offset + 50)}`);
    const [, close, name, attrSource, selfClose] = match;
    offset = tagPattern.lastIndex;
    if (close) {
      if (selfClose) throw new Error(`closing ${name} tag cannot be self-closing`);
      const top = stack.pop();
      if (!top || top.name !== name) throw new Error(`unexpected closing SVG tag ${name}`);
      if (name === 'svg') {
        if (stack.length || !coordinateGroupSeen) throw new Error('SVG root closed at an invalid depth');
        rootClosed = true;
      }
      continue;
    }
    if (rootClosed) throw new Error('markup follows the SVG root');
    if (name === 'svg') {
      if (stack.length || selfClose) throw new Error('SVG must have one non-self-closing outer root');
      const attrs = parseAttributes(attrSource, new Set([
        'style', 'xmlns', 'width', 'height', 'role', 'focusable', 'viewBox', 'xmlns:xlink',
      ]), 'svg root');
      for (const required of ['style', 'xmlns', 'width', 'height', 'role', 'focusable', 'viewBox'])
        if (!Object.hasOwn(attrs, required)) throw new Error(`svg root is missing ${required}`);
      if (attrs.xmlns !== 'http://www.w3.org/2000/svg' || attrs.role !== 'img' || attrs.focusable !== 'false')
        throw new Error('SVG root namespace or accessibility attributes differ from the writer contract');
      if (Object.hasOwn(attrs, 'xmlns:xlink') && attrs['xmlns:xlink'] !== 'http://www.w3.org/1999/xlink')
        throw new Error('unsupported xlink namespace');
      stack.push({ name: 'svg', attrs });
      continue;
    }
    if (name === 'g') {
      if (selfClose || !stack.length || stack.at(-1).name !== 'g' && stack.at(-1).name !== 'svg')
        throw new Error('groups must be non-self-closing children of SVG/groups');
      if (stack.length === 1) {
        if (coordinateGroupSeen) throw new Error('more than one top-level coordinate group');
        const attrs = parseAttributes(attrSource,
          new Set(['stroke', 'fill', 'stroke-width', 'transform']), 'root coordinate group');
        assert.deepEqual({ ...attrs }, {
          stroke: 'currentColor', fill: 'currentColor', 'stroke-width': '0', transform: 'scale(1,-1)',
        }, 'root coordinate group must be the exact unique MathJax y-flip/currentColor group');
        coordinateGroupSeen = true;
        stack.push({ name: 'g', coordinateRoot: true, transforms: [] });
      } else {
        if (!coordinateGroupSeen || !stack.some(item => item.coordinateRoot))
          throw new Error('nested group is outside the coordinate group');
        const attrs = parseAttributes(attrSource, new Set(['data-mml-node', 'transform']), 'nested group');
        if (attrs['data-mml-node'] !== 'mrow') throw new Error('unexpected writer group node label');
        const transforms = transformOps(attrs.transform);
        stack.push({ name: 'g', coordinateRoot: false, transforms });
      }
      continue;
    }
    if (Object.hasOwn(SVG_SHAPE_SCHEMA, name)) {
      if (!selfClose || stack.length < 3 || stack.at(-1)?.name !== 'g' || !stack.some(item => item.coordinateRoot))
        throw new Error(`${name} elements must be self-closing descendants of the coordinate group`);
      const attrs = parseAttributes(attrSource,
        new Set([...SVG_SHAPE_SCHEMA[name].geometry, ...SVG_PAINT_ATTRIBUTES]), name);
      for (const field of SVG_SHAPE_SCHEMA[name].required)
        if (!Object.hasOwn(attrs, field)) throw new Error(`${name} is missing ${field}`);
      const nestedTransforms = stack.filter(item => item.name === 'g' && !item.coordinateRoot)
        .flatMap(item => item.transforms);
      shapes.push(nativeShape(name, attrs, nestedTransforms));
      continue;
    }
    throw new Error(`unsupported SVG element <${name}>`);
  }
  if (!rootClosed || stack.length) throw new Error('SVG root is unclosed');
  if (!shapes.length) throw new Error('SVG contains no geometry paths');
  return shapes;
}

function compare(dumpPath, expectedPath) {
const expected = JSON.parse(fs.readFileSync(expectedPath, 'utf8'));
const lines = fs.readFileSync(dumpPath, 'utf8').trimEnd().split(/\r?\n/);
const records = lines.map(line => {
  const first = line.indexOf('\t'), second = line.indexOf('\t', first + 1);
  assert(first > 0 && second > first, `malformed native dump row: ${line.slice(0, 120)}`);
  return { case: line.slice(0, first), notation: line.slice(first + 1, second), svg: line.slice(second + 1) };
});
assert.equal(records.length, 56, 'native probe must emit every notation for both boxes');
assert.equal(expected.rows.length, 56, 'locked Source fixture must contain every notation for both boxes');
for (let i = 0; i < records.length; ++i) {
  const actual = records[i];
  const source = expected.rows[i];
  assert.equal(actual.case, source.case);
  assert.equal(actual.notation, source.notation);
  assert.deepEqual(parseSvg(actual.svg), sourceGeometry(source.source_svg),
    `${actual.case}/${actual.notation}: writer geometry, transform, or paint differs from pinned SVG output`);
}
console.log(`exact SVG geometry/transform/paint match for ${records.length} native writer outputs against MathJax 4.1.3`);
}

if (require.main === module) {
  const { dumpPath, expectedPath } = parseArgs(process.argv.slice(2));
  compare(dumpPath, expectedPath);
}
module.exports = { compare, parseSvg, sourceGeometry, sourcePath, decimal, add, div, key };
