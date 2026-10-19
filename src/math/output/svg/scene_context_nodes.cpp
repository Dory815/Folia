/*************************************************************
 * Canonical Context projection for output Scene nodes.
 *************************************************************/
module folia.math.output.svg.scene_context_nodes;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.context_property_bindings;
import folia.math.util.styles;
import std;

namespace folia::math::output::svg {
using namespace options;
struct SceneContextNodes::State : std::enable_shared_from_this<State> {
    struct SourceView;
    struct ServiceAccess {
        std::shared_ptr<State> state;
        std::shared_ptr<void> lease;
        State* operator->()const noexcept{return state.get();}
    };
    struct Payload {
        scene::NodeRef node;
        std::optional<ObjectRef> owner;
        std::optional<ObjectRef> definition;
        std::optional<ObjectRef> attributes;
        std::shared_ptr<styles::Styles> styles;
    };
    struct PresentationWrite {
        const scene::Node* node=nullptr;
        std::string key;
        Value value;
    };
    Context& context;
    ContextPropertyBindings* properties=nullptr;
    adaptors::lite::LiteModule* lite=nullptr;
    std::weak_ptr<void> serviceOwner;
    ObjectRef constructor;
    std::unordered_map<const scene::Node*,ObjectRef> handles;
    std::unordered_map<const scene::Node*,Value> sourceIdentities;
    std::unordered_map<const scene::Node*,std::shared_ptr<adaptors::lite::LiteNodeOwnerView>> sourceViews;
    std::function<scene::NodeRef(const Value&)> sourceChildResolver;
    std::vector<PresentationWrite> presentationWrites;
    std::vector<const scene::Node*> requestNodes;
    std::unordered_set<const scene::Node*> requestNodeSet;
    std::uint64_t requestGeneration=0;
    bool requestScopeActive=false;
    State(Context& cx,std::u16string name):context(cx){
        constructor=context.nativeConstructor(std::move(name),
            [](Context&,std::span<const Value>)->ObjectRef{
                throw Exception("TypeError",u"Scene node handles are internal values");
            });
    }
    static std::optional<std::size_t> rectIndex(std::string_view key){
        if(key=="x")return 0;if(key=="y")return 1;if(key=="width")return 2;if(key=="height")return 3;return {};
    }
    static bool geometryField(const scene::Node& node,std::string_view key){
        if(std::holds_alternative<scene::RectGeometry>(node.geometry))
            return rectIndex(key).has_value()||key=="rx"||key=="ry";
        if(std::holds_alternative<scene::ViewportGeometry>(node.geometry))
            return key=="x"||key=="y"||key=="width"||key=="height"||key=="viewBox"||
                key=="preserveAspectRatio"||key=="overflow";
        if(std::holds_alternative<scene::LineGeometry>(node.geometry))
            return key=="x1"||key=="y1"||key=="x2"||key=="y2"||key=="stroke-width";
        if(std::holds_alternative<scene::PolygonGeometry>(node.geometry))return key=="points";
        if(std::holds_alternative<scene::EllipseGeometry>(node.geometry))return key=="cx"||key=="cy"||key=="rx"||key=="ry";
        if(std::holds_alternative<scene::PathGeometry>(node.geometry))return key=="d";
        if(std::holds_alternative<scene::UseGeometry>(node.geometry))return key=="x"||key=="y"||key=="href"||key=="xlink:href";
        if(std::holds_alternative<scene::ForeignObjectGeometry>(node.geometry))
            return key=="x"||key=="y"||key=="width"||key=="height"||key=="transform";
        return key=="transform";
    }
    static bool paintField(const scene::Node& node,std::string_view key){
        if(key=="stroke-width"&&std::holds_alternative<scene::LineGeometry>(node.geometry))return false;
        return key=="fill"||key=="stroke"||key=="stroke-width"||key=="stroke-linecap"||
            key=="stroke-linejoin"||key=="stroke-dasharray"||key=="stroke-dashoffset";
    }
    static void ensureGeometry(scene::Node& node){
        if(!std::holds_alternative<std::monostate>(node.geometry))return;
        if(node.kind==u"rect")node.geometry=scene::RectGeometry{};
        else if(node.kind==u"svg")node.geometry=scene::ViewportGeometry{};
        else if(node.kind==u"line")node.geometry=scene::LineGeometry{};
        else if(node.kind==u"polygon"||node.kind==u"polyline")node.geometry=scene::PolygonGeometry{};
        else if(node.kind==u"ellipse")node.geometry=scene::EllipseGeometry{};
        else if(node.kind==u"path")node.geometry=scene::PathGeometry{};
        else if(node.kind==u"use")node.geometry=scene::UseGeometry{};
        else if(node.kind==u"foreignObject")node.geometry=scene::ForeignObjectGeometry{};
    }
    static std::string propertyString(const Property& property){
        return property_host::debugText(property_host::string(property));
    }
    static std::string transformText(const scene::Transform& transform){
        std::string out;
        auto add=[&](std::string value){if(!out.empty())out.push_back(' ');out+=std::move(value);};
        auto num=[](double x){return propertyString(Property(x));};
        for(const auto& op:transform.operations){
            switch(op.kind){
            case scene::TransformKind::Translate:add("translate("+num(op.x*1000)+(op.spaceSeparated?" ":",")+num(op.y*1000)+")");break;
            case scene::TransformKind::Scale:add("scale("+num(op.x)+(op.scalarScale?std::string{}:(op.spaceSeparated?" ":",")+num(op.y))+")");break;
            case scene::TransformKind::Rotate:add("rotate("+num(op.degrees)+(op.centerX||op.centerY?" "+num(op.centerX*1000)+" "+num(op.centerY*1000):std::string{})+")");break;
            case scene::TransformKind::Matrix:add("matrix("+num(op.a)+" "+num(op.b)+" "+num(op.c)+" "+num(op.d)+" "+num(op.e*1000)+" "+num(op.f*1000)+")");break;
            }
        }
        return out;
    }
    static Property geometryValue(const scene::Node& node,std::string_view key){
        auto number=[](double value){return Property(propertyString(Property(value*1000)));};
        if(const auto* g=std::get_if<scene::RectGeometry>(&node.geometry)){
            if(key=="x")return number(g->x);if(key=="y")return number(g->y);if(key=="width")return number(g->width);if(key=="height")return number(g->height);
            if(key=="rx"&&g->rx)return number(*g->rx);if(key=="ry"&&g->ry)return number(*g->ry);
        }else if(const auto* g=std::get_if<scene::ViewportGeometry>(&node.geometry)){
            if(key=="x")return number(g->x);if(key=="y")return number(g->y);
            if(key=="width")return g->widthRelative?Property(propertyString(Property(g->width*100))+"%"):number(g->width);
            if(key=="height")return g->heightRelative?Property(propertyString(Property(g->height*100))+"%"):number(g->height);
            if(key=="viewBox")return Property(propertyString(Property(g->viewX*1000))+" "+propertyString(Property(g->viewY*1000))+" "+propertyString(Property(g->viewWidth*1000))+" "+propertyString(Property(g->viewHeight*1000)));
            if(key=="preserveAspectRatio"){
                static constexpr std::array<std::string_view,10> names={"none","xMinYMin","xMidYMin","xMaxYMin","xMinYMid","xMidYMid","xMaxYMid","xMinYMax","xMidYMax","xMaxYMax"};
                auto result=std::string(names[static_cast<std::size_t>(g->alignment)]);
                if(g->alignment!=scene::ViewportAlignment::None&&g->emitFit)
                    result+=g->fit==scene::ViewportFit::Meet?" meet":" slice";
                return Property(result);
            }
            if(key=="overflow")return Property(g->clipToViewport?"hidden":"visible");
        }else if(const auto* g=std::get_if<scene::LineGeometry>(&node.geometry)){
            if(key=="x1")return number(g->from.x);if(key=="y1")return number(g->from.y);if(key=="x2")return number(g->to.x);if(key=="y2")return number(g->to.y);if(key=="stroke-width")return number(g->width);
        }else if(const auto* g=std::get_if<scene::EllipseGeometry>(&node.geometry)){
            if(key=="cx")return number(g->cx);if(key=="cy")return number(g->cy);if(key=="rx")return number(g->rx);if(key=="ry")return number(g->ry);
        }else if(const auto* g=std::get_if<scene::UseGeometry>(&node.geometry)){
            if(key=="x")return number(g->x);if(key=="y")return number(g->y);if(key=="href"||key=="xlink:href")return Property(g->href);
        }else if(const auto* g=std::get_if<scene::ForeignObjectGeometry>(&node.geometry)){
            if(key=="x")return Property(propertyString(Property(g->x))+"px");if(key=="y")return Property(propertyString(Property(g->y))+"px");
            if(key=="width")return Property(propertyString(Property(g->width))+"px");if(key=="height")return Property(propertyString(Property(g->height))+"px");
            if(key=="transform")return Property(transformText(g->transform));
        }
        if(key=="transform"&&node.transform)return Property(transformText(*node.transform));
        if(const auto* g=std::get_if<scene::PolygonGeometry>(&node.geometry);g&&key=="points"){
            std::string out;for(const auto& p:g->points){if(!out.empty())out.push_back(' ');out+=propertyString(Property(p.x*1000))+","+propertyString(Property(p.y*1000));}return Property(out);
        }
        if(const auto* g=std::get_if<scene::PathGeometry>(&node.geometry);g&&key=="d"&&g->outline)return Property(g->outline->sourcePath);
        return Missing{};
    }
    static Property paintValue(const scene::Node& node,std::string_view key){
        if(!node.paint)return Missing{};
        const auto& paint=*node.paint;
        if(key=="fill"&&paint.fill)return Property(*paint.fill);
        if(key=="stroke"&&paint.stroke)return Property(*paint.stroke);
        if(key=="stroke-width"&&paint.strokeWidth)return Property(propertyString(Property(*paint.strokeWidth*1000)));
        if(key=="stroke-linecap"&&paint.lineCap){
            const auto value=*paint.lineCap==scene::LineCap::Butt?"butt":*paint.lineCap==scene::LineCap::Round?"round":"square";
            return Property(value);
        }
        if(key=="stroke-linejoin"&&paint.lineJoin){
            const auto value=*paint.lineJoin==scene::LineJoin::Miter?"miter":*paint.lineJoin==scene::LineJoin::Round?"round":"bevel";
            return Property(value);
        }
        if(key=="stroke-dasharray"&&paint.dash){
            if(paint.dash->lengths.empty())return Property("none");
            std::string result;
            for(const auto length:paint.dash->lengths){if(!result.empty())result+=paint.dash->separator;result+=propertyString(Property(length*1000));}
            return Property(result);
        }
        if(key=="stroke-dashoffset"&&paint.dash)return Property(propertyString(Property(paint.dash->offset*1000)));
        return Missing{};
    }
    static double parseNumber(std::u16string_view text,bool pixels=false){
        if(pixels&&text.size()>=2&&text.substr(text.size()-2)==u"px")text.remove_suffix(2);
        const auto utf8=property_host::debugText(text);std::size_t consumed=0;double result;
        try{result=std::stod(utf8,&consumed);}catch(...){throw Exception("TypeError",u"Invalid SVG geometry number");}
        if(consumed!=utf8.size()||!std::isfinite(result))throw Exception("TypeError",u"Invalid SVG geometry number");
        return result;
    }
    static double parsePercent(std::u16string_view text){
        if(text.empty()||text.back()!=u'%')throw Exception("TypeError",u"Expected an SVG percentage");
        text.remove_suffix(1);
        const auto utf8=property_host::debugText(text);std::size_t consumed=0;double result;
        try{result=std::stod(utf8,&consumed);}catch(...){throw Exception("TypeError",u"Invalid SVG percentage");}
        if(consumed!=utf8.size()||!std::isfinite(result))throw Exception("TypeError",u"Invalid SVG percentage");
        return result/100;
    }
    static std::vector<double> numbers(std::u16string_view text){
        auto value=property_host::debugText(text);for(auto& c:value)if(c==',')c=' ';
        std::istringstream input(value);std::vector<double> out;double n;
        while(input>>n)out.push_back(n);
        if(!input.eof())throw Exception("TypeError",u"Invalid SVG geometry list");return out;
    }
    static void setGeometry(scene::Node& node,std::string_view key,std::u16string_view text){
        ensureGeometry(node);
        if(key=="transform"){
            scene::Transform value;std::size_t at=0;
            while(at<text.size()){
                while(at<text.size()&&(text[at]==u' '||text[at]==u'\t'||text[at]==u','))++at;
                if(at==text.size())break;const auto open=text.find(u'(',at);if(open==std::u16string_view::npos)throw Exception("TypeError",u"Invalid SVG transform");
                const auto close=text.find(u')',open+1);if(close==std::u16string_view::npos)throw Exception("TypeError",u"Invalid SVG transform");
                const auto name=property_host::debugText(text.substr(at,open-at));const auto args=numbers(text.substr(open+1,close-open-1));scene::TransformOperation op;
                if(name=="translate"&&args.size()>=1&&args.size()<=2){op.kind=scene::TransformKind::Translate;op.x=args[0]/1000;op.y=(args.size()==2?args[1]:0)/1000;}
                else if(name=="scale"&&args.size()>=1&&args.size()<=2){op.kind=scene::TransformKind::Scale;op.x=args[0];op.y=args.size()==2?args[1]:args[0];op.scalarScale=args.size()==1;}
                else if(name=="rotate"&&(args.size()==1||args.size()==3)){op.kind=scene::TransformKind::Rotate;op.degrees=args[0];if(args.size()==3){op.centerX=args[1]/1000;op.centerY=args[2]/1000;}}
                else if(name=="matrix"&&args.size()==6){op.kind=scene::TransformKind::Matrix;op.a=args[0];op.b=args[1];op.c=args[2];op.d=args[3];op.e=args[4]/1000;op.f=args[5]/1000;}
                else throw Exception("TypeError",u"Unsupported SVG transform operation");
                value.operations.push_back(op);at=close+1;
            }
            if(auto* foreign=std::get_if<scene::ForeignObjectGeometry>(&node.geometry))foreign->transform=value;else node.transform=value;
            return;
        }
        auto n=[&](bool px=false){return parseNumber(text,px);};
        if(auto* g=std::get_if<scene::RectGeometry>(&node.geometry)){
            if(auto i=rectIndex(key)){const auto value=n()/1000;switch(*i){case 0:g->x=value;break;case 1:g->y=value;break;case 2:g->width=value;break;case 3:g->height=value;break;}if(std::find(g->fields.begin(),g->fields.begin()+g->fieldCount,static_cast<scene::RectField>(*i))==g->fields.begin()+g->fieldCount&&g->fieldCount<g->fields.size())g->fields[g->fieldCount++]=static_cast<scene::RectField>(*i);}
            else if(key=="rx")g->rx=n()/1000;else if(key=="ry")g->ry=n()/1000;
        }else if(auto* g=std::get_if<scene::ViewportGeometry>(&node.geometry)){
            if(key=="x")g->x=n()/1000;else if(key=="y")g->y=n()/1000;
            else if(key=="width"){
                if(text.ends_with(u"%")){g->width=parsePercent(text);g->widthRelative=true;}
                else {g->width=n()/1000;g->widthRelative=false;}
            }else if(key=="height"){
                if(text.ends_with(u"%")){g->height=parsePercent(text);g->heightRelative=true;}
                else {g->height=n()/1000;g->heightRelative=false;}
            }
            else if(key=="viewBox"){const auto v=numbers(text);if(v.size()!=4)throw Exception("TypeError",u"viewBox requires four numbers");g->viewX=v[0]/1000;g->viewY=v[1]/1000;g->viewWidth=v[2]/1000;g->viewHeight=v[3]/1000;g->hasViewBox=true;}
            else if(key=="preserveAspectRatio"){
                std::istringstream input(property_host::debugText(text));
                std::vector<std::string> fields;std::string field;while(input>>field)fields.push_back(field);
                if(!fields.empty()&&fields.front()=="defer")fields.erase(fields.begin());
                const auto alignment=fields.empty()?std::string("xMidYMid"):fields[0];
                static const std::unordered_map<std::string,scene::ViewportAlignment> alignments={
                    {"none",scene::ViewportAlignment::None},{"xMinYMin",scene::ViewportAlignment::XMinYMin},
                    {"xMidYMin",scene::ViewportAlignment::XMidYMin},{"xMaxYMin",scene::ViewportAlignment::XMaxYMin},
                    {"xMinYMid",scene::ViewportAlignment::XMinYMid},{"xMidYMid",scene::ViewportAlignment::XMidYMid},
                    {"xMaxYMid",scene::ViewportAlignment::XMaxYMid},{"xMinYMax",scene::ViewportAlignment::XMinYMax},
                    {"xMidYMax",scene::ViewportAlignment::XMidYMax},{"xMaxYMax",scene::ViewportAlignment::XMaxYMax}};
                const auto found=alignments.find(alignment);
                if(found==alignments.end()||fields.size()>2)
                    throw Exception("TypeError",u"Invalid preserveAspectRatio value");
                g->alignment=found->second;
                if(fields.size()==2){
                    if(fields[1]=="meet")g->fit=scene::ViewportFit::Meet;
                    else if(fields[1]=="slice")g->fit=scene::ViewportFit::Slice;
                    else throw Exception("TypeError",u"Invalid preserveAspectRatio fit");
                    g->emitFit=true;
                }else g->emitFit=false;
            }else if(key=="overflow")g->clipToViewport=property_host::debugText(text)!="visible";
        }else if(auto* g=std::get_if<scene::LineGeometry>(&node.geometry)){
            if(key=="x1")g->from.x=n()/1000;else if(key=="y1")g->from.y=n()/1000;else if(key=="x2")g->to.x=n()/1000;else if(key=="y2")g->to.y=n()/1000;else if(key=="stroke-width")g->width=n()/1000;
        }else if(auto* g=std::get_if<scene::EllipseGeometry>(&node.geometry)){
            if(key=="cx")g->cx=n()/1000;else if(key=="cy")g->cy=n()/1000;else if(key=="rx")g->rx=n()/1000;else if(key=="ry")g->ry=n()/1000;
        }else if(auto* g=std::get_if<scene::UseGeometry>(&node.geometry)){
            if(key=="x")g->x=n()/1000;else if(key=="y")g->y=n()/1000;
            else if(key=="href"||key=="xlink:href"){
                g->href=property_host::debugText(text);
                g->xlink=key=="xlink:href";
            }
        }else if(auto* g=std::get_if<scene::ForeignObjectGeometry>(&node.geometry)){
            if(key=="x")g->x=n(true);else if(key=="y")g->y=n(true);else if(key=="width")g->width=n(true);else if(key=="height")g->height=n(true);
        }else if(auto* g=std::get_if<scene::PolygonGeometry>(&node.geometry);g&&key=="points"){
            const auto v=numbers(text);if(v.size()%2)throw Exception("TypeError",u"points requires coordinate pairs");g->points.clear();for(std::size_t i=0;i<v.size();i+=2)g->points.push_back({v[i]/1000,v[i+1]/1000});
        }else if(auto* g=std::get_if<scene::PathGeometry>(&node.geometry);g&&key=="d")g->outline=scene::decodeGlyphOutline(property_host::debugText(text));
    }
    static void setPaint(scene::Node& node,std::string_view key,std::u16string_view text){
        if(!node.paint)node.paint.emplace();
        auto& paint=*node.paint;
        if(key=="fill")paint.fill=property_host::debugText(text);
        else if(key=="stroke")paint.stroke=property_host::debugText(text);
        else if(key=="stroke-width")paint.strokeWidth=parseNumber(text)/1000;
        else if(key=="stroke-linecap"){
            if(text==u"butt")paint.lineCap=scene::LineCap::Butt;
            else if(text==u"round")paint.lineCap=scene::LineCap::Round;
            else if(text==u"square")paint.lineCap=scene::LineCap::Square;
            else throw Exception("TypeError",u"Invalid SVG stroke-linecap");
        }else if(key=="stroke-linejoin"){
            if(text==u"miter"||text==u"mitre")paint.lineJoin=scene::LineJoin::Miter;
            else if(text==u"round")paint.lineJoin=scene::LineJoin::Round;
            else if(text==u"bevel")paint.lineJoin=scene::LineJoin::Bevel;
            else throw Exception("TypeError",u"Invalid SVG stroke-linejoin");
        }else if(key=="stroke-dasharray"){
            if(text==u"none")paint.dash=scene::StrokeDash{};
            else {const auto values=numbers(text);scene::StrokeDash dash;dash.lengths.reserve(values.size());
                for(const auto value:values)dash.lengths.push_back(value/1000);
                const auto comma=text.find(u',');
                if(comma!=std::u16string_view::npos)
                    dash.separator=comma+1<text.size()&&(text[comma+1]==u' '||text[comma+1]==u'\t')?", ":",";
                paint.dash=std::move(dash);}
        }else if(key=="stroke-dashoffset"){
            const auto offset=parseNumber(text)/1000;
            if(!paint.dash)paint.dash=scene::StrokeDash{};
            paint.dash->offset=offset;
        }
    }
    static void appendForeignObjectPixelProjection(scene::Node& node){
        if(auto* foreign=std::get_if<scene::ForeignObjectGeometry>(&node.geometry)){
            // Source foreignObject transform attributes are expressed in SVG
            // user units, while its input coordinates are CSS pixels. Convert
            // the complete parsed transform's input basis once at the end.
            constexpr double svgUserUnitInEm=1.0/1000.0;
            foreign->transform.operations.push_back({
                .kind=scene::TransformKind::Scale,.x=svgUserUnitInEm,
                .y=svgUserUnitInEm,.scalarScale=true});
        }
    }
    static void setAttributes(scene::Node& node,PropertyList attributes){
        auto updated=node;
        const auto* priorForeign=std::get_if<scene::ForeignObjectGeometry>(&updated.geometry);
        const bool synthesizeForeignAttributes=!priorForeign||priorForeign->synthesizeMissingAttributes;
        if(std::holds_alternative<scene::RectGeometry>(updated.geometry))updated.geometry=scene::RectGeometry{};
        else if(std::holds_alternative<scene::ViewportGeometry>(updated.geometry))updated.geometry=scene::ViewportGeometry{};
        else if(std::holds_alternative<scene::LineGeometry>(updated.geometry))updated.geometry=scene::LineGeometry{};
        else if(std::holds_alternative<scene::PolygonGeometry>(updated.geometry))updated.geometry=scene::PolygonGeometry{};
        else if(std::holds_alternative<scene::PathGeometry>(updated.geometry))updated.geometry=scene::PathGeometry{};
        else if(std::holds_alternative<scene::UseGeometry>(updated.geometry))updated.geometry=scene::UseGeometry{};
        else if(std::holds_alternative<scene::ForeignObjectGeometry>(updated.geometry))
            updated.geometry=scene::ForeignObjectGeometry{.synthesizeMissingAttributes=synthesizeForeignAttributes};
        else if(std::holds_alternative<scene::EllipseGeometry>(updated.geometry))updated.geometry=scene::EllipseGeometry{};
        updated.paint.reset();
        updated.transform.reset();
        ensureGeometry(updated);
        PropertyList projected;
        for(const auto& key:attributes.keys()){
            const auto value=attributes.getOwn(key);
            if(geometryField(updated,key)){
                setGeometry(updated,key,property_host::string(value));
                // Keep the original SVG spelling and key order as serialization
                // metadata. Reads below come from the typed geometry slot.
                projected.setOwn(key,value);
            }else if(key=="style"){
                if(const auto record=PropertyList::record(value)){
                    styles::Styles owned;
                    for(const auto& styleKey:record->keys()){
                        const auto name=property_host::text(styleKey);
                        std::u16string cssName;
                        for(std::size_t index=0;index<name.size();++index){
                            if(name[index]>=u'A'&&name[index]<=u'Z'){
                                cssName.push_back(u'-');
                                cssName.push_back(static_cast<char16_t>(name[index]+(u'a'-u'A')));
                            }else cssName.push_back(name[index]);
                        }
                        owned.set(cssName,styles::StyleValue(property_host::string(record->get(styleKey))));
                    }
                    projected.setOwn(key,Property(property_host::debugText(owned.cssText())));
                }else projected.setOwn(key,value);
            }else if(paintField(updated,key)){
                setPaint(updated,key,property_host::string(value));
                projected.setOwn(key,value);
            }else projected.setOwn(key,value);
        }
        appendForeignObjectPixelProjection(updated);
        node.geometry=std::move(updated.geometry);
        node.paint=std::move(updated.paint);
        node.transform=std::move(updated.transform);
        node.attributes=std::move(projected);
        refreshRectFields(node);
    }
    static void refreshRectFields(scene::Node& node){
        auto* rect=std::get_if<scene::RectGeometry>(&node.geometry);if(!rect)return;
        rect->fieldCount=0;
        for(const auto& key:node.attributes.keys())if(const auto index=rectIndex(key)){
            const auto field=static_cast<scene::RectField>(*index);
            if(std::find(rect->fields.begin(),rect->fields.begin()+rect->fieldCount,field)==rect->fields.begin()+rect->fieldCount&&rect->fieldCount<rect->fields.size())
                rect->fields[rect->fieldCount++]=field;
        }
    }
    static void normalizeGeometryAttributes(scene::Node& node){
        const bool importGeometry=std::holds_alternative<std::monostate>(node.geometry);
        ensureGeometry(node);
        // Scene nodes created from a Source attribute record may not have had
        // typed geometry populated yet. Import each value once into the typed
        // slot while retaining its spelling and insertion order for output.
        for(const auto& key:node.attributes.keys())if(geometryField(node,key)){
            bool import=importGeometry;
            if(key=="transform"){
                if(const auto* foreign=std::get_if<scene::ForeignObjectGeometry>(&node.geometry))
                    import=foreign->transform.operations.empty();
                else import=!node.transform.has_value();
            }
            if(!import)continue;
            const auto value=node.attributes.getOwn(key);
            const auto spelling=property_host::string(value);
            if(!spelling.empty()||key=="d"||key=="transform")setGeometry(node,key,spelling);
        }
        for(const auto& key:node.attributes.keys())if(paintField(node,key)&&paintValue(node,key).missing())
            setPaint(node,key,property_host::string(node.attributes.getOwn(key)));
        if(importGeometry)appendForeignObjectPixelProjection(node);
        refreshRectFields(node);
    }
    static std::u16string attributeString(Context& cx,const Value& value){
        const auto primitive=cx.toPrimitive(value,PrimitiveHint::stringHint);
        if(const auto* text=std::get_if<std::u16string>(&primitive.data))return *text;
        if(const auto* number=std::get_if<double>(&primitive.data))return property_host::string(Property(*number));
        if(const auto* boolean=std::get_if<bool>(&primitive.data))return *boolean?u"true":u"false";
        if(std::holds_alternative<Null>(primitive.data))return u"null";
        if(std::holds_alternative<Undefined>(primitive.data))return u"undefined";
        throw Exception("TypeError",u"Cannot convert Symbol to string");
    }
    struct AttributesView final:NativeRecordView {
        std::shared_ptr<State> state;
        scene::NodeRef node;
        AttributesView(std::shared_ptr<State> owner,scene::NodeRef value)
            :state(std::move(owner)),node(std::move(value)){}
        static std::string keyText(const Key& key){
            const auto* text=std::get_if<std::u16string>(&key.data);
            if(!text)throw Exception("Unimplemented",u"SVG attributes do not support Symbol keys");
            return property_host::debugText(*text);
        }
        std::optional<Descriptor> ownDescriptor(const Key& key)const override{
            const auto lease=state->requireServices();
            if(!std::holds_alternative<std::u16string>(key.data))return {};
            const auto name=keyText(key);
            if(!node->attributes.hasOwn(name))return {};
            Property value=node->attributes.getOwn(name);
            if(geometryField(*node,name)||paintField(*node,name)){
                const auto geometry=geometryValue(*node,name);
                if(!geometry.missing())value=geometry;
                else {const auto paint=paintValue(*node,name);if(!paint.missing())value=paint;}
            }
            return Descriptor::data(state->properties
                ?state->properties->to(value):Value(property_host::string(value)));
        }
        bool ownPropertyEnumerable(const Key& key)const override{
            const auto lease=state->requireServices();
            if(!std::holds_alternative<std::u16string>(key.data))return false;
            return node->attributes.hasOwn(keyText(key));
        }
        std::vector<Key> ownKeys()const override{
            const auto lease=state->requireServices();
            std::vector<Key> result;
            const auto names=node->attributes.keys();result.reserve(names.size());
            for(const auto& name:names)result.emplace_back(property_host::text(name));
            return result;
        }
        void defineOwn(const Key& key,Descriptor descriptor)override{
            const auto lease=state->requireServices();
            const auto name=keyText(key);
            if(descriptor.accessor)throw Exception("Unimplemented",u"SVG attributes do not retain accessor descriptors");
            if(!descriptor.writable||!descriptor.enumerable||!descriptor.configurable)
                throw Exception("Unimplemented",u"SVG attributes do not retain descriptor flags");
            if(geometryField(*node,name)||paintField(*node,name)){
                const auto text=attributeString(state->context,descriptor.value);
                if(geometryField(*node,name))setGeometry(*node,name,text);
                else setPaint(*node,name,text);
                node->attributes.setOwn(name,Property(property_host::debugText(text)));
                refreshRectFields(*node);
                return;
            }
            if(!state->properties)throw Exception("Unimplemented",u"Scene node properties are not bound");
            auto property=state->properties->from(std::move(descriptor.value));
            if(name=="style")if(const auto record=PropertyList::record(property)){
                styles::Styles owned;
                for(const auto& styleKey:record->keys()){
                    const auto styleName=property_host::text(styleKey);
                    std::u16string cssName;
                    for(std::size_t index=0;index<styleName.size();++index){
                        if(styleName[index]>=u'A'&&styleName[index]<=u'Z'){
                            cssName.push_back(u'-');
                            cssName.push_back(static_cast<char16_t>(styleName[index]+(u'a'-u'A')));
                        }else cssName.push_back(styleName[index]);
                    }
                    owned.set(cssName,styles::StyleValue(property_host::string(record->get(styleKey))));
                }
                property=Property(property_host::debugText(owned.cssText()));
            }
            node->attributes.setOwn(name,std::move(property));
            if(name=="style")state->payloadForNode(node)->styles.reset();
        }
        void deleteOwn(const Key& key)override{
            const auto lease=state->requireServices();
            const auto name=keyText(key);
            if(!node->attributes.hasOwn(name))return;
            if(geometryField(*node,name)){
                if(name=="transform"){
                    if(auto* foreign=std::get_if<scene::ForeignObjectGeometry>(&node->geometry))foreign->transform={};
                    else node->transform.reset();
                }else if(auto* g=std::get_if<scene::RectGeometry>(&node->geometry)){
                    if(name=="x")g->x=0;else if(name=="y")g->y=0;
                    else if(name=="width")g->width=0;else if(name=="height")g->height=0;
                    else if(name=="rx")g->rx.reset();else if(name=="ry")g->ry.reset();
                }else if(auto* g=std::get_if<scene::ViewportGeometry>(&node->geometry)){
                    if(name=="x")g->x=0;else if(name=="y")g->y=0;
                    else if(name=="width"){g->width=0;g->widthRelative=false;}
                    else if(name=="height"){g->height=0;g->heightRelative=false;}
                    else if(name=="viewBox")g->hasViewBox=false;
                    else if(name=="preserveAspectRatio"){g->alignment=scene::ViewportAlignment::XMidYMid;g->fit=scene::ViewportFit::Meet;g->emitFit=false;}
                    else if(name=="overflow")g->clipToViewport=true;
                }else if(auto* g=std::get_if<scene::LineGeometry>(&node->geometry)){
                    if(name=="x1")g->from.x=0;else if(name=="y1")g->from.y=0;
                    else if(name=="x2")g->to.x=0;else if(name=="y2")g->to.y=0;
                    else if(name=="stroke-width"){g->width=0;g->emitStrokeWidth=false;}
                }else if(auto* g=std::get_if<scene::EllipseGeometry>(&node->geometry)){
                    if(name=="cx")g->cx=0;else if(name=="cy")g->cy=0;
                    else if(name=="rx")g->rx=0;else if(name=="ry")g->ry=0;
                }else if(auto* g=std::get_if<scene::UseGeometry>(&node->geometry)){
                    if(name=="x")g->x=0;else if(name=="y")g->y=0;
                    else {
                        const auto other=name=="href"?"xlink:href":"href";
                        if(node->attributes.hasOwn(other))setGeometry(*node,other,
                            property_host::string(node->attributes.getOwn(other)));
                        else {g->href.clear();g->xlink=false;}
                    }
                }else if(auto* g=std::get_if<scene::ForeignObjectGeometry>(&node->geometry)){
                    if(name=="x")g->x=0;else if(name=="y")g->y=0;
                    else if(name=="width")g->width=0;else if(name=="height")g->height=0;
                    else if(name=="transform")g->transform={};
                }else if(auto* g=std::get_if<scene::PolygonGeometry>(&node->geometry))g->points.clear();
                else if(auto* g=std::get_if<scene::PathGeometry>(&node->geometry);g&&name=="d")g->outline.reset();
            }else if(paintField(*node,name)&&node->paint){
                auto& paint=*node->paint;
                if(name=="fill")paint.fill.reset();
                else if(name=="stroke")paint.stroke.reset();
                else if(name=="stroke-width")paint.strokeWidth.reset();
                else if(name=="stroke-linecap")paint.lineCap.reset();
                else if(name=="stroke-linejoin")paint.lineJoin.reset();
                else if(name=="stroke-dasharray")paint.dash.reset();
                else if(name=="stroke-dashoffset"&&paint.dash)paint.dash->offset=0;
            }
            node->attributes.erase(name);
            if(name=="style")if(auto* payload=state->payloadForNode(node))payload->styles.reset();
            refreshRectFields(*node);
        }
        std::optional<ObjectRef> prototype()const override{return state->context.intrinsics().objectPrototype;}
        void setPrototype(std::optional<ObjectRef>)override{}
    };
    ObjectRef attributesObject(Payload& payload){
        if(payload.attributes)return *payload.attributes;
        payload.attributes=context.nativeRecord(
            std::make_shared<AttributesView>(shared_from_this(),payload.node),
            context.intrinsics().objectPrototype);
        return *payload.attributes;
    }
    std::optional<ObjectRef> parentOf(const scene::NodeRef& node){
        if(const auto identity=sourceIdentity(node)){
            const auto parent=context.get(*identity,Key(u"parent"));
            if(const auto* object=std::get_if<ObjectRef>(&parent.data))
                if(std::any_cast<Payload>(&std::as_const(context).payload(*object)))return *object;
            if(const auto source=sourceNode(parent))
                if(const auto found=handles.find(source->get());found!=handles.end())return found->second;
            return {};
        }
        const auto parent=node->parent.lock();
        return parent?std::optional<ObjectRef>{handle(parent)}:std::optional<ObjectRef>{};
    }
    std::optional<scene::NodeRef> sourceNode(const Value& value)const{
        for(const auto& [node,identity]:sourceIdentities)if(identity==value){
            const auto found=handles.find(node);
            if(found!=handles.end())return payloadReadOnly(found->second).node;
        }
        return {};
    }
    std::optional<scene::NodeRef> findProjectedNode(const Value& value)const{
        if(const auto* object=std::get_if<ObjectRef>(&value.data))
            for(const auto& [node,handle]:handles)
                if(handle==*object)return payloadReadOnly(handle).node;
        return sourceNode(value);
    }
    std::optional<Value> sourceIdentity(const scene::NodeRef& node)const{
        const auto found=sourceIdentities.find(node.get());
        if(found==sourceIdentities.end())return {};
        return found->second;
    }
    struct ContextChildOwner final:scene::ChildOwner {
        std::weak_ptr<State> state;
        std::weak_ptr<scene::Node> node;
        Value raw;
        ContextChildOwner(std::weak_ptr<State> owner,std::weak_ptr<scene::Node> target,Value value)
            :state(std::move(owner)),node(std::move(target)),raw(std::move(value)){}
        State::ServiceAccess owner()const{
            auto current=state.lock();
            if(!current)throw Exception("TypeError",u"Scene child owner has expired");
            return {current,current->requireServices()};
        }
        static std::size_t toLength(Context& context,const Value& value){
            const double number=context.toNumber(value);
            if(std::isnan(number)||number<=0)return 0;
            constexpr double maxSafe=9007199254740991.0;
            const double length=std::min(std::floor(number),maxSafe);
            return length>=static_cast<double>(std::numeric_limits<std::size_t>::max())
                ?std::numeric_limits<std::size_t>::max():static_cast<std::size_t>(length);
        }
        Value currentValue()const{
            auto access=owner();
            const auto target=node.lock();
            if(!target)throw Exception("TypeError",u"Scene child owner node has expired");
            if(const auto identity=access->sourceIdentity(target))
                return access->context.get(*identity,Key(u"children"));
            const auto found=access->handles.find(target.get());
            if(found==access->handles.end())return raw;
            return access->context.get(Value(found->second),Key(u"children"));
        }
        Value valueFor(const scene::NodeRef& node)const{
            auto current=owner();
            if(const auto identity=current->sourceIdentity(node))return *identity;
            return Value(current->handle(node));
        }
        std::vector<Value> values()const{
            const auto current=owner();
            const auto source=currentValue();
            // Source runs __read first, then __spreadArray copies indexed
            // properties. `has` deliberately sees inherited array slots.
            const auto sequence=current->context.readIterable(source);
            const auto count=toLength(current->context,current->context.get(sequence,Key(u"length")));
            std::vector<Value> result;result.reserve(count);
            for(std::size_t index=0;index<count;++index){
                const Key key(property_host::text(std::to_string(index)));
                if(current->context.has(sequence,key))result.push_back(current->context.get(sequence,key));
                else result.push_back(Value(Undefined{}));
            }
            return result;
        }
        std::size_t size()const override{
            auto current=owner();
            const auto target=currentValue();
            return toLength(current->context,current->context.get(target,Key(u"length")));
        }
        bool hasSlot(std::size_t index)const override{
            auto current=owner();
            return current->context.hasOwn(currentValue(),Key(property_host::text(std::to_string(index))));
        }
        std::vector<scene::NodeRef> snapshot()const override{
            const auto current=owner();
            const auto sourceValues=values();
            std::vector<scene::NodeRef> result;result.reserve(sourceValues.size());
            for(const auto& source:sourceValues){
                if(std::holds_alternative<Undefined>(source.data))
                    throw Exception("TypeError",u"Cannot read properties of undefined (reading 'kind')");
                if(const auto mapped=current->sourceNode(source)){result.push_back(*mapped);continue;}
                if(const auto* object=std::get_if<ObjectRef>(&source.data))
                    if(const auto* payload=std::any_cast<Payload>(&std::as_const(current->context).payload(*object))){result.push_back(payload->node);continue;}
                if(current->sourceChildResolver){const auto node=current->sourceChildResolver(source);if(node){result.push_back(node);continue;}}
                throw Exception("TypeError",u"Source child has no canonical Scene mapping");
            }
            return result;
        }
        scene::NodeRef at(std::size_t index)const override{
            const auto current=owner();
            const auto source=current->context.get(currentValue(),Key(property_host::text(std::to_string(index))));
            if(std::holds_alternative<Undefined>(source.data))return {};
            if(const auto mapped=current->sourceNode(source))return *mapped;
            if(const auto* object=std::get_if<ObjectRef>(&source.data))
                if(const auto* payload=std::any_cast<Payload>(&std::as_const(current->context).payload(*object)))return payload->node;
            if(current->sourceChildResolver){
                const auto node=current->sourceChildResolver(source);
                if(node)return node;
            }
            throw Exception("TypeError",u"Source child has no canonical Scene mapping");
        }
        void set(std::size_t index,scene::NodeRef node)override{
            auto current=owner();current->context.set(currentValue(),Key(property_host::text(std::to_string(index))),valueFor(node));
        }
        void append(scene::NodeRef node)override{
            auto current=owner();const auto target=currentValue();
            const auto push=current->context.get(target,Key(u"push"));
            const std::array<Value,1> args{valueFor(node)};
            (void)current->context.callMember(push,target,args,u"children.push");
        }
        void insert(std::size_t index,scene::NodeRef node)override{
            auto current=owner();const auto receiver=currentValue();
            const auto splice=current->context.get(receiver,Key(u"splice"));
            const std::array<Value,3> args{Value(static_cast<double>(index)),Value(0),valueFor(node)};
            (void)current->context.callMember(splice,receiver,args,u"Array.prototype.splice");
        }
        void erase(std::size_t index)override{
            auto current=owner();const auto receiver=currentValue();
            const auto splice=current->context.get(receiver,Key(u"splice"));
            const std::array<Value,2> args{Value(static_cast<double>(index)),Value(1)};
            (void)current->context.callMember(splice,receiver,args,u"Array.prototype.splice");
        }
        void deleteSlot(std::size_t index)override{
            auto current=owner();
            const auto target=currentValue();
            const auto* object=std::get_if<ObjectRef>(&target.data);
            if(!object)throw Exception("TypeError",u"Cannot delete a property of a non-object children value");
            current->context.erase(*object,Key(property_host::text(std::to_string(index))));
        }
        void resize(std::size_t length)override{
            auto current=owner();current->context.set(currentValue(),Key(u"length"),Value(static_cast<double>(length)));
        }
    };
    struct SourceAttributeOwner final:scene::AttributeOwner {
        std::weak_ptr<State> state;
        Value raw;
        SourceAttributeOwner(std::weak_ptr<State> owner,Value value):state(std::move(owner)),raw(std::move(value)){}
        State::ServiceAccess owner()const{
            auto current=state.lock();
            if(!current)throw Exception("TypeError",u"Scene attribute owner has expired");
            auto lease=current->requireServices();
            if(!current->properties)throw Exception("TypeError",u"Scene property provider has expired");
            return {current,std::move(lease)};
        }
        Property property(std::string_view name,bool own)const{
            auto current=owner();
            if(own&&!current->context.hasOwn(raw,Key(property_host::text(name))))return Missing{};
            return current->properties->from(current->context.get(raw,Key(property_host::text(name))));
        }
        Property get(std::string_view name)const override{return property(name,false);}
        Property getOwn(std::string_view name)const override{return property(name,true);}
        bool hasOwn(std::string_view name)const override{
            auto current=owner();return current->context.hasOwn(raw,Key(property_host::text(name)));
        }
        void set(std::string_view name,Property value)override{
            auto current=owner();current->context.set(raw,Key(property_host::text(name)),current->properties->to(std::move(value)));
        }
        void setOwn(std::string_view name,Property value)override{set(name,std::move(value));}
        void erase(std::string_view name)override{
            const auto current=owner();
            if(auto list=PropertyList::record(current->properties->from(raw)))list->erase(name);
        }
        std::vector<std::string> keys()const override{
            auto current=owner();std::vector<std::string> result;
            for(const auto& key:current->context.enumerableStringKeys(raw))
                result.push_back(property_host::debugText(key));
            return result;
        }
    };
    struct SourceGeometryOwner final:scene::GeometryOwner {
        std::weak_ptr<State> state;
        std::weak_ptr<scene::Node> node;
        SourceGeometryOwner(std::weak_ptr<State> owner,std::weak_ptr<scene::Node> value)
            :state(std::move(owner)),node(std::move(value)){}
        std::shared_ptr<const scene::GeometryOwner> cloneFor(const scene::NodeRef& target)const override{
            if(!target)throw Exception("TypeError",u"Cannot bind geometry owner to a null Scene node");
            return std::make_shared<SourceGeometryOwner>(state,target);
        }
        void apply(scene::Geometry& geometry,std::optional<scene::Transform>& transform,
            std::span<const std::pair<std::string,std::string>> attributes)const override{
            auto current=state.lock();
            auto target=node.lock();
            if(!current||!target)throw Exception("TypeError",u"Scene geometry owner has expired");
            const auto lease=current->requireServices();
            scene::Node projection;projection.kind=target->kind;projection.geometry=geometry;projection.transform=transform;
            State::ensureGeometry(projection);
            const auto* priorForeign=std::get_if<scene::ForeignObjectGeometry>(&projection.geometry);
            const bool synthesizeForeignAttributes=!priorForeign||priorForeign->synthesizeMissingAttributes;
            if(std::holds_alternative<scene::ForeignObjectGeometry>(projection.geometry))
                projection.transform.reset();
            if(std::holds_alternative<scene::RectGeometry>(projection.geometry))projection.geometry=scene::RectGeometry{};
            else if(std::holds_alternative<scene::ViewportGeometry>(projection.geometry))projection.geometry=scene::ViewportGeometry{};
            else if(std::holds_alternative<scene::LineGeometry>(projection.geometry))projection.geometry=scene::LineGeometry{};
            else if(std::holds_alternative<scene::PolygonGeometry>(projection.geometry))projection.geometry=scene::PolygonGeometry{};
            else if(std::holds_alternative<scene::PathGeometry>(projection.geometry))projection.geometry=scene::PathGeometry{};
            else if(std::holds_alternative<scene::UseGeometry>(projection.geometry))projection.geometry=scene::UseGeometry{};
            else if(std::holds_alternative<scene::ForeignObjectGeometry>(projection.geometry))
                projection.geometry=scene::ForeignObjectGeometry{.synthesizeMissingAttributes=synthesizeForeignAttributes};
            else if(std::holds_alternative<scene::EllipseGeometry>(projection.geometry))projection.geometry=scene::EllipseGeometry{};
            State::ensureGeometry(projection);
            for(const auto& [key,value]:attributes)if(State::geometryField(projection,key)){
                State::setGeometry(projection,key,property_host::text(value));
            }
            State::appendForeignObjectPixelProjection(projection);
            geometry=std::move(projection.geometry);
            transform=std::move(projection.transform);
        }
    };
    struct SourcePaintOwner final:scene::PaintOwner {
        std::weak_ptr<scene::Node> node;
        explicit SourcePaintOwner(std::weak_ptr<scene::Node> value):node(std::move(value)){}
        std::shared_ptr<const scene::PaintOwner> cloneFor(const scene::NodeRef& target)const override{
            if(!target)throw Exception("TypeError",u"Cannot bind paint owner to a null Scene node");
            return std::make_shared<SourcePaintOwner>(target);
        }
        void apply(std::optional<scene::ShapePaint>& paint,const scene::Geometry& geometry,
            std::span<const std::pair<std::string,std::string>> attributes)const override{
            if(!node.lock())throw Exception("TypeError",u"Scene paint owner has expired");
            // emitAttributes=false denotes already-resolved CSS/layout paint;
            // preserve it. Source presentation attributes are otherwise
            // reconstructed from the exact strings captured by Scene::freeze.
            if(paint&&!paint->emitAttributes)return;
            scene::Node projection;
            projection.geometry=geometry;
            for(const auto& [key,value]:attributes)if(State::paintField(projection,key))
                State::setPaint(projection,key,property_host::text(value));
            paint=std::move(projection.paint);
        }
    };
    struct SourceTextValue final:scene::SourceTextOwner {
        std::weak_ptr<State> state;
        Property raw;
        SourceTextValue(std::weak_ptr<State> owner,Property value):state(std::move(owner)),raw(std::move(value)){}
        State::ServiceAccess owner()const{
            auto current=state.lock();
            if(!current)throw Exception("TypeError",u"Scene text owner has expired");
            return {current,current->requireServices()};
        }
        Property get()const{(void)owner();return raw;}
        void set(Property value){(void)owner();raw=std::move(value);}
        std::u16string text()const override{return property_host::string(get());}
    };
    std::shared_ptr<ContextChildOwner> sourceChildOwner(const scene::NodeRef& node)const{
        return std::dynamic_pointer_cast<ContextChildOwner>(node->children.owner());
    }
    std::shared_ptr<SourceAttributeOwner> sourceAttributeOwner(const scene::NodeRef& node)const{
        return std::dynamic_pointer_cast<SourceAttributeOwner>(node->attributes.owner());
    }
    Value childrenArray(Payload& payload){
        const auto serviceLease=requireServices();
        if(const auto source=sourceChildOwner(payload.node)){
            return source->raw;
        }
        const auto owner=payload.node->children.owner();
        const auto length=owner->size();
        if(length>std::numeric_limits<std::uint32_t>::max())
            throw Exception("RangeError",u"Invalid array length");
        const auto object=context.array(static_cast<std::uint32_t>(length));
        for(std::size_t index=0;index<length;++index){
            if(!owner->hasSlot(index))continue;
            const auto child=owner->at(index);
            if(!child){
                context.set(Value(object),Key(property_host::text(std::to_string(index))),Value(Undefined{}));
                continue;
            }
            const auto source=sourceIdentity(child);
            const auto value=source?*source:Value(handle(child));
            context.set(Value(object),Key(property_host::text(std::to_string(index))),value);
        }
        payload.node->children.replaceOwner(std::make_shared<ContextChildOwner>(shared_from_this(),payload.node,Value(object)));
        return Value(object);
    }
    std::shared_ptr<void> requireServices()const{
        auto lease=serviceOwner.lock();
        if(!lease)throw Exception("TypeError",u"Scene provider services have expired");
        return lease;
    }
    Payload& payload(ObjectRef handle)const{
        auto* value=std::any_cast<Payload>(&context.payload(handle));
        if(!value||!value->node)throw Exception("TypeError",u"Invalid Scene node handle");
        return *value;
    }
    const Payload& payloadReadOnly(ObjectRef handle)const{
        const auto* value=std::any_cast<Payload>(&std::as_const(context).payload(handle));
        if(!value||!value->node)throw Exception("TypeError",u"Invalid Scene node handle");
        return *value;
    }
    Payload* payloadForNode(const scene::NodeRef& node)const{
        const auto found=handles.find(node.get());
        if(found==handles.end())return nullptr;
        auto* value=std::any_cast<Payload>(&context.payload(found->second));
        return value&&value->node==node?value:nullptr;
    }
    bool isPresentationWrite(const scene::NodeRef& node,std::string_view key,const Value& value)const{
        if(presentationWrites.empty())return false;
        const auto& active=presentationWrites.back();
        return active.node==node.get()&&active.key==key&&active.value==value;
    }
    void trackRequestNode(const scene::Node* node){
        if(!requestScopeActive||!requestNodeSet.insert(node).second)return;
        requestNodes.push_back(node);
    }
    ObjectRef handle(const scene::NodeRef& node,std::optional<ObjectRef> owner={},
                     std::optional<ObjectRef> definition={}){
        if(!node)throw Exception("TypeError",u"Cannot project a null Scene node");
        trackRequestNode(node.get());
        if(const auto found=handles.find(node.get());found!=handles.end()){
            auto& value=payload(found->second);
            if(value.node!=node)throw Exception("TypeError",u"Scene node identity collision");
            if(owner)value.owner=owner;
            if(definition)value.definition=definition;
            return found->second;
        }
        normalizeGeometryAttributes(*node);
        const auto result=context.instance(constructor,Payload{node,owner,definition,{}, {}});
        if(properties){
            auto nodeOwner=std::static_pointer_cast<void>(node);
            const auto kindCell=std::make_shared<NativeDataField>(nodeOwner,
                [node]{return Value(node->kind);},
                [node](Value value){if(const auto* text=std::get_if<std::u16string>(&value.data))node->kind=*text;else throw Exception("TypeError",u"Scene node kind must be a string");},
                [node]{node->kind.clear();});
            context.bindData(result,Key(u"kind"),kindCell);
            if(std::holds_alternative<scene::SourceTextPrimitive>(node->semantic)){
                const auto valueCell=std::make_shared<NativeDataField>(nodeOwner,
                    [self=shared_from_this(),result,node]{
                        const auto serviceLease=self->requireServices();
                        if(const auto identity=self->sourceIdentity(node))
                            if(const auto* text=std::get_if<scene::SourceTextPrimitive>(&node->semantic))
                                if(const auto owner=std::dynamic_pointer_cast<const SourceTextValue>(text->owner))
                                    return self->properties->to(owner->get());
                        const auto* text=std::get_if<scene::SourceTextPrimitive>(&node->semantic);
                        if(!text)throw Exception("TypeError",u"Scene text value is unavailable");
                        return Value(text->text);
                    },
                    [self=shared_from_this(),node](Value value){
                        auto* text=std::get_if<scene::SourceTextPrimitive>(&node->semantic);
                        if(!text)throw Exception("TypeError",u"Scene text value is unavailable");
                        if(const auto owner=std::dynamic_pointer_cast<const SourceTextValue>(text->owner)){
                            const auto serviceLease=self->requireServices();
                            std::const_pointer_cast<SourceTextValue>(owner)->set(self->properties->from(std::move(value)));
                            return;
                        }
                        const auto* string=std::get_if<std::u16string>(&value.data);
                        if(!string)throw Exception("TypeError",u"Scene text value must be a string");
                        text->text=*string;
                    },
                    [self=shared_from_this(),node]{
                        const auto serviceLease=self->requireServices();
                        if(auto* text=std::get_if<scene::SourceTextPrimitive>(&node->semantic)){
                            if(const auto owner=std::dynamic_pointer_cast<const SourceTextValue>(text->owner))
                                std::const_pointer_cast<SourceTextValue>(owner)->set(Missing{});
                            else text->text.clear();
                        }
                    });
                context.bindData(result,Key(u"value"),valueCell);
            }
            const auto propertyBinding=properties;
            const auto attributesCell=std::make_shared<NativeDataField>(nodeOwner,
                [self=shared_from_this(),result]{const auto serviceLease=self->requireServices();auto& payload=self->payload(result);if(auto source=self->sourceAttributeOwner(payload.node))return source->raw;return Value(self->attributesObject(payload));},
                [self=shared_from_this(),result,propertyBinding](Value value){
                    const auto serviceLease=self->requireServices();
                    auto& payload=self->payload(result);
                    const auto assigned=value;
                    const auto property=propertyBinding->from(std::move(value));
                    const auto record=PropertyList::record(property);
                    if(std::holds_alternative<scene::SourceDomPrimitive>(payload.node->semantic)){
                        if(record)setAttributes(*payload.node,*record);
                        payload.node->attributes.replaceOwner(std::make_shared<State::SourceAttributeOwner>(self,assigned));
                        payload.node->geometryOwner=std::make_shared<State::SourceGeometryOwner>(self,payload.node);
                        payload.node->paintOwner=std::make_shared<State::SourcePaintOwner>(payload.node);
                    }else{
                        if(!record)throw Exception("TypeError",u"Scene node attributes must be an object");
                        setAttributes(*payload.node,*record);
                    }
                    payload.styles.reset();
                },
                [self=shared_from_this(),result,node]{
                    const auto serviceLease=self->requireServices();
                    auto& payload=self->payload(result);
                    if(auto source=self->sourceAttributeOwner(node)){
                        source->raw=Undefined{};
                    }else{
                        payload.node->attributes=scene::AttributeList{};
                    }
                    payload.styles.reset();
                });
            context.bindData(result,Key(u"attributes"),attributesCell);
            const auto childrenCell=std::make_shared<NativeDataField>(nodeOwner,
                [self=shared_from_this(),result]{
                    const auto serviceLease=self->requireServices();
                    auto& payload=self->payload(result);
                    return self->childrenArray(payload);
                },
                [self=shared_from_this(),node](Value value){
                    const auto serviceLease=self->requireServices();
                    node->children.replaceOwner(std::make_shared<State::ContextChildOwner>(self,node,std::move(value)));
                },
                [self=shared_from_this(),node]{
                    const auto serviceLease=self->requireServices();
                    node->children.replaceOwner(std::make_shared<State::ContextChildOwner>(self,node,Value(Undefined{})));
                });
            context.bindData(result,Key(u"children"),childrenCell);
            const auto parentCell=std::make_shared<NativeDataField>(nodeOwner,
                [self=shared_from_this(),result]{
                    const auto serviceLease=self->requireServices();
                    auto& payload=self->payload(result);
                    if(const auto identity=self->sourceIdentity(payload.node))
                        return self->context.get(*identity,Key(u"parent"));
                    if(std::holds_alternative<ObjectRef>(payload.node->parentValue.data))
                        return payload.node->parentValue;
                    if(const auto parent=payload.node->parent.lock()){
                        if(const auto identity=self->sourceIdentity(parent))return *identity;
                        return Value(self->handle(parent));
                    }
                    return payload.node->parentValue;
                },
                [self=shared_from_this(),result](Value value){
                    const auto serviceLease=self->requireServices();
                    auto& payload=self->payload(result);
                    if(const auto identity=self->sourceIdentity(payload.node)){
                        self->context.set(*identity,Key(u"parent"),value);
                        return;
                    }
                    payload.node->parentValue=value;
                    payload.node->parent.reset();
                    if(const auto* object=std::get_if<ObjectRef>(&value.data)){
                        scene::NodeRef parentNode;
                        if(const auto* parent=std::any_cast<Payload>(&std::as_const(self->context).payload(*object)))
                            parentNode=parent->node;
                        else if(const auto source=self->sourceNode(value))parentNode=*source;
                        if(parentNode)payload.node->parent=parentNode;
                    }
                },
                [self=shared_from_this(),result]{
                    const auto serviceLease=self->requireServices();
                    auto& payload=self->payload(result);
                    payload.node->parentValue=Undefined{};
                    payload.node->parent.reset();
                });
            context.bindData(result,Key(u"parent"),parentCell);
            const auto stylesCell=std::make_shared<NativeDataField>(nodeOwner,
                [self=shared_from_this(),result]{
                    if(!self->lite)throw Exception("Unimplemented",u"Scene node Styles binding is not installed");
                    const auto serviceLease=self->requireServices();
                    auto& payload=self->payload(result);
                    if(!payload.styles&&payload.node->attributes.hasOwn("style"))
                        payload.styles=std::make_shared<styles::Styles>(
                            property_host::string(payload.node->attributes.getOwn("style")));
                    return payload.styles?Value(self->lite->stylesBindings().bind(payload.styles)):Value(Null{});
                },
                [self=shared_from_this(),result](Value value){
                    if(!self->lite)throw Exception("Unimplemented",u"Scene node Styles binding is not installed");
                    const auto serviceLease=self->requireServices();
                    auto& payload=self->payload(result);
                    if(std::holds_alternative<Null>(value.data)||std::holds_alternative<Undefined>(value.data)){payload.styles.reset();return;}
                    const auto* object=std::get_if<ObjectRef>(&value.data);
                    if(!object)throw Exception("TypeError",u"styles must be a Styles instance or null");
                    payload.styles=self->lite->styleFor(*object);
                },
                [self=shared_from_this(),result]{
                    const auto serviceLease=self->requireServices();
                    self->payload(result).styles.reset();
                });
            context.bindData(result,Key(u"styles"),stylesCell);
        }
        handles.emplace(node.get(),result);
        return result;
    }
    std::uint64_t beginRequestScope(){
        if(requestScopeActive)throw Exception("TypeError",u"Scene request owner scope is already active");
        requestNodes.clear();requestNodeSet.clear();requestScopeActive=true;
        return ++requestGeneration;
    }
    void retireRequestScope(const SceneContextNodes::RequestMark& mark){
        if(mark.owner!=this||!requestScopeActive||mark.generation!=requestGeneration)
            throw Exception("TypeError",u"Scene request mark does not belong to the current owner scope");
        while(!requestNodes.empty()){
            const auto* node=requestNodes.back();
            if(const auto found=handles.find(node);found!=handles.end()){
                context.retireObjectRecord(found->second);
                handles.erase(found);
            }
            sourceIdentities.erase(node);
            sourceViews.erase(node);
            requestNodeSet.erase(node);
            requestNodes.pop_back();
        }
        requestNodeSet.clear();
        requestScopeActive=false;
        ++requestGeneration;
    }
};

struct SceneContextNodes::State::SourceView final:adaptors::lite::LiteNodeOwnerView {
    std::weak_ptr<State> state;
    scene::NodeRef node;
    bool parentDetached=false;
    Property parentValue=Missing{};
    SourceView(std::weak_ptr<State> owner,scene::NodeRef value)
        :state(std::move(owner)),node(std::move(value)){}
    State::ServiceAccess owner()const{
        auto result=state.lock();
        if(!result)throw Exception("TypeError",u"Scene owner view has expired");
        auto lease=result->requireServices();
        if(!result->properties||!result->lite)
            throw Exception("TypeError",u"Scene provider services have expired");
        return {result,std::move(lease)};
    }
    Property getProperty(std::string_view field)const override{
        auto current=owner();
        if(!current->properties)throw Exception("Unimplemented",u"Scene node property bindings are not installed");
        if(field!="kind"&&field!="value"&&field!="attributes"&&field!="children"&&field!="parent"&&field!="styles")
            throw Exception("TypeError",u"Unsupported Scene-backed Lite field");
        if(field=="value"){
            const auto* text=std::get_if<scene::SourceTextPrimitive>(&node->semantic);
            if(!text)throw Exception("TypeError",u"Only Scene text and comment nodes have a value field");
            if(const auto owner=std::dynamic_pointer_cast<const State::SourceTextValue>(text->owner))
                return owner->get();
            return Property(property_host::debugText(text->text));
        }
        if(field=="parent"){
            if(!parentDetached)return parentValue;
            const auto identity=current->sourceIdentity(node);
            return identity?current->properties->from(current->context.get(*identity,Key(u"parent"))):Property(Missing{});
        }
        if(field=="styles"){
            const auto style=getStyles();
            const auto value=style?Value(current->lite->stylesBindings().bind(style)):Value(Null{});
            return current->properties->from(value);
        }
        const auto handle=current->handle(node);
        const std::u16string name(field.begin(),field.end());
        return current->properties->from(current->context.get(Value(handle),Key(name)));
    }
    bool hasOwnProperty(std::string_view field)const override{
        auto current=owner();
        // The canonical Lite Context identity owns the live descriptor. Query
        // it for every field so deletion and accessor replacement remain
        // visible through the typed owner view as well.
        if(const auto identity=current->sourceIdentity(node))
            return current->context.hasOwn(*identity,Key(property_host::text(field)));
        if(std::holds_alternative<scene::SourceTextPrimitive>(node->semantic))return field=="value";
        if(std::holds_alternative<scene::SourceDomPrimitive>(node->semantic))
            return field=="kind"||field=="attributes"||field=="children"||field=="styles";
        return false;
    }
    std::vector<Property> childrenSnapshot()const override{
        auto current=owner();
        const auto source=current->sourceChildOwner(node);
        if(!source)throw Exception("TypeError",u"Scene-backed Lite node has no Source children owner");
        std::vector<Property> result;
        for(const auto& value:source->values())result.push_back(current->properties->from(value));
        return result;
    }
    void setProperty(std::string_view field,Property value)override{
        auto current=owner();
        if(!current->properties)throw Exception("Unimplemented",u"Scene node property bindings are not installed");
        if(field!="kind"&&field!="value"&&field!="attributes"&&field!="children"&&field!="parent"&&field!="styles")
            throw Exception("TypeError",u"Unsupported Scene-backed Lite field");
        if(field=="value"){
            auto* text=std::get_if<scene::SourceTextPrimitive>(&node->semantic);
            if(!text)throw Exception("TypeError",u"Only Scene text and comment nodes have a value field");
            if(const auto owner=std::dynamic_pointer_cast<const State::SourceTextValue>(text->owner)){
                std::const_pointer_cast<State::SourceTextValue>(owner)->set(std::move(value));
                return;
            }
            text->text=property_host::string(value);
            return;
        }
        if(field=="parent"){
            const auto identity=current->sourceIdentity(node);
            if(identity)current->context.set(*identity,Key(u"parent"),current->properties->to(std::move(value)));
            return;
        }
        if(field=="styles"){
            const auto sourceValue=current->properties->to(std::move(value));
            if(std::holds_alternative<Null>(sourceValue.data)||std::holds_alternative<Undefined>(sourceValue.data)){
                setStyles({});
                return;
            }
            const auto* object=std::get_if<ObjectRef>(&sourceValue.data);
            if(!object)throw Exception("TypeError",u"styles must be a Styles instance or null");
            setStyles(current->lite->styleFor(*object));
            return;
        }
        const auto handle=current->handle(node);
        const std::u16string name(field.begin(),field.end());
        current->context.set(Value(handle),Key(name),current->properties->to(std::move(value)));
    }
    void setPropertyFromContext(std::string_view field,Property value)override{
        if(field!="parent"){setProperty(field,std::move(value));return;}
        auto current=owner();
        parentDetached=false;
        parentValue=value;
        (void)current;
    }
    void removeProperty(std::string_view field)override{
        auto current=owner();
        if(field!="kind"&&field!="value"&&field!="attributes"&&field!="children"&&field!="parent"&&field!="styles")
            throw Exception("TypeError",u"Unsupported Scene-backed Lite field");
        if(field=="parent"){
            // Invalidate the SourceView cache before removing the corresponding
            // Scene handle descriptor. The canonical Lite Context deletion
            // completes after this callback; later reads then observe that
            // current descriptor (including a subsequently defined accessor).
            parentValue=Missing{};
            parentDetached=true;
        }
        // The Scene Context identity is the native owner behind this Lite
        // projection. Delete its own data field through Context so the real
        // Scene owner callback updates the same backing slot; writing
        // Undefined here would leave an own property and a second authority.
        current->context.erase(current->handle(node),Key(property_host::text(field)));
    }
    std::shared_ptr<styles::Styles> getStyles()const override{
        auto current=owner();
        auto& payload=current->payload(current->handle(node));
        if(!payload.styles&&node->attributes.hasOwn("style"))
            payload.styles=std::make_shared<styles::Styles>(
                property_host::string(node->attributes.getOwn("style")));
        return payload.styles;
    }
    void setStyles(std::shared_ptr<styles::Styles> value)override{
        auto current=owner();
        current->payload(current->handle(node)).styles=std::move(value);
    }
};
struct SceneContextNodes::TargetScope::Impl {
    adaptors::lite::LiteModule& lite;
    adaptors::lite::LiteModule::AppendTarget append;
    adaptors::lite::LiteModule::AttributeTarget attribute;
    adaptors::lite::LiteModule::StyleTarget style;
    adaptors::lite::LiteModule::NodeOperationTarget nodeOperation;
    adaptors::lite::LiteModule::EventMetadataTarget eventMetadata;
    Impl(adaptors::lite::LiteModule& module,adaptors::lite::LiteModule::AppendTarget a,
         adaptors::lite::LiteModule::AttributeTarget s,adaptors::lite::LiteModule::StyleTarget t,
         adaptors::lite::LiteModule::NodeOperationTarget n,
         adaptors::lite::LiteModule::EventMetadataTarget e)
        :lite(module),append(std::move(a)),attribute(std::move(s)),style(std::move(t)),
         nodeOperation(std::move(n)),eventMetadata(std::move(e)){}
    ~Impl(){
        (void)lite.exchangeEventMetadataTarget(std::move(eventMetadata));
        (void)lite.exchangeNodeOperationTarget(std::move(nodeOperation));
        (void)lite.exchangeStyleTarget(std::move(style));
        (void)lite.exchangeAttributeTarget(std::move(attribute));
        (void)lite.exchangeAppendTarget(std::move(append));
    }
};
SceneContextNodes::TargetScope::TargetScope(std::unique_ptr<Impl> impl):impl_(std::move(impl)){}
SceneContextNodes::TargetScope::TargetScope(TargetScope&&)noexcept=default;
SceneContextNodes::TargetScope& SceneContextNodes::TargetScope::operator=(TargetScope&&)noexcept=default;
SceneContextNodes::TargetScope::~TargetScope()=default;
std::size_t SceneContextNodes::handleCountForDiagnostics()const noexcept{return state_->handles.size();}
SceneContextNodes::RequestOwnerCounts SceneContextNodes::requestOwnerCountsForDiagnostics()const noexcept{
    return {state_->handles.size(),state_->sourceIdentities.size(),state_->sourceViews.size()};
}
SceneContextNodes::RequestMark SceneContextNodes::beginRequestScope(){
    return {state_.get(),state_->beginRequestScope()};
}
void SceneContextNodes::retireRequestScope(RequestMark mark){state_->retireRequestScope(mark);}

SceneContextNodes::SceneContextNodes(Context& context,std::u16string name)
    :state_(std::make_shared<State>(context,std::move(name))){}
SceneContextNodes::~SceneContextNodes()=default;
ObjectRef SceneContextNodes::handle(const scene::NodeRef& node,
    std::optional<ObjectRef> owner,std::optional<ObjectRef> definition){
    return state_->handle(node,owner,definition);
}
bool SceneContextNodes::isHandle(const Value& value)const{
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object)return false;
    return std::any_cast<State::Payload>(&std::as_const(state_->context).payload(*object))!=nullptr;
}
scene::NodeRef SceneContextNodes::resolve(ObjectRef object)const{return state_->payload(object).node;}
std::optional<ObjectRef> SceneContextNodes::owner(ObjectRef object)const{return state_->payload(object).owner;}
std::optional<ObjectRef> SceneContextNodes::definition(ObjectRef object)const{return state_->payload(object).definition;}
void SceneContextNodes::associateSourceNode(const scene::NodeRef& node,const Value& identity){
    const auto serviceLease=state_->requireServices();
    if(!node||!std::holds_alternative<ObjectRef>(identity.data))
        throw Exception("TypeError",u"Source node identity must be an object");
    if(!state_->properties||!state_->lite)
        throw Exception("Unimplemented",u"Source-mapped Scene nodes require Lite and property bindings");
    const auto sceneHandle=state_->handle(node);
    state_->sourceIdentities[node.get()]=identity;
    if(std::holds_alternative<scene::SourceDomPrimitive>(node->semantic)){
        const auto attributesValue=state_->context.get(identity,Key(u"attributes"));
        node->attributes.replaceOwner(std::make_shared<State::SourceAttributeOwner>(state_,attributesValue));
        node->geometryOwner=std::make_shared<State::SourceGeometryOwner>(state_,node);
        node->paintOwner=std::make_shared<State::SourcePaintOwner>(node);
        const auto childValue=state_->context.get(identity,Key(u"children"));
        node->children.replaceOwner(std::make_shared<State::ContextChildOwner>(state_,node,childValue));
    }
    const auto property=state_->properties->from(identity);
    const auto raw=propertyNode(property);
    const auto* liteNode=std::get_if<DOMNodeRef>(&raw);
    if(!liteNode||!*liteNode)throw Exception("TypeError",u"Source identity is not a Lite DOM node");
    auto view=std::make_shared<State::SourceView>(state_,node);
    view->parentValue=state_->properties->from(state_->context.get(identity,Key(u"parent")));
    state_->sourceViews[node.get()]=view;
    state_->lite->bindOwnerView(*liteNode,view);
    if(auto* text=std::get_if<scene::SourceTextPrimitive>(&node->semantic)){
        text->owner=std::make_shared<State::SourceTextValue>(state_,
            state_->properties->from(state_->context.get(identity,Key(u"value"))));
        text->text.clear();
    }
}
std::optional<scene::NodeRef> SceneContextNodes::sourceNode(const Value& identity)const{
    return state_->sourceNode(identity);
}
std::optional<scene::NodeRef> SceneContextNodes::projectSourceChild(const Value& identity){
    const auto serviceLease=state_->requireServices();
    if(!std::holds_alternative<ObjectRef>(identity.data))return {};
    if(const auto existing=state_->sourceNode(identity))return existing;
    if(!state_->sourceChildResolver)return {};
    auto projected=state_->sourceChildResolver(identity);
    return projected?std::optional<scene::NodeRef>{std::move(projected)}:std::nullopt;
}
std::optional<scene::NodeRef> SceneContextNodes::findProjectedNode(const Value& identity)const{
    // This is a lookup only. In particular, an unprojected tooltip element
    // must not be forced into the Scene handle table by event registration.
    return state_->findProjectedNode(identity);
}
std::optional<Value> SceneContextNodes::sourceIdentity(const scene::NodeRef& node)const{
    return state_->sourceIdentity(node);
}
void SceneContextNodes::setSourceChildResolver(std::function<scene::NodeRef(const Value&)> resolver){
    state_->sourceChildResolver=std::move(resolver);
}
void SceneContextNodes::setOwner(ObjectRef object,std::optional<ObjectRef> owner){state_->payload(object).owner=owner;}
void SceneContextNodes::setDefinition(ObjectRef object,std::optional<ObjectRef> definition){state_->payload(object).definition=definition;}
void SceneContextNodes::bindPropertyBindings(ContextPropertyBindings& properties){
    if(&properties.context()!=&state_->context)throw Exception("TypeError",u"Scene node properties use another Context");
    if(state_->properties&&state_->properties!=&properties)throw Exception("TypeError",u"Scene node properties are already bound");
    state_->properties=&properties;
}
void SceneContextNodes::bindLiteModule(adaptors::lite::LiteModule& lite){
    if(&lite.context()!=&state_->context)throw Exception("TypeError",u"Scene node Lite module uses another Context");
    if(state_->lite&&state_->lite!=&lite)throw Exception("TypeError",u"Scene node Lite module is already bound");
    state_->lite=&lite;
    const auto prototype=std::get<ObjectRef>(state_->context.get(
        Value(state_->constructor),Key(u"prototype")).data);
    const Key eventKey(u"addEventListener");
    if(!state_->context.hasOwn(Value(prototype),eventKey)){
        std::weak_ptr<State> weak=state_;
        const auto addEventListener=state_->context.function(u"addEventListener",
            [weak](Context&,Value receiver,std::span<const Value> args)->Value{
                auto owner=weak.lock();
                if(!owner)throw Exception("TypeError",u"Scene node host has expired");
                const auto serviceLease=owner->requireServices();
                if(!owner->lite)throw Exception("TypeError",u"Scene node host has expired");
                const auto* target=std::get_if<ObjectRef>(&receiver.data);
                if(!target)throw Exception("TypeError",u"SceneNode.addEventListener receiver is not an object");
                const auto type=args.empty()?Value(Undefined{}):args[0];
                const auto callback=args.size()>1?args[1]:Value(Undefined{});
                owner->lite->registerHostEvent(*target,type,callback);
                return Undefined{};
            });
        state_->context.define(addEventListener,Key(u"length"),
            Descriptor::data(Value(2.0),false,false,true));
        state_->context.define(prototype,eventKey,
            Descriptor::data(Value(addEventListener),true,true,true));
    }
}
void SceneContextNodes::bindServiceOwner(std::weak_ptr<void> owner){
    if(owner.expired())throw Exception("TypeError",u"Scene provider owner has expired");
    if(!state_->serviceOwner.expired())throw Exception("TypeError",u"Scene provider owner is already bound");
    state_->serviceOwner=std::move(owner);
}
SceneContextNodes::TargetScope SceneContextNodes::installTargets(
    adaptors::lite::LiteModule& lite,scene::Builder& builder){
    if(&lite.context()!=&state_->context)throw Exception("TypeError",u"Lite target uses another Context");
    const auto shared=state_;
    adaptors::lite::LiteModule::AppendTarget append;
    adaptors::lite::LiteModule::AttributeTarget attribute;
    adaptors::lite::LiteModule::StyleTarget style;
    adaptors::lite::LiteModule::NodeOperationTarget nodeOperation;
    adaptors::lite::LiteModule::EventMetadataTarget eventMetadata;
    bool appendInstalled=false,attributeInstalled=false,styleInstalled=false;
    bool nodeOperationInstalled=false,eventMetadataInstalled=false;
    try{
    append=lite.exchangeAppendTarget([shared,&builder](Context&,Value,std::span<const Value> args)
        ->std::optional<Value>{
        const auto serviceLease=shared->requireServices();
        if(args.size()<2)return {};
        const auto* parentObject=std::get_if<ObjectRef>(&args[0].data);
        if(!parentObject)return {};
        const auto* parent=std::any_cast<State::Payload>(&std::as_const(shared->context).payload(*parentObject));
        scene::NodeRef parentNode=parent?parent->node:scene::NodeRef{};
        if(!parentNode)parentNode=shared->sourceNode(args[0]).value_or(scene::NodeRef{});
        if(!parentNode)return {};
        const auto* childObject=std::get_if<ObjectRef>(&args[1].data);
        if(!childObject)return {};
        scene::NodeRef child;
        for(const auto& [raw,handle]:shared->handles){(void)raw;if(handle==*childObject){child=shared->payload(handle).node;break;}}
        const auto source=child?std::optional<scene::NodeRef>{}:shared->sourceNode(args[1]);
        if(!child&&source)child=*source;
        if(!child)return {};
        builder.validateAppend(parentNode,child);
        // The Source adaptor owns the single mutation sequence after this
        // validation: remove(child), children.push(child), then parent write.
        // Mutating the Scene list here would append twice through its live view.
        return {};
    });
    appendInstalled=true;
    attribute=lite.exchangeAttributeTarget([shared]
        (Context& cx,Value,std::span<const Value> args)->std::optional<Value>{
        const auto serviceLease=shared->requireServices();
        if(args.empty())return {};
        const auto* object=std::get_if<ObjectRef>(&args[0].data);
        if(!object)return {};
        auto* target=std::any_cast<State::Payload>(&shared->context.payload(*object));
        const auto sourceNode=target?std::optional<scene::NodeRef>{}:shared->sourceNode(args[0]);
        if(!target&&sourceNode)target=&shared->payload(shared->handle(*sourceNode));
        if(!target)return {};
        if(args.size()<3)throw Exception("TypeError",u"setAttribute requires a name and value");
        auto primitiveString=[&](const Value& value){
            const auto primitive=cx.toPrimitive(value,PrimitiveHint::stringHint);
            if(const auto* str=std::get_if<std::u16string>(&primitive.data))return *str;
            if(const auto* number=std::get_if<double>(&primitive.data))return property_host::string(Property(*number));
            if(const auto* boolean=std::get_if<bool>(&primitive.data))return std::u16string(*boolean?u"true":u"false");
            if(std::holds_alternative<Null>(primitive.data))return std::u16string(u"null");
            if(std::holds_alternative<Undefined>(primitive.data))return std::u16string(u"undefined");
            throw Exception("TypeError",u"Cannot convert Symbol to string");
        };
        auto name=primitiveString(args[1]);
        if(args.size()>3&&!std::holds_alternative<Undefined>(args[3].data)&&
           !std::holds_alternative<Null>(args[3].data)){
            const auto namespaceText=primitiveString(args[3]);
            if(!namespaceText.empty()){
                const auto slash=namespaceText.find_last_of(u'/');
                const auto prefix=namespaceText.substr(slash==std::u16string::npos?0:slash+1);
                const auto colon=name.find_last_of(u':');
                name=prefix+u":"+name.substr(colon==std::u16string::npos?0:colon+1);
            }
        }
        const auto key=property_host::debugText(name);
        const auto value=primitiveString(args[2]);
        if(shared->isPresentationWrite(target->node,key,args[2])){
            target->node->attributes.setOwn(key,Property(property_host::debugText(value)));
            if(name==u"style")target->styles.reset();
            return Value(Undefined{});
        }
        if(State::geometryField(*target->node,key)){
            State::setGeometry(*target->node,key,value);
            target->node->attributes.setOwn(key,Property(property_host::debugText(value)));
            State::refreshRectFields(*target->node);
        }else if(State::paintField(*target->node,key)){
            State::setPaint(*target->node,key,value);
            target->node->attributes.setOwn(key,Property(property_host::debugText(value)));
        }else target->node->attributes.setOwn(key,Property(property_host::debugText(value)));
        if(name==u"style")target->styles.reset();
        return Value(Undefined{});
    });
    attributeInstalled=true;
    style=lite.exchangeStyleTarget([shared](Context& cx,Value,std::span<const Value> args)->std::optional<Value>{
        const auto serviceLease=shared->requireServices();
        if(args.empty())return {};
        const auto* object=std::get_if<ObjectRef>(&args[0].data);
        if(!object)return {};
        auto* target=std::any_cast<State::Payload>(&shared->context.payload(*object));
        if(!target){const auto source=shared->sourceNode(args[0]);if(source)target=&shared->payload(shared->handle(*source));}
        if(!target)return {};
        if(!shared->lite)throw Exception("Unimplemented",u"Scene node Styles binding is not installed");
        const auto sourceString=[&](const Value& value){
            const auto primitive=cx.toPrimitive(value,PrimitiveHint::stringHint);
            if(const auto* text=std::get_if<std::u16string>(&primitive.data))return *text;
            if(const auto* number=std::get_if<double>(&primitive.data))return property_host::string(Property(*number));
            if(const auto* boolean=std::get_if<bool>(&primitive.data))return std::u16string(*boolean?u"true":u"false");
            if(std::holds_alternative<Null>(primitive.data))return std::u16string(u"null");
            if(std::holds_alternative<Undefined>(primitive.data))return std::u16string(u"undefined");
            throw Exception("TypeError",u"Cannot convert Symbol to string");
        };
        const auto name=args.size()>1?sourceString(args[1]):std::u16string(u"undefined");
        const auto value=args.size()>2?sourceString(args[2]):std::u16string(u"undefined");
        if(!target->styles)target->styles=std::make_shared<styles::Styles>(property_host::string(target->node->attributes.get("style")));
        target->styles->set(name,styles::StyleValue(value));
        const auto css=target->styles->cssText();
        target->node->attributes.set("style",Property(property_host::debugText(css)));
        return Value(Undefined{});
    });
    styleInstalled=true;
    nodeOperation=lite.exchangeNodeOperationTarget([shared]
        (Context& cx,std::u16string_view operation,std::span<const Value> args)->std::optional<Value>{
        const auto serviceLease=shared->requireServices();
        auto sceneNode=[&](const Value& value)->std::optional<std::pair<ObjectRef,scene::NodeRef>>{
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)return {};
            for(const auto& [raw,handle]:shared->handles){
                (void)raw;
                if(handle==*object){const auto& node=shared->payloadReadOnly(handle).node;return std::pair{handle,node};}
            }
            const auto source=shared->sourceNode(value);
            if(!source)return {};
            return std::pair{shared->handle(*source),*source};
        };
        auto childIndex=[&](Value child)->Value{
            if(!options::truthy(cx.get(child,Key(u"parent"))))return Value(-1.0);
            const Value parent=cx.get(child,Key(u"parent"));
            const Value children=cx.get(parent,Key(u"children"));
            const Value findIndex=cx.get(children,Key(u"findIndex"));
            const auto predicateIdentity=std::make_shared<ObjectRef>();
            const ObjectRef predicate=cx.nativeConstructor(u"",
                [predicateIdentity](Context& context,std::span<const Value>)->ObjectRef{
                    const Value prototype=context.get(Value(*predicateIdentity),Key(u"prototype"));
                    if(const auto* object=std::get_if<ObjectRef>(&prototype.data))
                        return context.objectWithPrototype(*object);
                    return context.objectWithPrototype(context.intrinsics().objectPrototype);
                },
                [needle=child](Context&,Value,std::span<const Value> arguments)->Value{
                    return Value(!arguments.empty()&&arguments.front()==needle);
                });
            *predicateIdentity=predicate;
            cx.define(predicate,Key(u"length"),
                Descriptor::data(Value(1.0),false,false,true));
            const std::array<Value,1> arguments{Value(predicate)};
            return cx.callMember(findIndex,children,arguments,u"parent.children.findIndex");
        };
        auto nonNegativeIndex=[&](const Value& index){return cx.toNumber(index)>=0;};
        auto sourceIndexKey=[&](Value index)->Key{
            const Value primitive=cx.toPrimitive(std::move(index),PrimitiveHint::stringHint);
            if(const auto* symbol=std::get_if<SymbolRef>(&primitive.data))return Key(*symbol);
            if(const auto* text=std::get_if<std::u16string>(&primitive.data))return Key(*text);
            if(std::holds_alternative<Undefined>(primitive.data))return Key(u"undefined");
            if(std::holds_alternative<Null>(primitive.data))return Key(u"null");
            if(const auto* boolean=std::get_if<bool>(&primitive.data))return Key(*boolean?u"true":u"false");
            if(const auto* number=std::get_if<double>(&primitive.data))
                return Key(property_host::text(js_number::toString(*number)));
            throw Exception("TypeError",u"Cannot convert child index to a property key");
        };
        auto sourceLess=[&](Value left,Value right){
            const Value lhs=cx.toPrimitive(std::move(left),PrimitiveHint::numberHint);
            const Value rhs=cx.toPrimitive(std::move(right),PrimitiveHint::numberHint);
            const auto* lhsText=std::get_if<std::u16string>(&lhs.data);
            const auto* rhsText=std::get_if<std::u16string>(&rhs.data);
            if(lhsText&&rhsText)return *lhsText<*rhsText;
            return cx.toNumber(lhs)<cx.toNumber(rhs);
        };
        auto spliceChildren=[&](Value parent,Value index,Value deleteCount,
                                std::span<const Value> inserted){
            const Value children=cx.get(parent,Key(u"children"));
            const Value splice=cx.get(children,Key(u"splice"));
            std::vector<Value> fullArguments{std::move(index),std::move(deleteCount)};
            fullArguments.insert(fullArguments.end(),inserted.begin(),inserted.end());
            (void)cx.callMember(splice,children,fullArguments,u"parent.children.splice");
        };
        auto assignChildAt=[&](Value parent,Value index,Value replacement){
            const Value children=cx.get(parent,Key(u"children"));
            cx.set(children,sourceIndexKey(std::move(index)),std::move(replacement));
        };
        if(args.empty())return {};
        const Value targetValue=operation==u"replace"&&args.size()>1?args[1]:args[0];
        const auto target=sceneNode(targetValue);
        if(!target)return {};
        const auto& [handle,node]=*target;
        const bool sourceMapped=shared->sourceIdentity(node).has_value();
        if(sourceMapped&&operation!=u"parent"&&operation!=u"remove"&&
           operation!=u"replace"&&operation!=u"insert"&&
           operation!=u"childNodes"&&operation!=u"firstChild"&&
           operation!=u"lastChild"&&operation!=u"childNode"&&
           operation!=u"next"&&operation!=u"previous")return {};
        if(operation==u"parent")
            return cx.get(targetValue,Key(u"parent"));
        if(operation==u"remove"){
            const Value index=childIndex(targetValue);
            if(nonNegativeIndex(index)){
                const Value parent=cx.get(targetValue,Key(u"parent"));
                spliceChildren(parent,index,Value(1.0),{});
            }
            cx.set(targetValue,Key(u"parent"),Value(Null{}));
            return args[0];
        }
        if(operation==u"kind")return Value(node->kind);
        if(operation==u"removeAttribute"){
            if(args.size()<2)throw Exception("TypeError",u"removeAttribute requires an attribute name");
            const auto serviceLease=shared->requireServices();
            const auto primitive=cx.toPrimitive(args[1],PrimitiveHint::stringHint);
            const auto name=property_host::debugText(property_host::string(shared->properties->from(primitive)));
            node->attributes.erase(name);
            if(name=="style")shared->payload(handle).styles.reset();
            return Value(Undefined{});
        }
        if(operation==u"getAttribute"){
            if(args.size()<2)throw Exception("TypeError",u"getAttribute requires an attribute name");
            if(!shared->properties)throw Exception("Unimplemented",u"Scene node property bindings are not installed");
            const auto serviceLease=shared->requireServices();
            const auto primitive=cx.toPrimitive(args[1],PrimitiveHint::stringHint);
            const auto name=property_host::string(shared->properties->from(primitive));
            return shared->properties->to(node->attributes.get(property_host::debugText(name)));
        }
        if(operation==u"childNodes"){
            const Value children=cx.get(targetValue,Key(u"children"));
            const Value sequence=cx.readIterable(children);
            const auto count=State::ContextChildOwner::toLength(cx,cx.get(sequence,Key(u"length")));
            if(count>std::numeric_limits<std::uint32_t>::max())
                throw Exception("RangeError",u"Invalid array length");
            const auto copy=cx.array(static_cast<std::uint32_t>(count));
            for(std::size_t index=0;index<count;++index){
                const Key key(property_host::text(std::to_string(index)));
                if(cx.has(sequence,key))cx.set(Value(copy),key,cx.get(sequence,key));
            }
            return Value(copy);
        }
        if(operation==u"firstChild"){
            const Value children=cx.get(targetValue,Key(u"children"));
            return cx.get(children,Key(u"0"));
        }
        if(operation==u"lastChild"){
            const Value base=cx.get(targetValue,Key(u"children"));
            const Value lengthOwner=cx.get(targetValue,Key(u"children"));
            const Value length=cx.get(lengthOwner,Key(u"length"));
            const double last=cx.toNumber(length)-1.0;
            return cx.get(base,sourceIndexKey(Value(last)));
        }
        if(operation==u"childNode"){
            const Value children=cx.get(targetValue,Key(u"children"));
            return cx.get(children,sourceIndexKey(args.size()>1?args[1]:Value(Undefined{})));
        }
        if(operation==u"next"||operation==u"previous"){
            const Value parent=cx.get(targetValue,Key(u"parent"));
            if(!options::truthy(parent))return Value(Null{});
            const auto found=childIndex(targetValue);
            if(operation==u"next"){
                const Value index=property_host::add(cx,found,Value(1.0));
                const Value lengthOwner=cx.get(parent,Key(u"children"));
                const Value length=cx.get(lengthOwner,Key(u"length"));
                const double numericIndex=cx.toNumber(index);
                if(std::isnan(numericIndex)||numericIndex<0||!sourceLess(index,length))return Value(Null{});
                const Value slotOwner=cx.get(parent,Key(u"children"));
                return cx.get(slotOwner,sourceIndexKey(index));
            }
            const Value index(cx.toNumber(found)-1.0);
            const double previousIndex=cx.toNumber(index);
            if(std::isnan(previousIndex)||previousIndex<0)return Value(Null{});
            const Value children=cx.get(parent,Key(u"children"));
            return cx.get(children,sourceIndexKey(index));
        }
        if(operation==u"replace"){
            if(args.size()<2)throw Exception("TypeError",u"replace requires a replacement and old child");
            const auto replacement=sceneNode(args[0]);
            if(!replacement)return {};
            const Value index=childIndex(targetValue);
            if(!nonNegativeIndex(index))return args[1];
            const Value parent=cx.get(targetValue,Key(u"parent"));
            assignChildAt(parent,index,args[0]);
            cx.set(args[0],Key(u"parent"),cx.get(args[1],Key(u"parent")));
            cx.set(targetValue,Key(u"parent"),Value(Null{}));
            return args[1];
        }
        if(operation==u"insert"){
            if(args.size()<2)throw Exception("TypeError",u"insert requires a child and sibling");
            const auto child=sceneNode(args[0]);
            const auto sibling=sceneNode(args[1]);
            if(!child||!sibling)return {};
            if(options::truthy(cx.get(args[0],Key(u"parent")))){
                const Value oldIndex=childIndex(args[0]);
                if(nonNegativeIndex(oldIndex)){
                    const Value oldParent=cx.get(args[0],Key(u"parent"));
                    spliceChildren(oldParent,oldIndex,Value(1.0),{});
                }
                cx.set(args[0],Key(u"parent"),Value(Null{}));
            }
            if(options::truthy(cx.get(args[1],Key(u"parent")))){
                const Value index=childIndex(args[1]);
                const std::array<Value,1> inserted{args[0]};
                spliceChildren(cx.get(args[1],Key(u"parent")),index,Value(0.0),inserted);
                cx.set(args[0],Key(u"parent"),cx.get(args[1],Key(u"parent")));
            }
            return Value(Undefined{});
        }
        return {};
    });
    nodeOperationInstalled=true;
    eventMetadata=lite.exchangeEventMetadataTarget(
        [shared](Context&,ObjectRef receiver,std::u16string_view eventType){
            const auto projected=shared->findProjectedNode(Value(receiver));
            if(!projected)return;
            auto& eventTypes=(*projected)->eventTypes;
            if(std::find(eventTypes.begin(),eventTypes.end(),eventType)==eventTypes.end())
                eventTypes.emplace_back(eventType);
        });
    eventMetadataInstalled=true;
    return TargetScope(std::make_unique<TargetScope::Impl>(lite,std::move(append),
        std::move(attribute),std::move(style),std::move(nodeOperation),std::move(eventMetadata)));
    }catch(...){
        if(eventMetadataInstalled)(void)lite.exchangeEventMetadataTarget(std::move(eventMetadata));
        if(nodeOperationInstalled)(void)lite.exchangeNodeOperationTarget(std::move(nodeOperation));
        if(styleInstalled)(void)lite.exchangeStyleTarget(std::move(style));
        if(attributeInstalled)(void)lite.exchangeAttributeTarget(std::move(attribute));
        if(appendInstalled)(void)lite.exchangeAppendTarget(std::move(append));
        throw;
    }
}
void SceneContextNodes::withPresentationAttribute(ObjectRef handle,std::u16string_view name,
    const Value& value,const std::function<void()>& body){
    if(!body)throw Exception("TypeError",u"Scene presentation attribute requires a body");
    state_->presentationWrites.push_back({state_->payload(handle).node.get(),
        property_host::debugText(name),value});
    struct Restore {
        State& state;
        ~Restore(){state.presentationWrites.pop_back();}
    } restore{*state_};
    body();
}
Context& SceneContextNodes::context()const{return state_->context;}
}
