#include "support/folia_test.hpp"
import folia.math.output.svg.native_document;
import folia.math.output.svg.serializer;
import folia.math.output.svg.scene_context_nodes;
import folia.math.output.scene;
import folia.math.core.math_item;
import folia.math.core.mml_tree.xml_dom_adaptor;
import folia.math.core.mml_tree.mml_nodes.html_node;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts=folia::math::options;
using namespace opts;
namespace core=folia::math::core;
namespace mml=folia::math::mml;
namespace svg=folia::math::output::svg;
namespace scene=folia::math::output::scene;
namespace common=folia::math::output::common;
namespace lite=folia::math::adaptors::lite;

common::FontLoadHost fontHost(){
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&){};
    return host;
}

Value objectValue(const Value& value){
    if(const auto* object=std::get_if<ObjectRef>(&value.data))return value;
    throw Exception("TypeError",u"expected Context object");
}
std::shared_ptr<core::MathItem> mathItem(Context& context,
    const std::shared_ptr<handlers::html::HTMLMathDocument>& document,mml::MmlNode* root){
    const Value documentIdentity(document->sourceIdentity());
    const auto options=context.get(documentIdentity,Key(u"options"));
    const auto constructor=context.get(options,Key(u"MathItem"));
    const auto inputs=context.get(documentIdentity,Key(u"inputJax"));
    const auto input=context.get(inputs,Key(u"0"));
    const std::array<Value,3> arguments{Value(u"<native XML scene fixture>"),input,Value(true)};
    const auto identity=context.construct(std::get<ObjectRef>(objectValue(constructor).data),arguments);
    const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(identity));
    auto item=payload?std::dynamic_pointer_cast<core::MathItem>(*payload):nullptr;
    if(!item)throw Exception("TypeError",u"MathItem constructor returned no native item");
    // MathDocument.convert() sets start.node to adaptor.body(document) before
    // typesetting. This fixture constructs the MathItem directly, so establish
    // that same live DOM location before the output wrapper resolves the
    // surrounding font family. The end location remains the constructor's
    // default, matching Source convert(), which only sets start.node here.
    const auto startValue=document->domAdaptor().body(document->documentNode());
    const auto* startNode=std::get_if<DOMNodeRef>(&startValue);
    if(!startNode)throw Exception("TypeError",u"MathItem fixture has no document body");
    item->start=Property(PropertyList{{"node",nodeProperty(*startNode)},
                                      {"n",0},{"delim",""}});
    item->root=mml::NodeValue(root);
    item->setMetrics(Property(16),Property(8),Property(640),Property(2));
    return item;
}
}

