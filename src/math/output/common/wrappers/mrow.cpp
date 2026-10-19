module folia.math.output.common;
import :wrapper;
import :mrow;
import :wrapper_factory;
import :output_jax;
import folia.math.util.property_host;
import std;
namespace folia::math::output::common {
namespace { std::string stringOf(const Property& p){return property_host::debugText(property_host::string(p));} }
CommonMrow::CommonMrow(CommonWrapperFactory& wrapperFactory,mml::MmlNode& source,CommonWrapper* parentValue)
    :CommonWrapper(wrapperFactory,source,parentValue){}
void CommonMrow::initializeSource(){
    CommonWrapper::initializeSource();
    isStack=!parent||parent->node->isInferred()||parent->breakTop(this,this)!=this;
    stretchChildren();
    if(childNodes&&(*childNodes))for(std::size_t index=0;index<(*childNodes)->size();++index){
        auto* child=dynamic_cast<CommonWrapper*>((*childNodes)->get(index).requireNode("constructor child"));
        if(child->bbox&&!child->bbox->pwidth.empty()){bbox->pwidth=std::string(BBox::fullWidth);break;}
    }
}
std::size_t CommonMrow::breakCount(){
    if(breakCount_<0){
        breakCount_=0;
        if(childNodes){
            const auto children=*childNodes;
            if(!children)throw options::Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
            for(std::size_t i=0;i<children->size();++i){
                auto* child=dynamic_cast<CommonWrapper*>(children->get(i).requireNode("breakCount"));
                breakCount_+=static_cast<std::int64_t>(child->breakCount());
            }
        }
    }
    return static_cast<std::size_t>(breakCount_);
}
CommonWrapper* CommonMrow::breakTop(CommonWrapper*,CommonWrapper*){
    if(!isStack)return this;
    if(!parent)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'breakTop')");
    return parent->breakTop(this,this);
}
bool CommonMrow::fixesPWidth()const{return false;}
void CommonMrow::stretchChildren(){
    std::vector<CommonWrapper*> stretchy;
    if(childNodes&&(*childNodes))for(std::size_t index=0;index<(*childNodes)->size();++index){
        auto* child=dynamic_cast<CommonWrapper*>((*childNodes)->get(index).requireNode("stretchChildren"));
        if(child->canStretch("v"))stretchy.push_back(child);
    }
    const auto count=stretchy.size();
    const auto nodeCount=childNodes&&(*childNodes)?(*childNodes)->size():0;
    if(!count||nodeCount<=1)return;
    double height=0,depth=0;
    const bool all=count>1&&count==nodeCount;
    if(childNodes&&(*childNodes))for(std::size_t index=0;index<(*childNodes)->size();++index){
        auto* child=dynamic_cast<CommonWrapper*>((*childNodes)->get(index).requireNode("stretchChildren"));
        if(all||!child->stretch||child->stretch->dir==NONE){
            const double scale=child->getBBox()->rscale;
            const auto hd=child->getUnbrokenHD();
            height=std::max(height,hd[0]*scale);
            depth=std::max(depth,hd[1]*scale);
        }
    }
    for(auto* child:stretchy){
        const double scale=child->coreRScale();
        child->coreMO()->getStretchedVariant(std::array<double,2>{height/scale,depth/scale});
    }
}
void CommonMrow::computeBBox(BBox& target,bool recompute){
    const auto breaks=breakCount();
    lineBBox.clear();
    if(breaks)lineBBox.push_back(std::make_shared<LineBBox>(BBoxData{.w=0,.h=.75,.d=.25},std::array<double,2>{0,0}));
    target.empty();
    if(childNodes&&(*childNodes))for(std::size_t index=0;index<(*childNodes)->size();++index){
        auto* child=dynamic_cast<CommonWrapper*>((*childNodes)->get(index).requireNode("computeBBox"));
        target.append(*child->getOuterBBox());
        if(breaks)computeChildLineBBox(*child,index);
    }
    target.clean();
    if(breaks&&(!coreMO()||!coreMO()->node->isEmbellished()))computeLinebreakBBox(target);
    if(fixesPWidth()&&setChildPWidths(recompute)&&!recompute)computeBBox(target,true);
    vboxAdjust(target);
}
void CommonMrow::computeChildLineBBox(CommonWrapper& child,std::size_t index){
    if(lineBBox.empty())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'end')");
    auto& last=*lineBBox.back();last.end={static_cast<double>(index),0};
    last.append(*child.getLineBBox(0));
    const auto parts=child.breakCount()+1;
    for(std::size_t line=1;line<parts;++line){
        auto next=std::make_shared<LineBBox>(BBoxData{.w=0,.h=.75,.d=.25});
        next->start=next->end={static_cast<double>(index),static_cast<double>(line)};
        next->isFirst=true;next->append(*child.getLineBBox(line));lineBBox.push_back(std::move(next));
    }
}
void CommonMrow::computeLinebreakBBox(BBox& target){
    target.empty();
    const auto count=lineBBox.size();
    if(!count)return;
    const auto last=count-1;
    if(isStack){
        for(std::size_t index=0;index<count;++index){
            auto& line=*lineBBox[index];addMiddleBorders(line);
            if(index==0)addLeftBorders(line);
            if(index==last)addRightBorders(line);
        }
    }
    double y=0;
    for(std::size_t index=0;index<count;++index){
        const auto& line=*lineBBox[index];target.combine(line,0,y);
        const double nextHeight=index+1<count?lineBBox[index+1]->h:0;
        y-=std::max(.25,line.d)+line.lineLeading.value_or(0)+std::max(.75,nextHeight);
    }
    if(isStack){lineBBox.front()->L=bbox->L;lineBBox.back()->R=bbox->R;}
    else{
        for(const auto& line:lineBBox)target.w=std::max(target.w,line->w);
        shiftLines(target);
        const bool display=jax().math&&property_host::truthy(jax().math->display);
        if(!display&&!options::truthy(jax().optionsContext().get(linebreakOptions(),options::Key(u"inline")))){
            target.pwidth=std::string(BBox::fullWidth);
            if(node->isInferred()){
                if(!parent)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'bbox')");
                parent->bbox->pwidth=std::string(BBox::fullWidth);
            }
        }
    }
    target.clean();
}
void CommonMrow::vboxAdjust(BBox& target){
    if(!parent)return;
    const auto count=breakCount();
    const auto valign=parent->node->attributes()->get("data-vertical-align");
    if(count&&valign==Property("bottom"))dh=target.d-lineBBox.at(count-1)->d;
    else if(valign==Property("center")||(count&&valign==Property("middle"))){
        if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
        dh=(target.h+target.d)/2+font->params().axis_height-target.h;
    }else{dh=0;return;}
    target.h+=dh;target.d-=dh;
}
std::shared_ptr<LineBBox> CommonMrow::getLineBBox(std::size_t index){
    (void)getBBox();
    if(isStack)return CommonWrapper::getLineBBox(index);
    const auto leading=jax().optionsContext().get(linebreakOptions(),options::Key(u"lineleading"));
    const auto parsed=std::holds_alternative<options::Undefined>(leading.data)?std::optional<double>{}:std::optional<double>{jax().optionsContext().toNumber(leading)};
    return std::make_shared<LineBBox>(LineBBox::from(*getOuterBBox(),parsed));
}
void CommonMrow::shiftLines(BBox& outer){
    const double width=outer.w;
    const auto count=lineBBox.size();
    if(count<2)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'indentData')");
    auto pairAt=[&](const LineBBox& line,std::size_t index)->std::optional<std::pair<std::string,std::string>>{
        if(!line.indentData)return std::nullopt;
        const auto array=line.indentData;
        if(index>=array->size())return std::nullopt;
        const auto pair=PropertyArray::array(array->get(index));
        if(!pair)throw options::Exception("TypeError",u"object is not iterable");
        return std::pair{stringOf(pair->get(0)),stringOf(pair->get(1))};
    };
    const auto first=pairAt(*lineBBox[1],0).value_or(std::pair<std::string,std::string>{"left","0"});
    const auto last=count-1;
    for(std::size_t index=0;index<count;++index){
        auto& line=*lineBBox[index];
        const auto indent=index==0?std::optional(first):pairAt(line,index==last?2:1);
        const auto values=indent.value_or(std::pair<std::string,std::string>{"left","0"});
        const auto processed=processIndent(values.first,values.second,first.first,first.second,width);
        line.L=0;
        line.L=getAlignX(width,line,processed.first)+processed.second;
        outer.w=std::max(outer.w,line.L+line.w);
    }
}
bool CommonMrow::setChildPWidths(bool recompute,std::optional<double> width,bool clear){
    if(!breakCount())return CommonWrapper::setChildPWidths(recompute,width,clear);
    if(recompute)return false;
    if(width&&bbox->w!=*width){bbox->w=*width;shiftLines(*bbox);}
    return true;
}
void CommonMrow::breakToWidth(double width){linebreaks().breakToWidth(*this,width);}

}
