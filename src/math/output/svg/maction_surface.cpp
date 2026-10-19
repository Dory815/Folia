/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/maction.ts.
 * The action table is the actual Context Map attached to SvgMaction.actions.
 *************************************************************/
module folia.math.output.svg.maction_surface;
import folia.math.output.common.maction_source;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.regexp_host;
import folia.math.util.context_property_bindings;
import folia.math.util.property_host;
import folia.math.output.svg.output_jax;
import folia.math.output.scene;
import folia.math.core.dom_node;
import std;

namespace folia::math::output::svg {
using namespace options;

struct SvgMactionSourceSurface::State : std::enable_shared_from_this<State> {
    Context& context;
    MapContextBindings& maps;
    std::shared_ptr<common::CommonFactoryContextBindings> factories;
    std::shared_ptr<TimerHost> timers;
    std::weak_ptr<void> serviceGraph;
    ContextPropertyBindings* properties=nullptr;
    adaptors::lite::LiteModule* lite=nullptr;
    std::shared_ptr<SceneContextNodes> sceneNodes;
    ObjectRef actionMap{};
    ObjectRef sourceMathItemState{};
    ObjectRef tooltipData{};

    State(Context& cx,MapContextBindings& mapBindings,
          std::shared_ptr<common::CommonFactoryContextBindings> factoryBindings,
          std::shared_ptr<TimerHost> timerHost,std::shared_ptr<SceneContextNodes> nodes,
          std::shared_ptr<void> graph)
        :context(cx),maps(mapBindings),factories(std::move(factoryBindings)),timers(std::move(timerHost)),
         sceneNodes(std::move(nodes)),serviceGraph(graph){
        if(!factories)throw Exception("TypeError",u"SvgMaction requires Common wrapper bindings");
        if(!timers)throw Exception("TypeError",u"SvgMaction requires the session timer host");
        if(!sceneNodes||&sceneNodes->context()!=&cx)
            throw Exception("TypeError",u"SvgMaction requires the canonical Scene node projection");
        if(!graph)throw Exception("TypeError",u"SvgMaction requires the actual service graph");
        if(&cx!=&maps.context()||&cx!=&factories->context())
            throw Exception("TypeError",u"SvgMaction bindings must share one Context");
    }
    std::string sourceString(Value value){
        value=context.toPrimitive(std::move(value),PrimitiveHint::stringHint);
        if(const auto* text=std::get_if<std::u16string>(&value.data))return property_host::debugText(*text);
        if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?"true":"false";
        if(const auto* number=std::get_if<double>(&value.data))return property_host::debugText(property_host::string(Property(*number)));
        if(std::holds_alternative<Null>(value.data))return "null";
        if(std::holds_alternative<Undefined>(value.data))return "undefined";
        throw Exception("TypeError",u"Cannot convert Symbol to string");
    }
    std::u16string sourceText(Value value){
        value=context.toPrimitive(std::move(value),PrimitiveHint::stringHint);
        if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
        if(const auto* number=std::get_if<double>(&value.data))return property_host::string(Property(*number));
        if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?u"true":u"false";
        if(std::holds_alternative<Null>(value.data))return u"null";
        if(std::holds_alternative<Undefined>(value.data))return u"undefined";
        throw Exception("TypeError",u"Cannot convert Symbol to string");
    }
    PropertyList sourceAttributes(Value definition){
        if(!properties)throw Exception("Unimplemented",u"Maction property projection is not bound");
        PropertyList result;
        for(const auto& key:context.enumerableStringKeys(definition)){
            const auto value=context.get(definition,Key(key));
            if(key==u"style"){
                const auto* object=std::get_if<ObjectRef>(&value.data);
                if(object&&context.kind(*object)==ObjectKind::object){
                    result.setOwn(property_host::debugText(key),properties->from(value));
                    continue;
                }
            }
            result.setOwn(property_host::debugText(key),Property(sourceString(value)));
        }
        return result;
    }

    void appendSourceChild(scene::Builder& builder,const scene::NodeRef& parent,
                           const Value& value,std::size_t depth=0){
        if(depth>64)throw Exception("RangeError",u"Maction SVG content nesting is too deep");
        if(const auto* object=std::get_if<ObjectRef>(&value.data)){
            if(sceneNodes->isHandle(value)){
                builder.append(parent,sceneNodes->resolve(*object));
                return;
            }
            if(context.kind(*object)==ObjectKind::array){
                const auto lengthValue=context.get(*object,Key(u"length"));
                const auto length=context.toNumber(lengthValue);
                if(!std::isfinite(length)||length<0)
                    throw Exception("TypeError",u"Maction SVG content has an invalid length");
                const auto count=static_cast<std::size_t>(length);
                for(std::size_t index=0;index<count;++index){
                    const auto child=context.get(*object,Key(property_host::text(std::to_string(index))));
                    appendSourceChild(builder,parent,child,depth+1);
                }
                return;
            }
        }
        if(const auto* text=std::get_if<std::u16string>(&value.data)){
            builder.append(parent,builder.node(u"#text",{},*text));
            return;
        }
        if(const auto imported=sceneNodes->projectSourceChild(value)){
            builder.append(parent,*imported);
            return;
        }
        if(properties){
            const auto projected=propertyNode(properties->from(value));
            if(std::holds_alternative<DOMNodeRef>(projected))
                throw Exception("TypeError",u"Maction SVG DOM child has no canonical Scene projection");
        }
        throw Exception("TypeError",u"Maction SVG content must contain DOM nodes or strings");
    }

