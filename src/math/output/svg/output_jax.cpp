/*************************************************************
 * Source SVG output-jax lifecycle. Scene placement is performed exactly once
 * and the resulting immutable scene is then materialized through the adaptor.
 *************************************************************/
module folia.math.output.svg.output_jax;
import folia.math.cancellation;
import folia.math.output.scene;
import folia.math.output.svg.menclose_surface;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.lengths;
import folia.math.util.styles;
import folia.math.util.string;
import std;

namespace folia::math::output::svg {
namespace {
using namespace options;
constexpr std::string_view SVG_CACHE_ID="MJX-SVG-global-cache";
constexpr std::string_view SVG_STYLE_ID="MJX-SVG-styles";
scene::NodeRef cloneSceneNodeWithoutChildren(const scene::NodeRef& source){
    if(!source)throw Exception("TypeError",u"Cannot clone a null Scene node");
    auto clone=std::make_shared<scene::Node>();
    clone->kind=source->kind;
    const auto keys=source->attributes.keys();
    for(const auto& key:keys){
        clone->attributes.set(key,source->attributes.getOwn(key));
    }
    clone->geometry=source->geometry;
    clone->semantic=source->semantic;
    clone->paint=source->paint;
    if(source->paintOwner)clone->paintOwner=source->paintOwner->cloneFor(clone);
    clone->transform=source->transform;
    clone->transformSuffix=source->transformSuffix;
    clone->transformAttributeIndex=source->transformAttributeIndex;
    if(source->geometryOwner)clone->geometryOwner=source->geometryOwner->cloneFor(clone);
    return clone;
}
std::string trim(std::string value){
    while(!value.empty()&&value.back()=='0')value.pop_back();
    if(!value.empty()&&value.back()=='.')value.pop_back();
    return value;
}
std::u16string stringValue(const Property& value){return property_host::string(value);}
Property metric(const Property& value,std::string_view key){return property_host::get(value,key);}
void setSceneStyle(const scene::NodeRef& node,std::u16string_view name,std::string value){
    const auto previous=node->attributes.get("style");
    styles::Styles style(std::holds_alternative<Missing>(previous.value)
        ?std::u16string{}:property_host::string(previous));
    style.set(name,property_host::text(value));
    node->attributes.set("style",Property(property_host::debugText(style.cssText())));
}
}

SvgOutputJax::SvgOutputJax(OptionFunctionBindings& functions,ObjectRef identity,
    FunctionListContextBindings& filters,AbstractDOMAdaptor* adaptor,
    std::shared_ptr<core::JaxFilterRecordOwnerScope> owner)
    :CommonOutputJax(functions,identity,filters,adaptor,std::move(owner)){}

void SvgOutputJax::initializeSvg(Value supplied,ObjectRef defaultFactory,
    ObjectRef defaultFont,common::CommonRuntimeBindings& runtime,std::shared_ptr<void> owner){
    propertyBindings_=&runtime.properties;
    initializeCommon(std::move(supplied),defaultFactory,defaultFont,runtime,std::move(owner));
    sceneOutputConsumer_=[this](std::shared_ptr<const scene::Scene> source,DOMNodeRef parent){
        for(const auto& node:materialized(*source))domAdaptor().append(parent,node);
    };
    placements_.setDisplayHandler([this](common::CommonMath& wrapper){handleMathDisplay(wrapper);});
    placements_.setTextMeasurement([this](const scene::NodeRef& node,
        std::u16string_view chars,std::string_view variant){
        return measureTextWithCache(chars,variant,{},[this,node,chars,variant]{
            auto provider=textMeasurementProvider_;
            if(provider)return measuredTextWidth(provider(chars));
            scene::Builder snapshot;
            snapshot.addRoot(node);
            const auto completed=std::move(snapshot).finish();
            const auto nodes=materialized(completed);
            return measureTextNode(nodes.front());
        });
    });
    if(!placementsReady_){
        scene::registerScenePlacements(placements_);
        if(mencloseSurface_)
            scene::registerMenclosePlacement(placements_,*mencloseSurface_);
        placementsReady_=true;
    }
    auto& context=optionsContext();
    context.set(options,Key(u"matchFontHeight"),Value(true));
}

void SvgOutputJax::handleMathDisplay(common::CommonMath& wrapper){
    if(!container)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'setAttribute')");
    domAdaptor().setAttribute(container,u"display",Property(std::string("true")));
    const auto [align,shiftValue]=wrapper.sourceAlignShift();
    if(align!="center")domAdaptor().setAttribute(container,u"justify",Property(align));
    const auto bbox=wrapper.bbox;
    if(bbox->pwidth==BBox::fullWidth){
        domAdaptor().setAttribute(container,u"width",Property(std::string("full")));
        if(table){
            auto outer=table->getOuterBBox();
            double left=outer->L,width=outer->w,right=outer->R;
            if(align=="right")right=std::max(right?right:-shiftValue,-shiftValue);
            else if(align=="left")left=std::max(left?left:shiftValue,shiftValue);
            else if(align=="center")width+=2*std::abs(shiftValue);
            minwidth=std::max(0.0,left+width+right);
        }
    }else{
        shift=shiftValue;
    }
}


