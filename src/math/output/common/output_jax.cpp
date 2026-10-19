// MathJax 4.1.3 ts/output/common.ts; source-ordered common output setup.
module folia.math.output.common;
import :output_jax;
import :bindings;
import folia.math.core.math_item;
import folia.math.core.math_item_host;
import folia.math.core.math_list;
import folia.math.core.math_location;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.utf16;
import folia.math.util.lengths;
import folia.math.core.mml_tree.mml_nodes.mspace;
import std;

namespace folia::math::output::common {
using namespace options;

namespace {
Value argumentOrUndefined(Context& context,const Value& options,std::u16string_view key){
    return context.get(options,Key(std::u16string(key)));
}
std::shared_ptr<style_json::StyleJsonSheet> styleSheetOwner(StyleJsonContextBindings& bindings,const Value& value){
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object)throw Exception("TypeError",u"StyleJsonSheet option is not an object");
    return bindings.resolve(*object);
}
double metricNumber(const Property& value,std::string_view name){
    return property_host::number(property_host::get(value,name));
}
std::string trimFixed(std::string value){
    while(!value.empty()&&value.back()=='0')value.pop_back();
    if(!value.empty()&&value.back()=='.')value.pop_back();
    return value;
}
std::string utf8(std::u16string_view value){
    std::string result;
    for(std::size_t i=0;i<value.size();++i){
        std::uint32_t c=value[i];
        if(c>=0xD800&&c<=0xDBFF&&i+1<value.size()&&value[i+1]>=0xDC00&&value[i+1]<=0xDFFF)
            c=0x10000+((c-0xD800)<<10)+(value[++i]-0xDC00);
        if(c<0x80)result.push_back(static_cast<char>(c));
        else if(c<0x800){result.push_back(static_cast<char>(0xC0|(c>>6)));result.push_back(static_cast<char>(0x80|(c&63)));}
        else if(c<0x10000){result.push_back(static_cast<char>(0xE0|(c>>12)));result.push_back(static_cast<char>(0x80|((c>>6)&63)));result.push_back(static_cast<char>(0x80|(c&63)));}
        else{result.push_back(static_cast<char>(0xF0|(c>>18)));result.push_back(static_cast<char>(0x80|((c>>12)&63)));result.push_back(static_cast<char>(0x80|((c>>6)&63)));result.push_back(static_cast<char>(0x80|(c&63)));}
    }
    return result;
}
std::u16string fromPropertyString(const Property& value){return property_host::string(value);}
bool arrayTruthy(const Property& value,std::size_t index){return property_host::truthy(property_host::get(value,std::to_string(index)));}
core::MathDocumentService& requireDocument(core::MathDocumentReference reference,std::string_view member){
    if(const auto* document=std::get_if<core::MathDocumentService*>(&reference);document&&*document)
        return **document;
    property_host::typeError(std::string("Cannot read properties of ")+
        (std::holds_alternative<Missing>(reference)?"undefined":"null")+
        " (reading '"+std::string(member)+"')");
}
bool belowMetrics(core::MathItem& item){
    return core::math_item_host::less(item.state(),core::STATE.get("METRICS"));
}
bool displayMath(const core::MathItem& item){return property_host::truthy(item.display);}
std::optional<DOMNodeRef> parentOfMath(core::MathItem& item,core::MathDocumentService& document,
                                      AbstractDOMAdaptor& adaptor){
    auto location=core::readMathLocation(item.start,"node",&document.locationService());
    auto nodeValue=propertyNode(std::move(location));
    if(const auto* node=std::get_if<DOMNodeRef>(&nodeValue)){
        auto parent=adaptor.parent(*node);
        if(const auto* ref=std::get_if<DOMNodeRef>(&parent))return *ref;
    }
    return {};
}
}

CommonOutputJax::CommonOutputJax(OptionFunctionBindings& functions,ObjectRef identity,
    FunctionListContextBindings& filters,AbstractDOMAdaptor* adaptor)
    :core::AbstractOutputJax(functions,identity,filters),domAdaptor_(adaptor){}

AbstractDOMAdaptor& CommonOutputJax::domAdaptor()const{
    if(!domAdaptor_)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'node')");
    return *domAdaptor_;
}
std::shared_ptr<AbstractDOMAdaptor> CommonOutputJax::domAdaptorOwner()const{
    if(!domAdaptorOwner_)
        throw Exception("TypeError",u"Output Jax has no retained DOM adaptor owner");
    return domAdaptorOwner_;
}

DOMNodeRef CommonOutputJax::sourceDOMField(const CommonWrapper& wrapper,
                                            std::u16string_view key)const{
    if(!runtime_||!factory)
        throw Exception("Unimplemented",u"Source DOM field access is not initialized");
    auto& context=runtime_->properties.context();
    const auto instance=factory->objectFor(wrapper);
    const auto source=context.get(Value(instance),Key(std::u16string(key)));
    const auto value=runtime_->properties.from(source);
    const auto node=propertyNode(value);
    if(const auto* reference=std::get_if<DOMNodeRef>(&node))return *reference;
    if(std::holds_alternative<Missing>(node))
        throw Exception("TypeError",u"Cannot read properties of undefined (reading 'node')");
    if(std::holds_alternative<PropertyNull>(node))
        throw Exception("TypeError",u"Cannot read properties of null (reading 'node')");
    throw Exception("TypeError",u"Source field is not a DOM node");
}

Value CommonOutputJax::sourceDOMValue(DOMNodeRef node)const{
    if(!runtime_)throw Exception("Unimplemented",u"Source DOM call is not initialized");
    return runtime_->properties.to(nodeProperty(std::move(node)));
}

Value CommonOutputJax::sourceAdaptorCall(std::u16string_view name,
    std::span<const Value> arguments)const{
    if(!runtime_)throw Exception("Unimplemented",u"Source DOM call is not initialized");
    const auto* documentValue=std::get_if<core::MathDocumentService*>(&document);
    if(!documentValue||!*documentValue)
        throw Exception("TypeError",u"Cannot read properties of null (reading 'adaptor')");
    Context& context=runtime_->context();
    const Value documentIdentity((*documentValue)->sourceIdentity());
    const Value adaptor=context.get(documentIdentity,Key(u"adaptor"));
    const Value method=context.get(adaptor,Key(std::u16string(name)));
    std::u16string label=u"adaptor.";label+=name;
    return context.callMember(method,adaptor,arguments,label);
}

