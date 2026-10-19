module folia.math.output.svg.serializer;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.lengths;
import folia.math.cancellation;

namespace folia::math::output::svg {
using namespace folia::math;
using namespace folia::math::output::scene;
namespace {
thread_local Serializer::MaterializationObserver activeMaterializationObserver;
void observeMaterialization(){if(activeMaterializationObserver)activeMaterializationObserver();}
std::u16string ascii16(std::string value){return std::u16string(value.begin(),value.end());}
std::string fixed(double value,unsigned digits=1){
    if(std::abs(value*1000)<0.0006)return "0";
    auto result=js_number::toFixed(value*1000,digits);
    if(const auto point=result.find('.');point!=std::string::npos){
        while(!result.empty()&&result.back()=='0')result.pop_back();
        if(!result.empty()&&result.back()=='.')result.pop_back();
    }
    return result;
}
std::string transforms(const Transform& transform,double coordinateUnits=1000,double scaleProjection=1){
    std::string result;
    const auto append=[&](std::string value){
        if(!result.empty())result+=' ';
        result+=std::move(value);
    };
    if(!transform.operations.empty()){
        for(const auto& op:transform.operations){
            switch(op.kind){
            case TransformKind::Translate:
                append("translate("+fixed(op.x*coordinateUnits/1000)+(op.spaceSeparated?" ":op.commaSpaceSeparated?", ":",")+
                    fixed(op.y*coordinateUnits/1000)+")");
                break;
            case TransformKind::Scale:
                append("scale("+fixed(op.x*scaleProjection/1000,op.scaleDigits)+(op.scalarScale?std::string{}:
                    (op.spaceSeparated?" ":",")+fixed(op.y*scaleProjection/1000,op.scaleDigits))+")");
                break;
            case TransformKind::Rotate:
                append("rotate("+fixed(op.degrees/1000,3)+" "+fixed(op.centerX*coordinateUnits/1000)+" "+
                    fixed(op.centerY*coordinateUnits/1000)+")");
                break;
            case TransformKind::Matrix:
                append("matrix("+fixed(op.a/1000,3)+" "+fixed(op.b/1000,3)+" "+
                    fixed(op.c/1000,3)+" "+fixed(op.d/1000,3)+" "+
                    fixed(op.e*coordinateUnits/1000)+" "+fixed(op.f*coordinateUnits/1000)+")");
                break;
            }
        }
    }else{
        if(transform.translateX||transform.translateY)
            append("translate("+fixed(transform.translateX*coordinateUnits/1000)+","+
                fixed(transform.translateY*coordinateUnits/1000)+")");
        if(transform.scaleX!=1||transform.scaleY!=1)
            append("scale("+fixed(transform.scaleX*scaleProjection/1000,3)+","+
                fixed(transform.scaleY*scaleProjection/1000,3)+")");
    }
    return result;
}
std::string hexCode(std::uint32_t value){
    std::array<char,8> buffer{};
    const auto [end,error]=std::to_chars(buffer.data(),buffer.data()+buffer.size(),value,16);
    if(error!=std::errc{})throw std::runtime_error("SVG glyph codepoint conversion failed");
    std::string result(buffer.data(),end);
    std::transform(result.begin(),result.end(),result.begin(),[](unsigned char c){
        return static_cast<char>(std::toupper(c));
    });
    return result;
}
}
Serializer::MaterializationObserver Serializer::exchangeMaterializationObserver(
    MaterializationObserver observer){
    return std::exchange(activeMaterializationObserver,std::move(observer));
}
PropertyList Serializer::attributes(const FrozenNode& source){
    // Materialization is read-only with respect to the completed scene.  A
    // PropertyList copy shares its storage, so build an independent record and
    // copy the Source attributes in their original own-key order before adding
    // SVG-derived fields.
    PropertyList result;
    for(std::size_t index=0;index<source.attributes.size();++index){
        if(source.transformAttributeIndex==index)
            result.set("transform",Property(""));
        const auto& [key,value]=source.attributes[index];
        result.set(key,Property(value));
    }
    // Source DOM owns this complete attribute object. Keep typed geometry for
    // drawing without rewriting the original DOM spellings during SVG output.
    if(std::holds_alternative<SourceDomPrimitive>(source.semantic))return result;
    if(const auto* clip=std::get_if<ClipPrimitive>(&source.semantic))
        result.set("id",Property(clip->id));
    if(const auto* link=std::get_if<LinkPrimitive>(&source.semantic))
        result.set("href",Property(property_host::debugText(link->href)));
    if(const auto* image=std::get_if<ImagePrimitive>(&source.semantic))
        result.set("href",Property(property_host::debugText(image->href)));
    if(const auto* text=std::get_if<TextRunPrimitive>(&source.semantic);
       text&&text->fontSize&&result.hasOwn("font-size"))
        result.set("font-size",Property(fixed(*text->fontSize)+"px"));
    if(source.paint&&source.paint->dash){
        const auto& dash=*source.paint->dash;
        if(result.hasOwn("stroke-dasharray")){
            std::string value;
            for(const auto length:dash.lengths){
                if(!value.empty())value+=dash.separator;
                value+=fixed(length);
            }
            result.set("stroke-dasharray",Property(value.empty()?"none":value));
        }
        if(result.hasOwn("stroke-dashoffset"))
            result.set("stroke-dashoffset",Property(fixed(dash.offset)));
    }
    if(source.paint&&source.paint->emitAttributes){
        const auto& paint=*source.paint;
        if(paint.fill&&!result.hasOwn("fill"))result.set("fill",Property(*paint.fill));
        if(paint.stroke&&!result.hasOwn("stroke"))result.set("stroke",Property(*paint.stroke));
        if(paint.strokeWidth&&!result.hasOwn("stroke-width"))
            result.set("stroke-width",Property(fixed(*paint.strokeWidth)));
        if(paint.lineCap&&!result.hasOwn("stroke-linecap")){
            const auto value=*paint.lineCap==LineCap::Butt?"butt":*paint.lineCap==LineCap::Round?"round":"square";
            result.set("stroke-linecap",Property(value));
        }
        if(paint.lineJoin&&!result.hasOwn("stroke-linejoin")){
            const auto value=*paint.lineJoin==LineJoin::Miter?"miter":*paint.lineJoin==LineJoin::Round?"round":"bevel";
            result.set("stroke-linejoin",Property(value));
        }
    }
    if(const auto* rect=std::get_if<RectGeometry>(&source.geometry)){
        for(std::size_t i=0;i<rect->fieldCount;++i){
            switch(rect->fields[i]){
                case RectField::X:result.set("x",Property(fixed(rect->x)));break;
                case RectField::Y:result.set("y",Property(fixed(rect->y)));break;
                case RectField::Width:result.set("width",Property(fixed(rect->width)));break;
                case RectField::Height:result.set("height",Property(fixed(rect->height)));break;
            }
        }
        if(rect->rx&&!result.hasOwn("rx"))result.set("rx",Property(fixed(*rect->rx)));
        if(rect->ry&&!result.hasOwn("ry"))result.set("ry",Property(fixed(*rect->ry)));
    }else if(const auto* viewport=std::get_if<ViewportGeometry>(&source.geometry)){
        if(viewport->emitRect){
            result.set("x",Property(fixed(viewport->x)));result.set("y",Property(fixed(viewport->y)));
            result.set("width",Property(viewport->widthRelative
                ?js_number::toString(viewport->width*100)+"%":fixed(viewport->width)));
            result.set("height",Property(viewport->heightRelative
                ?js_number::toString(viewport->height*100)+"%":fixed(viewport->height)));
        }
        if(viewport->hasViewBox){
            const auto viewBox=fixed(viewport->viewX)+" "+fixed(viewport->viewY)+" "+
                fixed(viewport->viewWidth)+" "+fixed(viewport->viewHeight);
            result.set("viewBox",Property(viewBox));
        }
        if(result.hasOwn("preserveAspectRatio")){
            constexpr std::array<std::string_view,10> names={"none","xMinYMin","xMidYMin","xMaxYMin",
                "xMinYMid","xMidYMid","xMaxYMid","xMinYMax","xMidYMax","xMaxYMax"};
            std::string value(names[static_cast<std::size_t>(viewport->alignment)]);
            if(viewport->alignment!=ViewportAlignment::None&&viewport->emitFit)
                value+=viewport->fit==ViewportFit::Meet?" meet":" slice";
            result.set("preserveAspectRatio",Property(std::move(value)));
        }
    }else if(const auto* line=std::get_if<LineGeometry>(&source.geometry)){
        result.set("x1",Property(fixed(line->from.x)));result.set("y1",Property(fixed(line->from.y)));
        result.set("x2",Property(fixed(line->to.x)));result.set("y2",Property(fixed(line->to.y)));
        if(line->emitStrokeWidth)
            result.set("stroke-width",Property(fixed(line->width)));
    }else if(const auto* polygon=std::get_if<PolygonGeometry>(&source.geometry)){
        std::string points;
        for(const auto& point:polygon->points){if(!points.empty())points+=' ';points+=fixed(point.x)+","+fixed(point.y);}
        result.set("points",Property(points));
    }else if(const auto* ellipse=std::get_if<EllipseGeometry>(&source.geometry)){
        if(!result.hasOwn("cx"))result.set("cx",Property(fixed(ellipse->cx)));
        if(!result.hasOwn("cy"))result.set("cy",Property(fixed(ellipse->cy)));
        if(!result.hasOwn("rx"))result.set("rx",Property(fixed(ellipse->rx)));
        if(!result.hasOwn("ry"))result.set("ry",Property(fixed(ellipse->ry)));
    }else if(const auto* path=std::get_if<PathGeometry>(&source.geometry)){
        if(!path->outline)property_host::typeError("Cannot read properties of null (reading 'sourcePath')");
        result.set("d",Property(path->outline->sourcePath));
    }else if(const auto* use=std::get_if<UseGeometry>(&source.geometry)){
        result.set("x",Property(fixed(use->x)));
        result.set("y",Property(fixed(use->y)));
    }else if(const auto* foreign=std::get_if<ForeignObjectGeometry>(&source.geometry)){
        // Source formatter outputs are presentation metadata captured at the
        // real SVG call. Typed bounds and transforms remain the drawing data.
        if(foreign->synthesizeMissingAttributes){
            if(!result.hasOwn("y"))result.set("y",Property(fixed(foreign->y/1000)+"px"));
            if(!result.hasOwn("width"))result.set("width",Property(fixed(foreign->width/1000)+"px"));
            if(!result.hasOwn("height"))result.set("height",Property(fixed(foreign->height/1000)+"px"));
            if(!result.hasOwn("transform"))result.set("transform",Property(transforms(foreign->transform,1,1000)));
        }
    }
    const bool hasSourceTransform=std::ranges::any_of(source.attributes,
        [](const auto& attribute){return attribute.first=="transform";});
    // A transform written through the Source DOM is retained verbatim until a
    // later layout placement records a generated transform slot. Typed values
    // remain available to renderers and drive all computed placement.
    if(source.transform&&(!hasSourceTransform||source.transformAttributeIndex)){
        std::string value=transforms(*source.transform);
        if(source.transformSuffix&&!source.transformSuffix->empty()){
            if(!value.empty())value+=' ';
            value+=property_host::debugText(*source.transformSuffix);
        }
        // Placement follows primitive creation. A Source-recorded slot keeps
        // earlier handleScale transforms in their original attribute order.
        // Identity geometry alone does not introduce an empty DOM attribute.
        if(!value.empty())result.set("transform",Property(value));
    }
    return result;
}
DOMNodeRef Serializer::materialize(const FrozenNodeRef& source){
    cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
    if(!source)property_host::typeError("Cannot materialize null scene node");
    if(const auto* flow=std::get_if<InlineBreakPrimitive>(&source->semantic)){
        PropertyList attributes;
        if(flow->prebreak)attributes.set("prebreak",Property(true));
        else if(!flow->forced)attributes.set("newline",Property(true));
        else{
            const auto space=lengths::em(flow->space);
            constexpr std::array<double,6> widths{0,2.0/18,3.0/18,4.0/18,5.0/18,6.0/18};
            const auto found=std::ranges::find_if(widths,[&](double width){return lengths::em(width)==space;});
            if(found!=widths.end())attributes.set("size",Property(std::to_string(found-widths.begin())));
            else attributes.set("style",Property("letter-spacing: "+lengths::em(flow->space-1)));
        }
        return adaptor_.node(u"mjx-break",std::move(attributes),
            PropertyArray{nodeProperty(adaptor_.text(u" "))});
    }
    if(source->kind==u"#text"){
        if(const auto* sourceText=std::get_if<SourceTextPrimitive>(&source->semantic))
            return adaptor_.text(sourceText->currentText());
        const auto* text=std::get_if<TextRunPrimitive>(&source->semantic);
        if(!text)property_host::typeError("Scene text node has no TextRun payload");
        return adaptor_.text(text->text);
    }
    if(source->kind==u"#comment"){
        if(const auto* sourceText=std::get_if<SourceTextPrimitive>(&source->semantic))
            return adaptor_.comment(sourceText->currentText());
        const auto* comment=std::get_if<CommentRunPrimitive>(&source->semantic);
        if(!comment)property_host::typeError("Scene comment node has no CommentRun payload");
        return adaptor_.comment(comment->text);
    }
    PropertyArray children;
    for(const auto& child:source->children){
        cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
        children.push(nodeProperty(materialize(child)));
    }
    if(const auto* html=std::get_if<HtmlNodePrimitive>(&source->semantic))
        children.push(nodeProperty(materializeHtml(*html)));
    auto result=adaptor_.node(source->kind,attributes(*source),std::move(children),SVG_NAMESPACE);
    if(const auto* use=std::get_if<UseGeometry>(&source->geometry))
        adaptor_.setAttribute(result,u"href",Property(use->href),
                              use->xlink?XLINK_NAMESPACE:std::u16string_view{});
    if(const auto* glyph=std::get_if<GlyphPrimitive>(&source->semantic))
        adaptor_.setAttribute(result,u"data-c",Property(hexCode(glyph->codepoint)));
    return result;
}
DOMNodeRef Serializer::materializeHtml(const HtmlNodePrimitive& source){
    cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
    if(source.kind==u"#text")return adaptor_.text(source.text);
    if(source.kind==u"#comment")return adaptor_.comment(source.text);
    PropertyList attributes;
    for(const auto& [key,value]:source.attributes){
        cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
        attributes.set(property_host::debugText(key),Property(property_host::debugText(value)));
    }
    PropertyArray children;
    for(const auto& child:source.children){
        cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
        children.push(nodeProperty(materializeHtml(child)));
    }
    return adaptor_.node(source.kind,std::move(attributes),std::move(children),source.namespaceHint);
}
std::vector<DOMNodeRef> Serializer::materialize(const Scene& source){
    observeMaterialization();
    std::vector<DOMNodeRef> roots;
    roots.reserve(source.roots().size());
    for(const auto& root:source.roots()){
        cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
        roots.push_back(materialize(root));
    }
    return roots;
}
std::u16string Serializer::serialize(const DOMNodeRef& root)const{
    return adaptor_.outerHTML(root);
}
}
