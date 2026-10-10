// Root-owned operation fixtures; native implementations never supply expected data.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),source='ts/util/Options.ts',hash='b810ffc09864eb8ed9295d829b229194d7e38aaac6c8f01525bc1a4b190a61e3';
const bytes=fs.readFileSync(path.join(root,'.reference/MathJax-src',source));assert.equal(crypto.createHash('sha256').update(bytes).digest('hex'),hash);
const npmRoot=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');assert.equal(require(path.join(npmRoot,'package.json')).version,'4.1.3');
assert.equal(fs.readFileSync(path.join(npmRoot,source),'utf8').replaceAll('\r\n','\n'),bytes.toString('utf8').replaceAll('\r\n','\n'));
const O=require(path.join(npmRoot,'cjs/util/Options.js'));
const u=s=>'std::u16string{'+Array.from({length:s.length},(_,i)=>'char16_t(0x'+s.charCodeAt(i).toString(16)+')').join(',')+'}';
const n=x=>{const b=Buffer.alloc(8);b.writeDoubleLE(x);return 'number(0x'+b.readBigUInt64LE().toString(16)+'ULL)';};
const out=['// Frozen by root from pinned Options.js. Do not hand-edit.', '#include "support/folia_test.hpp"',
'import folia.math.util.options;','namespace op=folia::math::options;','using namespace boost::ut;',
'namespace {','double number(std::uint64_t bits) { return std::bit_cast<double>(bits); }',
'bool same(const op::Value& a,const op::Value& b) { return op::sameValue(a,b); }',
'struct DiagnosticsGuard { op::Diagnostics old=op::OPTIONS; ~DiagnosticsGuard(){op::OPTIONS=std::move(old);} };','}',
'suite math_port_options_reference_tests=[] {'];
let count=0,seq=0,cases=0;
class Case {
  constructor(name) {
    cases++;this.map=new Map();this.warnings=[];this.oldWarn=console.warn;this.oldMode=O.OPTIONS.invalidOption;this.oldError=O.OPTIONS.optionError;
    O.OPTIONS.invalidOption='warn';console.warn=m=>this.warnings.push(m);
    out.push('"math_port_options_'+name+'"_test=[] {','op::Context c; DiagnosticsGuard diagnostics_guard;',
      'std::vector<std::u16string> warnings; op::OPTIONS.invalidOption=op::InvalidOption::warn;',
      'op::OPTIONS.warningSink=[&](std::u16string message){warnings.push_back(std::move(message));};');
    for(const [obj,name]of [[Object.prototype,'objectPrototype'],[Function.prototype,'functionPrototype'],[Array.prototype,'arrayPrototype'],[O.Expandable.prototype,'expandablePrototype'],[Number.prototype,'numberPrototype'],[Boolean.prototype,'booleanPrototype'],[String.prototype,'stringPrototype'],[Symbol.prototype,'symbolPrototype'],[Object,'objectConstructor'],[Function,'functionConstructor'],[Array,'arrayConstructor'],[O.Expandable,'expandableConstructor'],[Number,'numberConstructor'],[Boolean,'booleanConstructor'],[String,'stringConstructor'],[Symbol,'symbolConstructor']])this.map.set(obj,'c.intrinsics().'+name);
  }
  bind(name,value,expr) {out.push(`auto ${name}=${expr};`);this.map.set(value,name);return value;}
  object(name,value,expr='c.object()') {return this.bind(name,value,expr);}
  value(x) {if(x===undefined)return'op::Value{}';if(x===null)return'op::Value(op::Null{})';if(typeof x==='boolean')return`op::Value(${x})`;if(typeof x==='number')return`op::Value(${n(x)})`;if(typeof x==='string')return`op::Value(${u(x)})`;assert.ok(this.map.has(x),'Unbound identity');return'op::Value('+this.map.get(x)+')';}
  key(x) {return typeof x==='string'?'op::Key('+u(x)+')':'op::Key('+this.map.get(x)+')';}
  set(obj,key,value) {obj[key]=value;out.push(`c.set(${this.value(obj)},${this.key(key)},${this.value(value)});`);}
  define(obj,key,d) {Object.defineProperty(obj,key,d);const desc=Object.getOwnPropertyDescriptor(obj,key);out.push(`c.define(${this.map.get(obj)},${this.key(key)},${this.desc(desc)});`);}
  desc(d) {return 'value' in d?`op::Descriptor::data(${this.value(d.value)},${d.writable},${d.enumerable},${d.configurable})`:`op::Descriptor::access(${d.get?this.map.get(d.get):'std::nullopt'},${d.set?this.map.get(d.set):'std::nullopt'},${d.enumerable},${d.configurable})`;}
  expect(value,actual) {if(value!==null&&(typeof value==='object'||typeof value==='function')&&!this.map.has(value)){const name='new_object_'+(++seq),v=name+'_value';out.push(`auto ${v}=${actual};expect(std::holds_alternative<op::ObjectRef>(${v}.data));if(!std::holds_alternative<op::ObjectRef>(${v}.data)) return;auto ${name}=std::get<op::ObjectRef>(${v}.data);`);count++;this.map.set(value,name);this.snapshot(value,name);return;}out.push(`expect(same(${actual},${this.value(value)}));`);count++;}
  snapshot(obj,actual=this.map.get(obj)) {
    assert.ok(actual);const kind=Array.isArray(obj)?'array':typeof obj==='function'?'function':'object';
    // Instances are identity leaves; ordinary newly copied objects are traversed.
    out.push(`expect(bool(c.kind(${actual})==op::ObjectKind::${kind}));`);count++;
    const proto=Object.getPrototypeOf(obj);assert.ok(proto===null||this.map.has(proto),'Unbound prototype');
    out.push(proto===null?`expect(!c.prototype(${actual}).has_value());`:`expect(bool(c.prototype(${actual})==std::optional<op::ObjectRef>(${this.map.get(proto)})));`);count++;
    const keys=Reflect.ownKeys(obj);out.push(`expect(bool(c.ownKeys(op::Value(${actual}))==std::vector<op::Key>{${keys.map(k=>this.key(k)).join(',')}}));`);count++;
    for(const key of keys){const d=Object.getOwnPropertyDescriptor(obj,key),name='descriptor_'+(++seq);out.push(`auto ${name}=c.descriptor(op::Value(${actual}),${this.key(key)}); expect(${name}.has_value());if(!${name}) return;`);count++;
      out.push(`expect(bool(${name}->accessor==${!('value'in d)} && ${name}->enumerable==${d.enumerable} && ${name}->configurable==${d.configurable}));`);count++;
      if('value'in d){out.push(`expect(bool(${name}->writable==${d.writable}));`);count++;this.expect(d.value,`${name}->value`);}
      else{out.push(`expect(bool(${name}->getter==${d.get?'std::optional<op::ObjectRef>('+this.map.get(d.get)+')':'std::nullopt'})); expect(bool(${name}->setter==${d.set?'std::optional<op::ObjectRef>('+this.map.get(d.set)+')':'std::nullopt'}));`);count+=2;}
    }
  }
  operation(fn,cpp) {const actual=fn(),name='result_'+(++seq);out.push(`auto ${name}=${cpp};`);this.expect(actual,name);return actual;}
  throws(fn,cpp,exact=false) {let error;try{fn();}catch(e){error=e;}assert.ok(error,'Expected reference exception');const name='caught_'+(++seq);out.push(`bool ${name}=false; try { ${cpp}; } catch(const op::Exception& error) { ${name}=true; expect(bool(error.name==${JSON.stringify(error.name)})); ${exact?`expect(bool(error.message==${u(error.message)}));`:''} } expect(${name});`);count+=exact?3:2;}
  finish() {out.push(`expect(bool(warnings==std::vector<std::u16string>{${this.warnings.map(u).join(',')}}));`,'};');count++;console.warn=this.oldWarn;O.OPTIONS.invalidOption=this.oldMode;O.OPTIONS.optionError=this.oldError;}
}
{
const t=new Case('classification');const a=t.object('a',{}),z=t.object('z',Object.create(null),'c.nullObject()'),e=t.object('e',O.expandable({}),'c.objectWithPrototype(c.intrinsics().expandablePrototype)'),arr=t.object('arr',[],'c.array()');
const fn=t.bind('fn',()=>{},'c.function(u"opaque")');class Item{};t.bind('ctor',Item,'c.constructor(u"Item")');const inst=t.bind('inst',new Item(),'c.instance(ctor)');
for(const x of [undefined,null,false,0,'',Symbol.iterator]){if(typeof x==='symbol')t.bind('sym',x,'c.symbol(u"Symbol.iterator")');t.operation(()=>O.isObject(x),`op::Value(op::isObject(c,${t.value(x)}))`);}
for(const x of [a,z,e,arr,fn,inst])t.operation(()=>O.isObject(x),`op::Value(op::isObject(c,${t.value(x)}))`);
t.set(z,'constructor',Object);t.operation(()=>O.isObject(z),'op::Value(op::isObject(c,op::Value(z)))');t.set(a,'constructor',123);t.operation(()=>O.isObject(a),'op::Value(op::isObject(c,op::Value(a)))');t.finish();
}
{
const t=new Case('keys_and_primitive_boxing'),a=t.object('a',{});for(const[k,v]of [['b',1],['2',2],['a',3],['01',4],['4294967295',5],['0',6]])t.set(a,k,v);
const s=t.bind('s',Symbol('same'),'c.symbol(u"same")'),s2=t.bind('s2',Symbol('same'),'c.symbol(u"same")');t.define(a,s,{value:7,enumerable:true});t.define(a,s2,{value:8,enumerable:false});t.define(a,'hidden',{value:9});
for(const x of [a,undefined,null,false,0,NaN,'','𝔸a']){out.push(`expect(bool(op::keys(c,${t.value(x)})==std::vector<op::Key>{${O.keys(x).map(k=>t.key(k)).join(',')}}));`);count++;}
const dst=t.object('dst',{});t.operation(()=>O.insert(dst,a,false),'op::insert(c,op::Value(dst),op::Value(a),false)');t.snapshot(dst);t.operation(()=>O.copy('𝔸a'),`op::copy(c,${t.value('𝔸a')})`);t.throws(()=>O.copy(null),'(void)op::copy(c,op::Null{})');t.throws(()=>O.copy(undefined),'(void)op::copy(c,op::Value{})');t.finish();
}
{
const t=new Case('descriptors_and_copy');let gets=0,sets=0;out.push('int gets=0,sets=0;');const g=t.bind('g',()=>{gets++;return 42;},'c.function(u"get",[&](op::Context&,op::Value,std::span<const op::Value>){++gets;return op::Value(42);})');
const s=t.bind('s',v=>{sets++;},'c.function(u"set",[&](op::Context&,op::Value,std::span<const op::Value>){++sets;return op::Value{};})');
const a=t.object('a',{});t.define(a,'access',{get:g,set:s,enumerable:true,configurable:true});t.define(a,'frozen',{value:1,writable:false,enumerable:true,configurable:false});t.define(a,'hidden',{value:2});
const result=t.operation(()=>O.copy(a),'op::copy(c,op::Value(a))');out.push(`expect(bool(gets==${gets} && sets==${sets}));`);count++;
t.operation(()=>result.access,`c.get(${t.value(result)},op::Key(u"access"))`);out.push(`expect(bool(gets==${gets}));`);count++;
t.throws(()=>O.insert(result,{frozen:3},false),`(void)op::insert(c,${t.value(result)},[&]{auto x=c.object();c.set(op::Value(x),op::Key(u"frozen"),op::Value(3));return op::Value(x);}(),false)`);t.snapshot(result,t.map.get(result));t.finish();
}
{
const t=new Case('deep_copy_and_expandable');const child=t.object('child',{});t.set(child,'v',3);class Item{};t.bind('ctor',Item,'c.constructor(u"Item")');const inst=t.bind('inst',new Item(),'c.instance(ctor)'),fn=t.bind('fn',()=>{},'c.function(u"opaque")');
const a=t.object('a',O.expandable({}),'c.objectWithPrototype(c.intrinsics().expandablePrototype)');t.set(a,'left',child);t.set(a,'right',child);t.set(a,'instance',inst);t.set(a,'callback',fn);
const copied=t.operation(()=>O.copy(a),'op::copy(c,op::Value(a))');out.push(`expect(bool(c.get(${t.value(copied)},op::Key(u"left"))!=c.get(${t.value(copied)},op::Key(u"right"))));`);count++;t.operation(()=>O.makeArray(fn),`op::makeArray(c,${t.value(fn)})`);
const ar=t.object('ar',[],'c.array()');t.operation(()=>O.makeArray(ar),'op::makeArray(c,op::Value(ar))');t.operation(()=>O.expandable(child),'op::expandable(c,op::Value(child))');t.finish();
}
{
const t=new Case('array_copy_and_holes');const ar=t.object('ar',new Array(6),'c.array(6)'),nested=t.object('nested',{});t.set(nested,'k',1);t.set(ar,'0',nested);t.set(ar,'3',undefined);t.set(ar,'extra',4);const s=t.bind('s',Symbol('hidden'),'c.symbol(u"hidden")');t.define(ar,s,{value:8,enumerable:false});const src=t.object('src',{}),dst=t.object('dst',{});t.set(src,'items',ar);t.operation(()=>O.insert(dst,src,false),'op::insert(c,op::Value(dst),op::Value(src),false)');t.snapshot(dst);t.snapshot(ar);t.finish();
}
{
const t=new Case('append_remove_identity');const marker=t.object('marker',{}),other=t.object('other',{}),ar=t.object('ar',[],'c.array()');for(const[i,v]of [marker,NaN,2,2,other,undefined].entries())t.set(ar,String(i),v);
const rm=t.object('rm',[],'c.array()');for(const[i,v]of [marker,NaN,2].entries())t.set(rm,String(i),v);const add=t.object('add',new Array(2),'c.array(2)');t.set(add,'1',marker);
const edits=t.object('edits',{});t.set(edits,'[-]',rm);t.set(edits,'[+]',add);const src=t.object('src',{}),dst=t.object('dst',{});t.set(src,'items',edits);t.set(dst,'items',ar);t.operation(()=>O.userOptions(dst,src),'op::userOptions(c,op::Value(dst),std::vector<op::Value>{op::Value(src)})');t.snapshot(dst);t.snapshot(ar);t.snapshot(add);t.finish();
}
{
const t=new Case('malformed_sentinels');const dst=t.object('dst',{}),ar=t.object('ar',[],'c.array()');t.set(ar,'0',1);t.set(dst,'items',ar);const edits=t.object('edits',{});t.set(edits,'[+]','bad');const src=t.object('src',{});t.set(src,'items',edits);t.operation(()=>O.insert(dst,src),'op::insert(c,op::Value(dst),op::Value(src))');t.snapshot(ar);
const sym=t.bind('sym',Symbol('unexpected'),'c.symbol(u"unexpected")'),mixed=t.object('mixed',{}),add=t.object('add',[],'c.array()');t.set(mixed,'[+]',add);t.set(mixed,sym,add);t.set(src,'items',mixed);t.throws(()=>O.insert(dst,src),'(void)op::insert(c,op::Value(dst),op::Value(src))',true);t.finish();
}
{
const t=new Case('warn_fatal_inheritance');const proto=t.object('proto',{});t.set(proto,'inherited',5);const dst=t.object('dst',Object.create(proto),'c.objectWithPrototype(proto)');t.set(dst,'undefinedOwn',undefined);t.set(dst,'known',0);const src=t.object('src',{});t.set(src,'inherited',6);t.set(src,'undefinedOwn',2);t.set(src,'bad',1);t.set(src,'known',7);t.operation(()=>O.insert(dst,src),'op::insert(c,op::Value(dst),op::Value(src))');t.snapshot(dst);
const fatal=t.object('fatal',{});t.set(fatal,'known',0);const fsrc=t.object('fsrc',{});t.set(fsrc,'known',3);t.set(fsrc,'bad',8);t.set(fsrc,'later',9);O.OPTIONS.invalidOption='fatal';out.push('op::OPTIONS.invalidOption=op::InvalidOption::fatal;');t.throws(()=>O.insert(fatal,fsrc),'(void)op::insert(c,op::Value(fatal),op::Value(fsrc))',true);t.snapshot(fatal);t.finish();
}
{
const t=new Case('function_destination_and_setter');const fn=t.bind('fn',()=>{},'c.function(u"opaque")'),nested=t.object('nested',{});t.set(nested,'x',1);const dst=t.object('dst',{}),src=t.object('src',{});t.set(dst,'value',fn);t.set(src,'value',nested);t.operation(()=>O.insert(dst,src,false),'op::insert(c,op::Value(dst),op::Value(src),false)');t.operation(()=>fn.x,'c.get(op::Value(fn),op::Key(u"x"))');
let reads=0,writes=0;out.push('int reads=0,writes=0;');const g=t.bind('g',()=>{reads++;return 9;},'c.function(u"g",[&](op::Context&,op::Value,std::span<const op::Value>){++reads;return op::Value(9);})'),s=t.bind('s',()=>{writes++;},'c.function(u"s",[&](op::Context&,op::Value,std::span<const op::Value>){++writes;return op::Value{};})');const p=t.object('p',{});t.define(p,'x',{get:g,set:s,enumerable:true,configurable:true});const target=t.object('target',Object.create(p),'c.objectWithPrototype(p)'),access=t.object('access',{});t.define(access,'x',{get:g,enumerable:true,configurable:true});t.operation(()=>O.insert(target,access,false),'op::insert(c,op::Value(target),op::Value(access),false)');out.push(`expect(bool(reads==${reads} && writes==${writes})); expect(!c.hasOwn(op::Value(target),op::Key(u"x")));`);count+=2;t.finish();
}
{
const t=new Case('selection_and_separation');const proto=t.object('proto',{});t.set(proto,'inherited',1);const opts=t.object('opts',Object.create(proto),'c.objectWithPrototype(proto)'),child=t.object('child',{});t.set(opts,'a',child);t.set(opts,'b',undefined);t.set(opts,'rest',3);t.operation(()=>O.selectOptions(opts,'a','b','inherited'),'op::selectOptions(c,op::Value(opts),std::vector<std::u16string>{u"a",u"b",u"inherited"})');
const sel=t.object('sel',{});t.set(sel,'a',0);const s=t.bind('s',Symbol('extra'),'c.symbol(u"extra")');t.set(sel,s,1);t.operation(()=>O.selectOptionsFromKeys(opts,sel),'op::selectOptionsFromKeys(c,op::Value(opts),op::Value(sel))');const selectorProto=t.object('selectorProto',{});t.set(selectorProto,'a',true);const sel2=t.object('sel2',Object.create(selectorProto),'c.objectWithPrototype(selectorProto)');t.set(sel2,'b',undefined);
const js=O.separateOptions(opts,sel2,sel),name='parts_'+(++seq);out.push(`auto ${name}=op::separateOptions(c,op::Value(opts),std::vector<op::Value>{op::Value(sel2),op::Value(sel)}); expect(bool(${name}.size()==${js.length}));if(${name}.size()!=${js.length}) return;`);count++;js.forEach((x,i)=>t.expect(x,`${name}[${i}]`));
for(const key of ['a','b','inherited','missing'])t.operation(()=>O.lookup(key,opts,child),`op::lookup(c,${u(key)},op::Value(opts),op::Value(child))`);t.operation(()=>O.lookup('missing',opts),'op::lookup(c,u"missing",op::Value(opts))');t.finish();
}
{
const t=new Case('defaults_and_proto_basket');const dst=t.object('dst',{}),d1=t.object('d1',{}),d2=t.object('d2',{});t.set(d1,'new',1);t.set(d2,'new',2);t.set(d2,'another',3);t.operation(()=>O.defaultOptions(dst,d1,d2),'op::defaultOptions(c,op::Value(dst),std::vector<op::Value>{op::Value(d1),op::Value(d2)})');t.snapshot(dst);
const source=t.object('source',{}),nested=t.object('nested',{});t.set(nested,'k',1);t.define(source,'__proto__',{value:nested,writable:true,enumerable:true,configurable:true});t.set(source,'ordinary',7);t.operation(()=>O.copy(source),'op::copy(c,op::Value(source))');t.finish();
}
{
const t=new Case('live_array_adapters');const grow=t.object('grow',new Array(1),'c.array(1)');
const gg=t.bind('gg',()=>{grow[2]=9;return 4;},'c.function(u"grow",[&](op::Context& ctx,op::Value,std::span<const op::Value>){ctx.set(op::Value(grow),op::Key(u"2"),op::Value(9));return op::Value(4);})');t.define(grow,'0',{get:gg,enumerable:true,configurable:true});
let values=[...grow];out.push('auto grown=c.arrayValues(grow);',`expect(bool(grown.size()==${values.length}));if(grown.size()!=${values.length}) return;`);count++;values.forEach((v,i)=>t.expect(v,`grown[${i}]`));t.snapshot(grow);
const shrink=t.object('shrink',new Array(3),'c.array(3)'),sg=t.bind('sg',()=>{shrink.length=1;return 5;},'c.function(u"shrink",[&](op::Context& ctx,op::Value,std::span<const op::Value>){ctx.set(op::Value(shrink),op::Key(u"length"),op::Value(1));return op::Value(5);})');t.define(shrink,'0',{get:sg,enumerable:true,configurable:true});
values=[...shrink];out.push('auto shrunk=c.arrayValues(shrink);',`expect(bool(shrunk.size()==${values.length}));if(shrunk.size()!=${values.length}) return;`);count++;values.forEach((v,i)=>t.expect(v,`shrunk[${i}]`));
const ar=t.object('ar',[1,,3],'c.array(3)');out.push('c.set(op::Value(ar),op::Key(u"0"),op::Value(1));c.set(op::Value(ar),op::Key(u"2"),op::Value(3));');let calls=0;out.push('int calls=0;');const result=ar.filter(v=>{calls++;if(v===1){delete ar[2];ar[1]=2;ar[3]=4;}return true;});
out.push('auto filtered=c.arrayFilter(ar,[&](op::Value v){++calls;if(v==op::Value(1)){c.erase(ar,op::Key(u"2"));c.set(op::Value(ar),op::Key(u"1"),op::Value(2));c.set(op::Value(ar),op::Key(u"3"),op::Value(4));}return true;});');t.expect(result,'op::Value(filtered)');out.push(`expect(bool(calls==${calls}));`);count++;t.snapshot(ar);
const holes=t.object('holes',new Array(3),'c.array(3)');t.set(holes,'2',NaN);t.define(Array.prototype,'1',{value:7,writable:true,enumerable:true,configurable:true});
for(const needle of [undefined,7,NaN])t.operation(()=>holes.indexOf(needle),`op::Value(static_cast<double>(c.arrayIndexOf(holes,${t.value(needle)})))`);
values=[...holes];out.push('auto inherited=c.arrayValues(holes);',`expect(bool(inherited.size()==${values.length}));if(inherited.size()!=${values.length}) return;`);count++;values.forEach((v,i)=>t.expect(v,`inherited[${i}]`));delete Array.prototype[1];out.push('c.erase(c.intrinsics().arrayPrototype,op::Key(u"1"));');t.finish();
}
{
const t=new Case('descriptor_edge_cases');const a=t.object('a',{});t.define(a,'nan',{value:NaN,writable:false,configurable:false});t.define(a,'zero',{value:0,writable:false,configurable:false});t.define(a,'nan',{value:NaN});
t.throws(()=>Object.defineProperty(a,'zero',{value:-0}),'c.define(a,op::Key(u"zero"),op::Descriptor::data(op::Value(-0.0),false,false,false))');
const ar=t.object('ar',new Array(5),'c.array(5)');t.define(ar,'2',{value:2,writable:true,enumerable:true,configurable:false});t.set(ar,'4',4);
t.throws(()=>Object.defineProperty(ar,'length',{value:1,writable:false}),'c.define(ar,op::Key(u"length"),op::Descriptor::data(op::Value(1),false,false,false))');t.snapshot(ar);
t.throws(()=>{ar[3]=3;},'c.set(op::Value(ar),op::Key(u"3"),op::Value(3))');t.throws(()=>{ar.length=1.5;},'c.set(op::Value(ar),op::Key(u"length"),op::Value(1.5))');t.finish();
}
{
const t=new Case('getter_evaluation_order');let constructorReads=0;out.push('int constructor_reads=0;');const a=t.object('a',{}),g=t.bind('g',()=>{constructorReads++;return O.Expandable;},'c.function(u"constructor",[&](op::Context& ctx,op::Value,std::span<const op::Value>){++constructor_reads;return op::Value(ctx.intrinsics().expandableConstructor);})');t.define(a,'constructor',{get:g,configurable:true});t.operation(()=>O.isObject(a),'op::Value(op::isObject(c,op::Value(a)))');out.push(`expect(bool(constructor_reads==${constructorReads}));`);count++;
const dst=t.object('dst',{}),ar=t.object('ar',[1,2,3],'c.array()');for(const[i,v]of ar.entries())out.push(`c.set(op::Value(ar),op::Key(${u(String(i))}),${t.value(v)});`);t.set(dst,'items',ar);
const rm=t.object('rm',[2],'c.array()');out.push('c.set(op::Value(rm),op::Key(u"0"),op::Value(2));');let removes=0;out.push('int removes=0;');const rg=t.bind('rg',()=>{removes++;return rm;},'c.function(u"remove",[&](op::Context&,op::Value,std::span<const op::Value>){++removes;return op::Value(rm);})'),edits=t.object('edits',{});t.define(edits,'[-]',{get:rg,enumerable:true,configurable:true});const src=t.object('src',{});t.set(src,'items',edits);t.operation(()=>O.insert(dst,src,false),'op::insert(c,op::Value(dst),op::Value(src),false)');t.snapshot(dst);out.push(`expect(bool(removes==${removes}));`);count++;
const first=t.object('first',[8],'c.array()'),second=t.object('second',[9],'c.array()');out.push('c.set(op::Value(first),op::Key(u"0"),op::Value(8));c.set(op::Value(second),op::Key(u"0"),op::Value(9));');let selected=first,appends=0;out.push('auto selected=first;int appends=0;');const left=t.object('left',new Array(1),'c.array(1)'),lg=t.bind('lg',()=>{selected=second;return 1;},'c.function(u"left",[&](op::Context&,op::Value,std::span<const op::Value>){selected=second;return op::Value(1);})');t.define(left,'0',{get:lg,enumerable:true,configurable:true});t.set(dst,'items',left);
const ag=t.bind('ag',()=>{appends++;return selected;},'c.function(u"append",[&](op::Context&,op::Value,std::span<const op::Value>){++appends;return op::Value(selected);})'),add=t.object('add',{});t.define(add,'[+]',{get:ag,enumerable:true,configurable:true});t.set(src,'items',add);t.operation(()=>O.insert(dst,src,false),'op::insert(c,op::Value(dst),op::Value(src),false)');t.snapshot(dst);out.push(`expect(bool(appends==${appends}));`);count++;t.finish();
}
{
const t=new Case('descriptor_conversion_order');const description=t.object('description',{});let events=[];out.push('std::vector<std::u16string> events;');
for(const [i,name]of ['enumerable','configurable','value','writable','get','set'].entries()){
const v=name==='value'?4:name==='get'||name==='set'?undefined:true;const getter=t.bind('field_'+i,()=>{events.push(name);return v;},`c.function(u"field",[&](op::Context&,op::Value,std::span<const op::Value>){events.push_back(${u(name)});return ${t.value(v)};})`);t.define(description,name,{get:getter,enumerable:true,configurable:true});}
t.throws(()=>Object.defineProperty({},'x',description),'(void)c.descriptorFromObject(op::Value(description))');out.push(`expect(bool(events==std::vector<std::u16string>{${events.map(u).join(',')}}));`);count++;t.finish();
}
{
const t=new Case('copy_abrupt_mutation');const source=t.object('source',{}),child=t.object('child',{});t.set(source,'child',child);t.set(source,'later',7);
const g=t.bind('g',()=>{delete source.later;return Object;},'c.function(u"constructor",[&](op::Context& ctx,op::Value,std::span<const op::Value>){ctx.erase(source,op::Key(u"later"));return op::Value(ctx.intrinsics().objectConstructor);})');t.define(child,'constructor',{get:g,configurable:true});t.throws(()=>O.copy(source),'(void)op::copy(c,op::Value(source))');t.snapshot(source);t.finish();
}
{
const t=new Case('accessor_receiver');const p=t.object('p',{}),target=t.object('target',Object.create(p),'c.objectWithPrototype(p)');let seen;out.push('op::Value seen;');
const g=t.bind('g',function(){return this;},'c.function(u"receiver",[](op::Context&,op::Value receiver,std::span<const op::Value>){return receiver;})'),s=t.bind('s',function(){seen=this;},'c.function(u"receiver",[&](op::Context&,op::Value receiver,std::span<const op::Value>){seen=receiver;return op::Value{};})');t.define(p,'x',{get:g,set:s,enumerable:true,configurable:true});t.operation(()=>target.x,'c.get(op::Value(target),op::Key(u"x"))');t.set(target,'x',3);t.expect(seen,'seen');t.finish();
}
{
const t=new Case('deep_valid_copy');const input={};let cursor=input;for(let i=0;i<511;++i)cursor=cursor.next={};cursor.leaf=7;const copied=O.copy(input);let leaf=copied;for(let i=0;i<511;++i)leaf=leaf.next;assert.equal(leaf.leaf,7);
out.push('auto input=c.object(),cursor=input;for(int i=0;i<511;++i){auto next=c.object();c.set(op::Value(cursor),op::Key(u"next"),op::Value(next));cursor=next;}c.set(op::Value(cursor),op::Key(u"leaf"),op::Value(7));',
'op::Value copied;bool completed=true;try{copied=op::copy(c,op::Value(input));}catch(const op::Exception&){completed=false;}expect(completed);if(completed){expect(bool(copied!=op::Value(input)));for(int i=0;i<511;++i)copied=c.get(copied,op::Key(u"next"));',`expect(same(c.get(copied,op::Key(u"leaf")),${t.value(leaf.leaf)}));}`);count+=3;t.finish();
}
out.push('"math_port_options_host_lifetime"_test=[] {','std::weak_ptr<int> weak; { op::Context c; auto owned=std::make_shared<int>(1); weak=owned; auto a=c.object(); auto b=c.object(); c.set(op::Value(a),op::Key(u"self"),op::Value(a)); c.set(op::Value(a),op::Key(u"other"),op::Value(b)); c.set(op::Value(b),op::Key(u"other"),op::Value(a)); (void)c.function(u"owned",{},std::any(owned)); owned.reset(); expect(!weak.expired()); } expect(weak.expired());',
'op::Context a,b; bool rejected=false;try{(void)b.get(op::Value(a.object()),op::Key(u"x"));}catch(const op::Exception& e){rejected=e.name=="TypeError";}expect(rejected);',
'auto symbol=a.symbol(u"foreign");for(int method=0;method<3;++method){rejected=false;try{if(method==0)(void)b.ownKeys(op::Value(symbol));if(method==1)(void)b.descriptor(op::Value(symbol),op::Key(u"x"));if(method==2)(void)b.has(op::Value(symbol),op::Key(u"x"));}catch(const op::Exception& e){rejected=e.name=="TypeError";}expect(rejected);}','};',
'"math_port_options_host_recursion_limit"_test=[] {op::Context c;auto cyclic=c.object();c.set(op::Value(cyclic),op::Key(u"self"),op::Value(cyclic));bool rejected=false;try{(void)op::copy(c,op::Value(cyclic));}catch(const op::Exception& e){rejected=e.name=="RangeError" && e.message==u"Maximum call stack size exceeded";}expect(rejected);};','};');count+=7;
const text=out.join('\n')+'\n',target=path.join(root,'tests/math/reference_options.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),text,'Frozen options fixtures changed');else fs.writeFileSync(target,text);
console.log(JSON.stringify({source,hash,cases,assertions:count,mode:process.argv.includes('--check')?'check':'generate'}));