mml::XmlDomAdaptor& CommonOutputJax::htmlAdaptor()const{
    if(!htmlAdaptorOwner_)
        throw Exception("TypeError",u"Cannot read properties of undefined (reading 'node')");
    return *htmlAdaptorOwner_;
}

bool CommonOutputJax::forceInlineBreaks()const{return false;}

double CommonOutputJax::getInitialScale()const{
    if(!math)throw Exception("TypeError",u"Cannot read properties of null (reading 'metrics')");
    return property_host::number(property_host::get(math->metrics,"scale"));
}

void CommonOutputJax::setScale(DOMNodeRef node,CommonWrapper& wrapper){
    const auto overflow=wrapper.node->attributes()->get("overflow");
    auto& cx=optionsContext();
    double scale=getInitialScale()*cx.toNumber(cx.get(options,Key(u"scale")));
    if(overflow==Property(std::string("scale"))&&math&&property_host::truthy(math->display)){
        const double width=wrapper.getOuterBBox()->w;
        const double rawAvailable=metricNumber(math->metrics,"containerWidth")-4;
        const double available=(std::isnan(rawAvailable)||std::isnan(pxPerEm))
            ?std::numeric_limits<double>::quiet_NaN():std::max(0.0,rawAvailable)/pxPerEm;
        if(width>available&&width)scale*=available/width;
    }
    if(scale!=1)domAdaptor().setStyle(node,u"fontSize",property_host::text(lengths::percent(scale)));
}

bool CommonOutputJax::markInlineBreak(bool marked,bool forcebreak,std::string_view linebreak,
    mml::MmlNode& parent,mml::MmlNode& child,mml::MmlNode* mo){
    child.setProperty("breakable",true);
    if(forcebreak&&linebreak!="newline"){
        child.setProperty("forcebreak",true);
        if(mo)mo->setProperty("forcebreak",true);
    }else{
        child.removeProperty(std::array<std::string,1>{"forcebreak"});
        if(mo)mo->removeProperty(std::array<std::string,1>{"forcebreak"});
        if(linebreak=="newline")child.setProperty("newline",true);
    }
    if(!marked){
        parent.setProperty("process-breaks",true);
        parent.getParent().requireNode("markInlineBreak")->setProperty("process-breaks",true);
        marked=true;
    }
    return marked;
}

void CommonOutputJax::markInlineBreaks(mml::NodeValue input){
    if(!input.isNode())return;
    auto& node=*input.requireNode("childNodes");
    const bool forcebreak=forceInlineBreaks();
    bool marked=false,postbreak=false;
    std::string markNext;
    const auto children=node.childArrayOwner();
    for(std::size_t i=0;children&&i<children->size();++i){
        auto& child=*children->get(i).requireNode("markInlineBreaks");
        if(!markNext.empty()){
            marked=markInlineBreak(marked,forcebreak,markNext,node,child);
            markNext.clear();postbreak=false;
        }else if(child.isEmbellished()){
            if(i==0)continue;
            auto* mo=child.coreMO().requireNode("markInlineBreaks");
            const auto texClass=mo->texClass();
            const auto attrs=mo->attributes();
            if(!attrs)throw Exception("TypeError",u"Cannot read properties of null (reading 'get')");
            const auto linebreak=attrs->get("linebreak");
            const auto style=attrs->get("linebreakstyle");
            const auto text=property_host::debugText(property_host::string(linebreak));
            const bool eligible=texClass==Property(static_cast<double>(mml::TEXCLASS.BIN))||
                texClass==Property(static_cast<double>(mml::TEXCLASS.REL))||
                (texClass==Property(static_cast<double>(mml::TEXCLASS.ORD))&&mo->hasSpacingAttributes())||
                linebreak!=Property(std::string("auto"));
            if(eligible&&linebreak!=Property(std::string("nobreak"))){
                if(style==Property(std::string("before"))){
                    if(!postbreak||linebreak!=Property(std::string("auto")))
                        marked=markInlineBreak(marked,forcebreak,text,node,child,mo);
                }else markNext=text;
            }
            postbreak=linebreak==Property(std::string("newline"))&&style==Property(std::string("after"));
        }else if(child.isKind("mspace")){
            auto* space=dynamic_cast<mml::MmlMspace*>(&child);
            if(!space)throw Exception("TypeError",u"mspace kind is not MmlMspace");
            const auto attrs=space->attributes();
            if(!attrs)throw Exception("TypeError",u"Cannot read properties of null (reading 'get')");
            const auto value=attrs->get("linebreak");
            const auto linebreak=property_host::debugText(property_host::string(value));
            if(value!=Property(std::string("nobreak"))&&space->canBreak())
                marked=markInlineBreak(marked,forcebreak,linebreak,node,child);
            postbreak=value==Property(std::string("newline"));
        }else{
            postbreak=false;
            const auto attrs=child.attributes();
            if(!attrs)throw Exception("TypeError",u"Cannot read properties of null (reading 'get')");
            if((child.isKind("mstyle")&&!property_host::truthy(attrs->get("style"))&&
                !attrs->hasExplicit("mathbackground"))||child.isKind("semantics")){
                const auto nested=child.childArrayOwner();
                if(nested&&!nested->empty())markInlineBreaks(nested->get(0));
                if(property_host::truthy(child.getProperty("process-breaks"))){
                    child.setProperty("inline-breaks",true);
                    if(nested&&!nested->empty())nested->get(0).requireNode("markInlineBreaks")->setProperty("inline-breaks",true);
                    node.getParent().requireNode("markInlineBreaks")->setProperty("process-breaks",u"true");
                }
            }else if(child.isKind("mrow")&&property_host::truthy(child.getProperty("data-semantic-added"))){
                markInlineBreaks(mml::NodeValue(&child));
                if(property_host::truthy(child.getProperty("process-breaks"))){
                    child.setProperty("inline-breaks",true);
                    node.getParent().requireNode("markInlineBreaks")->setProperty("process-breaks",u"true");
                }
            }
        }
    }
}

