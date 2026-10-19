/*************************************************************
 * MathJax output/svg/Wrappers/menclose.ts and output/svg/Notation.ts.
 *************************************************************/
module folia.math.output.svg.menclose_surface;
import folia.math.output.common;
import folia.math.util.option_types;
import folia.math.util.property_host;
import folia.math.util.styles;
import folia.math.output.svg.output_jax;
import std;

namespace folia::math::output::svg {
using namespace options;
common::CommonWrapperClassRef registerSvgMencloseClass(
    common::CommonFactoryContextBindings& bindings,ObjectRef actualConstructor){
    common::CommonWrapperClassData metadata;
    metadata.kind="menclose";
    metadata.sourceConstructor=actualConstructor;
    return common::registerCommonWrapperClass<common::CommonMenclose>(
        bindings,actualConstructor,std::move(metadata));
}
namespace {
constexpr std::array<std::u16string_view,29> notationNames={
    u"top",u"right",u"bottom",u"left",u"actuarial",u"madruwb",
    u"updiagonalstrike",u"downdiagonalstrike",u"horizontalstrike",u"verticalstrike",
    u"box",u"roundedbox",u"circle",u"phasorangle",u"uparrow",u"downarrow",
    u"leftarrow",u"rightarrow",u"updownarrow",u"leftrightarrow",
    u"updiagonalarrow",u"northeastarrow",u"southeastarrow",u"northwestarrow",
    u"southwestarrow",u"northeastsouthwestarrow",u"northwestsoutheastarrow",
    u"longdiv",u"radical"};
std::string sourceAttributeString(Context& context,Value value){
    value=context.toPrimitive(std::move(value),PrimitiveHint::stringHint);
    if(const auto* text=std::get_if<std::u16string>(&value.data))
        return property_host::debugText(*text);
    if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?"true":"false";
    if(const auto* number=std::get_if<double>(&value.data))
        return property_host::debugText(property_host::string(Property(*number)));
    if(std::holds_alternative<Null>(value.data))return "null";
    if(std::holds_alternative<Undefined>(value.data))return "undefined";
    throw Exception("TypeError",u"Cannot convert Symbol to string");
}
std::string sourceStyleText(Context& context,ObjectRef sourceStyle){
    styles::Styles style;
    for(const auto& name:context.enumerableStringKeys(Value(sourceStyle))){
        // Match Lite DOMAdaptor.setAttributes: convert CSS kebab names to
        // the camel-cased setStyle argument before Styles normalizes them.
        std::u16string camel;
        for(std::size_t index=0;index<name.size();++index){
            if(name[index]==u'-'&&index+1<name.size()&&name[index+1]>=u'a'&&name[index+1]<=u'z')
                camel+=static_cast<char16_t>(name[++index]-u'a'+u'A');
            else camel+=name[index];
        }
        const auto value=sourceAttributeString(context,
            context.get(Value(sourceStyle),Key(name)));
        style.set(camel,styles::StyleValue(property_host::text(value)));
    }
    return property_host::debugText(style.cssText());
}
PropertyList sourceSvgAttributes(Context& context,Value definition){
    PropertyList result;
    for(const auto& key:context.enumerableStringKeys(definition)){
        const auto value=context.get(definition,Key(key));
        if(key==u"style"){
            if(const auto* object=std::get_if<ObjectRef>(&value.data);
               object&&context.kind(*object)==ObjectKind::object){
                // Source Lite applies object-valued styles one key at a time
                // through setStyle; the Scene surface performs that operation
                // against its canonical Styles owner in withSceneTarget().
                result.setOwn(property_host::debugText(key),Property(std::string{}));
                continue;
            }
        }
        result.setOwn(property_host::debugText(key),
            Property(sourceAttributeString(context,value)));
    }
    return result;
}

Key indexKey(std::size_t index){
    std::u16string key;
    do{key.insert(key.begin(),static_cast<char16_t>(u'0'+index%10));index/=10;}while(index);
    return Key(std::move(key));
}
Value numericArray(Context& context,std::span<const double> values){
    const auto array=context.array(static_cast<std::uint32_t>(values.size()));
    for(std::size_t index=0;index<values.size();++index)
        context.set(Value(array),indexKey(index),Value(values[index]));
    return Value(array);
}
}

struct SvgMencloseSourceSurface::State : std::enable_shared_from_this<State> {
    static common::CommonFactoryContextBindings& requireFactory(
        const std::shared_ptr<common::CommonFactoryContextBindings>& owner){
        if(!owner)throw Exception("TypeError",u"SvgMenclose requires the actual Common factory owner");
        return *owner;
    }
    Context& context;
    MapContextBindings& maps;
    std::shared_ptr<SceneContextNodes> sceneNodes;
    ContextPropertyBindings* properties=nullptr;
    std::shared_ptr<common::CommonFactoryContextBindings> factoryOwner;
    common::CommonFactoryContextBindings& factories;
    std::weak_ptr<void> serviceGraph;
    Renderer renderer;
    PrimitiveRenderer primitiveRenderer;
    adaptors::lite::LiteModule* lite=nullptr;
    std::vector<ObjectRef> activeSceneNodes;