suite scene_attribute_parity=[] {
    "scene_child_callback_holds_provider_lease_until_iteration_finishes"_test=[] {
        Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<folia::math::regexp_host::RegExpBindings>(context);
        lite::LiteModule module(context,functions,properties,regexp);
        svg::SceneContextNodes nodes(context,u"LeaseScopeSceneNode");
        nodes.bindPropertyBindings(properties);
        nodes.bindLiteModule(module);
        auto provider=std::make_shared<int>(1);
        std::weak_ptr<void> providerLifetime=provider;
        nodes.bindServiceOwner(provider);

        auto node=std::make_shared<scene::Node>();
        node->kind=u"g";
        node->semantic=scene::SourceDomPrimitive{};
        const auto handle=nodes.handle(node);
        const auto iterable=context.object();
        const auto iterator=context.object();
        bool providerAliveInsideCallback=false;
        const auto iteratorMethod=context.function(u"[Symbol.iterator]",
            [iterator,&provider,&providerLifetime,&providerAliveInsideCallback]
            (Context&,Value,std::span<const Value>)->Value{
                provider.reset();
                providerAliveInsideCallback=!providerLifetime.expired();
                return Value(iterator);
            });
        context.define(iterable,Key(context.intrinsics().symbolIterator),
            Descriptor::data(Value(iteratorMethod)));
        const auto next=context.function(u"next",
            [](Context& cx,Value,std::span<const Value>)->Value{
                const auto step=cx.object();
                cx.define(step,Key(u"done"),Descriptor::data(Value(true)));
                return Value(step);
            });
        context.define(iterator,Key(u"next"),Descriptor::data(Value(next)));
        context.set(Value(handle),Key(u"children"),Value(iterable));

        scene::Builder builder;
        builder.addRoot(node);
        const auto completed=std::move(builder).finish();
        expect(providerAliveInsideCallback);
        expect(providerLifetime.expired());
        expect(completed.roots().size()==1_u);
        if(!completed.roots().empty())expect(completed.roots().front()->children.empty());
    };

    "external_scene_handle_fails_closed_after_service_owner_expires"_test=[] {
        Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto nodes=std::make_shared<svg::SceneContextNodes>(context,u"SceneLeaseTestNode");
        nodes->bindPropertyBindings(properties);
        auto lease=std::make_shared<int>(1);
        nodes->bindServiceOwner(lease);
        auto node=std::make_shared<scene::Node>();
        node->kind=u"g";
        const auto handle=nodes->handle(node);
        const auto readAttributes=context.get(Value(handle),Key(u"attributes"));
        const auto readChildren=context.get(Value(handle),Key(u"children"));
        lease.reset();
        const auto rejected=[&](auto&& operation){
            try{operation();return false;}
            catch(const Exception& error){return error.name=="TypeError";}
        };
        expect(rejected([&]{(void)context.get(Value(handle),Key(u"attributes"));}));
        expect(rejected([&]{(void)context.get(Value(handle),Key(u"children"));}));
        expect(rejected([&]{context.set(Value(handle),Key(u"children"),Value(Null{}));}));
        expect(rejected([&]{(void)context.get(Value(handle),Key(u"parent"));}));
        expect(rejected([&]{context.set(Value(handle),Key(u"parent"),Value(Null{}));}));
        expect(rejected([&]{(void)context.get(readAttributes,Key(u"data-unavailable"));}));
        // This is an escaped ordinary Context Array. Its own indexed data is
        // independent of the expired Scene service and remains readable.
        expect(context.get(readChildren,Key(u"0"))==Value(Undefined{}));
    };

    "native_session_context_keeps_actual_scene_providers_alive"_test=[] {
        auto session=std::make_shared<svg::NativeRenderSession>(fontHost());
        const auto contextOwner=session->contextOwner();
        auto& context=*contextOwner;
        auto node=std::make_shared<scene::Node>();
        node->kind=u"g";
        node->attributes.setOwn("data-owner",Property("service graph"));
        const auto handle=session->services().sceneContextNodes().handle(node);
        const auto attributes=context.get(Value(handle),Key(u"attributes"));
        expect(context.get(attributes,Key(u"data-owner"))==Value(u"service graph"));

        // Context-owned constructor leases retain the real graph and all
        // borrowed providers after the NativeRenderSession wrapper is gone.
        session.reset();
        expect(context.get(attributes,Key(u"data-owner"))==Value(u"service graph"));
    };

    "native_svg_inlinebreak_clone_does_not_clear_source_children"_test=[] {
        auto session=std::make_shared<svg::NativeRenderSession>(fontHost());
        auto& context=session->context();
        const auto outputOptions=context.object();
        context.set(outputOptions,Key(u"fontCache"),Value(u"none"));
        context.set(outputOptions,Key(u"exFactor"),Value(.5));
        context.set(outputOptions,Key(u"displayOverflow"),Value(u"linebreak"));
        const auto linebreaks=context.object();
        context.set(linebreaks,Key(u"inline"),Value(true));
        context.set(outputOptions,Key(u"linebreaks"),Value(linebreaks));
        (void)session->construct(Value(outputOptions));
        const auto inputOptions=context.object();
        auto document=svg::createNativeSvgDocument(*session,Undefined{},Value(inputOptions));
        const auto settings=context.object();
        context.set(settings,Key(u"display"),Value(false));
        context.set(settings,Key(u"em"),Value(16.0));
        context.set(settings,Key(u"ex"),Value(8.0));
        context.set(settings,Key(u"containerWidth"),Value(1280.0));
        const auto converted=document->convert(Value(u"x+y"),Value(settings));
        expect(!std::holds_alternative<Undefined>(converted.data));
        const auto completed=session->outputJax()->completedScene();
        expect(completed!=nullptr);
        if(completed){
            expect(!completed->roots().empty());
            if(!completed->roots().empty())expect(completed->roots().front()->kind==u"svg");
        }
    };

    "native_document_typeset_uses_actual_svg_helper_return_identity"_test=[] {
        auto session=std::make_shared<svg::NativeRenderSession>(fontHost());
        auto& context=session->context();
        const auto options=context.object();
        context.set(options,Key(u"fontCache"),Value(u"none"));
        context.set(options,Key(u"htmlHDW"),Value(u"use"));
        const auto jax=session->construct(Value(options));
        auto document=svg::createNativeSvgDocument(*session);
        auto adaptor=session->adaptor();
        expect(adaptor!=nullptr);
        if(!adaptor)return;

        auto factory=session->mathFactory();
        const std::span<const std::any> noArguments{};
        auto* math=factory->create("math",noArguments);
        auto* mtext=factory->create("mtext",noArguments);
        auto* htmlNode=dynamic_cast<mml::HtmlNode*>(factory->create("html",noArguments));
        expect(math!=nullptr);expect(mtext!=nullptr);expect(htmlNode!=nullptr);
        if(!math||!mtext||!htmlNode)return;
        const auto text=adaptor->text(u"original child");
        const auto comment=adaptor->comment(u"original comment");
        const auto bold=adaptor->node(u"b",PropertyList{{"class",Property("fixture")}},
            PropertyArray{Property(nodeProperty(text))});
        const auto html=adaptor->node(u"section",
            PropertyList{{"data-mjx-hdw",Property("1em .25em 2em")}},
            PropertyArray{Property(nodeProperty(bold)),Property(nodeProperty(comment))});
        auto xmlAdaptor=mml::XmlDomAdaptor::create(adaptor);
        htmlNode->setHTML(xmlAdaptor->fromDOM(html),xmlAdaptor.get());
        // TexHtmlConfiguration's pinned Source parser wraps HtmlNode in an
        // mtext token. Keep that legal token boundary so XML's read-only
        // prevLevel is never sent through AbstractMmlLayoutNode::updateTeXclass.
        (void)mtext->appendChild(mml::NodeValue(htmlNode));
        (void)math->appendChild(mml::NodeValue(mtext));

        auto sourceJax=jax->instanceIdentity();
        const auto originalValue=context.get(Value(sourceJax),Key(u"svg"));
        const auto* originalPtr=std::get_if<ObjectRef>(&originalValue.data);
        expect(originalPtr!=nullptr);
        if(!originalPtr)return;
        const auto original=*originalPtr;
        Value returnedByHelper=Undefined{};
        Value htmlPassedToHelper=Undefined{};
        scene::NodeRef geometryOwnerNode;
        Value geometryOwnerSource=Undefined{};
        Value geometryOwnerAttributes=Undefined{};
        std::size_t foreignObjectCalls=0;
        const auto helper=context.function(u"svg",
            [&,original](Context& cx,Value receiver,std::span<const Value> args)->Value{
                const Value first=cx.call(original,receiver,args);
                const auto* kind=args.empty()?nullptr:std::get_if<std::u16string>(&args[0].data);
                if(kind&&*kind==u"foreignObject"){
                    ++foreignObjectCalls;
                    const auto* array=args.size()<3?nullptr:std::get_if<ObjectRef>(&args[2].data);
                    expect(array!=nullptr);
                    if(array){
                        htmlPassedToHelper=cx.get(*array,Key(u"0"));
                        const auto attributes=cx.get(htmlPassedToHelper,Key(u"attributes"));
                        cx.set(attributes,Key(u"data-helper"),Value(u"source-helper"));
                    }
                    returnedByHelper=first;
                    // Create a later node through the same real Source helper,
                    // then return the earlier node exactly as this override does.
                    const auto decoyAttributes=cx.object();
                    const auto decoyChildren=cx.array();
                    const std::array<Value,3> decoyArgs{Value(u"g"),Value(decoyAttributes),Value(decoyChildren)};
                    (void)cx.call(original,receiver,decoyArgs);
                    const auto rectAttributes=cx.object();
                    cx.set(rectAttributes,Key(u"width"),Value(u"12"));
                    cx.set(rectAttributes,Key(u"height"),Value(u"4"));
                    cx.set(rectAttributes,Key(u"fill"),Value(u"blue"));
                    cx.set(rectAttributes,Key(u"stroke"),Value(u"black"));
                    const std::array<Value,3> rectArgs{Value(u"rect"),Value(rectAttributes),cx.array()};
                    const auto rectValue=cx.call(original,receiver,rectArgs);
                    geometryOwnerSource=rectValue;
                    geometryOwnerAttributes=rectAttributes;
                    // Keep the extra helper result inside the actual returned
                    // foreignObject. Only returned/imported descendants get a
                    // Scene identity during XML import.
                    const auto adaptorValue=cx.get(Value(sourceJax),Key(u"adaptor"));
                    const auto append=cx.get(adaptorValue,Key(u"append"));
                    const std::array<Value,2> appendArgs{first,rectValue};
                    (void)cx.callMember(append,adaptorValue,appendArgs,u"adaptor.append");
                }
                return first;
            });
        context.set(Value(sourceJax),Key(u"svg"),Value(helper));

        auto item=mathItem(context,document,math);
        item->typeset(core::MathDocumentReference{document.get()});
        expect(foreignObjectCalls==1_u);
        if(const auto mapped=session->services().sceneContextNodes().sourceNode(geometryOwnerSource)){
            geometryOwnerNode=*mapped;
            const auto* attributes=std::get_if<ObjectRef>(&geometryOwnerAttributes.data);
            expect(attributes!=nullptr);
            if(attributes)context.set(*attributes,Key(u"width"),Value(u"19"));
            const auto replacementAttributes=context.object();
            context.set(replacementAttributes,Key(u"width"),Value(u"19"));
            context.set(replacementAttributes,Key(u"height"),Value(u"4"));
            context.set(replacementAttributes,Key(u"fill"),Value(u"blue"));
            context.set(replacementAttributes,Key(u"stroke"),Value(u"black"));
            context.set(std::get<ObjectRef>(geometryOwnerSource.data),Key(u"attributes"),Value(replacementAttributes));
            context.set(replacementAttributes,Key(u"fill"),Value(u"red"));
            context.set(replacementAttributes,Key(u"stroke"),Value(u"green"));
        }
        expect(!std::holds_alternative<Undefined>(returnedByHelper.data));
        expect(!std::holds_alternative<Undefined>(htmlPassedToHelper.data));
        const auto sceneValue=jax->completedScene();
        expect(sceneValue!=nullptr);
        if(!sceneValue)return;
        expect(sceneValue->roots().size()==1_u);
        if(sceneValue->roots().empty())return;

        const auto& root=sceneValue->roots().front();
        std::function<scene::FrozenNodeRef(const scene::FrozenNodeRef&)> findForeign;
        findForeign=[&](const scene::FrozenNodeRef& node)->scene::FrozenNodeRef{
            if(node->kind==u"foreignObject")return node;
            for(const auto& child:node->children)if(auto found=findForeign(child))return found;
            return {};
        };
        const auto foreign=findForeign(root);
        expect(foreign!=nullptr);
        if(!foreign)return;
        expect(foreign->children.size()==2_u);
        if(foreign->children.empty())return;
        const auto imported=foreign->children.front();
        expect(imported->kind==u"mjx-html");
        const auto helperAttribute=std::find_if(imported->attributes.begin(),imported->attributes.end(),
            [](const auto& attribute){return attribute.first=="data-helper";});
        expect(helperAttribute!=imported->attributes.end());
        if(helperAttribute!=imported->attributes.end())expect(helperAttribute->second=="source-helper");
        expect(geometryOwnerNode!=nullptr);
        if(geometryOwnerNode){
            scene::Builder liveGeometryBuilder;
            liveGeometryBuilder.addRoot(geometryOwnerNode);
            const auto liveGeometryScene=std::move(liveGeometryBuilder).finish();
            expect(!liveGeometryScene.roots().empty());
            if(!liveGeometryScene.roots().empty()){
                const auto* rect=std::get_if<scene::RectGeometry>(&liveGeometryScene.roots().front()->geometry);
                expect(rect!=nullptr);
                if(rect)expect(rect->width==0.019);
                const auto& paint=liveGeometryScene.roots().front()->paint;
                expect(paint.has_value());
                if(paint){
                    expect(paint->fill==std::optional<std::string>("red"));
                    expect(paint->stroke==std::optional<std::string>("green"));
                }
            }
        }

        // The helper returned the first created DOM node, not the later decoy.
        const auto returnedObject=std::get<ObjectRef>(returnedByHelper.data);
        const auto returnedIdentity=context.get(Value(returnedObject),Key(u"kind"));
        expect(returnedIdentity==Value(u"foreignObject"));
        const auto mappedForeign=session->services().sceneContextNodes().sourceNode(Value(returnedObject));
        expect(mappedForeign.has_value());
        const auto freezeForeign=[&]()->scene::FrozenNodeRef{
            if(!mappedForeign)return {};
            scene::Builder builder;
            builder.addRoot(*mappedForeign);
            const auto snapshot=std::move(builder).finish();
            return snapshot.roots().empty()?scene::FrozenNodeRef{}:snapshot.roots().front();
        };
        const auto valueText=[](const Value& value){
            if(const auto* text=std::get_if<std::u16string>(&value.data))
                return property_host::debugText(*text);
            throw Exception("TypeError",u"Expected a Source string attribute");
        };
        const auto numberInAttribute=[](std::string value){
            const auto px=value.find("px");
            if(px!=std::string::npos)value.erase(px);
            return std::stod(value);
        };
        const auto applyPoint=[](const scene::Transform& transform,scene::Point point){
            for(auto it=transform.operations.rbegin();it!=transform.operations.rend();++it){
                const auto& operation=*it;
                switch(operation.kind){
                case scene::TransformKind::Scale:
                    point.x*=operation.x;point.y*=operation.y;break;
                case scene::TransformKind::Translate:
                    point.x+=operation.x;point.y+=operation.y;break;
                case scene::TransformKind::Matrix:{
                    const auto x=operation.a*point.x+operation.c*point.y+operation.e;
                    const auto y=operation.b*point.x+operation.d*point.y+operation.f;
                    point={x,y};break;
                }
                case scene::TransformKind::Rotate:{
                    const auto radians=operation.degrees*3.14159265358979323846/180.0;
                    const auto x=point.x-operation.centerX,y=point.y-operation.centerY;
                    point={operation.centerX+x*std::cos(radians)-y*std::sin(radians),
                        operation.centerY+x*std::sin(radians)+y*std::cos(radians)};break;
                }
                }
            }
            return point;
        };
        if(mappedForeign){
            const auto foreignAttributes=context.get(Value(returnedObject),Key(u"attributes"));
            const auto sourceTransform=valueText(context.get(foreignAttributes,Key(u"transform")));
            const auto scaleStart=sourceTransform.find("scale(");
            const auto scaleEnd=sourceTransform.find(')',scaleStart);
            expect(scaleStart!=std::string::npos&&scaleEnd!=std::string::npos);
            if(scaleStart!=std::string::npos&&scaleEnd!=std::string::npos){
                const auto svgScale=std::stod(sourceTransform.substr(scaleStart+6,scaleEnd-scaleStart-6));
                // The fixture sets metrics.em=16 and metrics.scale=2;
                // the XML wrapper's rscale is 1, so em=32 CSS px. SvgXmlNode
                // emits fixed(1/em), with SVG fixed scaling by 1000.
                expect(std::abs(svgScale/1000-1.0/32)<1e-9);
                const auto frozen=freezeForeign();
                const auto* geometry=frozen?std::get_if<scene::ForeignObjectGeometry>(&frozen->geometry):nullptr;
                expect(geometry!=nullptr);
                if(frozen)expect(!frozen->transform.has_value());
                if(geometry){
                    const auto sourceWidth=valueText(context.get(foreignAttributes,Key(u"width")));
                    const auto sourceHeight=valueText(context.get(foreignAttributes,Key(u"height")));
                    expect(std::abs(geometry->width-numberInAttribute(sourceWidth))<1e-9);
                    expect(std::abs(geometry->height-numberInAttribute(sourceHeight))<1e-9);
                    const auto point=applyPoint(geometry->transform,{4,8});
                    expect(std::abs(point.x-4*svgScale/1000)<1e-9);
                    expect(std::abs(point.y+8*svgScale/1000)<1e-9);
                }
            }
            // Replacing Source's live attributes must update the typed CSS-px
            // viewport and the complete transform used by a later freeze.
            const auto replacement=context.object();
            context.set(replacement,Key(u"data-mjx-xml"),Value(true));
            context.set(replacement,Key(u"y"),Value(u"-45px"));
            context.set(replacement,Key(u"width"),Value(u"321px"));
            context.set(replacement,Key(u"height"),Value(u"123px"));
            context.set(replacement,Key(u"transform"),
                Value(u"scale(125) rotate(180) matrix(1 0 0 -1 0 0) scale(1.5)"));
            context.set(Value(returnedObject),Key(u"attributes"),Value(replacement));
            auto frozen=freezeForeign();
            auto* geometry=frozen?std::get_if<scene::ForeignObjectGeometry>(&frozen->geometry):nullptr;
            expect(geometry!=nullptr);
            if(frozen)expect(!frozen->transform.has_value());
            if(geometry){
                expect(geometry->width==321.0);
                expect(geometry->height==123.0);
                const auto point=applyPoint(geometry->transform,{4,8});
                expect(std::abs(point.x+.75)<1e-9);
                expect(std::abs(point.y-1.5)<1e-9);
            }
            context.set(replacement,Key(u"transform"),Value(u"scale(250) matrix(1 0 0 -1 0 0)"));
            frozen=freezeForeign();
            geometry=frozen?std::get_if<scene::ForeignObjectGeometry>(&frozen->geometry):nullptr;
            expect(geometry!=nullptr);
            if(geometry){
                const auto point=applyPoint(geometry->transform,{4,8});
                expect(std::abs(point.x-1.0)<1e-9);
                expect(std::abs(point.y+2.0)<1e-9);
            }
            context.set(replacement,Key(u"transform"),Value(
                u"translate(100 200) scale(2 3) rotate(90) matrix(1 0 0 1 4 5)"));
            frozen=freezeForeign();
            geometry=frozen?std::get_if<scene::ForeignObjectGeometry>(&frozen->geometry):nullptr;
            expect(geometry!=nullptr);
            if(geometry){
                // SVG column-vector composition applies the list in reverse:
                // p=(4,8) -> matrix(1,1,4,5) -> rotate(90) -> scale(2,3)
                // -> translate(100,200) = (74,224) SVG units, then /1000 em.
                const auto point=applyPoint(geometry->transform,{4,8});
                expect(std::abs(point.x-.074)<1e-9);
                expect(std::abs(point.y-.224)<1e-9);
            }
            context.erase(replacement,Key(u"transform"));
            frozen=freezeForeign();
            geometry=frozen?std::get_if<scene::ForeignObjectGeometry>(&frozen->geometry):nullptr;
            expect(geometry!=nullptr);
            if(geometry){
                const auto point=applyPoint(geometry->transform,{4,8});
                expect(std::abs(point.x-.004)<1e-9);
                expect(std::abs(point.y-.008)<1e-9);
            }
        }
        const auto childList=context.get(Value(returnedObject),Key(u"children"));
        expect(context.arrayLength(std::get<ObjectRef>(childList.data))==2_u);
        const auto mappedHtml=session->services().sceneContextNodes().sourceNode(htmlPassedToHelper);
        expect(mappedHtml.has_value());
        if(mappedHtml){
            const auto mappedHandle=Value(session->services().sceneContextNodes().handle(*mappedHtml));
            const auto htmlIdentity=std::get<ObjectRef>(htmlPassedToHelper.data);
            expect(context.hasOwn(htmlIdentity,Key(u"kind")));
            expect(context.hasOwn(htmlIdentity,Key(u"attributes")));
            expect(context.hasOwn(htmlIdentity,Key(u"children")));
            expect(context.hasOwn(htmlIdentity,Key(u"styles")));
            expect(context.hasOwn(htmlIdentity,Key(u"parent")));
            expect(context.get(htmlIdentity,Key(u"parent"))==returnedByHelper);
            const auto sourceAttributes=context.get(htmlPassedToHelper,Key(u"attributes"));
            context.set(sourceAttributes,Key(u"data-after-import"),Value(u"same owner"));
            const auto sceneAttributes=context.get(mappedHandle,Key(u"attributes"));
            expect(context.get(sceneAttributes,Key(u"data-after-import"))==Value(u"same owner"));

            // Replacing an ordinary Source field preserves the new owner's
            // identity. Previously returned field objects remain detached.
            const auto replacementAttributes=context.object();
            context.set(replacementAttributes,Key(u"data-replacement"),Value(u"initial"));
            context.set(htmlPassedToHelper,Key(u"attributes"),Value(replacementAttributes));
            const auto currentAttributes=context.get(htmlPassedToHelper,Key(u"attributes"));
            expect(std::get<ObjectRef>(currentAttributes.data)==replacementAttributes);
            const auto sceneCurrentAttributes=context.get(mappedHandle,Key(u"attributes"));
            expect(std::get<ObjectRef>(sceneCurrentAttributes.data)==replacementAttributes);
            context.set(replacementAttributes,Key(u"data-live"),Value(u"updated"));
            expect(context.get(context.get(mappedHandle,Key(u"attributes")),Key(u"data-live"))==Value(u"updated"));
            context.set(sourceAttributes,Key(u"data-old-alias"),Value(u"detached"));
            expect(!context.hasOwn(replacementAttributes,Key(u"data-old-alias")));

            // getHTML creates mjx-html -> section -> b -> text for this fixture.
            Value textIdentity=htmlPassedToHelper;
            for(const auto* index:{u"0",u"0",u"0"}){
                const auto children=context.get(textIdentity,Key(u"children"));
                textIdentity=context.get(children,Key(index));
            }
            expect(context.get(textIdentity,Key(u"kind"))==Value(u"#text"));
            const auto textObjectIdentity=std::get<ObjectRef>(textIdentity.data);
            expect(context.hasOwn(textObjectIdentity,Key(u"value")));
            expect(!context.hasOwn(textObjectIdentity,Key(u"attributes")));
            expect(!context.hasOwn(textObjectIdentity,Key(u"children")));
            expect(!context.hasOwn(textObjectIdentity,Key(u"styles")));
            const auto mappedText=session->services().sceneContextNodes().sourceNode(textIdentity);
            expect(mappedText.has_value());
            if(mappedText){
                const auto textHandle=Value(session->services().sceneContextNodes().handle(*mappedText));
                context.set(textIdentity,Key(u"value"),Value(u"changed through Lite identity"));
                expect(context.get(textHandle,Key(u"value"))==Value(u"changed through Lite identity"));
                const auto textObject=std::get<ObjectRef>(textIdentity.data);
                const auto textNode=session->services().lite().nodeFor(textObject);
                expect(adaptor->value(textNode)==u"changed through Lite identity");
                const auto* frozen=std::get_if<scene::SourceTextPrimitive>(
                    &imported->children.front()->children.front()->children.front()->semantic);
                expect(frozen!=nullptr);
                if(frozen)expect(frozen->text==u"original child");
                context.set(textIdentity,Key(u"value"),Value(23.0));
                expect(context.get(textIdentity,Key(u"value"))==Value(23.0));
                expect(context.get(textHandle,Key(u"value"))==Value(23.0));
                expect(adaptor->value(textNode)==u"23");
            }

            const auto sectionChildren=context.get(htmlPassedToHelper,Key(u"children"));
            const auto section=context.get(sectionChildren,Key(u"0"));
            const auto sectionChildrenValue=context.get(section,Key(u"children"));
            const auto commentIdentity=context.get(sectionChildrenValue,Key(u"1"));
            expect(context.get(commentIdentity,Key(u"kind"))==Value(u"#comment"));
            const auto boldIdentity=context.get(sectionChildrenValue,Key(u"0"));
            context.set(sectionChildrenValue,Key(u"0"),commentIdentity);
            expect(context.get(sectionChildrenValue,Key(u"0"))==commentIdentity);
            expect(context.get(boldIdentity,Key(u"parent"))==section);
            // The pinned LiteAdaptor clone retains comments in children but
            // leaves their parent field unset; array slots and parent fields
            // are independent Source-owned state.
            expect(!context.hasOwn(commentIdentity,Key(u"parent")));
            context.set(boldIdentity,Key(u"parent"),htmlPassedToHelper);
            expect(context.get(boldIdentity,Key(u"parent"))==htmlPassedToHelper);
            expect(context.get(sectionChildrenValue,Key(u"0"))==commentIdentity);
            // The children field itself is an ordinary replaceable array
            // object. The Lite identity and Scene handle expose that same
            // current owner, while the completed Scene remains immutable.
            const auto replacementChildren=context.array();
            context.set(replacementChildren,Key(u"0"),commentIdentity);
            context.set(htmlPassedToHelper,Key(u"children"),Value(replacementChildren));
            const auto currentChildren=context.get(htmlPassedToHelper,Key(u"children"));
            expect(std::get<ObjectRef>(currentChildren.data)==replacementChildren);
            context.set(replacementChildren,Key(u"1"),textIdentity);
            expect(context.get(htmlPassedToHelper,Key(u"children"))==Value(replacementChildren));
            const auto sceneChildren=context.get(mappedHandle,Key(u"children"));
            expect(std::get<ObjectRef>(sceneChildren.data)==replacementChildren);
            expect(context.get(sceneChildren,Key(u"1"))==textIdentity);
            expect(context.arrayLength(replacementChildren)==2_u);
            // Scene completion is an immutable snapshot of the XML subtree at
            // typeset time; later writes through the retained Lite DOM identity
            // do not mutate the completed Scene.
            expect(imported->children.size()==1_u);
            expect(imported->children.front()->kind==u"section");
            const auto mappedComment=session->services().sceneContextNodes().sourceNode(commentIdentity);
            expect(mappedComment.has_value());
            if(mappedComment){
                const auto commentHandle=Value(session->services().sceneContextNodes().handle(*mappedComment));
                context.set(commentIdentity,Key(u"value"),Value(u"changed comment"));
                expect(context.get(commentHandle,Key(u"value"))==Value(u"changed comment"));
                const auto commentObject=std::get<ObjectRef>(commentIdentity.data);
                expect(adaptor->value(session->services().lite().nodeFor(commentObject))==u"changed comment");
            }

            // The adaptor's append operation follows Source's explicit order:
            // remove from the old parent's one indexed slot, append, then set
            // the parent field. Direct slot/parent writes above remain separate.
            const auto htmlObject=std::get<ObjectRef>(htmlPassedToHelper.data);
            const auto appendObject=std::get<ObjectRef>(commentIdentity.data);
            const auto htmlNode=session->services().lite().nodeFor(htmlObject);
            const auto appendNode=session->services().lite().nodeFor(appendObject);
            adaptor->append(htmlNode,appendNode);
            expect(context.get(commentIdentity,Key(u"parent"))==htmlPassedToHelper);
            expect(context.hasOwn(appendObject,Key(u"parent")));
            expect(context.hasOwn(htmlObject,Key(u"parent")));
            expect(context.arrayLength(std::get<ObjectRef>(sectionChildrenValue.data))==2_u);
            const auto htmlCurrentChildren=context.get(htmlPassedToHelper,Key(u"children"));
            expect(context.arrayLength(std::get<ObjectRef>(htmlCurrentChildren.data))==3_u);
            expect(context.get(htmlCurrentChildren,Key(u"2"))==commentIdentity);
            scene::Builder refreshed;
            const auto liveHtml=session->services().sceneContextNodes().resolve(
                session->services().sceneContextNodes().handle(*mappedHtml));
            refreshed.addRoot(liveHtml);
            const auto refreshedScene=std::move(refreshed).finish();
            expect(refreshedScene.roots().front()->children.size()==3_u);
            if(refreshedScene.roots().front()->children.size()==3_u)
                expect(refreshedScene.roots().front()->children[2]->kind==u"#comment");

            // Source deletes the own parent descriptor without changing the
            // old parent's children array. A later read-only data descriptor
            // is observed by the Lite adaptor; append removes the indexed old
            // slot before its attempted parent=null write throws.
            context.erase(appendObject,Key(u"parent"));
            expect(!context.hasOwn(appendObject,Key(u"parent")));
            expect(std::holds_alternative<Undefined>(context.get(commentIdentity,Key(u"parent")).data));
            expect(!std::holds_alternative<adaptors::lite::LiteNodeRef>(adaptor->parent(appendNode)));
            context.define(appendObject,Key(u"parent"),Descriptor::data(htmlPassedToHelper,false,true,true));
            const auto beforeReadonlyAppend=context.arrayLength(std::get<ObjectRef>(htmlCurrentChildren.data));
            bool readonlyAppendFailed=false;
            try{adaptor->append(htmlNode,appendNode);}
            catch(const Exception& error){readonlyAppendFailed=error.name=="TypeError";}
            expect(readonlyAppendFailed);
            const auto afterReadonlyAppend=context.arrayLength(std::get<ObjectRef>(htmlCurrentChildren.data));
            expect(afterReadonlyAppend<beforeReadonlyAppend);
            expect(beforeReadonlyAppend-afterReadonlyAppend==1);
            expect(context.get(commentIdentity,Key(u"parent"))==htmlPassedToHelper);

            std::size_t iteratorGetterCalls=0,nextCalls=0;
            const auto iterable=context.object();
            const std::array<Value,2> orderedChildren{commentIdentity,textIdentity};
            const auto iteratorMethod=context.function(u"[Symbol.iterator]",
                [&nextCalls,orderedChildren](Context& cx,Value,std::span<const Value>)->Value{
                    // Each traversal gets fresh iterator state; the global
                    // counter below measures calls only, never the index.
                    const auto cursor=std::make_shared<std::size_t>(0);
                    const auto iterator=cx.object();
                    const auto next=cx.function(u"next",
                        [&nextCalls,orderedChildren,cursor](Context& stepContext,Value,std::span<const Value>)->Value{
                            ++nextCalls;
                            const auto index=(*cursor)++;
                            const auto step=stepContext.object();
                            stepContext.define(step,Key(u"done"),Descriptor::data(Value(index>=orderedChildren.size())));
                            stepContext.define(step,Key(u"value"),Descriptor::data(
                                index<orderedChildren.size()?orderedChildren[index]:Value(Undefined{})));
                            return Value(step);
                        });
                    cx.define(iterator,Key(u"next"),Descriptor::data(next));
                    return Value(iterator);
                });
            const auto iteratorGetter=context.function(u"get [Symbol.iterator]",
                [&iteratorGetterCalls,iteratorMethod](Context&,Value,std::span<const Value>)->Value{
                    ++iteratorGetterCalls;return Value(iteratorMethod);
                });
            context.define(iterable,Key(context.intrinsics().symbolIterator),
                Descriptor::access(iteratorGetter,{},true,true));
            context.set(htmlPassedToHelper,Key(u"children"),Value(iterable));
            expect(context.get(htmlPassedToHelper,Key(u"children"))==Value(iterable));
            const auto adaptedChildren=adaptor->childNodes(htmlNode);
            expect(adaptedChildren.slots.size()==2_u);
            scene::Builder iterableBuilder;
            iterableBuilder.addRoot(liveHtml);
            const auto iterableScene=std::move(iterableBuilder).finish();
            expect(iteratorGetterCalls==2_u);
            expect(nextCalls==6_u);
            expect(iterableScene.roots().front()->children.size()==2_u);
            if(iterableScene.roots().front()->children.size()==2_u){
                expect(iterableScene.roots().front()->children[0]->kind==u"#comment");
                expect(iterableScene.roots().front()->children[1]->kind==u"#text");
            }
        }
    };
};
