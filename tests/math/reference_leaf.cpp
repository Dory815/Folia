// Generated from actual pinned MathJax classes by leaf-oracle.cjs. Do not hand-edit.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.class_binding;
import folia.math.core.tree.node_factory;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mn;
import folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.core.mml_tree.mml_nodes.ms;
import folia.math.core.mml_tree.mml_nodes.mspace;
import folia.math.core.mml_tree.mml_nodes.mstyle;
import folia.math.core.mml_tree.mml_nodes.merror;
import folia.math.core.mml_tree.mml_nodes.mphantom;
import folia.math.core.mml_tree.leaf_host;
namespace m=folia::math::mml;namespace tr=folia::math::tree;namespace h=m::host;using folia::math::Property;using folia::math::PropertyList;using folia::math::PropertyNull;using folia::math::Missing;using namespace boost::ut;namespace {
std::u16string units(std::initializer_list<unsigned> v){std::u16string s;for(auto x:v)s.push_back(static_cast<char16_t>(x));return s;}
bool prop(const Property&a,const Property&b){if(auto x=std::get_if<double>(&a.value))if(auto y=std::get_if<double>(&b.value))return std::isnan(*y)?std::isnan(*x):std::bit_cast<std::uint64_t>(*x)==std::bit_cast<std::uint64_t>(*y);return a==b;}
m::Children children(std::initializer_list<m::MmlNode*> v={}){return std::make_shared<std::vector<m::MmlNode*>>(v);}
m::MmlNode* at(m::MmlNode*n,std::size_t i){return n&&n->childNodes&&i<n->childNodes->size()?(*n->childNodes)[i]:nullptr;}
std::size_t count(m::MmlNode*n){return n&&n->childNodes?n->childNodes->size():0;}
struct Probe:m::AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::function<void(m::MmlNode*)> onClass;std::function<void(m::AttributeList,Property,Property,Property)> onInherit;std::string kind()const override{return "probe";}bool isSpacelike()const override{return h::truthy(getProperty("space"));}bool isEmbellished()const override{return h::truthy(getProperty("embellished"));}m::MmlNode* setTeXclass(m::MmlNode*p)override{if(onClass)onClass(p);return AbstractMmlNode::setTeXclass(p);}void setInheritedAttributes(m::AttributeList a={},Property d=false,Property l=0,Property p=false)override{if(onInherit)onInherit(a,d,l,p);AbstractMmlNode::setInheritedAttributes(a,d,l,p);}};
using F=tr::AbstractNodeFactory<m::MmlNode>;tr::FactoryClassMap<m::MmlNode> classes(){return {{"mrow",m::classBinding<m::MmlMrow>()},{"inferredMrow",m::classBinding<m::MmlInferredMrow>()},{"math",m::classBinding<m::MmlMath>()},{"mi",m::classBinding<m::MmlMi>()},{"mn",m::classBinding<m::MmlMn>()},{"mtext",m::classBinding<m::MmlMtext>()},{"ms",m::classBinding<m::MmlMs>()},{"mspace",m::classBinding<m::MmlMspace>()},{"mstyle",m::classBinding<m::MmlMstyle>()},{"merror",m::classBinding<m::MmlMerror>()},{"mphantom",m::classBinding<m::MmlMphantom>()},{"text",m::classBinding<m::TextNode>()},{"XML",m::classBinding<m::XMLNode>()},{"probe",m::classBinding<Probe>()}};}
m::MmlNode* create(F&f,std::string_view kind,PropertyList a={},m::Children c=children()){return f.create(kind,std::array<std::any,2>{a,c});}
m::TextNode* text(F&f,std::u16string value){auto*n=dynamic_cast<m::TextNode*>(f.create("text"));n->setText(std::move(value));return n;}
template<class Make>std::string bits(Make make){constexpr char hex[]="0123456789abcdef";std::string out;for(unsigned start=0;start<65536;start+=4){unsigned b=0;for(unsigned j=0;j<4;++j)if(make(static_cast<char16_t>(start+j)))b|=1u<<j;out.push_back(hex[b]);}return out;}
}
suite math_port_leaf_tests=[] {
"math_port_leaf_defaults_traits_and_constructor_fields"_test=[] {F f(classes());
{auto*n=f.create("mrow");
expect(bool((n->kind())=="mrow"));
expect(bool((n->isToken())==false));
expect(bool((n->isInferred())==false));
expect(bool((n->notParent())==false));
expect(bool((n->arity())==std::numeric_limits<double>::infinity()));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->linebreakAlign())=="data-align"));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==true));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(PropertyNull{})));
expect(bool(m::MmlMrow::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir"}));
expect(prop(m::MmlMrow::defaults().getOwn("mathbackground"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMrow::defaults().getOwn("mathcolor"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMrow::defaults().getOwn("mathsize"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMrow::defaults().getOwn("dir"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(bool((count(n))==0.0));
}
{auto*n=f.create("inferredMrow");
expect(bool((n->kind())=="inferredMrow"));
expect(bool((n->isToken())==false));
expect(bool((n->isInferred())==true));
expect(bool((n->notParent())==true));
expect(bool((n->arity())==std::numeric_limits<double>::infinity()));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->linebreakAlign())=="data-align"));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==true));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(PropertyNull{})));
expect(bool(m::MmlInferredMrow::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir"}));
expect(prop(m::MmlInferredMrow::defaults().getOwn("mathbackground"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlInferredMrow::defaults().getOwn("mathcolor"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlInferredMrow::defaults().getOwn("mathsize"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlInferredMrow::defaults().getOwn("dir"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(bool((count(n))==0.0));
}
{auto*n=f.create("math");
expect(bool((n->kind())=="math"));
expect(bool((n->isToken())==false));
expect(bool((n->isInferred())==false));
expect(bool((n->notParent())==false));
expect(bool((n->arity())==-1.0));
expect(bool((n->linebreakContainer())==true));
expect(bool((n->linebreakAlign())==""));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==true));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(PropertyNull{})));
expect(bool(m::MmlMath::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir","mathvariant","scriptlevel","displaystyle","display","maxwidth","overflow","altimg","altimg-width","altimg-height","altimg-valign","alttext","cdgroup","scriptsizemultiplier","scriptminsize","infixlinebreakstyle","lineleading","linebreakmultchar","indentshift","indentalign","indenttarget","indentalignfirst","indentshiftfirst","indentalignlast","indentshiftlast"}));
expect(prop(m::MmlMath::defaults().getOwn("mathbackground"),Property(h::debugText(units({116,114,97,110,115,112,97,114,101,110,116})))));
expect(prop(m::MmlMath::defaults().getOwn("mathcolor"),Property(h::debugText(units({})))));
expect(prop(m::MmlMath::defaults().getOwn("mathsize"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(m::MmlMath::defaults().getOwn("dir"),Property(h::debugText(units({108,116,114})))));
expect(prop(m::MmlMath::defaults().getOwn("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(m::MmlMath::defaults().getOwn("scriptlevel"),Property(0.0)));
expect(prop(m::MmlMath::defaults().getOwn("displaystyle"),Property(false)));
expect(prop(m::MmlMath::defaults().getOwn("display"),Property(h::debugText(units({105,110,108,105,110,101})))));
expect(prop(m::MmlMath::defaults().getOwn("maxwidth"),Property(h::debugText(units({})))));
expect(prop(m::MmlMath::defaults().getOwn("overflow"),Property(h::debugText(units({108,105,110,101,98,114,101,97,107})))));
expect(prop(m::MmlMath::defaults().getOwn("altimg"),Property(h::debugText(units({})))));
expect(prop(m::MmlMath::defaults().getOwn("altimg-width"),Property(h::debugText(units({})))));
expect(prop(m::MmlMath::defaults().getOwn("altimg-height"),Property(h::debugText(units({})))));
expect(prop(m::MmlMath::defaults().getOwn("altimg-valign"),Property(h::debugText(units({})))));
expect(prop(m::MmlMath::defaults().getOwn("alttext"),Property(h::debugText(units({})))));
expect(prop(m::MmlMath::defaults().getOwn("cdgroup"),Property(h::debugText(units({})))));
expect(prop(m::MmlMath::defaults().getOwn("scriptsizemultiplier"),Property(0.7071067811865475)));
expect(prop(m::MmlMath::defaults().getOwn("scriptminsize"),Property(h::debugText(units({46,52,101,109})))));
expect(prop(m::MmlMath::defaults().getOwn("infixlinebreakstyle"),Property(h::debugText(units({98,101,102,111,114,101})))));
expect(prop(m::MmlMath::defaults().getOwn("lineleading"),Property(h::debugText(units({49,48,48,37})))));
expect(prop(m::MmlMath::defaults().getOwn("linebreakmultchar"),Property(h::debugText(units({8290})))));
expect(prop(m::MmlMath::defaults().getOwn("indentshift"),Property(h::debugText(units({97,117,116,111})))));
expect(prop(m::MmlMath::defaults().getOwn("indentalign"),Property(h::debugText(units({97,117,116,111})))));
expect(prop(m::MmlMath::defaults().getOwn("indenttarget"),Property(h::debugText(units({})))));
expect(prop(m::MmlMath::defaults().getOwn("indentalignfirst"),Property(h::debugText(units({105,110,100,101,110,116,97,108,105,103,110})))));
expect(prop(m::MmlMath::defaults().getOwn("indentshiftfirst"),Property(h::debugText(units({105,110,100,101,110,116,115,104,105,102,116})))));
expect(prop(m::MmlMath::defaults().getOwn("indentalignlast"),Property(h::debugText(units({105,110,100,101,110,116,97,108,105,103,110})))));
expect(prop(m::MmlMath::defaults().getOwn("indentshiftlast"),Property(h::debugText(units({105,110,100,101,110,116,115,104,105,102,116})))));
expect(bool((count(n))==1.0));
expect(bool((at(n,0)->kind())=="inferredMrow"));
}
{auto*n=f.create("mi");
expect(bool((n->kind())=="mi"));
expect(bool((n->isToken())==true));
expect(bool((n->isInferred())==false));
expect(bool((n->notParent())==false));
expect(bool((n->arity())==std::numeric_limits<double>::infinity()));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->linebreakAlign())=="data-align"));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==false));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(0.0)));
expect(bool(m::MmlMi::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir","mathvariant"}));
expect(prop(m::MmlMi::defaults().getOwn("mathbackground"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMi::defaults().getOwn("mathcolor"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMi::defaults().getOwn("mathsize"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMi::defaults().getOwn("dir"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMi::defaults().getOwn("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(bool((count(n))==0.0));
}
{auto*n=f.create("mn");
expect(bool((n->kind())=="mn"));
expect(bool((n->isToken())==true));
expect(bool((n->isInferred())==false));
expect(bool((n->notParent())==false));
expect(bool((n->arity())==std::numeric_limits<double>::infinity()));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->linebreakAlign())=="data-align"));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==false));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(0.0)));
expect(bool(m::MmlMn::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir","mathvariant"}));
expect(prop(m::MmlMn::defaults().getOwn("mathbackground"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMn::defaults().getOwn("mathcolor"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMn::defaults().getOwn("mathsize"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMn::defaults().getOwn("dir"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMn::defaults().getOwn("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(bool((count(n))==0.0));
}
{auto*n=f.create("mtext");
expect(bool((n->kind())=="mtext"));
expect(bool((n->isToken())==true));
expect(bool((n->isInferred())==false));
expect(bool((n->notParent())==false));
expect(bool((n->arity())==std::numeric_limits<double>::infinity()));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->linebreakAlign())=="data-align"));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==true));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(0.0)));
expect(bool(m::MmlMtext::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir","mathvariant"}));
expect(prop(m::MmlMtext::defaults().getOwn("mathbackground"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMtext::defaults().getOwn("mathcolor"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMtext::defaults().getOwn("mathsize"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMtext::defaults().getOwn("dir"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMtext::defaults().getOwn("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(bool((count(n))==0.0));
}
{auto*n=f.create("ms");
expect(bool((n->kind())=="ms"));
expect(bool((n->isToken())==true));
expect(bool((n->isInferred())==false));
expect(bool((n->notParent())==false));
expect(bool((n->arity())==std::numeric_limits<double>::infinity()));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->linebreakAlign())=="data-align"));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==false));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(0.0)));
expect(bool(m::MmlMs::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir","mathvariant","lquote","rquote"}));
expect(prop(m::MmlMs::defaults().getOwn("mathbackground"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMs::defaults().getOwn("mathcolor"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMs::defaults().getOwn("mathsize"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMs::defaults().getOwn("dir"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMs::defaults().getOwn("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(m::MmlMs::defaults().getOwn("lquote"),Property(h::debugText(units({34})))));
expect(prop(m::MmlMs::defaults().getOwn("rquote"),Property(h::debugText(units({34})))));
expect(bool((count(n))==0.0));
}
{auto*n=f.create("mspace");
expect(bool((n->kind())=="mspace"));
expect(bool((n->isToken())==true));
expect(bool((n->isInferred())==false));
expect(bool((n->notParent())==false));
expect(bool((n->arity())==0.0));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->linebreakAlign())=="data-align"));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==true));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(-1.0)));
expect(bool(m::MmlMspace::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir","mathvariant","width","height","depth","linebreak","indentshift","indentalign","indenttarget","indentalignfirst","indentshiftfirst","indentalignlast","indentshiftlast"}));
expect(prop(m::MmlMspace::defaults().getOwn("mathbackground"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMspace::defaults().getOwn("mathcolor"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMspace::defaults().getOwn("mathsize"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMspace::defaults().getOwn("dir"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMspace::defaults().getOwn("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(m::MmlMspace::defaults().getOwn("width"),Property(h::debugText(units({48,101,109})))));
expect(prop(m::MmlMspace::defaults().getOwn("height"),Property(h::debugText(units({48,101,120})))));
expect(prop(m::MmlMspace::defaults().getOwn("depth"),Property(h::debugText(units({48,101,120})))));
expect(prop(m::MmlMspace::defaults().getOwn("linebreak"),Property(h::debugText(units({97,117,116,111})))));
expect(prop(m::MmlMspace::defaults().getOwn("indentshift"),Property(h::debugText(units({97,117,116,111})))));
expect(prop(m::MmlMspace::defaults().getOwn("indentalign"),Property(h::debugText(units({97,117,116,111})))));
expect(prop(m::MmlMspace::defaults().getOwn("indenttarget"),Property(h::debugText(units({})))));
expect(prop(m::MmlMspace::defaults().getOwn("indentalignfirst"),Property(h::debugText(units({105,110,100,101,110,116,97,108,105,103,110})))));
expect(prop(m::MmlMspace::defaults().getOwn("indentshiftfirst"),Property(h::debugText(units({105,110,100,101,110,116,115,104,105,102,116})))));
expect(prop(m::MmlMspace::defaults().getOwn("indentalignlast"),Property(h::debugText(units({105,110,100,101,110,116,97,108,105,103,110})))));
expect(prop(m::MmlMspace::defaults().getOwn("indentshiftlast"),Property(h::debugText(units({105,110,100,101,110,116,115,104,105,102,116})))));
expect(bool((count(n))==0.0));
}
{auto*n=f.create("mstyle");
expect(bool((n->kind())=="mstyle"));
expect(bool((n->isToken())==false));
expect(bool((n->isInferred())==false));
expect(bool((n->notParent())==false));
expect(bool((n->arity())==-1.0));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->linebreakAlign())=="data-align"));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==true));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(PropertyNull{})));
expect(bool(m::MmlMstyle::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir","scriptlevel","displaystyle","scriptsizemultiplier","scriptminsize","infixlinebreakstyle"}));
expect(prop(m::MmlMstyle::defaults().getOwn("mathbackground"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMstyle::defaults().getOwn("mathcolor"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMstyle::defaults().getOwn("mathsize"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMstyle::defaults().getOwn("dir"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMstyle::defaults().getOwn("scriptlevel"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMstyle::defaults().getOwn("displaystyle"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMstyle::defaults().getOwn("scriptsizemultiplier"),Property(0.7071067811865475)));
expect(prop(m::MmlMstyle::defaults().getOwn("scriptminsize"),Property(h::debugText(units({46,52,101,109})))));
expect(prop(m::MmlMstyle::defaults().getOwn("infixlinebreakstyle"),Property(h::debugText(units({98,101,102,111,114,101})))));
expect(bool((count(n))==1.0));
expect(bool((at(n,0)->kind())=="inferredMrow"));
}
{auto*n=f.create("merror");
expect(bool((n->kind())=="merror"));
expect(bool((n->isToken())==false));
expect(bool((n->isInferred())==false));
expect(bool((n->notParent())==false));
expect(bool((n->arity())==-1.0));
expect(bool((n->linebreakContainer())==true));
expect(bool((n->linebreakAlign())=="data-align"));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==false));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(0.0)));
expect(bool(m::MmlMerror::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir"}));
expect(prop(m::MmlMerror::defaults().getOwn("mathbackground"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMerror::defaults().getOwn("mathcolor"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMerror::defaults().getOwn("mathsize"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMerror::defaults().getOwn("dir"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(bool((count(n))==1.0));
expect(bool((at(n,0)->kind())=="inferredMrow"));
}
{auto*n=f.create("mphantom");
expect(bool((n->kind())=="mphantom"));
expect(bool((n->isToken())==false));
expect(bool((n->isInferred())==false));
expect(bool((n->notParent())==false));
expect(bool((n->arity())==-1.0));
expect(bool((n->linebreakContainer())==false));
expect(bool((n->linebreakAlign())=="data-align"));
expect(bool((n->isEmpty())==true));
expect(bool((n->isSpacelike())==true));
expect(prop(n->texClass()?Property(*n->texClass()):Property(PropertyNull{}),Property(0.0)));
expect(bool(m::MmlMphantom::defaults().keys()==std::vector<std::string>{"mathbackground","mathcolor","mathsize","dir"}));
expect(prop(m::MmlMphantom::defaults().getOwn("mathbackground"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMphantom::defaults().getOwn("mathcolor"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMphantom::defaults().getOwn("mathsize"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(prop(m::MmlMphantom::defaults().getOwn("dir"),Property(h::debugText(units({95,105,110,104,101,114,105,116,95})))));
expect(bool((count(n))==1.0));
expect(bool((at(n,0)->kind())=="inferredMrow"));
}
};
"math_port_leaf_utf16_pattern_full_unit_domain"_test=[] {F f(classes());
expect(bool((bits([](char16_t c){return m::MmlMi::singleCharacter()(std::u16string(1,c));}))=="ffbdffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffcfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
expect(bool((bits([](char16_t c){return m::MmlMi::singleCharacter()(std::u16string(u"a")+c);}))=="000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffffffffffffffffffffffffff000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000fff7000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffffffffffffff000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffffff00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"));
expect(bool((bits([](char16_t c){return m::MmlMi::singleCharacter()(units({55296})+c);}))=="ffbdffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffcfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
expect(bool((bits([](char16_t c){return m::MmlMi::operatorName()(std::u16string(1,c));}))=="0000000000000000efffff70efffff7000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"));
expect(bool((bits([](char16_t c){return h::onlySpaces(std::u16string(1,c));}))=="00e30000100000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ff700000003800000000000800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000"));
};
"math_port_leaf_row_core_null_persistence_and_queries"_test=[] {F f(classes());
auto*row=dynamic_cast<m::MmlMrow*>(f.create("mrow"));auto*a=f.create("probe");auto*b=f.create("probe");auto*space=f.create("mspace");a->setProperty("embellished",true);b->setProperty("embellished",true);row->setChildren(children({space,a}));row->childNodes->insert(row->childNodes->begin(),nullptr);
expect(bool((row->isEmbellished())==true));
expect(bool((row->core()==a)==true));
expect(bool((row->coreMO()==a)==true));
expect(bool((row->nonSpaceLength())==1.0));
expect(bool((row->firstNonSpace()==a)==true));
expect(bool((row->lastNonSpace()==a)==true));
{bool caught=false;try{(void)row->isSpacelike();}catch(const tr::FactoryError&error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
row->childNodes->push_back(b);
expect(bool((row->isEmbellished())==false));
expect(bool((row->core()==row)==true));
row->childNodes=children({space});
expect(bool((row->isEmbellished())==false));
expect(bool((row->nonSpaceLength())==0.0));
expect(bool((row->firstNonSpace()==nullptr)==true));
expect(bool((row->isSpacelike())==true));
};
"math_port_leaf_row_delimiter_nullish_and_class_transfer"_test=[] {F f(classes());
{auto*r=f.create("mrow");auto*a=f.create("probe");auto*prev=f.create("probe");a->setTexClass(2);prev->setTexClass(3);r->setChildren(children({a}));r->setProperty("open",Property(Missing{}));auto*result=r->setTeXclass(prev);
expect(bool((result==r)==false));
expect(prop(r->texClass()?Property(*r->texClass()):Property(PropertyNull{}),Property(2.0)));
expect(prop(r->prevClass()?Property(*r->prevClass()):Property(PropertyNull{}),Property(3.0)));
expect(prop(a->prevClass()?Property(*a->prevClass()):Property(PropertyNull{}),Property(PropertyNull{})));
}
{auto*r=f.create("mrow");auto*a=f.create("probe");auto*prev=f.create("probe");a->setTexClass(2);prev->setTexClass(3);r->setChildren(children({a}));r->setProperty("open",Property(PropertyNull{}));auto*result=r->setTeXclass(prev);
expect(bool((result==r)==false));
expect(prop(r->texClass()?Property(*r->texClass()):Property(PropertyNull{}),Property(2.0)));
expect(prop(r->prevClass()?Property(*r->prevClass()):Property(PropertyNull{}),Property(3.0)));
expect(prop(a->prevClass()?Property(*a->prevClass()):Property(PropertyNull{}),Property(PropertyNull{})));
}
{auto*r=f.create("mrow");auto*a=f.create("probe");auto*prev=f.create("probe");a->setTexClass(2);prev->setTexClass(3);r->setChildren(children({a}));r->setProperty("open",Property(false));auto*result=r->setTeXclass(prev);
expect(bool((result==r)==true));
expect(prop(r->texClass()?Property(*r->texClass()):Property(PropertyNull{}),Property(7.0)));
expect(prop(r->prevClass()?Property(*r->prevClass()):Property(PropertyNull{}),Property(3.0)));
expect(prop(a->prevClass()?Property(*a->prevClass()):Property(PropertyNull{}),Property(PropertyNull{})));
}
{auto*r=f.create("mrow");auto*a=f.create("probe");auto*prev=f.create("probe");a->setTexClass(2);prev->setTexClass(3);r->setChildren(children({a}));r->setProperty("open",Property(h::debugText(units({}))));auto*result=r->setTeXclass(prev);
expect(bool((result==r)==true));
expect(prop(r->texClass()?Property(*r->texClass()):Property(PropertyNull{}),Property(7.0)));
expect(prop(r->prevClass()?Property(*r->prevClass()):Property(PropertyNull{}),Property(3.0)));
expect(prop(a->prevClass()?Property(*a->prevClass()):Property(PropertyNull{}),Property(PropertyNull{})));
}
{auto*r=f.create("mrow");auto*a=f.create("probe");auto*prev=f.create("probe");a->setTexClass(2);prev->setTexClass(3);r->setChildren(children({a}));r->setProperty("open",Property(0.0));auto*result=r->setTeXclass(prev);
expect(bool((result==r)==true));
expect(prop(r->texClass()?Property(*r->texClass()):Property(PropertyNull{}),Property(7.0)));
expect(prop(r->prevClass()?Property(*r->prevClass()):Property(PropertyNull{}),Property(3.0)));
expect(prop(a->prevClass()?Property(*a->prevClass()):Property(PropertyNull{}),Property(PropertyNull{})));
}
{auto*r=f.create("mrow");auto*a=f.create("probe");auto*prev=f.create("probe");a->setTexClass(2);prev->setTexClass(3);r->setChildren(children({a}));r->setProperty("open",Property(h::debugText(units({40}))));auto*result=r->setTeXclass(prev);
expect(bool((result==r)==true));
expect(prop(r->texClass()?Property(*r->texClass()):Property(PropertyNull{}),Property(7.0)));
expect(prop(r->prevClass()?Property(*r->prevClass()):Property(PropertyNull{}),Property(3.0)));
expect(prop(a->prevClass()?Property(*a->prevClass()):Property(PropertyNull{}),Property(PropertyNull{})));
}
};
"math_port_leaf_inferred_join_capture_cycle_and_utf16"_test=[] {F f(classes());
auto*r=f.create("inferredMrow");auto*x=text(f,units({97,55296}));r->childNodes=children({x,nullptr,r});
expect(bool(h::text(r->toString())==units({91,97,55296,44,44,91,93,93})));
};
"math_port_leaf_mi_italic_and_auto_operator"_test=[] {F f(classes());
{auto*n=create(f,"mi",{},children({text(f,units({120}))}));n->setInheritedAttributes();auto*result=n->setTeXclass(nullptr);
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({105,116,97,108,105,99})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({105,116,97,108,105,99})))));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
expect(bool((n->texClass().value_or(-99))==0.0));
expect(bool((result==n)==true));
}
{auto*n=create(f,"mi",{},children({text(f,units({115,105,110}))}));n->setInheritedAttributes();auto*result=n->setTeXclass(nullptr);
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->getProperty("autoOP"),Property(true)));
expect(bool((n->texClass().value_or(-99))==1.0));
expect(bool((result==n)==true));
}
{auto*n=create(f,"mi",{},children({text(f,units({97,98,99,50}))}));n->setInheritedAttributes();auto*result=n->setTeXclass(nullptr);
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->getProperty("autoOP"),Property(true)));
expect(bool((n->texClass().value_or(-99))==1.0));
expect(bool((result==n)==true));
}
{auto*n=create(f,"mi",{},children({text(f,units({50,120}))}));n->setInheritedAttributes();auto*result=n->setTeXclass(nullptr);
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
expect(bool((n->texClass().value_or(-99))==0.0));
expect(bool((result==n)==true));
}
{auto*n=create(f,"mi",{},children({text(f,units({97,45,98}))}));n->setInheritedAttributes();auto*result=n->setTeXclass(nullptr);
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
expect(bool((n->texClass().value_or(-99))==0.0));
expect(bool((result==n)==true));
}
{auto*n=create(f,"mi",{},children({text(f,units({97,10}))}));n->setInheritedAttributes();auto*result=n->setTeXclass(nullptr);
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
expect(bool((n->texClass().value_or(-99))==0.0));
expect(bool((result==n)==true));
}
{auto*n=create(f,"mi",{},children({text(f,units({55357,56832}))}));n->setInheritedAttributes();auto*result=n->setTeXclass(nullptr);
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({105,116,97,108,105,99})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({105,116,97,108,105,99})))));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
expect(bool((n->texClass().value_or(-99))==0.0));
expect(bool((result==n)==true));
}
{auto*n=create(f,"mi",{},children({text(f,units({97,768}))}));n->setInheritedAttributes();auto*result=n->setTeXclass(nullptr);
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({105,116,97,108,105,99})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({105,116,97,108,105,99})))));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
expect(bool((n->texClass().value_or(-99))==0.0));
expect(bool((result==n)==true));
}
{auto*n=create(f,"mi",{},children({text(f,units({97,6847}))}));n->setInheritedAttributes();auto*result=n->setTeXclass(nullptr);
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({110,111,114,109,97,108})))));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
expect(bool((n->texClass().value_or(-99))==0.0));
expect(bool((result==n)==true));
}
{auto*n=create(f,"mi",{},children({text(f,units({55296,55296}))}));n->setInheritedAttributes();auto*result=n->setTeXclass(nullptr);
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({105,116,97,108,105,99})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({105,116,97,108,105,99})))));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
expect(bool((n->texClass().value_or(-99))==0.0));
expect(bool((result==n)==true));
}
auto*n=create(f,"mi",PropertyList{{"mathvariant","bold"}},children({text(f,u"x")}));n->setInheritedAttributes();
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({98,111,108,100})))));
expect(prop(n->attributes()->getInherited("mathvariant"),Property(h::debugText(units({105,116,97,108,105,99})))));
};
"math_port_leaf_space_explicit_presence_width_and_newlines"_test=[] {F f(classes());
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(h::debugText(units({})))}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==true));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(h::debugText(units({45,49,101,109})))}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(h::debugText(units({32,65279,45,48})))}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(h::debugText(units({45,48})))}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(0.0)}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==true));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(-0.0)}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==true));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(-1.0)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(Missing{})}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==true));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(PropertyNull{})}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==true));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(false)}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==true));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(h::debugText(units({133,45,49})))}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==true));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width",Property(h::debugText(units({8232,45,49})))}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"height",Property(PropertyNull{})}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"height",Property(false)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"height",Property(0.0)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"height",Property(h::debugText(units({})))}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"depth",Property(PropertyNull{})}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"depth",Property(false)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"depth",Property(0.0)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"depth",Property(h::debugText(units({})))}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"style",Property(PropertyNull{})}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"style",Property(false)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"style",Property(0.0)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"style",Property(h::debugText(units({})))}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"mathbackground",Property(PropertyNull{})}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"mathbackground",Property(false)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"mathbackground",Property(0.0)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"mathbackground",Property(h::debugText(units({})))}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"background",Property(PropertyNull{})}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"background",Property(false)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"background",Property(0.0)}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"background",Property(h::debugText(units({})))}}));
expect(bool((n->canBreak())==false));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(PropertyNull{})}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(false)}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(0.0)}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(h::debugText(units({})))}}));
expect(bool((n->canBreak())==true));
expect(bool((n->isSpacelike())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(h::debugText(units({110,101,119,108,105,110,101})))}}));
expect(bool((n->hasNewline())==true));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(h::debugText(units({105,110,100,101,110,116,105,110,103,110,101,119,108,105,110,101})))}}));
expect(bool((n->hasNewline())==true));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(h::debugText(units({103,111,111,100,98,114,101,97,107})))}}));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(true)}}));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(PropertyNull{})}}));
expect(bool((n->hasNewline())==false));
}
};
"math_port_leaf_mtext_spaces_and_presence"_test=[] {F f(classes());
{auto*n=create(f,"mtext",{},children({text(f,units({}))}));
expect(bool((n->isSpacelike())==true));
}
{auto*n=create(f,"mtext",{},children({text(f,units({32}))}));
expect(bool((n->isSpacelike())==true));
}
{auto*n=create(f,"mtext",{},children({text(f,units({65279,8232}))}));
expect(bool((n->isSpacelike())==true));
}
{auto*n=create(f,"mtext",{},children({text(f,units({133}))}));
expect(bool((n->isSpacelike())==false));
}
{auto*n=create(f,"mtext",{},children({text(f,units({120}))}));
expect(bool((n->isSpacelike())==false));
}
{auto*n=create(f,"mtext",{},children({text(f,units({55357,56832}))}));
expect(bool((n->isSpacelike())==false));
}
{auto*n=create(f,"mtext",PropertyList{{"style",false}},children({text(f,u" ")}));
expect(bool((n->isSpacelike())==false));
}
{auto*n=create(f,"mtext",PropertyList{{"mathbackground",false}},children({text(f,u" ")}));
expect(bool((n->isSpacelike())==false));
}
{auto*n=create(f,"mtext",PropertyList{{"background",false}},children({text(f,u" ")}));
expect(bool((n->isSpacelike())==false));
}
};
"math_port_leaf_mstyle_raw_relative_levels_and_prime"_test=[] {F f(classes());
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({43,50})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(2.0),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({32,45,48,120,50})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(2.0),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(0.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({51,120})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(2.0),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(3.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({48,120,49,48})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(2.0),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(16.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(false)}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(2.0),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(std::numeric_limits<double>::quiet_NaN())));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(PropertyNull{})}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(2.0),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(true)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({43,50})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(h::debugText(units({50}))),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(h::debugText(units({50,50})))));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({32,45,48,120,50})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(h::debugText(units({50}))),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(h::debugText(units({50,45,50})))));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({51,120})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(h::debugText(units({50}))),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(3.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({48,120,49,48})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(h::debugText(units({50}))),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(16.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(false)}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(h::debugText(units({50}))),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(std::numeric_limits<double>::quiet_NaN())));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(PropertyNull{})}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(h::debugText(units({50}))),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(h::debugText(units({50})))));
expect(prop(observed[2],Property(true)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({43,50})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(PropertyNull{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({32,45,48,120,50})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(PropertyNull{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(-2.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({51,120})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(PropertyNull{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(3.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({48,120,49,48})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(PropertyNull{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(16.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(false)}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(PropertyNull{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(std::numeric_limits<double>::quiet_NaN())));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(PropertyNull{})}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(PropertyNull{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(PropertyNull{})));
expect(prop(observed[2],Property(true)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({43,50})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(Missing{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({32,45,48,120,50})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(Missing{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(-2.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({51,120})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(Missing{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(3.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(h::debugText(units({48,120,49,48})))}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(Missing{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(16.0)));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(false)}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(Missing{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(std::numeric_limits<double>::quiet_NaN())));
expect(prop(observed[2],Property(false)));
expect(bool((n->notParent())==true));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto*n=create(f,"mstyle",PropertyList{{"scriptlevel",Property(PropertyNull{})}},children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},true,Property(Missing{}),true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(0.0)));
expect(prop(observed[2],Property(true)));
expect(bool((n->notParent())==true));
}
};
"math_port_leaf_mstyle_display_and_cramped"_test=[] {F f(classes());
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(true)}};auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(true)}};a.set("data-cramped",Property(false));auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(true)}};a.set("data-cramped",Property(h::debugText(units({121,101,115}))));auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(h::debugText(units({121,101,115})))));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(false)}};auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(false)}};a.set("data-cramped",Property(false));auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(false)}};a.set("data-cramped",Property(h::debugText(units({121,101,115}))));auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(h::debugText(units({121,101,115})))));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(h::debugText(units({116,114,117,101})))}};auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(h::debugText(units({116,114,117,101})))}};a.set("data-cramped",Property(false));auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(h::debugText(units({116,114,117,101})))}};a.set("data-cramped",Property(h::debugText(units({121,101,115}))));auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(h::debugText(units({121,101,115})))));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(1.0)}};auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(1.0)}};a.set("data-cramped",Property(false));auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(1.0)}};a.set("data-cramped",Property(h::debugText(units({121,101,115}))));auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(h::debugText(units({121,101,115})))));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(PropertyNull{})}};auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(true)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(PropertyNull{})}};a.set("data-cramped",Property(false));auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"displaystyle",Property(PropertyNull{})}};a.set("data-cramped",Property(h::debugText(units({121,101,115}))));auto*n=create(f,"mstyle",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,4,true);
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(4.0)));
expect(prop(observed[2],Property(h::debugText(units({121,101,115})))));
}
};
"math_port_leaf_math_display_mode_and_explicit_precedence"_test=[] {F f(classes());
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",Property(h::debugText(units({105,110,108,105,110,101})))}};auto*n=create(f,"math",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,2,"prime");
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(h::debugText(units({112,114,105,109,101})))));
expect(prop(n->attributes()->getInherited("displaystyle"),Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",Property(h::debugText(units({98,108,111,99,107})))}};auto*n=create(f,"math",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,2,"prime");
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(h::debugText(units({112,114,105,109,101})))));
expect(prop(n->attributes()->getInherited("displaystyle"),Property(true)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",Property(h::debugText(units({105,110,108,105,110,101})))}};a.set("displaystyle",Property(false));auto*n=create(f,"math",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,2,"prime");
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(h::debugText(units({112,114,105,109,101})))));
expect(prop(n->attributes()->getInherited("displaystyle"),Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",Property(h::debugText(units({98,108,111,99,107})))}};a.set("displaystyle",Property(false));auto*n=create(f,"math",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,2,"prime");
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(h::debugText(units({112,114,105,109,101})))));
expect(prop(n->attributes()->getInherited("displaystyle"),Property(true)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",Property(h::debugText(units({105,110,108,105,110,101})))}};a.set("displaystyle",Property(true));auto*n=create(f,"math",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,2,"prime");
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(h::debugText(units({112,114,105,109,101})))));
expect(prop(n->attributes()->getInherited("displaystyle"),Property(true)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",Property(h::debugText(units({98,108,111,99,107})))}};a.set("displaystyle",Property(true));auto*n=create(f,"math",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,2,"prime");
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(h::debugText(units({112,114,105,109,101})))));
expect(prop(n->attributes()->getInherited("displaystyle"),Property(true)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",Property(h::debugText(units({105,110,108,105,110,101})))}};a.set("displaystyle",Property(h::debugText(units({121,101,115}))));auto*n=create(f,"math",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,2,"prime");
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(h::debugText(units({112,114,105,109,101})))));
expect(prop(n->attributes()->getInherited("displaystyle"),Property(true)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",Property(h::debugText(units({98,108,111,99,107})))}};a.set("displaystyle",Property(h::debugText(units({121,101,115}))));auto*n=create(f,"math",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,2,"prime");
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(h::debugText(units({112,114,105,109,101})))));
expect(prop(n->attributes()->getInherited("displaystyle"),Property(true)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",Property(h::debugText(units({105,110,108,105,110,101})))}};a.set("displaystyle",Property(0.0));auto*n=create(f,"math",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,2,"prime");
expect(prop(observed[0],Property(false)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(h::debugText(units({112,114,105,109,101})))));
expect(prop(n->attributes()->getInherited("displaystyle"),Property(false)));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));auto a=PropertyList{{"display",Property(h::debugText(units({98,108,111,99,107})))}};a.set("displaystyle",Property(0.0));auto*n=create(f,"math",a,children({spy}));std::array<Property,3> observed;spy->onInherit=[&](m::AttributeList,Property d,Property l,Property p){observed={d,l,p};};n->setInheritedAttributes({},false,2,"prime");
expect(prop(observed[0],Property(true)));
expect(prop(observed[1],Property(2.0)));
expect(prop(observed[2],Property(h::debugText(units({112,114,105,109,101})))));
expect(prop(n->attributes()->getInherited("displaystyle"),Property(true)));
}
};
"math_port_leaf_math_constructor_defaults_survive_registry_rebind"_test=[] {F f(classes());
auto*n=f.create("math");auto*spy=dynamic_cast<Probe*>(f.create("probe"));at(n,0)->appendChild(spy);auto old=m::MmlMath::defaults().get("scriptlevel");m::MmlMath::defaults().set("scriptlevel",7);struct Other:m::MmlMath{using MmlMath::MmlMath;static PropertyList& defaults(){static PropertyList d=[] {PropertyList p;p.assign(m::MmlMath::defaults());p.set("scriptlevel",11);return p;}();return d;}};f.setNodeClass("math",m::classBinding<Other>());Property level;spy->onInherit=[&](m::AttributeList,Property,Property l,Property){level=l;};n->setInheritedAttributes();
expect(prop(level,Property(7.0)));
m::MmlMath::defaults().set("scriptlevel",old);
};
"math_port_leaf_math_nested_null_options_failure_after_allocation"_test=[] {F f(classes());
auto*parent=f.create("probe");auto*n=f.create("math");parent->appendChild(n);m::MmlNode*error=nullptr;auto c=m::classBinding<m::MmlMerror>();auto original=c->construct;c->construct=[&](m::Factory&f,tr::FactoryArgs a){error=original(f,a);return error;};f.setNodeClass("merror",c);
{bool caught=false;try{n->verifyTree();}catch(const tr::FactoryError&error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
expect(bool((at(parent,0)==n)==true));
expect(bool((error!=nullptr)==true));
if(error){
expect(prop(error->attributes()->getExplicit("data-mjx-message"),Property(h::debugText(units({73,109,112,114,111,112,101,114,32,110,101,115,116,105,110,103,32,111,102,32,109,97,116,104,32,116,97,103,115})))));
expect(bool((count(error))==1.0));
expect(bool((count(at(error,0)))==0.0));
}
};
};
