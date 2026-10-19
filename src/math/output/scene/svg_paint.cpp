module folia.math.output.scene;
import :svg_paint;
import folia.math.util.js_number;
import folia.math.util.property_host;
import folia.math.util.string;
import std;

namespace folia::math::output::scene {
namespace {
using folia::math::Property;
using folia::math::PropertyList;
using folia::math::property_host::truthy;

Property sceneAttribute(Property value){
    return Property(folia::math::property_host::debugText(
        folia::math::property_host::string(value)));
}

std::string fixed(common::CommonWrapper& wrapper,double value,unsigned digits=1){
    // SvgWrapper.fixed(x,n) delegates to CommonOutputJax.fixed(x*1000,n).
    return wrapper.jax().fixed(value*1000,digits);
}
Property attr(const common::CommonWrapper& wrapper,std::string_view name){
    const auto attributes=wrapper.node->attributes();
    if(!attributes)folia::math::property_host::typeError("Cannot read properties of null (reading 'getExplicit')");
    return attributes->getExplicit(name);
}
void put(const std::span<const NodeRef> nodes,std::string_view key,Property value){
    for(const auto& node:nodes){
        if(!node)folia::math::property_host::typeError("Cannot read properties of null (reading 'setAttribute')");
            node->attributes.set(key,sceneAttribute(value));
    }
}
void insertFront(const NodeRef& parent,const NodeRef& child){
    if(!parent)folia::math::property_host::typeError("Cannot read properties of null (reading 'firstChild')");
    if(!child)folia::math::property_host::typeError("Cannot read properties of null (reading 'insert')");
    parent->children.insert(0,child);
    child->parent=parent;
    child->parentValue=options::Value(options::Undefined{});
}
void solidBorder(common::CommonWrapper& wrapper,Builder& builder,const NodeRef& parent,
                 const NodeRef& child,const std::array<Point,4>& points,std::string_view color,double dx){
    std::string serialized;
    PolygonGeometry geometry;
    geometry.points.reserve(points.size());
    for(const auto& point:points){
        geometry.points.push_back({point.x-dx,point.y});
        if(!serialized.empty())serialized+=' ';
        serialized+=fixed(wrapper,point.x-dx)+","+fixed(wrapper,point.y);
    }
    auto polygon=builder.node(u"polygon",PropertyList{{"points",Property(serialized)},
        {"stroke",Property("none")}});
    if(!color.empty())polygon->attributes.set("fill",Property(std::string(color)));
    polygon->geometry=std::move(geometry);
    ShapePaint paint;
    paint.stroke="none";
    if(!color.empty())paint.fill=std::string(color);
    paint.emitAttributes=false;
    polygon->paint=std::move(paint);
    if(child){
        for(std::size_t index=0;index<parent->children.size();++index){
            if(parent->children[index]==child){
                parent->children.insert(index,polygon);
                polygon->parent=parent;
                polygon->parentValue=options::Value(options::Undefined{});
                return;
            }
        }
        builder.append(parent,polygon);
    }else builder.append(parent,polygon);
}
}

std::vector<NodeRef> createSvgRoots(common::CommonWrapper& wrapper,Builder& builder,
                                    std::span<const NodeRef> parents){
    const auto href=attr(wrapper,"href");
    const bool linked=truthy(href);
    const bool embellished=wrapper.node->isEmbellished()&&!wrapper.node->isKind("mo");
    double dx=0;
    if(wrapper.styleData){
        dx=wrapper.styleData->padding[3];
        if(wrapper.styleData->border)dx+=wrapper.styleData->border->width[3];
        if(!wrapper.styleData->padding[3]&&
           (!wrapper.styleData->border||!wrapper.styleData->border->width[3]))dx=0;
    }
    std::vector<NodeRef> result;
    result.reserve(parents.size());
    for(std::size_t i=0;i<parents.size();++i)
        result.push_back(builder.node(u"g",PropertyList{{"data-mml-node",Property(wrapper.node->kind())}}));
    std::size_t index=0;
    for(const auto& parent:parents){
        auto group=result[index];
        NodeRef target=parent;
        if(linked){
            auto anchor=builder.node(u"a",PropertyList{{"href",href}});
            anchor->semantic=LinkPrimitive{property_host::string(href)};
            anchor->paint=ShapePaint{.fill=std::string("blue"),.stroke=std::string("blue"),
                .emitAttributes=false};
            builder.append(parent,anchor);
            target=anchor;
            const auto line=embellished?std::shared_ptr<common::LineBBox>{}:wrapper.getLineBBox(index);
            const auto outer=embellished?wrapper.getOuterBBox():std::shared_ptr<BBox>{};
            const auto& box=outer?*outer:*line;
            PropertyList hitbox{{"data-hitbox",Property(true)},{"fill",Property("none")},
                {"stroke",Property("none")},{"pointer-events",Property("all")},
                {"width",Property(fixed(wrapper,box.w))},{"height",Property(fixed(wrapper,box.h+box.d))},
                {"x",(index==0||embellished)?Property(fixed(wrapper,-dx)):Property(0)},
                {"y",Property(fixed(wrapper,-box.d))}};
            auto rect=builder.node(u"rect",std::move(hitbox));
            rect->geometry=RectGeometry{(index==0||embellished)?-dx:0,-box.d,
                                        box.w,box.h+box.d};
            builder.append(group,rect);
        }
        builder.append(target,group);
        ++index;
    }
    return result;
}

SvgLineStack createSvgLineStack(common::CommonWrapper& wrapper,Builder& builder,
                                std::size_t breakCount,std::span<const NodeRef> parents){
    if(parents.empty())folia::math::property_host::typeError("Cannot read properties of undefined (reading 'append')");
    const auto kind=wrapper.node->kind();
    const bool inferred=wrapper.node->isInferred();
    PropertyList definition=inferred
        ?PropertyList{{"data-mjx-linestack",Property(true)}}
        :PropertyList{{"data-mml-node",Property(kind)}};
    auto outer=builder.node(u"g",std::move(definition));
    NodeRef container=parents.front();
    const auto attributes=wrapper.node->attributes();
    const auto href=attributes->get("href");
    if(truthy(href)){
        const auto anchor=builder.node(u"a",PropertyList{{"href",href}});
        anchor->semantic=LinkPrimitive{property_host::string(href)};
        anchor->paint=ShapePaint{.fill=std::string("blue"),.stroke=std::string("blue"),
            .emitAttributes=false};
        const bool embellished=wrapper.node->isEmbellished();
        const auto line=embellished?std::shared_ptr<common::LineBBox>{}:wrapper.getLineBBox(0);
        const auto outerBox=embellished?wrapper.getOuterBBox():std::shared_ptr<BBox>{};
        const auto& box=outerBox?*outerBox:*line;
        auto hitbox=builder.node(u"rect",PropertyList{
            {"data-hitbox",Property(true)},{"fill",Property("none")},
            {"stroke",Property("none")},{"pointer-events",Property("all")},
            {"width",Property(fixed(wrapper,box.w))},{"height",Property(fixed(wrapper,box.h+box.d))},
            {"x",Property(fixed(wrapper,0))},{"y",Property(fixed(wrapper,-box.d))}});
        hitbox->geometry=RectGeometry{0,-box.d,box.w,box.h+box.d};
        builder.append(outer,hitbox);
        builder.append(anchor,outer);
        builder.append(container,anchor);
    }else{
        builder.append(container,outer);
    }
    SvgLineStack result;
    result.outer=outer;
    result.lines.reserve(breakCount+1);
    for(std::size_t index=0;index<=breakCount;++index){
        auto line=builder.node(u"g",PropertyList{
            {"data-mjx-linebox",Property(true)},
            {"data-mjx-lineno",Property(static_cast<double>(index))}});
        builder.append(outer,line);
        result.lines.push_back(std::move(line));
    }
    return result;
}

void decorateSvgWrapper(common::CommonWrapper& wrapper,Builder& builder,std::span<const NodeRef> nodes,
                        std::size_t lineIndex){
    if(nodes.empty())return;
    if(wrapper.styles){
        const auto css=wrapper.styles->cssText();
        if(!css.empty())put(nodes,"style",Property(folia::math::property_host::debugText(css)));
    }
    double dx=0;
    const auto styleData=wrapper.styleData;
    if(styleData){
        const double padding=styleData->padding[3];
        const double border=styleData->border?styleData->border->width[3]:0;
        if(padding||border)dx=padding+border;
        if(styleData->margin[3])for(const auto& node:nodes){
            if(!node->transformAttributeIndex)
                node->transformAttributeIndex=node->attributes.keys().size();
            node->transform=Transform{0,0,1,1,{
                {TransformKind::Translate,styleData->margin[3],0}}};
        }
    }
    const auto box=wrapper.getBBox();
    if(box->rscale!=1)for(const auto& node:nodes){
        // Source handleScale overwrites a margin transform; place() later
        // prepends translation to this scale during child placement.
        TransformOperation scale{TransformKind::Scale,box->rscale,box->rscale};
        scale.scalarScale=true;
        node->transform=Transform{0,0,1,1,{scale}};
        if(!node->transformAttributeIndex)
            node->transformAttributeIndex=node->attributes.keys().size();
    }

    // SvgWrapper.handleBorder runs before handleColor, and both insert before
    // the current first child.  Consequently backgrounds precede borders in
    // final paint order, while later child glyphs remain above both.
    if(styleData&&styleData->border){
        const auto& border=*styleData->border;
        const double fuzz=.005;
        const std::size_t n=nodes.size()-1;
        for(std::size_t k=0;k<nodes.size();++k){
            const double left=(!n||!k)?1:0;
            const double right=(!n||k==n)?1:0;
            const auto line=wrapper.node->isEmbellished()&&!wrapper.node->isKind("mo")
                ?std::shared_ptr<common::LineBBox>{}:wrapper.getLineBBox(k);
            const auto outer=(wrapper.node->isEmbellished()&&!wrapper.node->isKind("mo"))
                ?wrapper.getOuterBBox():std::shared_ptr<BBox>{};
            const auto& bbox=outer?*outer:*line;
            const double h=bbox.h-styleData->margin[0]+fuzz;
            const double d=bbox.d-styleData->margin[2]+fuzz;
            const double w=bbox.w-styleData->margin[1]-styleData->margin[3]+fuzz;
            const Point ort{w,h},olt{-fuzz,h},orb{w,-d},olb{-fuzz,-d};
            const Point irt{w-right*border.width[1],h-border.width[0]};
            const Point ilt{-fuzz+left*border.width[3],h-border.width[0]};
            const Point irb{w-right*border.width[1],-d+border.width[2]};
            const Point ilb{-fuzz+left*border.width[3],-d+border.width[2]};
            const std::array<std::array<Point,4>,4> paths{{
                {olt,ort,irt,ilt},{orb,ort,irt,irb},{olb,orb,irb,ilb},{olb,olt,ilt,ilb}}};
            const auto originalFirst=nodes[k]->children.empty()?NodeRef{}:nodes[k]->children.front();
            const double borderDx=left*dx;
            for(std::size_t side=0;side<4;++side){
                const double thickness=border.width[side];
                if(!thickness||(side==3&&!left)||(side==1&&!right))continue;
                const auto& points=paths[side];
                if(border.style[side]=="dashed"||border.style[side]=="dotted"){
                    const bool dotted=border.style[side]=="dotted";
                    const double half=thickness/2;
                    constexpr std::array<double,4> tx1{1,-1,1,1};
                    constexpr std::array<double,4> ty1{-1,1,1,1};
                    constexpr std::array<double,4> tx2{-1,-1,-1,1};
                    constexpr std::array<double,4> ty2{-1,-1,1,-1};
                    const double x1=points[0].x+tx1[side]*half-borderDx;
                    const double y1=points[0].y+ty1[side]*half;
                    const double x2=points[1].x+tx2[side]*half-borderDx;
                    const double y2=points[1].y+ty2[side]*half;
                    const double length=std::abs(side%2?y2-y1:x2-x1);
                    const double count=dotted?std::ceil(length/(2*thickness)):
                        std::ceil((length-thickness)/(4*thickness));
                    const double segment=length/(4*count+1);
                    PropertyList attrs{{"x1",Property(fixed(wrapper,x1))},{"y1",Property(fixed(wrapper,y1))},
                        {"x2",Property(fixed(wrapper,x2))},{"y2",Property(fixed(wrapper,y2))},
                        {"stroke-width",Property(fixed(wrapper,thickness))},
                        {"stroke",Property(border.color[side])},
                        {"stroke-linecap",Property(dotted?"round":"square")},
                        {"stroke-dasharray",Property(dotted
                            ?"1 "+fixed(wrapper,length/count-.002)
                            :fixed(wrapper,segment)+" "+fixed(wrapper,3*segment))}};
                    auto node=builder.node(u"line",std::move(attrs));
                    node->geometry=LineGeometry{{x1,y1},{x2,y2},thickness};
                    ShapePaint paint;
                    paint.stroke=border.color[side];
                    paint.lineCap=dotted?LineCap::Round:LineCap::Square;
                    // Source addBorderBroken uses a literal 1 SVG unit for
                    // dots, while the remaining pattern values use fixed().
                    paint.dash=dotted?StrokeDash{{.001,length/count-.002}}:
                        StrokeDash{{segment,3*segment}};
                    paint.emitAttributes=false;
                    node->paint=std::move(paint);
                    insertFront(nodes[k],node);
                }else{
                    solidBorder(wrapper,builder,nodes[k],originalFirst,points,border.color[side],borderDx);
                }
            }
        }
    }

    Property color=attr(wrapper,"mathcolor");
    if(!truthy(color))color=attr(wrapper,"color");
    if(truthy(color)){
        put(nodes,"fill",color);put(nodes,"stroke",color);
        const auto value=property_host::debugText(property_host::string(color));
        for(const auto& node:nodes){
            if(!node->paint)node->paint.emplace();
            node->paint->fill=value;
            node->paint->stroke=value;
            node->paint->emitAttributes=false;
        }
    }
    Property background=attr(wrapper,"mathbackground");
    if(!truthy(background))background=attr(wrapper,"background");
    if(!truthy(background)&&wrapper.styles){
        const auto style=wrapper.styles->get(u"background-color");
        background=Property(folia::math::property_host::debugText(style));
    }
    if(truthy(background)){
        const bool embellished=wrapper.node->isEmbellished()&&!wrapper.node->isKind("mo");
        std::size_t i=0;
        for(const auto& parent:nodes){
            const auto line=embellished?std::shared_ptr<common::LineBBox>{}:wrapper.getLineBBox(i++);
            const auto outer=embellished?wrapper.getOuterBBox():std::shared_ptr<BBox>{};
            const auto& bbox=outer?*outer:*line;
            const double x=(i==1||embellished)?-dx:0;
            auto rect=builder.node(u"rect",PropertyList{{"fill",background},
                {"x",(i==1||embellished)?Property(fixed(wrapper,x)):Property(0)},
                {"y",Property(fixed(wrapper,-bbox.d))},{"width",Property(fixed(wrapper,bbox.w))},
                {"height",Property(fixed(wrapper,bbox.h+bbox.d))},{"data-bgcolor",Property(true)}});
            rect->geometry=RectGeometry{x,-bbox.d,bbox.w,bbox.h+bbox.d};
            rect->paint=ShapePaint{.fill=property_host::debugText(property_host::string(background)),
                .emitAttributes=false};
            insertFront(parent,rect);
        }
    }

    const auto attributes=wrapper.node->attributes();
    if(!attributes)folia::math::property_host::typeError("Cannot read properties of null (reading 'getExplicitNames')");
    const auto defaults=attributes->getAllDefaults();
    static const std::map<std::string,bool,std::less<>> skip{
        {"fontfamily",true},{"fontsize",true},{"fontweight",true},{"fontstyle",true},
        {"color",true},{"background",true},{"class",true},{"href",true},
        {"style",true},{"xmlns",true}};
    for(const auto& name:attributes->getExplicitNames()){
        const auto skipped=skip.find(name);
        const bool skipValue=skipped!=skip.end()&&skipped->second;
        if((skipped!=skip.end()&&!skipped->second)||
           (!defaults.has(name)&&!skipValue&&!nodes.front()->attributes.hasOwn(name))){
            const auto value=attributes->getExplicit(name);
            for(const auto& node:nodes)node->attributes.set(name,sceneAttribute(value));
        }
    }
    const auto classes=attributes->get("class");
    if(truthy(classes)){
        auto tokens=folia::math::string_util::split(folia::math::property_host::string(classes));
        std::vector<std::u16string> merged;
        for(const auto& node:nodes){
            const auto old=node->attributes.get("class");
            if(truthy(old))merged=folia::math::string_util::split(folia::math::property_host::string(old));
            else merged.clear();
            for(const auto& token:tokens)
                if(std::find(merged.begin(),merged.end(),token)==merged.end())merged.push_back(token);
            std::u16string joined;
            for(const auto& token:merged){if(!joined.empty())joined.push_back(u' ');joined+=token;}
            node->attributes.set("class",Property(folia::math::property_host::debugText(joined)));
        }
    }
    (void)lineIndex;
}

void placeSvgNode(common::CommonWrapper& wrapper,Builder& builder,const NodeRef& target,
                  double x,double y,bool wrapperRoot){
    if(const auto& observer=builder.layoutObserver();observer&&observer->place)
        // Source place(x,y) omits the element argument when positioning the
        // wrapper's own DOM. The typed target remains the actual Scene node.
        observer->place(wrapper,x,y,wrapperRoot?NodeRef{}:target,!wrapperRoot);
    if(wrapperRoot){
        const auto box=wrapper.getBBox();
        double dx=0;
        if(wrapper.styleData){
            dx=wrapper.styleData->padding[3];
            if(wrapper.styleData->border)dx+=wrapper.styleData->border->width[3];
        }
        x+=dx*box->rscale;
    }
    if(!(x||y))return;

    if(wrapperRoot){
        const auto sourceAttributes=wrapper.node->attributes();
        if(sourceAttributes&&truthy(sourceAttributes->get("id"))){
            if(!target)folia::math::property_host::typeError("Cannot read properties of null (reading 'children')");
            const auto box=wrapper.getBBox();
            const double h=box->h;
            auto originalChildren=target->children.snapshot();
            target->children.clear();
            auto group=builder.node(u"g",PropertyList{{"data-idbox",Property(true)},
                {"transform",Property("translate(0,"+fixed(wrapper,-h)+")")}},
                {},originalChildren.size());
            builder.transform(group,Transform{0,-h,1,1});
            for(const auto& child:originalChildren)builder.append(group,child);
            auto textContent=builder.node(u"#text",{},u"");
            auto text=builder.node(u"text",PropertyList{{"data-id-align",Property(true)}},u"",1);
            builder.append(text,textContent);
            builder.append(target,text);
            builder.append(target,group);
            y+=h*box->rscale;
        }
    }
    if(!target)folia::math::property_host::typeError("Cannot read properties of null (reading 'transform')");
    if(!target->transform&&target->attributes.hasOwn("transform")){
        const auto original=target->attributes.get("transform");
        if(truthy(original))target->transformSuffix=folia::math::property_host::string(original);
    }
    auto transform=target->transform.value_or(Transform{});
    if(transform.operations.empty()){
        if(transform.translateX||transform.translateY)
            transform.operations.push_back(TransformOperation{
                TransformKind::Translate,transform.translateX,transform.translateY});
        if(transform.scaleX!=1||transform.scaleY!=1)
            transform.operations.push_back(TransformOperation{
                TransformKind::Scale,transform.scaleX,transform.scaleY});
        transform.translateX=transform.translateY=0;
        transform.scaleX=transform.scaleY=1;
    }
    // Source place() prepends each translation to the existing expression.
    // Preserve that order even when consecutive translations sum to zero.
    transform.operations.insert(transform.operations.begin(),
        TransformOperation{TransformKind::Translate,x,y});
    target->transform=transform;
}
}
