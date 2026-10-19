// Source witness for SVG Maction's actual static Map and instance methods.
'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const path = require('node:path');
const root = path.resolve(__dirname, '../..');
const packages = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax');
const src = path.join(packages, 'src/cjs');
const load = file => require(path.join(src, file));
load('output/svg.js');
load('output/svg/Wrapper.js');
const file = path.join(src, 'output/svg/Wrappers/maction.js');
const { SvgMaction } = require(file);
const { TooltipData } = require(path.join(src, 'output/common/Wrappers/maction.js'));
const { CommonMactionMixin } = require(path.join(src, 'output/common/Wrappers/maction.js'));
const { STATE } = require(path.join(src, 'core/MathItem.js'));

const keys = [...SvgMaction.actions.keys()];
assert.deepEqual(keys, ['toggle', 'tooltip', 'statusline']);
for (const key of keys) {
  const pair = SvgMaction.actions.get(key);
  assert.equal(Array.isArray(pair), true);
  assert.equal(typeof pair[0], 'function');
  assert.equal(typeof pair[1], 'object');
}
assert.equal(SvgMaction.actions instanceof Map, true);
assert.equal(Object.getOwnPropertyDescriptor(SvgMaction, 'actions').enumerable, true);
assert.equal(SvgMaction.prototype.setEventHandler.length, 3);
assert.equal(SvgMaction.prototype.Px.length, 1);
const styleSelectors = Object.keys(SvgMaction.styles);
assert.deepEqual(styleSelectors, [
  '[jax="SVG"] mjx-tool', '[jax="SVG"] mjx-tool > mjx-tip', 'mjx-tool > mjx-tip',
  'g[data-mml-node="maction"][data-toggle]', 'mjx-status', 'g[data-mjx-collapsed]',
  '@media (prefers-color-scheme: dark) /* svg maction */',
]);
assert.deepEqual(Object.keys(SvgMaction.styles['mjx-tool > mjx-tip']), [
  'display', 'line-height', 'padding', 'border', 'background-color', 'color', 'box-shadow',
]);

// The Common constructor reads the current constructor Map, destructures its
// pair, assigns both fields, and only then dynamically calls getParameters.
const commonEvents = [];
const actionFunction = function (node, data) { commonEvents.push(['action', this, node, data]); };
const actionData = { marker: 19 };
const actionsMap = new Map([['custom', [actionFunction, actionData]]]);
const observedGet = actionsMap.get;
actionsMap.get = function (key) {
  commonEvents.push(['map.get', this, key]);
  return observedGet.call(this, key);
};
function BaseMaction(factory, node, parent) {
  this.factory = factory; this.node = node; this.parent = parent; this.childNodes = [];
}
const TestCommonMaction = CommonMactionMixin(BaseMaction);
TestCommonMaction.actions = actionsMap;
TestCommonMaction.prototype.getParameters = function () {
  commonEvents.push(['getParameters', this, this.action, this.data]);
};
const commonNode = { attributes: { get(key) { commonEvents.push(['attributes.get', this, key]); return 'custom'; } } };
const commonInstance = new TestCommonMaction({}, commonNode);
assert.equal(commonInstance.action, actionFunction);
assert.equal(commonInstance.data, actionData);
assert.deepEqual(commonEvents.map(event => event[0]), ['attributes.get', 'map.get', 'getParameters']);
assert.equal(commonEvents[0][1], commonNode.attributes);
assert.equal(commonEvents[1][1], actionsMap);
assert.equal(commonEvents[2][1], commonInstance);
assert.equal(commonEvents[2][2], actionFunction);
assert.equal(commonEvents[2][3], actionData);
const unknownInstance = new TestCommonMaction({}, {
  attributes: { get() { return 'missing'; } },
});
assert.equal(typeof unknownInstance.action, 'function');
assert.deepEqual(unknownInstance.data, {});
assert.equal(unknownInstance.action(unknownInstance, unknownInstance.data), undefined);
const parameterEvents = [];
function ParameterBase(factory, node, parent) {
  BaseMaction.call(this, factory, node, parent);
  this.length2em = function (value) {
    parameterEvents.push(['length2em', this, value]);
    return `em:${value}`;
  };
}
ParameterBase.prototype = Object.create(BaseMaction.prototype);
ParameterBase.prototype.constructor = ParameterBase;
const ParameterMaction = CommonMactionMixin(ParameterBase);
ParameterMaction.actions = new Map();
const parameterNode = { attributes: { get(key) {
  parameterEvents.push(['attributes.get', this, key]);
  return key === 'actiontype' ? 'missing' : '  .25em   3ex  ';
} } };
const parameterInstance = new ParameterMaction({}, parameterNode);
assert.deepEqual(parameterEvents.map(event => event[0]), [
  'attributes.get', 'attributes.get', 'length2em', 'length2em',
]);
assert.equal(parameterInstance.tipDx, 'em:.25em');
assert.equal(parameterInstance.tipDy, 'em:3ex');
assert.equal(parameterEvents[2][1], parameterInstance);
assert.equal(parameterEvents[3][1], parameterInstance);