bool SvgOutputJax::forceInlineBreaks()const{
    auto& context=const_cast<SvgOutputJax*>(this)->optionsContext();
    return options::truthy(context.get(context.get(options,Key(u"linebreaks")),Key(u"inline")));
}


core::MathOutput SvgOutputJax::escaped(core::MathItem& item,core::MathDocumentReference documentValue){
    setDocument(documentValue);
    const auto textNode=text(stringValue(item.math));
    return html(u"span",{},PropertyArray{nodeProperty(textNode)});
}

core::MathOutput SvgOutputJax::styleSheet(core::MathDocumentReference documentValue){
    if(svgStyles_)return svgStyles_;
    const auto result=CommonOutputJax::styleSheet(documentValue);
    const auto* node=std::get_if<DOMNodeRef>(&result);
    if(!node)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'setAttribute')");
    svgStyles_=*node;
    domAdaptor().setAttribute(svgStyles_,u"id",Property(std::string(SVG_STYLE_ID)));
    return svgStyles_;
}

core::MathOutput SvgOutputJax::pageElements(core::MathDocumentReference documentValue){
    auto& context=optionsContext();
    const auto mode=context.get(options,Key(u"fontCache"));
    if(mode==Value(u"none"))return PropertyNull{};
    if(mode!=Value(u"global"))throw Exception("Unimplemented",u"SVG local font cache is not yet scene-backed");
    if(findCache(documentValue))return PropertyNull{};
    throw Exception("Unimplemented",u"SVG global font cache requires a retained document-level scene cache");
}

void SvgOutputJax::insertStyles(const style_json::StyleJson& styles){
    if(svgStyles_)domAdaptor().insertRules(svgStyles_,style_json::StyleJsonSheet(styles).getStyleRules());
}

std::string SvgOutputJax::ex(double value)const{
    const double xHeight=font->params().x_height;
    value/=xHeight;
    if(std::abs(value)<.001)return "0";
    return trim(js_number::toFixed(value,3))+"ex";
}

std::vector<DOMNodeRef> SvgOutputJax::materialized(const scene::Scene& source){
    Serializer serializer(domAdaptor());
    return serializer.materialize(source);
}

common::UnknownBBox SvgOutputJax::measuredTextWidth(double width)const{
    return {width,.75,.2};
}

std::array<scene::NodeRef,2> SvgOutputJax::createSVG(scene::Builder& builder,
    double h,double d,double w,DOMNodeRef projectedParent){
    if(!math)throw Exception("TypeError",u"Cannot read properties of null (reading 'metrics')");
    const double em=property_host::number(metric(math->metrics,"em"));
    const double px=em/1000;
    const double W=std::max(w,px),H=std::max(h+d,px);
    auto flip=builder.node(u"g",PropertyList{{"stroke",Property("currentColor")},
        {"fill",Property("currentColor")},{"stroke-width",Property(0.0)},
        {"transform",Property("scale(1,-1)")}});
    flip->transform=scene::Transform{0,0,1,-1};
    flip->paint=scene::ShapePaint{.fill=std::string("currentColor"),
        .stroke=std::string("currentColor"),.strokeWidth=0,.emitAttributes=false};
    const auto viewBox=std::string("0 ")+fixed(-h*1000,1)+" "+
        fixed(W*1000,1)+" "+fixed(H*1000,1);
    PropertyList rootAttributes{{"xmlns",Property(property_host::debugText(SVGNS))},
        {"width",Property(ex(W))},{"height",Property(ex(H))},{"role",Property("img")},
        {"focusable",Property(false)},
        {"style",Property(PropertyList{{"vertical-align",Property(ex(-d))}})},
        {"viewBox",Property(viewBox)}};
    auto root=builder.node(u"svg",std::move(rootAttributes),{},1);
    root->geometry=scene::ViewportGeometry{0,-h,W,H,0,-h,W,H,true,false};
    std::get<scene::ViewportGeometry>(root->geometry).clipToViewport=false;
    builder.append(root,flip);
    if(projectedParent)appendProjectedRoot(projectedParent,root);
    const auto useXlink=options::truthy(optionsContext().get(options,Key(u"useXlink")));
    if(W==.001){
        root->attributes.set("preserveAspectRatio",Property("xMidYMid slice"));
        std::get<scene::ViewportGeometry>(root->geometry).fit=scene::ViewportFit::Slice;
        std::get<scene::ViewportGeometry>(root->geometry).emitFit=true;
        if(w<0)domAdaptor().setStyle(container,u"margin-right",property_host::text(ex(w)));
    }
    const auto mode=optionsContext().get(options,Key(u"fontCache"));
    if(mode!=Value(u"none")&&useXlink)
        root->attributes.set("xmlns:xlink",Property(property_host::debugText(XLINKNS)));
    return {root,flip};
}

