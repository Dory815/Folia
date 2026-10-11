// Root independent observations from actual pinned MapHandler CJS.
'use strict';
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),base=path.join(root,'tools/mathjax-bundle/node_modules/@mathjax/src/cjs');
for(const[name,sha]of Object.entries({'input/tex/MapHandler':'d82e3034776d2392689aad86500b5e60dbe0aa5540853b48f308841879f3cf67','input/tex/TokenMap':'9e3f086080a6ab052b68b22a5d9ee3dbc5e17d7b0c03a31e789664b01df36cf7','util/FunctionList':'571f57801b863b2d9f01a66dc72f2836b7fd06a1a48a3fe0057e4ec3d51a7278'}))assert.equal(crypto.createHash('sha256').update(fs.readFileSync(path.join(base,name+'.js'))).digest('hex'),sha);
const{MapHandler,SubHandler,SubHandlers}=require(path.join(base,'input/tex/MapHandler.js')),{CharacterMap}=require(path.join(base,'input/tex/TokenMap.js'));
const cases=[];function add(name,flags){assert.ok(flags.every(x=>typeof x==='boolean'));cases.push({name,flags});}
const map=(name,{contains=()=>true,lookup=()=>null,parse=()=>null}={})=>({name,parser:null,parserFor(){return null;},contains,lookup,parse});
const names=xs=>Array.from(xs._configuration).map(r=>r.item.name),items=xs=>Array.from(xs._fallback);
{
 const a=map('p20-registry'),b=map('p20-registry');MapHandler.register(a);const flags=[MapHandler.getMap(a.name)===a];MapHandler.register(b);flags.push(MapHandler.getMap(a.name)===b,MapHandler.getMap('p20-absent')===undefined);
 for(const key of [NaN,0,null,undefined,{},false]){const m=map(key);MapHandler.register(m);flags.push(MapHandler.getMap(key)===m);}
 const m=map(-0);MapHandler.register(m);flags.push(MapHandler.getMap(0)===m);let reads=0;const getter={get name(){++reads;return 'p20-getter';}};MapHandler.register(getter);flags.push(reads===1,MapHandler.getMap('p20-getter')===getter);try{MapHandler.register(null);flags.push(false);}catch(e){flags.push(e.name==='TypeError');}add('registry',flags);
}
{
 const s=new SubHandler(),a=map('p20-a'),b=map('p20-b'),c=map('p20-c');[a,b,c].forEach(MapHandler.register);const f=()=>false;s.add([a.name,b.name,c.name],f,5);const flags=[s.toString()==='p20-c, p20-b, p20-a',names(s).join('|')===[c.name,b.name,a.name].join('|'),items(s)[0].item===f,items(s)[0].priority===5];s.add([a.name],null,1);flags.push(s.retrieve(a.name)===a,names(s)[0]===a.name);s.remove([a.name]);flags.push(names(s).join('|')===[a.name,c.name,b.name].join('|'));s.remove([],f);flags.push(items(s).length===0);add('priority_remove',flags);
}
{
 const s=new SubHandler(),a=map('p20-part-a'),c=map('p20-part-c'),f=()=>{};[a,c].forEach(MapHandler.register);const warnings=[],saved=console.log;try{console.log=x=>warnings.push(x);s.add([a.name,'p20-missing',c.name],f);}finally{console.log=saved;}add('missing_partial',[s.toString()===c.name,items(s).length===0,warnings.length===1,warnings[0]==="TexParser Warning: Configuration 'p20-missing' not found! Omitted."]);
}
{
 const s=new SubHandler(),a=map('p20-dup'),f=()=>{};MapHandler.register(a);s.add([a.name,a.name],f,3);s.add([],f,1);s.remove([a.name],f);add('duplicates',[names(s).length===1,names(s)[0]===a.name,items(s).length===1,items(s)[0].priority===1]);
}
{
 const flags=[];for(const [i,value]of [false,0,'',null,undefined,NaN].entries()){const s=new SubHandler(),events=[],a=map('p20-falsy-'+i,{parse(){events.push('map');return value;},contains(){throw Error('parse must not call contains');}}),p={};MapHandler.register(a);let record;function fallback(env,t){events.push('fallback');flags.push(this===record,env===p,t==='x');return {ignored:true};}s.add([a.name],fallback);record=items(s)[0];flags.push(s.parse([p,'x'])===undefined,events.join('|')==='map|fallback');}add('falsy_parse',flags);
}
{
 const s=new SubHandler(),events=[],a=map('p20-break-a',{parse(){events.push('a');return SubHandler.FALLBACK;}}),b=map('p20-break-b',{parse(){events.push('b');return true;}});[a,b].forEach(MapHandler.register);s.add([b.name,a.name],()=>events.push('fallback'));add('fallback_break',[s.parse([{},'x'])===undefined,events.join('|')==='a|fallback']);
 const q=new SubHandler(),other=Symbol('fallback'),m=map('p20-other-symbol',{parse(){return other;}});MapHandler.register(m);q.add([m.name],()=>{throw Error('wrong fallback');});add('other_symbol',[q.parse([{},'x'])===other]);
}
{
 const s=new SubHandler(),events=[],a=map('p20-live-a',{parse(){events.push('a');s.remove([a.name]);return false;}}),b=map('p20-live-b',{parse(){events.push('b');return true;}}),c=map('p20-live-c',{parse(){events.push('c');return 7;}});[a,b,c].forEach(MapHandler.register);s.add([c.name,b.name,a.name],()=>{throw Error('wrong fallback');});add('live_iteration',[s.parse([{},'x'])===7,events.join('|')==='a|c']);
}
{
 const flags=[];for(const value of [true,7,'x',{},Symbol('x')]){const s=new SubHandler(),m=map('p20-truthy-'+flags.length,{parse(){return value;}});MapHandler.register(m);s.add([m.name],()=>{throw Error('wrong fallback');});flags.push(s.parse([{},'x'])===value);}add('truthy_results',flags);
 const s=new SubHandler();try{s.parse([{},'x']);add('empty_fallback',[false]);}catch(e){add('empty_fallback',[e.name==='TypeError']);}
 const t=new SubHandler();t.add([],true);try{t.parse([{},'x']);add('noncallable_fallback',[false]);}catch(e){add('noncallable_fallback',[e.name==='TypeError']);}
}
{
 const s=new SubHandler(),events=[],a=map('p20-look-a',{contains(t){events.push('a:'+t);return false;},lookup(){throw Error('wrong lookup');}}),b=map('p20-look-b',{contains(t){events.push('b:'+t);return true;},lookup(t){events.push('lookup:'+t);return false;}});[a,b].forEach(MapHandler.register);s.add([b.name,a.name],null);add('lookup',[s.lookup('x')===false,events.join('|')==='a:x|b:x|lookup:x',s.applicable('x')===b,s.contains('x')===true]);const q=new SubHandler();add('empty_queries',[q.lookup('x')===null,q.applicable('x')===null,q.retrieve('x')===null,q.contains('x')===false,q.toString()==='']);
}
{
 const s=new SubHandler(),m=new CharacterMap('p20-char',()=>true,{nil:[null,null],missing:[undefined,null],false:[false,null],text:'y'});s.add([m.name],null);add('character_null',[s.contains('nil')===false,s.contains('missing')===true,s.contains('false')===true,s.contains('text')===true,s.applicable('nil')===m,s.lookup('nil')===m.lookup('nil')]);
 m.lookup=()=>null;m.contains=()=>true;try{s.contains('x');add('character_missing_lookup',[false]);}catch(e){add('character_missing_lookup',[e.name==='TypeError']);}
}
{
 const s=new SubHandler(),m=map(NaN);MapHandler.register(m);s.add([NaN],null);add('strict_name',[s.retrieve(NaN)===null,s.toString()==='NaN']);
}
{
 const s=new SubHandler(),events=[],p={},reason={};function later(){events.push('later');}function first(env,t){events.push('first');const flags=[this.item===first,this.priority===1,env===p,t==='x'];s.remove([],first);this.item=later;this.priority=9;add('fallback_receiver',flags);return false;}s.add([],later,5);s.add([],first,1);const record=items(s)[0];add('fallback_result',[s.parse([p,'x'])===undefined,events.join('|')==='first',record.item===later,record.priority===9,items(s).length===1]);
 s.add([],()=>{throw reason;},0);try{s.parse([p,'x']);add('throw_identity',[false]);}catch(e){add('throw_identity',[e===reason]);}
}
{
 const all=new SubHandlers(),a=map('p20-hand-a'),b=map('p20-hand-b');[a,b].forEach(MapHandler.register);const inherited={macro:[b.name]},h=Object.assign(Object.create(inherited),{environment:[a.name],character:[b.name]}),f=()=>{},fallbacks=Object.create({environment:f});all.add(h,fallbacks,4);add('subhandlers_add',[all.get('environment') instanceof SubHandler,all.get('macro')===undefined,all.get('character').retrieve(b.name)===b,items(all.get('environment'))[0].item===f,all.retrieve(a.name)===a,Array.from(all.keys()).join('|')==='environment|character']);all.remove({environment:[a.name],absent:[]},{environment:f});add('subhandlers_remove',[all.retrieve(a.name)===null,all.get('environment') instanceof SubHandler,items(all.get('environment')).length===0]);
 const replacement=new SubHandler();all.set('environment',replacement);add('subhandlers_set',[all.get('environment')===replacement,Array.from(all.keys()).join('|')==='environment|character',all.retrieve('p20-absent')===null]);
}
{
 const all=new SubHandlers();all.add({'10':[],'2':[],macro:[],character:[]},{});const it=all.keys();const flags=[it.next().value==='2'];all.set('later',new SubHandler());flags.push(Array.from(it).join('|')==='10|macro|character|later');all.set('after-done',new SubHandler());flags.push(it.next().done===true);add('live_keys',flags);
}
{
 const s=new SubHandler(),array=['p20-array-change-missing','p20-array-change-last'],last=map(array[1]);MapHandler.register(last);const logs=[],saved=console.log;try{console.log=x=>{logs.push(x);array[0]=last.name;};s.add(array,null);}finally{console.log=saved;}add('add_snapshot',[names(s).length===1,names(s)[0]===last.name,logs.length===1]);
}
const text=JSON.stringify({stage:'actual Source MapHandler; concrete token maps not accepted',source:'7cacdaef3ed1df90b4f05cc0b3183a2956ae8f848f20ae8833932faa4efcb2a2',cases},null,2)+'\n',target=path.join(root,'docs/math-port/evidence/p20a-map-handler-source.json');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),text);else fs.writeFileSync(target,text);
console.log(JSON.stringify({cases:cases.length,observations:cases.reduce((n,c)=>n+c.flags.length,0),mode:process.argv.includes('--check')?'check':'generate'}));
