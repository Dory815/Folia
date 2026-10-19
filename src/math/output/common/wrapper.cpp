/*************************************************************
 * MathJax 4.1.3 ts/output/common/Wrapper.ts.
 * Common wrapper measurement and MML association.
 *************************************************************/
module folia.math.output.common;
import folia.math.cancellation;
import :wrapper;
import :wrapper_factory;
import :output_jax;
import :mo;
import :mrow;
import folia.math.util.lengths;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.lengths;
import folia.math.util.styles;
import folia.math.core.mml_tree.mml_nodes.mo;
import std;

namespace folia::math::output::common {
WrapperNodeMap::WrapperNodeMap(options::Context& context,
    options::MapContextBindings& maps,NodeIdentity nodeIdentity,
    WrapperIdentity wrapperIdentity,WrapperResolver wrapperResolver)
{
    initialize(context,maps,std::move(nodeIdentity),std::move(wrapperIdentity),
               std::move(wrapperResolver));
}
void WrapperNodeMap::initialize(options::Context& context,
    options::MapContextBindings& maps,NodeIdentity nodeIdentity,
    WrapperIdentity wrapperIdentity,WrapperResolver wrapperResolver){
    if(&maps.context()!=&context)
        throw options::Exception("TypeError",u"Wrapper nodeMap requires one Context and canonical identity resolvers");
    if(storage_)throw options::Exception("TypeError",u"Wrapper nodeMap is already initialized");
    if(!nodeIdentity||!wrapperIdentity||!wrapperResolver)
        throw options::Exception("TypeError",u"Wrapper nodeMap requires canonical identity resolvers");
    context_=&context;
    storage_=maps.createStorage();
    object_=maps.bind(storage_);
    nodeIdentity_=std::move(nodeIdentity);
    wrapperIdentity_=std::move(wrapperIdentity);
    wrapperResolver_=std::move(wrapperResolver);
}
options::ObjectRef WrapperNodeMap::contextObject()const{
    if(!storage_)throw options::Exception("Unimplemented",u"Wrapper nodeMap has no Context Map identity");
    return object_;
}
void WrapperNodeMap::insert_or_assign(mml::MmlNode* node,CommonWrapper* wrapper){
    if(!storage_)throw options::Exception("Unimplemented",u"Wrapper nodeMap has no canonical Map storage");
    if(!node||!wrapper)throw options::Exception("TypeError",u"Wrapper nodeMap cannot store a null identity");
    const auto key=nodeIdentity_(*node),value=wrapperIdentity_(*wrapper);
    (void)context_->kind(key);(void)context_->kind(value);
    storage_->set(options::Value(key),options::Value(value));
}
void WrapperNodeMap::clear(){if(storage_)storage_->clear();}
CommonWrapper* WrapperNodeMap::find(mml::MmlNode* node)const{
    if(!storage_)throw options::Exception("Unimplemented",u"Wrapper nodeMap has no canonical Map storage");
    if(!node)return nullptr;
    const auto key=nodeIdentity_(*node);
    (void)context_->kind(key);
    if(!storage_->has(options::Value(key)))return nullptr;
    const auto value=storage_->get(options::Value(key));
    const auto* object=std::get_if<options::ObjectRef>(&value.data);
    if(!object)throw options::Exception("TypeError",u"Wrapper nodeMap value is not a wrapper object");
    return &wrapperResolver_(*object);
}
void WrapperNodeMap::forEachValue(const std::function<void(CommonWrapper&)>& body)const{
    if(!storage_)throw options::Exception("Unimplemented",u"Wrapper nodeMap has no canonical Map storage");
    if(!body)throw options::Exception("TypeError",u"Wrapper nodeMap value callback is not callable");
    // Map.values() is a live iterator. Re-read the current storage length and
    // slot after each callback, so appends/deletes have ordinary Map order.
    for(std::size_t index=0;index<storage_->entries().size();++index){
        const auto& entry=storage_->entries()[index];
        if(!entry.live)continue;
        const auto* object=std::get_if<options::ObjectRef>(&entry.value.data);
        if(!object)throw options::Exception("TypeError",u"Wrapper nodeMap value is not a wrapper object");
        body(wrapperResolver_(*object));
    }
}

namespace {
std::string utf8(std::u16string_view text) {
    std::string out;
    for (std::size_t i=0;i<text.size();++i) {
        std::uint32_t c=text[i];
        if (c>=0xD800&&c<=0xDBFF&&i+1<text.size()&&text[i+1]>=0xDC00&&text[i+1]<=0xDFFF)
            c=0x10000+((c-0xD800)<<10)+(text[++i]-0xDC00);
        if(c<0x80)out.push_back(static_cast<char>(c));
        else if(c<0x800){out.push_back(static_cast<char>(0xC0|(c>>6)));out.push_back(static_cast<char>(0x80|(c&0x3F)));}
        else if(c<0x10000){out.push_back(static_cast<char>(0xE0|(c>>12)));out.push_back(static_cast<char>(0x80|((c>>6)&0x3F)));out.push_back(static_cast<char>(0x80|(c&0x3F)));}
        else {out.push_back(static_cast<char>(0xF0|(c>>18)));out.push_back(static_cast<char>(0x80|((c>>12)&0x3F)));out.push_back(static_cast<char>(0x80|((c>>6)&0x3F)));out.push_back(static_cast<char>(0x80|(c&0x3F)));}
    }
    return out;
}
std::u16string utf16(std::string_view text) {
    std::u16string out;
    for(std::size_t i=0;i<text.size();){
        const auto c=static_cast<unsigned char>(text[i++]);
        std::uint32_t value=c;
        unsigned extra=0;
        if((c&0xE0)==0xC0){value=c&0x1F;extra=1;}
        else if((c&0xF0)==0xE0){value=c&0x0F;extra=2;}
        else if((c&0xF8)==0xF0){value=c&0x07;extra=3;}
        for(unsigned j=0;j<extra&&i<text.size();++j)value=(value<<6)|(static_cast<unsigned char>(text[i++])&0x3F);
        if(value<=0xFFFF)out.push_back(static_cast<char16_t>(value));
        else {value-=0x10000;out.push_back(static_cast<char16_t>(0xD800+(value>>10)));out.push_back(static_cast<char16_t>(0xDC00+(value&0x3FF)));}
    }
    return out;
}
std::u16string textOf(const Property& value) { return property_host::string(value); }
std::string stringOf(const Property& value) { return utf8(textOf(value)); }
double numberOf(const Property& value) { return property_host::number(value); }
std::string optionString(const options::Value& value) {
    if(std::holds_alternative<options::Undefined>(value.data))return "undefined";
    if(std::holds_alternative<options::Null>(value.data))return "null";
    if(const auto* p=std::get_if<bool>(&value.data))return *p?"true":"false";
    if(const auto* p=std::get_if<double>(&value.data))return js_number::toString(*p);
    if(const auto* p=std::get_if<std::u16string>(&value.data))return utf8(*p);
    throw options::Exception("Unimplemented",u"CommonWrapper option string conversion for object values has no native binding");
}
bool jsSpace(char16_t c) {
    return (c>=0x0009&&c<=0x000D)||c==0x0020||c==0x00A0||c==0x1680||
           (c>=0x2000&&c<=0x200A)||c==0x2028||c==0x2029||c==0x202F||
           c==0x205F||c==0x3000||c==0xFEFF;
}
std::u16string_view trimJsSpace(std::u16string_view value) {
    std::size_t first=0,last=value.size();
    while(first<last&&jsSpace(value[first]))++first;
    while(last>first&&jsSpace(value[last-1]))--last;
    return value.substr(first,last-first);
}
bool zeroIndentPattern(std::u16string_view value) {
    value=trimJsSpace(value);
    if(value.empty()||value.front()!=u'0')return false;
    for(char16_t c:value.substr(1))if(c<u'a'||c>u'z')return false;
    return true;
}
std::string keyPart(std::string_view prefix,std::string_view side,std::string_view suffix={}) {
    std::string result(prefix);result.append(side);result.append(suffix);return result;
}
double mathMLSpace(bool script,bool noDictionary,double size) {
    constexpr double small=2.0/18.0,mo=5.0/18.0;
    return noDictionary?(script?small:mo):(script?(size<small?0:small):size);
}
std::string mappedVariant(std::string_view table,std::string_view key,std::string_view value) {
    static const std::map<std::string,std::map<std::string,std::string,std::less<>>,std::less<>> bold={
        {"bold",{{"normal","bold"},{"italic","bold-italic"},{"fraktur","bold-fraktur"},{"script","bold-script"},{"sans-serif","bold-sans-serif"},{"sans-serif-italic","sans-serif-bold-italic"}}},
        {"normal",{{"bold","normal"},{"bold-italic","italic"},{"bold-fraktur","fraktur"},{"bold-script","script"},{"bold-sans-serif","sans-serif"},{"sans-serif-bold-italic","sans-serif-italic"}}}
    };
    static const std::map<std::string,std::map<std::string,std::string,std::less<>>,std::less<>> italic={
        {"italic",{{"normal","italic"},{"bold","bold-italic"},{"sans-serif","sans-serif-italic"},{"bold-sans-serif","sans-serif-bold-italic"}}},
        {"normal",{{"italic","normal"},{"bold-italic","bold"},{"sans-serif-italic","sans-serif"},{"sans-serif-bold-italic","bold-sans-serif"}}}
    };
    const auto& maps=table=="bold"?bold:italic;
    const auto found=maps.find(std::string(key));
    if(found==maps.end())return std::string(value);
    const auto item=found->second.find(std::string(value));
    return item==found->second.end()?std::string(value):item->second;
}
}

CommonWrapper::CommonWrapper(CommonWrapperFactory& factory,mml::MmlNode& source,CommonWrapper* parentValue)
    :Base(factory,&source),commonFactory_(&factory),selectedClass_(factory.selectedClass()),parent(parentValue),font(factory.jax().font) {
}

void CommonWrapper::initializeSource(){
    auto& source=*node;
    bbox=std::make_shared<BBox>(BBox::zero());
    getStyles();
    getStyleData();
    getVariant();
    getScale();
    getSpace();
    auto wrapped=std::make_shared<tree::ChildArray<CommonWrapper>>();
    const auto sourceChildren=source.childArrayOwner();
    if(sourceChildren){
        const std::size_t capturedLength=sourceChildren->size();
        wrapped->resize(capturedLength);
        for(std::size_t i=0;i<capturedLength;++i){
            if(!sourceChildren->has(i))continue;
            const auto child=sourceChildren->get(i);
            // Source Array#map calls this.wrap(child) and then assigns the
            // returned value into the mapped array.  Its wrap() call uses
            // this wrapper as the creator's parent, but does not append a
            // second time to this.childNodes.
            CommonWrapper* result=wrapNode(*child.requireNode("map"),nullptr);
            wrapped->set(i,result);
            if(result->bbox&&result->bbox->pwidth.size()&&
               (property_host::truthy(source.notParent())||source.isKind("math")))bbox->pwidth=std::string(BBox::fullWidth);
        }
    }
    childNodes=std::move(wrapped);
}

void CommonWrapper::runSourceInitialization(){
    if(sourceInitializationPhase_!=SourceInitializationPhase::Uninitialized)
        throw options::Exception("TypeError",u"Common wrapper Source initialization has already started");
    sourceInitializationPhase_=SourceInitializationPhase::Initializing;
    try{
        initializeSource();
        sourceInitializationPhase_=SourceInitializationPhase::Complete;
    }catch(...){
        sourceInitializationPhase_=SourceInitializationPhase::Failed;
        throw;
    }
}

void CommonWrapper::getStyles(){
    const auto value=node->attributes()->getExplicit("style");
    if(!property_host::truthy(value))return;
    const auto* css=std::get_if<std::string>(&value.value);
    if(!css)throw options::Exception("TypeError",u"Style attribute must be a string");
    styles=std::make_unique<styles::Styles>(utf16(*css));
    for(const auto& name:std::array<std::string_view,6>{"fontSize","fontFamily","fontWeight","fontStyle","fontVariant","font"}){
        const auto removed=styles->get(std::u16string(name.begin(),name.end()));
        if(removed.empty())continue;
        if(!removedStyles)removedStyles=std::make_shared<StringMap>();
        (*removedStyles)[std::string(name)]=utf8(removed);
        styles->set(utf16(name),styles::StyleValue(u""));
    }
}

void CommonWrapper::getStyleData(){
    if(!styles)return;
    StyleData data;
    bool hasPadding=false,hasBorder=false,hasMargin=false;
    for(const auto& side:BBox::boxSides){
        const auto i=static_cast<std::size_t>(side.index);
        const auto suffix=side.name;
        const auto borderKey=keyPart("border",suffix);
        const auto widthName=borderKey+"Width";
        const auto width=styles->get(utf16(widthName));
        if(!width.empty()){
            hasBorder=true;
            // A border shorthand populates all four side widths.  Preserve
            // the previously decoded sides instead of replacing the whole
            // optional Border record for each side in BBox::boxSides.
            if(!data.border)data.border.emplace();
            data.border->width[i]=std::max(0.0,length2em(Property(utf8(width)),1));
            const auto styleName=borderKey+"Style",colorName=borderKey+"Color";
            const auto style=styles->get(utf16(styleName));
            const auto color=styles->get(utf16(colorName));
            data.border->style[i]=style.empty()?"solid":utf8(style);
            data.border->color[i]=utf8(color);
        }
        const auto paddingName=keyPart("padding",suffix);
        const auto padding=styles->get(utf16(paddingName));
        if(!padding.empty()){
            hasPadding=true;
            data.padding[i]=std::max(0.0,length2em(Property(utf8(padding)),1));
        }
        const auto marginName=keyPart("margin",suffix);
        const auto margin=styles->get(utf16(marginName));
        if(!margin.empty()){
            hasMargin=true;
            data.margin[i]=length2em(Property(utf8(margin)),1);
        }
    }
    if(hasPadding||hasBorder||hasMargin)styleData=std::move(data);
}

void CommonWrapper::getVariant(){
    if(!node->isToken())return;
    const auto attributes=node->attributes();
    const Property mathvariant=attributes->get("mathvariant");
    std::string selected=stringOf(mathvariant);
    if(attributes->hasExplicit("mathvariant")){
        if(!font||!font->getVariant(selected)){
            if(font)font->sourceConsoleWarn({std::string("Invalid variant: ")+selected});
            selected="normal";
        }
        variant=std::move(selected);
        return;
    }
    const auto values=attributes->getList(std::vector<std::string>{"fontfamily","fontweight","fontstyle"});
    std::string family=stringOf(values.get("fontfamily"));
    std::string weight=stringOf(values.get("fontweight"));
    std::string style=stringOf(values.get("fontstyle"));
    if(removedStyles){
        if(auto i=removedStyles->find("fontFamily");i!=removedStyles->end())family=i->second;
        if(auto i=removedStyles->find("fontWeight");i!=removedStyles->end())weight=i->second;
        if(auto i=removedStyles->find("fontStyle");i!=removedStyles->end())style=i->second;
    }
    if(!family.empty()&&family!="undefined"){
        if(!weight.empty()&&weight!="undefined"){
            bool digits=!weight.empty()&&std::all_of(weight.begin(),weight.end(),[](unsigned char c){return c>='0'&&c<='9';});
            if(digits){int parsed=0;std::from_chars(weight.data(),weight.data()+weight.size(),parsed);weight=parsed>600?"bold":"normal";}
        }
        variant=explicitVariant(family,weight,style);
        return;
    }
    if(property_host::truthy(node->getProperty("variantForm")))selected="-tex-variant";
    const auto& boldKey=(weight.empty()||weight=="undefined")?std::string("undefined"):weight;
    if(boldKey!="undefined")selected=mappedVariant("bold",boldKey,selected);
    const auto& italicKey=(style.empty()||style=="undefined")?std::string("undefined"):style;
    if(italicKey!="undefined")selected=mappedVariant("italic",italicKey,selected);
    variant=std::move(selected);
}

std::string CommonWrapper::explicitVariant(std::string_view family,std::string_view weight,std::string_view style){
    if(!styles)styles=std::make_unique<styles::Styles>();
    styles->set(u"fontFamily",styles::StyleValue(utf16(family)));
    if(!weight.empty())styles->set(u"fontWeight",styles::StyleValue(utf16(weight)));
    if(!style.empty())styles->set(u"fontStyle",styles::StyleValue(utf16(style)));
    return "-explicitFont";
}

void CommonWrapper::getScale(){
    const auto attributes=node->attributes();
    const double parentScale=parent?parent->bbox->scale:1;
    double scale=1;
    const double scriptLevel=std::min(numberOf(attributes->get("scriptlevel")),2.0);
    Property fontSize=attributes->get("fontsize");
    Property mathSize=(node->isToken()||node->isKind("mstyle"))
        ?attributes->get("mathsize"):attributes->getInherited("mathsize");
    if(scriptLevel!=0)scale=std::pow(numberOf(attributes->get("scriptsizemultiplier")),scriptLevel);
    if(removedStyles&&(!property_host::truthy(fontSize))){
        const auto found=removedStyles->find("fontSize");
        if(found!=removedStyles->end())fontSize=Property(found->second);
    }
    if(property_host::truthy(fontSize)&&!attributes->hasExplicit("mathsize"))mathSize=fontSize;
    if(mathSize!=Property("1"))scale*=length2em(mathSize,1,1);
    if(scriptLevel!=0){
        const double minimum=length2em(attributes->get("scriptminsize"),0.4,1);
        if(scale<minimum)scale=minimum;
    }
    bbox->scale=scale;
    bbox->rscale=scale/parentScale;
}

bool CommonWrapper::isTopEmbellished()const{
    if(!node->isEmbellished())return false;
    const auto parentValue=node->getParent();
    return !parentValue.isNode()||!parentValue.requireNode("isEmbellished")->isEmbellished();
}

void CommonWrapper::getSpace(){
    const bool isTop=isTopEmbellished();
    const bool hasSpacing=node->hasSpacingAttributes();
    const auto mathml=jax().optionsContext().get(jax().options,options::Key(u"mathmlSpacing"));
    if(options::truthy(mathml)||hasSpacing){if(isTop)getMathMLSpacing();}
    else getTeXSpacing(isTop,hasSpacing);
}

void CommonWrapper::getTeXSpacing(bool isTop,bool hasSpacing){
    if(!hasSpacing){
        const Property space=node->texSpacing();
        if(property_host::truthy(space))bbox->L=length2em(space);
    }
    if(isTop||hasSpacing){
        auto* mo=node->coreMO().requireNode("attributes");
        const auto attributes=mo->attributes();
        if(attributes->isSet("lspace"))bbox->L=std::max(0.0,length2em(attributes->get("lspace")));
        if(attributes->isSet("rspace"))bbox->R=std::max(0.0,length2em(attributes->get("rspace")));
    }
}

void CommonWrapper::getMathMLSpacing(){
    auto* mo=dynamic_cast<mml::MmlMo*>(node->coreMO().requireNode("getProperty"));
    if(!mo)throw options::Exception("TypeError",u"Core operator is not an MmlMo");
    mml::MmlNode* child=mo->coreParent();
    const auto parentValue=child->getParent();
    if(!parentValue.isNode())return;
    auto* parentNode=parentValue.requireNode("childNodes");
    if(!parentNode->isKind("mrow"))return;
    const auto children=parentNode->childArrayOwner();
    if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'length')");
    if(children->size()==1)return;
    const auto index=parentNode->childIndex(child);
    if(!index)return;
    const auto attrs=mo->attributes();
    const bool script=numberOf(attrs->get("scriptlevel"))>0;
    const bool noDictionary=property_host::truthy(mo->getProperty("noDictDef"));
    bbox->L=attrs->isSet("lspace")?std::max(0.0,length2em(attrs->get("lspace"))):mathMLSpace(script,noDictionary,numberOf(mo->lspace));
    bbox->R=attrs->isSet("rspace")?std::max(0.0,length2em(attrs->get("rspace"))):mathMLSpace(script,noDictionary,numberOf(mo->rspace));
    if(*index==0)return;
    const auto previousChildren=parentNode->childArrayOwner();
    if(!previousChildren)throw options::Exception("TypeError",u"Cannot read properties of null (reading '"+
        property_host::text(std::to_string(*index-1))+u"')");
    const auto previous=previousChildren->get(*index-1);
    if(!previous.isNode()||!previous.requireNode("isEmbellished")->isEmbellished())return;
    auto* previousWrapper=jax().nodeMap->find(previous.nodeOrNull());
    if(!previousWrapper)return;
    if(previousWrapper->getBBox()->R)bbox->L=std::max(0.0,bbox->L-previousWrapper->getBBox()->R);
}