void SvgOutputJax::addInlineBreak(std::vector<scene::NodeRef>& roots,double space,bool forced){
    const auto append=[&](bool prebreak){
        auto node=std::make_shared<scene::Node>();
        node->kind=u"mjx-break";
        node->semantic=scene::InlineBreakPrimitive{space,forced,prebreak};
        roots.push_back(std::move(node));
    };
    if(!forced)append(true);
    append(false);
}

std::vector<scene::NodeRef> SvgOutputJax::handleInlineBreaks(common::CommonWrapper& wrapper,
    scene::Builder& builder,const scene::NodeRef& svg,const scene::NodeRef& group,
    DOMNodeRef nestedParent){
    if(!wrapper.childNodes||!*wrapper.childNodes||!(*wrapper.childNodes)->size())
        throw Exception("TypeError",u"Cannot read properties of undefined (reading 'breakCount')");
    auto* row=(*wrapper.childNodes)->get(0).requireNode("breakCount");
    const auto count=row->breakCount();
    if(!count)return {svg};
    if(group->children.empty()||group->children.front()->children.empty())
        throw Exception("TypeError",u"Cannot read properties of null (reading 'children')");
    const auto mathNode=group->children.front();
    const auto& lines=mathNode->children.front()->children;
    // Source detaches the typeset group before creating any line SVG.  The
    // local group/mathNode references keep its line list alive while Builder
    // reparents line children into their completed line groups below.
    if(nestedParent)removeProjectedRoot(group);
    std::vector<scene::NodeRef> roots;
    for(std::size_t index=0;index<=count;++index){
        const auto line=index<row->lineBBox.size()&&row->lineBBox[index]
            ?row->lineBBox[index]:row->getLineBBox(index);
        const auto [breakWrapper,mo]=row->getBreakNode(*line);
        const double scale=breakWrapper->getBBox()->scale;
        auto [nextSvg,nextGroup]=createSVG(builder,line->h*scale,line->d*scale,line->w*scale,
            nestedParent);
        // Source uses adaptor.clone(math,false): copy current own fields into a
        // fresh node with an independent, empty child list.  ChildList itself
        // intentionally shares its owner on ordinary copies.
        auto nextMath=cloneSceneNodeWithoutChildren(mathNode);
        if(index>=lines.size())
            throw Exception("TypeError",u"Cannot read properties of undefined (reading 'children')");
        for(const auto& child:lines[index]->children.snapshot())builder.append(nextMath,child);
        builder.append(nextGroup,nextMath);
        if(nestedParent)insertProjectedBefore(nextSvg,svg);
        const bool forced=mo&&property_host::truthy(mo->node->getProperty("forcebreak"));
        const auto breakStart=roots.size();
        if(forced&&mo->node->attributes()->get("linebreakstyle")==Property("after")){
            auto* parent=breakWrapper->parent;
            if(!parent)throw Exception("TypeError",u"Cannot read properties of null (reading 'node')");
            const auto childIndex=parent->node->childIndex(mml::NodeValue(breakWrapper->node));
            const auto children=parent->childNodes;
            common::CommonWrapper* next=nullptr;
            if(childIndex&&children&&*children&&*childIndex+1<(*children)->size())
                next=(*children)->get(*childIndex+1).requireNode("getLineBBox");
            const double space=next?next->getLineBBox(0)->originalL*scale:0;
            if(space)addInlineBreak(roots,space,forced);
        }else if(forced||index){
            const double space=breakWrapper&&index?breakWrapper->getLineBBox(0)->originalL*scale:0;
            if(space||!forced)addInlineBreak(roots,space,
                forced||property_host::truthy(breakWrapper->node->getProperty("forcebreak")));
        }
        if(nestedParent)for(std::size_t at=breakStart;at<roots.size();++at)
            insertProjectedBefore(roots[at],nextSvg);
        roots.push_back(std::move(nextSvg));
    }
    // The Source moves any remaining defs from the original SVG to the
    // first line after removing its typeset group.
    for(const auto& child:svg->children.snapshot()){
        if(child!=group)builder.append(roots.front(),child);
    }
    if(nestedParent)removeProjectedRoot(svg);
    return roots;
}

