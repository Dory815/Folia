/*************************************************************
 * Source SvgWrapper glyph-path, text-run and cache placement operations.
 *************************************************************/
module folia.math.output.scene;
import :glyph_paint;
import :svg_paint;
import :svg_paint;
import folia.math.util.property_host;
import folia.math.util.string;
import std;

namespace folia::math::output::scene {
namespace {
using folia::math::Property;
using folia::math::property_host::truthy;
std::string hexCode(std::uint32_t value){
    std::array<char,8> data{};
    auto [end,error]=std::to_chars(data.data(),data.data()+data.size(),value,16);
    if(error!=std::errc{})throw std::runtime_error("Glyph codepoint conversion failed");
    std::string result(data.data(),end);
    std::transform(result.begin(),result.end(),result.begin(),[](unsigned char c){return static_cast<char>(std::toupper(c));});
    return result;
}
Property slot(const common::CharDataArray& data,std::size_t index){
    return index<data.size()?data.get(index):Property(folia::math::Missing{});
}
bool hasProperty(const Property& value,std::string_view key){
    auto record=PropertyList::record(value);
    if(!record)throw options::Exception("TypeError",u"Cannot use 'in' operator to search in a non-object glyph record");
    return record->has(key);
}
void observePlace(common::CommonWrapper& wrapper,Builder& builder,double x,double y,
                  const NodeRef& element){
    const auto observer=builder.layoutObserver();
    if(observer&&observer->place)observer->place(wrapper,x,y,element,true);
}
std::u16string fromCodepoint(std::uint32_t c){
    std::u16string result;
    if(c<=0xFFFF)result.push_back(static_cast<char16_t>(c));
    else{c-=0x10000;result.push_back(static_cast<char16_t>(0xD800+(c>>10)));result.push_back(static_cast<char16_t>(0xDC00+(c&0x3FF)));}
    return result;
}
}

Property glyphSlot(const common::CharDataArray& data,std::size_t index){
    return index<data.size()?data.get(index):Property(folia::math::Missing{});
}

std::shared_ptr<std::vector<std::uint32_t>> glyphUnicodeChars(
    common::CommonWrapper& wrapper,std::u16string_view text,std::string_view variant){
    auto chars=std::make_shared<std::vector<std::uint32_t>>();
    for(std::size_t i=0;i<text.size();++i){
        std::uint32_t c=text[i];
        if(c>=0xD800&&c<=0xDBFF&&i+1<text.size()&&text[i+1]>=0xDC00&&text[i+1]<=0xDFFF)
            c=0x10000+((c-0xD800)<<10)+(text[++i]-0xDC00);
        chars->push_back(c);
    }
    return wrapper.remapChars(std::move(chars));
}

NodeRef emitTextRun(common::CommonWrapper& wrapper,Builder& builder,const NodeRef& parent,
                 std::u16string text,std::string_view variant,double x,double y,bool place){
    if(text.empty())return {};
    const auto metrics=wrapper.jax().currentMetrics();
    const double size=wrapper.font->params().x_height/metrics.ex*metrics.em*1000;
    auto child=builder.node(u"#text",{},text);
    auto node=builder.node(u"text",PropertyList{{"data-variant",Property(std::string(variant))},
        {"transform",Property("scale(1,-1)")},
        {"font-size",Property(wrapper.jax().fixed(size,1)+"px")}}, {},1);
    // SvgOutputJax.unknownText creates this inversion before any placement.
    // The font size is in 1000-unit SVG coordinates at the Source call;
    // retain its full-precision mathematical-em value for every consumer.
    node->transform=Transform{0,0,1,-1};
    const auto chars=glyphUnicodeChars(wrapper,text,variant);
    if(variant!="-explicitFont"&&
       !(chars->size()==1&&chars->front()>=0x1D400&&chars->front()<=0x1D7FF)){
        const auto css=wrapper.font->getCssFont(variant);
        const auto family=property_host::string(css.get(0));
        const auto resolved=property_host::debugText(family);
        node->attributes.set("font-family",Property(resolved));
        const bool italic=truthy(css.get(1)),bold=truthy(css.get(2));
        if(italic)node->attributes.set("font-style",Property("italic"));
        if(bold)node->attributes.set("font-weight",Property("bold"));
        node->semantic=TextRunPrimitive{text,std::string(variant),resolved,italic,bold,size/1000};
    }else node->semantic=TextRunPrimitive{text,std::string(variant),{},false,false,size/1000};
    builder.append(node,child);
    builder.append(parent,node);
    if(place)placeSvgNode(wrapper,builder,node,x,y,false);
    return node;
}

GlyphResult emitGlyph(common::CommonWrapper& wrapper,Builder& builder,WrapperFactory& placements,
    const NodeRef& parent,std::uint32_t codepoint,double x,double y,
    std::string_view variant,unsigned depth,std::optional<bool> buffer,bool observeCall,
    const common::CharDataArray* sourceData){
    if(depth>32)throw options::Exception("RangeError",u"Composite glyph recursion exceeded");
    const auto observer=builder.layoutObserver();
    std::optional<std::size_t> traceToken;
    if(observeCall&&observer&&observer->beginPlaceChar)
        traceToken=observer->beginPlaceChar(wrapper,codepoint,x,y,parent,variant,buffer);
    const auto data=sourceData?*sourceData:wrapper.getVariantChar(variant,codepoint);
    const double width=property_host::number(glyphSlot(data,2));
    const Property charOptions=glyphSlot(data,3);
    if(truthy(property_host::get(charOptions,"unknown"))){
        const auto node=emitTextRun(wrapper,builder,parent,fromCodepoint(codepoint),variant,x,y);
        const double textWidth=placements.measureTextNode(node,fromCodepoint(codepoint),variant).w;
        if(traceToken&&observer->finishPlaceChar)observer->finishPlaceChar(*traceToken,buffer.value_or(false)?0:textWidth);
        if(buffer.value_or(false))return {0,true};
        return {textWidth,true};
    }
    if(hasProperty(charOptions,"p")){
        const auto rawPath=property_host::get(charOptions,"p");
        std::string path;
        if(truthy(rawPath))path="M"+property_host::debugText(property_host::string(rawPath))+"Z";
        const auto outline=decodeGlyphOutline(path);
        NodeRef node;
        auto* mo=dynamic_cast<common::CommonMo*>(&wrapper);
        const auto cached=mo?placements.cacheGlyph(*mo,codepoint,variant,path,builder):std::nullopt;
        if(cached){
            node=builder.node(u"use",PropertyList{{"data-c",Property(hexCode(codepoint))}});
            node->geometry=UseGeometry{"#"+*cached,0,0,placements.useXlink()};
        }else{
            // Source pathNode passes both fields to svg() at creation.
            node=builder.node(u"path",PropertyList{{"data-c",Property(hexCode(codepoint))},
                {"d",Property(path)}});
            node->attributes.erase("d");
            node->geometry=PathGeometry{outline};
            builder.retain(outline);
        }
        node->semantic=GlyphPrimitive{codepoint,std::string(variant),wrapper.font->CLASS().NAME,outline};
        node->attributes.set("data-c",Property(hexCode(codepoint)));
        builder.append(parent,node);
        observePlace(wrapper,builder,x,y,node);
        if(!node->transform)node->transform=Transform{x,y,1,1};
        if(traceToken&&observer->finishPlaceChar)observer->finishPlaceChar(*traceToken,width);
        return {width,true};
    }
    if(hasProperty(charOptions,"c")){
        const auto composite=property_host::string(property_host::get(charOptions,"c"));
        auto group=builder.node(u"g",PropertyList{{"data-c",Property(hexCode(codepoint))}});
        builder.append(parent,group);
        observePlace(wrapper,builder,x,y,group);
        group->transform=Transform{x,y,1,1};
        const auto chars=glyphUnicodeChars(wrapper,composite,variant);
        double dx=0;
        for(const auto child:*chars){auto placed=emitGlyph(wrapper,builder,placements,group,child,dx,0,variant,depth+1,{},true);dx+=placed.width;}
        if(traceToken&&observer->finishPlaceChar)observer->finishPlaceChar(*traceToken,dx);
        return {dx,true};
    }
    if(traceToken&&observer->finishPlaceChar)observer->finishPlaceChar(*traceToken,width);
    return {width,false};
}

void placeGlyphText(common::CommonWrapper& wrapper,Builder& builder,WrapperFactory& placements,
    const NodeRef& parent,std::span<const std::uint32_t> chars,std::string_view variant,
    bool buffer){
    double x=0;std::u16string unknown;
    const auto flush=[&](){
        if(unknown.empty())return 0.0;
        const auto node=emitTextRun(wrapper,builder,parent,unknown,variant,x,0);
        const double width=placements.measureTextNode(node,unknown,variant).w;
        x+=width;
        unknown.clear();
        return width;
    };
    for(const auto codepoint:chars){
        const auto observer=builder.layoutObserver();
        std::optional<std::size_t> traceToken;
        if(observer&&observer->beginPlaceChar)
            traceToken=observer->beginPlaceChar(wrapper,codepoint,x,0,parent,variant,buffer);
        const auto data=wrapper.getVariantChar(variant,codepoint);
        const Property charOptions=glyphSlot(data,3);
        if(truthy(property_host::get(charOptions,"unknown"))){
            unknown+=fromCodepoint(codepoint);
            const double width=buffer?0:flush();
            if(traceToken&&observer->finishPlaceChar)observer->finishPlaceChar(*traceToken,width);
            continue;
        }
        const double dx=flush();
        const auto placed=emitGlyph(wrapper,builder,placements,parent,codepoint,x,0,variant,0,
                                    buffer,false,&data);
        x+=placed.width;
        if(traceToken&&observer->finishPlaceChar)
            observer->finishPlaceChar(*traceToken,dx+placed.width);
    }
    flush();
}

void emitGlyphToRoots(common::CommonWrapper& wrapper,Builder& builder,WrapperFactory& placements,
    std::span<const NodeRef> roots,std::uint32_t cp,double x,double y,std::string_view variant,
    double scaleX,double scaleY){
    for(const auto& root:roots){
        if(!root)continue;
        const auto before=root->children.size();
        (void)emitGlyph(wrapper,builder,placements,root,cp,x,y,variant,0);
        for(std::size_t i=before;i<root->children.size();++i)if(root->children[i]&&root->children[i]->transform){
            root->children[i]->transform->scaleX=scaleX;root->children[i]->transform->scaleY=scaleY;
        }
    }
}

NodeRef cloneSceneNode(const NodeRef& source){
    if(!source)throw options::Exception("TypeError",u"Cannot clone a null scene node");
    auto clone=std::make_shared<Node>();clone->kind=source->kind;
    for(const auto& key:source->attributes.keys())clone->attributes.set(key,source->attributes.get(key));
    clone->geometry=source->geometry;clone->semantic=source->semantic;clone->paint=source->paint;
    clone->transform=source->transform;clone->transformSuffix=source->transformSuffix;
    clone->transformAttributeIndex=source->transformAttributeIndex;
    const auto sourceChildren=source->children.snapshot();
    clone->children.reserve(sourceChildren.size());
    for(const auto& child:sourceChildren){
        auto childClone=cloneSceneNode(child);
        clone->children.push_back(childClone);
        // MathJax LiteAdaptor.clone assigns parent only to element children;
        // cloned #text and #comment nodes keep an unset parent field.
        if(child->kind!=u"#text"&&child->kind!=u"#comment")
            childClone->parent=clone;
    }
    return clone;
}

void emitGlyphExtender(common::CommonWrapper& wrapper,Builder& builder,WrapperFactory& placements,
    std::span<const NodeRef> roots,std::uint32_t cp,std::string_view variant,bool vertical,
    double x,double y,double width,double height,double viewX,double viewY,double viewWidth,
    double viewHeight,double scale){
    if(!cp)return;
    const auto fixed=[&](double value){return wrapper.jax().fixed(value*1000,1);};
    PropertyList definition{{"width",Property(fixed(width))},{"height",Property(fixed(height))}};
    if(vertical){definition.set("y",Property(fixed(y)));definition.set("x",Property(fixed(x)));}
    else{definition.set("x",Property(fixed(x)));definition.set("y",Property(fixed(y)));}
    definition.set("viewBox",Property(fixed(viewX)+" "+fixed(viewY)+" "+fixed(viewWidth)+" "+fixed(viewHeight)));
    auto viewport=builder.node(u"svg",std::move(definition));
    viewport->geometry=ViewportGeometry{x,y,width,height,viewX,viewY,viewWidth,viewHeight};
    const auto before=viewport->children.size();
    (void)emitGlyph(wrapper,builder,placements,viewport,cp,0,0,variant,0);
    if(before<viewport->children.size()&&viewport->children[before]->transform){
        if(vertical)viewport->children[before]->transform->scaleY=scale;
        else viewport->children[before]->transform->scaleX=scale;
    }
    if(roots.empty())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'append')");
    builder.append(roots[0],viewport);
    if(roots.size()>1&&roots[1])builder.append(roots[1],cloneSceneNode(viewport));
}
}
