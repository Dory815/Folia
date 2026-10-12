// Root independent compiling faults; requires implementation agents idle.
'use strict';const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),body='src/math/input/tex/unit_util.cpp',host='src/math/input/tex/unit_pattern.ixx';
const saved=new Map([body,host].map(file=>[file,fs.readFileSync(path.join(root,file),'utf8')]));
const faults=[
 ['constructor-duplicate-first-value',body,[['else found->second=value;','else (void)value;']]],
 ['set-pattern-regeneration',body,[['updateDimen();\n    return this;','/* no regeneration */\n    return this;']]],
 ['get-zero-truthiness',body,[['if(found!=map_.end()&&property_host::truthy(Property(found->second)))','if(found!=map_.end())']]],
 ['get-hardcoded-pt',body,[['return Missing{};','return 0.1;']]],
 ['erase-absent-pattern-identity',body,[['if(found==map_.end())return false;','if(found==map_.end()){updateDimen();return false;}']]],
 ['rest-final-whitespace',body,[['unit+u" ?"','unit+u"\\\\s*"']]],
 ['match-comma-normalization',body,[['unit_util_host::firstComma(unit_util_host::slot(*match,1))','unit_util_host::slot(*match,1)']]],
 ['mu-not-normalized',body,[['if(matched[1]!=Property("mu"))return matched;','if(true)return matched;']]],
 ['mu-live-factor',body,[['const Property factor=util.UNIT_CASES->get(matched[1]);','const Property factor=1.0/18;']]],
 ['match-utf16-length',body,[['const std::size_t length=unit_util_host::text(unit_util_host::slot(*match,0)).size();','const auto units=unit_util_host::text(unit_util_host::slot(*match,0));const std::size_t length=units.size()-std::ranges::count_if(units,[](char16_t c){return c>=0xDC00&&c<=0xDFFF;});']]],
 ['dimen-invalid-default',body,[['property_host::truthy(match[0])?match[0]:Property("1")','property_host::truthy(match[0])?match[0]:Property("0")']]],
 ['em-cutoff-inclusive',body,[['if(std::abs(value)<0.0006)','if(std::abs(value)<=0.0006)']]],
 ['em-exponent-zero-regex',body,[['fixed=std::regex_replace(fixed,std::regex(R"(\\.?0+$)"),"");','if(std::abs(value)<1e21)fixed=std::regex_replace(fixed,std::regex(R"(\\.?0+$)"),"");']]],
 ['trim-nonstring-identity',body,[['if(!raw)return text;','if(!raw)return unit_util_host::string(property_host::string(text));']]],
 ['trim-escaped-ending-ascii-space',body,[['source.back()==u\' \')','js_input::isWhitespace(source.back()))']]],
 ['pattern-unicode-whitespace',host,[['\\xFEFF','']]]
];
function run(label){const r=cp.spawnSync('cmd.exe',['/d','/c','build_test.bat','*math_port_unit_util*'],{cwd:root,encoding:'utf8',maxBuffer:64*1024*1024});assert.ifError(r.error);const log=(r.stdout||'')+(r.stderr||'');fs.writeFileSync(path.join(root,'build','p44-unit-util-'+label+'.log'),log);return{status:r.status,log};}
const results=[];try{
 assert.equal(run('mutation-baseline').status,0);
 for(const[id,file,changes]of faults){let changed=saved.get(file);for(const[from,to]of changes){assert.equal(changed.split(from).length,2,id+' exact anchor');changed=changed.replace(from,to);}fs.writeFileSync(path.join(root,file),changed);const r=run('fault-'+id),compiling=!/error C\d|fatal error|ninja: build stopped/.test(r.log),detected=r.status!==0&&/FAILED/.test(r.log);results.push({id,file,compiling,detected,status:r.status});fs.writeFileSync(path.join(root,file),saved.get(file));console.log(JSON.stringify(results.at(-1)));assert.ok(compiling&&detected,id+' must compile and fail original assertions');}
}finally{for(const[file,text]of saved)fs.writeFileSync(path.join(root,file),text);}
assert.equal(run('mutation-restored').status,0);fs.writeFileSync(path.join(root,'docs/math-port/evidence/p44-unit-util-mutations.json'),JSON.stringify({sources:[...saved].map(([file,text])=>({file,sha256:crypto.createHash('sha256').update(text).digest('hex')})),faults:results},null,2)+'\n');