void CommonOutputJax::unmarkInlineBreaks(mml::NodeValue input){
    if(!input.isNode())return;
    auto& node=*input.requireNode("unmarkInlineBreaks");
    node.removeProperty(std::array<std::string,2>{"forcebreak","breakable"});
    node.coreMO().requireNode("unmarkInlineBreaks")->removeProperty(std::array<std::string,1>{"forcebreak"});
    if(!property_host::truthy(node.getProperty("process-breaks")))return;
    node.removeProperty(std::array<std::string,1>{"process-breaks"});
    const auto children=node.childArrayOwner();
    for(std::size_t i=0;children&&i<children->size();++i)
        unmarkInlineBreaks(children->get(i));
}

std::shared_ptr<LayoutJob> CommonOutputJax::beginLayoutJob(
    core::MathItem& item,core::MathDocumentReference documentValue,DOMNodeRef containerValue){
    auto job=std::make_shared<LayoutJob>(item,documentValue,containerValue);
    if(nodeMapFactory_){
        job->nodeMap=nodeMapFactory_();
        if(!job->nodeMap)
            throw Exception("TypeError",u"Source nodeMap factory returned no Map owner");
    }
    completedJob_.reset();
    activeJob_=job;
    math=&item;
    document=documentValue;
    container=containerValue;
    table=nullptr;
    nodeMap=job->nodeMap;
    if(nodeMapPublisher_){
        if(!nodeMap)throw Exception("TypeError",u"Source nodeMap is not initialized");
        nodeMapPublisher_(options::Value(nodeMap->contextObject()));
        nodeMapPublished_=true;
    }
    return job;
}

std::shared_ptr<LayoutJob> CommonOutputJax::activeLayoutJob()const{
    if(!activeJob_)
        throw Exception("TypeError",u"Cannot read properties of null (reading 'layout job')");
    return activeJob_;
}

void CommonOutputJax::setLayoutObserver(
    std::function<void(const CommonWrapper&,const LayoutJob&)> observer){
    layoutObserver_=std::move(observer);
}

void CommonOutputJax::processMathObserved(CommonWrapper& wrapper,DOMNodeRef parent){
    processMath(wrapper,std::move(parent));
    if(layoutObserver_&&activeJob_)layoutObserver_(wrapper,*activeJob_);
}


void CommonOutputJax::bindNodeMap(NodeMapFactory factory,NodeMapPublisher publisher){
    if(!factory||!publisher)
        throw Exception("TypeError",u"CommonOutputJax requires the canonical Source Map owner and publisher");
    nodeMapFactory_=std::move(factory);
    nodeMapPublisher_=std::move(publisher);
}

void CommonOutputJax::clearPublishedNodeMap(){
    if(nodeMapPublished_&&nodeMapPublisher_){
        nodeMapPublisher_(options::Value(options::Null{}));
        nodeMapPublished_=false;
    }
    nodeMap.reset();
}

void CommonOutputJax::finishLayoutJob(){
    completedJob_=activeJob_;
    activeJob_.reset();
    clearPublishedNodeMap();
    table=nullptr;
}
std::shared_ptr<LayoutJob> CommonOutputJax::takeCurrentLayoutJobForPrivateRendererRetirement()noexcept{
    auto result=std::exchange(activeJob_,std::shared_ptr<LayoutJob>{});
    if(!result)result=completedJob_.lock();
    completedJob_.reset();
    try{clearPublishedNodeMap();}
    catch(...){nodeMap.reset();nodeMapPublished_=false;}
    table=nullptr;
    math=nullptr;
    document=Missing{};
    container={};
    return result;
}

CommonWrapper* CommonOutputJax::adoptWrapper(std::unique_ptr<CommonWrapper> wrapper){
    if(!activeJob_)
        throw Exception("TypeError",u"Cannot read properties of null (reading 'adopt')");
    return activeJob_->adopt(std::move(wrapper));
}

ExtendedMetrics CommonOutputJax::currentMetrics()const{
    if(!math)throw Exception("TypeError",u"Cannot read properties of null (reading 'metrics')");
    const auto& value=math->metrics;
    return {metricNumber(value,"em"),metricNumber(value,"ex"),
            metricNumber(value,"containerWidth"),metricNumber(value,"scale"),
            property_host::get(value,"family")};
}

LinebreakOptions CommonOutputJax::currentLinebreakOptions()const{
    auto& context=const_cast<CommonOutputJax*>(this)->optionsContext();
    const Value source=context.get(options,Key(u"linebreaks"));
    const Value inlineValue=context.get(source,Key(u"inline"));
    const Value widthValue=context.get(source,Key(u"width"));
    const Value leadingValue=context.get(source,Key(u"lineleading"));
    LinebreakOptions result;
    if(const auto* value=std::get_if<bool>(&inlineValue.data))result.inlineBreaks=*value;
    else if(std::holds_alternative<Null>(inlineValue.data)||std::holds_alternative<Undefined>(inlineValue.data))result.inlineBreaks=false;
    else result.inlineBreaks=true;
    if(const auto* value=std::get_if<std::u16string>(&widthValue.data))
        result.width=property_host::debugText(*value);
    else if(const auto* value=std::get_if<double>(&widthValue.data))result.width=js_number::toString(*value);
    else if(const auto* value=std::get_if<bool>(&widthValue.data))result.width=*value?"true":"false";
    else if(std::holds_alternative<Null>(widthValue.data))result.width="null";
    else result.width="undefined";
    if(const auto* value=std::get_if<double>(&leadingValue.data))result.lineleading=*value;
    else if(const auto* value=std::get_if<bool>(&leadingValue.data))result.lineleading=*value?1:0;
    else if(const auto* value=std::get_if<std::u16string>(&leadingValue.data))
        result.lineleading=property_host::number(Property(property_host::debugText(*value)));
    else result.lineleading=std::numeric_limits<double>::quiet_NaN();
    return result;
}

double CommonOutputJax::getLinebreakWidth(){
    if(!math)throw Exception("TypeError",u"Cannot read properties of null (reading 'metrics')");
    const auto width=metricNumber(math->metrics,"containerWidth");
    const double relative=width/pxPerEm;
    auto* root=math->root.requireNode("attributes");
    const auto attributes=root->attributes();
    if(!attributes)throw Exception("TypeError",u"Cannot read properties of null (reading 'get')");
    const Property maximum=attributes->get("maxwidth");
    const Property selected=property_host::truthy(maximum)
        ?maximum:Property(currentLinebreakOptions().width);
    const auto selectedText=property_host::debugText(property_host::string(selected));
    containerWidth=property_host::number(
        lengths::length2em(std::string_view(selectedText),relative,1,pxPerEm));
    return containerWidth;
}

