'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
function findRoot(start=__dirname) {
  for(let current=path.resolve(start);current!==path.dirname(current);current=path.dirname(current))
    if(fs.existsSync(path.join(current,'docs/math-port/scope.json')) && fs.existsSync(path.join(current,'tools/mathjax-bundle/node_modules/@mathjax/src/package.json'))) return current;
  return null;
}
const rootArg=process.argv.indexOf('--repo-root');
const sceneMode=process.argv.includes('--scene');
const captureOnly=process.argv.includes('--capture-only');
const writeFixture=process.argv.includes('--write-fixture');
assert.ok(!(captureOnly&&writeFixture),'--capture-only and --write-fixture are mutually exclusive');
const positionalArgs=process.argv.slice(2).filter(x=>!['--scene','--capture-only','--write-fixture'].includes(x));
assert.ok(rootArg<0 ? positionalArgs.length===0 : positionalArgs.length===2&&positionalArgs[0]==='--repo-root'&&positionalArgs[1],'usage: node common-limits-source-witness.cjs [--repo-root <repository>] [--scene] [--capture-only|--write-fixture]');
const root=rootArg<0?findRoot():path.resolve(process.argv[rootArg+1]);
assert.ok(root,'pass --repo-root <repository> or run from within the repository');
const pkg = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const lock = JSON.parse(fs.readFileSync(path.join(root,'tests/math/fixtures/common-source/limits-source-lock.json'),'utf8'));
assert.equal(JSON.parse(fs.readFileSync(path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/package.json'))).version,lock.version,'MathJax package version');
for (const [relative, expected] of Object.entries(lock.sha256)) {
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(pkg, relative))).digest('hex');
  assert.equal(actual, expected, `pinned MathJax source changed: ${relative}`);
}
for (const [relative, expected] of Object.entries(lock.sourceTsSha256)) {
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(root, relative))).digest('hex');
  assert.equal(actual, expected, `pinned TypeScript source changed: ${relative}`);
}
for (const [relative, expected] of Object.entries(lock.externalSha256 ?? {})) {
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(root, relative))).digest('hex');
  assert.equal(actual, expected, `pinned dependency source changed: ${relative}`);
}
const { MmlFactory } = require(path.join(pkg, 'core/MmlTree/MmlFactory.js'));
const { BBox } = require(path.join(pkg, 'util/BBox.js'));
const { CommonScriptbaseMixin } = require(path.join(pkg, 'output/common/Wrappers/scriptbase.js'));
const { CommonMunderMixin, CommonMoverMixin, CommonMunderoverMixin } = require(path.join(pkg, 'output/common/Wrappers/munderover.js'));
const params = {sub1:.2,sub2:.16,sup1:.4,sup2:.3,sup3:.5,sub_drop:.1,sup_drop:.1,x_height:.4,
 rule_thickness:.04,separation_factor:5,big_op_spacing1:.1,big_op_spacing2:.1,big_op_spacing3:.1,big_op_spacing4:.1,big_op_spacing5:.15,scriptspace:.05,extra_ic:0};
