'use strict';
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '../../../..');
const pkg = path.join(root, 'tools/mathjax-bundle/node_modules/@mathjax/src');
const pinned = [
  ['ts/input/tex.ts', '77F300E2E6FD91851B84EB8DCB4190BCC3A87D9FC37F6BCAE0A4D6223EEEA389'],
  ['ts/input/tex/NodeFactory.ts', 'CE4BF2465993CD0F2346B64C68DD313CCC051413323442E1E11DDD5A7F1978B8'],
  ['ts/input/tex/TexError.ts', '0748AA17602C42451D97BB48488E769EDEF41AE9E193C9D3DF70D198B9BA167F'],
  ['cjs/input/tex.js', 'EF2852B197173F373E62326016E980AEF01FFC838F5F766AB6076262F944EADA'],
  ['cjs/input/tex/NodeFactory.js', '426E3CA8822D209EDC8A22B5E9F11C5631E52213B232220A2A55E81828374DB5'],
  ['cjs/input/tex/TexError.js', 'A5713111FF2AB646740DA13CA6273CB5F2A7CEFF9D0FB6A2E116185E4994E64F'],
];
for (const [relative, expected] of pinned) {
  const actual = crypto.createHash('sha256').update(fs.readFileSync(path.join(pkg, relative))).digest('hex').toUpperCase();
  assert.equal(actual, expected, `pinned source changed: ${relative}`);
}
const {TeX} = require(path.join(pkg, 'cjs/input/tex.js'));
const {MmlFactory} = require(path.join(pkg, 'cjs/core/MmlTree/MmlFactory.js'));
const {AbstractMathItem} = require(path.join(pkg, 'cjs/core/MathItem.js'));
const TexError = require(path.join(pkg, 'cjs/input/tex/TexError.js')).default;
function make(options={packages:['base']}) { const jax=new TeX(options); jax.setMmlFactory(new MmlFactory()); return jax; }
function tree(node) { return {kind:node.kind, isInferred:node.isInferred, text:node.getText?.(), attributes:node.attributes?.getAllAttributes?.(), children:(node.childNodes||[]).map(tree)}; }
function findKind(node, kind) { if (!node) return null; if (node.kind===kind) return node; for (const child of node.children||[]) { const found=findKind(child,kind); if(found)return found; } return null; }
function expectError(label, node, expected, standardErrorTree=true) {
  const error=findKind(node,'merror');
  assert.ok(error, `${label}: expected merror`);
  assert.equal(error.attributes?.['data-mjx-error'], expected, `${label}: exact data-mjx-error`);
  assert.equal(error.children.length,1,`${label}: merror child count`);
  assert.equal(error.children[0].kind,'inferredMrow',`${label}: child kind`);
  assert.equal(error.children[0].isInferred,true,`${label}: child isInferred`);
  if (standardErrorTree) {
    assert.equal(error.children[0].children.length,1,`${label}: inferred row child count`);
    assert.equal(error.children[0].children[0].kind,'mtext',`${label}: inferred row child kind`);
    assert.equal(error.children[0].children[0].text,expected,`${label}: mtext text`);
    assert.equal(error.children[0].children[0].children.length,1,`${label}: mtext leaf count`);
    assert.equal(error.children[0].children[0].children[0].kind,'text',`${label}: mtext leaf kind`);
    assert.equal(error.children[0].children[0].children[0].text,expected,`${label}: text leaf value`);
  } else {
    assert.equal(error.children[0].children.length,0,`${label}: factory-created inferred row remains empty`);
  }
}
const jax=make();
const err=new TexError('BoundaryID','first line\nignored line');
const typed=tree(jax.formatError(err));
expectError('typed-direct-formatter',typed,'first line');
const defaultCallback=tree(jax.options.formatError(jax,err));
expectError('default-options-callback',defaultCallback,'first line');
const accessOrder=[];
const accessorError={};
Object.defineProperty(accessorError,'message',{get(){accessOrder.push('message');return 'ordered line\nignored';}});
Object.defineProperty(accessorError,'id',{get(){accessOrder.push('id');return 'OrderID';}});
const orderedDirect=tree(jax.formatError(accessorError));
expectError('typed-formatError-property-order',orderedDirect,'ordered line');
assert.deepEqual(accessOrder,['message','id']);
accessOrder.length=0;
const orderedCallback=tree(jax.options.formatError(jax,accessorError));
expectError('default-callback-property-order',orderedCallback,'ordered line');
assert.deepEqual(accessOrder,['message','id']);

const borrowedOwner=make(), borrowedReceiver=make();
borrowedOwner.parseOptions.nodeFactory.set('error',(factory)=>factory.create('node','merror',[],{'data-mjx-error':'owner-factory'}));
borrowedReceiver.parseOptions.nodeFactory.set('error',(factory)=>factory.create('node','merror',[],{'data-mjx-error':'receiver-factory'}));
const borrowed=tree(TeX.prototype.formatError.call(borrowedReceiver,err));
expectError('borrowed-prototype-method-uses-call-receiver',borrowed,'receiver-factory',false);

const parserError=tree(jax.compile(new AbstractMathItem('\\notACommand',jax,true),{}));
expectError('parser-error-uses-default-callback',parserError,'Undefined control sequence \\notACommand');
const calls=[];
const custom=make({packages:['base'],formatError(receiver,error){
  calls.push({receiverIsJax:receiver===custom,id:error.id,message:error.message});
  return custom.parseOptions.nodeFactory.create('node','merror',[],{'data-mjx-error':'custom-format'});
}});
const customError=tree(custom.compile(new AbstractMathItem('\\notACommand',custom,true),{}));
expectError('parser-error-preserves-custom-formatError-result',customError,'custom-format',false);
assert.deepEqual(calls,[{receiverIsJax:true,id:'UndefinedControlSequence',message:'Undefined control sequence \\notACommand'}]);
const throwing=make({packages:['base'],formatError(){throw new Error('formatter failure');}});
assert.throws(()=>throwing.compile(new AbstractMathItem('\\notACommand',throwing,true),{}),/formatter failure/);
process.stdout.write(JSON.stringify({source:{version:'MathJax 4.1.3',commit:'fb987178c2d279a99b0db5ea02e751691703955e',sha256:Object.fromEntries(pinned.map(([file,hash])=>[file,hash]))},cases:{treeShape:'merror > inferredMrow(isInferred=true) > mtext > text; exact text asserted at mtext and leaf',typed:'pass',defaultCallback:'pass',propertyAccessOrder:'pass',borrowedReceiver:'pass',defaultParseError:'pass',customParseError:'pass',customCalls:calls,formatterErrorPropagation:'pass'}},null,2)+'\n');
