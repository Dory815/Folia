// Root generated original operator access and capability errors.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mo_host;
import folia.math.core.mml_tree.mml_nodes.mo;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;namespace h=host;std::string mode;NodeValue accessCoreValue;struct Probe:MmlMo{using MmlMo::MmlMo;MmlNode*coreParent()override{return mode=="null-core-parent"?nullptr:MmlMo::coreParent();}};struct Part:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "access-part";}bool isEmbellished()const override{return mode.starts_with("emb-");}bool isToken()const override{return mode=="token-lie";}NodeValue coreMO()override{return accessCoreValue;}};struct Fake:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "access-fake";}bool isKind(std::string_view k)const override{return k=="mover";}};
struct Case{std::string_view mode,expected;};std::string run(const Case&i){mode=i.mode;MmlFactory f;f.setNodeClass("access-probe",classBinding<Probe>());f.setNodeClass("access-part",classBinding<Part>());f.setNodeClass("access-fake",classBinding<Fake>());auto*mo=dynamic_cast<Probe*>(f.create("access-probe"));auto*part=f.create("access-part");auto*other=f.create("access-part");auto*text=dynamic_cast<TextNode*>(f.create("text"));text->setText(u"t");auto*token=f.create("mi");auto*tokenText=dynamic_cast<TextNode*>(f.create("text"));tokenText->setText(u"token");token->appendChild(tokenText);accessCoreValue=mode=="emb-U"?NodeValue(Missing{}):mode=="emb-N"?NodeValue(PropertyNull{}):mode=="emb-text"?NodeValue(text):NodeValue(other);std::string error="ok",result="THREW";try{if(mode=="math-missing"){f.deleteNodeClass("math");result=mo->coreParent()==mo?"mo":"other";}else if(mode=="null-core-parent")result=h::debugText(h::string(mo->isAccent()));else if(mode.starts_with("plain-index")){auto*p=f.create("access-fake");if(mode=="plain-index-nullarray")p->childNodes.reset();mo->setParent(p);result=h::debugText(h::string(mo->isAccent()));}else result=h::debugText(mo->coreText(mode=="input-U"?NodeValue(Missing{}):mode=="input-N"?NodeValue(PropertyNull{}):mode=="token"?NodeValue(token):NodeValue(part)));}catch(const Error&e){error=e.name+"|"+h::debugText(e.message);}return error+"|"+result;}
suite operatorAccess=[] {"math_port_operator_exact_access_source"_test=[] {const Case cases[]={
{"emb-U","TypeError|Cannot read properties of undefined (reading 'getText')|THREW"},
{"emb-N","TypeError|Cannot read properties of null (reading 'getText')|THREW"},
{"emb-base","TypeError|parent.coreMO(...).getText is not a function|THREW"},
{"emb-text","ok|t"},
{"token-lie","TypeError|parent.getText is not a function|THREW"},
{"token","ok|token"},
{"math-missing","TypeError|Right-hand side of 'instanceof' is not an object|THREW"},
{"null-core-parent","TypeError|Cannot read properties of null (reading 'parent')|THREW"},
{"plain-index","TypeError|Cannot read properties of undefined (reading 'coreMO')|THREW"},
{"plain-index-nullarray","TypeError|Cannot read properties of null (reading 'undefined')|THREW"},
{"input-U","ok|"},
{"input-N","ok|"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.mode;};};}
