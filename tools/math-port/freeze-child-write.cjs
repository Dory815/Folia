// Root expectations from the actual Source append assignment failures.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),source=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23-sparse-child-source.json')));
const q=JSON.stringify,rows=['U','N'].map(tag=>{
 const c=source.cases.find(c=>c.name==='appendChild-'+tag);assert.equal(c.error.name,'TypeError');
 assert.equal(c.old.values.at(-1),tag); // Source writes the slot before this failure.
 return `{${tag==='U'?'fm::Missing{}':'fm::PropertyNull{}'},${q(c.error.name)},u${q(c.error.message)}}`;
});
const text=`// Root-generated actual Source nullish assignment errors; do not edit.
#include "support/folia_test.hpp"
import folia.math.core.tree.child_array;
namespace fm=folia::math;namespace tr=fm::tree;using namespace boost::ut;namespace {
struct Product{};using Value=tr::ChildValue<Product>;
struct Input{Value value;std::string_view name;std::u16string_view message;};
const Input inputs[]={${rows.join(',')}};
}
suite math_port_child_write=[] {
 "math_port_child_write_actual_source_receiver_errors"_test=[] {
  for(const auto& input:inputs){bool caught=false;try{input.value.requireNodeForWrite("parent");}catch(const fm::options::Exception& e){caught=true;expect(e.name==input.name);expect(e.message==input.message);}expect(caught);}
  Product node;expect(Value(&node).requireNodeForWrite("parent")==&node);
 };
};
`;
const out=path.join(root,'tests/math/reference_child_write.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),text);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,text);}
console.log(JSON.stringify({source_cases:2,tests:1,assertions:7}));
