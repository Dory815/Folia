/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/scriptbase.ts.
 * Source implementation for the shared script/limits wrapper base.
 *************************************************************/
module folia.math.output.common;
import :scriptbase;
import :wrapper;
import :mo;
import :linebreak_visitor;
import folia.math.core.mml_tree.attributes;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.util.property_host;
import folia.math.output.common.direction;
import std;

namespace folia::math::output::common {
namespace {
CommonWrapper* childAt(CommonWrapper& wrapper,std::size_t index){
    if(!wrapper.childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    const auto children=*wrapper.childNodes;
    if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading '0')");
    const auto value=children->get(index);
    return dynamic_cast<CommonWrapper*>(value.nodeOrNull());
}
std::size_t codePointCount(std::u16string_view text){
    std::size_t count=0;
    for(std::size_t i=0;i<text.size();++i){
        const auto c=text[i];
        if(c>=0xD800&&c<=0xDBFF&&i+1<text.size()&&text[i+1]>=0xDC00&&text[i+1]<=0xDFFF)++i;
        ++count;
    }
    return count;
}
Property attr(CommonWrapper& wrapper,std::string_view key){return wrapper.node->attributes()->get(key);}
std::optional<double> lineLeading(CommonScriptbase& wrapper){
    auto& context=wrapper.jax().optionsContext();
    const auto options=wrapper.linebreakOptions();
    const auto value=context.get(options,options::Key(u"lineleading"));
    if(std::holds_alternative<options::Undefined>(value.data))return {};
    return context.toNumber(value);
}
}

CommonScriptbase::CommonScriptbase(CommonWrapperFactory& factory,mml::MmlNode& node,
                                   CommonWrapper* parentValue,bool useItalicCorrection,
                                   std::size_t constructorScriptChild)
    :CommonWrapper(factory,node,parentValue),useItalicCorrection_(useItalicCorrection),
     constructorScriptChild_(constructorScriptChild){}
void CommonScriptbase::initializeSource(){
    CommonWrapper::initializeSource();
    auto* core=getBaseCore();
    baseCore=core;
    if(!core)return;
    setBaseAccentsFor(*core);
    baseScale=getBaseScale();
    baseIc=getBaseIc();
    baseIsChar=isCharBase();
    const auto script=childAt(*this,constructorScriptChild_);
    if(baseIsChar&&script){
        const auto mo=script->coreMO();
        if(mo) isMathAccent=!mo->node->getProperty("mathaccent").missing();
    }
    checkLineAccents();
    baseRemoveIc=!isLineAbove&&!isLineBelow&&(!useItalicCorrection_||isMathAccent);
}

CommonWrapper* CommonScriptbase::baseChild()const{
    std::size_t index=0;
    if(const auto* nodeValue=dynamic_cast<const mml::MmlMsubsup*>(node))index=nodeValue->base();
    else if(const auto* limits=dynamic_cast<const mml::MmlMunderover*>(node))index=limits->base();
    return childAt(*const_cast<CommonScriptbase*>(this),index);
}
CommonWrapper* CommonScriptbase::scriptChild()const{return childAt(*const_cast<CommonScriptbase*>(this),1);}

CommonWrapper* CommonScriptbase::getBaseCore(){
    auto* core=getSemanticBase();
    if(!core)core=childAt(*this,0);
    auto* current=core;
    while(current){
        const auto* currentNode=current->node;
        bool unwrap=false;
        if(current->childNodes){
            const auto children=*current->childNodes;
            if(!children)throw options::Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
            const auto one=children->size()==1;
            unwrap=one&&(currentNode->isKind("mrow")||currentNode->isKind("TeXAtom")||
                         currentNode->isKind("mstyle")||
                         (currentNode->isKind("mpadded")&&!property_host::truthy(currentNode->getProperty("vbox")))||
                         currentNode->isKind("mphantom")||currentNode->isKind("semantics"));
        }
        if(currentNode->isKind("munderover")){
            if(auto* script=dynamic_cast<CommonScriptbase*>(current);script&&script->isMathAccent)unwrap=true;
        }
        if(!unwrap)break;
        setBaseAccentsFor(*current);
        current=childAt(*current,0);
    }
    if(!current){baseHasAccentOver=false;baseHasAccentUnder=false;}
    return current?current:childAt(*this,0);
}

void CommonScriptbase::setBaseAccentsFor(CommonWrapper& core){
    if(!core.node->isKind("munderover"))return;
    if(!baseHasAccentOver)baseHasAccentOver=property_host::truthy(core.node->attributes()->get("accent"));
    if(!baseHasAccentUnder)baseHasAccentUnder=property_host::truthy(core.node->attributes()->get("accentunder"));
}

CommonWrapper* CommonScriptbase::getSemanticBase(){
    const auto value=node->attributes()->getExplicit("data-semantic-fencepointer");
    if(value.missing())return nullptr;
    return getBaseFence(baseChild(),property_host::debugText(property_host::string(value)));
}

CommonWrapper* CommonScriptbase::getBaseFence(CommonWrapper* fence,std::string_view id){
    if(!fence||!fence->node->attributes()||id.empty())return nullptr;
    if(fence->node->attributes()->getExplicit("data-semantic-id")==Property(std::string(id)))return fence;
    if(!fence->childNodes)return nullptr;
    const auto children=*fence->childNodes;
    if(!children)throw options::Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for(std::size_t i=0;i<children->size();++i){
        const auto value=children->get(i);
        auto* child=dynamic_cast<CommonWrapper*>(value.nodeOrNull());
        if(auto* result=getBaseFence(child,id))return result;
    }
    return nullptr;
}

double CommonScriptbase::getBaseScale()const{
    double scale=1;
    auto* child=baseCore;
    while(child&&child!=this){
        scale*=child->getOuterBBox()->rscale;
        child=child->parent;
    }
    return scale;
}
double CommonScriptbase::getBaseIc()const{return baseCore->getOuterBBox()->ic*baseScale;}
double CommonScriptbase::getAdjustedIc()const{return baseIc?1.05*baseIc+0.05:0;}
bool CommonScriptbase::isCharBase()const{
    if(!baseCore)return false;
    const auto kind=baseCore->node->kind();
    bool accepted=kind=="mi"||kind=="mn";
    if(kind=="mo")if(auto* mo=dynamic_cast<CommonMo*>(baseCore))accepted=!mo->size.has_value();
    return accepted&&baseCore->getOuterBBox()->rscale==1&&codePointCount(baseCore->getText())==1;
}
void CommonScriptbase::checkLineAccents(){
    if(node->kind()!="munderover"&&node->kind()!="mover"&&node->kind()!="munder")return;
    if(node->kind()=="mover")isLineAbove=isLineAccent(scriptChild());
    else if(node->kind()=="munder")isLineBelow=isLineAccent(scriptChild());
    else if(const auto* limits=dynamic_cast<const mml::MmlMunderover*>(node)){
        isLineAbove=isLineAccent(childAt(*this,limits->over()));
        isLineBelow=isLineAccent(childAt(*this,limits->under()));
    }
}
bool CommonScriptbase::isLineAccent(CommonWrapper* script)const{
    if(!script)return false;
    auto* core=script->coreMO();
    return core&&core->node->isToken()&&core->getText()==u"\u2015";
}

double CommonScriptbase::getBaseWidth()const{
    auto* child=baseChild();const auto line=child->getLineBBox(child->breakCount());
    return line->w*line->rscale-(baseRemoveIc?baseIc:0)+font->params().extra_ic;
}
std::array<double,2> CommonScriptbase::getOffset()const{return {0,0};}
double CommonScriptbase::baseCharZero(double n)const{
    const bool largeop=property_host::truthy(baseCore->node->attributes()->get("largeop"));
    bool sized=false;
    if(baseCore->node->kind()=="mo")if(auto* mo=dynamic_cast<CommonMo*>(baseCore))sized=mo->size.has_value()&&*mo->size!=0;
    return baseIsChar&&!largeop&&!sized&&baseScale==1?0:n;
}
double CommonScriptbase::getV()const{
    auto* base=baseCore;const auto bbox=base->getLineBBox(base->breakCount());
    const auto script=scriptChild()->getOuterBBox();const auto& tex=font->params();
    const double shift=length2em(attr(const_cast<CommonScriptbase&>(*this),"subscriptshift"),tex.sub1);
    return std::max({baseCharZero(bbox->d*baseScale+tex.sub_drop*script->rscale),shift,
                     script->h*script->rscale-0.8*tex.x_height});
}
double CommonScriptbase::getU()const{
    auto* base=baseCore;const auto bbox=base->getLineBBox(base->breakCount());
    const auto script=scriptChild()->getOuterBBox();const auto& tex=font->params();
    const auto attrs=node->attributes()->getList({"displaystyle","superscriptshift"});
    const bool display=property_host::truthy(attrs.get("displaystyle"));
    const bool prime=property_host::truthy(node->getProperty("texprimestyle"));
    const double initial=prime?tex.sup3:(display?tex.sup1:tex.sup2);
    const double shift=length2em(attrs.get("superscriptshift"),initial);
    return std::max({baseCharZero(bbox->h*baseScale-tex.sup_drop*script->rscale),shift,
                     script->d*script->rscale+0.25*tex.x_height});
}
bool CommonScriptbase::hasMovableLimits()const{
    const auto display=attr(*const_cast<CommonScriptbase*>(this),"displaystyle");
    auto* mo=baseChild()->coreMO();
    return !property_host::truthy(display)&&mo&&property_host::truthy(mo->node->attributes()->get("movablelimits"));
}
std::array<double,2> CommonScriptbase::getOverKU(const BBox& base,const BBox& over)const{
    const bool accent=property_host::truthy(attr(*const_cast<CommonScriptbase*>(this),"accent"));
    const auto& tex=font->params();const double d=over.d*over.rscale;
    const double t=tex.rule_thickness*tex.separation_factor;
    const double delta=baseHasAccentOver.value_or(false)?t:0;
    const double T=isLineAbove?3*tex.rule_thickness:t;
    const double k=(accent?T:std::max(tex.big_op_spacing1,tex.big_op_spacing3-std::max(0.0,d)))-delta;
    return {k,base.h*base.rscale+k+d};
}
std::array<double,2> CommonScriptbase::getUnderKV(const BBox& base,const BBox& under)const{
    const bool accent=property_host::truthy(attr(*const_cast<CommonScriptbase*>(this),"accentunder"));
    const auto& tex=font->params();const double h=under.h*under.rscale;
    const double t=tex.rule_thickness*tex.separation_factor;
    const double delta=baseHasAccentUnder.value_or(false)?t:0;
    const double T=isLineBelow?3*tex.rule_thickness:t;
    const double k=(accent?T:std::max(tex.big_op_spacing2,tex.big_op_spacing4-h))-delta;
    return {k,-(base.d*base.rscale+k+h)};
}
std::vector<double> CommonScriptbase::getDeltaW(const std::vector<BBox>& boxes,std::array<double,3> delta)const{
    std::vector<double> widths; widths.reserve(boxes.size());
    for(const auto& box:boxes)widths.push_back(box.w*box.rscale);
    if(widths.empty())return {};
    if(baseRemoveIc&&!property_host::truthy(baseCore->node->attributes()->get("largeop")))widths[0]-=baseIc;
    const double w=*std::max_element(widths.begin(),widths.end());
    const auto align=attr(*const_cast<CommonScriptbase*>(this),"align");
    const auto alignText=property_host::string(align);
    std::vector<double> dw(widths.size());double m=0;
    for(std::size_t i=0;i<widths.size();++i){
        const double shift=alignText==u"center"?(w-widths[i])/2:alignText==u"right"?w-widths[i]:0;
        const double d=shift+(i<delta.size()?delta[i]:0);dw[i]=d;if(d<m)m=-d;
    }
    if(m)for(auto& d:dw)d+=m;
    for(std::size_t i=1;i<std::min<std::size_t>(3,boxes.size());++i)dw[i]+=boxes[i].dx*boxes[0].rscale;
    return dw;
}
double CommonScriptbase::getDelta(CommonWrapper& script,bool noskew)const{
    const auto accent=attr(*const_cast<CommonScriptbase*>(this),"accent");
    const auto base=baseCore->getOuterBBox();double sk=base->sk;const double ic=base->ic;
    if(property_host::truthy(accent))sk-=script.getOuterBBox()->sk;
    return ((property_host::truthy(accent)&&!noskew?sk:0)+font->skewIcFactor*ic)*baseScale;
}
void CommonScriptbase::stretchChildren(){
    if(!childNodes)throw options::Exception("TypeError",u"this.childNodes is not iterable");
    const auto firstChildren=*childNodes;
    if(!firstChildren)throw options::Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    std::vector<CommonWrapper*> stretchy;
    for(std::size_t i=0;i<firstChildren->size();++i){auto* child=dynamic_cast<CommonWrapper*>(firstChildren->get(i).requireNode("canStretch"));if(child->canStretch(HORIZONTAL))stretchy.push_back(child);}
    const auto count=stretchy.size();
    if(!childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'length')");
    const auto countChildren=*childNodes;
    if(!countChildren)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'length')");
    const auto nodeCount=countChildren->size();
    if(!count||nodeCount<=1)return;
    if(!childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'Symbol(Symbol.iterator)')");
    const auto secondChildren=*childNodes;
    if(!secondChildren)throw options::Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    const bool all=count>1&&count==nodeCount;double W=0;
    for(std::size_t i=0;i<secondChildren->size();++i){
        auto* child=dynamic_cast<CommonWrapper*>(secondChildren->get(i).requireNode("getOuterBBox"));
        const bool noStretch=!child->stretch||child->stretch->dir==NONE;
        if(all||noStretch){const auto box=child->getOuterBBox(noStretch);W=std::max(W,box->w*box->rscale);}
    }
    for(auto* child:stretchy){auto* core=child->coreMO();if(core&&!core->size)core->getStretchedVariant(std::array<double,1>{W/child->coreRScale()});}
}