std::string CommonOutputJax::fixed(double value,std::uint32_t digits)const{
    if(std::abs(value)<0.0006)return "0";
    return trimFixed(js_number::toFixed(value,digits));
}

std::string CommonOutputJax::sourceFixed(double value,std::optional<std::uint32_t> digits)const{
    Context& context=optionsContext();
    const Value receiver(instanceIdentity());
    const Value method=context.get(receiver,Key(u"fixed"));
    std::vector<Value> arguments{Value(value)};
    if(digits)arguments.emplace_back(static_cast<double>(*digits));
    const Value raw=context.callMember(method,receiver,arguments,u"OutputJax.fixed");
    const Value primitive=context.toPrimitive(raw,PrimitiveHint::stringHint);
    if(const auto* text=std::get_if<std::u16string>(&primitive.data))return utf16::encode(*text);
    if(const auto* number=std::get_if<double>(&primitive.data))return js_number::toString(*number);
    if(const auto* boolean=std::get_if<bool>(&primitive.data))return *boolean?"true":"false";
    if(std::holds_alternative<Null>(primitive.data))return "null";
    if(std::holds_alternative<Undefined>(primitive.data))return "undefined";
    if(std::holds_alternative<SymbolRef>(primitive.data))
        throw Exception("TypeError",u"Cannot convert a Symbol value to a string");
    throw Exception("TypeError",u"OutputJax.fixed result did not become a primitive");
}

UnknownBBox CommonOutputJax::measureText(std::u16string_view textValue,std::string_view variantValue,CssFontData fontValue){
    const auto node=unknownText(textValue,variantValue);
    if(variantValue=="-explicitFont")
        domAdaptor().setAttributes(node,PropertyList{{"style",Property(cssFontStyles(fontValue))}});
    return measureTextNodeWithCache(node,textValue,variantValue,std::move(fontValue));
}

UnknownBBox CommonOutputJax::measureTextNodeWithCache(DOMNodeRef node,std::u16string_view chars,
    std::string_view variantValue,CssFontData fontValue){
    return measureTextWithCache(chars,variantValue,std::move(fontValue),
        [this,node]{return measureTextNode(node);});
}

UnknownBBox CommonOutputJax::measureTextWithCache(std::u16string_view chars,
    std::string_view variantValue,CssFontData fontValue,std::function<UnknownBBox()> measure){
    std::string key(variantValue);
    if(variantValue=="-explicitFont"){
        const auto family=property_host::get(fontValue,"0");
        const bool italic=arrayTruthy(fontValue,1),bold=arrayTruthy(fontValue,2);
        key=utf8(property_host::string(family))+"-"+(italic?"T":"F")+"-"+(bold?"T":"F")+"-";
    }
    auto& variant=unknownCache_[key];
    if(!variant)variant=std::make_shared<UnknownMap>();
    const auto map=variant;
    const auto characterKey=std::u16string(chars);
    if(const auto found=map->find(characterKey);found!=map->end())return found->second;
    if(!measure)throw Exception("TypeError",u"Text measurement callback is unavailable");
    const auto measured=measure();
    // Source captures the inner Map before measuring. A callback may reset
    // the current variant map, but this measurement still writes to the same
    // captured Map object.
    map->insert_or_assign(characterKey,measured);
    return measured;
}

DOMNodeRef CommonOutputJax::getTestElement(DOMNodeRef node,bool display){
    auto& adaptor=domAdaptor();
    if(!testInline){
        PropertyList inlineStyle{{"display",Property("inline-block")},{"width",Property("100%")},
            {"font-style",Property("normal")},{"font-weight",Property("normal")},
            {"font-size",Property("100%")},{"font-size-adjust",Property("none")},
            {"text-indent",Property(0.0)},{"text-transform",Property("none")},
            {"letter-spacing",Property("normal")},{"word-spacing",Property("normal")},
            {"overflow",Property("hidden")},{"height",Property("1px")},
            {"margin-right",Property("-1px")}};
        PropertyList leftStyle{{"display",Property("inline-block")},{"width",Property(0.0)},
            {"float",Property("left")}};
        PropertyList exStyle{{"position",Property("absolute")},{"overflow",Property("hidden")},
            {"width",Property("1px")},{"height",Property("60ex")}};
        PropertyList rightStyle{{"display",Property("inline-block")},{"width",Property(0.0)},
            {"float",Property("right")}};
        const auto left=adaptor.node(u"mjx-left-box",PropertyList{{"style",Property(leftStyle)}});
        const auto ex=adaptor.node(u"mjx-ex-box",PropertyList{{"style",Property(exStyle)}});
        const auto right=adaptor.node(u"mjx-right-box",PropertyList{{"style",Property(rightStyle)}});
        PropertyArray children{nodeProperty(left),nodeProperty(ex),nodeProperty(right)};
        testInline=adaptor.node(u"mjx-test",PropertyList{{"style",Property(inlineStyle)}},std::move(children));

        testDisplay=adaptor.clone(testInline);
        adaptor.setStyle(testDisplay,u"display",u"table");
        adaptor.setStyle(testDisplay,u"margin-right",u"");
        const auto first=adaptor.firstChild(testDisplay);
        if(const auto* ref=std::get_if<DOMNodeRef>(&first))adaptor.setStyle(*ref,u"display",u"none");
        const auto last=adaptor.lastChild(testDisplay);
        if(const auto* ref=std::get_if<DOMNodeRef>(&last)){
            adaptor.setStyle(*ref,u"display",u"table-cell");
            adaptor.setStyle(*ref,u"width",u"10000em");
            adaptor.setStyle(*ref,u"float",u"");
        }
    }
    const auto clone=adaptor.clone(display?testDisplay:testInline);
    return adaptor.append(node,clone);
}

