'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const root = process.cwd();
const factoryPath = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/Tree/Factory.js');
const wrapperPath = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/core/Tree/WrapperFactory.js');
const commonFactoryPath = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs/output/common/WrapperFactory.js');
const sha = p => crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex');
assert.equal(sha(factoryPath), '739db2e5760c29d7bb4f1bc7baa08ff8d727f4d117522aff5ede017f812cffef');
assert.equal(sha(wrapperPath), 'bbeb7c85e0dfd3c8e437d500d37f083a6b031cd9490b58d41c3285e8f7f679d5');
assert.equal(sha(commonFactoryPath), '57ce2e232ec1919fa518699226466ceef52cbcb2af75559137d2bb2dc83ea2ab');
const { AbstractFactory } = require(factoryPath);
const { AbstractWrapperFactory } = require(wrapperPath);
const { CommonWrapperFactory } = require(commonFactoryPath);

class Product {
  constructor(factory, ...args) { this.factory = factory; this.args = args; }
}
class DerivedFactory extends AbstractFactory {}
DerivedFactory.defaultNodes = { defaulted: Product };
const defaulted = new DerivedFactory(null);
assert.deepEqual(defaulted.getKinds(), ['defaulted']);
assert.equal(defaulted.getNodeClass('defaulted'), Product);
assert.equal(defaulted.nodeIsKind(new Product(defaulted), 'defaulted'), true);
const deleteProbe = new DerivedFactory({ temporary: Product });
deleteProbe.deleteNodeClass('temporary');
assert.equal(deleteProbe.getNodeClass('temporary'), undefined);
assert.deepEqual(deleteProbe.getKinds(), []);
const product = defaulted.create('defaulted', 'x', 7);
assert(product instanceof Product);
assert.equal(product.factory, defaulted);
assert.deepEqual(product.args, ['x', 7]);

const existing = Object.create(DerivedFactory.prototype);
existing.constructor = DerivedFactory;
const callResult = AbstractFactory.call(existing, { explicit: Product });
assert.equal(callResult, undefined);
assert.equal(existing.getNodeClass('explicit'), Product);
assert.deepEqual(existing.getKinds(), ['explicit']);
assert.equal(existing.defaultKind, 'unknown');
assert.equal(existing.node instanceof Object, true);
const explicitKinds = existing.getKinds();
const baseThrowingConstructor = Object.create(AbstractFactory.prototype);
let baseConstructorReads = 0;
Object.defineProperty(baseThrowingConstructor, 'constructor', { configurable: true, get() { baseConstructorReads++; throw new Error('constructor getter must not run'); } });
const baseThrowingResult = AbstractFactory.call(baseThrowingConstructor, { explicit: Product });
assert.equal(baseConstructorReads, 0);
assert.equal(baseThrowingResult, undefined);
assert.equal(baseThrowingConstructor.getNodeClass('explicit'), Product);
const defaultExisting = Object.create(DerivedFactory.prototype);
const defaultCallResult = AbstractFactory.call(defaultExisting, null);
assert.equal(defaultCallResult, undefined);
assert.equal(defaultExisting.constructor, DerivedFactory);
assert.deepEqual(defaultExisting.getKinds(), ['defaulted']);
assert.equal(defaultExisting.getNodeClass('defaulted'), Product);

