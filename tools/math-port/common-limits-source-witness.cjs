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
assert.ok(rootArg<0 ? process.argv.length===2 : (process.argv[rootArg+1] && process.argv.length===rootArg+2),'usage: node common-limits-source-witness.cjs [--repo-root <repository>]');
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
const report={version:lock.version,source:'executed pinned MathJax CJS MmlFactory and CommonMunder/CommonMover/CommonMunderover mixins with real util/BBox',cases:{under:{script:under.scriptChild.getText(),bbox:underResult},over:{script:over.scriptChild.getText(),bbox:overResult},both:{under:both.underChild.getText(),over:both.overChild.getText(),bbox:bothResult},accent:{accent:accented.node.attributes.get('accent'),bbox:accentResult},lineAccent:{under:lineUnderResult,over:lineOverResult},movable:{enabled:movable.hasMovableLimits(),bbox:movableResult},movableUnder:{enabled:movableUnder.hasMovableLimits(),offset:movableUnder.getOffset(),bbox:movableUnderResult},movableOver:{enabled:movableOver.hasMovableLimits(),offset:movableOver.getOffset(),bbox:movableOverResult}}};
const output=path.join(root,'build/math-port/common-source/limits-source.json');
fs.mkdirSync(path.dirname(output),{recursive:true});
const contents=JSON.stringify(report,null,2)+'\n';
const expected=fs.readFileSync(path.join(root,'tests/math/fixtures/common-source/limits-source-expected.json'),'utf8');
assert.deepEqual(JSON.parse(contents),JSON.parse(expected),'pinned native limits comparison evidence changed');
fs.writeFileSync(output,contents);
process.stdout.write(`verified ${Object.keys(report.cases).length} limits cases; evidence ${path.relative(root,output)}\n`);
