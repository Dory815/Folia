'use strict';

// Source-side capability shim: register only; never dispatch, schedule, or rerender.
let currentRequest = null;
let nextTargetId = 1;
const targetIds = new WeakMap();

function install(adaptor) {
  const elementPrototype = Object.getPrototypeOf(adaptor.node('svg', {}));
  elementPrototype.addEventListener = function (type, handler) {
    if (!currentRequest) throw new Error('passive event registration outside request scope');
    if (typeof type !== 'string' || typeof handler !== 'function') {
      throw new TypeError('passive host requires an event name and callback');
    }
    if (!targetIds.has(this)) targetIds.set(this, nextTargetId++);
    const record = { targetId: targetIds.get(this), target: this, type, handler };
    currentRequest.records.push(record);
    currentRequest.targets.add(this);
  };
}

function beginRequest() {
  if (currentRequest) throw new Error('passive host request scopes must not overlap');
  const request = { records: [], targets: new Set() };
  currentRequest = request;
  let retired = false;
  return {
    snapshot() {
      if (retired) throw new Error('passive host request already retired');
      return request.records.map(({ targetId, type }) => ({ targetId, type }));
    },
    assertCallbacksRetained() {
      if (retired) throw new Error('passive host request already retired');
      if (request.records.some(record => typeof record.handler !== 'function')) {
        throw new Error('original Source callback identity was not retained');
      }
    },
    retire() {
      if (retired) return;
      retired = true;
      request.records.length = 0;
      request.targets.clear();
      if (currentRequest === request) currentRequest = null;
    },
  };
}

module.exports = { install, beginRequest };
