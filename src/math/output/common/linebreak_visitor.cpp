module folia.math.output.common;
import :linebreak_visitor;
import :wrapper;
import :wrapper_factory;
import :mo;
import :mspace;
import :mrow;
import :output_jax;
import :mfrac;
import :msqrt;
import :mroot;
import :mfenced;
import :msubsup;
import :mmultiscripts;
import :mtext;
import :maction;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mmultiscripts;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.cancellation;
import std;

namespace folia::math::output::common {
namespace {
double number(const Property& value) { return property_host::number(value); }
std::string text(const Property& value) { return property_host::debugText(property_host::string(value)); }
std::vector<std::u16string> splitText(std::u16string_view value) {
    std::vector<std::u16string> result;
    std::size_t start=0;
    for(std::size_t i=0;i<value.size();++i)if(value[i]==u' '){result.emplace_back(value.substr(start,i-start));start=i+1;}
    result.emplace_back(value.substr(start));
    return result;
}
std::size_t indexArg(std::span<const std::any> args) {
    if(args.empty())return 0;
    if(const auto* value=std::any_cast<std::size_t>(&args[0]))return *value;
    if(const auto* value=std::any_cast<double>(&args[0]))return static_cast<std::size_t>(*value);
    if(const auto* value=std::any_cast<int>(&args[0]))return static_cast<std::size_t>(*value);
    throw options::Exception("TypeError",u"Linebreak visitor index is not numeric");
}
std::vector<std::any> oneIndex(std::size_t i){return {std::any(i)};}
double jsMax(double a,double b){return (std::isnan(a)||std::isnan(b))?std::numeric_limits<double>::quiet_NaN():std::max(a,b);}
std::optional<double> lineLeading(CommonWrapper& wrapper){
    const auto linebreakOptions=wrapper.linebreakOptions();
    auto& context=wrapper.jax().optionsContext();
    const auto raw=context.get(linebreakOptions,options::Key(u"lineleading"));
    return context.toNumber(raw);
}
}

Linebreaks::Linebreaks(CommonWrapperFactory& factory,MethodLookup lookup)
    :tree::AbstractVisitor<CommonWrapper>(
        static_cast<tree::FactoryServices<CommonWrapper>&>(factory),std::move(lookup),
        [](CommonWrapper& wrapper)->std::optional<tree::VisitorChildren<CommonWrapper>>{
            if(!wrapper.childNodes)return {};
            const auto captured=*wrapper.childNodes;
            if(!captured)throw options::Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
            return tree::VisitorChildren<CommonWrapper>{[captured]{return captured->size();},[captured](std::size_t index){return captured->get(index);}};
        }),factory_(&factory){}

LinebreakVisitor::MethodLookup LinebreakVisitor::methods(){
    return [](std::string_view name){
        tree::VisitorMethod<CommonWrapper> result;
        result.truthy=true;
        if(name=="visitMoNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMoNode(wrapper,args);};
        else if(name=="visitMspaceNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMspaceNode(wrapper,args);};
        else if(name=="visitMtextNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMtextNode(wrapper,args);};
        else if(name=="visitMrowNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMrowNode(wrapper,args);};
        else if(name=="visitInferredMrowNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitInferredMrowNode(wrapper,args);};
        else if(name=="visitMfracNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMfracNode(wrapper,args);};
        else if(name=="visitMsqrtNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMsqrtNode(wrapper,args);};
        else if(name=="visitMrootNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMrootNode(wrapper,args);};
        else if(name=="visitMsubNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMsubNode(wrapper,args);};
        else if(name=="visitMsupNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMsupNode(wrapper,args);};
        else if(name=="visitMsubsupNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMsubsupNode(wrapper,args);};
        else if(name=="visitMmultiscriptsNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMmultiscriptsNode(wrapper,args);};
        else if(name=="visitMfencedNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMfencedNode(wrapper,args);};
        else if(name=="visitMactionNode")result.function=[](auto& self,auto& wrapper,auto args){return static_cast<LinebreakVisitor&>(self).visitMactionNode(wrapper,args);};
        else result.truthy=false;
        return result;
    };
}
std::size_t LinebreakVisitor::lineArgument(std::span<const std::any> args){return indexArg(args);}
LinebreakVisitor::LinebreakVisitor(CommonWrapperFactory& factory):Linebreaks(factory,methods()){}

std::any LinebreakVisitor::visitNode(CommonWrapper& wrapper,std::span<const std::any> args){
    cancellation::checkpoint(cancellation::CheckpointStage::Layout);
    ++state_.depth;
    if(wrapper.node->isEmbellished()&&!wrapper.node->isKind("mo"))
        (void)visitEmbellishedOperator(wrapper,args);
    else
        (void)tree::AbstractVisitor<CommonWrapper>::visitNode(wrapper,args);
    --state_.depth;
    return {};
}

void LinebreakVisitor::addWidth(const BBox& box,std::optional<double> width){
    double value=width.value_or(box.L+box.w+box.R);
    if(!value)return;
    value*=box.rscale;
    state_.w+=value;
    if(!state_.potential.empty())state_.potential.front().widthToNext+=value;
    processBreak();
}
void LinebreakVisitor::processBreak(){
    auto& state=state_;
    while(!state.potential.empty()&&state.w>state.width){
        cancellation::checkpoint(cancellation::CheckpointStage::Layout);
        const auto br=state.potential.back();state.potential.pop_back();
        const double pw=br.beforeWidth,dw=br.afterWidth,w=br.widthToNext;
        state.breaks.push_back(BreakEntry{br.identity,br.wrapperAndIndex});
        double newWidth=dw+w;
        for(const auto& candidate:state.potential)newWidth+=candidate.widthToNext;
        state.w=newWidth;
        if(state.prevBreak&&state.prevWidth+pw<=state.width){
            std::erase_if(state.breaks,[&](const auto& item){return item.identity==state.prevBreak->identity;});
            state.prevWidth+=pw;
        }else state.prevWidth=pw+dw;
        for(auto& candidate:state.potential)candidate.beforeWidth-=pw;
        state.prevBreak=br;
        state.mathLeft-=pw;
    }
}
void LinebreakVisitor::pushBreak(CommonWrapper& wrapper,double penalty,double width,IndexData index){
    auto& state=state_;
    if(penalty>=NOBREAK||(state.w==0&&state.prevWidth==0))return;
    while(!state.potential.empty()&&state.potential.front().penalty>penalty*0.99){
        const auto removed=state.potential.front();state.potential.erase(state.potential.begin());
        if(!state.potential.empty())state.potential.front().widthToNext+=removed.widthToNext;
    }
    BreakData data;
    data.wrapperAndIndex={&wrapper,index};
    data.penalty=penalty;
    data.beforeWidth=state.w-(state.prevBreak?state.prevBreak->afterWidth:0);
    data.afterWidth=width;
    state.potential.insert(state.potential.begin(),std::move(data));
}
LinebreakState LinebreakVisitor::createState(CommonWrapper& wrapper){
    const double width=wrapper.getBBox()->w;
    LinebreakState result;result.mathWidth=width;result.mathLeft=width;return result;
}
std::pair<double,double> LinebreakVisitor::borderLR(const CommonWrapper& wrapper)const{
    if(!wrapper.styleData)return {0,0};
    const auto& data=*wrapper.styleData;
    const auto border=data.border?data.border->width:std::array<double,4>{};
    return {border[3]+data.padding[3],border[1]+data.padding[1]};
}
void LinebreakVisitor::breakLineToWidth(CommonWrapper& wrapper,std::size_t i){
    state_.potential.clear();state_.w=0;state_.prevWidth=0;state_.prevBreak.reset();state_.depth=0;
    auto args=oneIndex(i);(void)visitNode(wrapper,args);
}
void LinebreakVisitor::breakToWidth(CommonWrapper& wrapper,double width){
    auto previous=std::move(state_);state_=createState(wrapper);state_.width=width;
    const auto count=wrapper.breakCount();
    for(std::size_t i=0;i<=count;++i){
        const auto line=i<wrapper.lineBBox.size()&&wrapper.lineBBox[i]?wrapper.lineBBox[i]:wrapper.getLineBBox(i);
        if(line->w>width)breakLineToWidth(wrapper,i);
    }
    const auto breaks=state_.breaks;
    for(const auto& entry:breaks){
        const auto [target,index]=entry.wrapperAndIndex;
        if(!target)continue;
        if(!index){
            auto* mo=target->coreMO();if(!mo)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'setBreakStyle')");
            const auto style=mo->node->attributes()->get("linebreakstyle");
            mo->setBreakStyle(property_host::truthy(style)?text(style):"before");
        }else{
            auto* text=dynamic_cast<CommonMtext*>(target);
            if(!text)throw options::Exception("TypeError",u"ww.setBreakAt is not callable");
            text->setBreakAt(index);
        }
        target->invalidateBBox();
    }
    state_=std::move(previous);
}

std::any LinebreakVisitor::visitDefault(CommonWrapper& wrapper,std::span<const std::any> args){
    const auto i=lineArgument(args);auto line=wrapper.getLineBBox(i);
    bool firstChild=false;
    if(wrapper.childNodes&&*wrapper.childNodes&&!(*wrapper.childNodes)->empty())
        firstChild=(*wrapper.childNodes)->get(0).isNode();
    if(wrapper.node->isToken()||wrapper.node->linebreakContainer()||!firstChild){
        addWidth(*line);return {};
    }
    const auto [left,right]=borderLR(wrapper);
    if(i==0)addWidth(*line,line->L+left);
    const auto children=*wrapper.childNodes;
    const auto child=children->get(0);
    auto* nested=dynamic_cast<CommonWrapper*>(child.requireNode("visitDefault"));
    auto childArgs=oneIndex(i);(void)visitNode(*nested,childArgs);
    if(i==wrapper.breakCount())addWidth(*line,line->R+right);
    return {};
}
std::any LinebreakVisitor::visitEmbellishedOperator(CommonWrapper& wrapper,std::span<const std::any>){
    auto* mo=wrapper.coreMO();
    if(!mo)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'node')");
    auto box=LineBBox::from(*wrapper.getOuterBBox(),lineLeading(wrapper));
    box.getIndentData(*mo->node);
    const auto style=mo->getBreakStyle(text(mo->node->attributes()->get("linebreakstyle")));
    const auto outer=PropertyArray::array(box.indentData->get(1));
    const auto inner=PropertyArray::array(box.indentData->get(0));
    if(!outer||!inner)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '1')");
    const auto shift=mo->processIndent("",text(outer->get(1)),"",text(inner->get(1)),state_.width).second;
    const auto penalty=moPenalty(*mo);
    if(style=="before"){pushBreak(wrapper,penalty,shift-box.L,{});addWidth(box);}
    else{addWidth(box);const double width=(style=="after"?0:(mo->multChar?mo->multChar->getBBox()->w:box.w))+shift;pushBreak(wrapper,penalty,width,{});}
    return {};
}
std::any LinebreakVisitor::visitMoNode(CommonWrapper& wrapper,std::span<const std::any>){
    auto* mo=dynamic_cast<CommonMo*>(&wrapper);if(!mo)throw options::Exception("TypeError",u"mo wrapper has incompatible type");
    const auto box=LineBBox::from(*mo->getOuterBBox(),lineLeading(*mo));
    auto mutableBox=box;mutableBox.getIndentData(*mo->node);
    const auto style=mo->getBreakStyle(text(mo->node->attributes()->get("linebreakstyle")));
    const auto indent=mutableBox.indentData->get(1);auto pair=PropertyArray::array(indent);
    if(!pair)throw options::Exception("TypeError",u"object is not iterable");
    const auto firstPair=PropertyArray::array(mutableBox.indentData->get(0));
    if(!firstPair)throw options::Exception("TypeError",u"object is not iterable");
    const double dw=mo->processIndent("",text(pair->get(1)),"",text(firstPair->get(1)),state_.width).second;
    const double penalty=moPenalty(*mo);
    if(style=="before"){pushBreak(wrapper,penalty,dw-mutableBox.L,{});addWidth(mutableBox);}
    else {addWidth(mutableBox);const double width=(style=="after"?0:(mo->multChar?mo->multChar->getBBox()->w:mutableBox.w))+dw;pushBreak(wrapper,penalty,width,{});}
    return {};
}
std::any LinebreakVisitor::visitMspaceNode(CommonWrapper& wrapper,std::span<const std::any> args){
    auto* mspace=dynamic_cast<CommonMspace*>(&wrapper);if(!mspace)throw options::Exception("TypeError",u"mspace wrapper has incompatible type");
    const auto line=mspace->getLineBBox(lineArgument(args));
    if(mspace->canBreak()){
        const double penalty=mspacePenalty(*mspace);line->getIndentData(*mspace->node);
        const auto data=line->indentData;const auto pair0=PropertyArray::array(data->get(0)),pair1=PropertyArray::array(data->get(1));
        if(!pair0||!pair1)throw options::Exception("TypeError",u"object is not iterable");
        const double shift=mspace->processIndent("",text(pair1->get(1)),"",text(pair0->get(1)),state_.width).second;
        pushBreak(wrapper,penalty,shift-line->w,{});
    }
    addWidth(*line);return {};
}
std::any LinebreakVisitor::visitMtextNode(CommonWrapper& wrapper,std::span<const std::any> args){
    const auto value=wrapper.getText();
    if(value.find(u' ')==std::u16string::npos)return visitDefault(wrapper,args);
    auto* mtext=dynamic_cast<CommonMtext*>(&wrapper);
    if(!mtext)throw options::Exception("TypeError",u"mtext wrapper has incompatible type");
    mtext->clearBreakPoints();
    const double space=mtext->textWidth(u" ");
    const auto bbox=wrapper.getBBox();
    const auto [left,right]=borderLR(wrapper);
    addWidth(*bbox,bbox->L+left);
    if(!mtext->childNodes||!(*mtext->childNodes))
        throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'keys')");
    const auto children=*mtext->childNodes;
    for(std::size_t j=0;j<children->size();++j){
        auto* child=dynamic_cast<CommonWrapper*>(children->get(j).requireNode("visitMtextNode"));
        if(child->node->isKind("text")){
            auto* textNode=dynamic_cast<mml::TextNode*>(child->node);
            if(!textNode)throw options::Exception("TypeError",u"text child is not a TextNode");
            auto words=splitText(textNode->getText());
            if(words.empty())words.emplace_back();
            const auto last=words.back();words.pop_back();
            for(std::size_t k=0;k<words.size();++k){
                addWidth(*bbox,mtext->textWidth(words[k]));
                pushBreak(wrapper,mtextPenalty(),-space,IndexData{{j,k+1}});
                addWidth(*bbox,space);
            }
            addWidth(*bbox,mtext->textWidth(last));
        }else addWidth(*child->getBBox());
    }
    addWidth(*bbox,bbox->R+right);
    return {};
}
double LinebreakVisitor::mtextPenalty()const{return std::floor((state_.width/jsMax(0.0001,state_.mathLeft-state_.w))*500)+std::floor(((state_.width-state_.w)/state_.width)*2500)+800*state_.depth;}
std::any LinebreakVisitor::visitMrowNode(CommonWrapper& wrapper,std::span<const std::any> args){
    const auto i=lineArgument(args);auto line=i<wrapper.lineBBox.size()&&wrapper.lineBBox[i]?wrapper.lineBBox[i]:wrapper.getLineBBox(i);
    const auto start=line->start.value_or(std::array<double,2>{0,0});
    const auto initialChildren=wrapper.childNodes?*wrapper.childNodes:tree::FactoryChildren<CommonWrapper>{};
    const double defaultEnd=initialChildren&&!initialChildren->empty()?static_cast<double>(initialChildren->size()-1):-1.0;
    const auto end=line->end.value_or(std::array<double,2>{defaultEnd,0});
    const auto [left,right]=borderLR(wrapper);addWidth(*line,line->L+left);
    for(std::int64_t index=static_cast<std::int64_t>(start[0]);index<=static_cast<std::int64_t>(end[0]);++index){
        if(!wrapper.childNodes||!(*wrapper.childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
        const auto children=*wrapper.childNodes;const auto child=children->get(static_cast<std::size_t>(index));
        auto* nested=dynamic_cast<CommonWrapper*>(child.requireNode("visitMrowNode"));
        const auto childIndex=static_cast<std::size_t>(index);
        auto childArgs=oneIndex(childIndex==static_cast<std::size_t>(start[0])?static_cast<std::size_t>(start[1]):childIndex==static_cast<std::size_t>(end[0])?static_cast<std::size_t>(end[1]):0);
        (void)visitNode(*nested,childArgs);
    }
    addWidth(*line,line->R+right);return {};
}
std::any LinebreakVisitor::visitInferredMrowNode(CommonWrapper& wrapper,std::span<const std::any> args){
    --state_.depth;(void)visitMrowNode(wrapper,args);++state_.depth;return {};
}
std::any LinebreakVisitor::visitMfracNode(CommonWrapper& wrapper,std::span<const std::any> args){
    auto* frac=dynamic_cast<CommonMfrac*>(&wrapper);if(!frac)throw options::Exception("TypeError",u"mfrac wrapper has incompatible type");
    if(!property_host::truthy(frac->node->attributes()->get("bevelled"))&&frac->getOuterBBox()->w>state_.width){
        if(!frac->childNodes||!(*frac->childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
        auto children=*frac->childNodes;const auto first=children->get(0),second=children->get(1);
        (void)breakToWidth(*dynamic_cast<CommonWrapper*>(first.requireNode("breakToWidth")),state_.width);
        (void)breakToWidth(*dynamic_cast<CommonWrapper*>(second.requireNode("breakToWidth")),state_.width);
    }
    return visitDefault(wrapper,args);
}
std::any LinebreakVisitor::visitMsqrtNode(CommonWrapper& wrapper,std::span<const std::any> args){
    auto* root=dynamic_cast<CommonMsqrt*>(&wrapper);if(!root)throw options::Exception("TypeError",u"msqrt wrapper has incompatible type");
    if(root->getOuterBBox()->w>state_.width){
        if(!root->childNodes||!(*root->childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
        auto* base=dynamic_cast<CommonWrapper*>((*root->childNodes)->get(root->base()).requireNode("rootWidth"));
        auto width=oneIndex(lineArgument(args));(void)breakToWidth(*base,state_.width-root->rootWidth());
        root->getStretchedSurd();
    }
    return visitDefault(wrapper,args);
}
std::any LinebreakVisitor::visitMrootNode(CommonWrapper& wrapper,std::span<const std::any> args){return visitMsqrtNode(wrapper,args);}
std::any LinebreakVisitor::visitMsubNode(CommonWrapper& wrapper,std::span<const std::any> args){
    (void)visitDefault(wrapper,args);
    auto* sub=dynamic_cast<CommonMsub*>(&wrapper);if(!sub)throw options::Exception("TypeError",u"msub wrapper has incompatible type");
    auto* script=sub->scriptChild();if(!script)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
    const auto box=script->getOuterBBox();const auto [left,right]=borderLR(wrapper);
    addWidth(*sub->getLineBBox(lineArgument(args)),sub->getOffset()[0]+left+box->rscale*box->w+sub->font->params().scriptspace+right);return {};
}
std::any LinebreakVisitor::visitMsupNode(CommonWrapper& wrapper,std::span<const std::any> args){
    (void)visitDefault(wrapper,args);
    auto* sup=dynamic_cast<CommonMsup*>(&wrapper);if(!sup)throw options::Exception("TypeError",u"msup wrapper has incompatible type");
    auto* script=sup->scriptChild();if(!script)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
    const auto box=script->getOuterBBox();const auto [left,right]=borderLR(wrapper);
    addWidth(*sup->getLineBBox(lineArgument(args)),sup->getOffset()[0]+left+box->rscale*box->w+sup->font->params().scriptspace+right);return {};
}
std::any LinebreakVisitor::visitMsubsupNode(CommonWrapper& wrapper,std::span<const std::any> args){
    (void)visitDefault(wrapper,args);
    auto* both=dynamic_cast<CommonMsubsup*>(&wrapper);if(!both)throw options::Exception("TypeError",u"msubsup wrapper has incompatible type");
    auto* sub=both->subChild();auto* sup=both->supChild();
    if(!sub||!sup)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
    const auto subbox=sub->getOuterBBox(),supbox=sup->getOuterBBox();
    const double width=std::max(subbox->rscale*subbox->w,both->getAdjustedIc()+supbox->rscale*supbox->w)+both->font->params().scriptspace;
    const auto [left,right]=borderLR(wrapper);addWidth(*both->getLineBBox(lineArgument(args)),left+width+right);return {};
}
std::any LinebreakVisitor::visitMmultiscriptsNode(CommonWrapper& wrapper,std::span<const std::any> args){
    auto* scripts=dynamic_cast<CommonMmultiscripts*>(&wrapper);if(!scripts)throw options::Exception("TypeError",u"mmultiscripts wrapper has incompatible type");
    const auto& data=scripts->scriptsData();
    if(data.numPrescripts){const double width=std::max(data.psup.rscale*data.psup.w,data.psub.rscale*data.psub.w);addWidth(*wrapper.getLineBBox(lineArgument(args)),width+scripts->font->params().scriptspace);}
    (void)visitDefault(wrapper,args);
    if(data.numScripts){const double width=std::max(data.sup.rscale*data.sup.w,data.sub.rscale*data.sub.w);addWidth(*wrapper.getLineBBox(lineArgument(args)),width+scripts->font->params().scriptspace);}
    return {};
}
std::any LinebreakVisitor::visitMfencedNode(CommonWrapper& wrapper,std::span<const std::any> args){
    auto* fenced=dynamic_cast<CommonMfenced*>(&wrapper);if(!fenced)throw options::Exception("TypeError",u"mfenced wrapper has incompatible type");
    const auto i=lineArgument(args);auto line=fenced->getLineBBox(i);const auto [left,right]=borderLR(wrapper);
    if(i==0)addWidth(*line,line->L+left);if(!fenced->mrow)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'breakCount')");
    auto childArgs=oneIndex(i);(void)visitNode(*fenced->mrow,childArgs);if(i==wrapper.breakCount())addWidth(*line,line->R+right);return {};
}
std::any LinebreakVisitor::visitMactionNode(CommonWrapper& wrapper,std::span<const std::any> args){
    auto* action=dynamic_cast<CommonMaction*>(&wrapper);
    if(!action)throw options::Exception("TypeError",u"maction wrapper has incompatible type");
    const auto i=lineArgument(args);auto line=action->getLineBBox(i);const auto [left,right]=borderLR(wrapper);
    if(i==0)addWidth(*line,line->L+left);
    auto* selected=action->selected();
    if(!selected)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'visitNode')");
    auto childArgs=oneIndex(i);(void)visitNode(*selected,childArgs);
    if(i==action->breakCount())addWidth(*line,line->R+right);
    return {};
}

CommonMo* LinebreakVisitor::getPrevious(CommonMo& mo)const{
    auto* child=mo.node;auto parent=child->getParent();auto index=parent.isNode()?parent.nodeOrNull()->childIndex(mml::NodeValue(child)):std::nullopt;
    while(parent.isNode()&&(property_host::truthy(parent.nodeOrNull()->notParent())||parent.nodeOrNull()->isKind("mrow"))&&index&&*index==0){
        child=parent.nodeOrNull();parent=child->getParent();index=parent.isNode()?parent.nodeOrNull()->childIndex(mml::NodeValue(child)):std::nullopt;
    }
    if(!parent.isNode()||!index||*index==0)return nullptr;
    const auto previous=parent.nodeOrNull()->childArrayOwner()->get(*index-1);if(!previous.isNode()||!previous.nodeOrNull()->isEmbellished())return nullptr;
    const auto core=previous.nodeOrNull()->coreMO();if(!core.isNode())return nullptr;
    return dynamic_cast<CommonMo*>(factory().jax().nodeMap->find(core.nodeOrNull()));
}
double LinebreakVisitor::moPenalty(CommonMo& mo){
    const auto attrs=mo.node->attributes();const auto linebreak=attrs->get("linebreak"),fence=attrs->get("fence"),form=attrs->get("form");
    double penalty=std::floor((state_.width/jsMax(0.0001,state_.mathLeft-state_.w))*500)+std::floor(((state_.width-state_.w)/state_.width)*2500);
    const auto texClass=mo.node->texClass();
    const bool open=(property_host::truthy(fence)&&form==Property("prefix"))||number(texClass)==mml::TEXCLASS.OPEN;
    const bool close=(property_host::truthy(fence)&&form==Property("postfix"))||number(texClass)==mml::TEXCLASS.CLOSE;
    if(open){
        const auto previous=mo.node->prevClass();if(number(previous)==mml::TEXCLASS.BIN||number(previous)==mml::TEXCLASS.REL||number(previous)==mml::TEXCLASS.OP)penalty+=5000;
        else {auto* prev=getPrevious(mo);if(prev&&(prev->node->attributes()->get("form")!=Property("postfix")||prev->node->attributes()->get("linebreak")==Property("nobreak")))penalty+=5000;else penalty-=500;}
        const auto parent=mo.node->getParent();
        if(parent.isNode()&&parent.nodeOrNull()->isKind("mmultiscripts")){
            auto* first=parent.nodeOrNull();
            const auto firstToken=[&](auto&& self,mml::MmlNode* current)->mml::MmlNode*{
                if(current->isToken())return current;
                return self(self,current->childArrayOwner()->get(0).requireNode("0"));
            };
            if(firstToken(firstToken,first)==mo.node){
                bool prescripts=false;
                const auto children=first->childArrayOwner();
                for(std::size_t i=0;i<children->size();++i)
                    if(children->get(i).isNode()&&children->get(i).nodeOrNull()->isKind("mprescripts")){prescripts=true;break;}
                if(prescripts)penalty=NOBREAK;
            }
        }
        ++state_.depth;
    }
    if(close){
        bool scriptClose=false;
        const auto parent=mo.node->getParent();
        if(parent.isNode()&&parent.nodeOrNull()->isKind("msubsup")){
            auto* script=parent.nodeOrNull();
            bool rightPrescripts=false;
            if(script->isKind("mmultiscripts")){
                const auto children=script->childArrayOwner();
                if(children&&children->size()>1){const auto slot=children->get(1);rightPrescripts=slot.isNode()&&slot.nodeOrNull()->isKind("mprescripts");}
            }
            if(!rightPrescripts){
                const auto children=script->childArrayOwner();
                if(children&&children->size()){
                    auto* last=children->get(0).requireNode("0");
                    while(!last->isToken())last=last->childArrayOwner()->get(last->childArrayOwner()->size()-1).requireNode("last");
                    scriptClose=last==mo.node;
                }
            }
        }
        penalty=scriptClose?NOBREAK:penalty+500;
        --state_.depth;
    }
    if(number(texClass)==mml::TEXCLASS.BIN)penalty-=250;else if(number(texClass)==mml::TEXCLASS.REL)penalty-=500;
    penalty+=800*state_.depth;
    const auto style=property_host::truthy(linebreak)?text(linebreak):"auto";
    if(style=="newline")return 0;
    if(style=="nobreak")return NOBREAK;
    if(style=="goodbreak")return penalty-200*state_.depth;
    if(style=="badbreak")return penalty+200*state_.depth;
    return penalty;
}
double LinebreakVisitor::mspacePenalty(CommonMspace& node){
    const auto attr=node.node->attributes()->get("linebreak");
    const auto box=node.getBBox();const double width=box->w;
    double penalty=std::floor((state_.width/jsMax(0.0001,state_.mathLeft-state_.w))*500)+std::floor(((state_.width-state_.w)/state_.width)*2500);
    if(!node.canBreak()||width<0)penalty=NOBREAK;else if(width>=1)penalty-=100*(width+4);
    penalty+=800*state_.depth;const auto style=property_host::truthy(attr)?text(attr):"auto";
    if(style=="newline")return 0;if(style=="nobreak")return NOBREAK;if(style=="goodbreak")penalty-=200*state_.depth;if(style=="badbreak")penalty+=200*state_.depth;return penalty;
}
}
