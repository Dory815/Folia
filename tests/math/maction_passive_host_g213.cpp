#include "support/folia_test.hpp"
import folia.math;
import folia.math.adaptors.lite.lite_module;
import folia.math.output.scene;
import folia.math.output.svg.scene_context_nodes;
import folia.math.util.context_property_bindings;
import folia.math.util.regexp_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace scene=folia::math::output::scene;
namespace svg=folia::math::output::svg;
namespace lite=folia::math::adaptors::lite;
namespace opts=folia::math::options;

struct ProbeAttachedOwner final:opts::NativeAttachedOwner {
    opts::Value edge;
    bool visitContextEdges(const opts::ContextEdgeVisitor& visit)const override{
        visit(edge);return true;
    }
};

void addEvent(opts::Context& context,opts::ObjectRef receiver,std::u16string_view type,
              opts::ObjectRef callback){
    const auto method=context.get(opts::Value(receiver),opts::Key(u"addEventListener"));
    context.callMember(method,opts::Value(receiver),
        std::array<opts::Value,2>{opts::Value(std::u16string(type)),opts::Value(callback)});
}
bool reaches(const opts::ContextTraceReport& report,opts::ObjectRef object){
    return std::ranges::find(report.objects(),object)!=report.objects().end();
}
bool hasEvent(const scene::FrozenNodeRef& node,std::u16string_view eventType){
    if(!node)return false;
    if(std::ranges::find(node->eventTypes,eventType)!=node->eventTypes.end())return true;
    for(const auto& child:node->children)if(hasEvent(child,eventType))return true;
    return false;
}
bool hasEvent(const scene::Scene& value,std::u16string_view eventType){
    for(const auto& root:value.roots())if(hasEvent(root,eventType))return true;
    return false;
}
bool hasBothEvents(const scene::FrozenNodeRef& node,std::u16string_view first,
                   std::u16string_view second){
    if(!node)return false;
    const bool hasFirst=std::ranges::find(node->eventTypes,first)!=node->eventTypes.end();
    const bool hasSecond=std::ranges::find(node->eventTypes,second)!=node->eventTypes.end();
    if(hasFirst&&hasSecond)return true;
    for(const auto& child:node->children)if(hasBothEvents(child,first,second))return true;
    return false;
}
bool hasBothEvents(const scene::Scene& value,std::u16string_view first,
                   std::u16string_view second){
    for(const auto& root:value.roots())if(hasBothEvents(root,first,second))return true;
    return false;
}
}

