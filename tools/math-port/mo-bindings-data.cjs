// Root-owned source static initialization and native constructor bridge.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {root,M,names}=require('./mo-pattern-reference.cjs');
const ts=require(path.join(root,'build/math-port-tooling/node_modules/typescript'));
assert.equal(ts.version,'5.9.3');
const source=ts.createSourceFile('mo.ts',fs.readFileSync(path.join(root,'.reference/MathJax-src/ts/core/MmlTree/MmlNodes/mo.ts'),'utf8'),ts.ScriptTarget.Latest,true);
const cls=source.statements.find(x=>ts.isClassDeclaration(x)&&x.name.text==='MmlMo');
const defaults=cls.members.find(x=>x.name?.getText(source)==='defaults').initializer;
const extra=defaults.properties.filter(ts.isPropertyAssignment).map(x=>x.name.text);
const q=JSON.stringify,p=v=>typeof v==='string'?q(v):typeof v==='boolean'?String(v):typeof v==='number'?String(v)+'.0':(()=>{throw Error('Unbound source static value');})();
const out=['// Generated source fields/bindings from locked MathJax 4.1.3 mo.ts.',
'// Apache-2.0; see LICENSE-MathJax.txt. Root owns this file.',
'module folia.math.core.mml_tree.mml_nodes.mo;','import folia.math.core.mml_tree.operator_dictionary;','import std;',
'namespace folia::math::mml {namespace {',
'PropertyList moDefaults=[] {PropertyList result;const auto base=AbstractMmlTokenNode::defaults();for(const auto&key:base.keys())result.setOwn(key,base.getOwn(key));'];
for(const key of extra)out.push('result.setOwn('+q(key)+','+p(M.defaults[key])+');');
out.push('return result;}();','Property classTable=operator_dict::OPTABLE();','Property classSpacing=operator_dict::MMLSPACING();','Property primeMap=PropertyList{'+Object.keys(M.remapPrimes).map(k=>'{'+q(k)+','+p(M.remapPrimes[k])+'}').join(',')+'};');
for(const name of names)out.push('MoPatternHandle '+name+'Pattern=std::make_shared<Pattern>(makeMoPattern(MoPattern::'+name+'));');
out.push('}','PropertyList& MmlMo::defaults(){return moDefaults;}','Property& MmlMo::operatorTable(){return classTable;}','Property& MmlMo::mathSpacing(){return classSpacing;}','Property& MmlMo::remapPrimes(){return primeMap;}');
for(const name of names)out.push('MoPatternHandle& MmlMo::'+name+'(){return '+name+'Pattern;}');
out.push('const MoClassServices& MmlMo::constructorOperators()const{const auto*p=std::any_cast<std::shared_ptr<MoClassServices>>(&constructorStaticServices());if(!p||!*p)throw Error("TypeError",u"Missing Mo constructor static services");return **p;}',
'void MmlMo::initializeClassFields(){_texClass=PropertyNull{};lspace=5.0/18.0;rspace=5.0/18.0;}',
'std::string MmlMo::kind()const{return "mo";}','bool MmlMo::isEmbellished()const{return true;}',
'void MmlMo::setTexClass(Property value){_texClass=std::move(value);}','}');
const output=out.join('\n')+'\n',target=path.join(root,'src/math/core/mml_tree/mml_nodes/mo_bindings.cpp');
if(process.argv.includes('--check'))assert.equal(fs.readFileSync(target,'utf8').replaceAll('\r\n','\n'),output);else fs.writeFileSync(target,output);
console.log(JSON.stringify({extra_defaults:extra.length,prime_remaps:Object.keys(M.remapPrimes).length,patterns:names.length,mode:process.argv.includes('--check')?'check':'generate'}));