function box(data={}) { return new BBox({w:.5,h:.7,d:.2,...data}); }
class ChildWrapper {
 constructor(node,parent,metrics) { this.node=node; this.parent=parent; this.childNodes=[]; this.bbox=metrics; }
 getOuterBBox() { return this.bbox.copy(); }
 getLineBBox() { return this.bbox.copy(); }
 breakCount() { return 0; }
 getText() { return this.node.getText ? this.node.getText() : ''; }
 coreMO() { return this; }
 canStretch() { return false; }
}
function Base(factory,node,parent=null) {
 this.factory=factory; this.node=node; this.parent=parent; this.childNodes=(node.childNodes||[]).map((n,i)=>new ChildWrapper(n,this, i===0?box({w:.5,h:.7,d:.2,ic:.08,sk:.02}):box({w:.2,h:.3,d:.1,ic:0,sk:0})));
 this.bbox=box(); this.font={params,skewIcFactor:.75}; this.jax={face:{params}}; this.isLineAbove=false; this.isLineBelow=false;
}
Base.prototype.getOuterBBox=function(){return this.bbox.copy();};
Base.prototype.getLineBBox=function(){return this.bbox.copy();};
Base.prototype.setChildPWidths=function(){};
Base.prototype.length2em=function(value,fallback){return value===''||value==null?fallback:Number(value);};
const Script=CommonScriptbaseMixin(Base);
const Munder=CommonMunderMixin(Script), Mover=CommonMoverMixin(Script), Munderover=CommonMunderoverMixin(Script);
const factory=new MmlFactory();
const text=s=>factory.create('text').setText(s);
const mi=s=>factory.create('mi',{},[text(s)]);
const run=(kind,kids,attrs={})=>{ const node=factory.create(kind,attrs,kids); return kind==='munder'?new Munder({},node):kind==='mover'?new Mover({},node):new Munderover({},node); };
function bboxOf(w) { const b=BBox.zero(); w.computeBBox(b); return {w:b.w,h:b.h,d:b.d}; }
const under=run('munder',[mi('x'),mi('i')]);
const underResult=bboxOf(under); assert.equal(under.scriptChild.node.getText(),'i');
const over=run('mover',[mi('x'),mi('2')]);
const overResult=bboxOf(over); assert.equal(over.scriptChild.node.getText(),'2');
const both=run('munderover',[mi('x'),mi('i'),mi('2')]);
const bothResult=bboxOf(both); assert.equal(both.underChild.node.getText(),'i'); assert.equal(both.overChild.node.getText(),'2');
const accented=run('mover',[mi('x'),mi('^')],{accent:true});
const accentResult=bboxOf(accented);
assert.ok(accentResult.h > overResult.h, 'accent raises the base to x_height before placement');
const lineUnder=run('munder',[mi('x'),factory.create('mo',{},[text('―')])]);
const lineUnderResult=bboxOf(lineUnder);
const lineOver=run('mover',[mi('x'),factory.create('mo',{},[text('―')])]);
const lineOverResult=bboxOf(lineOver);
assert.equal(lineUnder.isLineBelow,true,'munder line accent is recognized from the actual Mo token');
assert.equal(lineOver.isLineAbove,true,'mover line accent is recognized from the actual Mo token');
const movableBase=factory.create('mo',{movablelimits:true},[text('∫')]);
const movable=run('munderover',[movableBase,mi('i'),mi('2')],{displaystyle:false});
assert.equal(movable.hasMovableLimits(),true);
const movableResult=bboxOf(movable);
const baseBox=movable.baseChild.getOuterBBox(), scriptBox=movable.scriptChild.getOuterBBox();
assert.deepEqual(movableResult,{w:Math.max(baseBox.w,movable.getBaseWidth()+scriptBox.w)+params.scriptspace,h:Math.max(baseBox.h,scriptBox.h),d:Math.max(baseBox.d,scriptBox.d)});
const movableUnder=run('munder',[factory.create('mo',{movablelimits:true},[text('∫')]),mi('i')],{displaystyle:false});
const movableUnderResult=bboxOf(movableUnder);
assert.equal(movableUnder.hasMovableLimits(),true);
const movableOver=run('mover',[factory.create('mo',{movablelimits:true},[text('∫')]),mi('2')],{displaystyle:false});
const movableOverResult=bboxOf(movableOver);
assert.equal(movableOver.hasMovableLimits(),true);

function cppString(value) {
  return JSON.stringify(value);
}

function cppU16(value) {
  return `u${JSON.stringify(value)}`;
}

function cppRaw(value, limit = 3000) {
  const parts = [];
  let part = '', bytes = 0;
  for (const cp of value) {
    const size = Buffer.byteLength(cp);
    if (part && bytes + size > limit) {
      parts.push(part);
      part = '';
      bytes = 0;
    }
    part += cp;
    bytes += size;
  }
  if (part || !parts.length) parts.push(part);
  return parts.map((text, i) => {
    let delimiter = `LIMITS${i}`;
    while (text.includes(`)${delimiter}"`)) delimiter += 'X';
    return `R"${delimiter}(${text})${delimiter}"`;
  }).join('\n    ');
}