ExtendedMetrics CommonOutputJax::measureMetrics(DOMNodeRef node,bool getFamily){
    auto& adaptor=domAdaptor();
    Property family(std::string{});
    if(getFamily){
        if(runtime_){
            const std::array<Value,1> arguments{sourceDOMValue(node)};
            family=runtime_->properties.from(sourceAdaptorCall(u"fontFamily",arguments));
        }else family=Property(utf8(adaptor.fontFamily(node)));
    }
    const double em=adaptor.fontSize(node);
    const auto exbox=adaptor.childNode(node,1);
    double width=0,height=0;
    if(const auto* child=std::get_if<DOMNodeRef>(&exbox)){
        const auto size=adaptor.nodeSize(*child);
        width=size.first;height=size.second;
    }
    auto& cx=optionsContext();
    const double ex=width?height/60:em*cx.toNumber(cx.get(options,Key(u"exFactor")));
    double containerWidth;
    if(!width)containerWidth=1000000;
    else if(adaptor.getStyle(node,u"display")==u"table")
        containerWidth=adaptor.nodeSize(std::get<DOMNodeRef>(adaptor.lastChild(node))).first-1;
    else{
        const auto right=std::get<DOMNodeRef>(adaptor.lastChild(node));
        const auto left=std::get<DOMNodeRef>(adaptor.firstChild(node));
        containerWidth=adaptor.nodeBBox(right).left-adaptor.nodeBBox(left).left-2;
    }
    const double minimum=cx.toNumber(cx.get(options,Key(u"minScale")));
    const bool match=options::truthy(cx.get(options,Key(u"matchFontHeight")));
    const double fit=match?ex/font->params().x_height/em:1.0;
    const double scale=(std::isnan(minimum)||std::isnan(fit))
        ?std::numeric_limits<double>::quiet_NaN():std::max(minimum,fit);
    return {em,ex,containerWidth,scale,family};
}

ExtendedMetrics CommonOutputJax::getMetricsFor(DOMNodeRef node,bool display){
    auto& context=optionsContext();
    const auto inheritText=context.get(options,Key(u"mtextInheritFont"));
    const auto inheritError=options::truthy(inheritText)
        ?Value(true):context.get(options,Key(u"merrorInheritFont"));
    const bool getFamily=options::truthy(inheritText)||options::truthy(inheritError);
    const auto test=getTestElement(node,display);
    auto metrics=measureMetrics(test,getFamily);
    metrics.display=display;
    domAdaptor().remove(test);
    return metrics;
}

PropertyList CommonOutputJax::cssFontStyles(const CssFontData& fontValue,PropertyList styles)const{
    const auto name=font->getFamily(utf8(property_host::string(property_host::get(fontValue,"0"))));
    styles.set("font-family",Property(name));
    if(arrayTruthy(fontValue,1))styles.set("font-style",Property("italic"));
    if(arrayTruthy(fontValue,2))styles.set("font-weight",Property("bold"));
    return styles;
}

CssFontData CommonOutputJax::getFontData(styles::Styles& sourceStyles){
    auto family=sourceStyles.get(u"font-family");
    auto style=sourceStyles.get(u"font-style");
    auto weight=sourceStyles.get(u"font-weight");
    return PropertyArray{Property(font->getFamily(utf8(family))),
                         Property(style==u"italic"),Property(weight==u"bold")};
}

void CommonOutputJax::addFontStyles(style_json::StyleJsonSheet& target){
    target.addStyles(font->styles());
}

void CommonOutputJax::addWrapperStyles(style_json::StyleJsonSheet& target){
    for(const auto& kind:factory->getKinds())addClassStyles(factory->getWrapperClass(kind),target);
}

void CommonOutputJax::addClassStyles(const CommonWrapperClassRef& type,style_json::StyleJsonSheet& target){
    if(!type)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'addStyles')");
    const auto* data=std::any_cast<CommonWrapperClassData>(&type->staticData);
    if(!data)throw Exception("Unimplemented",u"Common wrapper class has no typed Source static data binding");
    if(!runtime_||!data->sourceConstructor)
        throw Exception("Unimplemented",u"Common wrapper class has no actual Source constructor binding");
    auto& context=runtime_->context();
    const auto styleBindings=runtime_->modules.find<StyleJsonContextBindings>(
        u"@mathjax/src/ts/util/StyleJson");
    if(!styleBindings)
        throw Exception("Unimplemented",u"The actual StyleJsonSheet Context identity is unavailable");
    const auto styleObject=styleBindings->bind(styleJson);
    const Value sourceConstructor(*data->sourceConstructor);
    const Value addStyles=context.get(sourceConstructor,Key(u"addStyles"));
    const std::array<Value,2> arguments{Value(styleObject),Value(instanceIdentity())};
    (void)context.callMember(addStyles,sourceConstructor,arguments,u"SvgWrapper.addStyles");
}

void CommonOutputJax::insertStyles(const style_json::StyleJson&){ }