    State(Context& cx,MapContextBindings& mapBindings,
          std::shared_ptr<common::CommonFactoryContextBindings> factoryBindings,
          std::shared_ptr<SceneContextNodes> nodeProjection,
          std::shared_ptr<void> graph)
        :context(cx),maps(mapBindings),sceneNodes(std::move(nodeProjection)),factoryOwner(std::move(factoryBindings)),
         factories(requireFactory(factoryOwner)),serviceGraph(graph){
        if(!graph)
            throw Exception("TypeError",u"SvgMenclose requires the actual retained service graph and Common factory owner");
        if(&cx!=&maps.context()||&cx!=&factories.context())
            throw Exception("TypeError",u"SvgMenclose static bindings must share one Context");
        if(!sceneNodes||&sceneNodes->context()!=&cx)
            throw Exception("TypeError",u"SvgMenclose requires the canonical Scene node projection");
    }
    bool isSceneNode(Value value)const{
        return sceneNodes->isHandle(value);
    }
    scene::NodeRef sceneNode(ObjectRef object)const{return sceneNodes->resolve(object);}
    ObjectRef makeSceneNodeHandle(scene::NodeRef node,std::optional<ObjectRef> owner={},
                                 std::optional<ObjectRef> definition={}){
        return sceneNodes->handle(node,owner,definition);
    }
    void setDefinition(ObjectRef object,ObjectRef definition){
        sceneNodes->setDefinition(object,definition);
    }
    scene::NodeRef nodePayload(ObjectRef object)const{return sceneNode(object);}
    std::optional<ObjectRef> activeSceneNode()const{
        if(activeSceneNodes.empty())return {};
        return activeSceneNodes.back();
    }
    void withAppend(common::CommonMenclose& wrapper,scene::Builder& builder,ObjectRef rootHandle,
                    const std::function<void()>& body){
        if(!lite)throw Exception("Unimplemented",u"SVG menclose Lite append target is not bound");
        const auto instance=wrapper.factory().objectFor(wrapper);
        sceneNodes->setOwner(rootHandle,instance);
        const auto domKey=Key(u"dom");
        const auto oldDom=context.get(Value(instance),domKey);
        const bool hadOwnDom=context.hasOwn(Value(instance),domKey);
        auto dom=context.array(1);
        context.set(Value(dom),Key(u"0"),Value(rootHandle));
        auto restore=[&]{
            if(hadOwnDom)context.set(Value(instance),domKey,oldDom);
            else context.erase(instance,domKey);
        };
        try{
        context.set(Value(instance),domKey,Value(dom));
        auto targetScope=sceneNodes->installTargets(*lite,builder);
        // The render closure is synchronous. Restore the exact own-property
        // state even when a Source callback throws.
        body();
        }catch(...){restore();throw;}
        restore();
    }
    common::CommonMenclose& resolve(const Value& receiver){
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(!object)throw Exception("TypeError",u"SvgMenclose callback receiver is not an object");
        auto& wrapper=factories.resolveWrapper(*object);
        auto* result=dynamic_cast<common::CommonMenclose*>(&wrapper);
        if(!result)throw Exception("TypeError",u"SvgMenclose callback receiver has the wrong native class");
        return *result;
    }
    Value field(common::CommonMenclose& wrapper,std::u16string_view name){
        return context.get(Value(wrapper.factory().objectFor(wrapper)),Key(std::u16string(name)));
    }
    double number(common::CommonMenclose& wrapper,std::u16string_view name){
        return context.toNumber(field(wrapper,name));
    }
    Value array4(const std::array<double,4>& value){return numericArray(context,value);}
    Value array2(const std::array<double,2>& value){return numericArray(context,value);}
    Value call(const Value& receiver,std::u16string_view key,std::span<const Value> args={}){
        const auto method=context.get(receiver,Key(std::u16string(key)));
        const auto* function=std::get_if<ObjectRef>(&method.data);
        if(!function)throw Exception("TypeError",u"Source menclose member is not callable: "+std::u16string(key));
        return context.call(*function,receiver,args);
    }
    ObjectRef definition(std::u16string_view name){
        const auto object=context.object();
        context.set(Value(object),Key(u"renderer"),Value(function(name,u"renderer")));
        if(name==u"radical")
            context.set(Value(object),Key(u"init"),Value(function(name,u"init")));
        context.set(Value(object),Key(u"bbox"),Value(function(name,u"bbox")));
        if(name==u"top"||name==u"right"||name==u"bottom"||name==u"left"||
           name==u"actuarial"||name==u"madruwb"||name==u"box"||name==u"phasorangle")
            context.set(Value(object),Key(u"border"),Value(function(name,u"border")));
        if(name==u"top"||name==u"right"||name==u"bottom"||name==u"left"||
           name==u"actuarial"||name==u"madruwb"||name==u"box"||
           name==u"phasorangle"||name==u"updiagonalarrow"||name==u"northeastarrow"||
           name==u"southeastarrow"||name==u"northwestarrow"||name==u"southwestarrow"||
           name==u"northeastsouthwestarrow"||name==u"northwestsoutheastarrow"||
           name==u"uparrow"||name==u"downarrow"||name==u"leftarrow"||name==u"rightarrow"||
           name==u"updownarrow"||name==u"leftrightarrow")
            context.set(Value(object),Key(u"remove"),Value(removeList(name)));
        if(name==u"radical"){
            context.set(Value(object),Key(u"renderChild"),Value(true));
        }
        return object;
    }
    std::u16string removeList(std::u16string_view name){
        if(name==u"actuarial")return u"top right";
        if(name==u"madruwb")return u"bottom right";
        if(name==u"box")return u"left right top bottom";
        if(name==u"phasorangle")return u"bottom";
        if(name==u"uparrow")return u"verticalstrike";
        if(name==u"downarrow")return u"verticakstrike";
        if(name==u"leftarrow")return u"horizontalstrike";
        if(name==u"rightarrow")return u"horizontalstrike";
        if(name==u"updownarrow")return u"verticalstrike uparrow downarrow";
        if(name==u"leftrightarrow")return u"horizontalstrike leftarrow rightarrow";
        if(name==u"updiagonalarrow")return u"updiagonalstrike northeastarrow";
        if(name==u"northeastarrow")return u"updiagonalstrike updiagonalarrow";
        if(name==u"southeastarrow")return u"downdiagonalstrike";
        if(name==u"northwestarrow")return u"downdiagonalstrike";
        if(name==u"southwestarrow")return u"updiagonalstrike";
        if(name==u"northeastsouthwestarrow")return u"updiagonalstrike northeastarrow updiagonalarrow southwestarrow";
        if(name==u"northwestsoutheastarrow")return u"downdiagonalstrike northwestarrow southeastarrow";
        return {};
    }
    ObjectRef function(std::u16string_view name,std::u16string_view operation){
        const auto weak=std::weak_ptr<State>(shared_from_this());
        auto graph=serviceGraph.lock();
        if(!graph)throw Exception("TypeError",u"SvgMenclose render service graph has expired");
        const auto functionName=std::u16string(name)+u"."+std::u16string(operation);
        const auto result=context.function(functionName,
            [weak,graph=std::move(graph),name=std::u16string(name),operation=std::u16string(operation)](
                Context&,Value receiver,std::span<const Value> args)->Value{
                auto self=weak.lock();
                if(!self)throw Exception("Unimplemented",u"SvgMenclose Source surface has been released");
                // Notation callbacks receive the Common wrapper as their
                // first argument; renderer is normally called as a method of
                // the notation object, while init is a local function call.
                // Never mistake either function's `this` for the node.
                const Value& node=args.empty()?receiver:args.front();
                auto& wrapper=self->resolve(node);
                if(operation==u"renderer"){
                    if(!self->renderer)throw Exception("Unimplemented",
                        u"SVG menclose scene renderer is not installed for notation: "+name);
                    return self->renderer(wrapper,name,args);
                }
                if(operation==u"init"){
                    if(!wrapper.childNodes||!*wrapper.childNodes||(*wrapper.childNodes)->size()==0)
                        throw Exception("TypeError",u"Cannot read properties of undefined (reading 'createMsqrt')");
                    auto* child=dynamic_cast<common::CommonWrapper*>(
                        (*wrapper.childNodes)->get(0).requireNode("createMsqrt"));
                    if(!child)throw Exception("TypeError",u"Menclose radical child is not a wrapper");
                    // Source's notation init callback receives the actual
                    // CommonMenclose instance as its first argument. Its own
                    // `this` is the notation definition object, so method
                    // lookup and assignment must use the canonical wrapper
                    // identity rather than the callback receiver.
                    const auto node=Value(wrapper.factory().objectFor(wrapper));
                    const auto childIdentity=wrapper.factory().objectFor(*child);
                    const std::array<Value,1> createArgs={Value(childIdentity)};
                    const auto square=self->call(node,u"createMsqrt",createArgs);
                    self->context.set(node,Key(u"msqrt"),square);
                    return Undefined{};
                }
                std::array<double,4> result{};
                if(operation==u"border"){
                    const double thickness=self->number(wrapper,u"thickness");
                    if(name==u"top")result[0]=thickness;
                    else if(name==u"right")result[1]=thickness;
                    else if(name==u"bottom")result[2]=thickness;
                    else if(name==u"left")result[3]=thickness;
                    else if(name==u"actuarial"){result[0]=thickness;result[1]=thickness;}
                    else if(name==u"madruwb"){result[2]=thickness;result[1]=thickness;}
                    else if(name==u"box")result.fill(thickness);
                    else if(name==u"phasorangle")result[2]=thickness;
                    return self->array4(result);
                }
                if(operation==u"bbox"){
                    if(name==u"top"||name==u"right"||name==u"bottom"||name==u"left"||
                       name==u"actuarial"||name==u"madruwb"){
                        const double thickness=self->number(wrapper,u"thickness");
                        const double padding=self->number(wrapper,u"padding");
                        if(name==u"top")result[0]=thickness+padding;
                        else if(name==u"right")result[1]=thickness+padding;
                        else if(name==u"bottom")result[2]=thickness+padding;
                        else if(name==u"left")result[3]=thickness+padding;
                        else if(name==u"actuarial"){result[0]=thickness+padding;result[1]=thickness+padding;}
                        else {result[2]=thickness+padding;result[1]=thickness+padding;}
                    }
                    else if(name==u"horizontalstrike"){
                        const double padding=self->number(wrapper,u"padding");
                        result[1]=padding;result[3]=padding;
                    }
                    else if(name==u"verticalstrike"){
                        const double padding=self->number(wrapper,u"padding");
                        result[0]=padding;result[2]=padding;
                    }
                    else if(name==u"box"||name==u"roundedbox"||name==u"circle"||
                            name==u"updiagonalstrike"||name==u"downdiagonalstrike"){
                        const double thickness=self->number(wrapper,u"thickness");
                        const double padding=self->number(wrapper,u"padding");
                        result.fill(thickness+padding);
                    }
                    else if(name==u"phasorangle"){
                        const double padding=self->number(wrapper,u"padding");
                        const double thickness=self->number(wrapper,u"thickness");
                        result={padding,padding/2,padding/2+thickness,3*padding/2+thickness};
                    }
                    else if(name==u"longdiv"){
                        const double padding=self->number(wrapper,u"padding");
                        const double thickness=self->number(wrapper,u"thickness");
                        result={padding+thickness,padding,padding,2*padding+thickness/2};
                    }
                    else if(name==u"radical"){
                        const auto trbl=self->call(Value(wrapper.factory().objectFor(wrapper)),u"sqrtTRBL");
                        const auto* array=std::get_if<ObjectRef>(&trbl.data);
                        if(!array)throw Exception("TypeError",u"sqrtTRBL did not return an array");
                        std::array<double,4> values{};
                        for(std::size_t i=0;i<values.size();++i)
                            values[i]=self->context.toNumber(self->context.get(*array,indexKey(i)));
                        return self->array4(values);
                    }
                    else if(name==u"uparrow"||name==u"downarrow"||name==u"leftarrow"||
                            name==u"rightarrow"||name==u"updownarrow"||name==u"leftrightarrow"){
                        const double padding=self->number(wrapper,u"padding");
                        const double thickness=self->number(wrapper,u"thickness");
                        const auto arrowhead=self->field(wrapper,u"arrowhead");
                        const auto* head=std::get_if<ObjectRef>(&arrowhead.data);
                        if(!head)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'x')");
                        const double x=self->context.toNumber(self->context.get(*head,Key(u"x")));
                        const double dx=self->context.toNumber(self->context.get(*head,Key(u"dx")));
                        const double extent=std::max(padding,thickness*(x+dx+1));
                        if(name==u"uparrow")result={extent,0,padding,0};
                        else if(name==u"downarrow")result={padding,0,extent,0};
                        else if(name==u"leftarrow")result={0,padding,0,extent};
                        else if(name==u"rightarrow")result={0,extent,0,padding};
                        else if(name==u"updownarrow")result={extent,0,extent,0};
                        else result={0,extent,0,extent};
                        if(wrapper.childNodes&&*wrapper.childNodes&&(*wrapper.childNodes)->size()){
                            auto* child=dynamic_cast<common::CommonWrapper*>((*wrapper.childNodes)->get(0).requireNode("getBBox"));
                            if(!child)throw Exception("TypeError",u"Menclose child is not a wrapper");
                            const auto box=child->getBBox();
                            const double edgeThickness=self->number(wrapper,u"thickness");
                            const double y=self->context.toNumber(self->context.get(*head,Key(u"y")));
                            const double edgeV=std::max(0.0,edgeThickness*y-(box->h+box->d)/2);
                            const double edgeH=std::max(0.0,edgeThickness*y-box->w/2);
                            if(name==u"uparrow"||name==u"downarrow"||name==u"updownarrow"){
                                result[1]=std::max(result[1],edgeH);result[3]=std::max(result[3],edgeH);
                            }else{
                                result[0]=std::max(result[0],edgeV);result[2]=std::max(result[2],edgeV);
                            }
                        }
                    }
                    else if(name==u"updiagonalarrow"||name==u"northeastarrow"||
                            name==u"southeastarrow"||name==u"northwestarrow"||
                            name==u"southwestarrow"||name==u"northeastsouthwestarrow"||
                            name==u"northwestsoutheastarrow"){
                        const auto receiver=Value(wrapper.factory().objectFor(wrapper));
                        const auto arrowValue=self->call(receiver,u"arrowData");
                        const auto* arrowObject=std::get_if<ObjectRef>(&arrowValue.data);
                        if(!arrowObject)throw Exception("TypeError",u"arrowData did not return an object");
                        const double arrowAngle=self->context.toNumber(self->context.get(*arrowObject,Key(u"a")));
                        const double arrowX=self->context.toNumber(self->context.get(*arrowObject,Key(u"x")));
                        const double arrowY=self->context.toNumber(self->context.get(*arrowObject,Key(u"y")));
                        const auto head=self->field(wrapper,u"arrowhead");
                        const auto* object=std::get_if<ObjectRef>(&head.data);
                        if(!object)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'x')");
                        const auto ax=self->context.toNumber(self->context.get(*object,Key(u"x")));
                        const auto ay=self->context.toNumber(self->context.get(*object,Key(u"y")));
                        const auto adx=self->context.toNumber(self->context.get(*object,Key(u"dx")));
                        const std::array<Value,2> modArgs={Value(ax+adx),Value(ay)};
                        const auto modValue=self->call(receiver,u"getArgMod",modArgs);
                        const auto* modArray=std::get_if<ObjectRef>(&modValue.data);
                        if(!modArray)throw Exception("TypeError",u"getArgMod did not return an array");
                        const double b=self->context.toNumber(self->context.get(*modArray,indexKey(0)));
                        const double ar=self->context.toNumber(self->context.get(*modArray,indexKey(1)));
                        const double thickness=self->number(wrapper,u"thickness");
                        // CommonDiagonalArrow.bbox measures the unrotated
                        // arrow data. Direction and rotation belong only to
                        // its renderer and do not change these extenders.
                        const double a=arrowAngle;
                        const double dy=arrowY+(b>a?thickness*ar*std::sin(b-a):0);
                        const double dx=arrowX+(b>std::numbers::pi/2-a?thickness*ar*std::sin(b+a-std::numbers::pi/2):0);
                        result={dy,dx,dy,dx};
                    }
                    return self->array4(result);
                }
                throw Exception("Unimplemented",u"Unknown SvgMenclose notation callback");
            });
        return result;
    }

    ObjectRef primitiveMethod(std::u16string_view name){
        const auto weak=std::weak_ptr<State>(shared_from_this());
        auto graph=serviceGraph.lock();
        if(!graph)throw Exception("TypeError",u"SvgMenclose render service graph has expired");
        const auto methodName=std::u16string(name);
        return context.function(methodName,
            [weak,graph=std::move(graph),methodName](Context&,Value receiver,
                std::span<const Value> args)->Value{
                auto self=weak.lock();
                if(!self)throw Exception("Unimplemented",u"SvgMenclose Source surface has been released");
                if(!self->primitiveRenderer)
                    throw Exception("Unimplemented",u"SVG menclose primitive Scene sink is not installed");
                auto& wrapper=self->resolve(receiver);
                const auto sceneValue=self->primitiveRenderer(wrapper,methodName,args);
                const auto* handle=std::get_if<ObjectRef>(&sceneValue.data);
                if(!handle||!self->isSceneNode(sceneValue))
                    throw Exception("TypeError",u"SvgMenclose primitive did not return its canonical Scene node handle");
                if(methodName==u"arrow")return sceneValue;
                const auto definitionValue=self->sceneNodes->definition(*handle);
                if(!definitionValue)
                    throw Exception("TypeError",u"SVG menclose Scene node handle has no Source attribute record");
                const auto svg=self->context.get(receiver,Key(u"svg"));
                const auto* svgFunction=std::get_if<ObjectRef>(&svg.data);
                if(!svgFunction||self->context.kind(*svgFunction)!=ObjectKind::function)
                    throw Exception("TypeError",u"this.svg is not a function");
                std::u16string tag;
                if(methodName==u"box")tag=u"rect";
                else if(methodName==u"fill")tag=u"path";
                else tag=methodName;
                const auto definition=*definitionValue;
                const auto children=self->context.array();
                const std::array<Value,3> svgArgs{Value(std::move(tag)),
                    Value(definition),Value(children)};
                self->activeSceneNodes.push_back(*handle);
                struct ActiveRestore {
                    std::vector<ObjectRef>& stack;
                    ~ActiveRestore(){stack.pop_back();}
                } activeRestore{self->activeSceneNodes};
                auto result=self->context.call(*svgFunction,receiver,svgArgs);
                return result;
            });
    }
};

