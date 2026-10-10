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
expect(bool((bits([](char16_t c){return m::MmlMi::singleCharacter()(std::u16string(1,c));}))=="ffbdffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffcfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
expect(bool((bits([](char16_t c){return m::MmlMi::singleCharacter()(std::u16string(u"a")+c);}))=="000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffffffffffffffffffffffffff000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000fff7000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffffffffffffff00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"0000000000000000000000000000000000000000000000000000ffffffff00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"));
expect(bool((bits([](char16_t c){return m::MmlMi::singleCharacter()(units({55296})+c);}))=="ffbdffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffcfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
expect(bool((bits([](char16_t c){return m::MmlMi::operatorName()(std::u16string(1,c));}))=="0000000000000000efffff70efffff70000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"));
expect(bool((bits([](char16_t c){return h::onlySpaces(std::u16string(1,c));}))=="00e30000100000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"ff700000003800000000000800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000"));
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
"math_port_leaf_source_field_phase_after_virtual_super_hooks"_test=[] {F f(classes());
struct Trace:m::MmlMi{using MmlMi::MmlMi;static std::vector<Property>& trace(){static std::vector<Property> v;return v;}static int& sets(){static int v=0;return v;}void setChildren(m::Children c)override{trace().push_back(texClass()?Property(*texClass()):Property(PropertyNull{}));MmlMi::setChildren(c);}void setTexClass(std::optional<double> v)override{++sets();MmlMi::setTexClass(v);}};f.setNodeClass("mi",m::classBinding<Trace>());auto*n=f.create("mi");
expect(bool((Trace::trace().size())==1.0));
expect(prop(Trace::trace()[0],Property(PropertyNull{})));
expect(bool((Trace::sets())==0.0));
expect(bool((n->texClass().value_or(-99))==0.0));
};
"math_port_leaf_row_last_search_reads_replaced_array"_test=[] {F f(classes());
struct SpaceProbe:Probe{using Probe::Probe;std::function<void()> onSpace;bool isSpacelike()const override{if(onSpace)onSpace();return Probe::isSpacelike();}};f.setNodeClass("probe",m::classBinding<SpaceProbe>());auto*r=dynamic_cast<m::MmlMrow*>(f.create("mrow"));auto*a=f.create("probe");auto*b=dynamic_cast<SpaceProbe*>(f.create("probe"));auto*replacement=f.create("probe");a->setProperty("space",true);b->setProperty("space",true);r->childNodes=children({a,b});b->onSpace=[&]{r->childNodes=children({replacement});};
expect(bool((r->lastNonSpace()==replacement)==true));
};
"math_port_leaf_row_core_uses_current_array_after_predicate"_test=[] {F f(classes());
struct Flicker:Probe{using Probe::Probe;std::function<void()> callback;bool isEmbellished()const override{if(callback)callback();return Probe::isEmbellished();}};f.setNodeClass("probe",m::classBinding<Flicker>());auto*r=f.create("mrow");auto*a=dynamic_cast<Flicker*>(f.create("probe"));auto*replacement=f.create("probe");a->setProperty("embellished",true);r->childNodes=children({a});a->callback=[&]{r->childNodes=children({replacement});};
expect(bool((r->core()==replacement)==true));
r->childNodes=children({a});a->callback=[&]{r->childNodes=children({nullptr});};
{bool caught=false;try{(void)r->coreMO();}catch(const tr::FactoryError&error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
};
"math_port_leaf_row_live_class_iteration_and_current_first_child"_test=[] {F f(classes());
auto*r=f.create("mrow");auto*a=dynamic_cast<Probe*>(f.create("probe"));auto*late=f.create("probe");auto*replacement=f.create("probe");a->setTexClass(2);late->setTexClass(3);replacement->setTexClass(1);auto original=children({a});r->childNodes=original;a->onClass=[&](m::MmlNode*){original->push_back(late);r->childNodes=children({replacement});};auto*result=r->setTeXclass(nullptr);
expect(bool((result==late)==true));
expect(bool((r->texClass().value_or(-99))==1.0));
expect(bool((at(r,0)==replacement)==true));
};
"math_port_leaf_mi_tuple_and_property_presence"_test=[] {F f(classes());
{auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setProperty("autoOP",Property(PropertyNull{}));n->setTeXclass(nullptr);
expect(bool((n->texClass().value_or(-99))==0.0));
expect(prop(n->getProperty("autoOP"),Property(PropertyNull{})));
}
{auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setProperty("autoOP",Property(false));n->setTeXclass(nullptr);
expect(bool((n->texClass().value_or(-99))==0.0));
expect(prop(n->getProperty("autoOP"),Property(false)));
}
{auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setProperty("autoOP",Property(0.0));n->setTeXclass(nullptr);
expect(bool((n->texClass().value_or(-99))==0.0));
expect(prop(n->getProperty("autoOP"),Property(0.0)));
}
{auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setProperty("autoOP",Property(h::debugText(units({}))));n->setTeXclass(nullptr);
expect(bool((n->texClass().value_or(-99))==0.0));
expect(prop(n->getProperty("autoOP"),Property(h::debugText(units({})))));
}
{auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setProperty("texClass",Property(PropertyNull{}));n->setTeXclass(nullptr);
expect(bool((n->texClass().value_or(-99))==0.0));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
}
{auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setProperty("texClass",Property(false));n->setTeXclass(nullptr);
expect(bool((n->texClass().value_or(-99))==0.0));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
}
{auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setProperty("texClass",Property(0.0));n->setTeXclass(nullptr);
expect(bool((n->texClass().value_or(-99))==0.0));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
}
{auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setProperty("texClass",Property(h::debugText(units({}))));n->setTeXclass(nullptr);
expect(bool((n->texClass().value_or(-99))==0.0));
expect(prop(n->getProperty("autoOP"),Property(Missing{})));
}
auto*n=create(f,"mi",{},children({text(f,u"x")}));n->setInheritedAttributes(PropertyList{{"mathvariant",m::attributeTuple("mstyle","")}});
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({})))));
};
"math_port_leaf_math_mode_display_is_inherited_not_explicit"_test=[] {F f(classes());
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));PropertyList a{{"mode","display"}};auto*n=create(f,"math",a,children({spy}));Property observed;spy->onInherit=[&](m::AttributeList,Property d,Property,Property){observed=d;};n->setInheritedAttributes();
expect(prop(observed,Property(true)));
expect(prop(n->attributes()->getInherited("display"),Property(h::debugText(units({98,108,111,99,107})))));
expect(prop(n->attributes()->get("display"),Property(h::debugText(units({98,108,111,99,107})))));
}
{auto*spy=dynamic_cast<Probe*>(f.create("probe"));PropertyList a{{"mode","display"}};a.set("display","inline");auto*n=create(f,"math",a,children({spy}));Property observed;spy->onInherit=[&](m::AttributeList,Property d,Property,Property){observed=d;};n->setInheritedAttributes();
expect(prop(observed,Property(false)));
expect(prop(n->attributes()->getInherited("display"),Property(h::debugText(units({98,108,111,99,107})))));
expect(prop(n->attributes()->get("display"),Property(h::debugText(units({105,110,108,105,110,101})))));
}
};
"math_port_leaf_mi_operator_assignment_uses_virtual_setter"_test=[] {F f(classes());
struct Observe:m::MmlMi{using MmlMi::MmlMi;static int& sets(){static int n=0;return n;}void setTexClass(std::optional<double> v)override{++sets();MmlMi::setTexClass(v);}};f.setNodeClass("mi",m::classBinding<Observe>());auto*n=create(f,"mi",{},children({text(f,u"sin")}));n->setTeXclass(nullptr);
expect(bool((Observe::sets())==1.0));
expect(bool((n->texClass().value_or(-99))==1.0));
};
"math_port_leaf_math_each_attributes_access_is_virtual"_test=[] {F f(classes());
struct Sequence:m::MmlMath{using MmlMath::MmlMath;std::vector<std::shared_ptr<folia::math::Attributes>> sequence;mutable std::size_t cursor=0;std::shared_ptr<folia::math::Attributes> attributes()const override{if(sequence.empty())return MmlMath::attributes();if(cursor>=sequence.size())throw tr::FactoryError("TypeError",u"sequence exhausted");return sequence[cursor++];}void run(){setChildInheritedAttributes({},false,0,false);}};f.setNodeClass("math",m::classBinding<Sequence>());auto*n=dynamic_cast<Sequence*>(f.create("math"));for(int i=0;i<7;++i)n->sequence.push_back(std::make_shared<folia::math::Attributes>(m::MmlMath::defaults(),m::MmlMath::defaults()));n->sequence[4]->set("display","block");n->cursor=0;n->run();
expect(bool((n->cursor)==7.0));
expect(prop(n->sequence[5]->getInherited("displaystyle"),Property(true)));
};
"math_port_leaf_mstyle_defaults_precede_first_write_and_partial_throw"_test=[] {F f(classes());
struct Partial:m::MmlMstyle{using MmlMstyle::MmlMstyle;bool armed=false;mutable int calls=0;std::shared_ptr<folia::math::Attributes> attributes()const override{if(armed&&++calls==2)throw tr::FactoryError("TypeError",u"second read");return MmlMstyle::attributes();}};f.setNodeClass("mstyle",m::classBinding<Partial>());auto*n=dynamic_cast<Partial*>(f.create("mstyle"));auto saved=n->attributes();n->armed=true;n->calls=0;
{bool caught=false;try{n->setInheritedAttributes({},Missing{},Missing{},Missing{});}catch(const tr::FactoryError&error){caught=true;expect(bool(error.name=="TypeError"));}expect(caught);}
expect(prop(saved->getInherited("displaystyle"),Property(false)));
expect(bool((n->calls)==2.0));
};
"math_port_leaf_inferred_join_snapshots_array_and_length"_test=[] {F f(classes());
struct Trigger:m::TextNode{using TextNode::TextNode;std::function<void()> callback;std::string toString()const override{if(callback)callback();return TextNode::toString();}};f.setNodeClass("text",m::classBinding<Trigger>());auto*r=f.create("inferredMrow");auto*x=dynamic_cast<Trigger*>(text(f,u"a"));auto*y=text(f,u"b");auto*late=text(f,u"c");auto original=children({x,y});r->childNodes=original;x->callback=[&]{original->push_back(late);r->childNodes=children();};
expect(bool((r->toString())=="[a,b]"));
expect(bool((count(r))==0.0));
};
"math_port_leaf_mstyle_dispatches_only_literal_first_child"_test=[] {F f(classes());
auto*n=f.create("mstyle");auto*a=dynamic_cast<Probe*>(f.create("probe"));auto*b=dynamic_cast<Probe*>(f.create("probe"));n->childNodes=children({a,b});std::array<int,2> calls{};a->onInherit=[&](m::AttributeList,Property,Property,Property){++calls[0];};b->onInherit=[&](m::AttributeList,Property,Property,Property){++calls[1];};n->setInheritedAttributes();
expect(bool((calls[0])==1.0));
expect(bool((calls[1])==0.0));
};
"math_port_leaf_newline_is_suppressed_by_break_constraints"_test=[] {F f(classes());
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(h::debugText(units({110,101,119,108,105,110,101})))},{"height",Property(0.0)}}));
expect(bool((n->hasNewline())==false));
}
{auto*n=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"linebreak",Property(h::debugText(units({105,110,100,101,110,116,105,110,103,110,101,119,108,105,110,101})))},{"width",Property(h::debugText(units({45,49,101,109})))}}));
expect(bool((n->hasNewline())==false));
}
};
"math_port_leaf_static_bindings_copy_alias_and_live_callbacks"_test=[] {F f(classes());
expect(bool((Property(m::MmlMrow::defaults())==Property(m::MmlInferredMrow::defaults()))==true));
m::MmlMrow::defaults().set("p9-own",1);
expect(prop(m::MmlInferredMrow::defaults().get("p9-own"),Property(1.0)));
expect(prop(m::MmlMi::defaults().get("p9-own"),Property(Missing{})));
m::MmlMrow::defaults().erase("p9-own");
auto old=m::MmlMi::singleCharacter();m::MmlMi::singleCharacter()=[](std::u16string_view s){return s==u"xyz";};auto*n=create(f,"mi",{},children({text(f,u"xyz")}));n->setInheritedAttributes();
expect(prop(n->attributes()->get("mathvariant"),Property(h::debugText(units({105,116,97,108,105,99})))));
m::MmlMi::singleCharacter()=old;
m::MmlMspace::NONSPACELIKE().push_back("width");auto*space=dynamic_cast<m::MmlMspace*>(create(f,"mspace",PropertyList{{"width","1em"}}));
expect(bool((space->canBreak())==false));
m::MmlMspace::NONSPACELIKE().pop_back();
};
"math_port_leaf_native_recursive_budget_recovery"_test=[] {F f(classes());auto*root=f.create("mrow");auto*tail=root;for(int i=1;i<512;++i){auto*next=f.create("mrow");tail->childNodes=children({next});tail=next;}expect(root->isSpacelike());tail->childNodes=children({f.create("mrow")});bool caught=false;try{(void)root->isSpacelike();}catch(const tr::FactoryError&e){caught=e.name=="RangeError";}expect(caught);expect(f.create("mrow")->isSpacelike());root=f.create("inferredMrow");tail=root;for(int i=1;i<512;++i){auto*next=f.create("inferredMrow");tail->childNodes=children({next});tail=next;}expect(bool(root->toString().size()==1024));tail->childNodes=children({f.create("inferredMrow")});caught=false;try{(void)root->toString();}catch(const tr::FactoryError&e){caught=e.name=="RangeError";}expect(caught);expect(bool(f.create("inferredMrow")->toString()=="[]"));};
};