void CommonOutputJax::initializeCommon(Value supplied,ObjectRef defaultFactory,
    ObjectRef defaultFont,CommonRuntimeBindings& runtime,std::shared_ptr<void> owner){
    runtime_=&runtime;
    auto& context=runtime.context();
    if(&optionsContext()!=&context)
        throw Exception("TypeError",u"Common output owner belongs to a different Context");
    if(std::holds_alternative<Undefined>(supplied.data))supplied=Value(context.object());

    // Source evaluates fontData/instanceof/constructor/OPTIONS and separates
    // font options before calling AbstractOutputJax's constructor.
    const Value fontData=argumentOrUndefined(context,supplied,u"fontData");
    const bool suppliedFont=runtime.fonts.isFontData(fontData);
    Value fontClassValue;
    std::shared_ptr<FontData> existingFont;
    if(suppliedFont){
        fontClassValue=context.get(fontData,Key(u"constructor"));
        const auto object=std::get<ObjectRef>(fontData.data);
        existingFont=runtime.fonts.resolve(object);
    }else{
        fontClassValue=truthy(fontData)?fontData:Value(defaultFont);
    }
    const Value fontClassOptions=context.get(fontClassValue,Key(u"OPTIONS"));
    const std::array<Value,1> selectors{fontClassOptions};
    auto separated=options::separateOptions(context,supplied,selectors);
    if(separated.size()!=2)throw Exception("Unimplemented",u"Common option separation did not return both source records");

    // This is the native equivalent of `super(jaxOptions)`. It is intentionally
    // sequenced after separateOptions and before factory/style/font setup.
    // Keep the actual Jax owner for Source instance-field bindings.  The
    // Context object and the native Jax are two views of one instance; the
    // `font` slot below resolves through FontDataContextBindings rather than
    // manufacturing a parallel plain object.
    const auto sourceFieldOwner=owner;
    initializeOutputJax(std::move(separated[0]),std::move(owner));
    const Value selectedFactory=argumentOrUndefined(context,options,u"wrapperFactory");
    ObjectRef factoryObject;
    if(truthy(selectedFactory)){
        const auto* object=std::get_if<ObjectRef>(&selectedFactory.data);
        if(!object)throw Exception("TypeError",u"options.wrapperFactory is not an object");
        factoryObject=*object;
        factory=runtime.factories.resolve(*object);
    }else{
        const std::array<Value,0> noArguments{};
        factoryObject=context.construct(defaultFactory,noArguments);
        factory=runtime.factories.resolve(factoryObject);
    }
    // Source assigns CommonOutputJax.factory before assigning
    // CommonWrapperFactory.jax. Keep the field as a live view of that exact
    // typed owner, using the one canonical factory identity registry rather
    // than another stored Context value.
    {
        auto* self=this;
        auto sourceFactory=std::make_shared<NativeDataField>(sourceFieldOwner,
            [self]()->Value{
                if(!self->runtime_||!self->factory)
                    throw Exception("TypeError",u"Cannot read properties of undefined (reading 'factory')");
                return Value(self->runtime_->factories.bind(self->factory));
            },
            [self](Value value){
                if(!self->runtime_)
                    throw Exception("Unimplemented",u"CommonOutputJax.factory has no CommonFactoryContextBindings");
                const auto* identity=std::get_if<ObjectRef>(&value.data);
                if(!identity)
                    throw Exception("TypeError",u"CommonOutputJax.factory must be a canonical CommonWrapperFactory object");
                self->factory=self->runtime_->factories.resolve(*identity);
            });
        context.bindData(instanceIdentity(),Key(u"factory"),std::move(sourceFactory));
    }
    factory->jaxValue=Value(instanceIdentity());
    runtime.factories.bindWrapperFactory(*factory);

    const Value selectedStyles=argumentOrUndefined(context,options,u"styleJson");
    auto styleBindings=runtime.modules.getOrCreate<StyleJsonContextBindings>(
        u"@mathjax/src/ts/util/StyleJson",[&]{
            return std::make_shared<StyleJsonContextBindings>(context,runtime.properties);
        });
    (void)styleBindings->constructor();
    if(truthy(selectedStyles))styleJson=styleSheetOwner(*styleBindings,selectedStyles);
    else styleJson=styleBindings->construct();

    if(existingFont)font=std::move(existingFont);
    else {
        const auto* fontConstructor=std::get_if<ObjectRef>(&fontClassValue.data);
        if(!fontConstructor)throw Exception("TypeError",u"FontData class is not a constructor");
        const std::array<Value,1> fontArguments{separated[1]};
        const auto instance=context.construct(*fontConstructor,fontArguments);
        font=runtime.fonts.resolve(instance);
    }
    const auto fontIdentity=runtime.fonts.identity(font.get());
    if(!fontIdentity)
        throw Exception("Unimplemented",u"CommonOutputJax font has no canonical Context identity");
    sourceFontValue_=Value(*fontIdentity);
    {
        auto* self=this;
        auto sourceFont=std::make_shared<NativeDataField>(sourceFieldOwner,
            [self]()->Value{return self->sourceFontValue_;},
            [self](Value value){
                if(!self->runtime_)
                    throw Exception("Unimplemented",u"CommonOutputJax.font has no FontData Context binding");
                self->sourceFontValue_=std::move(value);
                if(self->runtime_->fonts.isFontData(self->sourceFontValue_)){
                    const auto identity=std::get<ObjectRef>(self->sourceFontValue_.data);
                    self->font=self->runtime_->fonts.resolve(identity);
                }else{
                    // Preserve arbitrary Source assignments. Typed consumers
                    // must resolve the current Source value when used and may
                    // not silently continue with the previous font owner.
                    self->font.reset();
                }
            });
        context.bindData(instanceIdentity(),Key(u"font"),std::move(sourceFont));
    }
    const ObjectRef spacing=context.object();
    context.set(Value(spacing),Key(u"mathmlSpacing"),argumentOrUndefined(context,options,u"mathmlSpacing"));
    font->setOptions(Value(spacing));

    const Value actualConstructor=context.get(Value(instanceIdentity()),Key(u"constructor"));
    context.set(actualConstructor,Key(u"genericFont"),fontClassValue);
    unknownCache_.clear();

    const Value linebreakOptions=argumentOrUndefined(context,options,u"linebreaks");
    const Value requestedVisitor=context.get(linebreakOptions,Key(u"LinebreakVisitor"));
    auto visitorBindings=runtime.modules.getOrCreate<LinebreakVisitorContextBindings>(
        u"@mathjax/src/ts/output/common/LinebreakVisitor",[&]{
            return std::make_shared<LinebreakVisitorContextBindings>(context,runtime.factories);
        });
    const auto defaultVisitor=visitorBindings->registerConstructor();
    ObjectRef visitorConstructor=defaultVisitor;
    if(truthy(requestedVisitor)){
        const auto* requested=std::get_if<ObjectRef>(&requestedVisitor.data);
        if(!requested)throw Exception("TypeError",u"LinebreakVisitor option is not a constructor");
        visitorConstructor=*requested;
    }
    const std::array<Value,1> visitorArguments{Value(factoryObject)};
    auto [visitor,visitorObject]=visitorBindings->construct(visitorConstructor,visitorArguments);
    linebreaks=std::move(visitor);
    const auto* nativeOwner=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(instanceIdentity()));
    if(!nativeOwner||!*nativeOwner)
        throw Exception("Unimplemented",u"Common output Jax has no retained native owner for its linebreak field");
    auto ownerPin=std::static_pointer_cast<void>(*nativeOwner);
    auto field=std::make_shared<NativeDataField>(ownerPin,
        [this,visitorBindings]()->Value{
            if(!linebreaks)return Null{};
            const auto identity=visitorBindings->objectFor(linebreaks.get());
            if(!identity)throw Exception("Unimplemented",u"Current linebreak service has no canonical Context identity");
            return *identity;
        },
        [this,visitorBindings](Value value){
            if(std::holds_alternative<Null>(value.data)||std::holds_alternative<Undefined>(value.data)){
                linebreaks.reset();
                return;
            }
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"linebreaks is not a Linebreaks instance");
            linebreaks=visitorBindings->resolve(*object);
        });
    context.bindData(instanceIdentity(),Key(u"linebreaks"),std::move(field));
    (void)visitorObject;
}