double CommonWrapper::length2em(const Property& value,double size,std::optional<double> scale)const{
    const double currentScale=scale.value_or(bbox?bbox->scale:1.0);
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'params')");
    const double thickness=font->params().rule_thickness;
    const auto lookupKey=[&]()->std::optional<std::string>{
        const auto primitive=property_host::primitive(value,options::PrimitiveHint::stringHint);
        if(std::holds_alternative<options::SymbolRef>(primitive.value))return {};
        return utf8(property_host::string(primitive));
    };
    const auto factor=[](const std::optional<std::string>& key){
        if(key=="medium")return 1.0;
        if(key=="thin")return 2.0/3.0;
        if(key=="thick")return 5.0/3.0;
        return 0.0;
    };
    // Source lookup does Object.hasOwn(table, key), then table[key]. Each
    // operation converts an object key separately; the second key may differ.
    if(factor(lookupKey())){
        const auto selected=factor(lookupKey());
        if(selected)return selected*thickness;
    }
    // util/lengths.ts applies String(length) before matching units. Explicit
    // String(Symbol) is allowed; an object conversion returning Symbol is not.
    const auto text=std::get_if<options::SymbolRef>(&value.value)
        ?utf8(jax().optionsContext().symbolString(std::get<options::SymbolRef>(value.value)))
        :utf8(property_host::string(value));
    return numberOf(lengths::length2em(text,size,currentScale,jax().pxPerEm));
}

