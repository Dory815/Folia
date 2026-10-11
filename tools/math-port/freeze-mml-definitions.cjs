// Root one-time definition binding/stub freeze. Not an implementation or fallback.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const root=path.resolve(__dirname,'../..'),base=path.join(root,'src/math/core/mml_tree'),plan=[],ledger=[];
// Scanner ignores comments and character/string literals while balancing C++.
function closing(s,start,open,close){let depth=1,i=start+1;for(;i<s.length;++i){
 if(s.startsWith('//',i)){i=s.indexOf('\n',i);if(i<0)break;continue;}
 if(s.startsWith('/*',i)){const end=s.indexOf('*/',i+2);assert.ok(end>=0);i=end+1;continue;}
 if(s[i]==='"'||s[i]==="'"){const quote=s[i++];for(;i<s.length;++i){if(s[i]==='\\'){++i;continue;}if(s[i]===quote)break;}continue;}
 if(s[i]===open)++depth;else if(s[i]===close&&!--depth)return i;
 }throw Error('Unclosed C++ definition');}
function file(f){const before=fs.readFileSync(f,'utf8').replaceAll('\r\n','\n');assert.ok(!before.includes('P23f Root stub'),'Do not erase delivered implementations');let s=before;
 s=s.replace(/MmlNode\*\s+((?:AbstractMml\w+|Mml\w+)::(?:Parent|core|coreMO|setTeXclass|adjustTeXclass))\(([^)]*)\)/g,(_,name,args)=>'NodeValue '+name+'('+args.replaceAll('MmlNode*','NodeValue')+')')
  .replace(/bool\s+((?:AbstractMml\w+|Mml\w+|MathChoice)::notParent)\(/g,'Property $1(')
  .replace(/(void\s+AbstractMml\w+::(?:inheritAttributesFrom|updateTeXclass|getPrevClass))\(([^)]*)\)/g,(_,name,args)=>name+'('+args.replaceAll('MmlNode*','NodeValue')+')')
  .replace(/MmlNode\*\s+(AbstractMmlNode::(?:appendChild|replaceChild))\(([^)]*)\)/g,(_,name,args)=>'NodeValue '+name+'('+args.replaceAll('MmlNode*','NodeValue')+')')
  .replace(/(std::u16string\s+MmlMo::coreText)\(([^)]*)\)/g,(_,name,args)=>name+'('+args.replaceAll('MmlNode*','NodeValue')+')')
  .replace(/MmlNode\*\s+adjustOperatorTeXclass\(([^)]*)\)/g,(_,args)=>'NodeValue adjustOperatorTeXclass('+args.replace('MmlNode*','NodeValue')+')');
 const matches=[...s.matchAll(/\b((?:AbstractMml\w+|Mml\w+|MathChoice|TextNode|XMLNode|HtmlNode)::(\w+))\s*\(/g)];
 const ranges=[];for(const m of matches){const open=s.indexOf('(',m.index),end=closing(s,open,'(',')');let tail=end+1;while(/\s/.test(s[tail]??''))++tail;
  if(s.startsWith('const',tail)){tail+=5;while(/\s/.test(s[tail]??''))++tail;}if(s[tail]!=='{')continue;
  const stop=closing(s,tail,'{','}'),body=s.slice(tail+1,stop),name=m[2];
  const tagged=['setTeXclass','adjustTeXclass','getPrevClass','updateTeXclass','inheritAttributesFrom','appendChild','replaceChild'].includes(name);
  const simple=/^\s*return\s+(?:this|prev|previous|true|false);\s*$/.test(body);
  const affected=/\bchildNodes\b|->parent\b|\bparent\b|\bParent\s*\(|\b(?:core|coreMO|setTeXclass)\s*\(|\b(?:arithmeticChild|tableNode|tableChild|tableSlot|tableCreate|multiscriptNode|multiscriptChild|multiscriptSlot|multiscriptInsert|moChild|moRequired)\s*\(/.test(body);
  if((tagged&&!simple)||affected){ranges.push([tail,stop,m[1]]);}
 }
 // Source helpers are explicit dereference/Get boundaries, never dense projections.
 for(const m of s.matchAll(/\b(child_at|child_zero|required_child_zero|adjustOperatorTeXclass)\s*\(/g)){
  const open=s.indexOf('(',m.index),end=closing(s,open,'(',')');let start=end+1;while(/\s/.test(s[start]??''))++start;if(s[start]!=='{')continue;const stop=closing(s,start,'{','}');ranges.push([start,stop,m[1]]);
 }
 const unique=[...new Map(ranges.map(r=>[r[0],r])).values()].sort((a,b)=>b[0]-a[0]);
 for(const[start,end,name]of unique){s=s.slice(0,start)+'{\n    // P23f Root stub: '+name+'.\n    throw Error("Unimplemented",u"Root-frozen tagged MML operation");\n}'+s.slice(end+1);ledger.push({file:path.relative(root,f).replaceAll('\\','/'),method:name});}
 if(s!==before)plan.push({file:f,text:s});
}
function walk(dir){for(const e of fs.readdirSync(dir,{withFileTypes:true})){const f=path.join(dir,e.name);if(e.isDirectory())walk(f);else if(e.name.endsWith('.cpp'))file(f);}}walk(base);
assert.ok(ledger.some(x=>x.method==='AbstractMmlNode::initializeMml'),'Actual constructor red must be frozen');assert.ok(ledger.length>50);assert.ok(plan.length<50);
// No partial writes precede inventory/scanner validation.
for(const p of plan)fs.writeFileSync(p.file,p.text);
const out=path.join(root,'docs/math-port/evidence/p23f-definition-red-freeze.json');assert.ok(!fs.existsSync(out));fs.writeFileSync(out,JSON.stringify({status:'Root definition/signature freeze; implementations and semantic acceptance pending',definition_files:plan.length,real_stubs:ledger.length,methods:ledger.sort((a,b)=>a.file.localeCompare(b.file)||a.method.localeCompare(b.method))},null,2)+'\n');
console.log(JSON.stringify({definition_files:plan.length,real_stubs:ledger.length}));