function cppAttribute(attribute) {
  const name = cppU16(attribute.name);
  switch (typeof attribute.value) {
    case 'boolean': return `{${name}, mathml_scene_test::AttributeKind::boolean, {}, 0, ${attribute.value}}`;
    case 'number':
      assert.ok(Number.isFinite(attribute.value), `Non-finite MML attribute ${attribute.name}`);
      return `{${name}, mathml_scene_test::AttributeKind::number, {}, ${attribute.value}, false}`;
    case 'string': return `{${name}, mathml_scene_test::AttributeKind::text, ${cppU16(attribute.value)}, 0, false}`;
    default: throw new Error(`Unsupported actual MML attribute ${attribute.name}: ${typeof attribute.value}`);
  }
}

function flattenActualMml(rootNode) {
  const nodes = [], attributes = [], indices = new WeakMap();
  function visit(node, parent) {
    const index = nodes.length;
    indices.set(node, index);
    const first = attributes.length;
    const own = Object.entries(node.attributes?.getAllAttributes?.() ?? {});
    for (const [name, value] of own) attributes.push({name, value});
    nodes.push({kind:node.kind, inferred:node.isKind?.('inferredMrow') ?? false,
      parent, first, count:attributes.length-first,
      text:node.kind === 'text' ? node.getText() : ''});
    for (const child of node.childNodes ?? []) visit(child, index);
  }
  visit(rootNode, 'std::size_t(-1)');
  return {nodes, attributes, indices};
}