void SvgOutputJax::appendProjectedRoot(DOMNodeRef parent,const scene::NodeRef& root){
    if(!sceneContextNodes_||!propertyBindings_)
        throw Exception("Unimplemented",u"Nested Scene projection is not bound");
    auto& context=optionsContext();
    const Value parentIdentity=propertyBindings_->to(nodeProperty(parent));
    if(!sceneContextNodes_->projectSourceChild(parentIdentity))
        throw Exception("TypeError",u"Nested Scene parent has no canonical projection");
    const Value adaptor=context.get(Value(instanceIdentity()),Key(u"adaptor"));
    const Value append=context.get(adaptor,Key(u"append"));
    const std::array<Value,2> args={parentIdentity,Value(sceneContextNodes_->handle(root))};
    (void)context.callMember(append,adaptor,args,u"adaptor.append");
}

void SvgOutputJax::insertProjectedBefore(const scene::NodeRef& child,
    const scene::NodeRef& sibling){
    auto& context=optionsContext();
    const Value adaptor=context.get(Value(instanceIdentity()),Key(u"adaptor"));
    const Value insert=context.get(adaptor,Key(u"insert"));
    const std::array<Value,2> args={Value(sceneContextNodes_->handle(child)),
        Value(sceneContextNodes_->handle(sibling))};
    (void)context.callMember(insert,adaptor,args,u"adaptor.insert");
}

void SvgOutputJax::removeProjectedRoot(const scene::NodeRef& root){
    auto& context=optionsContext();
    const Value adaptor=context.get(Value(instanceIdentity()),Key(u"adaptor"));
    const Value remove=context.get(adaptor,Key(u"remove"));
    const std::array<Value,1> args={Value(sceneContextNodes_->handle(root))};
    (void)context.callMember(remove,adaptor,args,u"adaptor.remove");
}