const order = [];
const firstMap = {
  set(k, v) { order.push(['first.set', k, v === Product]); mapCurrent = secondMap; },
  get(k) { order.push(['first.get', k]); return Product; },
};
const secondMap = {
  set(k) { order.push(['second.set', k]); },
  get(k) { order.push(['second.get', k]); return Product; },
};
let mapCurrent = firstMap;
Object.defineProperty(existing, 'nodeMap', { configurable: true, get() { order.push(['nodeMap.get']); return mapCurrent; } });
let constructorCalls = 0;
class CountedProduct { constructor(factory, ...args) { constructorCalls++; this.factory=factory; this.args=args; } }
// Capture the actual Source sequence independently of C++: setNodeClass reads
// nodeMap for set, reads it again for get, and the generated creator delays new.
const delayed = Object.create(AbstractFactory.prototype);
delayed.defaultKind='unknown'; delayed.nodeMap=firstMap; delayed.node={};
// Use a real class to observe that KIND is captured during registration and
// construction occurs only when the generated node function is called.
class Deferred { constructor(factory, ...args) { constructorCalls++; this.factory=factory; this.args=args; } }
const setOrder = [];
const mapB = { set(k,v){setOrder.push('B.set');}, get(k){setOrder.push('B.get');return Deferred;} };
const mapA = { set(k,v){setOrder.push('A.set'); mapForSet=mapB;}, get(k){setOrder.push('A.get');return Deferred;} };
let mapForSet=mapA;
Object.defineProperty(delayed, 'nodeMap', {configurable:true,get(){setOrder.push('nodeMap');return mapForSet;}});
delayed.setNodeClass('later', Deferred);
assert.deepEqual(setOrder, ['nodeMap','A.set','nodeMap','B.get']);
assert.equal(constructorCalls, 0);
const deferredProduct=delayed.node.later('arg');
assert.equal(constructorCalls, 1);
assert.equal(deferredProduct.factory, delayed);
assert.deepEqual(deferredProduct.args, ['arg']);

const wrapper = Object.create(AbstractWrapperFactory.prototype);
wrapper.defaultKind='unknown'; wrapper.nodeMap=new Map(); wrapper.node={};
let wrapCalls;
wrapper.create=function(...args){ wrapCalls={receiver:this,args}; return 'wrapped'; };
const wrapNode={kind:'leaf'};
assert.equal(wrapper.wrap(wrapNode,'extra',3),'wrapped');
assert.equal(wrapCalls.receiver,wrapper);
assert.deepEqual(wrapCalls.args,[wrapNode.kind,wrapNode,'extra',3]);
class DerivedWrapper extends AbstractWrapperFactory {}
DerivedWrapper.defaultNodes = { wrappedDefault: Product };
const wrapperExisting=Object.create(AbstractWrapperFactory.prototype);
wrapperExisting.constructor=DerivedWrapper;
const wrapperCallResult=AbstractWrapperFactory.call(wrapperExisting,{w:Product});
const wrapperThrowingConstructor = Object.create(AbstractWrapperFactory.prototype);
let wrapperConstructorReads = 0;
Object.defineProperty(wrapperThrowingConstructor, 'constructor', { configurable: true, get() { wrapperConstructorReads++; throw new Error('wrapper constructor getter must not run'); } });
const wrapperThrowingResult = AbstractWrapperFactory.call(wrapperThrowingConstructor, { w: Product });
assert.equal(wrapperConstructorReads, 0);
assert.equal(wrapperThrowingResult, wrapperThrowingConstructor);
assert.equal(wrapperThrowingConstructor.getNodeClass('w'), Product);
assert.equal(wrapperCallResult,wrapperExisting);
assert.equal(wrapperExisting.getNodeClass('w'),Product);

const wrapperDefaultExisting=Object.create(DerivedWrapper.prototype);
wrapperDefaultExisting.constructor=DerivedWrapper;
const wrapperDefaultCallResult=AbstractWrapperFactory.call(wrapperDefaultExisting,null);
assert.equal(wrapperDefaultCallResult,wrapperDefaultExisting);
assert.deepEqual(wrapperDefaultExisting.getKinds(),['wrappedDefault']);
assert.equal(wrapperDefaultExisting.getNodeClass('wrappedDefault'),Product);

