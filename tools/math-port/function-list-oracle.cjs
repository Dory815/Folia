// Root acceptance: execute pinned MathJax CJS and native Promise stimuli.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict'),vm=require('node:vm');
const root=path.resolve(__dirname,'../..'),base=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src');
const hashes={};
const cjsHashes={FunctionList:'571f57801b863b2d9f01a66dc72f2836b7fd06a1a48a3fe0057e4ec3d51a7278',PrioritizedList:'787bd21f3634312af666917d8f140145864f98d09ede1bdde3c957d28ec2bc25'};
for(const name of ['FunctionList','PrioritizedList']){
 const source='ts/util/'+name+'.ts',bytes=fs.readFileSync(path.join(root,'.reference/MathJax-src',source));
 const sha=crypto.createHash('sha256').update(bytes).digest('hex');
 assert.equal(sha,JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/scope.json'))).files.find(f=>f.source===source).source_sha256);
 assert.equal(fs.readFileSync(path.join(base,source),'utf8').replaceAll('\r\n','\n'),bytes.toString().replaceAll('\r\n','\n'));
 hashes[source]=sha;hashes['cjs/util/'+name+'.js']=crypto.createHash('sha256').update(fs.readFileSync(path.join(base,'cjs/util/'+name+'.js'))).digest('hex');assert.equal(hashes['cjs/util/'+name+'.js'],cjsHashes[name]);
}
const {FunctionList}=require(path.join(base,'cjs/util/FunctionList.js')),{PrioritizedList}=require(path.join(base,'cjs/util/PrioritizedList.js'));
const {valueEmitter}=require('./oracle-values.cjs');
const u=s=>'u"'+s.replaceAll('\\','\\\\').replaceAll('"','\\"').replaceAll('\n','\\n')+'"',v=valueEmitter(u),{p}=v,cases=[];
const error=e=>e instanceof Error?{error:e.name}:e;
const raw={missing:undefined,null:null,false:false,true:true,zero:0,minuszero:-0,nan:NaN,empty:'',string:'false',array:[],record:{a:false}};
function deferred(){let resolve,reject;const promise=new Promise((a,b)=>{resolve=a;reject=b;});return {promise,resolve,reject};}
function scenario(mode,kind,action,phase){
 const queue=deferred(),second=deferred(),calls=[],ids=new Map(),recordIds=new Map();let sequence=0,triggered=false;
 second.promise.catch(()=>{}); // Some list mutations skip the second callback.
 const list=new FunctionList(),box={value:7},args=[box,17];let old=list.items;
 const itemName=item=>ids.get(item)??'raw';
 function rid(record){if(!recordIds.has(record))recordIds.set(record,++sequence);return recordIds.get(record);}
 const state=()=>({calls:structuredClone(calls),items:list.items.map(r=>r?{id:rid(r),item:itemName(r.item),priority:r.priority}:null),old:old.map(r=>r?{id:rid(r),item:itemName(r.item),priority:r.priority}:null),box:box.value,args:args[1]});
 const foreign=vm.runInNewContext('Promise.resolve(false)');
 function result(){if(kind==='promise')return queue.promise;if(kind==='second')return queue.promise;if(kind==='fulfilled')return Promise.resolve(false);if(kind==='rejected')return Promise.reject({reason:'input'});if(kind==='foreign')return foreign;if(kind==='thenable')return {then(){throw Error('must not run');}};if(kind==='throw')throw {reason:'callback'};if(kind==='typeerror')throw new TypeError('source');if(kind==='symbol')return Symbol('raw');return raw[kind];}
 function mutate(which){
  if(which==='remove_self')list.remove(a);
  if(which==='remove_next')list.remove(b);
  if(which==='append')list.add(d,30);
  if(which==='insert_before')list.add(d,-10);
  if(which==='rebind')list.items=[{item:d,priority:40}];
  if(which==='rebind_append'){list.items=[{item:d,priority:40}];old.push({item:d,priority:50});}
  if(which==='clear')old.length=0;
  if(which==='null_next')old[1]=null;
  if(which==='replace_next')old[1].item=d;
  if(which==='priority_next')old[1].priority=-99;
  if(which==='args'){box.value=99;args[1]=77;}
 }
 function fn(name,body){const f=function(...data){calls.push({name,record:rid(this),item:itemName(this.item),priority:this.priority,same:data[0]===box,box:data[0]?.value,arg:data[1],argc:data.length});return body(this);};ids.set(f,name);return f;}
 const d=fn('d',()=>true),b=fn('b',()=>kind==='second'?second.promise:kind==='continuation_throw'?(()=>{throw {reason:'continuation'};})():true),c=fn('c',()=>true);
 const a=fn('a',receiver=>{if(!triggered){triggered=true;mutate(action);if(action==='receiver'){receiver.item=d;receiver.priority=81;}if(action==='reentrant')list.execute(box,23);}return kind==='continuation_throw'?queue.promise:result();});
 list.add(a,0);list.add(b,10);list.add(c,20);old=list.items;old.forEach(rid);
 return {list,queue,second,state,mutate,args};
}
function synchronous(kind,action){const s=scenario('sync',kind,action,'none');let outcome;try{outcome=s.list.execute(...s.args);}catch(e){outcome=error(e);}return {...s.state(),outcome};}
async function asynchronous(kind,action,phase){const s=scenario('async',kind,action,phase);let outcome='pending';const output=s.list.asyncExecute(...s.args);output.then(x=>{outcome=x;},e=>{outcome=error(e);});const states=[];const snap=()=>states.push({...s.state(),outcome:structuredClone(outcome)});snap();s.mutate(phase);if(kind==='promise'||kind==='second'||kind==='continuation_throw')s.queue.resolve(false);for(let i=0;i<6;i++){await Promise.resolve();snap();if(i===1&&kind==='second')s.second.reject({reason:'second'});}return states;}
function construction(kind){const calls=[],ids=new Map(),a=()=>true,b=()=>true,c=()=>true;ids.set(a,'a');ids.set(b,'b');ids.set(c,'c');let defs=null;
 if(kind==='empty')defs=[];
 if(kind==='mixed')defs=[a,[b,2,'ignored'],[a,2],[c,undefined],[],null];
 if(kind==='numeric')defs=[[a,NaN],[b,-Infinity],[c,Infinity],[a,-0],[b,0],[c,2],[a,2]];
 if(kind==='live'||kind==='shrink'||kind==='default')defs=[a,[b,undefined],c];
 class Probe extends FunctionList{
  addList(values){calls.push(['list',values.length]);return super.addList(values);}
  add(item,priority=PrioritizedList.DEFAULTPRIORITY){calls.push(['add',ids.get(item)??'raw',priority]);if(calls.length===2){if(kind==='live')defs.push([a,-1]);if(kind==='shrink')defs.length=1;if(kind==='default')PrioritizedList.DEFAULTPRIORITY=31;}return super.add(item,priority);}
 }
 const saved=PrioritizedList.DEFAULTPRIORITY;try{const list=new Probe(defs);return {calls,items:list.items.map(r=>({item:ids.get(r.item)??'raw',priority:r.priority}))};}finally{PrioritizedList.DEFAULTPRIORITY=saved;}
}
async function promiseHost(kind){const first=deferred(),second=deferred(),trace=[],states=[];let result='pending',output=first.promise;
 const named=name=>v=>{trace.push([name,v]);return v;};
 if(kind==='handlers'){first.promise.then(named('a'));first.promise.then(named('b'));first.resolve(7);first.reject(8);first.resolve(9);first.promise.then(named('c'));}
 if(kind==='reject'){output=first.promise.then(named('unused')).catch(e=>{trace.push(['catch',e]);return 11;});first.reject(7);first.resolve(8);}
 if(kind==='throw'){output=first.promise.then(()=>{trace.push(['throw']);throw {raw:3};}).catch(e=>{trace.push(['catch',e]);return false;});first.resolve(7);}
 if(kind==='adopt'){first.resolve(second.promise);first.reject(99);first.resolve(19);second.resolve(13);}
 if(kind==='adopt_reject'){first.resolve(second.promise);second.reject({raw:5});}
 if(kind==='return_promise'){output=first.promise.then(v=>{trace.push(['return',v]);return second.promise;});first.resolve(7);second.resolve(17);}
 if(kind==='self')first.resolve(first.promise);
 if(kind==='pending')first.promise.then(named('never'));
 if(kind==='chain'){output=first.promise.then(named('one')).then(named('two'));first.resolve(19);}
 if(kind==='late'){first.resolve(23);output=first.promise.catch(()=>99).then(named('late'));}
 output.then(v=>{result=v;},e=>{result=error(e);});const snap=()=>states.push({trace:structuredClone(trace),result:structuredClone(result)});snap();for(let i=0;i<7;i++){await Promise.resolve();snap();}return states;
}
function priorityCapture(){const list=new PrioritizedList();list.add('a',1);list.add('b',2);const old=list.items,it=list[Symbol.iterator](),events=[];events.push(it.next());list.items=[{item:'new',priority:9}];old.push({item:'c',priority:3});events.push(it.next());old.splice(0,1);events.push(it.next());return events;}
async function invalid(kind,mode){const calls=[],list=new FunctionList();list.add(()=>{calls.push('before');return true;},0);list.add(raw[kind],1);list.add(()=>{calls.push('after');return true;},2);if(kind==='null_storage')list.items=null;let outcome;try{outcome=mode==='sync'?list.execute():await list.asyncExecute();}catch(e){outcome=error(e);}return {calls,outcome};}
(async()=>{
 for(const kind of Object.keys(raw).concat(['symbol','thenable','foreign','throw','typeerror']))for(const action of ['none','remove_self','remove_next','append','insert_before','rebind','rebind_append','clear','null_next','replace_next','priority_next','args','receiver','reentrant']){
  cases.push('"math_port_function_sync_'+kind+'_'+action+'"_test=[] {expect(deep(synchronous('+JSON.stringify(kind)+','+JSON.stringify(action)+'),'+p(synchronous(kind,action))+'));};');
 }
 for(const kind of Object.keys(raw).concat(['symbol','thenable','foreign','throw','typeerror','promise','fulfilled','rejected','second','continuation_throw']))for(const action of ['none','remove_self','append','rebind','args','receiver']){
  cases.push('"math_port_function_async_'+kind+'_'+action+'"_test=[] {expect(deep(asynchronous('+JSON.stringify(kind)+','+JSON.stringify(action)+',"none"),'+p(await asynchronous(kind,action,'none'))+'));};');
 }
 for(const phase of ['remove_self','remove_next','append','insert_before','rebind','rebind_append','clear','null_next','replace_next','priority_next','args'])cases.push('"math_port_function_async_external_'+phase+'"_test=[] {expect(deep(asynchronous("promise","none",'+JSON.stringify(phase)+'),'+p(await asynchronous('promise','none',phase))+'));};');
 for(const kind of ['null','empty','mixed','numeric','live','shrink','default'])cases.push('"math_port_function_constructor_'+kind+'"_test=[] {expect(deep(construction('+JSON.stringify(kind)+'),'+p(construction(kind))+'));};');
 for(const kind of ['handlers','reject','throw','adopt','adopt_reject','return_promise','self','pending','chain','late'])cases.push('"math_port_function_promise_host_'+kind+'"_test=[] {expect(deep(promiseHost('+JSON.stringify(kind)+'),'+p(await promiseHost(kind))+'));};');
 cases.push('"math_port_function_priority_capture"_test=[] {expect(deep(priorityCapture(),'+p(priorityCapture())+'));};');
 for(const kind of Object.keys(raw).concat('null_storage'))for(const mode of ['sync','async'])cases.push('"math_port_function_invalid_'+kind+'_'+mode+'"_test=[] {expect(deep(invalid('+JSON.stringify(kind)+','+JSON.stringify(mode)+'),'+p(await invalid(kind,mode))+'));};');
 const header=fs.readFileSync(path.join(__dirname,'function-list-native.inc'),'utf8').replaceAll('\r\n','\n');
 const output='// Generated from pinned CJS by function-list-oracle.cjs. Root owns expectations.\n'+header+'\n'+v.declarations().join('\n')+'\nnamespace {suite function_reference_tests=[] {\n'+cases.join('\n')+'\n};}\n';
 const file=path.join(root,'tests/math/reference_function_list.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(file,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(file,output);
 console.log(JSON.stringify({source_hashes:hashes,cases:cases.length,values:v.size(),mode:process.argv.includes('--check')?'check':'generate'}));
})().catch(e=>{console.error(e);process.exitCode=1;});
