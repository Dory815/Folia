// Root Source299 append/child subset: actual native full MML factory and classes.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mi;
namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::mml;
struct ParentHook:MmlMi {
 using MmlMi::MmlMi;
 std::function<void(NodeValue)> after;
 void setParent(NodeValue v)override{MmlMi::setParent(v);if(after)after(v);}
};
struct Observed:MmlFactory {
 std::vector<MmlNode*> nodes;
 std::vector<std::string> trace;
 Observed(){setNodeClass("mi",classBinding<ParentHook>());}
 MmlNode* create(std::string_view kind,tree::FactoryArgs args)override{auto* n=MmlFactory::create(kind,args);nodes.push_back(n);return n;}
 std::string tag(NodeValue v)const{if(v.kind()==tree::ChildKind::Undefined)return "U";if(v.kind()==tree::ChildKind::Null)return "N";auto it=std::find(nodes.begin(),nodes.end(),v.nodeOrNull());return it==nodes.end()?"UNBOUND":"n"+std::to_string(it-nodes.begin());}
};
struct Case{std::string name,kind,shape,mutation,op;bool proto;std::string expected;};
std::string ascii(std::u16string_view s){std::string v;for(auto c:s)v+=static_cast<char>(c);return v;}
Children shape(std::string_view s,const std::array<MmlNode*,3>& nodes){auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->resize(s.size());for(std::size_t i=0;i<s.size();++i){auto c=s[i];if(c=='H')continue;a->set(i,c=='U'?NodeValue(Missing{}):c=='N'?NodeValue(PropertyNull{}):NodeValue(nodes[c-'a']));}return a;}
std::string array(const Children& a,const Observed& f){std::string own,has,values;for(std::size_t i=0;i<a->size();++i){own+=a->hasOwn(i)?'1':'0';has+=a->has(i)?'1':'0';if(i)values+=',';values+=f.tag(a->get(i));}return std::to_string(a->size())+':'+own+':'+has+':'+values;}
std::string run(const Case& c){Observed f;std::array<MmlNode*,3> seed;for(auto& n:seed)n=f.create("mi",{});auto* row=f.create("inferredMrow",{});auto* node=f.create(c.kind,{});row->childNodes=shape(c.shape,seed);auto original=row->childNodes;std::vector<std::pair<std::string,Children>> captured{{"original",original}};
 if(c.proto)original->setPrototype(shape("Hb",seed));
 if(!c.mutation.empty()){
  captured.push_back({"target",node->childNodes});
  dynamic_cast<ParentHook*>(seed[0])->after=[&](NodeValue v){f.trace.push_back("set:"+f.tag(v));if(c.mutation=="append")original->push_back(seed[2]);if(c.mutation=="delete")original->erase(1);if(c.mutation=="shrink")original->resize(1);if(c.mutation=="rebind-source")row->childNodes=shape("c",seed);if(c.mutation=="rebind-target")node->childNodes=shape("c",seed);};
 }
 std::string error="ok",result;try{if(c.op=="set")node->setChildren(original);else result=f.tag(node->appendChild(row));}catch(const Error& e){error=e.name+'|'+ascii(e.message);}
 std::string out=error+'|';for(std::size_t i=0;i<f.nodes.size();++i){if(i)out+=';';auto* n=f.nodes[i];out+=f.tag(n)+':'+n->kind()+':'+f.tag(n->getParent())+':'+array(n->childNodes,f);}out+='|';std::sort(captured.begin(),captured.end(),[](auto&a,auto&b){return a.first<b.first;});for(std::size_t i=0;i<captured.size();++i){if(i)out+=';';out+=captured[i].first+'='+array(captured[i].second,f);}out+='|';out+=result;out+='|';for(std::size_t i=0;i<f.trace.size();++i){if(i)out+=',';out+=f.trace[i];}return out;
}
suite mml_append_source=[] {
 "math_port_mml_append_actual_source"_test=[] {
  const Case cases[]={
        {"append-inferred-mrow-H-H-c","mrow","HHc","","append",false,"ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:001:001:U,U,n2;n4:mrow:N:1:1:1:n2|original=3:001:001:U,U,n2|n3|"},
        {"append-inferred-mfrac-H-H-c","mfrac","HHc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:001:001:U,U,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:001:001:U,U,n2||"},
        {"append-inferred-mrow-H-U-c","mrow","HUc","","append",false,"TypeError|Cannot set properties of undefined (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:011:011:U,U,n2;n4:mrow:N:1:1:1:U|original=3:011:011:U,U,n2||"},
        {"append-inferred-mfrac-H-U-c","mfrac","HUc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:011:011:U,U,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:011:011:U,U,n2||"},
        {"append-inferred-mrow-H-N-c","mrow","HNc","","append",false,"TypeError|Cannot set properties of null (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:011:011:U,N,n2;n4:mrow:N:1:1:1:N|original=3:011:011:U,N,n2||"},
        {"append-inferred-mfrac-H-N-c","mfrac","HNc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:011:011:U,N,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:011:011:U,N,n2||"},
        {"append-inferred-mrow-H-a-c","mrow","Hac","","append",false,"ok|n0:mi:n4:0:::;n1:mi:N:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:011:011:U,n0,n2;n4:mrow:N:2:11:11:n0,n2|original=3:011:011:U,n0,n2|n3|"},
        {"append-inferred-mfrac-H-a-c","mfrac","Hac","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:011:011:U,n0,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:011:011:U,n0,n2||"},
        {"append-inferred-mrow-H-b-c","mrow","Hbc","","append",false,"ok|n0:mi:N:0:::;n1:mi:n4:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:011:011:U,n1,n2;n4:mrow:N:2:11:11:n1,n2|original=3:011:011:U,n1,n2|n3|"},
        {"append-inferred-mfrac-H-b-c","mfrac","Hbc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:011:011:U,n1,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:011:011:U,n1,n2||"},
        {"append-inferred-mrow-U-H-c","mrow","UHc","","append",false,"TypeError|Cannot set properties of undefined (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:101:101:U,U,n2;n4:mrow:N:1:1:1:U|original=3:101:101:U,U,n2||"},
        {"append-inferred-mfrac-U-H-c","mfrac","UHc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:101:101:U,U,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:101:101:U,U,n2||"},
        {"append-inferred-mrow-U-U-c","mrow","UUc","","append",false,"TypeError|Cannot set properties of undefined (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:U,U,n2;n4:mrow:N:1:1:1:U|original=3:111:111:U,U,n2||"},
        {"append-inferred-mfrac-U-U-c","mfrac","UUc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:U,U,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:111:111:U,U,n2||"},
        {"append-inferred-mrow-U-N-c","mrow","UNc","","append",false,"TypeError|Cannot set properties of undefined (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:U,N,n2;n4:mrow:N:1:1:1:U|original=3:111:111:U,N,n2||"},
        {"append-inferred-mfrac-U-N-c","mfrac","UNc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:U,N,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:111:111:U,N,n2||"},
        {"append-inferred-mrow-U-a-c","mrow","Uac","","append",false,"TypeError|Cannot set properties of undefined (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:U,n0,n2;n4:mrow:N:1:1:1:U|original=3:111:111:U,n0,n2||"},
        {"append-inferred-mfrac-U-a-c","mfrac","Uac","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:U,n0,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:111:111:U,n0,n2||"},
        {"append-inferred-mrow-U-b-c","mrow","Ubc","","append",false,"TypeError|Cannot set properties of undefined (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:U,n1,n2;n4:mrow:N:1:1:1:U|original=3:111:111:U,n1,n2||"},
        {"append-inferred-mfrac-U-b-c","mfrac","Ubc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:U,n1,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:111:111:U,n1,n2||"},
        {"append-inferred-mrow-N-H-c","mrow","NHc","","append",false,"TypeError|Cannot set properties of null (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:101:101:N,U,n2;n4:mrow:N:1:1:1:N|original=3:101:101:N,U,n2||"},
        {"append-inferred-mfrac-N-H-c","mfrac","NHc","","append",false,"TypeError|Cannot read properties of null (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:101:101:N,U,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:101:101:N,U,n2||"},
        {"append-inferred-mrow-N-U-c","mrow","NUc","","append",false,"TypeError|Cannot set properties of null (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:N,U,n2;n4:mrow:N:1:1:1:N|original=3:111:111:N,U,n2||"},
        {"append-inferred-mfrac-N-U-c","mfrac","NUc","","append",false,"TypeError|Cannot read properties of null (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:N,U,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:111:111:N,U,n2||"},
        {"append-inferred-mrow-N-N-c","mrow","NNc","","append",false,"TypeError|Cannot set properties of null (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:N,N,n2;n4:mrow:N:1:1:1:N|original=3:111:111:N,N,n2||"},
        {"append-inferred-mfrac-N-N-c","mfrac","NNc","","append",false,"TypeError|Cannot read properties of null (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:N,N,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:111:111:N,N,n2||"},
        {"append-inferred-mrow-N-a-c","mrow","Nac","","append",false,"TypeError|Cannot set properties of null (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:N,n0,n2;n4:mrow:N:1:1:1:N|original=3:111:111:N,n0,n2||"},
        {"append-inferred-mfrac-N-a-c","mfrac","Nac","","append",false,"TypeError|Cannot read properties of null (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:N,n0,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:111:111:N,n0,n2||"},
        {"append-inferred-mrow-N-b-c","mrow","Nbc","","append",false,"TypeError|Cannot set properties of null (setting 'parent')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:N,n1,n2;n4:mrow:N:1:1:1:N|original=3:111:111:N,n1,n2||"},
        {"append-inferred-mfrac-N-b-c","mfrac","Nbc","","append",false,"TypeError|Cannot read properties of null (reading 'isInferred')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:N,n1,n2;n4:mfrac:N:0:::;n5:mrow:N:0:::|original=3:111:111:N,n1,n2||"},
        {"append-inferred-mrow-a-H-c","mrow","aHc","","append",false,"ok|n0:mi:n4:0:::;n1:mi:N:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:101:101:n0,U,n2;n4:mrow:N:2:11:11:n0,n2|original=3:101:101:n0,U,n2|n3|"},
        {"append-inferred-mfrac-a-H-c","mfrac","aHc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:n5:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:101:101:n0,U,n2;n4:mfrac:N:0:::;n5:mrow:N:1:1:1:n0|original=3:101:101:n0,U,n2||"},
        {"append-inferred-mrow-a-U-c","mrow","aUc","","append",false,"TypeError|Cannot set properties of undefined (setting 'parent')|n0:mi:n4:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:n0,U,n2;n4:mrow:N:2:11:11:n0,U|original=3:111:111:n0,U,n2||"},
        {"append-inferred-mfrac-a-U-c","mfrac","aUc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:n5:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:n0,U,n2;n4:mfrac:N:0:::;n5:mrow:N:1:1:1:n0|original=3:111:111:n0,U,n2||"},
        {"append-inferred-mrow-a-N-c","mrow","aNc","","append",false,"TypeError|Cannot set properties of null (setting 'parent')|n0:mi:n4:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:n0,N,n2;n4:mrow:N:2:11:11:n0,N|original=3:111:111:n0,N,n2||"},
        {"append-inferred-mfrac-a-N-c","mfrac","aNc","","append",false,"TypeError|Cannot read properties of null (reading 'isInferred')|n0:mi:n5:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:n0,N,n2;n4:mfrac:N:0:::;n5:mrow:N:1:1:1:n0|original=3:111:111:n0,N,n2||"},
        {"append-inferred-mrow-a-a-c","mrow","aac","","append",false,"ok|n0:mi:n4:0:::;n1:mi:N:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:111:111:n0,n0,n2;n4:mrow:N:3:111:111:n0,n0,n2|original=3:111:111:n0,n0,n2|n3|"},
        {"append-inferred-mfrac-a-a-c","mfrac","aac","","append",false,"ok|n0:mi:n5:0:::;n1:mi:N:0:::;n2:mi:n5:0:::;n3:inferredMrow:N:3:111:111:n0,n0,n2;n4:mfrac:N:1:1:1:n5;n5:mrow:n4:3:111:111:n0,n0,n2|original=3:111:111:n0,n0,n2|n5|"},
        {"append-inferred-mrow-a-b-c","mrow","abc","","append",false,"ok|n0:mi:n4:0:::;n1:mi:n4:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:111:111:n0,n1,n2;n4:mrow:N:3:111:111:n0,n1,n2|original=3:111:111:n0,n1,n2|n3|"},
        {"append-inferred-mfrac-a-b-c","mfrac","abc","","append",false,"ok|n0:mi:n5:0:::;n1:mi:n5:0:::;n2:mi:n5:0:::;n3:inferredMrow:N:3:111:111:n0,n1,n2;n4:mfrac:N:1:1:1:n5;n5:mrow:n4:3:111:111:n0,n1,n2|original=3:111:111:n0,n1,n2|n5|"},
        {"append-inferred-mrow-b-H-c","mrow","bHc","","append",false,"ok|n0:mi:N:0:::;n1:mi:n4:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:101:101:n1,U,n2;n4:mrow:N:2:11:11:n1,n2|original=3:101:101:n1,U,n2|n3|"},
        {"append-inferred-mfrac-b-H-c","mfrac","bHc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:n5:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:101:101:n1,U,n2;n4:mfrac:N:0:::;n5:mrow:N:1:1:1:n1|original=3:101:101:n1,U,n2||"},
        {"append-inferred-mrow-b-U-c","mrow","bUc","","append",false,"TypeError|Cannot set properties of undefined (setting 'parent')|n0:mi:N:0:::;n1:mi:n4:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:n1,U,n2;n4:mrow:N:2:11:11:n1,U|original=3:111:111:n1,U,n2||"},
        {"append-inferred-mfrac-b-U-c","mfrac","bUc","","append",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:N:0:::;n1:mi:n5:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:n1,U,n2;n4:mfrac:N:0:::;n5:mrow:N:1:1:1:n1|original=3:111:111:n1,U,n2||"},
        {"append-inferred-mrow-b-N-c","mrow","bNc","","append",false,"TypeError|Cannot set properties of null (setting 'parent')|n0:mi:N:0:::;n1:mi:n4:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:n1,N,n2;n4:mrow:N:2:11:11:n1,N|original=3:111:111:n1,N,n2||"},
        {"append-inferred-mfrac-b-N-c","mfrac","bNc","","append",false,"TypeError|Cannot read properties of null (reading 'isInferred')|n0:mi:N:0:::;n1:mi:n5:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:n1,N,n2;n4:mfrac:N:0:::;n5:mrow:N:1:1:1:n1|original=3:111:111:n1,N,n2||"},
        {"append-inferred-mrow-b-a-c","mrow","bac","","append",false,"ok|n0:mi:n4:0:::;n1:mi:n4:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:111:111:n1,n0,n2;n4:mrow:N:3:111:111:n1,n0,n2|original=3:111:111:n1,n0,n2|n3|"},
        {"append-inferred-mfrac-b-a-c","mfrac","bac","","append",false,"ok|n0:mi:n5:0:::;n1:mi:n5:0:::;n2:mi:n5:0:::;n3:inferredMrow:N:3:111:111:n1,n0,n2;n4:mfrac:N:1:1:1:n5;n5:mrow:n4:3:111:111:n1,n0,n2|original=3:111:111:n1,n0,n2|n5|"},
        {"append-inferred-mrow-b-b-c","mrow","bbc","","append",false,"ok|n0:mi:N:0:::;n1:mi:n4:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:111:111:n1,n1,n2;n4:mrow:N:3:111:111:n1,n1,n2|original=3:111:111:n1,n1,n2|n3|"},
        {"append-inferred-mfrac-b-b-c","mfrac","bbc","","append",false,"ok|n0:mi:N:0:::;n1:mi:n5:0:::;n2:mi:n5:0:::;n3:inferredMrow:N:3:111:111:n1,n1,n2;n4:mfrac:N:1:1:1:n5;n5:mrow:n4:3:111:111:n1,n1,n2|original=3:111:111:n1,n1,n2|n5|"},
        {"append-inferred-prototype-false","mrow","aHc","","append",false,"ok|n0:mi:n4:0:::;n1:mi:N:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:101:101:n0,U,n2;n4:mrow:N:2:11:11:n0,n2|original=3:101:101:n0,U,n2|n3|"},
        {"append-inferred-prototype-true","mrow","aHc","","append",true,"ok|n0:mi:n4:0:::;n1:mi:n4:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:101:111:n0,n1,n2;n4:mrow:N:3:111:111:n0,n1,n2|original=3:101:111:n0,n1,n2|n3|"},
        {"append-inferred-parent-setter-append","mrow","ab","append","append",false,"ok|n0:mi:n4:0:::;n1:mi:n4:0:::;n2:mi:N:0:::;n3:inferredMrow:N:3:111:111:n0,n1,n2;n4:mrow:N:2:11:11:n0,n1|original=3:111:111:n0,n1,n2;target=2:11:11:n0,n1|n3|set:n4"},
        {"append-inferred-parent-setter-delete","mrow","ab","delete","append",false,"ok|n0:mi:n4:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:2:10:10:n0,U;n4:mrow:N:1:1:1:n0|original=2:10:10:n0,U;target=1:1:1:n0|n3|set:n4"},
        {"append-inferred-parent-setter-shrink","mrow","ab","shrink","append",false,"ok|n0:mi:n4:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:1:1:1:n0;n4:mrow:N:1:1:1:n0|original=1:1:1:n0;target=1:1:1:n0|n3|set:n4"},
        {"append-inferred-parent-setter-rebind-source","mrow","ab","rebind-source","append",false,"ok|n0:mi:n4:0:::;n1:mi:n4:0:::;n2:mi:N:0:::;n3:inferredMrow:N:1:1:1:n2;n4:mrow:N:2:11:11:n0,n1|original=2:11:11:n0,n1;target=2:11:11:n0,n1|n3|set:n4"},
        {"append-inferred-parent-setter-rebind-target","mrow","ab","rebind-target","append",false,"ok|n0:mi:n4:0:::;n1:mi:n4:0:::;n2:mi:N:0:::;n3:inferredMrow:N:2:11:11:n0,n1;n4:mrow:N:2:11:11:n2,n1|original=2:11:11:n0,n1;target=1:1:1:n0|n3|set:n4"},
        {"setChildren-parent-setter-append","mrow","ab","append","set",false,"ok|n0:mi:n4:0:::;n1:mi:n4:0:::;n2:mi:n4:0:::;n3:inferredMrow:N:3:111:111:n0,n1,n2;n4:mrow:N:3:111:111:n0,n1,n2|original=3:111:111:n0,n1,n2;target=0:::||set:n4"},
        {"setChildren-parent-setter-delete","mrow","ab","delete","set",false,"TypeError|Cannot read properties of undefined (reading 'isInferred')|n0:mi:n4:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:2:10:10:n0,U;n4:mrow:N:1:1:1:n0|original=2:10:10:n0,U;target=0:::||set:n4"},
        {"setChildren-parent-setter-shrink","mrow","ab","shrink","set",false,"ok|n0:mi:n4:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:inferredMrow:N:1:1:1:n0;n4:mrow:N:1:1:1:n0|original=1:1:1:n0;target=0:::||set:n4"},
        {"setChildren-parent-setter-rebind-source","mrow","ab","rebind-source","set",false,"ok|n0:mi:n4:0:::;n1:mi:n4:0:::;n2:mi:N:0:::;n3:inferredMrow:N:1:1:1:n2;n4:mrow:N:2:11:11:n0,n1|original=2:11:11:n0,n1;target=0:::||set:n4"},
        {"setChildren-parent-setter-rebind-target","mrow","ab","rebind-target","set",false,"ok|n0:mi:n4:0:::;n1:mi:n4:0:::;n2:mi:N:0:::;n3:inferredMrow:N:2:11:11:n0,n1;n4:mrow:N:2:11:11:n2,n1|original=2:11:11:n0,n1;target=0:::||set:n4"},
  };for(const auto& c:cases)expect(eq(run(c),c.expected))<<c.name;
 };
};
}