void CommonScriptbase::computeBBox(BBox& box,bool recompute){
    box.empty();box.append(*baseChild()->getOuterBBox());appendScripts(box);box.clean();setChildPWidths(recompute);
}
BBox CommonScriptbase::appendScripts(BBox& box)const{
    const double w=getBaseWidth();const auto offset=getOffset();
    box.combine(*scriptChild()->getOuterBBox(),w+offset[0],offset[1]);
    box.w+=font->params().scriptspace;return box;
}
std::size_t CommonScriptbase::breakCount(){
    if(breakCount_<0){
        if(node->isEmbellished())breakCount_=static_cast<std::int64_t>(coreMO()->embellishedBreakCount());
        else if(!node->linebreakContainer())breakCount_=static_cast<std::int64_t>(childAt(*this,0)->breakCount());
        else breakCount_=0;
    }
    return static_cast<std::size_t>(breakCount_);
}
CommonWrapper* CommonScriptbase::breakTop(CommonWrapper* mrow,CommonWrapper* child){
    const auto position=child?child->node->childPosition():std::optional<std::size_t>{};
    if(node->linebreakContainer()||!parent|| (position&&*position))return mrow;
    return parent->breakTop(mrow,this);
}
std::shared_ptr<LineBBox> CommonScriptbase::computeLineBBox(std::size_t i){
    const auto n=breakCount();
    if(!n)return std::make_shared<LineBBox>(LineBBox::from(*getOuterBBox(),lineLeading(*this)));
    auto bbox=baseChild()->getLineBBox(i)->copy();
    if(i<n){if(i==0)addLeftBorders(bbox);addMiddleBorders(bbox);}
    else{appendScripts(bbox);addMiddleBorders(bbox);addRightBorders(bbox);}
    return std::make_shared<LineBBox>(std::move(bbox));
}
}