const commonFactory = new CommonWrapperFactory({ product: Product });
const commonProduct = new Product(commonFactory);
const commonProductIsKind = commonFactory.nodeIsKind(commonProduct, 'product');
const commonPlainIsKind = commonFactory.nodeIsKind({}, 'product');
commonFactory.nodeMap.set('invalid', {});
let commonInvalidRhsError = '';
try { commonFactory.nodeIsKind(commonProduct, 'invalid'); }
catch (error) { commonInvalidRhsError = `${error.name}: ${error.message}`; }
const prototypeChain=[];
let proto=AbstractWrapperFactory.prototype;
while(proto){prototypeChain.push(proto.constructor?.name||null);proto=Object.getPrototypeOf(proto);}
const descriptors={
  factoryCtorLength:AbstractFactory.length,
  wrapperCtorLength:AbstractWrapperFactory.length,
  createLength:AbstractFactory.prototype.create.length,
  setNodeClassLength:AbstractFactory.prototype.setNodeClass.length,
  getNodeClassLength:AbstractFactory.prototype.getNodeClass.length,
  deleteNodeClassLength:AbstractFactory.prototype.deleteNodeClass.length,
  nodeIsKindLength:AbstractFactory.prototype.nodeIsKind.length,
  getKindsLength:AbstractFactory.prototype.getKinds.length,
  wrapLength:AbstractWrapperFactory.prototype.wrap.length,
  factoryDefaultNodesOwn:Object.prototype.hasOwnProperty.call(AbstractFactory,'defaultNodes'),
  defaultNodes:Object.keys(AbstractFactory.defaultNodes),
  defaultNodesDescriptor:Object.getOwnPropertyDescriptor(AbstractFactory,'defaultNodes'),
  factoryPrototypeParent:Object.getPrototypeOf(AbstractFactory.prototype)===Object.prototype,
  wrapperProtoExtendsFactory:Object.getPrototypeOf(AbstractWrapperFactory.prototype)===AbstractFactory.prototype,
  wrapperConstructorParent:Object.getPrototypeOf(AbstractWrapperFactory)===AbstractFactory,
};
const result={
  provenance:{factorySha256:sha(factoryPath),wrapperFactorySha256:sha(wrapperPath),commonFactorySha256:sha(commonFactoryPath)},
  sourceApi:descriptors,
  defaultNodes:{kinds:defaulted.getKinds(),created:product instanceof Product,args:product.args},
  factoryCallExisting:{returnsUndefined:callResult===undefined,sameReceiver:existing.getNodeClass('explicit')===Product,kinds:explicitKinds},
  explicitNodesAvoidConstructorGetter:{baseReads:baseConstructorReads,baseReturnsUndefined:baseThrowingResult===undefined,wrapperReads:wrapperConstructorReads,wrapperReturnsSameReceiver:wrapperThrowingResult===wrapperThrowingConstructor},
  currentConstructorDefaults:{returnsUndefined:defaultCallResult===undefined,constructorIsDerived:defaultExisting.constructor===DerivedFactory,kinds:defaultExisting.getKinds()},
  setNodeClassOrder:setOrder,
  deferredCreation:{beforeInvoke:0,afterInvoke:constructorCalls,factoryReceiver:deferredProduct.factory===delayed,args:deferredProduct.args},
  wrapperCallExisting:{returnsSameReceiver:wrapperCallResult===wrapperExisting,kind:wrapperExisting.getKinds()},
  wrapperCurrentConstructorDefaults:{returnsSameReceiver:wrapperDefaultCallResult===wrapperDefaultExisting,constructorIsDerived:wrapperDefaultExisting.constructor===DerivedWrapper,kinds:wrapperDefaultExisting.getKinds()},
  wrap:{result:'wrapped',receiverIsFactory:wrapCalls.receiver===wrapper,args:wrapCalls.args},
  commonFactoryInstanceof:{product:commonProductIsKind,plainObject:commonPlainIsKind,invalidRhs:commonInvalidRhsError},
  prototypeChain:prototypeChain,
};
console.log(JSON.stringify(result,null,2));










