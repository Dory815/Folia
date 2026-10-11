// Root transports original TextNode method-call observations to the typed binding.
'use strict';
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');const root=path.resolve(__dirname,'../..');
const source=JSON.parse(fs.readFileSync(path.join(root,'docs/math-port/evidence/p23j-inheritance-token-isolation-source.json')));assert.equal(source.cases.length,8);
const frozen=fs.readFileSync(path.join(root,'tests/math/reference_mml_inheritance.cpp'),'utf8').replaceAll('\r\n','\n');
const anchor='suite inheritance=[] {"math_port_mml_inheritance_original_closure"_test=[] {const Case cases[]={';assert.equal(frozen.split(anchor).length,2);let observer=frozen.slice(0,frozen.indexOf(anchor));
function change(from,to){assert.equal(observer.split(from).length,2);observer=observer.replace(from,to);}
change('std::function<void()> after;std::u16string getText()','std::function<void()> after,afterInheritance;protected:void inheritAttributes(AttributeList a,Property d,Property l,Property p)override{AbstractMmlEmptyNode::inheritAttributes(a,d,l,p);if(afterInheritance)afterInheritance();}public:std::u16string getText()');
change(' auto v=arguments(c.args);',' if(c.mode=="skipProbe")dynamic_cast<Text*>(nodes[\'t\'])->afterInheritance=[&]{trace.push_back("text:inherit");};\n auto v=arguments(c.args);');
const rows=source.cases.map(({input:i,expected})=>' {'+[i.op,i.kind,i.shape,i.mode??'',i.prototype??''].map(JSON.stringify).join(',')+','+[i.args,i.attrs,i.arity??255,Boolean(i.explicit),Boolean(i.arrayNull),Boolean(i.attributesNull),Boolean(i.noncallable)].join(',')+','+JSON.stringify(expected)+'},').join('\n');
const data=observer+'suite isolation=[] {"math_port_mml_inheritance_original_token_isolation"_test=[] {const Case cases[]={\n'+rows+'\n };for(const auto&c:cases)expect(eq(run(c),c.expected))<<c.op<<c.kind<<c.args;};};\n}\n';
const out=path.join(root,'tests/math/reference_mml_inheritance_token_isolation.cpp');if(process.argv.includes('--check'))assert.equal(fs.readFileSync(out,'utf8').replaceAll('\r\n','\n'),data);else{assert.ok(!fs.existsSync(out));fs.writeFileSync(out,data);}console.log(source.cases.length+' actual Source token isolation records bound');