std::string CommonWrapper::percent(double value)const{return lengths::percent(value);}
std::string CommonWrapper::em(double value)const{return lengths::em(value);}
std::string CommonWrapper::px(double value,double minimum)const{return lengths::px(value,minimum,metrics().em);}
double CommonWrapper::getAlignX(double width,const BBox& box,std::string_view align)const{
    if(align=="right")return width-(box.w+box.R)*box.rscale;
    if(align=="left")return box.L*box.rscale;
    return (width-box.w*box.rscale)/2;
}
double CommonWrapper::getAlignY(double height,double depth,double h,double d,std::string_view align)const{
    if(align=="top")return height-h;
    if(align=="bottom")return d-depth;
    if(align=="center")return (height-h-(depth-d))/2;
    return 0;
}

CommonWrapperFactory& CommonWrapper::factory()const {
    if(!commonFactory_)throw options::Exception("TypeError",u"CommonWrapper has no factory");
    return *commonFactory_;
}
CommonOutputJax& CommonWrapper::jax()const {
    return factory().jax();
}
DOMNodeRef CommonWrapper::sourceHtml(std::u16string_view type,PropertyList definition,
                                     PropertyArray content){
    return jax().html(type,std::move(definition),std::move(content));
}
AbstractDOMAdaptor& CommonWrapper::adaptor()const{return jax().domAdaptor();}
const ExtendedMetrics& CommonWrapper::metrics()const {
    // The current metrics record is owned by the output jax; this value is
    // refreshed per Source math operation rather than copied onto wrappers.
    static thread_local ExtendedMetrics value;
    value=jax().currentMetrics();
    return value;
}
double CommonWrapper::containerWidth()const{return parent?parent->containerWidth():jax().containerWidth;}
Linebreaks& CommonWrapper::linebreaks()const {
    // Filled by CommonOutputJax after its actual factory is constructed.
    if(!jax().linebreaks)throw options::Exception("TypeError",u"CommonOutputJax linebreak service is not initialized");
    return *jax().linebreaks;
}
options::Value CommonWrapper::linebreakOptions()const {
    return jax().optionsContext().get(jax().options,options::Key(u"linebreaks"));
}
bool CommonWrapper::fixesPWidth()const{return !property_host::truthy(node->notParent())&&!node->isToken();}
std::size_t CommonWrapper::breakCount(){
    if(breakCount_<0){
        if(node->isEmbellished()){
            auto* operatorWrapper=coreMO();
            auto* mo=dynamic_cast<CommonMo*>(operatorWrapper);
            if(!mo)throw options::Exception("TypeError",u"Embellished operator wrapper has no embellishedBreakCount");
            breakCount_=static_cast<std::int64_t>(mo->embellishedBreakCount());
        }
        else if(node->arity()<0&&!node->linebreakContainer()&&childNodes&&*childNodes&&(*childNodes)->size()){
            auto* first=dynamic_cast<CommonWrapper*>((*childNodes)->get(0).requireNode("isStack"));
            auto* row=dynamic_cast<CommonMrow*>(first);
            if(row&&row->isStack)breakCount_=static_cast<std::int64_t>(row->breakCount());
            else breakCount_=0;
        }
        else breakCount_=0;
    }
    return static_cast<std::size_t>(breakCount_);
}
CommonWrapper* CommonWrapper::breakTop(CommonWrapper* mrow,CommonWrapper*){
    return node->linebreakContainer()||!parent?mrow:parent->breakTop(mrow,this);
}
CommonWrapper* CommonWrapper::wrap(mml::MmlNode* source,tree::FactoryArgs args){
    if(!source)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'kind')");
    CommonWrapper* requestedParent=nullptr;
    if(!args.empty()) {
        if (args.front().type()==typeid(CommonWrapper*))
            requestedParent=std::any_cast<CommonWrapper*>(args.front());
        else if (args.front().type()==typeid(std::nullptr_t))
            requestedParent=nullptr;
    }
    return wrapNode(*source,requestedParent);
}
CommonWrapper* CommonWrapper::wrapNode(mml::MmlNode& source,CommonWrapper* parentValue){
    // AbstractWrapperFactory::wrap forwards [node, ...args] into the selected
    // Source wrapper constructor.  The second argument is parent || this.
    const std::array<std::any,1> args{std::any(parentValue ? parentValue : this)};
    CommonWrapper* result=factory().wrap(&source,args);
    if(parentValue&&parentValue->childNodes&&*parentValue->childNodes)(*parentValue->childNodes)->push_back(result);
    jax().nodeMap->insert_or_assign(&source,result);
    return result;
}
CommonWrapper::BBoxRef CommonWrapper::getBBox(bool save){
    if(bboxComputed_)return bbox;
    cancellation::checkpoint(cancellation::CheckpointStage::Layout);
    BBox* target=save?bbox.get():nullptr;
    BBox temporary=BBox::zero();
    computeBBox(target?*target:temporary);
    bboxComputed_=save;
    return target?bbox:std::make_shared<BBox>(temporary);
}
void CommonWrapper::computeBBox(BBox& target,bool recompute){
    target.empty();
    if(childNodes){
        const auto children=*childNodes;
        if(!children)throw options::Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        for(std::size_t index=0;index<children->size();++index){
            cancellation::checkpoint(cancellation::CheckpointStage::Layout);
            const auto child=children->get(index);
            auto* wrapped=dynamic_cast<CommonWrapper*>(child.requireNode("getOuterBBox"));
            target.append(*wrapped->getOuterBBox());
        }
    }
    target.clean();
    if(fixesPWidth()&&setChildPWidths(recompute)&&!recompute)computeBBox(target,true);
}
std::shared_ptr<LineBBox> CommonWrapper::computeLineBBox(std::size_t index){
    if(!childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
    const auto children=*childNodes;
    if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading '0')");
    auto* child=dynamic_cast<CommonWrapper*>(children->get(0).requireNode("getLineBBox"));
    return getChildLineBBox(*child,index);
}
std::shared_ptr<LineBBox> CommonWrapper::getLineBBox(std::size_t index){
    if(index>=lineBBox.size())lineBBox.resize(index+1);
    if(lineBBox[index])return lineBBox[index];
    const auto count=breakCount();
    if(count){
        auto line=embellishedBBox(index);
        if(!line)line=computeLineBBox(index);
        lineBBox[index]=line;
        if(index==0){
            if(!node->isKind("mo")&&node->isEmbellished())line->originalL=getBBox()->L;
            else line->L=getBBox()->L;
        }
        if(index==count)line->R=getBBox()->R;
    }else{
        const auto outer=getOuterBBox();
        lineBBox[index]=std::make_shared<LineBBox>(LineBBox::from(*outer,jax().currentLinebreakOptions().lineleading));
    }
    return lineBBox[index];
}
std::shared_ptr<LineBBox> CommonWrapper::embellishedBBox(std::size_t index){
    if(!node->isEmbellished()||node->isKind("mo"))return {};
    auto* coreOperator=coreMO();
    if(!coreOperator)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'moLineBBox')");
    auto* mo=dynamic_cast<CommonMo*>(coreOperator);
    if(!mo)throw options::Exception("TypeError",u"Embellished operator wrapper has no moLineBBox method");
    return std::make_shared<LineBBox>(mo->moLineBBox(index,mo->embellishedBreakStyle(),getOuterBBox().get()));
}
std::shared_ptr<LineBBox> CommonWrapper::getChildLineBBox(CommonWrapper& child,std::size_t index){
    const auto lines=child.getLineBBox(index);
    auto result=lines;
    if(styleData||bbox->L||bbox->R)result=std::make_shared<LineBBox>(lines->copy());
    addMiddleBorders(*result);
    const auto count=breakCount();
    if(index==0){result->L+=bbox->L;addLeftBorders(*result);}
    else if(index==count){result->R+=bbox->R;addRightBorders(*result);}
    return result;
}
std::array<CommonWrapper*,2> CommonWrapper::getBreakNode(LineBBox& line){
    if(!line.start)return {this,nullptr};
    const auto [childIndex,lineIndex]=*line.start;
    if(node->isEmbellished())return {this,coreMO()};
    auto children=childNodes;
    if(children&&*children&&(*children)->size()){
        const auto first=(*children)->get(0);
        if(first.isNode()){
            auto* firstWrapper=first.requireNode("node");
            if(firstWrapper->node->isInferred()||node->isKind("semantics"))
                children=firstWrapper->childNodes;
        }
    }
    if(node->isToken()||!children||!*children||childIndex>=(*children)->size())return {this,nullptr};
    auto* child=dynamic_cast<CommonWrapper*>((*children)->get(childIndex).requireNode("getBreakNode"));
    return child->getBreakNode(*child->getLineBBox(lineIndex));
}
double CommonWrapper::sideStyleSize(std::size_t side)const{
    if(!styleData||side>=4)return 0;
    const auto border=styleData->border?styleData->border->width[side]:0;
    return border+styleData->padding[side]+styleData->margin[side];
}
void CommonWrapper::addLeftBorders(BBox& target)const{if(styleData)target.w+=sideStyleSize(3);}
void CommonWrapper::addMiddleBorders(BBox& target)const{
    if(!styleData)return;target.h+=sideStyleSize(0);target.d+=sideStyleSize(2);
}
void CommonWrapper::addRightBorders(BBox& target)const{if(styleData)target.w+=sideStyleSize(1);}
void CommonWrapper::copySkewIC(BBox& target)const{
    if(!childNodes)return;const auto children=*childNodes;if(!children)return;
    if(children->size()){
        const auto firstValue=children->get(0);
        if(firstValue.isNode()){
            auto* first=dynamic_cast<CommonWrapper*>(firstValue.requireNode("bbox"));
            if(first->bbox->sk)target.sk=first->bbox->sk;
            if(first->bbox->dx)target.dx=first->bbox->dx;
        }
        const auto lastValue=children->get(children->size()-1);
        if(lastValue.isNode()){
            auto* last=dynamic_cast<CommonWrapper*>(lastValue.requireNode("bbox"));
            if(last->bbox->ic){target.ic=last->bbox->ic;target.w+=target.ic;}
        }
    }
}
CommonWrapper::BBoxRef CommonWrapper::getOuterBBox(bool save){
    auto current=getBBox(save);
    if(!styleData)return current;
    auto outer=std::make_shared<BBox>(current->copy());
    const auto border=styleData->border?styleData->border->width:std::array<double,4>{};
    for(const auto& side:BBox::boxSides){
        const auto i=static_cast<std::size_t>(side.index);
        const auto margin=styleData->margin[i],padding=styleData->padding[i];
        if(side.dimension=="w"&&side.index==1)outer->w+=margin+padding+border[i];
        else if(side.dimension=="w"&&side.index==3){outer->w+=margin+padding+border[i];}
        else if(side.dimension=="h")outer->h+=margin+padding+border[i];
        else outer->d+=margin+padding+border[i];
    }
    return outer;
}
std::array<double,2> CommonWrapper::getUnbrokenHD(){
    double h=0,d=0;for(std::size_t i=0,n=breakCount()+1;i<n;++i){const auto line=getLineBBox(i);h=std::max(h,line->h);d=std::max(d,line->d);}return {h,d};
}
void CommonWrapper::breakToWidth(double){
    // The base Source wrapper deliberately does nothing.  The concrete
    // Common LinebreakVisitor performs the recursive work when configured.
}
bool CommonWrapper::setChildPWidths(bool recompute,std::optional<double> width,bool clear){
    if(recompute)return false;
    if(clear)bbox->pwidth.clear();
    bool changed=false;
    if(!childNodes)throw options::Exception("TypeError",u"object undefined is not iterable (cannot read property Symbol(Symbol.iterator))");
    const auto children=*childNodes;
    if(!children)throw options::Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for(std::size_t i=0;i<children->size();++i){
        const auto value=children->get(i);
        auto* child=dynamic_cast<CommonWrapper*>(value.requireNode("getBBox"));
        const auto box=child->getBBox();
        if(!box->pwidth.empty()&&child->setChildPWidths(false,width.value_or(box->w),clear))changed=true;
    }
    return changed;
}
void CommonWrapper::invalidateBBox(bool bubble){
    if(bboxComputed_||breakCount_>=0){bboxComputed_=false;lineBBox.clear();breakCount_=-1;if(parent&&bubble)parent->invalidateBBox();}
}
CommonWrapper* CommonWrapper::core(){const auto value=node->core();if(!value.isNode())return nullptr;return jax().nodeMap->find(value.nodeOrNull());}
CommonMo* CommonWrapper::coreMO(){const auto value=node->coreMO();if(!value.isNode())return nullptr;return dynamic_cast<CommonMo*>(jax().nodeMap->find(value.nodeOrNull()));}
double CommonWrapper::coreRScale()const{double scale=bbox->rscale;CommonWrapper* current=const_cast<CommonWrapper*>(this)->coreMO();while(current&&current!=this){scale*=current->bbox->rscale;current=current->parent;}return scale;}
double CommonWrapper::getRScale()const{double scale=1;for(auto* current=const_cast<CommonWrapper*>(this);current;current=current->parent)scale*=current->bbox->rscale;return scale;}
std::u16string CommonWrapper::getText()const{
    std::u16string out;if(!node->isToken())return out;
    const auto children=node->childArrayOwner();
    if(!children)throw options::Exception("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    children->forOf([&](mml::NodeValue child){
        if(auto* text=dynamic_cast<mml::TextNode*>(child.nodeOrNull()))out+=text->getText();
    });
    return out;
}
std::shared_ptr<std::vector<std::uint32_t>> CommonWrapper::unicodeChars(
    std::u16string_view text,std::string_view name)const {
    auto result=std::make_shared<std::vector<std::uint32_t>>();
    result->reserve(text.size());
    for(std::size_t i=0;i<text.size();++i){
        std::uint32_t code=text[i];
        if(code>=0xD800&&code<=0xDBFF&&i+1<text.size()&&text[i+1]>=0xDC00&&text[i+1]<=0xDFFF)
            code=0x10000+((code-0xD800)<<10)+(text[++i]-0xDC00);
        result->push_back(code);
    }
    if(!font)return result;
    const auto variantData=font->getVariant(name);
    if(!variantData||!variantData->get().chars)return result;
    const auto chars=variantData->get().chars;
    for(auto& code:*result){
        const auto record=chars->get(code);
        if(!record)continue;
        const auto* property=std::get_if<Property>(&*record);
        if(!property)continue;
        const auto tuple=PropertyArray::array(*property);
        if(!tuple)continue;
        const auto optionValue=tuple->get(3);
        if(optionValue.missing()||std::holds_alternative<PropertyNull>(optionValue.value))continue;
        const auto optionRecord=PropertyList::record(optionValue);
        if(!optionRecord)throw options::Exception("TypeError",u"Character options are not an object");
        const auto smp=optionRecord->get("smp");
        if(!property_host::truthy(smp))continue;
        if(const auto* number=std::get_if<double>(&smp.value))code=static_cast<std::uint32_t>(*number);
        else throw options::Exception("Unimplemented",u"Character SMP remapping requires a numeric code point");
    }
    return result;
}
std::shared_ptr<std::vector<std::uint32_t>> CommonWrapper::remapChars(
    std::shared_ptr<std::vector<std::uint32_t>> chars)const { return chars; }
