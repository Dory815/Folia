// Root binding migration of emitted C++ observations only. JS Source and golds
// are never translated or changed by this helper.
'use strict';
const assert=require('node:assert/strict');
module.exports=function bind(text){
 function edit(from,to){assert.ok(text.includes(from),'Missing native observer binding: '+from);text=text.replaceAll(from,to);}
 edit('Probe* appendChild(Probe* c)override','tr::ChildValue<Probe> appendChild(tr::ChildValue<Probe> c)override');
 edit('auto* result=A::appendChild(c);if(onAppend)onAppend(c);','auto result=A::appendChild(c);if(onAppend)onAppend(c.requireNode("onAppend"));');
 edit('return std::make_shared<std::vector<Probe*>>(c);','auto a=std::make_shared<tr::ChildArray<Probe>>();for(auto* p:c)a->push_back(p);return a;');
 edit('std::make_shared<std::vector<Wrapped*>>','std::make_shared<tr::ChildArray<Wrapped>>');
 edit('std::initializer_list<Wrapped*>','std::initializer_list<tr::ChildValue<Wrapped>>');
 edit('auto* replaced=p->replaceChild(b,a);','auto replaced=p->replaceChild(b,a);');
 edit('->parent','->getParent()');
 edit('p->childNodes->front()','p->childNodes->get(0)');
 edit('q->childNodes->back()','q->childNodes->get(q->childNodes->size()-1)');
 for(const i of [0,1])edit(`(*copied->childNodes)[${i}]`,`copied->childNodes->get(${i})`);
 edit('get(0)->getParent()','get(0).requireNode("parent")->getParent()');
 edit('get(1)->getParent()','get(1).requireNode("parent")->getParent()');
 edit('get(1)->toString()','get(1).requireNode("toString")->toString()');
 edit('get(1)->kind()','get(1).requireNode("kind")->kind()');
 assert.doesNotMatch(text,/vector<(?:Probe|Wrapped)\*>|->parent|\(\*\w+->childNodes\)\[|childNodes->(?:front|back)/);
 return text;
};