// Execute the Source action callbacks against receivers that record the
// actual member calls and receiver objects.
const actionEvents = [];
const originalEnriched = STATE.ENRICHED;
STATE.ENRICHED = 140;
const actionNode = {
  dom: [{ id: 'toggle-dom' }],
  node: {
    attributes: { get(name) { actionEvents.push(['attribute', name]); return name === 'selection' ? '2' : 'mjx-1'; } },
    nextToggleSelection() { actionEvents.push(['nextToggleSelection', this]); },
  },
  adaptor: { setAttribute(...args) { actionEvents.push(['setAttribute', ...args]); } },
  factory: { jax: {
    math: {
      end: { node: {} },
      rerender(...args) { actionEvents.push(['rerender', ...args]); },
    },
    document: { id: 'document' },
  } },
  setEventHandler(type, callback) { actionEvents.push(['setEventHandler', type]); this.click = callback; },
};
SvgMaction.actions.get('toggle')[0](actionNode, {});
assert.deepEqual(actionEvents.slice(0, 3).map(event => event[0]), ['attribute', 'setAttribute', 'setEventHandler']);
const clickEvent = { stopPropagation() { actionEvents.push(['stopPropagation']); } };
actionNode.click(clickEvent);
assert.equal(actionEvents.some(event => event[0] === 'nextToggleSelection'), true);
assert.deepEqual(actionEvents.find(event => event[0] === 'rerender').slice(1), [actionNode.factory.jax.document, STATE.ENRICHED]);
assert.equal(actionEvents.at(-1)[0], 'stopPropagation');
STATE.ENRICHED = originalEnriched;

// TooltipData.stopTimers must preserve the two Map operation sequences and
// pass the returned handles to the current clearTimeout function.
const timerEvents = [];
class ObservedMap extends Map {
  constructor(label, entries) { super(entries); this.label = label; }
  has(key) { timerEvents.push([this.label, 'has', key]); return super.has(key); }
  get(key) { timerEvents.push([this.label, 'get', key]); return super.get(key); }
  delete(key) { timerEvents.push([this.label, 'delete', key]); return super.delete(key); }
}
const oldClearTimeout = global.clearTimeout;
global.clearTimeout = handle => timerEvents.push(['clearTimeout', handle]);
try {
  const node = { id: 'timer-node' };
  const data = {
    clearTimer: new ObservedMap('clear', [[node, 17]]),
    hoverTimer: new ObservedMap('hover', [[node, 23]]),
  };
  TooltipData.stopTimers(node, data);
  assert.deepEqual(timerEvents.map(event => event.slice(0, 2)), [
    ['clear', 'has'], ['clear', 'get'], ['clearTimeout', 17], ['clear', 'delete'],
    ['hover', 'has'], ['hover', 'get'], ['clearTimeout', 23], ['hover', 'delete'],
  ]);
  assert.equal(data.clearTimer.size, 0);
  assert.equal(data.hoverTimer.size, 0);
} finally {
  global.clearTimeout = oldClearTimeout;
}