std::vector<scene::NodeRef> SvgOutputJax::buildScene(
    common::CommonWrapper& wrapper,DOMNodeRef parent,scene::Builder& builder,
    bool appendToProjectedParent){
    const auto mode=optionsContext().get(options,Key(u"fontCache"));
    if(mode!=Value(u"none"))
        throw Exception("Unimplemented",u"SVG font caches require the completed Scene writer");
    struct ContainerScope {
        DOMNodeRef& target;DOMNodeRef previous;
        ContainerScope(DOMNodeRef& target,DOMNodeRef current)
            :target(target),previous(std::exchange(target,std::move(current))){}
        ~ContainerScope(){target=std::move(previous);}
    } scope(container,parent);
    const auto bbox=wrapper.getOuterBBox();

    auto [root,flip]=createSVG(builder,bbox->h,bbox->d,bbox->w,
        appendToProjectedParent?parent:DOMNodeRef{});
    if(!bbox->pwidth.empty()){
        styles::Styles style(property_host::string(root->attributes.get("style")));
        setSceneStyle(root,u"min-width",property_host::debugText(style.get(u"width")));
        root->attributes.set("width",Property(bbox->pwidth));
        root->attributes.set("data-mjx-viewBox",root->attributes.get("viewBox"));
        root->attributes.erase("viewBox");
        std::get<scene::ViewportGeometry>(root->geometry).hasViewBox=false;
        const auto metrics=wrapper.metrics();
        const double scale=metrics.ex/(font->params().x_height*1000);
        scene::TransformOperation scaling{scene::TransformKind::Scale,scale,-scale};
        scaling.scaleDigits=6;
        scene::TransformOperation translation{scene::TransformKind::Translate,0,-bbox->h};
        translation.commaSpaceSeparated=true;
        flip->transform=scene::Transform{0,0,1,1,{scaling,translation}};
        // Source's SVG wrapper writes its generated full-width transform into
        // the existing transform attribute slot.  Keep that ordering metadata
        // when replacing the typed transform installed by createSVG().
        if(!flip->transformAttributeIndex)
            flip->transformAttributeIndex=flip->attributes.keys().size();
    }
    minwidth=shift=0;
    const std::array<scene::NodeRef,1> parents{flip};
    cancellation::checkpoint(cancellation::CheckpointStage::Layout);
    (void)placements_.build(wrapper,builder,parents);
    cancellation::checkpoint(cancellation::CheckpointStage::Layout);
    if(minwidth){
        setSceneStyle(root,u"minWidth",ex(minwidth));
        domAdaptor().setStyle(parent,u"min-width",property_host::text(ex(minwidth)));
    }else if(shift){
        const auto justify=domAdaptor().getAttribute(parent,u"justify");
        const auto align=property_host::truthy(justify)?property_host::debugText(property_host::string(justify)):std::string("center");
        if(align=="center"||align=="left")setSceneStyle(root,u"margin-left",ex(shift));
        if(align=="center"||align=="right")setSceneStyle(root,u"margin-right",ex(-shift));
    }
    const auto roots=property_host::truthy(wrapper.node->getProperty("process-breaks"))
        ?handleInlineBreaks(wrapper,builder,root,flip,
            appendToProjectedParent?parent:DOMNodeRef{}):std::vector<scene::NodeRef>{root};
    return roots;
}

std::shared_ptr<scene::Scene> SvgOutputJax::produceScene(
    common::CommonWrapper& wrapper,DOMNodeRef parent){
    // Keep Source's container value visible through the top-level freeze.
    struct ContainerScope {
        DOMNodeRef& slot;
        DOMNodeRef previous;
        ContainerScope(DOMNodeRef& slot,DOMNodeRef current)
            :slot(slot),previous(std::exchange(slot,std::move(current))){}
        ~ContainerScope(){slot=std::move(previous);}
    } containerScope(container,parent);
    scene::Builder builder;
    builder.setLayoutObserver(placements_.layoutObserver());
    struct ActiveBuilderScope {
        scene::Builder*& slot;
        scene::Builder* previous;
        ActiveBuilderScope(scene::Builder*& slot,scene::Builder& builder)
            :slot(slot),previous(std::exchange(slot,&builder)){}
        ~ActiveBuilderScope(){slot=previous;}
    } active{activeSceneBuilder_,builder};
    const auto roots=buildScene(wrapper,parent,builder,false);
    for(const auto& node:roots)builder.addRoot(node);
    cancellation::checkpoint(cancellation::CheckpointStage::Scene);
    auto sceneValue=std::make_shared<scene::Scene>(std::move(builder).finish());
    cancellation::checkpoint(cancellation::CheckpointStage::Scene);
    return sceneValue;
}

void SvgOutputJax::processMath(common::CommonWrapper& wrapper,DOMNodeRef parent){
    if(activeSceneBuilder_){
        if(!sceneContextNodes_||!liteModule_)
            throw Exception("Unimplemented",u"Nested Scene target is not bound");
        auto targets=sceneContextNodes_->installTargets(*liteModule_,*activeSceneBuilder_);
        (void)buildScene(wrapper,std::move(parent),*activeSceneBuilder_,true);
        return;
    }
    auto sceneValue=produceScene(wrapper,parent);
    if(!sceneOutputConsumer_)
        throw Exception("TypeError",u"SVG output Jax has no Scene output consumer");
    auto consumer=sceneOutputConsumer_;
    consumer(sceneValue,std::move(parent));
    completedScene_=std::move(sceneValue);
}

