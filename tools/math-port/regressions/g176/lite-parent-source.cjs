'use strict';
const assert=require('node:assert/strict');
const {createReference}=require('../../reference-profile.cjs');
const reference=createReference();
const {adaptor}=reference;

const sceneParent=adaptor.node('g');
const pushedChild=adaptor.node('text');
sceneParent.children.push(pushedChild);
assert.equal(sceneParent.children[0],pushedChild);
assert.equal(pushedChild.parent,undefined,
  'a direct Array.push changes the slot but does not write the separate parent field');

const sparseParent=adaptor.node('g');
const first=adaptor.node('text');
const second=adaptor.node('path');
adaptor.append(sparseParent,first);
adaptor.append(sparseParent,second);
delete sparseParent.children[0];
assert.equal(sparseParent.children.length,2,
  'delete children[index] leaves the array length unchanged');
assert.equal(0 in sparseParent.children,false,
  'delete children[index] leaves an array hole');
assert.equal(sparseParent.children[1],second,
  'delete children[index] does not shift later slots');
assert.equal(first.parent,sparseParent,
  'deleting a slot does not infer or mutate the independent parent field');
const spliceParent=adaptor.node('g');
const spliceFirst=adaptor.node('text');
const spliceSecond=adaptor.node('path');
adaptor.append(spliceParent,spliceFirst);
adaptor.append(spliceParent,spliceSecond);
spliceParent.children.splice(0,1);
assert.equal(spliceParent.children.length,1,
  'Array.splice structurally removes and shifts a slot');
assert.equal(spliceParent.children[0],spliceSecond);
assert.equal(spliceFirst.parent,spliceParent,
  'splice itself does not infer or mutate the separate parent field');

const section=adaptor.node('section');
const bold=adaptor.node('b');
const text=adaptor.text('text');
const comment=adaptor.comment('comment');
adaptor.append(section,bold);
adaptor.append(section,text);
adaptor.append(section,comment);
const clone=adaptor.clone(section);
assert.equal(clone.children[0].parent,clone,
  'LiteAdaptor.clone assigns cloned element descendants to their clone parent');
assert.equal(clone.children[1].parent,undefined,
  'LiteAdaptor.clone leaves cloned text parents unset');
assert.equal(clone.children[2].parent,undefined,
  'LiteAdaptor.clone leaves cloned comment parents unset');

const before={slot:section.children[0]===bold,boldParent:bold.parent===section};
section.children[0]=comment;
const after={slot:section.children[0]===comment,boldParent:bold.parent===section,
  commentParent:comment.parent===section};
assert.deepEqual(before,{slot:true,boldParent:true});
assert.deepEqual(after,{slot:true,boldParent:true,commentParent:true});

process.stdout.write(JSON.stringify({
  provenance:reference.provenance.upstream,
  directPush:{slotIsChild:sceneParent.children[0]===pushedChild,parent:pushedChild.parent},
  sparseDelete:{length:sparseParent.children.length,slot0Present:0 in sparseParent.children,
    slot1IsSecond:sparseParent.children[1]===second,firstParentIsParent:first.parent===sparseParent},
  splice:{length:spliceParent.children.length,slot0IsSecond:spliceParent.children[0]===spliceSecond,
    removedParentIsParent:spliceFirst.parent===spliceParent},
  rawReplacement:{before,after},
  deepClone:{elementParentIsClone:clone.children[0].parent===clone,
    textParentIsUndefined:clone.children[1].parent===undefined,
    commentParentIsUndefined:clone.children[2].parent===undefined}
},null,2)+'\n');