// Text tooltip construction uses the actual title/text/svg/adaptor methods in
// Source order and skips event registration.
const textEvents = [];
const textDom = { id: 'text-dom' };
const textRect = { id: 'rect' };
const textTipNode = {
  isKind(kind) { textEvents.push(['isKind', this, kind]); return kind === 'mtext'; },
  getText() { textEvents.push(['getText', this]); return 'plain tip'; },
};
const textTitle = { id: 'title' };
const textNode = {
  childNodes: [{}, { node: textTipNode }],
  dom: [textDom],
  firstChild(node) { textEvents.push(['firstChild', this, node]); return textRect; },
  adaptor: {
    insert(node, before) { textEvents.push(['insert', this, node, before]); },
  },
  text(value) { textEvents.push(['text', this, value]); return { value }; },
  svg(...args) { textEvents.push(['svg', this, ...args]); return textTitle; },
  setEventHandler(...args) { textEvents.push(['setEventHandler', this, ...args]); },
};
SvgMaction.actions.get('tooltip')[0](textNode, {});
assert.deepEqual(textEvents.map(event => event[0]), ['firstChild', 'isKind', 'getText', 'text', 'svg', 'insert']);
assert.equal(textEvents[1][1], textTipNode);
assert.equal(textEvents[2][1], textTipNode);
assert.equal(textEvents[4][1], textNode);
assert.deepEqual(textEvents[4].slice(2), ['title', {}, [{ value: 'plain tip' }]]);
assert.deepEqual(textEvents[5].slice(2), [textTitle, textRect]);

// A mathematical tooltip creates and processes the actual temporary MML node,
// then registers both per-DOM handlers in Source order.
const mathEvents = [];
const mathDom = { id: 'math-dom' };
const mathRect = { id: 'math-rect' };
const tipMml = { id: 'tip-mml', parent: null, isKind(kind) { mathEvents.push(['isKind', kind]); return false; } };
const mathMml = { id: 'math-mml' };
const toolNode = { id: 'tool' };
const foreignNode = { id: 'foreign' };
const hiddenNode = { id: 'hidden' };
const processWrapper = { id: 'process-wrapper' };
const mathAdaptor = {
  firstChild(node) { mathEvents.push(['firstChild', node]); return mathRect; },
  append(...args) { mathEvents.push(['append', ...args]); return hiddenNode; },
};
const mathNode = {
  childNodes: [{}, { node: tipMml }], dom: [mathDom],
  firstChild(node) { mathEvents.push(['firstChild', node]); return mathRect; },
  node: { factory: { create(...args) { mathEvents.push(['create', ...args]); return mathMml; } } },
  adaptor: mathAdaptor,
  jax: { container: { id: 'container' }, factory: { wrap(node) { mathEvents.push(['wrap', node]); return processWrapper; } }, processMath(...args) { mathEvents.push(['processMath', ...args]); } },
  html(...args) { mathEvents.push(['html', ...args]); return args[0] === 'mjx-tool' ? toolNode : { id: 'tip' }; },
  svg(...args) { mathEvents.push(['svg', ...args]); return foreignNode; },
  setEventHandler(...args) { mathEvents.push(['setEventHandler', ...args]); },
};
const mathData = {
  stopTimers(dom, data) { mathEvents.push(['stopTimers', dom, data]); },
  hoverTimer: new Map(), clearTimer: new Map(), postDelay: 600, clearDelay: 100,
};
SvgMaction.actions.get('tooltip')[0](mathNode, mathData);
assert.deepEqual(mathEvents.slice(0, 10).map(event => event[0]), [
  'firstChild', 'isKind', 'create', 'html', 'html', 'svg', 'append', 'wrap', 'firstChild', 'processMath',
]);
assert.equal(tipMml.parent, mathNode.node);
const mathHandlers = mathEvents.filter(event => event[0] === 'setEventHandler');
assert.deepEqual(mathHandlers.map(event => [event[1], event[3]]), [
  ['mouseover', mathDom], ['mouseout', mathDom],
]);
assert.equal(typeof mathHandlers[0][2], 'function');
assert.equal(typeof mathHandlers[1][2], 'function');
const queuedTimers = [];
const oldSetTimeout = global.setTimeout;
global.setTimeout = (callback, delay) => { queuedTimers.push({ callback, delay }); return queuedTimers.length; };
try {
  const mouseEvent = { stopPropagation() { mathEvents.push(['eventStop']); } };
  mathHandlers[0][2](mouseEvent);
  assert.deepEqual(mathEvents.slice(-2).map(event => event[0]), ['stopTimers', 'eventStop']);
  assert.equal(mathData.hoverTimer.get(mathDom), 1);
  assert.equal(queuedTimers[0].delay, 600);
  mathHandlers[1][2](mouseEvent);
  assert.equal(mathData.clearTimer.get(mathDom), 2);
  assert.equal(queuedTimers[1].delay, 100);
} finally {
  global.setTimeout = oldSetTimeout;
}