bool SvgOutputJax::findCache(core::MathDocumentReference reference)const{
    if(const auto* document=std::get_if<core::MathDocumentService*>(&reference);document&&*document){
        const auto value=(**document).documentNode();
        const auto* node=std::get_if<DOMNodeRef>(&value);
        if(!node)return false;
        const auto body=domAdaptor().body(DOMNodeValue(*node));
        const auto* bodyNode=std::get_if<DOMNodeRef>(&body);
        if(!bodyNode)return false;
        const auto svgs=domAdaptor().tags(*bodyNode,u"svg");
        for(auto it=svgs.rbegin();it!=svgs.rend();++it)
            if(domAdaptor().getAttribute(*it,u"id")==Property(std::string(SVG_CACHE_ID)))return true;
        return false;
    }
    return false;
}

DOMNodeRef SvgOutputJax::unknownText(std::u16string_view textValue,std::string_view variant){
    const auto metrics=currentMetrics();
    const double scale=font->params().x_height/metrics.ex*metrics.em*1000;
    PropertyList attributes{{"data-variant",Property(std::string(variant))},
        {"transform",Property("scale(1,-1)")},
        {"font-size",Property(fixed(scale,1)+"px")}};
    auto textNode=domAdaptor().text(textValue);
    auto svg=sourceSvgNode(u"text",std::move(attributes),PropertyArray{nodeProperty(textNode)});
    if(variant!="-explicitFont"){
        const auto codepoints=string_util::unicodeChars(textValue);
        const bool math=codepoints.size()==1&&codepoints.front()>=0x1D400&&codepoints.front()<=0x1D7FF;
        if(!math){
            const auto css=font->getCssFont(variant);
            domAdaptor().setAttribute(svg,u"font-family",css.get(0));
            if(property_host::truthy(css.get(1)))domAdaptor().setAttribute(svg,u"font-style",Property("italic"));
            if(property_host::truthy(css.get(2)))domAdaptor().setAttribute(svg,u"font-weight",Property("bold"));
        }
    }
    return svg;
}

common::UnknownBBox SvgOutputJax::measureTextNode(DOMNodeRef node){
    auto& adaptor=domAdaptor();
    auto text=adaptor.clone(node);
    adaptor.removeAttribute(text,u"transform");
    const auto ex=fixed(font->params().x_height*1000,1);
    auto svg=sourceSvgNode(u"svg",PropertyList{{"position",Property("absolute")},
        {"visibility",Property("hidden")},{"width",Property("1ex")},
        {"height",Property("1ex")},{"top",Property(0)},{"left",Property(0)},
        {"viewBox",Property("0 0 "+ex+" "+ex)}},PropertyArray{nodeProperty(text)});
    const auto body=adaptor.body(adaptor.document);
    const auto* actualBody=std::get_if<DOMNodeRef>(&body);
    if(!actualBody)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'append')");
    adaptor.append(*actualBody,svg);
    const auto size=adaptor.nodeSize(text,1000,true);
    adaptor.remove(svg);
    return measuredTextWidth(size.first);
}

DOMNodeRef SvgOutputJax::sourceSvgNode(std::u16string_view kind,PropertyList def,PropertyArray content){
    if(!propertyBindings_)throw Exception("Unimplemented",u"SVG property bindings are unavailable");
    auto& context=optionsContext();
    const auto receiver=Value(instanceIdentity());
    const auto method=context.get(receiver,Key(u"svg"));
    const auto* callable=std::get_if<ObjectRef>(&method.data);
    if(!callable||context.kind(*callable)!=ObjectKind::function)
        throw Exception("TypeError",u"this.svg is not a function");
    const std::array<Value,3> args{Value(std::u16string(kind)),
        propertyBindings_->to(Property(std::move(def))),
        propertyBindings_->to(Property(std::move(content)))};
    const auto returned=context.call(*callable,receiver,args);
    const auto node=propertyNode(propertyBindings_->from(returned));
    if(const auto* actual=std::get_if<DOMNodeRef>(&node))return *actual;
    throw Exception("TypeError",u"SVG method did not return a DOM node");
}

void SvgOutputJax::bindSceneContextNodes(std::shared_ptr<SceneContextNodes> nodes,
    std::shared_ptr<adaptors::lite::LiteModule> lite){
    if(!nodes||!lite||&nodes->context()!=&optionsContext()||&lite->context()!=&optionsContext())
        throw Exception("TypeError",u"SVG scene bridge belongs to another Context");
    sceneContextNodes_=std::move(nodes);liteModule_=std::move(lite);
    sceneContextNodes_->setSourceChildResolver([this](const Value& value){
        if(!propertyBindings_)throw Exception("Unimplemented",u"SVG property bindings are unavailable");
        const auto raw=propertyBindings_->from(value);
        const auto node=propertyNode(raw);
        const auto* dom=std::get_if<DOMNodeRef>(&node);
        if(!dom||!*dom)return scene::NodeRef{};
        return importXmlSvgNode(std::make_shared<scene::Node>(),*dom);
    });
}

