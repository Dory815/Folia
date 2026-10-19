'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(process.cwd(), 'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
const pkg = require(path.resolve(root, '../package.json'));
const pinnedCommit = 'fb987178c2d279a99b0db5ea02e751691703955e';
assert.equal(pkg.version, '4.1.3');
const hashes = {
  'core/MmlTree/MML.js': '380332B7F0CC13104130AFFB65532EBCA2AC214CCD897B93EDDA551CEA443684',
  'core/MmlTree/MmlNode.js': '86C5EB572DCF2652C3C52A60B66C720D231E17F600CC9AEEF3ACE4AEAFC81D45',
  'core/MmlTree/MmlNodes/mi.js': '47058D7DDB5B24ED530936BAFFE834A06C0557EC7DBDC03312D3FFA0ACA17241',
  'core/MmlTree/Attributes.js': 'EFA567C0C573F13F136F1D37B0AF6BDC1A675F70CC13A9AE1CD7334321F30749',
  'core/Tree/Node.js': '94174004B89F19BF3C98E8EEC5FABDD0BCBF1E13E0B740963FA69554AD8A12A7',
};
for (const [file, expected] of Object.entries(hashes)) {
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(root,file))).digest('hex').toUpperCase();
  assert.equal(actual, expected, `${file} differs from pinned Source`);
}
const MML = require(path.join(root,'core/MmlTree/MML.js')).MML;
const events=[];
const foreign={};
const getNodeClass=function(kind){events.push(`call:${kind}`);return MML[kind];};
const create=function(kind,...args){events.push(`call:create:${kind}`);return new MML[kind](foreign,...args);};
Object.defineProperty(foreign,'getNodeClass',{configurable:true,enumerable:true,get(){events.push('get:getNodeClass');return getNodeClass;}});
Object.defineProperty(foreign,'create',{configurable:true,enumerable:true,get(){events.push('get:create');return create;}});
const attributes={mathvariant:'bold'}, children=[];
const created=new MML.mi(foreign,attributes,children);
assert.equal(created.kind,'mi');
assert.strictEqual(created.factory,foreign);
assert.equal(created.attributes.get('mathvariant'),'bold');
assert.deepEqual(events,['get:getNodeClass','call:mi','get:getNodeClass','call:math']);

const defaultMiPrototype=MML.mi.prototype;
// Replacing .prototype with an empty object really removes inherited Source
// methods. A successful construction is invalid; Source throws at setChildren.
MML.mi.prototype={};
assert.throws(()=>new MML.mi(foreign,attributes,children),TypeError);
MML.mi.prototype=defaultMiPrototype;

// A custom replacement prototype is valid when it retains the class methods.
const replacementPrototype=Object.create(defaultMiPrototype);
MML.mi.prototype=replacementPrototype;
const replacementNode=new MML.mi(foreign,attributes,children);
assert.strictEqual(Object.getPrototypeOf(replacementNode),replacementPrototype);
assert.equal(replacementNode.kind,'mi');
assert.equal(replacementNode.attributes.get('mathvariant'),'bold');
MML.mi.prototype=defaultMiPrototype;

// A non-object .prototype falls back to Object.prototype. The fixture installs
// the two members needed by this receiver before asking Source to construct it.
const oldSetChildren=Object.getOwnPropertyDescriptor(Object.prototype,'setChildren');
const oldKind=Object.getOwnPropertyDescriptor(Object.prototype,'kind');
try {
  Object.defineProperty(Object.prototype,'setChildren',{configurable:true,writable:true,value:function(){}});
  Object.defineProperty(Object.prototype,'kind',{configurable:true,writable:true,value:'mi'});
  MML.mi.prototype=17;
  const fallbackNode=new MML.mi(foreign,attributes,children);
  assert.strictEqual(Object.getPrototypeOf(fallbackNode),Object.prototype);
  assert.equal(fallbackNode.kind,'mi');
  assert.equal(fallbackNode.attributes.get('mathvariant'),'bold');
} finally {
  MML.mi.prototype=defaultMiPrototype;
  if(oldSetChildren)Object.defineProperty(Object.prototype,'setChildren',oldSetChildren);else delete Object.prototype.setChildren;
  if(oldKind)Object.defineProperty(Object.prototype,'kind',oldKind);else delete Object.prototype.kind;
}

events.length=0;
const suppliedPrototype=Object.create(defaultMiPrototype);
const decoyFactory={};
Object.defineProperty(decoyFactory,'getNodeClass',{configurable:true,get(){events.push('decoy:get:getNodeClass');return function(kind){events.push(`decoy:call:${kind}`);return MML[kind];};}});
let assignedFactory;
Object.defineProperty(suppliedPrototype,'factory',{configurable:true,get(){return decoyFactory;},set(value){assignedFactory=value;}});
const receiver=Object.create(suppliedPrototype);
const callResult=MML.mi.call(receiver,foreign,attributes,children);
assert.strictEqual(callResult,receiver);
assert.strictEqual(assignedFactory,foreign);
assert.strictEqual(receiver.factory,decoyFactory);
assert.equal(receiver.kind,'mi');
assert.equal(receiver.attributes.get('mathvariant'),'bold');
assert.deepEqual(events,['get:getNodeClass','call:mi','get:getNodeClass','call:math']);
const callEvents=[...events];

events.length=0;
const failingFactory={create};
failingFactory.getNodeClass=function(kind){
  if(kind==='mi')throw new TypeError('lookup stopped');
  return MML.math;
};
const failedReceiver=Object.create(defaultMiPrototype);
assert.throws(()=>MML.mi.call(failedReceiver,failingFactory,{},[]),/lookup stopped/);
assert.equal(Object.hasOwn(failedReceiver,'factory'),true);
assert.equal(Object.hasOwn(failedReceiver,'prevClass'),true);
assert.equal(Object.hasOwn(failedReceiver,'childNodes'),true);
assert.equal(Object.hasOwn(failedReceiver,'attributes'),false);

console.log(JSON.stringify({version:pkg.version,pinnedCommit,sourceHashes:hashes,
  ordinaryNew:{kind:created.kind,rawFactoryRetained:created.factory===foreign,mathvariant:created.attributes.get('mathvariant'),events:['get:getNodeClass','call:mi','get:getNodeClass','call:math']},
  emptyReplacementPrototype:{throwsAtMissingSetChildren:true},
  inheritedReplacementPrototype:{prototypeRetained:Object.getPrototypeOf(replacementNode)===replacementPrototype,kind:replacementNode.kind},
  primitivePrototypeFallback:{prototypeIsObjectPrototype:true,kind:'mi'},
  ordinaryCall:{returnsReceiver:callResult===receiver,setterReceivedForeignFactory:assignedFactory===foreign,getterStillReturnsDecoy:receiver.factory===decoyFactory,mathvariant:receiver.attributes.get('mathvariant'),events:callEvents},
  failingLookup:{throwsBeforeAttributes:true,ownFields:['factory','prevClass','childNodes'],attributesOwn:false}
},null,2));


