/*************************************************************
 * MathJax output/svg/Wrappers/menclose.ts and Notation.ts, lowered to
 * backend-independent typed Scene nodes. The actual CommonMenclose wrapper,
 * its live Source notation Map, and the one LayoutJob remain authoritative.
 *************************************************************/
module folia.math.output.scene;
import :menclose;
import folia.math.output.scene.foundation;
import folia.math.core.tree.property;
import folia.math.util.option_types;
import folia.math.util.context_property_bindings;
import folia.math.util.property_host;
import folia.math.util.string;
import folia.math.util.js_number;
import std;

namespace folia::math::output::scene {
namespace {
using namespace options;
struct Box {double w=0,h=0,d=0;};
std::u16string index(std::size_t value){
    std::u16string result;
    do{result.insert(result.begin(),static_cast<char16_t>(u'0'+value%10));value/=10;}while(value);
    return result;
}
Value callMember(Context& context,ObjectRef receiver,std::u16string_view name,
                 std::span<const Value> args={}){
    const auto value=context.get(Value(receiver),Key(std::u16string(name)));
    const auto* method=std::get_if<ObjectRef>(&value.data);
    if(!method)throw Exception("TypeError",u"Source SVG wrapper method is not callable: "+std::u16string(name));
    return context.call(*method,Value(receiver),args);
}
double number(Context& context,const Value& value){return context.toNumber(value);}
bool sourceTruthyNumber(double value){return value!=0&&!std::isnan(value);}
Box bbox(Context& context,ObjectRef receiver){
    const auto value=callMember(context,receiver,u"getBBox");
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object)throw Exception("TypeError",u"getBBox did not return an object");
    return {number(context,context.get(*object,Key(u"w"))),
            number(context,context.get(*object,Key(u"h"))),
            number(context,context.get(*object,Key(u"d")))};
}
double readField(Context& context,ObjectRef receiver,std::u16string_view key){
    return number(context,context.get(receiver,Key(std::u16string(key))));
}
// CommonWrapper.fixed defaults to one digit; menclose calls that method
// without an explicit precision for all dimensions and path coordinates.
std::string fixed(common::CommonWrapper& wrapper,double value,unsigned digits=1){
    auto& context=wrapper.jax().optionsContext();
    const auto identity=wrapper.factory().objectFor(wrapper);
    const auto method=context.get(Value(identity),Key(u"fixed"));
    const auto* function=std::get_if<ObjectRef>(&method.data);
    if(!function||context.kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",u"this.fixed is not a function");
    const std::array<Value,2> args{Value(value),Value(static_cast<double>(digits))};
    const auto result=context.call(*function,Value(identity),args);
    const auto* text=std::get_if<std::u16string>(&result.data);
    if(!text)throw Exception("TypeError",u"SvgWrapper.fixed did not return a string");
    return property_host::debugText(*text);
}
std::string jaxFixed(common::CommonWrapper& wrapper,double value,unsigned digits=3){
    auto& context=wrapper.jax().optionsContext();
    const auto identity=wrapper.jax().instanceIdentity();
    const auto method=context.get(Value(identity),Key(u"fixed"));
    const auto* function=std::get_if<ObjectRef>(&method.data);
    if(!function||context.kind(*function)!=ObjectKind::function)
        throw Exception("TypeError",u"this.jax.fixed is not a function");
    const std::array<Value,2> args{Value(value),Value(static_cast<double>(digits))};
    const auto result=context.call(*function,Value(identity),args);
    const auto* text=std::get_if<std::u16string>(&result.data);
    if(!text)throw Exception("TypeError",u"SvgOutputJax.fixed did not return a string");
    return property_host::debugText(*text);
}
Property stringProperty(std::string value){return Property(std::move(value));}
NodeRef line(Builder& builder,common::CommonWrapper& wrapper,
             double x1,double y1,double x2,double y2,double width){
    PropertyList attrs;
    attrs.setOwn("x1",stringProperty(fixed(wrapper,x1)));
    attrs.setOwn("y1",stringProperty(fixed(wrapper,y1)));
    attrs.setOwn("x2",stringProperty(fixed(wrapper,x2)));
    attrs.setOwn("y2",stringProperty(fixed(wrapper,y2)));
    attrs.setOwn("stroke-width",stringProperty(fixed(wrapper,width)));
    auto result=builder.node(u"line",std::move(attrs));
    result->geometry=LineGeometry{{x1,y1},{x2,y2},width};
    // Source line() omits `stroke`: it inherits the currentColor set by the
    // enclosing SVG group.  Keep the typed width here, but do not materialize
    // a synthetic stroke attribute on the line itself.
    result->paint=ShapePaint{.strokeWidth=width};
    return result;
}
NodeRef rect(Builder& builder,common::CommonWrapper& wrapper,double w,double h,double d,
             double thickness,double radius=0){
    PropertyList attrs;
    attrs.setOwn("x",stringProperty(fixed(wrapper,thickness/2)));
    attrs.setOwn("y",stringProperty(fixed(wrapper,thickness/2-d)));
    attrs.setOwn("width",stringProperty(fixed(wrapper,w-thickness)));
    attrs.setOwn("height",stringProperty(fixed(wrapper,h+d-thickness)));
    attrs.setOwn("fill",Property("none"));
    attrs.setOwn("stroke-width",stringProperty(fixed(wrapper,thickness)));
    if(radius){attrs.setOwn("rx",stringProperty(fixed(wrapper,radius)));}
    auto result=builder.node(u"rect",std::move(attrs));
    RectGeometry geometry{thickness/2,thickness/2-d,w-thickness,h+d-thickness};
    if(radius){geometry.rx=radius;geometry.ry=radius;}
    result->geometry=geometry;
    result->paint=ShapePaint{.fill=std::string("none"),.strokeWidth=thickness};
    return result;
}
NodeRef ellipse(Builder& builder,common::CommonWrapper& wrapper,double w,double h,double d,
                double thickness){
    const double rx=(w-thickness)/2,ry=(h+d-thickness)/2,cx=w/2,cy=(h-d)/2;
    PropertyList attrs;
    attrs.setOwn("rx",stringProperty(fixed(wrapper,rx)));
    attrs.setOwn("ry",stringProperty(fixed(wrapper,ry)));
    attrs.setOwn("cx",stringProperty(fixed(wrapper,cx)));
    attrs.setOwn("cy",stringProperty(fixed(wrapper,cy)));
    attrs.setOwn("fill",Property("none"));
    attrs.setOwn("stroke-width",stringProperty(fixed(wrapper,thickness)));
    auto result=builder.node(u"ellipse",std::move(attrs));
    result->geometry=EllipseGeometry{cx,cy,rx,ry};
    result->paint=ShapePaint{.fill=std::string("none"),.strokeWidth=thickness};
    return result;
}
std::string pathData(common::CommonWrapper& wrapper,
                     std::span<const std::variant<std::string,double>> tokens){
    std::string result;
    for(const auto& handle:tokens){
        if(!result.empty())result.push_back(' ');
        if(const auto* command=std::get_if<std::string>(&handle))result+=*command;
        else result+=fixed(wrapper,std::get<double>(handle));
    }
    return result;
}
NodeRef path(Builder& builder,common::CommonWrapper& wrapper,
             std::span<const std::variant<std::string,double>> tokens,
             double thickness,bool fill,std::string_view join="round"){
    const auto d=pathData(wrapper,tokens);
    PropertyList style{{"stroke-width",stringProperty(fixed(wrapper,thickness))}};
    PropertyList attrs;
    attrs.setOwn("d",Property(d));
    if(!fill)attrs.setOwn("style",Property(style));
    if(!fill){attrs.setOwn("stroke-linecap",Property("round"));attrs.setOwn("stroke-linejoin",Property(std::string(join)));}
    if(!fill)attrs.setOwn("fill",Property("none"));
    auto result=builder.node(u"path",std::move(attrs));
    // Source path()/fill() format numeric parts only when creating d. Keep
    // those original parts in the decoded outline, before SVG rounding.
    // String parts (including arc flags such as "0,1") remain unchanged.
    std::string precise;
    for(const auto& token:tokens){
        if(!precise.empty())precise.push_back(' ');
        if(const auto* command=std::get_if<std::string>(&token))precise+=*command;
        else precise+=js_number::toString(std::get<double>(token)*1000);
    }
    auto outline=std::make_shared<GlyphOutline>(*decodeGlyphOutline(std::move(precise)));
    outline->sourcePath=d;
    result->geometry=PathGeometry{std::move(outline)};
    // Both Source path() and fill() inherit stroke/fill from the flipped SVG
    // group unless their own call explicitly includes a corresponding attr.
    // path() supplies fill="none"; fill() supplies only d.  Preserve those
    // semantics without making Serializer invent local paint attributes.
    result->paint=fill?ShapePaint{}:ShapePaint{
        .fill=std::string("none"),.strokeWidth=thickness,
        .lineCap=LineCap::Round,
        .lineJoin=join=="mitre"?LineJoin::Miter:LineJoin::Round};
    return result;
}
std::array<double,4> lineData(std::u16string_view name,const Box& box,double thickness){
    const double t=thickness/2;
    if(name==u"top")return {0,box.h-t,box.w,box.h-t};
    if(name==u"right")return {box.w-t,-box.d,box.w-t,box.h};
    if(name==u"bottom")return {0,t-box.d,box.w,t-box.d};
    if(name==u"left")return {t,-box.d,t,box.h};
    if(name==u"verticalstrike")return {box.w/2,box.h,box.w/2,-box.d};
    if(name==u"horizontalstrike")return {0,(box.h-box.d)/2,box.w,(box.h-box.d)/2};
    if(name==u"updiagonalstrike")return {t,t-box.d,box.w-t,box.h-t};
    return {t,box.h-t,box.w-t,t-box.d};
}
}

void MenclosePlacement::placeContent(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> roots){
    auto* wrapper=dynamic_cast<common::CommonMenclose*>(&base);
    if(!wrapper)throw Exception("TypeError",u"menclose placement received an incompatible wrapper");
    if(roots.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'append')");
    auto& context=wrapper->jax().optionsContext();
    source_->withSceneTarget(*wrapper,builder,[&]{
        const auto rootHandle=source_->createSceneNodeHandle(roots.front());
        auto previousPrimitiveRenderer=source_->exchangePrimitiveRenderer([&builder,source=source_](common::CommonMenclose& current,
            std::u16string_view method,std::span<const Value> args)->Value{
            auto& cx=current.jax().optionsContext();
            auto priorObserver=builder.layoutObserver();
            // arrow() invokes fill() through the live Source method. Its nested
            // SVG creation must keep the enclosing builder observer available.
            // Only the direct geometry allocation precedes the Source svg() call.
            if(method!=u"arrow")(void)builder.exchangeLayoutObserver({});
            struct ObserverRestore {
                Builder& builder;
                LayoutEventObserverRef previous;
                ~ObserverRestore(){(void)builder.exchangeLayoutObserver(std::move(previous));}
            } observerRestore{builder,std::move(priorObserver)};
            const auto numberAt=[&](std::size_t i,double fallback=0.0){
                return i<args.size()?cx.toNumber(args[i]):fallback;
            };
            const auto stringAt=[&](std::size_t i)->std::string{
                if(i>=args.size())return {};
                if(const auto* text=std::get_if<std::u16string>(&args[i].data))return property_host::debugText(*text);
                throw Exception("TypeError",u"SVG menclose path join must be a string");
            };
            NodeRef node;
            if(method==u"line"){
                if(args.empty())throw Exception("TypeError",u"Cannot destructure undefined as an iterator");
                const auto* tuple=std::get_if<ObjectRef>(&args[0].data);
                if(!tuple)throw Exception("TypeError",u"SvgMenclose.line requires its coordinate tuple");
                const double x1=cx.toNumber(cx.get(*tuple,Key(u"0")));
                const double y1=cx.toNumber(cx.get(*tuple,Key(u"1")));
                const double x2=cx.toNumber(cx.get(*tuple,Key(u"2")));
                const double y2=cx.toNumber(cx.get(*tuple,Key(u"3")));
                const auto identity=current.factory().objectFor(current);
                const double thickness=cx.toNumber(cx.get(Value(identity),Key(u"thickness")));
                node=line(builder,current,x1,y1,x2,y2,thickness);
            }else if(method==u"box"){
                node=rect(builder,current,numberAt(0),numberAt(1),numberAt(2),
                          cx.toNumber(cx.get(Value(current.factory().objectFor(current)),Key(u"thickness"))),
                          numberAt(3));
            }else if(method==u"ellipse"){
                node=ellipse(builder,current,numberAt(0),numberAt(1),numberAt(2),
                             cx.toNumber(cx.get(Value(current.factory().objectFor(current)),Key(u"thickness"))));
            }else if(method==u"path"||method==u"fill"){
                const bool fill=method==u"fill";
                std::size_t begin=fill?0:1;
                std::vector<std::variant<std::string,double>> tokens;
                for(std::size_t i=begin;i<args.size();++i){
                    if(const auto* text=std::get_if<std::u16string>(&args[i].data))
                        tokens.emplace_back(property_host::debugText(*text));
                    else tokens.emplace_back(cx.toNumber(args[i]));
                }
            node=path(builder,current,tokens,
                    cx.toNumber(cx.get(Value(current.factory().objectFor(current)),Key(u"thickness"))),fill,
                    fill?"round":(args.empty()?"round":stringAt(0)));
            }else if(method==u"arrow"){
                const auto identity=current.factory().objectFor(current);
                const auto box=bbox(cx,identity);
                const double W=numberAt(0),angle=numberAt(1);
                const bool doubled=options::truthy(args.size()>2?args[2]:Value(false));
                const std::u16string offset=args.size()>3&&std::holds_alternative<std::u16string>(args[3].data)
                    ?std::get<std::u16string>(args[3].data):std::u16string{};
                const double dist=numberAt(4);
                const double t=cx.toNumber(cx.get(Value(identity),Key(u"thickness")));
                const auto headValue=cx.get(Value(identity),Key(u"arrowhead"));
                const auto* head=std::get_if<ObjectRef>(&headValue.data);
                if(!head)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'x')");
                const double hx=t*cx.toNumber(cx.get(*head,Key(u"x")));
                const double hy=t*cx.toNumber(cx.get(*head,Key(u"y")));
                const double hdx=t*cx.toNumber(cx.get(*head,Key(u"dx")));
                const double dw=(W-box.w)/2,m=(box.h-box.d)/2,t2=t/2;
                std::vector<std::variant<std::string,double>> tokens;
                if(doubled)tokens={std::string("M"),box.w+dw,m,std::string("l"),-(hx+hdx),hy,
                    std::string("l"),hdx,t2-hy,std::string("L"),hx-dw,m+t2,
                    std::string("l"),hdx,hy-t2,std::string("l"),-(hx+hdx),-hy,
                    std::string("l"),hx+hdx,-hy,std::string("l"),-hdx,hy-t2,
                    std::string("L"),box.w+dw-hx,m-t2,std::string("l"),-hdx,t2-hy,std::string("Z")};
                else tokens={std::string("M"),box.w+dw,m,std::string("l"),-(hx+hdx),hy,
                    std::string("l"),hdx,t2-hy,std::string("L"),-dw,m+t2,
                    std::string("l"),0.0,-t,std::string("L"),box.w+dw-hx,m-t2,
                    std::string("l"),-hdx,t2-hy,std::string("Z")};
                std::vector<TransformOperation> operations;
                if(sourceTruthyNumber(dist)){
                    TransformOperation translate;translate.kind=TransformKind::Translate;
                    if(offset==u"X")translate.x=-dist;else translate.y=dist;
                    operations.push_back(translate);
                }
                if(sourceTruthyNumber(angle)){
                    TransformOperation rotate;rotate.kind=TransformKind::Rotate;
                    rotate.degrees=-angle*180/std::numbers::pi;
                    rotate.centerX=box.w/2;rotate.centerY=m;
                    operations.push_back(rotate);
                }
                std::vector<Value> fillArgs;
                fillArgs.reserve(tokens.size());
                for(const auto& handle:tokens){
                    if(const auto* command=std::get_if<std::string>(&handle))
                        fillArgs.emplace_back(property_host::text(*command));
                    else fillArgs.emplace_back(std::get<double>(handle));
                }
                const auto fillMethod=cx.get(Value(identity),Key(u"fill"));
                const auto* fillFunction=std::get_if<ObjectRef>(&fillMethod.data);
                if(!fillFunction||cx.kind(*fillFunction)!=ObjectKind::function)
                    throw Exception("TypeError",u"this.fill is not a function");
                const auto fillResult=cx.call(*fillFunction,Value(identity),fillArgs);
                const auto* fillHandle=std::get_if<ObjectRef>(&fillResult.data);
                if(!fillHandle||!source->isSceneNodeHandle(fillResult))
                    throw Exception("TypeError",u"SvgMenclose.fill did not return its Scene node");
                auto fillNode=source->sceneNodePayload(*fillHandle);
                if(!fillNode)throw Exception("TypeError",u"SvgMenclose.fill handle has no Scene node");
                if(!fillNode->transform)fillNode->transform.emplace();
                fillNode->transform->operations=operations;
                std::vector<std::string> transforms;
                for(const auto& operation:operations){
                    if(operation.kind==TransformKind::Translate){
                        transforms.push_back("translate("+jaxFixed(current,operation.x*1000)+" "+
                            jaxFixed(current,operation.y*1000)+")");
                    }else if(operation.kind==TransformKind::Rotate){
                        transforms.push_back("rotate("+jaxFixed(current,operation.degrees)+" "+
                            fixed(current,operation.centerX)+" "+
                            fixed(current,operation.centerY)+")");
                    }
                }
                if(!transforms.empty()){
                    std::string transform;
                    for(const auto& item:transforms){if(!transform.empty())transform.push_back(' ');transform+=item;}
                    const auto adaptor=cx.get(Value(identity),Key(u"adaptor"));
                    const auto setter=cx.get(adaptor,Key(u"setAttribute"));
                    const auto* setterFunction=std::get_if<ObjectRef>(&setter.data);
                    if(!setterFunction||cx.kind(*setterFunction)!=ObjectKind::function)
                        throw Exception("TypeError",u"this.adaptor.setAttribute is not a function");
                    const std::array<Value,3> setterArgs={fillResult,Value(u"transform"),
                        Value(property_host::text(transform))};
                source->withPresentationAttribute(*fillHandle,u"transform",setterArgs[2],[&]{
                    (void)cx.call(*setterFunction,adaptor,setterArgs);
                });
                }
                return fillResult;
            }else{
                throw Exception("Unimplemented",u"SvgMenclose primitive method is not implemented: "+std::u16string(method));
            }
            const auto handle=source->createSceneNodeHandle(node);
            const auto definition=cx.object();
            for(const auto& key:node->attributes.keys()){
                const auto value=node->attributes.getOwn(key);
                const auto* text=std::get_if<std::string>(&value.value);
                if(!text)throw Exception("TypeError",u"Menclose SVG attribute was not materialized as a string");
                cx.set(Value(definition),Key(property_host::text(key)),
                       Value(property_host::text(*text)));
            }
            if(method==u"path"){
                const auto identity=current.factory().objectFor(current);
                const double thickness=cx.toNumber(cx.get(Value(identity),Key(u"thickness")));
                const auto style=cx.object();
                cx.set(Value(style),Key(u"stroke-width"),
                    Value(property_host::text(fixed(current,thickness))));
                cx.set(Value(definition),Key(u"style"),Value(style));
            }
            source->setSceneNodeDefinition(handle,definition);
            return Value(handle);
        });
        const auto callPrimitive=[&](std::u16string_view method,std::span<const Value> args){
            const auto identity=wrapper->factory().objectFor(*wrapper);
            const auto fn=context.get(Value(identity),Key(std::u16string(method)));
            const auto* function=std::get_if<ObjectRef>(&fn.data);
            if(!function)throw Exception("TypeError",u"SvgMenclose primitive method is not callable");
            return context.call(*function,Value(identity),args);
        };
        const auto callPath=[&](std::u16string_view method,std::string_view join,
            std::span<const std::variant<std::string,double>> tokens){
            std::vector<Value> args;
            if(method==u"path")args.emplace_back(property_host::text(join));
            for(const auto& handle:tokens){
                if(const auto* command=std::get_if<std::string>(&handle))
                    args.emplace_back(property_host::text(*command));
                else args.emplace_back(std::get<double>(handle));
            }
            return callPrimitive(method,args);
        };
        struct PrimitiveRendererRestore {
            MencloseSourceService& source;
            MencloseSourceService::PrimitiveRenderer previous;
            ~PrimitiveRendererRestore(){source.bindPrimitiveRenderer(std::move(previous));}
        } primitiveRendererRestore{*source_,std::move(previousPrimitiveRenderer)};
        source_->withSceneAppend(*wrapper,builder,rootHandle,[&]{
            const auto identity=wrapper->factory().objectFor(*wrapper);
            // Source SvgMenclose calls getBBoxExtenders before creating its block.
            const auto extendersValue=callMember(context,identity,u"getBBoxExtenders");
            const auto* extenders=std::get_if<ObjectRef>(&extendersValue.data);
            if(!extenders)throw Exception("TypeError",u"getBBoxExtenders did not return an array");
            const double left=number(context,context.get(*extenders,Key(u"3")));
            PropertyList blockAttributes;
            if(left>0)blockAttributes.setOwn("transform",Property("translate("+fixed(*wrapper,left)+", 0)"));
            auto block=builder.node(u"g",std::move(blockAttributes));
            if(left>0){
                TransformOperation translation{TransformKind::Translate,left,0};
                translation.commaSpaceSeparated=true;
                block->transform=Transform{0,0,1,1,{translation}};
            }
            auto appendNode=[&](const NodeRef& node){
                source_->appendSceneNode(*wrapper,rootHandle,source_->createSceneNodeHandle(node));
            };
            appendNode(block);

            // Source checks the live renderChild field before choosing the ordinary
            // single-child route. The radical renderer is itself a Source Map value.
            const auto renderChild=context.get(identity,Key(u"renderChild"));
            if(options::truthy(renderChild)){
                const auto* callback=std::get_if<ObjectRef>(&renderChild.data);
                if(!callback)throw Exception("TypeError",u"renderChild is not callable");
                const std::array<Value,2> args{Value(identity),Undefined{}};
                    const auto previous=source_->exchangeRenderer([&](common::CommonMenclose& current,
                    std::u16string_view name,std::span<const Value>){
                    if(&current!=wrapper||name!=u"radical")throw Exception("TypeError",u"Invalid menclose renderChild receiver");
                    const auto msqrt=context.get(identity,Key(u"msqrt"));
                    const auto* squareObject=std::get_if<ObjectRef>(&msqrt.data);
                    if(!squareObject)throw Exception("TypeError",u"Cannot read properties of null (reading 'toSVG')");
                    auto& square=source_->resolveWrapper(*squareObject);
                    const std::array<NodeRef,1> parent={block};
                    (void)placements.build(square,builder,parent);
                    const auto trblValue=callMember(context,identity,u"sqrtTRBL");
                    const auto* trbl=std::get_if<ObjectRef>(&trblValue.data);
                    if(!trbl)throw Exception("TypeError",u"sqrtTRBL did not return an array");
                    const double rootLeft=number(context,context.get(*trbl,Key(u"3")));
                    placeSvgNode(*wrapper,builder,block,-rootLeft,0,true);
                    return Undefined{};
                });
                try{(void)context.call(*callback,Value(Undefined{}),args);}
                catch(...){source_->bindRenderer(previous);throw;}
                source_->bindRenderer(previous);
            }else{
                if(!wrapper->childNodes||!*wrapper->childNodes||(*wrapper->childNodes)->size()==0)
                    throw Exception("TypeError",u"Cannot read properties of undefined (reading 'toSVG')");
                auto* child=dynamic_cast<common::CommonWrapper*>((*wrapper->childNodes)->get(0).requireNode("toSVG"));
                if(!child)throw Exception("TypeError",u"Menclose child is not an SVG wrapper");
                const std::array<NodeRef,1> parent={block};
                auto rendered=placements.build(*child,builder,parent);
                if(!rendered.empty())placeSvgNode(*child,builder,rendered.front(),0,0,true);
            }

            const auto notationsValue=context.get(identity,Key(u"notations"));
            const auto* notations=std::get_if<ObjectRef>(&notationsValue.data);
            if(!notations)throw Exception("TypeError",u"Cannot convert undefined or null to object");
            for(const auto& key:context.ownKeys(*notations)){
                const auto* name=std::get_if<std::u16string>(&key.data);
                if(!name)continue;
                const auto descriptor=context.descriptor(*notations,key);
                if(!descriptor||!descriptor->enumerable)continue;
                const auto definition=context.get(*notations,key);
                const auto childFlag=context.get(definition,Key(u"renderChild"));
                if(options::truthy(childFlag))continue;
                const auto renderer=context.get(definition,Key(u"renderer"));
                const auto* function=std::get_if<ObjectRef>(&renderer.data);
                if(!function)throw Exception("TypeError",u"Notation renderer is not callable");
                const auto previous=source_->exchangeRenderer([&,name=*name](common::CommonMenclose& current,
                    std::u16string_view renderedName,std::span<const Value> args){
                    if(&current!=wrapper||renderedName!=name)throw Exception("TypeError",u"Notation callback receiver mismatch");
                    // SvgMenclose.toSVG appends every notation primitive to svg[0];
                    // only the wrapped child sits inside the translated `block`.
                    auto append=[&](const NodeRef& node){appendNode(node);};
                    if(name==u"top"||name==u"right"||name==u"bottom"||name==u"left"||
                       name==u"updiagonalstrike"||name==u"downdiagonalstrike"||
                       name==u"horizontalstrike"||name==u"verticalstrike"||
                       name==u"actuarial"||name==u"madruwb"){
                        auto emit=[&](std::u16string_view lineKind,std::string_view offset={}){
                            const auto box=bbox(context,identity);
                            const double coordinateThickness=readField(context,identity,u"thickness");
                            auto coordinates=lineData(lineKind,box,coordinateThickness);
                            if(!offset.empty()){
                                const std::array<Value,1> args={Value(property_host::text(offset))};
                                const double delta=number(context,callMember(context,identity,u"getOffset",args));
                                // Notation.lineOffset applies its adjustment only for
                                // a JS-truthy result. In particular NaN is falsey.
                                if(sourceTruthyNumber(delta)){
                                    if(offset=="X"){coordinates[0]-=delta;coordinates[2]-=delta;}
                                    else{coordinates[1]-=delta;coordinates[3]-=delta;}
                                }
                            }
                            const double shapeThickness=readField(context,identity,u"thickness");
                            const std::array<Value,4> coords={Value(coordinates[0]),Value(coordinates[1]),Value(coordinates[2]),Value(coordinates[3])};
                            const auto tuple=context.array(4);
                            for(std::size_t i=0;i<coords.size();++i)context.set(Value(tuple),Key(std::u16string(1,static_cast<char16_t>(u'0'+i))),coords[i]);
                            const std::array<Value,1> lineArgs={Value(tuple)};
                            const auto rendered=callPrimitive(u"line",lineArgs);
                            if(const auto* handle=std::get_if<ObjectRef>(&rendered.data))source_->appendSceneNode(*wrapper,rootHandle,*handle);
                            else throw Exception("TypeError",u"SvgMenclose.line did not return an SVG node");
                            (void)shapeThickness;
                        };
                        if(name==u"actuarial"){emit(u"top");emit(u"right");}
                        else if(name==u"madruwb"){emit(u"bottom");emit(u"right");}
                        else emit(name);
                        return Undefined{};
                    }
                    if(name==u"box"||name==u"roundedbox"){
                        const auto box=bbox(context,identity);
                        double radius=0;
                        if(name==u"roundedbox")radius=readField(context,identity,u"thickness")+
                                                            readField(context,identity,u"padding");
                        const double thickness=readField(context,identity,u"thickness");
                        const std::array<Value,4> shapeArgs={Value(box.w),Value(box.h),Value(box.d),Value(radius)};
                        const auto rendered=callPrimitive(u"box",shapeArgs);
                        if(const auto* handle=std::get_if<ObjectRef>(&rendered.data))source_->appendSceneNode(*wrapper,rootHandle,*handle);
                        else throw Exception("TypeError",u"SvgMenclose.box did not return an SVG node");
                        return Undefined{};
                    }
                    if(name==u"circle"){
                        const auto box=bbox(context,identity);
                        const double thickness=readField(context,identity,u"thickness");
                        const std::array<Value,3> shapeArgs={Value(box.w),Value(box.h),Value(box.d)};
                        const auto rendered=callPrimitive(u"ellipse",shapeArgs);
                        const auto* handle=std::get_if<ObjectRef>(&rendered.data);
                        if(!handle)throw Exception("TypeError",u"SvgMenclose.ellipse did not return an SVG node");
                        source_->appendSceneNode(*wrapper,rootHandle,*handle);
                        return Undefined{};
                    }
                    if(name==u"phasorangle"){
                        const auto box=bbox(context,identity);
                        const double padding=readField(context,identity,u"padding");
                        const std::array<Value,2> args={Value(1.75*padding),Value(box.h+box.d)};
                        const auto angleValue=callMember(context,identity,u"getArgMod",args);
                        const auto* pair=std::get_if<ObjectRef>(&angleValue.data);
                        if(!pair)throw Exception("TypeError",u"getArgMod did not return an array");
                        const double angle=number(context,context.get(*pair,Key(u"0")));
                        const double t=readField(context,identity,u"thickness")/2,hd=box.h+box.d,cosine=std::cos(angle);
                        std::vector<std::variant<std::string,double>> tokens={std::string("M"),box.w,t-box.d,
                            std::string("L"),t+cosine*t,t-box.d,std::string("L"),cosine*hd+t,hd-box.d-t};
                        const auto rendered=callPath(u"path","mitre",tokens);
                        const auto* handle=std::get_if<ObjectRef>(&rendered.data);
                        if(!handle)throw Exception("TypeError",u"SvgMenclose.path did not return an SVG node");
                        source_->appendSceneNode(*wrapper,rootHandle,*handle);
                        return Undefined{};
                    }
                    if(name==u"longdiv"){
                        const auto box=bbox(context,identity);
                        const double t=readField(context,identity,u"thickness")/2;
                        const double padding=readField(context,identity,u"padding");
                        std::vector<std::variant<std::string,double>> tokens={std::string("M"),t,t-box.d,
                            std::string("a"),padding-t/2,(box.h+box.d)/2-4*t,0.0,std::string("0,1"),
                            0.0,box.h+box.d-2*t,std::string("L"),box.w-t,box.h-t};
                        const auto rendered=callPath(u"path","round",tokens);
                        const auto* handle=std::get_if<ObjectRef>(&rendered.data);
                        if(!handle)throw Exception("TypeError",u"SvgMenclose.path did not return an SVG node");
                        source_->appendSceneNode(*wrapper,rootHandle,*handle);
                        return Undefined{};
                    }
                    if(name==u"uparrow"||name==u"downarrow"||name==u"leftarrow"||name==u"rightarrow"||
                       name==u"updownarrow"||name==u"leftrightarrow"||name==u"updiagonalarrow"||
                       name==u"northeastarrow"||name==u"southeastarrow"||name==u"northwestarrow"||
                       name==u"southwestarrow"||name==u"northeastsouthwestarrow"||name==u"northwestsoutheastarrow"){
                        double angle=0, W=0, dist=0;
                        bool doubled=false;
                        std::u16string_view offset;
                        if(name==u"uparrow"){angle=-std::numbers::pi/2;offset=u"X";}
                        else if(name==u"downarrow"){angle=std::numbers::pi/2;offset=u"X";}
                        else if(name==u"leftarrow"){angle=std::numbers::pi;offset=u"Y";}
                        else if(name==u"rightarrow"){angle=0;offset=u"Y";}
                        else if(name==u"updownarrow"){angle=std::numbers::pi/2;doubled=true;offset=u"X";}
                        else if(name==u"leftrightarrow"){doubled=true;offset=u"Y";}
                        double direction=1, pi=0;
                        if(name==u"updiagonalarrow"||name==u"northeastarrow"||name==u"southwestarrow"||
                           name==u"northeastsouthwestarrow")direction=-1;
                        if(name==u"northwestarrow"||name==u"southwestarrow")pi=std::numbers::pi;
                        if(name==u"updiagonalarrow"||name==u"northeastarrow"||name==u"southeastarrow"||
                           name==u"northwestarrow"||name==u"southwestarrow"||
                           name==u"northeastsouthwestarrow"||name==u"northwestsoutheastarrow"){
                            const auto aw=callMember(context,identity,u"arrowAW");
                            const auto* pair=std::get_if<ObjectRef>(&aw.data);
                            if(!pair)throw Exception("TypeError",u"arrowAW did not return an array");
                            angle=direction*(number(context,context.get(*pair,Key(u"0")))-pi);
                            W=number(context,context.get(*pair,Key(u"1")));
                            doubled=name==u"northeastsouthwestarrow"||name==u"northwestsoutheastarrow";
                        }else{
                            const auto box=bbox(context,identity);
                            W=(offset==u"X"?box.h+box.d:box.w);
                            const std::array<Value,1> offsetArg={Value(std::u16string(offset))};
                            dist=number(context,callMember(context,identity,u"getOffset",offsetArg));
                        }
                        const std::array<Value,5> arrowArgs={Value(W),Value(angle),Value(doubled),
                            Value(std::u16string(offset)),Value(dist)};
                        const auto rendered=callPrimitive(u"arrow",arrowArgs);
                        const auto* handle=std::get_if<ObjectRef>(&rendered.data);
                        if(!handle)throw Exception("TypeError",u"SvgMenclose.arrow did not return an SVG node");
                        source_->appendSceneNode(*wrapper,rootHandle,*handle);
                        return Undefined{};
                    }
                    throw Exception("Unimplemented",u"SVG menclose renderer is not implemented for this notation");
                });
                try{
                    const std::array<Value,2> args={Value(identity),Undefined{}};
                    (void)context.call(*function,Value(Undefined{}),args);
                }catch(...){source_->bindRenderer(previous);throw;}
                source_->bindRenderer(previous);
            }
        });
    });
}

void registerMenclosePlacement(WrapperFactory& factory,MencloseSourceService& source){
    factory.registerKind("menclose",std::make_shared<MenclosePlacement>(source));
}
}
