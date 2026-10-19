module folia.math.output.common;
import :wrapper;
import :mo;
import :wrapper_factory;
import :output_jax;
import folia.math.util.lengths;
import folia.math.util.property_host;
import folia.math.core.mml_tree.mml_nodes.mo;
import std;
namespace folia::math::output::common {
namespace { std::string stringOf(const Property& p){return property_host::debugText(property_host::string(p));} std::u16string utf16(std::string_view s){return property_host::text(s);} }
CommonMo::CommonMo(CommonWrapperFactory& wrapperFactory,mml::MmlNode& source,CommonWrapper* parentValue)
    :CommonWrapper(wrapperFactory,source,parentValue){}
void CommonMo::initializeSource(){
    CommonWrapper::initializeSource();
    auto* mo=dynamic_cast<mml::MmlMo*>(node);
    if(!mo)throw options::Exception("TypeError",u"CommonMo requires an MmlMo node");
    isAccent=property_host::truthy(mo->isAccent());
    const auto mult=mo->attributes()->get("linebreakmultchar");
    if(property_host::truthy(mult)&&getText()==u"\u2062"&&stringOf(mult)!="\u2062")
        multChar=&createMo(utf16(stringOf(mult)));
    setBreakStyle();
}
void CommonMo::getVariant(){
    const auto attributes=node->attributes();
    if(property_host::truthy(attributes->get("largeop"))){
        variant=property_host::truthy(attributes->get("displaystyle"))?"-largeop":"-smallop";
        return;
    }
    if(!attributes->hasExplicit("mathvariant")&&node->getProperty("pseudoscript")==Property(false)){
        variant="-tex-variant";
        return;
    }
    CommonWrapper::getVariant();
}
std::shared_ptr<std::vector<std::uint32_t>> CommonMo::remapChars(
    std::shared_ptr<std::vector<std::uint32_t>> chars)const{
    const auto primes=node->getProperty("primes");
    if(property_host::truthy(primes)){
        const auto* text=std::get_if<std::string>(&primes.value);
        if(!text)throw options::Exception("TypeError",u"primes is not a string");
        return unicodeChars(utf16(*text),variant);
    }
    if(chars&&chars->size()==1){
        auto* mo=dynamic_cast<mml::MmlMo*>(node);
        if(!mo)throw options::Exception("TypeError",u"CommonMo has an incompatible MML node");
        auto* coreParent=mo->coreParent();
        const auto parentValue=coreParent->getParent();
        const bool accent=isAccent&&parentValue.requireNode("isKind")->isKind("mrow")==false;
        const auto remapped=font?font->getRemappedChar(accent?"accent":"mo",static_cast<double>((*chars)[0])):
                                 std::optional<std::u16string>{};
        if(remapped)return unicodeChars(*remapped,variant);
    }
    return chars;
}
std::size_t CommonMo::breakCount(){return breakStyle.empty()?0:1;}
std::size_t CommonMo::embellishedBreakCount()const{return embellishedBreakStyle().empty()?0:1;}
std::string CommonMo::embellishedBreakStyle()const{return breakStyle.empty()?getBreakStyle():breakStyle;}
std::string CommonMo::getBreakStyle(std::string_view forced)const{
    const auto attributes=node->attributes();
    std::string style(forced);
    const auto linebreak=attributes->get("linebreak");
    if(style.empty()&&(linebreak==Property("newline")||property_host::truthy(node->getProperty("forcebreak"))))
        style=stringOf(attributes->get("linebreakstyle"));
    if(style=="infixlinebreakstyle")style=stringOf(attributes->get(style));
    return style;
}
void CommonMo::setBreakStyle(std::string_view linebreak){
    const auto parentNode=node->getParent();
    const bool parentEmbellished=parentNode.isNode()&&parentNode.requireNode("isEmbellished")->isEmbellished();
    breakStyle=(parentEmbellished&&linebreak.empty())?std::string{}:getBreakStyle(linebreak);
    if(!breakCount()||!multChar)return;
    if(!parent)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'node')");
    const auto index=parent->node->childIndex(mml::NodeValue(node));
    if(!index)return;
    const auto siblings=parent->node->childArrayOwner();
    if(!siblings||*index+1>=siblings->size())return;
    const auto next=siblings->get(*index+1);
    if(next.isNode())next.requireNode("setTeXclass")->setTeXclass(mml::NodeValue(multChar->node));
}
void CommonMo::protoBBox(BBox& target){
    if(stretch&&stretch->dir!=NONE&&!size)
        getStretchedVariant(std::array<double,1>{0});
    if(stretch&&stretch->dir!=NONE&&size&&*size<0)return;
    CommonWrapper::computeBBox(target);
    const auto attributes=node->attributes();
    const auto* mo=dynamic_cast<mml::MmlMo*>(node);
    if(target.w==0&&attributes->hasExplicit("fence")&&getText().empty()&&mo&&
       (property_host::number(const_cast<mml::MmlMo*>(mo)->texClass())==static_cast<double>(mml::TEXCLASS.OPEN)||
        property_host::number(const_cast<mml::MmlMo*>(mo)->texClass())==static_cast<double>(mml::TEXCLASS.CLOSE))&&
       !options::truthy(jax().optionsContext().get(jax().options,options::Key(u"mathmlSpacing")))){
        if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
        target.R=font->params().nulldelimiterspace;
    }
    copySkewIC(target);
}
void CommonMo::computeBBox(BBox& target,bool){
    protoBBox(target);
    const auto attributes=node->attributes();
    if(property_host::truthy(attributes->get("symmetric"))&&(!stretch||stretch->dir!=HORIZONTAL)){
        const double offset=(target.h+target.d)/2+(font?font->params().axis_height:0)-target.h;
        target.h+=offset;target.d-=offset;
    }
    const bool sizeNonnegative=!size||*size>=0;
    if(property_host::truthy(node->getProperty("mathaccent"))&&(!stretch||stretch->dir==NONE||sizeNonnegative))target.w=0;
}
LineBBox CommonMo::moLineBBox(std::size_t index,std::string_view style,const BBox* outer){
    const auto leadingValue=node->attributes()->get("lineleading");
    const double leading=length2em(leadingValue,jax().currentLinebreakOptions().lineleading);
    if(index==0&&style=="before"){
        auto line=LineBBox::from(BBox::zero(),leading);
        line.originalL=bbox->L;bbox->L=0;return line;
    }
    const auto base=outer?*outer:*getOuterBBox();
    auto line=LineBBox::from(base,leading);
    if(index==1){
        if(style=="after"){
            line.w=line.h=line.d=0;line.isFirst=true;bbox->R=0;
        }else if(style=="duplicate")line.L=0;
        else if(multChar)line=LineBBox::from(*multChar->getOuterBBox(),leading);
        line.getIndentData(*node);
    }
    return line;
}
std::shared_ptr<LineBBox> CommonMo::computeLineBBox(std::size_t index){
    return std::make_shared<LineBBox>(moLineBBox(index,breakStyle));
}
double CommonMo::getAccentOffset(){auto target=BBox::makeEmpty();protoBBox(target);return -target.w/2;}
double CommonMo::getCenterOffset(BBox* box){
    BBox temporary;
    if(!box){CommonWrapper::computeBBox(temporary);box=&temporary;}
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    return (box->h+box->d)/2+font->params().axis_height-box->h;
}
double CommonMo::getSize(std::string_view name,double fallback){
    const auto attributes=node->attributes();
    if(attributes->isSet(name))return length2em(attributes->get(name),1,1);
    return fallback;
}
double CommonMo::getWH(std::span<const double> dimensions)const{
    if(dimensions.empty())return 0;
    if(dimensions.size()==1)return dimensions[0];
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    const double h=dimensions[0],d=dimensions[1],axis=font->params().axis_height;
    return property_host::truthy(node->attributes()->get("symmetric"))
        ?2*std::max(h-axis,d+axis):h+d;
}
double CommonMo::checkExtendedHeight(double dimension,const DelimiterData& delimiter)const{
    if(delimiter.fullExt){
        const auto [extSize,endSize]=*delimiter.fullExt;
        dimension=endSize+std::ceil(std::max(0.0,dimension-endSize)/extSize)*extSize;
    }
    return dimension;
}
std::array<double,2> CommonMo::getBaseline(std::span<const double> requested,double dimension,const DelimiterData& delimiter)const{
    const bool hasPair=requested.size()==2&&requested[0]+requested[1]==dimension;
    const bool symmetric=property_host::truthy(node->attributes()->get("symmetric"));
    const double h0=hasPair?requested[0]:dimension,d0=hasPair?requested[1]:0;
    double h=h0+d0,d=0;
    if(symmetric){
        if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
        const double axis=font->params().axis_height;
        if(hasPair)h=2*std::max(h0-axis,d0+axis);
        d=h/2-axis;
    }else if(hasPair)d=d0;
    else{
        const auto chd=delimiter.HDW?std::array<double,2>{(*delimiter.HDW)[0],(*delimiter.HDW)[1]}:
                                    std::array<double,2>{0.75,0.25};
        d=chd[1]*(h/(chd[0]+chd[1]));
    }
    return {h-d,d};
}
void CommonMo::getStretchBBox(std::span<const double> requested,double dimension,const DelimiterData& delimiter){
    if(delimiter.min&&*delimiter.min>dimension)dimension=*delimiter.min;
    if(!delimiter.HDW)throw options::Exception("TypeError",u"Cannot destructure delimiter HDW");
    auto [h,d,w]=*delimiter.HDW;
    if(stretch&&stretch->dir==VERTICAL){const auto baseline=getBaseline(requested,dimension,delimiter);h=baseline[0];d=baseline[1];}
    else{
        w=dimension;
        if(stretch&&stretch->hd&&
           !options::truthy(jax().optionsContext().get(jax().options,options::Key(u"mathmlSpacing")))){
            if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
            const double factor=font->params().extender_factor;
            h=h*(1-factor)+(*stretch->hd)[0]*factor;
            d=d*(1-factor)+(*stretch->hd)[1]*factor;
        }
    }
    bbox->h=h;bbox->d=d;bbox->w=w;
}
void CommonMo::setDelimSize(double character,double index){
    const auto delimiter=stretch;
    if(!delimiter)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'schar')");
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getSizeVariant')");
    variant=font->getSizeVariant(character,index).value_or("");
    size=index;
    double selected=character;
    if(delimiter->schar&&!delimiter->schar->empty()&&index>=0){
        const auto slot=static_cast<std::size_t>(std::min(index,static_cast<double>(delimiter->schar->size()-1)));
        if((*delimiter->schar)[slot])selected=(*delimiter->schar)[slot];
    }
    stretch=std::make_shared<DelimiterData>(*delimiter);
    stretch->c=selected;
    if(!childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    const auto children=*childNodes;
    if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading '0')");
    children->get(0).requireNode("invalidateBBox")->invalidateBBox();
}
void CommonMo::getStretchedVariant(std::span<const double> requested,bool exact){
    if(!stretch||stretch->dir==NONE)return;
    double dimension=getWH(requested);
    const double minimum=getSize("minsize",0),maximum=getSize("maxsize",std::numeric_limits<double>::infinity());
    const bool mathaccent=property_host::truthy(node->getProperty("mathaccent"));
    dimension=std::max(minimum,std::min(maximum,dimension));
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    const double factor=font->params().delimiterfactor/1000.0,shortfall=font->params().delimitershortfall;
    const bool minimumIsTruthy=!std::isnan(minimum)&&minimum!=0;
    const double target=(minimumIsTruthy||exact)?dimension:(mathaccent?std::min(dimension/factor,dimension+shortfall):std::max(dimension*factor,dimension-shortfall));
    const auto characters=unicodeChars(getText());
    if(characters->empty())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'codePointAt')");
    const double character=static_cast<double>((*characters)[0]);
    auto delimiter=stretch;
    if(size&&!std::isnan(*size)&&*size!=0){
        const auto current=font->getDelimiter(character);
        size.reset();
        if(!current){stretch.reset();throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'c')");}
        const auto* data=std::get_if<DelimiterDataRef>(&*current);
        if(!data||!*data){stretch.reset();throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'c')");}
        stretch=delimiter=*data;
    }
    const double selected=delimiter->c&&!std::isnan(*delimiter->c)&&*delimiter->c!=0?*delimiter->c:character;
    std::size_t index=0;
    if(delimiter->sizes){
        for(const double candidate:*delimiter->sizes){
            if(candidate>=target){if(mathaccent&&index)--index;setDelimSize(selected,static_cast<double>(index));return;}
            ++index;
        }
    }
    if(delimiter->stretch){
        size=-1;
        invalidateBBox();
        getStretchBBox(requested,checkExtendedHeight(dimension,*delimiter),*delimiter);
    }else setDelimSize(selected,static_cast<double>(index)-1);
}
bool CommonMo::canStretch(std::string_view direction){
    if(stretch&&stretch->dir!=NONE)return stretch->dir==direction;
    if(!property_host::truthy(node->attributes()->get("stretchy")))return false;
    const auto text=getText();
    const auto chars=unicodeChars(text);
    if(chars->size()!=1)return false;
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getDelimiter')");
    const auto delimiter=font->getDelimiter(static_cast<double>((*chars)[0]));
    if(delimiter){
        if(const auto* data=std::get_if<DelimiterDataRef>(&*delimiter);data&&*data&&(*data)->dir==direction){stretch=*data;return true;}
    }
    stretch=NOSTRETCH;
    return false;
}

}