async function captureLimitsScene() {
  const {createReference} = require('./reference-profile.cjs');
  const ref = createReference();
  assert.deepEqual(ref.provenance.packages, JSON.parse(fs.readFileSync(
    path.join(root, 'docs/math-port/scope.json'), 'utf8')).profile.tex_packages);
  const {MathML} = require(path.join(pkg, 'input/MathML.js'));
  const input = new MathML();
  const outputJax = ref.output;
  const document = ref.mathjax.document('', {InputJax:input, OutputJax:outputJax});
  const cases = [];
  for (const kind of ['munder', 'mover', 'munderover']) {
    for (const condition of ['inline-movable-integral', 'display-sum-limits',
                             'explicit-accent-x', 'wide-stretchy-accent-row']) {
      let display = condition === 'display-sum-limits';
      let base, under = '', over = '', attrs = '';
      if (condition === 'inline-movable-integral') {
        base = '<mo movablelimits="true">∫</mo>';
        under = '<mi>i</mi>';
        over = '<mi>2</mi>';
      } else if (condition === 'display-sum-limits') {
        base = '<mo largeop="true" movablelimits="true">∑</mo>';
        under = '<mi>i</mi>';
        over = '<mi>n</mi>';
      } else if (condition === 'explicit-accent-x') {
        base = '<mi>x</mi>';
        attrs = kind === 'munder' ? ' accentunder="true"'
          : kind === 'mover' ? ' accent="true"'
          : ' accent="true" accentunder="true"';
        under = '<mo>⏟</mo>';
        over = '<mo>⏞</mo>';
      } else {
        base = '<mrow><mi>x</mi><mi>y</mi><mi>z</mi><mi>w</mi></mrow>';
        attrs = kind === 'munder' ? ' accentunder="true"'
          : kind === 'mover' ? ' accent="true"'
          : ' accent="true" accentunder="true"';
        under = '<mo stretchy="true">⏟</mo>';
        over = '<mo stretchy="true">⏞</mo>';
      }
      const children = kind === 'munder' ? `${base}${under}`
        : kind === 'mover' ? `${base}${over}` : `${base}${under}${over}`;
      const name = `${kind}-${condition}`;
      const mathml = `<math xmlns="http://www.w3.org/1998/Math/MathML"${display ? ' display="block"' : ''}>`+
        `<${kind}${attrs}>${children}</${kind}></math>`;
      cases.push({name, kind, condition, mathml, display, em:16, ex:8, width:640});
    }
  }
  assert.equal(cases.length, 12);
  const records = [];
  const fontCachePrototype = Object.getPrototypeOf(outputJax.fontCache);
  const originalUseLocalID = fontCachePrototype.useLocalID;
  let activePrefixes = null;
  fontCachePrototype.useLocalID = function(...args) {
    const result = originalUseLocalID.apply(this, args);
    if (activePrefixes) activePrefixes.push(this.localID);
    return result;
  };
  const originalProcessMath = outputJax.processMath;
  let active = null;
  outputJax.processMath = function(wrapper, node) {
    assert.ok(active, 'limits Source processMath has an active capture');
    const candidates = [];
    const findTargets = current => {
      if (current.node.kind === active.kind) candidates.push(current);
      for (const child of current.childNodes ?? []) findTargets(child);
    };
    findTargets(wrapper);
    assert.equal(candidates.length, 1, `${active.name}: one actual SVG limits wrapper before Source emission`);
    active.target = candidates[0];
    if (active.condition === 'inline-movable-integral' ||
        active.condition === 'display-sum-limits')
      active.movableLimits = active.target.hasMovableLimits();
    if (active.condition === 'wide-stretchy-accent-row') {
      const accentIndices = active.kind === 'munderover' ? [1,2] : [1];
      active.stretchSizes = accentIndices.map(index => active.target.childNodes[index].coreMO().size);
    }
    const result = originalProcessMath.apply(this, arguments);
    active.actualDisplay = this.math.display;
    active.mml = flattenActualMml(wrapper.node);
    const rootBBox = wrapper.getBBox();
    active.rootBBox = Object.fromEntries(['w','h','d','scale','rscale','L','R']
      .map(key => [key, rootBBox[key]]));
    return result;
  };
  try {
    for (const fontCache of ['none', 'local']) {
      for (const entry of cases) {
        outputJax.fontCache.clearCache();
        outputJax.options.fontCache = fontCache;
        outputJax.options.localID = null;
        activePrefixes = [];
        active = {...entry};
        const outer = await document.convertPromise(entry.mathml, {
          display:entry.display, em:entry.em, ex:entry.ex, containerWidth:entry.width
        });
        const svg = ref.adaptor.serializeXML(outer);
        assert.ok(active.mml && active.rootBBox && svg, `${entry.name}/${fontCache}: Source capture incomplete`);
        const target = active.target;
        assert.ok(target, `${entry.name}/${fontCache}: actual limits wrapper was captured`);
        const nodeIndex = active.mml.indices.get(target.node);
        assert.ok(nodeIndex !== undefined, `${entry.name}: target node appears in actual compiled MML flattening`);
        assert.equal(active.mml.nodes[nodeIndex].kind, entry.kind,
          `${entry.name}: target index refers to the expected compiled MML node`);
        assert.equal(active.actualDisplay, entry.display,
          `${entry.name}: captured Source MathItem display follows the input condition`);
        const targetAttributes = active.mml.attributes.slice(
          active.mml.nodes[nodeIndex].first,
          active.mml.nodes[nodeIndex].first + active.mml.nodes[nodeIndex].count);
        if (entry.condition === 'inline-movable-integral') {
          assert.equal(active.movableLimits, true,
            `${entry.name}: actual Common wrapper selects the movable-limits branch`);
        } else if (entry.condition === 'display-sum-limits') {
          assert.equal(active.movableLimits, false,
            `${entry.name}: display math selects ordinary limits placement`);
        }
        if (entry.condition === 'explicit-accent-x' ||
            entry.condition === 'wide-stretchy-accent-row') {
          const expectedAccents = entry.kind === 'munder' ? [['accentunder', true]]
            : entry.kind === 'mover' ? [['accent', true]]
            : [['accent', true], ['accentunder', true]];
          for (const [name, value] of expectedAccents)
            assert.ok(targetAttributes.some(attribute => attribute.name === name && attribute.value === value),
              `${entry.name}: actual compiled MML retains ${name}=${value}`);
        }
        const targetBox = target.getBBox();
        const limitsBBox = Object.fromEntries(['w','h','d','scale','rscale','L','R']
          .map(key => [key, targetBox[key]]));
        if (entry.condition === 'wide-stretchy-accent-row') {
          const accentIndices = entry.kind === 'munder' ? [1]
            : entry.kind === 'mover' ? [1] : [1,2];
          const baseBox = target.childNodes[0].getOuterBBox();
          for (const index of accentIndices) {
            const accentBox = target.childNodes[index].getOuterBBox();
            assert.ok(accentBox.w >= baseBox.w - 1e-9,
              `${entry.name}: actual Source accent wrapper stretched to the wide base`);
            assert.notEqual(active.stretchSizes[accentIndices.indexOf(index)], null,
              `${entry.name}: Source CommonMo selected a stretched glyph variant/assembly`);
          }
        }
        assert.ok(Buffer.byteLength(svg) > 0, `${entry.name}: full Source SVG present`);
        assert.ok(activePrefixes.every(prefix => typeof prefix === 'string'),
          `${entry.name}: actual LocalID prefixes are strings`);
        if (fontCache === 'none') assert.equal(activePrefixes.length, 0,
          `${entry.name}: no local cache requests in none mode`);
        else assert.ok(activePrefixes.length > 0,
          `${entry.name}: actual local cache requests observed`);
        records.push({...entry, fontCache, nodes:active.mml.nodes, attributes:active.mml.attributes,
          rootBBox:active.rootBBox, limitsNodeIndex:nodeIndex, limitsBBox,
          localCachePrefixes:activePrefixes.slice(), svg});
        process.stdout.write(`${entry.name}/${fontCache}: node=${nodeIndex} svg=${Buffer.byteLength(svg)}B prefixes=${activePrefixes.length}\n`);
        active = null;
        activePrefixes = null;
      }
    }
  } finally {
    active = null;
    activePrefixes = null;
    outputJax.processMath = originalProcessMath;
    fontCachePrototype.useLocalID = originalUseLocalID;
  }
  assert.equal(records.length, 24, 'all 12 input cases complete in none/local modes');
  return {ref, records};
}