void CommonOutputJax::setAdaptor(core::OutputAdaptorReference reference){
    core::AbstractOutputJax::setAdaptor(std::move(reference));
    const Value* value=std::get_if<Value>(&adaptor);
    if(!value){
        if(std::holds_alternative<Missing>(adaptor))throw Exception("TypeError",u"Cannot read properties of undefined (reading 'canMeasureNodes')");
        if(std::holds_alternative<PropertyNull>(adaptor))throw Exception("TypeError",u"Cannot read properties of null (reading 'canMeasureNodes')");
        throw Exception("Unimplemented",u"Native DOM adaptor requires a canonical Context identity");
    }
    const auto* object=std::get_if<ObjectRef>(&value->data);
    if(!object)throw Exception("TypeError",u"DOM adaptor is not an object");
    auto& context=optionsContext();
    context.kind(*object);
    const auto* payload=std::any_cast<std::shared_ptr<AbstractDOMAdaptor>>(&context.payload(*object));
    if(!payload||!*payload)throw Exception("Unimplemented",u"DOM adaptor identity has no retained actual adaptor owner");
    domAdaptorOwner_=*payload;
    domAdaptor_=domAdaptorOwner_.get();
    if(!htmlAdaptorOwner_||htmlAdaptorOwner_->domAdaptor()!=domAdaptorOwner_)
        htmlAdaptorOwner_=mml::XmlDomAdaptor::create(domAdaptorOwner_);
    const Value htmlHDW=context.get(options,Key(u"htmlHDW"));
    if(const auto* value=std::get_if<std::u16string>(&htmlHDW.data);value&&*value==u"auto")
        context.set(options,Key(u"htmlHDW"),property_host::truthy((*payload)->canMeasureNodes)?Value(u"ignore"):Value(u"force"));
}

void CommonOutputJax::setDocument(core::MathDocumentReference reference){
    if(const auto* documentValue=std::get_if<core::MathDocumentService*>(&reference);
       documentValue&&*documentValue){
        document=*documentValue;
        domAdaptor().document=(**documentValue).documentNode();
    }
}

DOMNodeRef CommonOutputJax::createNode(){
    auto& context=optionsContext();
    const auto constructor=context.get(Value(instanceIdentity()),Key(u"constructor"));
    const auto name=context.get(constructor,Key(u"NAME"));
    PropertyList attributes{{"class",Property("MathJax")}};
    if(runtime_)attributes.set("jax",runtime_->properties.from(name));
    else throw Exception("Unimplemented",u"Common output runtime bindings are unavailable");
    return html(u"mjx-container",std::move(attributes));
}

void CommonOutputJax::toDOM(core::MathItem& item,DOMNodeRef node,
                            core::MathDocumentReference documentValue){
    setDocument(documentValue);
    math=&item;
    container=node;
    pxPerEm=item.metrics.missing()?std::numeric_limits<double>::quiet_NaN():
        metricNumber(item.metrics,"ex")/font->params().x_height;
    auto filterData=[&](FunctionList& filters)->DOMNodeRef{
        if(!runtime_)throw Exception("Unimplemented",u"Common output runtime bindings are unavailable");
        const Value data(runtime_->properties.to(nodeProperty(node)));
        const auto result=core::AbstractOutputJax::executeFilters(
            filters,core::OutputMathItemReference(&item),documentValue,std::any(data));
        const auto* value=std::any_cast<Value>(&result);
        if(!value)throw Exception("Unimplemented",u"Output filter returned data outside the Context value profile");
        auto projected=propertyNode(runtime_->properties.from(*value));
        if(const auto* ref=std::get_if<DOMNodeRef>(&projected))return *ref;
        throw Exception("TypeError",u"Output filter data is not a DOM node");
    };
    if(preFilters)(void)filterData(*preFilters);

    const bool createdJob=!activeJob_;
    if(createdJob)beginLayoutJob(item,documentValue,node);
    if(activeJob_&&activeJob_->nodeMap){activeJob_->nodeMap->clear();nodeMap=activeJob_->nodeMap;}
    else throw Exception("TypeError",u"Cannot set properties of null (setting 'nodeMap')");

    auto* root=item.root.requireNode("attributes");
    auto attributes=root->attributes();
    if(!attributes)throw Exception("TypeError",u"Cannot read properties of null (reading 'getAllInherited')");
    const Value displayOverflow=optionsContext().get(options,Key(u"displayOverflow"));
    attributes->getAllInherited().set("overflow",runtime_->properties.from(displayOverflow));
    const auto overflow=attributes->getAllInherited().get("overflow");
    domAdaptor().setAttribute(node,u"overflow",overflow);
    if(overflow==Property("linebreak"))getLinebreakWidth();

    const Value linebreaks=optionsContext().get(options,Key(u"linebreaks"));
    const Value inlineOption=optionsContext().get(linebreaks,Key(u"inline"));
    const bool makeBreaks=truthy(inlineOption)&&!displayMath(item);
    bool inlineMarked=property_host::truthy(root->getProperty("inlineMarked"));
    if(inlineMarked&&(!makeBreaks||Property(forceInlineBreaks())!=root->getProperty("inlineForced"))){
        unmarkInlineBreaks(mml::NodeValue(root));
        root->removeProperty(std::array<std::string,2>{"inlineMarked","inlineForced"});
        inlineMarked=false;
    }
    if(makeBreaks&&!inlineMarked){
        const auto children=root->childArrayOwner();
        if(children&&!children->empty())markInlineBreaks(children->get(0));
        root->setProperty("inlineMarked",true);
        root->setProperty("inlineForced",forceInlineBreaks());
    }
    (void)root->setTeXclass(PropertyNull{});
    auto* wrapper=factory->wrap(root);
    if(!wrapper)throw Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
    setScale(node,*wrapper);
    processMathObserved(*wrapper,node);
    clearPublishedNodeMap();
    if(postFilters)(void)filterData(*postFilters);
    if(createdJob)finishLayoutJob();
}