scene::NodeRef SvgOutputJax::importXmlSvgNode(scene::Builder& builder,
    scene::NodeRef node,const DOMNodeRef& source){
    (void)builder;
    return importXmlSvgNode(std::move(node),source);
}

scene::NodeRef SvgOutputJax::importXmlSvgNode(scene::NodeRef node,const DOMNodeRef& source){
    const auto identity=propertyBindings_->to(nodeProperty(source));
    if(const auto mapped=sceneContextNodes_->sourceNode(identity))return *mapped;
    const auto kind=domAdaptor().kind(source);
    // LiteText has a value and parent, but no children field. Do not ask its
    // LiteAdaptor for childNodes while importing a leaf into the Scene graph.
    DOMNodeArray children;
    if(kind!=u"#text"&&kind!=u"#comment")children=domAdaptor().childNodes(source);
    node->kind=kind;
    node->attributes=PropertyList{};
    if(kind!=u"#text"&&kind!=u"#comment"){
        const auto attributesValue=optionsContext().get(identity,Key(u"attributes"));
        const auto rawAttributes=propertyBindings_->from(attributesValue);
        if(const auto attributes=PropertyList::record(rawAttributes))node->attributes=*attributes;
        else for(const auto& attribute:domAdaptor().allAttributes(source))
            node->attributes.set(property_host::debugText(attribute.name),attribute.value);
    }
    if(kind==u"#text")node->semantic=scene::SourceTextPrimitive{property_host::string(domAdaptor().getProperty(source,"value")),false};
    else if(kind==u"#comment")node->semantic=scene::SourceTextPrimitive{property_host::string(domAdaptor().getProperty(source,"value")),true};
    else node->semantic=scene::SourceDomPrimitive{};
    sceneContextNodes_->associateSourceNode(node,identity);
    (void)sceneContextNodes_->handle(node);
    for(std::size_t index=0;index<children.slots.size();++index){
        const auto childValue=propertyNode(children.slots.get(index));
        const auto* child=std::get_if<DOMNodeRef>(&childValue);
        if(!child||!*child)throw Exception("TypeError",u"SVG child slot is not a DOM node");
        (void)importXmlSvgNode(std::make_shared<scene::Node>(),*child);
    }
    return node;
}

void SvgOutputJax::setScale(DOMNodeRef node,common::CommonWrapper& wrapper){
    CommonOutputJax::setScale(node,wrapper);
}
options::Value SvgOutputJax::dispatchSvg(std::span<const options::Value> args){
    auto& context=optionsContext();
    const auto receiver=Value(instanceIdentity());
    const auto method=context.get(receiver,Key(u"html"));
    const auto* function=std::get_if<ObjectRef>(&method.data);
    if(!function||context.kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",u"this.html is not a function");
    if(sceneSvgTarget_&&defaultHtmlMethod_&&*function==*defaultHtmlMethod_){
        if(auto value=sceneSvgTarget_(args))return std::move(*value);
    }
    const auto def=args.size()<2||std::holds_alternative<Undefined>(args[1].data)
        ?Value(context.object()):args[1];
    const auto content=args.size()<3||std::holds_alternative<Undefined>(args[2].data)
        ?Value(context.array()):args[2];
    const auto observer=placements_.layoutObserver();
    if(observer&&observer->paint){
        const auto kind=property_host::string(propertyBindings_->from(
            context.toPrimitive(args.empty()?Value(Undefined{}):args[0],PrimitiveHint::stringHint)));
        const auto rawDef=propertyBindings_->from(def);
        const auto record=PropertyList::record(rawDef);
        if(!record)throw Exception("TypeError",u"SVG definition is not an object");
        const auto rawContent=propertyBindings_->from(content);
        const auto children=PropertyArray::array(rawContent);
        if(!children)throw Exception("TypeError",u"SVG content is not an array");
        observer->paint(nullptr,kind,*record,children->size());
    }
    std::array<Value,4> forwarded{args.empty()?Value(Undefined{}):args[0],
        def,content,Value(std::u16string(SVGNS))};
    return context.call(*function,receiver,forwarded);
}
}