// Statusline writes the text attribute before registering mouseover/mouseout;
// the callbacks append/remove one cached status node and stop propagation.
const statusEvents = [];
const statusDom = { id: 'status-dom' };
const statusMml = {
  isKind(kind) { statusEvents.push(['isKind', kind]); return kind === 'mtext'; },
  getText() { statusEvents.push(['getText']); return 'status text'; },
};
const statusData = { status: null };
const statusNode = {
  childNodes: [{}, { node: statusMml }], dom: [statusDom],
  adaptor: {
    setAttribute(...args) { statusEvents.push(['setAttribute', ...args]); },
    document: { id: 'document' },
    body() { statusEvents.push(['body']); return { id: 'body' }; },
    append(...args) { statusEvents.push(['append', ...args]); return { id: 'status' }; },
    remove(node) { statusEvents.push(['remove', node]); },
  },
  text(value) { statusEvents.push(['text', value]); return { value }; },
  html(...args) { statusEvents.push(['html', ...args]); return { id: 'status-html' }; },
  setEventHandler(type, callback) { statusEvents.push(['setEventHandler', type]); this[type] = callback; },
};
SvgMaction.actions.get('statusline')[0](statusNode, statusData);
assert.deepEqual(statusEvents.slice(0, 3).map(event => event[0]), ['isKind', 'getText', 'setAttribute']);
assert.equal(statusData.text, undefined);
const statusEvent = { stopPropagation() { statusEvents.push(['stopPropagation']); } };
statusNode.mouseover(statusEvent);
assert.equal(statusData.status.id, 'status');
statusNode.mouseout(statusEvent);
assert.equal(statusData.status, null);
assert.equal(statusEvents.filter(event => event[0] === 'stopPropagation').length, 2);

const events = [];
const handler = () => {};
const target = { addEventListener(...args) { events.push(args); } };
const receiver = { dom: [target] };
SvgMaction.prototype.setEventHandler.call(receiver, 'click', handler);
assert.deepEqual(events, [['click', handler]]);
const explicitTarget = { addEventListener(...args) { events.push(args); } };
SvgMaction.prototype.setEventHandler.call(receiver, 'mouseover', handler, explicitTarget);
assert.deepEqual(events[1], ['mouseover', handler]);

const pxReceiver = { px(value) { events.push(['px', value]); return 'actual px'; } };
assert.equal(SvgMaction.prototype.Px.call(pxReceiver, 2.25), 'actual px');
assert.deepEqual(events[2], ['px', 2.25]);

const original = SvgMaction.actions;
const replacement = new Map([['custom', [handler, { marker: 7 }]]]);
SvgMaction.actions = replacement;
assert.equal(SvgMaction.actions, replacement);
assert.equal(SvgMaction.actions.get('custom')[1].marker, 7);
SvgMaction.actions = original;

const source = require('node:fs').readFileSync(file);
console.log(JSON.stringify({
  schema: 1,
  sourceSha256: crypto.createHash('sha256').update(source).digest('hex'),
  constructor: SvgMaction.name,
  actions: keys,
  styleSelectors,
  descriptors: {
    actions: Object.getOwnPropertyDescriptor(SvgMaction, 'actions'),
    setEventHandler: Object.getOwnPropertyDescriptor(SvgMaction.prototype, 'setEventHandler'),
    Px: Object.getOwnPropertyDescriptor(SvgMaction.prototype, 'Px'),
  },
  checks: ['actual-map', 'source-style-selector-order', 'three-handlers',
    'common-constructor-map-order-and-fallback',
    'event-method-receiver', 'explicit-dom', 'Px-delegation',
    'toggle-order-and-rerender', 'tooltip-text-order', 'tooltip-math-construction-and-timer-registration',
    'statusline-output-and-events', 'stop-timers-map-and-clear-order', 'map-replacement'],
}));