core::MathOutput CommonOutputJax::typeset(core::MathItem& item,
                                          core::MathDocumentReference documentValue){
    auto& context=optionsContext();
    if(!runtime_)throw Exception("Unimplemented",u"Common output runtime bindings are unavailable");
    const auto constructor=context.get(Value(instanceIdentity()),Key(u"constructor"));
    const auto generic=context.get(constructor,Key(u"genericFont"));
    const auto fontIdentity=runtime_->fonts.identity(font.get());
    if(!fontIdentity)throw Exception("Unimplemented",u"FontData instance has no canonical Context identity");
    const auto fontConstructor=context.get(Value(*fontIdentity),Key(u"constructor"));
    context.set(constructor,Key(u"genericFont"),fontConstructor);
    setDocument(documentValue);
    const auto node=createNode();
    try{toDOM(item,node,documentValue);}
    catch(...){context.set(constructor,Key(u"genericFont"),generic);throw;}
    context.set(constructor,Key(u"genericFont"),generic);
    return node;
}

std::shared_ptr<BBox> CommonOutputJax::getBBox(core::MathItem& item,core::MathDocumentReference documentValue){
    setDocument(documentValue);
    math=&item;
    auto* root=item.root.requireNode("setTeXclass");
    (void)root->setTeXclass(PropertyNull{});
    const bool createdJob=!activeJob_;
    if(createdJob)beginLayoutJob(item,documentValue,DOMNodeRef{});
    if(!activeJob_||!activeJob_->nodeMap)
        throw Exception("TypeError",u"Cannot set properties of null (setting 'nodeMap')");
    activeJob_->nodeMap->clear();
    nodeMap=activeJob_->nodeMap;
    auto* wrapper=factory->wrap(root);
    if(!wrapper)throw Exception("TypeError",u"Cannot read properties of null (reading 'getOuterBBox')");
    const auto bbox=wrapper->getOuterBBox();
    clearPublishedNodeMap();
    if(createdJob)finishLayoutJob();
    return bbox;
}

std::vector<std::vector<std::pair<DOMNodeRef,ExtendedMetrics>>>
CommonOutputJax::getMetricMaps(core::MathDocumentReference reference){
    auto& source=requireDocument(reference,"math");
    auto& adaptor=domAdaptor();
    std::vector<std::vector<std::pair<DOMNodeRef,DOMNodeRef>>> domMaps(2);
    source.mathList().forEach([&](core::MathItem& item){
        const auto parent=parentOfMath(item,source,adaptor);
        if(!parent||!belowMetrics(item))return;
        auto& map=domMaps[displayMath(item)?1:0];
        if(std::find_if(map.begin(),map.end(),[&](const auto& entry){return entry.first==*parent;})==map.end())
            map.emplace_back(*parent,getTestElement(*parent,displayMath(item)));
    });
    const bool getFamily=truthy(optionsContext().get(options,Key(u"mtextInheritFont")))||
        truthy(optionsContext().get(options,Key(u"merrorInheritFont")));
    std::vector<std::vector<std::pair<DOMNodeRef,ExtendedMetrics>>> maps(2);
    for(std::size_t i=0;i<domMaps.size();++i)
        for(const auto& [parent,test]:domMaps[i])
            maps[i].emplace_back(parent,measureMetrics(test,getFamily));
    for(const auto& group:domMaps)
        for(const auto& [parent,test]:group){(void)parent;adaptor.remove(test);}
    return maps;
}

void CommonOutputJax::getMetrics(core::MathDocumentReference reference){
    setDocument(reference);
    auto& source=requireDocument(reference,"math");
    auto maps=getMetricMaps(reference);
    auto& adaptor=domAdaptor();
    source.mathList().forEach([&](core::MathItem& item){
        const auto parent=parentOfMath(item,source,adaptor);
        if(!belowMetrics(item)||!parent)return;
        const auto& map=maps[displayMath(item)?1:0];
        const auto found=std::find_if(map.begin(),map.end(),[&](const auto& entry){return entry.first==*parent;});
        if(found==map.end())throw Exception("TypeError",u"Cannot destructure metrics for an unmeasured math container");
        const auto& value=found->second;
        item.setMetrics(Property(value.em),Property(value.ex),Property(value.containerWidth),Property(value.scale));
        if(truthy(optionsContext().get(options,Key(u"mtextInheritFont")))){
            auto record=PropertyList::record(item.outputData);
            if(!record)property_host::typeError("Cannot set properties of undefined (setting 'mtextFamily')");
            record->set("mtextFamily",value.family);
        }
        if(truthy(optionsContext().get(options,Key(u"merrorInheritFont")))){
            auto record=PropertyList::record(item.outputData);
            if(!record)property_host::typeError("Cannot set properties of undefined (setting 'merrorFamily')");
            record->set("merrorFamily",value.family);
        }
        item.state(core::STATE.get("METRICS"));
    });
}

core::MathOutput CommonOutputJax::styleSheet(core::MathDocumentReference reference){
    setDocument(reference);
    if(!runtime_)throw Exception("Unimplemented",u"Common output runtime bindings are unavailable");
    styleJson->clear();
    auto& context=runtime_->context();
    const auto constructor=context.get(Value(instanceIdentity()),Key(u"constructor"));
    styleJson->addStyles(runtime_->properties.from(context.get(constructor,Key(u"commonStyles"))));
    auto& source=requireDocument(reference,"getStyles");
    const auto documentIdentity=Value(source.sourceIdentity());
    if(context.has(documentIdentity,Key(u"getStyles"))){
        const auto getter=context.get(documentIdentity,Key(u"getStyles"));
        const auto styles=context.callMember(getter,documentIdentity,{},u"MathDocument.getStyles");
        context.forEachIterable(styles,[&](Value item){styleJson->addStyles(runtime_->properties.from(item));});
    }
    addWrapperStyles(*styleJson);
    addFontStyles(*styleJson);
    const auto css=styleJson->cssText();
    std::u16string textValue; textValue.reserve(css.size()+2);textValue.push_back(u'\n');textValue+=css;textValue.push_back(u'\n');
    const auto textNode=text(textValue);
    const auto sheet=html(u"style",PropertyList{{"id",Property("MJX-styles")}},PropertyArray{nodeProperty(textNode)});
    return sheet;
}

DOMNodeRef CommonOutputJax::html(std::u16string_view type,PropertyList def,PropertyArray content,std::u16string_view ns){
    return domAdaptor().node(type,std::move(def),std::move(content),ns);
}
DOMNodeRef CommonOutputJax::text(std::u16string_view value){return domAdaptor().text(value);}
}