    Value call(Value receiver,std::u16string_view name,std::span<const Value> args={}){
        const auto fn=context.get(receiver,Key(std::u16string(name)));
        const auto* object=std::get_if<ObjectRef>(&fn.data);
        if(!object||context.kind(*object)!=ObjectKind::function)
            throw Exception("TypeError",u"Source member is not callable: "+std::u16string(name));
        return context.call(*object,receiver,args);
    }
    void setFunctionLength(ObjectRef function,std::uint32_t length){
        context.define(function,Key(u"length"),Descriptor::data(Value(static_cast<double>(length)),false,false,true));
    }
    ObjectRef pair(ObjectRef handler,ObjectRef data){
        const auto result=context.array(2);
        context.set(Value(result),Key(u"0"),Value(handler));
        context.set(Value(result),Key(u"1"),Value(data));
        return result;
    }
    ObjectRef styleRule(std::initializer_list<std::pair<std::u16string_view,std::u16string_view>> values){
        const auto result=context.object();
        for(const auto& [name,value]:values)
            context.set(Value(result),Key(std::u16string(name)),Value(std::u16string(value)));
        return result;
    }
    ObjectRef styles(){
        const auto result=context.object();
        auto set=[&](std::u16string_view selector,ObjectRef rule){
            context.set(Value(result),Key(std::u16string(selector)),Value(rule));
        };
        set(u"[jax=\"SVG\"] mjx-tool",styleRule({
            {u"display",u"inline-block"},{u"position",u"relative"},{u"width",u"0"},{u"height",u"0"}}));
        set(u"[jax=\"SVG\"] mjx-tool > mjx-tip",styleRule({
            {u"position",u"absolute"},{u"top",u"0"},{u"left",u"0"}}));
        set(u"mjx-tool > mjx-tip",styleRule({
            {u"display",u"inline-block"},{u"line-height",u"0"},{u"padding",u".2em"},
            {u"border",u"1px solid #888"},{u"background-color",u"#F8F8F8"},{u"color",u"black"},
            {u"box-shadow",u"2px 2px 5px #AAAAAA"}}));
        set(u"g[data-mml-node=\"maction\"][data-toggle]",styleRule({{u"cursor",u"pointer"}}));
        set(u"mjx-status",styleRule({
            {u"display",u"block"},{u"position",u"fixed"},{u"left",u"1em"},{u"bottom",u"1em"},
            {u"min-width",u"25%"},{u"padding",u".2em .4em"},{u"border",u"1px solid #888"},
            {u"font-size",u"90%"},{u"background-color",u"#F8F8F8"},{u"color",u"black"}}));
        set(u"g[data-mjx-collapsed]",styleRule({{u"fill",u"#55F"}}));
        const auto dark=context.object();
        context.set(Value(dark),Key(u"mjx-tool > mjx-tip"),Value(styleRule({
            {u"background-color",u"#303030"},{u"color",u"#E0E0E0"},{u"box-shadow",u"2px 2px 5px #000"}})));
        context.set(Value(dark),Key(u"mjx-status"),Value(styleRule({
            {u"background-color",u"#303030"},{u"color",u"#E0E0E0"}})));
        context.set(Value(dark),Key(u"g[data-mjx-collapsed]"),Value(styleRule({{u"fill",u"#88F"}})));
        set(u"@media (prefers-color-scheme: dark) /* svg maction */",dark);
        return result;
    }
    Value schedule(Value callback,Value delay){
        const auto* function=std::get_if<ObjectRef>(&callback.data);
        if(!function||context.kind(*function)!=ObjectKind::function)
            throw Exception("TypeError",u"setTimeout callback is not callable");
        const auto milliseconds=context.toNumber(delay);
        return Value(static_cast<double>(timers->schedule(milliseconds,*function)));
    }
    void cancel(const Value& handle){timers->cancel(handle);}
    std::shared_ptr<void> graphLease()const{
        auto graph=serviceGraph.lock();
        if(!graph)throw Exception("Unimplemented",u"SvgMaction service graph has been released");
        return graph;
    }
    ObjectRef installHandler(std::u16string name,
        std::function<Value(State&,std::span<const Value>)> body){
        const auto strong=shared_from_this();
        const auto graph=graphLease();
        const auto function=context.function(std::move(name),[strong,graph,body=std::move(body)](
            Context&,Value,std::span<const Value> args)->Value{
                return body(*strong,args);
            });
        setFunctionLength(function,2);
        return function;
    }
    ObjectRef toggle(){
        return installHandler(u"",[](State& self,std::span<const Value> args)->Value{
            if(args.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'dom')");
            const auto receiver=args[0];
            const auto dom=self.context.get(receiver,Key(u"dom"));
            const auto setAttribute=self.context.function(u"",
                [weak=std::weak_ptr<State>(self.shared_from_this()),receiver]
                (Context&,Value,std::span<const Value> itemArgs)->Value{
                    auto state=weak.lock();
                    if(!state)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                    if(itemArgs.empty())throw Exception("TypeError",u"Maction DOM iteration supplied no node");
                    const auto wrapperAdaptor=state->context.get(receiver,Key(u"adaptor"));
                    const auto setAttribute=state->context.get(wrapperAdaptor,Key(u"setAttribute"));
                    const auto* setter=std::get_if<ObjectRef>(&setAttribute.data);
                    if(!setter||state->context.kind(*setter)!=ObjectKind::function)
                        throw Exception("TypeError",u"this.adaptor.setAttribute is not a function");
                    const auto selection=state->context.get(receiver,Key(u"node"));
                    const auto attributes=state->context.get(selection,Key(u"attributes"));
                    const auto get=state->context.get(attributes,Key(u"get"));
                    const auto* getter=std::get_if<ObjectRef>(&get.data);
                    if(!getter||state->context.kind(*getter)!=ObjectKind::function)
                        throw Exception("TypeError",u"this.node.attributes.get is not a function");
                    const std::array<Value,1> selectionKey={Value(u"selection")};
                    const auto selectionValue=state->context.call(*getter,attributes,selectionKey);
                    const std::array<Value,3> callArgs={itemArgs[0],Value(u"data-toggle"),
                        Value(state->sourceText(selectionValue))};
                    return state->context.call(*setter,wrapperAdaptor,callArgs);
                });
            self.setFunctionLength(setAttribute,1);
            const auto forEach=self.context.get(dom,Key(u"forEach"));
            const auto* iterate=std::get_if<ObjectRef>(&forEach.data);
            if(!iterate)throw Exception("TypeError",u"this.dom.forEach is not a function");
            const std::array<Value,1> iterationArgs={Value(setAttribute)};
            (void)self.context.call(*iterate,dom,iterationArgs);
            const auto factory=self.context.get(receiver,Key(u"factory"));
            const auto jax=self.context.get(factory,Key(u"jax"));
            const auto math=self.context.get(jax,Key(u"math"));
            const auto document=self.context.get(jax,Key(u"document"));
            const auto mml=self.context.get(receiver,Key(u"node"));
            const auto click=self.context.function(u"",[weak=std::weak_ptr<State>(self.shared_from_this()),receiver,math,document,mml]
                (Context&,Value,std::span<const Value> eventArgs)->Value{
                    auto state=weak.lock();
                    if(!state)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                    if(eventArgs.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'stopPropagation')");
                    const auto end=state->context.get(math,Key(u"end"));
                    const auto endNode=state->context.get(end,Key(u"node"));
                    if(!options::truthy(endNode)){
                        const auto start=state->context.get(math,Key(u"start"));
                        const auto assignmentEnd=state->context.get(math,Key(u"end"));
                        const auto typesetRoot=state->context.get(math,Key(u"typesetRoot"));
                        state->context.set(assignmentEnd,Key(u"node"),typesetRoot);
                        state->context.set(start,Key(u"node"),typesetRoot);
                        const auto nStart=state->context.get(math,Key(u"start"));
                        const auto nEnd=state->context.get(math,Key(u"end"));
                        state->context.set(nEnd,Key(u"n"),Value(0));
                        state->context.set(nStart,Key(u"n"),Value(0));
                    }
                    (void)state->call(mml,u"nextToggleSelection");
                    const auto rerender=state->context.get(math,Key(u"rerender"));
                    const auto* rerenderFunction=std::get_if<ObjectRef>(&rerender.data);
                    if(!rerenderFunction||state->context.kind(*rerenderFunction)!=ObjectKind::function)
                        throw Exception("TypeError",u"math.rerender is not a function");
                    const auto mmlAttributes=state->context.get(mml,Key(u"attributes"));
                    const auto get=state->context.get(mmlAttributes,Key(u"get"));
                    const auto* getter=std::get_if<ObjectRef>(&get.data);
                    if(!getter||state->context.kind(*getter)!=ObjectKind::function)
                        throw Exception("TypeError",u"this.node.attributes.get is not a function");
                    const std::array<Value,1> idArgs={Value(u"data-maction-id")};
                    const auto id=state->context.call(*getter,mmlAttributes,idArgs);
                    const auto rerenderState=Value(options::truthy(id)
                        ?state->context.get(Value(state->sourceMathItemState),Key(u"ENRICHED"))
                        :state->context.get(Value(state->sourceMathItemState),Key(u"RERENDER")));
                    const std::array<Value,2> rerenderArgs={document,rerenderState};
                    (void)state->context.call(*rerenderFunction,math,rerenderArgs);
                    (void)state->call(eventArgs.front(),u"stopPropagation");
                    return Undefined{};
                });
            self.setFunctionLength(click,1);
            const std::array<Value,2> handlerArgs={Value(u"click"),Value(click)};
            (void)self.call(receiver,u"setEventHandler",handlerArgs);
            return Undefined{};
        });
    }
    ObjectRef tooltip(){
        return installHandler(u"",[](State& self,std::span<const Value> args)->Value{
            if(args.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'childNodes')");
            const auto receiver=args[0];
            const auto actionData=args.size()>1?args[1]:Value(Undefined{});
            const auto children=self.context.get(receiver,Key(u"childNodes"));
            const auto tip=self.context.get(children,Key(u"1"));
            if(!options::truthy(tip))return Undefined{};
            const auto tipNode=self.context.get(tip,Key(u"node"));
            const auto dom=self.context.get(receiver,Key(u"dom"));
            const auto callback=self.context.function(u"",[
                weak=std::weak_ptr<State>(self.shared_from_this()),receiver,tipNode,actionData]
                (Context&,Value,std::span<const Value> callbackArgs)->Value{
                    auto state=weak.lock();
                    if(!state)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                    if(callbackArgs.empty())throw Exception("TypeError",u"DOM iteration supplied no node");
                    const auto domNode=callbackArgs.front();
                    const std::array<Value,1> firstArgs={domNode};
                    const auto rect=state->call(receiver,u"firstChild",firstArgs);
                    const auto isText=options::truthy(state->call(tipNode,u"isKind",
                        std::array<Value,1>{Value(u"mtext")}));
                    if(isText){
                        const auto text=state->call(tipNode,u"getText");
                        const auto adaptor=state->context.get(receiver,Key(u"adaptor"));
                        const auto insert=state->context.get(adaptor,Key(u"insert"));
                        const auto* insertFunction=std::get_if<ObjectRef>(&insert.data);
                        if(!insertFunction||state->context.kind(*insertFunction)!=ObjectKind::function)
                            throw Exception("TypeError",u"this.adaptor.insert is not a function");
                        const auto svg=state->context.get(receiver,Key(u"svg"));
                        const auto* svgFunction=std::get_if<ObjectRef>(&svg.data);
                        if(!svgFunction||state->context.kind(*svgFunction)!=ObjectKind::function)
                            throw Exception("TypeError",u"this.svg is not a function");
                        const auto textMethod=state->context.get(receiver,Key(u"text"));
                        const auto* textFunction=std::get_if<ObjectRef>(&textMethod.data);
                        if(!textFunction||state->context.kind(*textFunction)!=ObjectKind::function)
                            throw Exception("TypeError",u"this.text is not a function");
                        const std::array<Value,1> textArgs={text};
                        const auto textNode=state->context.call(*textFunction,receiver,textArgs);
                        const auto titleChildren=state->context.array(1);
                        state->context.set(Value(titleChildren),Key(u"0"),textNode);
                        const auto definition=state->context.object();
                        const std::array<Value,3> svgArgs={Value(u"title"),Value(definition),Value(titleChildren)};
                        const auto title=state->context.call(*svgFunction,receiver,svgArgs);
                        const std::array<Value,2> insertArgs={title,rect};
                        (void)state->context.call(*insertFunction,adaptor,insertArgs);
                        return Undefined{};
                    }

                    // Source math tooltip constructs and processes the real
                    // temporary MML/DOM nodes before installing either event.
                    const auto adaptor=state->context.get(receiver,Key(u"adaptor"));
                    const auto jax=state->context.get(receiver,Key(u"jax"));
                    const auto container=state->context.get(jax,Key(u"container"));
                    const auto sourceMml=state->context.get(receiver,Key(u"node"));
                    const auto mmlFactory=state->context.get(sourceMml,Key(u"factory"));
                    const auto mathChildren=state->context.array(1);
                    state->context.set(Value(mathChildren),Key(u"0"),tipNode);
                    const auto mathProperties=state->context.object();
                    const std::array<Value,3> createArgs={Value(u"math"),Value(mathProperties),Value(mathChildren)};
                    const auto math=state->call(mmlFactory,u"create",createArgs);
                    const auto tipElement=state->call(receiver,u"html",std::array<Value,1>{Value(u"mjx-tip")});
                    const auto toolChildren=state->context.array(1);
                    state->context.set(Value(toolChildren),Key(u"0"),tipElement);
                    const auto toolProperties=state->context.object();
                    const std::array<Value,3> toolArgs={Value(u"mjx-tool"),Value(toolProperties),Value(toolChildren)};
                    const auto tool=state->call(receiver,u"html",toolArgs);
                    const auto hiddenStyle=state->context.object();
                    state->context.set(Value(hiddenStyle),Key(u"display"),Value(u"none"));
                    const auto hiddenProperties=state->context.object();
                    state->context.set(Value(hiddenProperties),Key(u"style"),Value(hiddenStyle));
                    const auto hiddenChildren=state->context.array(1);
                    state->context.set(Value(hiddenChildren),Key(u"0"),tool);
                    const std::array<Value,3> hiddenArgs={Value(u"foreignObject"),Value(hiddenProperties),Value(hiddenChildren)};
                    const auto foreignObject=state->call(receiver,u"svg",hiddenArgs);
                    const std::array<Value,2> appendArgs={rect,foreignObject};
                    const auto hidden=state->call(adaptor,u"append",appendArgs);
                    const auto factory=state->context.get(jax,Key(u"factory"));
                    const std::array<Value,1> wrapArgs={math};
                    const auto wrapper=state->call(factory,u"wrap",wrapArgs);
                    const auto toolFirst=state->call(adaptor,u"firstChild",std::array<Value,1>{tool});
                    const std::array<Value,2> processArgs={wrapper,toolFirst};
                    (void)state->call(jax,u"processMath",processArgs);
                    state->context.set(tipNode,Key(u"parent"),sourceMml);

                    const auto data=actionData;
                    const auto over=state->context.function(u"",
                        [weak,receiver,data,domNode,tool,hidden,jax,adaptor,container]
                        (Context&,Value,std::span<const Value> eventArgs)->Value{
                            auto current=weak.lock();
                            if(!current)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                            if(eventArgs.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'stopPropagation')");
                            const std::array<Value,2> stopArgs={domNode,data};
                            (void)current->call(data,u"stopTimers",stopArgs);
                            const auto map=current->context.get(data,Key(u"hoverTimer"));
                            const auto set=current->context.get(map,Key(u"set"));
                            const auto* setFunction=std::get_if<ObjectRef>(&set.data);
                            if(!setFunction||current->context.kind(*setFunction)!=ObjectKind::function)
                                throw Exception("TypeError",u"data.hoverTimer.set is not a function");
                            const auto callback=current->context.function(u"",
                                [weak,receiver,domNode,tool,jax,adaptor,container]
                                (Context&,Value,std::span<const Value>)->Value{
                                    auto state=weak.lock();
                                    if(!state)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                                    (void)state->call(adaptor,u"setStyle",std::array<Value,3>{tool,Value(u"left"),Value(u"0")});
                                    (void)state->call(adaptor,u"setStyle",std::array<Value,3>{tool,Value(u"top"),Value(u"0")});
                                    (void)state->call(adaptor,u"append",std::array<Value,2>{container,tool});
                                    const auto tooltipBox=state->call(adaptor,u"nodeBBox",std::array<Value,1>{tool});
                                    const auto nodeBox=state->call(adaptor,u"nodeBBox",std::array<Value,1>{domNode});
                                    const auto metrics=state->context.get(receiver,Key(u"metrics"));
                                    const auto em=state->context.toNumber(state->context.get(metrics,Key(u"em")));
                                    const auto dx=(state->context.toNumber(state->context.get(nodeBox,Key(u"right")))-
                                        state->context.toNumber(state->context.get(tooltipBox,Key(u"left"))))/em+
                                        state->context.toNumber(state->context.get(receiver,Key(u"tipDx")));
                                    const auto dy=(state->context.toNumber(state->context.get(nodeBox,Key(u"bottom")))-
                                        state->context.toNumber(state->context.get(tooltipBox,Key(u"bottom"))))/em+
                                        state->context.toNumber(state->context.get(receiver,Key(u"tipDy")));
                                    const auto pxX=state->call(receiver,u"Px",std::array<Value,1>{Value(dx)});
                                    const auto pxY=state->call(receiver,u"Px",std::array<Value,1>{Value(dy)});
                                    (void)state->call(adaptor,u"setStyle",std::array<Value,3>{tool,Value(u"left"),pxX});
                                    (void)state->call(adaptor,u"setStyle",std::array<Value,3>{tool,Value(u"top"),pxY});
                                    return Undefined{};
                                });
                            const auto delay=current->context.get(data,Key(u"postDelay"));
                            const auto timer=current->schedule(Value(callback),delay);
                            (void)current->context.call(*setFunction,map,std::array<Value,2>{domNode,timer});
                            (void)current->call(eventArgs.front(),u"stopPropagation");
                            return Undefined{};
                        });
                    state->setFunctionLength(over,1);
                    const auto out=state->context.function(u"",
                        [weak,domNode,data,hidden,tool,adaptor]
                        (Context&,Value,std::span<const Value> eventArgs)->Value{
                            auto current=weak.lock();
                            if(!current)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                            if(eventArgs.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'stopPropagation')");
                            (void)current->call(data,u"stopTimers",std::array<Value,2>{domNode,data});
                            const auto map=current->context.get(data,Key(u"clearTimer"));
                            const auto set=current->context.get(map,Key(u"set"));
                            const auto* setFunction=std::get_if<ObjectRef>(&set.data);
                            if(!setFunction||current->context.kind(*setFunction)!=ObjectKind::function)
                                throw Exception("TypeError",u"data.clearTimer.set is not a function");
                            const auto callback=current->context.function(u"",
                                [weak,hidden,tool,adaptor](Context&,Value,std::span<const Value>)->Value{
                                    auto state=weak.lock();
                                    if(!state)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                                    (void)state->call(adaptor,u"append",std::array<Value,2>{hidden,tool});
                                    return Undefined{};
                                });
                            const auto delay=current->context.get(data,Key(u"clearDelay"));
                            const auto timer=current->schedule(Value(callback),delay);
                            (void)current->context.call(*setFunction,map,std::array<Value,2>{domNode,timer});
                            (void)current->call(eventArgs.front(),u"stopPropagation");
                            return Undefined{};
                        });
                    state->setFunctionLength(out,1);
                    const std::array<Value,3> overArgs={Value(u"mouseover"),Value(over),domNode};
                    (void)state->call(receiver,u"setEventHandler",overArgs);
                    const std::array<Value,3> outArgs={Value(u"mouseout"),Value(out),domNode};
                    (void)state->call(receiver,u"setEventHandler",outArgs);
                    return Undefined{};
                });
            self.context.forEachIterable(dom,[&](Value domNode){
                (void)self.context.call(callback,Value(Undefined{}),std::array<Value,1>{domNode});
            });
            return Undefined{};
        });
    }
    ObjectRef statusline(){
        return installHandler(u"",[](State& self,std::span<const Value> args)->Value{
            if(args.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'childNodes')");
            const auto receiver=args[0];
            const auto children=self.context.get(receiver,Key(u"childNodes"));
            const auto tip=self.context.get(children,Key(u"1"));
            if(!options::truthy(tip))return Undefined{};
            const auto tipNode=self.context.get(tip,Key(u"node"));
            const std::array<Value,1> kindArgs={Value(u"mtext")};
            const auto isKind=self.context.get(tipNode,Key(u"isKind"));
            const auto* isKindFunction=std::get_if<ObjectRef>(&isKind.data);
            if(!isKindFunction)throw Exception("TypeError",u"tip.node.isKind is not a function");
            if(!options::truthy(self.context.call(*isKindFunction,tipNode,kindArgs)))return Undefined{};
            const auto adaptor=self.context.get(receiver,Key(u"adaptor"));
            const auto textMethod=self.context.get(tipNode,Key(u"getText"));
            const auto* getText=std::get_if<ObjectRef>(&textMethod.data);
            if(!getText)throw Exception("TypeError",u"TextNode.getText is not a function");
            const auto text=self.context.call(*getText,tipNode);
            const auto data=args.size()>1?args[1]:Value(Undefined{});
            const auto dom=self.context.get(receiver,Key(u"dom"));
            const auto forEach=self.context.get(dom,Key(u"forEach"));
            const auto* forEachFunction=std::get_if<ObjectRef>(&forEach.data);
            if(!forEachFunction)throw Exception("TypeError",u"this.dom.forEach is not a function");
            const auto setAttribute=self.context.function(u"",
                [weak=std::weak_ptr<State>(self.shared_from_this()),adaptor,text]
                (Context&,Value,std::span<const Value> itemArgs)->Value{
                    auto state=weak.lock();
                    if(!state)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                    const auto set=state->context.get(adaptor,Key(u"setAttribute"));
                    const auto* fn=std::get_if<ObjectRef>(&set.data);
                    if(!fn)throw Exception("TypeError",u"this.adaptor.setAttribute is not a function");
                    const std::array<Value,3> callArgs={itemArgs.front(),Value(u"data-statusline"),text};
                    return state->context.call(*fn,adaptor,callArgs);
                });
            self.setFunctionLength(setAttribute,1);
            const std::array<Value,1> forEachArgs={Value(setAttribute)};
            self.context.call(*forEachFunction,dom,forEachArgs);
            const auto over=self.context.function(u"",
                [weak=std::weak_ptr<State>(self.shared_from_this()),receiver,data,text]
                (Context&,Value,std::span<const Value> eventArgs)->Value{
                    auto state=weak.lock();
                    if(!state)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                    if(eventArgs.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'stopPropagation')");
                    const auto status=state->context.get(data,Key(u"status"));
                    if(std::holds_alternative<Null>(status.data)){
                        const auto wrapperAdaptor=state->context.get(receiver,Key(u"adaptor"));
                        const auto bodyMethod=state->context.get(wrapperAdaptor,Key(u"body"));
                        const auto* bodyFunction=std::get_if<ObjectRef>(&bodyMethod.data);
                        if(!bodyFunction||state->context.kind(*bodyFunction)!=ObjectKind::function)
                            throw Exception("TypeError",u"this.adaptor.body is not a function");
                        const auto document=state->context.get(wrapperAdaptor,Key(u"document"));
                        const std::array<Value,1> bodyArgs={document};
                        const auto body=state->context.call(*bodyFunction,wrapperAdaptor,bodyArgs);
                        const auto appendMethod=state->context.get(wrapperAdaptor,Key(u"append"));
                        const auto* appendFunction=std::get_if<ObjectRef>(&appendMethod.data);
                        if(!appendFunction||state->context.kind(*appendFunction)!=ObjectKind::function)
                            throw Exception("TypeError",u"this.adaptor.append is not a function");
                        const auto htmlMethod=state->context.get(receiver,Key(u"html"));
                        const auto* htmlFunction=std::get_if<ObjectRef>(&htmlMethod.data);
                        if(!htmlFunction||state->context.kind(*htmlFunction)!=ObjectKind::function)
                            throw Exception("TypeError",u"this.html is not a function");
                        const std::array<Value,1> textArgs={text};
                        const auto textNode=state->call(receiver,u"text",textArgs);
                        const auto content=state->context.array(1);
                        state->context.set(Value(content),Key(u"0"),textNode);
                        const auto definition=state->context.object();
                        const std::array<Value,3> htmlArgs={Value(u"mjx-status"),Value(definition),Value(content)};
                        const auto statusNode=state->context.call(*htmlFunction,receiver,htmlArgs);
                        const std::array<Value,2> appendArgs={body,statusNode};
                        const auto inserted=state->context.call(*appendFunction,wrapperAdaptor,appendArgs);
                        state->context.set(data,Key(u"status"),inserted);
                    }
                    (void)state->call(eventArgs.front(),u"stopPropagation");
                    return Undefined{};
                });
            self.setFunctionLength(over,1);
            const auto out=self.context.function(u"",
                [weak=std::weak_ptr<State>(self.shared_from_this()),receiver,data]
                (Context&,Value,std::span<const Value> eventArgs)->Value{
                    auto state=weak.lock();
                    if(!state)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                    if(eventArgs.empty())throw Exception("TypeError",u"Cannot read properties of undefined (reading 'stopPropagation')");
                    const auto status=state->context.get(data,Key(u"status"));
                    if(options::truthy(status)){
                        const auto adaptor=state->context.get(receiver,Key(u"adaptor"));
                        const auto remove=state->context.get(adaptor,Key(u"remove"));
                        const auto* removeFunction=std::get_if<ObjectRef>(&remove.data);
                        if(!removeFunction||state->context.kind(*removeFunction)!=ObjectKind::function)
                            throw Exception("TypeError",u"this.adaptor.remove is not a function");
                        const std::array<Value,1> removeArgs={state->context.get(data,Key(u"status"))};
                        (void)state->context.call(*removeFunction,adaptor,removeArgs);
                        state->context.set(data,Key(u"status"),Value(Null{}));
                    }
                    (void)state->call(eventArgs.front(),u"stopPropagation");
                    return Undefined{};
                });
            self.setFunctionLength(out,1);
            const std::array<Value,2> overArgs={Value(u"mouseover"),Value(over)};
            (void)self.call(receiver,u"setEventHandler",overArgs);
            const std::array<Value,2> outArgs={Value(u"mouseout"),Value(out)};
            (void)self.call(receiver,u"setEventHandler",outArgs);
            return Undefined{};
        });
    }
    ObjectRef eventMethod(){
        const auto weak=std::weak_ptr<State>(shared_from_this());
        const auto graph=graphLease();
        auto function=context.function(u"setEventHandler",[weak,graph](Context&,Value receiver,
            std::span<const Value> args)->Value{
            auto self=weak.lock();
            if(!self)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
            const auto eventType=args.empty()?Value(Undefined{}):args[0];
            const auto handler=args.size()<2?Value(Undefined{}):args[1];
            Value nodes=args.size()>2&&options::truthy(args[2])?Value(self->context.array(1)):
                self->context.get(receiver,Key(u"dom"));
            if(args.size()>2&&options::truthy(args[2]))
                self->context.set(nodes,Key(u"0"),args[2]);
            const auto iterate=self->context.get(nodes,Key(u"forEach"));
            const auto* forEach=std::get_if<ObjectRef>(&iterate.data);
            if(!forEach)throw Exception("TypeError",u"this.dom.forEach is not a function");
            const auto adding=self->context.function(u"",
                [weak,eventType,handler](Context& cx,Value,std::span<const Value> values)->Value{
                    auto state=weak.lock();
                    if(!state)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
                    if(values.empty())throw Exception("TypeError",u"DOM iteration supplied no node");
                    const auto node=values[0];
                    const auto method=cx.get(node,Key(u"addEventListener"));
                    const auto* callable=std::get_if<ObjectRef>(&method.data);
                    if(!callable)throw Exception("TypeError",u"node.addEventListener is not a function");
                    const std::array<Value,2> callArgs={eventType,handler};
                    return cx.call(*callable,node,callArgs);
                });
            self->setFunctionLength(adding,1);
            const std::array<Value,1> iterationArgs={Value(adding)};
            self->context.call(*forEach,nodes,iterationArgs);
            return Undefined{};
        });
        context.define(function,Key(u"length"),Descriptor::data(Value(3),false,false,true));
        return function;
    }
    ObjectRef pxMethod(){
        const auto weak=std::weak_ptr<State>(shared_from_this());
        const auto graph=graphLease();
        auto function=context.function(u"Px",[weak,graph](Context&,Value receiver,
            std::span<const Value> args)->Value{
            auto self=weak.lock();
            if(!self)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
            const auto px=self->context.get(receiver,Key(u"px"));
            const auto* function=std::get_if<ObjectRef>(&px.data);
            if(!function||self->context.kind(*function)!=ObjectKind::function)
                throw Exception("TypeError",u"this.px is not a function");
            const auto value=args.empty()?Value(Undefined{}):args.front();
            const std::array<Value,1> forwarded={value};
            return self->context.call(*function,receiver,forwarded);
        });
        context.define(function,Key(u"length"),Descriptor::data(Value(1),false,false,true));
        return function;
    }
    ObjectRef getParametersMethod(){
        const auto weak=std::weak_ptr<State>(shared_from_this());
        const auto graph=graphLease();
        auto function=context.function(u"getParameters",[weak,graph](Context&,Value receiver,
            std::span<const Value>)->Value{
            auto self=weak.lock();
            if(!self)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
            const auto node=self->context.get(receiver,Key(u"node"));
            const auto attributes=self->context.get(node,Key(u"attributes"));
            const auto get=self->context.get(attributes,Key(u"get"));
            const auto* getter=std::get_if<ObjectRef>(&get.data);
            if(!getter||self->context.kind(*getter)!=ObjectKind::function)
                throw Exception("TypeError",u"this.node.attributes.get is not a function");
            const std::array<Value,1> getArgs={Value(u"data-offsets")};
            auto offsets=self->context.call(*getter,attributes,getArgs);
            if(!options::truthy(offsets))offsets=Value(u"");
            const auto trim=self->context.get(offsets,Key(u"trim"));
            const auto* trimFunction=std::get_if<ObjectRef>(&trim.data);
            if(!trimFunction||self->context.kind(*trimFunction)!=ObjectKind::function)
                throw Exception("TypeError",u"offsets.trim is not a function");
            const auto trimmed=self->context.call(*trimFunction,offsets);
            const auto* text=std::get_if<std::u16string>(&trimmed.data);
            if(!text)throw Exception("TypeError",u"offsets.trim(...).split is not a function");
            std::vector<std::u16string> parts;
            for(const auto& part:regexp_host::split(*text,u"\\s+",u""))
                if(part)parts.push_back(*part);
            auto length2em=[&](Value value){
                const auto method=self->context.get(receiver,Key(u"length2em"));
                const auto* callable=std::get_if<ObjectRef>(&method.data);
                if(!callable||self->context.kind(*callable)!=ObjectKind::function)
                    throw Exception("TypeError",u"this.length2em is not a function");
                const std::array<Value,1> methodArgs={std::move(value)};
                const auto result=self->context.call(*callable,receiver,methodArgs);
                return result;
            };
            const auto dx=parts.empty()||parts[0].empty()
                ?self->context.get(Value(self->tooltipData),Key(u"dx"))
                :Value(parts[0]);
            const auto dy=parts.size()<2||parts[1].empty()
                ?self->context.get(Value(self->tooltipData),Key(u"dy"))
                :Value(parts[1]);
            const auto convertedDx=length2em(dx);
            const auto convertedDy=length2em(dy);
            self->context.set(receiver,Key(u"tipDx"),convertedDx);
            self->context.set(receiver,Key(u"tipDy"),convertedDy);
            return Undefined{};
        });
        context.define(function,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        return function;
    }
};

SvgMactionSourceSurface::SvgMactionSourceSurface(Context& context,MapContextBindings& maps,
    std::shared_ptr<common::CommonFactoryContextBindings> factories,std::shared_ptr<TimerHost> timers,
    std::shared_ptr<SceneContextNodes> nodes,std::shared_ptr<void> graph)
    :state_(std::make_shared<State>(context,maps,std::move(factories),std::move(timers),
                                   std::move(nodes),std::move(graph))){}
SvgMactionSourceSurface::~SvgMactionSourceSurface()=default;

ObjectRef SvgMactionSourceSurface::install(ObjectRef constructor,ObjectRef commonMactionConstructor,
                                           ObjectRef sourceMathItemState){
    auto& state=*state_;
    if(state.context.kind(sourceMathItemState)!=ObjectKind::object)
        throw Exception("TypeError",u"MathItem STATE must be a canonical object in this Context");
    state.sourceMathItemState=sourceMathItemState;
    const auto storage=state.maps.createStorage();
    const auto map=state.maps.bind(storage);
    const auto toggle=state.toggle();
    state.maps.set(map,Value(u"toggle"),Value(state.pair(toggle,state.context.object())));
    const auto tooltip=state.tooltip();
    auto tooltipData=state.context.object();
    state.tooltipData=tooltipData;
    state.context.set(Value(tooltipData),Key(u"dx"),Value(u".2em"));
    state.context.set(Value(tooltipData),Key(u"dy"),Value(u".1em"));
    state.context.set(Value(tooltipData),Key(u"postDelay"),Value(600));
    state.context.set(Value(tooltipData),Key(u"clearDelay"),Value(100));
    state.context.set(Value(tooltipData),Key(u"hoverTimer"),Value(state.maps.create()));
    state.context.set(Value(tooltipData),Key(u"clearTimer"),Value(state.maps.create()));
    const auto stopTimers=state.context.function(u"stopTimers",
        [weak=std::weak_ptr<State>(state.shared_from_this())](Context&,Value,
        std::span<const Value> args)->Value{
            auto current=weak.lock();
            if(!current)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
            const auto node=args.empty()?Value(Undefined{}):args[0];
            const auto data=args.size()<2?Value(Undefined{}):args[1];
            for(const auto key:{std::u16string_view(u"clearTimer"),std::u16string_view(u"hoverTimer")}){
                const auto map=current->context.get(data,Key(std::u16string(key)));
                const auto has=current->context.get(map,Key(u"has"));
                const auto* hasFunction=std::get_if<ObjectRef>(&has.data);
                if(!hasFunction||current->context.kind(*hasFunction)!=ObjectKind::function)
                    throw Exception("TypeError",u"timer Map.has is not a function");
                const std::array<Value,1> mapArgs={node};
                if(!options::truthy(current->context.call(*hasFunction,map,mapArgs)))continue;
                const auto get=current->context.get(map,Key(u"get"));
                const auto* getFunction=std::get_if<ObjectRef>(&get.data);
                if(!getFunction||current->context.kind(*getFunction)!=ObjectKind::function)
                    throw Exception("TypeError",u"timer Map.get is not a function");
                const auto timer=current->context.call(*getFunction,map,mapArgs);
                current->cancel(timer);
                const auto erase=current->context.get(map,Key(u"delete"));
                const auto* deleteFunction=std::get_if<ObjectRef>(&erase.data);
                if(!deleteFunction||current->context.kind(*deleteFunction)!=ObjectKind::function)
                    throw Exception("TypeError",u"timer Map.delete is not a function");
                (void)current->context.call(*deleteFunction,map,mapArgs);
            }
            return Undefined{};
        });
    state.setFunctionLength(stopTimers,2);
    state.context.set(Value(tooltipData),Key(u"stopTimers"),Value(stopTimers));
    state.maps.set(map,Value(u"tooltip"),Value(state.pair(tooltip,tooltipData)));
    const auto status=state.statusline();
    auto statusData=state.context.object();
    state.context.set(Value(statusData),Key(u"status"),Value(Null{}));
    state.maps.set(map,Value(u"statusline"),Value(state.pair(status,statusData)));
    state.actionMap=map;
    state.context.set(Value(constructor),Key(u"styles"),Value(state.styles()));
    state.context.set(Value(constructor),Key(u"actions"),Value(map));
    const auto prototypeValue=state.context.get(Value(constructor),Key(u"prototype"));
    const auto* prototype=std::get_if<ObjectRef>(&prototypeValue.data);
    if(!prototype)throw Exception("TypeError",u"SvgMaction constructor has no prototype");
    ObjectRef svgWrapperConstructor=constructor;
    while(svgWrapperConstructor!=ObjectRef{}){
        const auto name=state.context.get(Value(svgWrapperConstructor),Key(u"name"));
        if(const auto* text=std::get_if<std::u16string>(&name.data);
           text&&*text==u"SvgWrapper")break;
        const auto parent=state.context.prototype(svgWrapperConstructor);
        if(!parent)throw Exception("TypeError",u"SvgMaction has no SvgWrapper base constructor");
        svgWrapperConstructor=*parent;
    }
    const auto svgWrapperPrototypeValue=state.context.get(
        Value(svgWrapperConstructor),Key(u"prototype"));
    const auto* svgWrapperPrototype=std::get_if<ObjectRef>(&svgWrapperPrototypeValue.data);
    if(!svgWrapperPrototype)throw Exception("TypeError",u"SvgWrapper constructor has no prototype");
    const auto firstChild=state.context.function(u"firstChild",
        [weak=std::weak_ptr<State>(state.shared_from_this())]
        (Context&,Value receiver,std::span<const Value> args)->Value{
            auto current=weak.lock();
            if(!current)throw Exception("Unimplemented",u"SvgMaction source methods have been released");
            Value dom;
            if(args.empty()||std::holds_alternative<Undefined>(args[0].data)){
                const auto nodes=current->context.get(receiver,Key(u"dom"));
                dom=current->context.get(nodes,Key(u"0"));
            }else dom=args[0];
            const auto adaptor=current->context.get(receiver,Key(u"adaptor"));
            Value child=current->call(adaptor,u"firstChild",std::array<Value,1>{dom});
            if(options::truthy(child)&&
               current->call(adaptor,u"kind",std::array<Value,1>{child})==Value(u"text")&&
               options::truthy(current->call(adaptor,u"getAttribute",
                   std::array<Value,2>{child,Value(u"data-id-align")}))){
                const auto next=current->call(adaptor,u"next",std::array<Value,1>{child});
                child=current->call(adaptor,u"firstChild",std::array<Value,1>{next});
            }
            if(options::truthy(child)&&
               current->call(adaptor,u"kind",std::array<Value,1>{child})==Value(u"rect")&&
               options::truthy(current->call(adaptor,u"getAttribute",
                   std::array<Value,2>{child,Value(u"data-hitbox")})))
                child=current->call(adaptor,u"next",std::array<Value,1>{child});
            return child;
        });
    state.setFunctionLength(firstChild,1);
    state.context.define(*svgWrapperPrototype,Key(u"firstChild"),
        Descriptor::data(Value(firstChild),true,false,true));
    state.context.define(*prototype,Key(u"setEventHandler"),
        Descriptor::data(Value(state.eventMethod()),true,false,true));
    state.context.define(*prototype,Key(u"Px"),
        Descriptor::data(Value(state.pxMethod()),true,false,true));
    const auto commonPrototypeValue=state.context.get(Value(commonMactionConstructor),Key(u"prototype"));
    const auto* commonPrototype=std::get_if<ObjectRef>(&commonPrototypeValue.data);
    if(!commonPrototype)throw Exception("TypeError",u"CommonMaction constructor has no prototype");
    state.context.define(*commonPrototype,Key(u"getParameters"),
        Descriptor::data(Value(state.getParametersMethod()),true,false,true));
    return map;
}

void SvgMactionSourceSurface::bindLiteModule(adaptors::lite::LiteModule& lite){
    if(&lite.context()!=&state_->context)
        throw Exception("TypeError",u"Maction LiteModule belongs to a different Context");
    if(state_->lite&&state_->lite!=&lite)
        throw Exception("TypeError",u"Maction LiteModule is already bound");
    state_->lite=&lite;
}
void SvgMactionSourceSurface::bindPropertyBindings(ContextPropertyBindings& properties){
    if(&properties.context()!=&state_->context)
        throw Exception("TypeError",u"Maction property bindings belong to a different Context");
    if(state_->properties&&state_->properties!=&properties)
        throw Exception("TypeError",u"Maction property bindings are already bound");
    state_->properties=&properties;
}

Value SvgMactionSourceSurface::runAction(common::CommonMaction& wrapper,scene::Builder& builder,
                                         std::span<const scene::NodeRef> roots){
    auto& state=*state_;
    if(!state.lite)throw Exception("Unimplemented",u"Maction Lite append target is not bound");
    if(!state.properties)throw Exception("Unimplemented",u"Maction property projection is not bound");
    const auto identity=wrapper.factory().objectFor(wrapper);
    const auto dom=state.context.array(static_cast<std::uint32_t>(roots.size()));
    for(std::size_t index=0;index<roots.size();++index){
        if(!roots[index])throw Exception("TypeError",u"Maction DOM root is null");
        const auto handle=state.sceneNodes->handle(roots[index]);
        state.context.set(Value(dom),Key(property_host::text(std::to_string(index))),Value(handle));
    }
    // SvgWrapper.standardSvgNodes leaves this actual Source field on the
    // wrapper. Its entries are handles for the same Builder nodes, not a
    // second DOM or Scene tree.
    state.context.set(Value(identity),Key(u"dom"),Value(dom));

    auto previousSvgTarget=dynamic_cast<SvgOutputJax&>(wrapper.jax()).exchangeSceneSvgTarget(
        [&state,&builder](std::span<const Value> args)->std::optional<Value>{
            if(args.empty())return {};
            const auto kindValue=state.context.toPrimitive(args[0],PrimitiveHint::stringHint);
            const auto* kind=std::get_if<std::u16string>(&kindValue.data);
            if(!kind)throw Exception("TypeError",u"SvgJax.svg node type is not a string");
            const auto definition=args.size()>1?args[1]:Value(Undefined{});
            const auto content=args.size()>2?args[2]:Value(Undefined{});
            std::size_t childCount=0;
            if(const auto* array=std::get_if<ObjectRef>(&content.data)){
                if(state.context.kind(*array)==ObjectKind::array){
                    const auto length=state.context.get(content,Key(u"length"));
                    const double raw=state.context.toNumber(length);
                    if(std::isfinite(raw)&&raw>0)childCount=static_cast<std::size_t>(raw);
                }
            }
            auto attributes=state.sourceAttributes(definition);
            auto node=builder.node(*kind,attributes,{},childCount);
            for(std::size_t index=0;index<childCount;++index){
                const auto child=state.context.get(content,Key(property_host::text(std::to_string(index))));
                state.appendSourceChild(builder,node,child);
            }
            const auto handle=state.sceneNodes->handle(node);
            if(*kind==u"foreignObject"){
                // Let the public handle normalization import explicit helper
                // attributes into typed geometry before selecting Source's
                // current-attribute serialization policy.
                auto* geometry=std::get_if<scene::ForeignObjectGeometry>(&node->geometry);
                if(!geometry)throw Exception("TypeError",u"foreignObject handle has no typed geometry");
                geometry->synthesizeMissingAttributes=false;
            }
            return Value(handle);
        });
    struct SvgRestore{
        SvgOutputJax& jax;
        SvgOutputJax::SceneSvgTarget previous;
        ~SvgRestore(){(void)jax.exchangeSceneSvgTarget(std::move(previous));}
    } svgRestore{dynamic_cast<SvgOutputJax&>(wrapper.jax()),std::move(previousSvgTarget)};

    auto targets=state.sceneNodes->installTargets(*state.lite,builder);

    return common::invokeCommonMactionAction(wrapper);
}
Context& SvgMactionSourceSurface::context()const{return state_->context;}
}
