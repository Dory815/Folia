// Root one-time concrete declaration/red freeze; never a runtime compatibility path.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),file=path.join(root,'src/math/core/tree/node.ixx');
let source=fs.readFileSync(file,'utf8').replaceAll('\r\n','\n');
assert.ok(!source.includes('P23c frozen stub'),'Do not erase a delivered implementation');
source=source.replaceAll('std::make_shared<std::vector<Product*>>()','std::make_shared<ChildArray<Product>>()')
 .replaceAll('Product* appendChild(Product* child)','ChildValue<Product> appendChild(ChildValue<Product> child)')
 .replaceAll('Product* replaceChild(Product* newChild, Product* oldChild)','ChildValue<Product> replaceChild(ChildValue<Product> newChild, ChildValue<Product> oldChild)')
 .replaceAll('Product* removeChild(Product* child)','ChildValue<Product> removeChild(ChildValue<Product> child)')
 .replaceAll('childIndex(Product* child)','childIndex(ChildValue<Product> child)')
 .replaceAll('Product* replaceChild(Product*, Product* oldChild)','ChildValue<Product> replaceChild(ChildValue<Product>, ChildValue<Product> oldChild)')
 .replaceAll('childIndex(Product*)','childIndex(ChildValue<Product>)');
// Root-known method definition anchors; skip quoted braces while finding the body.
for(const anchor of ['void setChildren(FactoryChildren<Product> children)','ChildValue<Product> appendChild(ChildValue<Product> child)','ChildValue<Product> replaceChild(ChildValue<Product> newChild, ChildValue<Product> oldChild)','ChildValue<Product> removeChild(ChildValue<Product> child)','std::optional<std::size_t> childIndex(ChildValue<Product> child)','Product* copy(bool keepIds = false)','std::any walkTree(const TreeVisitor<Product>& visit','std::string toString() const']){
 const at=source.indexOf(anchor);assert.ok(at>=0,anchor);const marker=' override {';const body=source.indexOf(marker,at);assert.ok(body>=0,anchor);const start=body+marker.length-1;let depth=1,end=start+1,quote='';
 for(;depth&&end<source.length;++end){const c=source[end];if(quote){if(c==='\\'){++end;continue;}if(c===quote)quote='';continue;}if(c==='"'||c==="'"){quote=c;continue;}if(c==='{')++depth;else if(c==='}')--depth;}
 assert.equal(depth,0);source=source.slice(0,start)+'{\n        // P23c frozen stub: '+anchor.split('(')[0]+'.\n        throw FactoryError("Unimplemented",u"Root-frozen sparse Node operation");\n    }'+source.slice(end);
}
fs.writeFileSync(file,source);console.log('Root sparse Node declarations and real red stubs written.');