function cppLimitsTree(record, index) {
  const prefix = `sourceLimits${index}`;
  const nodes = record.nodes.map(node =>
    `{${cppString(node.kind)}, ${node.inferred}, ${node.parent}, ${node.first}, ${node.count}, ${cppU16(node.text)}}`);
  const attributes = record.attributes.map(cppAttribute);
  const declarations =
    `constexpr std::array<mathml_scene_test::Node, ${nodes.length}> ${prefix}Nodes{{\n  ${nodes.join(',\n  ')}\n}};\n`+
    `constexpr std::array<mathml_scene_test::Attribute, ${attributes.length}> ${prefix}Attributes{{\n`+
    `${attributes.length ? '  ' + attributes.join(',\n  ') : ''}\n}};\n`+
    `constexpr std::array<std::string_view, ${record.localCachePrefixes.length}> ${prefix}Prefixes{{`+
    `${record.localCachePrefixes.map(cppString).join(',')}}};\n`;
  const bbox = value => `LimitsSceneBBox{${value.w},${value.h},${value.d},${value.scale},${value.rscale},${value.L},${value.R}}`;
  const item = `LimitsSceneSourceCase{${cppString(record.name)},${cppString(record.fontCache)},`+
    `${record.display},${record.em},${record.ex},${record.width},${prefix}Nodes,${prefix}Attributes,`+
    `${bbox(record.rootBBox)},${record.limitsNodeIndex},${bbox(record.limitsBBox)},${prefix}Prefixes,${cppRaw(record.svg)}}`;
  return {declarations, item};
}