suite maction_passive_host_g213=[] {
    "maction_passive Context attached owner is traced and clears on retirement"_test=[] {
        opts::Context context,other;
        const auto receiver=context.object();
        const auto callback=context.function(u"attached callback",[](opts::Context&,opts::Value,
            std::span<const opts::Value>){return opts::Value(opts::Undefined{});});
        auto attached=std::make_shared<ProbeAttachedOwner>();
        attached->edge=opts::Value(callback);
        const auto* expectedOwner=attached.get();
        std::weak_ptr<ProbeAttachedOwner> weak=attached;
        context.attachOwner(receiver,attached);
        // Mutable std::any access must not disable the independent attached-owner edge.
        context.payload(receiver)=17;
        expect(context.attachedOwner(receiver)==attached);
        expect(reaches(context.traceReachable(std::array<opts::ObjectRef,1>{receiver}),callback));
        expect(throws<opts::Exception>([&]{context.attachOwner(receiver,std::make_shared<ProbeAttachedOwner>());}));
        context.retireObjectRecord(receiver);
        attached.reset();
        expect(weak.expired()) << "record retirement drops the strong attached owner";
        expect(!context.attachedOwner(receiver));
        expect(!context.clearAttachedOwner(receiver,expectedOwner));
        expect(throws<opts::Exception>([&]{(void)other.attachedOwner(receiver);}))
            << "stale lookup distinguishes a foreign Context identity";
    };

    "maction_passive Lite registrations expose callback edges until request retirement"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        lite::LiteModule module(context,functions,properties,regexp);
        auto adaptor=module.constructAdaptor();
        const auto receiver=module.objectFor(adaptor->node(u"svg"));
        const auto mark=module.requestMark();
        int calls=0;
        const auto first=context.function(u"click",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>){++calls;return opts::Value(opts::Undefined{});});
        const auto second=context.function(u"hover",[](opts::Context&,opts::Value,
            std::span<const opts::Value>){return opts::Value(opts::Undefined{});});
        std::vector<std::u16string> registrationOrder;
        std::vector<opts::ObjectRef> receivers;
        auto prior=module.exchangeEventMetadataTarget([&](opts::Context&,opts::ObjectRef target,
            std::u16string_view type){receivers.push_back(target);registrationOrder.emplace_back(type);});
        addEvent(context,receiver,u"click",first);
        addEvent(context,receiver,u"mouseover",second);
        (void)module.exchangeEventMetadataTarget(std::move(prior));

        const std::array<opts::ObjectRef,1> roots{receiver};
        const auto active=context.traceReachable(roots);
        expect(receivers.size()==2_u&&receivers[0]==receiver&&receivers[1]==receiver);
        expect(static_cast<bool>(registrationOrder==std::vector<std::u16string>{u"click",u"mouseover"}));
        expect(reaches(active,first)&&reaches(active,second));

        const auto failedReceiver=module.objectFor(adaptor->node(u"g"));
        const auto failedCallback=context.function(u"rolled back listener",[](opts::Context&,opts::Value,
            std::span<const opts::Value>){return opts::Value(opts::Undefined{});});
        auto priorThrowing=module.exchangeEventMetadataTarget(
            [](opts::Context&,opts::ObjectRef,std::u16string_view){
                throw std::runtime_error("metadata observer failure");
            });
        bool observerThrew=false;
        try{addEvent(context,failedReceiver,u"mouseout",failedCallback);}
        catch(const std::runtime_error&){observerThrew=true;}
        (void)module.exchangeEventMetadataTarget(std::move(priorThrowing));
        expect(observerThrew);
        expect(!context.attachedOwner(failedReceiver)) << "failed observer leaves no empty listener owner";
        expect(!reaches(context.traceReachable(std::array<opts::ObjectRef,1>{failedReceiver}),failedCallback))
            << "failed observer rolls back the original callback edge";

        module.retireRequestNodesSince(mark);
        const auto retired=context.traceReachable(roots);
        expect(!reaches(retired,first)&&!reaches(retired,second));
        // The host removed only its own edges. A caller's independent callback
        // reference remains a valid Context function after request retirement.
        (void)context.call(first,opts::Undefined{});
        expect(calls==1_i);
    };

    "maction_passive expired Scene bridge checks provider lifetime before the Lite pointer"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        auto provider=std::make_shared<int>(1);
        svg::SceneContextNodes projection(context,u"ExpiredMactionSceneNode");
        projection.bindServiceOwner(provider);
        opts::ObjectRef receiver{};
        opts::Value bridge=opts::Undefined{};
        {
            lite::LiteModule module(context,functions,properties,regexp);
            projection.bindLiteModule(module);
            auto node=std::make_shared<scene::Node>();
            node->kind=u"svg";
            receiver=projection.handle(node);
            bridge=context.get(opts::Value(receiver),opts::Key(u"addEventListener"));
        }
        // The bridge's weak State can remain alive through projection while
        // its raw Lite pointer has expired; the provider lease must fail first.
        provider.reset();
        const auto callback=context.function(u"stale provider callback",[](opts::Context&,opts::Value,
            std::span<const opts::Value>){return opts::Value(opts::Undefined{});});
        const std::array<opts::Value,2> args{opts::Value(u"click"),opts::Value(callback)};
        bool expired=false;
        try{context.callMember(bridge,opts::Value(receiver),args);}
        catch(const opts::Exception& error){
            expired=error.name=="TypeError"&&error.message==u"Scene provider services have expired";
        }
        expect(expired) << "expired Scene bridge rejects before dereferencing its raw LiteModule pointer";
    };
    "maction_passive Scene metadata preserves projection and scope restoration"_test=[] {
        scene::Scene frozen;
        {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        lite::LiteModule module(context,functions,properties,regexp);
        auto adaptor=module.constructAdaptor();
        svg::SceneContextNodes projection(context,u"MactionPassiveSceneNode");
        const auto serviceOwner=std::make_shared<int>(1);
        projection.bindServiceOwner(serviceOwner);
        projection.bindPropertyBindings(properties);
        projection.bindLiteModule(module);
        const auto liteMark=module.requestMark();
        const auto sceneMark=projection.beginRequestScope();

        const auto sourceIdentity=module.objectFor(adaptor->node(u"svg"));
        const auto unprojected=module.objectFor(adaptor->node(u"g"));
        auto projectedNode=std::make_shared<scene::Node>();
        projectedNode->kind=u"svg";
        projectedNode->semantic=scene::SourceDomPrimitive{};
        projection.associateSourceNode(projectedNode,opts::Value(sourceIdentity));
        const auto sceneReceiver=projection.handle(projectedNode);
        scene::Builder builder;
        builder.addRoot(projectedNode);

        int fallbackTargetCalls=0;
        // Install a fallback target that can verify restoration after nested
        // normal and exceptional TargetScope exits.
        auto prior=module.exchangeEventMetadataTarget([&](opts::Context&,opts::ObjectRef,
            std::u16string_view){++fallbackTargetCalls;});
        const auto callback=context.function(u"listener",[](opts::Context&,opts::Value,
            std::span<const opts::Value>){return opts::Value(opts::Undefined{});});
        try{
            auto outer=projection.installTargets(module,builder);
            {
                auto inner=projection.installTargets(module,builder);
                addEvent(context,sceneReceiver,u"click",callback);
                addEvent(context,sourceIdentity,u"click",callback);
                addEvent(context,sourceIdentity,u"click",callback);
                addEvent(context,sourceIdentity,u"mouseover",callback);
                addEvent(context,unprojected,u"mouseout",callback);
                expect(static_cast<bool>(projectedNode->eventTypes==std::vector<std::u16string>{u"click",u"mouseover"}));
                expect(reaches(context.traceReachable(std::array<opts::ObjectRef,1>{sceneReceiver}),callback));
                expect(throws<opts::Exception>([&]{(void)module.nodeFor(sceneReceiver);}))
                    << "Scene Maction receiver is not forced through a Lite NodeId";
                expect(projection.handleCountForDiagnostics()==1_u);
                throw std::runtime_error("exercise TargetScope unwinding");
            }
        }catch(const std::runtime_error&){}

        // The pre-scope observer is restored after both nested scopes unwind.
        addEvent(context,unprojected,u"click",callback);
        expect(fallbackTargetCalls==1_i);
        expect(static_cast<bool>(projectedNode->eventTypes==std::vector<std::u16string>{u"click",u"mouseover"}));

        // A per-instance custom DOM method shadows the built-in Lite method and
        // is called with the real receiver/arguments.
        int customCalls=0;
        const auto custom=context.function(u"custom addEventListener",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value> args){
                if(receiver==opts::Value(unprojected)&&args.size()==2_u)++customCalls;
                return opts::Value(opts::Undefined{});
            });
        context.set(opts::Value(unprojected),opts::Key(u"addEventListener"),opts::Value(custom));
        const auto customMethod=context.get(opts::Value(unprojected),opts::Key(u"addEventListener"));
        context.callMember(customMethod,opts::Value(unprojected),
            std::array<opts::Value,2>{opts::Value(u"mouseout"),opts::Value(callback)});
        expect(customCalls==1_i);
        int sceneCustomCalls=0;
        const auto customSceneMethod=context.function(u"custom Scene addEventListener",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value> args){
                if(receiver==opts::Value(sceneReceiver)&&args.size()==2_u)++sceneCustomCalls;
                return opts::Value(opts::Undefined{});
            });
        context.set(opts::Value(sceneReceiver),opts::Key(u"addEventListener"),
            opts::Value(customSceneMethod));
        const auto sceneMethod=context.get(opts::Value(sceneReceiver),opts::Key(u"addEventListener"));
        context.callMember(sceneMethod,opts::Value(sceneReceiver),
            std::array<opts::Value,2>{opts::Value(u"mouseout"),opts::Value(callback)});
        expect(sceneCustomCalls==1_i) << "Scene receiver's own event method wins over the bridge prototype";
        (void)module.exchangeEventMetadataTarget(std::move(prior));

        frozen=std::move(builder).finish();
        projection.retireRequestScope(sceneMark);
        module.retireRequestNodesSince(liteMark);
        }
        // Context, LiteModule, SceneContextNodes, and request callbacks have
        // all retired; the immutable Scene retains only the pure event names.
        expect(frozen.roots().size()==1_u);
        if(frozen.roots().size()==1_u){
            const auto& node=frozen.roots().front();
            expect(static_cast<bool>(node->eventTypes==std::vector<std::u16string>{u"click",u"mouseover"}));
            expect(node->attributes.empty());
        }
    };

    "maction_passive public Renderer freezes events from Scene DOM receivers"_test=[] {
        std::optional<scene::Scene> toggleScene,tooltipScene;
        std::string toggleSvg,tooltipSvg;
        {
            folia::math::Renderer renderer;
            auto toggle=renderer.render(uR"(\toggle{x}{y}\endtoggle)");
            auto tooltip=renderer.render(uR"(\mathtip{x+1}{y^2})");
            toggleScene.emplace(std::move(toggle.scene));
            tooltipScene.emplace(std::move(tooltip.scene));
            toggleSvg=std::move(toggle.svg);
            tooltipSvg=std::move(tooltip.svg);
        }
        expect(toggleScene.has_value()&&hasEvent(*toggleScene,u"click"))
            << "toggle event metadata survives Renderer destruction and scene freeze";
        expect(tooltipScene.has_value()&&hasBothEvents(*tooltipScene,u"mouseover",u"mouseout"))
            << "tooltip mouseover/mouseout metadata stays on the action node after Renderer destruction";
        expect(toggleSvg.find("onclick=")==std::string::npos&&
               toggleSvg.find("eventTypes")==std::string::npos)
            << "event metadata is not serialized as SVG attributes";
        expect(tooltipSvg.find("onmouseover=")==std::string::npos&&
               tooltipSvg.find("onmouseout=")==std::string::npos&&
               tooltipSvg.find("eventTypes")==std::string::npos)
            << "passive events are metadata only; no dispatch attribute is emitted";
    };
};