SvgMencloseSourceSurface::SvgMencloseSourceSurface(Context& context,
    MapContextBindings& maps,std::shared_ptr<common::CommonFactoryContextBindings> factories,
    std::shared_ptr<SceneContextNodes> nodes,std::shared_ptr<void> serviceGraph)
    :state_(std::make_shared<State>(context,maps,std::move(factories),std::move(nodes),std::move(serviceGraph))){}
SvgMencloseSourceSurface::~SvgMencloseSourceSurface()=default;

ObjectRef SvgMencloseSourceSurface::install(ObjectRef constructor){
    auto storage=state_->maps.createStorage();
    const auto map=state_->maps.bind(storage);
    for(const auto name:notationNames)
        state_->maps.set(map,Value(std::u16string(name)),Value(state_->definition(name)));
    state_->context.set(Value(constructor),Key(u"notations"),Value(map));
    const auto prototypeValue=state_->context.get(Value(constructor),Key(u"prototype"));
    const auto* prototype=std::get_if<ObjectRef>(&prototypeValue.data);
    if(!prototype)throw Exception("TypeError",u"SvgMenclose constructor has no prototype object");
    for(const auto name:{u"line",u"box",u"ellipse",u"path",u"fill",u"arrow"})
        state_->context.set(Value(*prototype),Key(name),Value(state_->primitiveMethod(name)));
    // MapContextBindings owns the real MapStorage payload; never replace it.
    // The constructor/notational function objects above independently retain
    // the provider graph as Context-owned callback payload, while State keeps
    // only a weak graph edge, so no Graph→surface→Graph cycle is formed.
    return map;
}
void SvgMencloseSourceSurface::withSceneTarget(common::CommonMenclose& wrapper,
    scene::Builder& builder,const std::function<void()>& body){
    if(!body)throw Exception("TypeError",u"SvgMenclose scene target requires a body");
    auto* jax=dynamic_cast<SvgOutputJax*>(&wrapper.jax());
    if(!jax)throw Exception("TypeError",u"SvgMenclose requires the actual SvgOutputJax");
    auto previous=jax->exchangeSceneSvgTarget(
        [state=state_,&builder,&wrapper](std::span<const Value> args)->std::optional<Value>{
            if(args.size()<3)
                throw Exception("TypeError",u"SvgJax.svg requires type, attributes, and children");
            const auto active=state->activeSceneNode();
            if(!active)return {};
            const auto node=state->nodePayload(*active);
            if(!node)throw Exception("TypeError",u"SvgJax.svg Scene node handle does not contain a node");
            const auto* kind=std::get_if<std::u16string>(&args[0].data);
            if(!kind)throw Exception("TypeError",u"SvgJax.svg kind must be a string");
            if(!state->properties)
                throw Exception("Unimplemented",u"Menclose property bridge is not installed");
            auto attributes=sourceSvgAttributes(state->context,args[1]);
            const auto style=state->context.get(args[1],Key(u"style"));
            if(const auto* styleObject=std::get_if<ObjectRef>(&style.data);
               styleObject&&state->context.kind(*styleObject)==ObjectKind::object){
                const auto css=sourceStyleText(state->context,*styleObject);
                if(css.empty())attributes.erase("style");
                else attributes.setOwn("style",Property(css));
            }
            node->attributes=attributes;
            if(const auto& observer=builder.layoutObserver();observer&&observer->paint)
                observer->paint(&wrapper,*kind,attributes,0);
            return Value(*active);
        });
    struct Restore {
        SvgOutputJax& jax;
        SvgOutputJax::SceneSvgTarget previous;
        ~Restore(){(void)jax.exchangeSceneSvgTarget(std::move(previous));}
    } restore{*jax,std::move(previous)};
    body();
}
Context& SvgMencloseSourceSurface::context()const{return state_->context;}
ContextPropertyBindings& SvgMencloseSourceSurface::propertyBindings()const{
    if(!state_->properties)throw Exception("Unimplemented",u"Menclose property bridge is not installed");
    return *state_->properties;
}
void SvgMencloseSourceSurface::bindPropertyBindings(ContextPropertyBindings& properties){
    if(&properties.context()!=&state_->context)
        throw Exception("TypeError",u"Menclose property bridge belongs to a different Context");
    if(state_->properties&&state_->properties!=&properties)
        throw Exception("TypeError",u"Menclose property bridge is already bound");
    state_->properties=&properties;
}
void SvgMencloseSourceSurface::bindRenderer(Renderer renderer){state_->renderer=std::move(renderer);}
SvgMencloseSourceSurface::Renderer SvgMencloseSourceSurface::exchangeRenderer(Renderer renderer){
    return std::exchange(state_->renderer,std::move(renderer));
}
void SvgMencloseSourceSurface::bindPrimitiveRenderer(PrimitiveRenderer renderer){
    state_->primitiveRenderer=std::move(renderer);
}
SvgMencloseSourceSurface::PrimitiveRenderer SvgMencloseSourceSurface::exchangePrimitiveRenderer(PrimitiveRenderer renderer){
    return std::exchange(state_->primitiveRenderer,std::move(renderer));
}
void SvgMencloseSourceSurface::bindLiteModule(adaptors::lite::LiteModule& lite){
    if(&lite.context()!=&state_->context)
        throw options::Exception("TypeError",u"Menclose LiteModule belongs to a different Context");
    state_->lite=&lite;
}
bool SvgMencloseSourceSurface::isSceneNodeHandle(const Value& value)const{
    return state_->isSceneNode(value);
}
common::CommonWrapper& SvgMencloseSourceSurface::resolveWrapper(ObjectRef object)const{
    return state_->factories.resolveWrapper(object);
}
ObjectRef SvgMencloseSourceSurface::createSceneNodeHandle(scene::NodeRef node){
    return state_->makeSceneNodeHandle(std::move(node));
}
void SvgMencloseSourceSurface::withPresentationAttribute(ObjectRef handle,
    std::u16string_view key,const Value& value,const std::function<void()>& body){
    state_->sceneNodes->withPresentationAttribute(handle,key,value,body);
}
void SvgMencloseSourceSurface::setSceneNodeDefinition(ObjectRef handle,ObjectRef definition){
    state_->setDefinition(handle,definition);
}
scene::NodeRef SvgMencloseSourceSurface::sceneNodePayload(ObjectRef handle)const{
    return state_->nodePayload(handle);
}
std::optional<ObjectRef> SvgMencloseSourceSurface::activeSceneNodeHandle()const{
    return state_->activeSceneNode();
}
void SvgMencloseSourceSurface::withSceneAppend(common::CommonMenclose& wrapper,
    scene::Builder& builder,ObjectRef rootHandle,const std::function<void()>& body){
    state_->withAppend(wrapper,builder,rootHandle,body);
}
Value SvgMencloseSourceSurface::appendSceneNode(common::CommonMenclose& wrapper,
    ObjectRef parentHandle,ObjectRef childHandle){
    const auto instance=wrapper.factory().objectFor(wrapper);
    const auto adaptor=state_->context.get(Value(instance),Key(u"adaptor"));
    const auto method=state_->context.get(adaptor,Key(u"append"));
    const auto* function=std::get_if<ObjectRef>(&method.data);
    if(!function)throw Exception("TypeError",u"adaptor.append is not callable");
    const std::array<Value,2> args{Value(parentHandle),Value(childHandle)};
    return state_->context.call(*function,adaptor,args);
}
Value SvgMencloseSourceSurface::render(common::CommonMenclose& wrapper,
    std::u16string_view name,std::span<const Value> args){
    if(!state_->renderer)throw Exception("Unimplemented",u"SVG menclose Scene target is not bound");
    return state_->renderer(wrapper,name,args);
}
}