async function mainScene() {
  const {ref, records} = await captureLimitsScene();
  const provenance = {
    upstream:lock.version,
    source:'actual compiled MathML through the pinned full38 MathJax reference profile and NewCM SVG output; no measurements or IDs synthesized',
    compiledSourceSha256:ref.provenance.compiled_source_sha256,
    packages:ref.provenance.packages,
    caseCount:records.length,
  };
  const generated = records.map(cppLimitsTree);
  const fixture = `// Pinned MathJax ${lock.version}; direct MathML limits Source; full38 NewCM profile.\n`+
    `// Generated from actual compiled MML, Common wrapper BBoxes, local IDs, and complete SVG.\n`+
    `struct LimitsSceneBBox { double w,h,d,scale,rscale,L,R; };\n`+
    `struct LimitsSceneSourceCase { std::string_view name,font_cache; bool display; double em,ex,width; `+
    `std::span<mathml_scene_test::Node const> nodes; std::span<mathml_scene_test::Attribute const> attributes; `+
    `LimitsSceneBBox root_bbox; std::size_t limits_node_index; LimitsSceneBBox limits_bbox; `+
    `std::span<std::string_view const> local_cache_prefixes; std::string_view svg; };\n\n`+
    generated.map(item => item.declarations).join('\n')+
    `\nconstexpr std::array<LimitsSceneSourceCase, ${generated.length}> sourceLimitsSceneCases{{\n`+
    generated.map(item => `  ${item.item}`).join(',\n')+`\n}};\n`;
  const evidencePath = path.join(root, 'build/math-port/common-source/limits-mathml-scene.json');
  fs.mkdirSync(path.dirname(evidencePath), {recursive:true});
  fs.writeFileSync(evidencePath, JSON.stringify({provenance, records:records.map(({nodes,attributes,...rest}) =>
    ({...rest, nodes, attributes}))}, null, 2)+'\n');
  const target = path.join(root, 'tests/math/fixtures/common-source/limits-mathml-scene.inc');
  const previous = fs.existsSync(target) ? fs.readFileSync(target, 'utf8').replaceAll('\r\n','\n') : null;
  if (writeFixture) {
    if (previous !== fixture) fs.writeFileSync(target, fixture);
  } else if (!captureOnly) {
    assert.equal(previous, fixture, 'Independent limits MathML Source fixture differs; review before --write-fixture');
  }
  const numericBefore = fs.readFileSync(path.join(root, 'tests/math/fixtures/common-source/limits-source-expected.json'));
  assert.equal(crypto.createHash('sha256').update(numericBefore).digest('hex'),
    '439b8fd72e0e6a1505107e6547d385536eff2bd73108ee3958900e21e36e644e',
    'numeric limits fixture must remain byte-identical');
  process.stdout.write(`Verified ${records.length} direct MathML limits Source cases; fixture ${path.relative(root,target)}; evidence ${path.relative(root,evidencePath)}\n`);
}