mml::TextNode* CommonWrapper::mmlText(std::u16string text){
    const std::array<std::any,0> noArguments{};
    auto* created=node->factoryOwner().create("text",noArguments);
    auto* result=dynamic_cast<mml::TextNode*>(created);
    if(!result)throw options::Exception("TypeError",u"MML text factory product has no setText method");
    return result->setText(std::move(text));
}
mml::MmlNode* CommonWrapper::mmlNode(std::string_view kind,PropertyList properties,
                                     std::vector<mml::MmlNode*> children){
    auto childArray=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    childArray->resize(children.size());
    for(std::size_t i=0;i<children.size();++i)
        childArray->set(i,children[i]?mml::NodeValue(children[i]):mml::NodeValue(PropertyNull{}));
    const std::array<std::any,2> arguments{std::any(std::move(properties)),std::any(std::move(childArray))};
    return node->factoryOwner().create(kind,arguments);
}
CommonMo& CommonWrapper::createMo(std::u16string_view text){
    auto* textNode=mmlText(std::u16string(text));
    PropertyList properties;
    properties.setOwn("stretchy",Property(true));
    auto* mo=mmlNode("mo",std::move(properties),std::vector<mml::MmlNode*>{textNode});
    mo->inheritAttributesFrom(mml::NodeValue(node));
    mo->setParent(node->getParent());
    auto* wrapped=wrap(mo);
    wrapped->parent=this;
    auto* result=dynamic_cast<CommonMo*>(wrapped);
    if(!result)throw options::Exception("TypeError",u"The mo wrapper factory returned an incompatible wrapper");
    return *result;
}
double CommonWrapper::getWrapWidth(std::size_t index){
    if(!childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getBBox')");
    const auto children=*childNodes;
    if(!children)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getBBox')");
    const auto value=children->get(index);
    auto* child=dynamic_cast<CommonWrapper*>(value.requireNode("getBBox"));
    return child->getBBox()->w;
}
CharDataArray CommonWrapper::getVariantChar(std::string_view variantName,std::uint32_t codepoint){
    if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getChar')");
    const auto character=font->getChar(variantName,static_cast<double>(codepoint));
    PropertyArray result;
    if(character){
        const auto array=PropertyArray::array(*character);
        if(!array)throw options::Exception("TypeError",u"Character data is not an array");
        result=*array;
    }else{
        PropertyList unknown;
        unknown.setOwn("unknown",Property(true));
        result=PropertyArray{Property(0),Property(0),Property(0),Property(unknown)};
    }
    if(result.size()==3)result.set(3,Property(PropertyList{}));
    return result;
}
bool CommonWrapper::canStretch(std::string_view direction){
    (void)direction;stretch=NOSTRETCH;
    if(node->isEmbellished())if(auto* inner=core();inner&&inner!=this&&inner->canStretch(direction))stretch=inner->stretch;
    return stretch&&stretch->dir!=std::string(NONE);
}
std::pair<std::string,double> CommonWrapper::getAlignShift(){
    const auto attrs=node->attributes()->getAllAttributes();Property align=attrs.get("indentalign"),shift=attrs.get("indentshift");
    Property firstAlign=attrs.get("indentalignfirst"),firstShift=attrs.get("indentshiftfirst");
    if(firstAlign!=Property("indentalign"))align=firstAlign;
    if(firstShift!=Property("indentshift"))shift=firstShift;
    auto result=processIndent(stringOf(align),stringOf(shift));return result;
}
std::pair<std::string,double> CommonWrapper::processIndent(std::string_view alignment,std::string_view shift,std::string_view defaultAlign,std::string_view defaultShift,std::optional<double> width){
    if(!jax().math||!property_host::truthy(jax().math->display))return {"left",0};
    std::string align(alignment),indent(shift),baseAlign(defaultAlign),baseShift(defaultShift);
    auto inlineMarked=[&](){return property_host::truthy(jax().math->root.requireNode("getProperty")->getProperty("inlineMarked"));};
    if(baseAlign.empty()||baseAlign=="auto")
        baseAlign=inlineMarked()?"left":optionString(jax().optionsContext().get(jax().options,options::Key(u"displayAlign")));
    if(baseShift.empty()||baseShift=="auto")
        baseShift=inlineMarked()?"0":optionString(jax().optionsContext().get(jax().options,options::Key(u"displayIndent")));
    if(align=="auto")align=baseAlign;
    if(indent=="auto"){
        indent=baseShift;
        const auto units=utf16(indent);
        if(align=="right"&&!zeroIndentPattern(units)){
            const auto trimmed=trimJsSpace(units);
            indent="-"+utf8(trimmed);
            if(indent.starts_with("--"))indent.erase(0,2);
        }
    }
    const auto dim=length2em(Property(indent),width.value_or(metrics().containerWidth));
    return {align,dim};
}
std::string CommonWrapper::getChildAlign(std::size_t)const{return "left";}

LayoutJob::LayoutJob(core::MathItem& mathValue,
                     core::MathDocumentReference documentValue,
                     DOMNodeRef containerValue)
    :math(&mathValue),document(std::move(documentValue)),
     container(std::move(containerValue)){}

CommonWrapper* LayoutJob::adopt(std::unique_ptr<CommonWrapper> wrapper){
    if(!wrapper)
        throw options::Exception("TypeError",u"Cannot adopt an empty CommonWrapper");
    auto* result=wrapper.get();
    wrappers_.push_back(std::move(wrapper));
    return result;
}
}
