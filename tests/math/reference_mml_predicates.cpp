// Root actual full-registry Source299 predicate subset; no boolean fixture substitution.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
namespace {
using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct Observed:MmlFactory{
 std::vector<MmlNode*> nodes;
 MmlNode* create(std::string_view kind,tree::FactoryArgs args)override{auto* n=MmlFactory::create(kind,args);nodes.push_back(n);return n;}
 std::string tag(NodeValue v)const{if(v.kind()==tree::ChildKind::Undefined)return "U";if(v.kind()==tree::ChildKind::Null)return "N";auto it=std::find(nodes.begin(),nodes.end(),v.nodeOrNull());return it==nodes.end()?"UNBOUND":"n"+std::to_string(it-nodes.begin());}
};
struct Case{std::string name,member,shape,expected;};
std::string ascii(std::u16string_view s){std::string v;for(auto c:s)v+=static_cast<char>(c);return v;}
std::string array(const Children& a,const Observed& f){std::string own,has,values;for(std::size_t i=0;i<a->size();++i){own+=a->hasOwn(i)?'1':'0';has+=a->has(i)?'1':'0';if(i)values+=',';values+=f.tag(a->get(i));}return std::to_string(a->size())+':'+own+':'+has+':'+values;}
std::string run(const Case& c){Observed f;std::array<MmlNode*,3> seed;for(auto& n:seed)n=f.create("mi",{});auto* node=f.create("mrow",{});auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->resize(c.shape.size());for(std::size_t i=0;i<c.shape.size();++i){auto v=c.shape[i];if(v!='H')a->set(i,v=='U'?NodeValue(Missing{}):v=='N'?NodeValue(PropertyNull{}):NodeValue(seed[v-'a']));}node->childNodes=a;
 std::string error="ok",result;try{bool v=c.member=="isEmpty"?node->isEmpty():c.member=="isSpacelike"?node->isSpacelike():node->isEmbellished();result=v?"true":"false";}catch(const Error& e){error=e.name+'|'+ascii(e.message);}
 std::string out=error+'|';for(std::size_t i=0;i<f.nodes.size();++i){if(i)out+=';';auto* n=f.nodes[i];out+=f.tag(n)+':'+n->kind()+':'+f.tag(n->getParent())+':'+array(n->childNodes,f);}return out+'|'+result;
}
suite mml_predicate_source=[] {"math_port_mml_predicates_actual_source"_test=[] {const Case cases[]={
        {"mrow-isEmpty-H-H-c","isEmpty","HHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:001:001:U,U,n2|true"},
        {"mrow-isSpacelike-H-H-c","isSpacelike","HHc","TypeError|Cannot read properties of undefined (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:001:001:U,U,n2|"},
        {"mrow-isEmbellished-H-H-c","isEmbellished","HHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:001:001:U,U,n2|false"},
        {"mrow-isEmpty-H-U-c","isEmpty","HUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,U,n2|true"},
        {"mrow-isSpacelike-H-U-c","isSpacelike","HUc","TypeError|Cannot read properties of undefined (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,U,n2|"},
        {"mrow-isEmbellished-H-U-c","isEmbellished","HUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,U,n2|false"},
        {"mrow-isEmpty-H-N-c","isEmpty","HNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,N,n2|true"},
        {"mrow-isSpacelike-H-N-c","isSpacelike","HNc","TypeError|Cannot read properties of undefined (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,N,n2|"},
        {"mrow-isEmbellished-H-N-c","isEmbellished","HNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,N,n2|false"},
        {"mrow-isEmpty-H-a-c","isEmpty","Hac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,n0,n2|true"},
        {"mrow-isSpacelike-H-a-c","isSpacelike","Hac","TypeError|Cannot read properties of undefined (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,n0,n2|"},
        {"mrow-isEmbellished-H-a-c","isEmbellished","Hac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,n0,n2|false"},
        {"mrow-isEmpty-H-b-c","isEmpty","Hbc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,n1,n2|true"},
        {"mrow-isSpacelike-H-b-c","isSpacelike","Hbc","TypeError|Cannot read properties of undefined (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,n1,n2|"},
        {"mrow-isEmbellished-H-b-c","isEmbellished","Hbc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:011:011:U,n1,n2|false"},
        {"mrow-isEmpty-U-H-c","isEmpty","UHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:U,U,n2|true"},
        {"mrow-isSpacelike-U-H-c","isSpacelike","UHc","TypeError|Cannot read properties of undefined (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:U,U,n2|"},
        {"mrow-isEmbellished-U-H-c","isEmbellished","UHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:U,U,n2|false"},
        {"mrow-isEmpty-U-U-c","isEmpty","UUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,U,n2|true"},
        {"mrow-isSpacelike-U-U-c","isSpacelike","UUc","TypeError|Cannot read properties of undefined (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,U,n2|"},
        {"mrow-isEmbellished-U-U-c","isEmbellished","UUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,U,n2|false"},
        {"mrow-isEmpty-U-N-c","isEmpty","UNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,N,n2|true"},
        {"mrow-isSpacelike-U-N-c","isSpacelike","UNc","TypeError|Cannot read properties of undefined (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,N,n2|"},
        {"mrow-isEmbellished-U-N-c","isEmbellished","UNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,N,n2|false"},
        {"mrow-isEmpty-U-a-c","isEmpty","Uac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,n0,n2|true"},
        {"mrow-isSpacelike-U-a-c","isSpacelike","Uac","TypeError|Cannot read properties of undefined (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,n0,n2|"},
        {"mrow-isEmbellished-U-a-c","isEmbellished","Uac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,n0,n2|false"},
        {"mrow-isEmpty-U-b-c","isEmpty","Ubc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,n1,n2|true"},
        {"mrow-isSpacelike-U-b-c","isSpacelike","Ubc","TypeError|Cannot read properties of undefined (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,n1,n2|"},
        {"mrow-isEmbellished-U-b-c","isEmbellished","Ubc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:U,n1,n2|false"},
        {"mrow-isEmpty-N-H-c","isEmpty","NHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:N,U,n2|true"},
        {"mrow-isSpacelike-N-H-c","isSpacelike","NHc","TypeError|Cannot read properties of null (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:N,U,n2|"},
        {"mrow-isEmbellished-N-H-c","isEmbellished","NHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:N,U,n2|false"},
        {"mrow-isEmpty-N-U-c","isEmpty","NUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,U,n2|true"},
        {"mrow-isSpacelike-N-U-c","isSpacelike","NUc","TypeError|Cannot read properties of null (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,U,n2|"},
        {"mrow-isEmbellished-N-U-c","isEmbellished","NUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,U,n2|false"},
        {"mrow-isEmpty-N-N-c","isEmpty","NNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,N,n2|true"},
        {"mrow-isSpacelike-N-N-c","isSpacelike","NNc","TypeError|Cannot read properties of null (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,N,n2|"},
        {"mrow-isEmbellished-N-N-c","isEmbellished","NNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,N,n2|false"},
        {"mrow-isEmpty-N-a-c","isEmpty","Nac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,n0,n2|true"},
        {"mrow-isSpacelike-N-a-c","isSpacelike","Nac","TypeError|Cannot read properties of null (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,n0,n2|"},
        {"mrow-isEmbellished-N-a-c","isEmbellished","Nac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,n0,n2|false"},
        {"mrow-isEmpty-N-b-c","isEmpty","Nbc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,n1,n2|true"},
        {"mrow-isSpacelike-N-b-c","isSpacelike","Nbc","TypeError|Cannot read properties of null (reading 'isSpacelike')|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,n1,n2|"},
        {"mrow-isEmbellished-N-b-c","isEmbellished","Nbc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:N,n1,n2|false"},
        {"mrow-isEmpty-a-H-c","isEmpty","aHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:n0,U,n2|true"},
        {"mrow-isSpacelike-a-H-c","isSpacelike","aHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:n0,U,n2|false"},
        {"mrow-isEmbellished-a-H-c","isEmbellished","aHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:n0,U,n2|false"},
        {"mrow-isEmpty-a-U-c","isEmpty","aUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,U,n2|true"},
        {"mrow-isSpacelike-a-U-c","isSpacelike","aUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,U,n2|false"},
        {"mrow-isEmbellished-a-U-c","isEmbellished","aUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,U,n2|false"},
        {"mrow-isEmpty-a-N-c","isEmpty","aNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,N,n2|true"},
        {"mrow-isSpacelike-a-N-c","isSpacelike","aNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,N,n2|false"},
        {"mrow-isEmbellished-a-N-c","isEmbellished","aNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,N,n2|false"},
        {"mrow-isEmpty-a-a-c","isEmpty","aac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,n0,n2|true"},
        {"mrow-isSpacelike-a-a-c","isSpacelike","aac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,n0,n2|false"},
        {"mrow-isEmbellished-a-a-c","isEmbellished","aac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,n0,n2|false"},
        {"mrow-isEmpty-a-b-c","isEmpty","abc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,n1,n2|true"},
        {"mrow-isSpacelike-a-b-c","isSpacelike","abc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,n1,n2|false"},
        {"mrow-isEmbellished-a-b-c","isEmbellished","abc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n0,n1,n2|false"},
        {"mrow-isEmpty-b-H-c","isEmpty","bHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:n1,U,n2|true"},
        {"mrow-isSpacelike-b-H-c","isSpacelike","bHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:n1,U,n2|false"},
        {"mrow-isEmbellished-b-H-c","isEmbellished","bHc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:101:101:n1,U,n2|false"},
        {"mrow-isEmpty-b-U-c","isEmpty","bUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,U,n2|true"},
        {"mrow-isSpacelike-b-U-c","isSpacelike","bUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,U,n2|false"},
        {"mrow-isEmbellished-b-U-c","isEmbellished","bUc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,U,n2|false"},
        {"mrow-isEmpty-b-N-c","isEmpty","bNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,N,n2|true"},
        {"mrow-isSpacelike-b-N-c","isSpacelike","bNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,N,n2|false"},
        {"mrow-isEmbellished-b-N-c","isEmbellished","bNc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,N,n2|false"},
        {"mrow-isEmpty-b-a-c","isEmpty","bac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,n0,n2|true"},
        {"mrow-isSpacelike-b-a-c","isSpacelike","bac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,n0,n2|false"},
        {"mrow-isEmbellished-b-a-c","isEmbellished","bac","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,n0,n2|false"},
        {"mrow-isEmpty-b-b-c","isEmpty","bbc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,n1,n2|true"},
        {"mrow-isSpacelike-b-b-c","isSpacelike","bbc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,n1,n2|false"},
        {"mrow-isEmbellished-b-b-c","isEmbellished","bbc","ok|n0:mi:N:0:::;n1:mi:N:0:::;n2:mi:N:0:::;n3:mrow:N:3:111:111:n1,n1,n2|false"},
 };for(const auto& c:cases)expect(eq(run(c),c.expected))<<c.name;};};
}
