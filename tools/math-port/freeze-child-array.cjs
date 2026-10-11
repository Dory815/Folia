// Root generated C++ inputs/expectations from the independent actual-Array corpus.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),f=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23-child-array-source.json')));
const snap=s=>`${s.length}:${s.own.map(Number).join('')}:${s.has.map(Number).join('')}:${s.values.join('')}:${s.outside[0]}:${Number(s.outside[1])}`;
const kinds=['set','delete','resize','push','pop','slice','splice','indexOf','forEach'];
const data=f.cases.map(c=>{const o=c.operation;let result=c.result===null?'':typeof c.result==='object'?Array.isArray(c.result)?c.result.map(([v,i,s])=>`${v}@${i}:${Number(s)};`).join(''):snap(c.result):String(c.result);return `{${JSON.stringify(c.shape.join(''))},${c.prototype},${kinds.indexOf(o.kind)},${o.index??o.length??o.start??0},${o.end??0},${o.end===null},${o.count??0},${o.count===null},${JSON.stringify(o.value??'')},${JSON.stringify((o.added??[]).join(''))},${JSON.stringify(o.mutation??'')},${JSON.stringify(result)},${JSON.stringify(snap(c.after))}}`;}).join(',\n');
const source=`// Root-frozen actual Source Array observations, not native golds.
#include "support/folia_test.hpp"
import folia.math.core.tree.child_array;
namespace tr=folia::math::tree;namespace fm=folia::math;using namespace boost::ut;
namespace {
struct Product{char id;};using Value=tr::ChildValue<Product>;using Array=tr::ChildArray<Product>;using Handle=Array::Handle;
struct Input{std::string_view shape;bool proto;int kind;std::int64_t position,end;bool noEnd;std::size_t count;bool noCount;std::string_view value,added,mutation,result,after;};
const Input inputs[]={
${data}
};
struct Context{
 Product a{'a'},b{'b'},c{'c'};
 Value value(char symbol){if(symbol=='U')return fm::Missing{};if(symbol=='N')return fm::PropertyNull{};return symbol=='a'?Value(&a):symbol=='b'?Value(&b):Value(&c);}
 Handle array(std::string_view shape){auto out=std::make_shared<Array>();out->resize(shape.size());for(std::size_t i=0;i<shape.size();++i)if(shape[i]!='H')out->set(i,value(shape[i]));return out;}
};
char valueTag(Value v){return v.kind()==tr::ChildKind::Undefined?'U':v.kind()==tr::ChildKind::Null?'N':v.requireNode()->id;}
std::string snapshot(const Handle& a){std::string own,has,values;for(std::size_t i=0;i<a->size();++i){own+=a->hasOwn(i)?'1':'0';has+=a->has(i)?'1':'0';values+=valueTag(a->get(i));}return std::to_string(a->size())+":"+own+":"+has+":"+values+":"+valueTag(a->get(5))+":"+(a->has(5)?"1":"0");}
}
suite math_port_child_array=[] {
 "math_port_child_array_actual_source_operations"_test=[] {
  for(const auto& i:inputs){Context c;auto a=c.array(i.shape);if(i.proto)a->setPrototype(c.array("HbHHHc"));std::string result;
   switch(i.kind){
   case 0:a->set(static_cast<std::size_t>(i.position),c.value(i.value[0]));break;
   case 1:a->erase(static_cast<std::size_t>(i.position));break;
   case 2:a->resize(static_cast<std::size_t>(i.position));break;
   case 3:result=std::to_string(a->push_back(c.value(i.value[0])));break;
   case 4:result=valueTag(a->pop_back());break;
   case 5:result=snapshot(a->slice(i.position,i.noEnd?std::nullopt:std::optional<std::int64_t>(i.end)));break;
   case 6:{std::vector<Value> added;for(char v:i.added)added.push_back(c.value(v));result=snapshot(a->splice(i.position,i.noCount?std::nullopt:std::optional<std::size_t>(i.count),added));break;}
   case 7:{auto found=a->indexOf(c.value(i.value[0]),i.position);result=found?std::to_string(*found):"-1";break;}
   case 8:a->forEach([&](Value v,std::size_t index,Array& self){result+=std::string(1,valueTag(v))+"@"+std::to_string(index)+":"+(&self==a.get()?"1;":"0;");if(index==0){if(i.mutation=="append")a->push_back(&c.c);if(i.mutation=="delete")a->erase(1);if(i.mutation=="shrink")a->resize(1);if(i.mutation=="fill")a->set(1,&c.b);}});break;
   }
   expect(result==i.result);expect(snapshot(a)==i.after);
  }
 };
 "math_port_child_value_tag_identity_and_errors"_test=[] {
  Context c;Value undefined,missing=fm::Missing{},null=fm::PropertyNull{},pointerNull=static_cast<Product*>(nullptr),node=&c.a;
  expect(undefined==missing);expect(null==pointerNull);expect(undefined!=null);expect(node==Value(&c.a));expect(node!=Value(&c.b));expect(node.isNode());expect(!undefined.isNode());expect(!null.isNode());expect(node.requireNode()==&c.a);expect(node.nodeOrNull()==&c.a);expect(undefined.nodeOrNull()==nullptr);expect(null.nodeOrNull()==nullptr);
  for(auto v:{undefined,null}){bool caught=false;try{v.requireNode("parent");}catch(const fm::options::Exception& e){caught=true;expect(e.name=="TypeError");auto expected=v.kind()==tr::ChildKind::Null?u"Cannot read properties of null (reading 'parent')":u"Cannot read properties of undefined (reading 'parent')";expect(e.message==expected);}expect(caught);}
 };
 "math_port_child_array_prototype_cycle_and_callback_receiver_lifetime"_test=[] {
  Context c;auto a=c.array("aH"),b=c.array("Hb");a->setPrototype(b);bool caught=false;try{b->setPrototype(a);}catch(const fm::options::Exception& e){caught=e.name=="TypeError";}expect(caught);expect(!b->prototype());expect(a->prototype()==b);expect(a->get(1)==Value(&c.b));expect(!a->hasOwn(1));expect(a->has(1));a->setPrototype({});expect(a->get(1)==Value{});expect(!a->has(1));
  auto owner=c.array("ab"),captured=owner;owner=c.array("c");captured->set(1,&c.c);expect(snapshot(owner)=="1:1:1:c:U:0");expect(snapshot(captured)=="2:11:11:ac:U:0");
  std::weak_ptr<Array> lifetime=owner;int seen=0;owner->forEach([&](Value,std::size_t,Array&){owner.reset();expect(!lifetime.expired());++seen;});expect(lifetime.expired());expect(seen==1);
 };
};
`;
const out=path.join(root,'tests/math/reference_child_array.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),source);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,source);}
console.log(JSON.stringify({cases:f.cases.length,tests:3}));