if (sceneMode) {
  mainScene().catch(error => {process.stderr.write(error.stack+'\n'); process.exitCode=1;});
} else {
const {createReference}=require('./reference-profile.cjs');
const ref=createReference();
assert.equal(ref.provenance.compiled_source_sha256,
  'cbb0d3b6a08fbde6614b070f111fb58cc3cfe9d00a9713163fd4cd6d0300355a');
const {MathML}=require(path.join(pkg,'input/MathML.js'));
const outputJax=ref.output;
const document=ref.mathjax.document('',{InputJax:new MathML(),OutputJax:outputJax});
function findWrapper(wrapper,kind) {
  if(wrapper.node.kind===kind) return wrapper;
  for(const child of wrapper.childNodes) {
    const found=findWrapper(child,kind);
    if(found) return found;
  }
  return null;
}
function captureSvg(kind,body) {
  let rootWrapper;
  const processMath=outputJax.processMath;
  outputJax.processMath=function(wrapper,parent) {
    const result=processMath.call(this,wrapper,parent);
    rootWrapper=wrapper;
    return result;
  };
  try {
    document.convert(`<math xmlns="http://www.w3.org/1998/Math/MathML">${body}</math>`,
      {display:false,em:16,ex:8,containerWidth:640});
    assert.ok(rootWrapper,`SVG processMath reached for ${kind}`);
    const wrapper=findWrapper(rootWrapper,kind);
    assert.ok(wrapper,`SVG wrapper ${kind} found`);
    const bbox=wrapper.getOuterBBox();
    const childLabel=child=>child.node.getText?.()||child.node.kind;
    return {
      bbox:{w:bbox.w,h:bbox.h,d:bbox.d},
      offset:wrapper.getOffset(),
      childSlots:wrapper.childNodes.map(childLabel),
      scriptSlot:wrapper.scriptChild?wrapper.childNodes.indexOf(wrapper.scriptChild):null,
      underSlot:wrapper.underChild?wrapper.childNodes.indexOf(wrapper.underChild):null,
      overSlot:wrapper.overChild?wrapper.childNodes.indexOf(wrapper.overChild):null,
      baseKind:wrapper.baseChild?.node.kind??null,
      baseMovablelimits:wrapper.baseChild?.node.attributes.get('movablelimits')??null,
    };
  } finally {
    outputJax.processMath=processMath;
  }
}
function captureSvgCases() {
  return {
    munder:captureSvg('munder','<munder><mo movablelimits="true">∫</mo><mi>i</mi></munder>'),
    mover:captureSvg('mover','<mover><mo movablelimits="true">∫</mo><mi>2</mi></mover>'),
    munderover:captureSvg('munderover',
      '<munderover><mo movablelimits="true">∫</mo><mi>i</mi><mi>2</mi></munderover>'),
  };
}
const isolatedMixins={
  under:{script:under.scriptChild.getText(),bbox:underResult},
  over:{script:over.scriptChild.getText(),bbox:overResult},
  both:{under:both.underChild.getText(),over:both.overChild.getText(),bbox:bothResult},
  accent:{accent:accented.node.attributes.get('accent'),bbox:accentResult},
  lineAccent:{under:lineUnderResult,over:lineOverResult},
  movable:{enabled:movable.hasMovableLimits(),bbox:movableResult},
  movableUnder:{enabled:movableUnder.hasMovableLimits(),offset:movableUnder.getOffset(),bbox:movableUnderResult},
  movableOver:{enabled:movableOver.hasMovableLimits(),offset:movableOver.getOffset(),bbox:movableOverResult},
};
const report={
  version:lock.version,
  source:'isolated Common mixin probes are algorithm-only; independent actual SVG wrappers captured after processMath from pinned MathJax 4.1.3 reference profile and NewCM using concrete MathML inputs',
  cases:{svgActual:captureSvgCases(),isolatedMixins},
};
const output=path.join(root,'build/math-port/common-source/limits-source.json');
fs.mkdirSync(path.dirname(output),{recursive:true});
const contents=JSON.stringify(report,null,2)+'\n';
fs.writeFileSync(output,contents);
const expectedFile=path.join(root,'tests/math/fixtures/common-source/limits-source-expected.json');
if(process.argv.includes('--write-fixture')) {
  const existing=fs.existsSync(expectedFile)?fs.readFileSync(expectedFile,'utf8'):null;
  if(existing?.replaceAll('\r\n','\n')!==contents) fs.writeFileSync(expectedFile,contents);
} else if(!process.argv.includes('--capture-only')) {
  const expected=fs.readFileSync(expectedFile,'utf8');
  assert.deepEqual(JSON.parse(contents),JSON.parse(expected),
    'pinned limits evidence changed; inspect Source values before updating the fixture');
}
process.stdout.write(`captured ${Object.keys(report.cases.svgActual).length} actual SVG limits wrappers plus ${Object.keys(report.cases.isolatedMixins).length} isolated mixin probes; evidence ${path.relative(root,output)}\n`);
}
