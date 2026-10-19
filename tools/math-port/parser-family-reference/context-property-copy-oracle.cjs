'use strict';
// Original ECMAScript Object.assign observations for the explicit native
// ContextPropertyBindings shallow-copy boundary. This is not a replacement
// parser or a MathJax algorithm oracle.
function observe() {
  const proto = { inherited: 7 };
  const source = Object.create(proto);
  Object.defineProperty(source, 'hidden', { value: 8, enumerable: false });
  source['2'] = 2; source['1'] = 1; source.child = {};
  let getterReceiver = false, getterCalls = 0;
  Object.defineProperty(source, 'first', { enumerable: true, get() {
    ++getterCalls; getterReceiver = this === source;
    delete source.gone; source.added = 5; source.updated = 9;
    return 6;
  }});
  source.gone = 3; source.updated = 4;
  const copy = Object.assign({}, source);
  const protoSource = {};
  Object.defineProperty(protoSource, '__proto__', { value: proto, enumerable: true });
  const protoCopy = Object.assign({}, protoSource);
  return {
    keys: Object.keys(copy),
    values: [copy['1'], copy['2'], copy.first, copy.updated],
    ownInherited: Object.hasOwn(copy, 'inherited'),
    ownHidden: Object.hasOwn(copy, 'hidden'),
    ownGone: Object.hasOwn(copy, 'gone'),
    ownAdded: Object.hasOwn(copy, 'added'),
    getterCalls, getterReceiver,
    nestedIdentity: copy.child === source.child,
    protoSetter: Object.getPrototypeOf(protoCopy) === proto && !Object.hasOwn(protoCopy, '__proto__')
  };
}
const result = observe();
if (require.main === module) process.stdout.write(JSON.stringify(result) + '\n');
module.exports = { observe };
