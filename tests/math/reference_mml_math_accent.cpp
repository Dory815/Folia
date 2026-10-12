// Root generated original __read bounds and accent callback failures.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mo_host;
import folia.math.core.mml_tree.mml_nodes.mo;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;namespace h=host;bool active=false;std::string policy;std::vector<std::string>trace;MmlNode*mo=nullptr,*parent=nullptr;void note(std::string s){if(active)trace.push_back(s);}struct Probe:MmlMo{using MmlMo::MmlMo;NodeValue Parent()const override{note("Parent");return ::parent;}Property getProperty(std::string_view k)const override{note("get/"+std::string(k));return MmlMo::getProperty(k);}void setProperty(std::string_view k,Property v)override{note("set/"+std::string(k)+"/"+h::debugText(h::string(v)));MmlMo::setProperty(k,v);}void hook(){MmlMo::checkMathAccent(u"x");}bool isMathAccent(std::optional<std::u16string_view> text={})override{note("accent");return policy=="true";}bool isMathAccentWithWidth(std::optional<std::u16string_view> text={})override{note("width");return policy=="width";}};struct Part:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string label;std::string kind()const override{return "accent-part";}bool isEmbellished()const override{note("emb/"+label);return label!="base";}NodeValue coreMO()override{note("core/"+label);return mo;}};
struct Case{std::size_t size;std::string_view shape,policy,expected;};std::string run(const Case&i){active=false;trace.clear();policy=i.policy;MmlFactory f;f.setNodeClass("probe-accent",classBinding<Probe>());f.setNodeClass("accent-part",classBinding<Part>());auto*probe=dynamic_cast<Probe*>(f.create("probe-accent"));mo=probe;parent=f.create("munderover");std::array<Part*,3>parts;std::array<std::string,3>names{"base","under","over"};for(std::size_t j=0;j<3;j++){parts[j]=dynamic_cast<Part*>(f.create("accent-part"));parts[j]->label=names[j];}auto a=std::make_shared<tree::ChildArray<MmlNode>>();a->resize(i.size);if(i.shape=="prototype"){auto proto=std::make_shared<tree::ChildArray<MmlNode>>();for(auto*p:parts)proto->push_back(p);a->setPrototype(proto);}else for(std::size_t j=0;j<i.size;j++)a->set(j,parts[j]);if(i.shape=="null-array")a.reset();if(i.shape=="null-base")a->set(0,PropertyNull{});if(i.shape=="over-only")a->erase(1);parent->childNodes=a;std::string error="ok";active=true;try{probe->hook();}catch(const Error&e){error=e.name+"|"+h::debugText(e.message);}active=false;std::string events;for(const auto&s:trace){if(!events.empty())events+=',';events+=s;}return error+"|"+h::debugText(h::string(probe->getProperty("mathaccent")))+"|"+events;}
suite mathAccent=[] {"math_port_math_accent_iterator_source"_test=[] {const Case cases[]={
{0,"own","true","TypeError|Cannot read properties of undefined (reading 'isEmbellished')|undefined|Parent,get/mathaccent"},
{0,"own","width","TypeError|Cannot read properties of undefined (reading 'isEmbellished')|undefined|Parent,get/mathaccent"},
{0,"own","none","TypeError|Cannot read properties of undefined (reading 'isEmbellished')|undefined|Parent,get/mathaccent"},
{0,"prototype","true","TypeError|Cannot read properties of undefined (reading 'isEmbellished')|undefined|Parent,get/mathaccent"},
{0,"prototype","width","TypeError|Cannot read properties of undefined (reading 'isEmbellished')|undefined|Parent,get/mathaccent"},
{0,"prototype","none","TypeError|Cannot read properties of undefined (reading 'isEmbellished')|undefined|Parent,get/mathaccent"},
{1,"own","true","ok|undefined|Parent,get/mathaccent,emb/base"},
{1,"own","width","ok|undefined|Parent,get/mathaccent,emb/base"},
{1,"own","none","ok|undefined|Parent,get/mathaccent,emb/base"},
{1,"prototype","true","ok|undefined|Parent,get/mathaccent,emb/base"},
{1,"prototype","width","ok|undefined|Parent,get/mathaccent,emb/base"},
{1,"prototype","none","ok|undefined|Parent,get/mathaccent,emb/base"},
{2,"own","true","ok|true|Parent,get/mathaccent,emb/base,emb/under,core/under,accent,set/mathaccent/true"},
{2,"own","width","ok|false|Parent,get/mathaccent,emb/base,emb/under,core/under,accent,width,set/mathaccent/false"},
{2,"own","none","ok|undefined|Parent,get/mathaccent,emb/base,emb/under,core/under,accent,width"},
{2,"prototype","true","ok|true|Parent,get/mathaccent,emb/base,emb/under,core/under,accent,set/mathaccent/true"},
{2,"prototype","width","ok|false|Parent,get/mathaccent,emb/base,emb/under,core/under,accent,width,set/mathaccent/false"},
{2,"prototype","none","ok|undefined|Parent,get/mathaccent,emb/base,emb/under,core/under,accent,width"},
{3,"own","true","ok|true|Parent,get/mathaccent,emb/base,emb/under,core/under,emb/over,core/under,accent,set/mathaccent/true"},
{3,"own","width","ok|false|Parent,get/mathaccent,emb/base,emb/under,core/under,emb/over,core/under,accent,width,set/mathaccent/false"},
{3,"own","none","ok|undefined|Parent,get/mathaccent,emb/base,emb/under,core/under,emb/over,core/under,accent,width"},
{3,"prototype","true","ok|true|Parent,get/mathaccent,emb/base,emb/under,core/under,emb/over,core/under,accent,set/mathaccent/true"},
{3,"prototype","width","ok|false|Parent,get/mathaccent,emb/base,emb/under,core/under,emb/over,core/under,accent,width,set/mathaccent/false"},
{3,"prototype","none","ok|undefined|Parent,get/mathaccent,emb/base,emb/under,core/under,emb/over,core/under,accent,width"},
{3,"null-array","true","TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))|undefined|Parent,get/mathaccent"},
{3,"null-array","width","TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))|undefined|Parent,get/mathaccent"},
{3,"null-array","none","TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))|undefined|Parent,get/mathaccent"},
{3,"null-base","true","TypeError|Cannot read properties of null (reading 'isEmbellished')|undefined|Parent,get/mathaccent"},
{3,"null-base","width","TypeError|Cannot read properties of null (reading 'isEmbellished')|undefined|Parent,get/mathaccent"},
{3,"null-base","none","TypeError|Cannot read properties of null (reading 'isEmbellished')|undefined|Parent,get/mathaccent"},
{3,"over-only","true","TypeError|Cannot read properties of undefined (reading 'coreMO')|undefined|Parent,get/mathaccent,emb/base,emb/over"},
{3,"over-only","width","TypeError|Cannot read properties of undefined (reading 'coreMO')|undefined|Parent,get/mathaccent,emb/base,emb/over"},
{3,"over-only","none","TypeError|Cannot read properties of undefined (reading 'coreMO')|undefined|Parent,get/mathaccent,emb/base,emb/over"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.size<<i.shape<<i.policy;};};}
