#include "support/folia_test.hpp"
import folia.math.util.option_types;
import folia.math.util.map_bindings;
import folia.math.util.native_identities;
namespace {
using namespace boost::ut;
namespace opts=folia::math::options;

bool contains(std::span<const opts::ObjectRef> values,opts::ObjectRef value){
    return std::find(values.begin(),values.end(),value)!=values.end();
}
bool contains(std::span<const opts::SymbolRef> values,opts::SymbolRef value){
    return std::find(values.begin(),values.end(),value)!=values.end();
}
opts::ObjectRef nullObject(opts::Context& context){return context.nullObject();}

struct EdgeTrace final:opts::NativeOwnerTrace {
    std::vector<opts::Value> edges;
    bool complete=true;
    bool visitContextEdges(const opts::ContextEdgeVisitor& visit)const override{
        for(const auto& edge:edges)visit(edge);
        return complete;
    }
};

struct NativeRecordFixture final:opts::NativeRecordView {
    mutable std::size_t ownReads=0,keyReads=0;
    mutable std::size_t prototypeReads=0;
    std::optional<opts::ObjectRef> currentPrototype;
    bool throwOnObservation=false;
    void check()const{if(throwOnObservation)throw 1;}
    std::optional<opts::Descriptor> ownDescriptor(const opts::Key&)const override{
        check();
        ++ownReads;return {};
    }
    bool ownPropertyEnumerable(const opts::Key&)const override{check();return false;}
    std::vector<opts::Key> ownKeys()const override{check();++keyReads;return {};}
    void defineOwn(const opts::Key&,opts::Descriptor)override{check();}
    void deleteOwn(const opts::Key&)override{check();}
    std::optional<opts::ObjectRef> prototype()const override{check();++prototypeReads;return currentPrototype;}
    void setPrototype(std::optional<opts::ObjectRef> value)override{check();currentPrototype=value;}
};

struct NativeRecordPrototypeTrace final:opts::NativeOwnerTrace {
    std::shared_ptr<NativeRecordFixture> owner;
    bool visitContextEdges(const opts::ContextEdgeVisitor& visit)const override{
        // The owner reports its stored authoritative edge without calling the
        // NativeRecordView virtual prototype API.
        if(owner->currentPrototype)visit(opts::Value(*owner->currentPrototype));
        return true;
    }
};

static_assert(!std::is_default_constructible_v<opts::ContextTraceReport>);
static_assert(!std::is_default_constructible_v<opts::ContextTraceReport::CompleteEdgeClosure>);
suite context_raw_edge_trace=[] {
"math_port_context_raw_trace_keeps_default_intrinsic_prototype_opaque_owners_pinned"_test=[] {
    opts::Context context;const auto root=context.object();
    const std::array roots{root};const auto report=context.traceReachable(roots);
    expect(contains(report.objects(),root));
    expect(contains(report.objects(),context.intrinsics().objectPrototype));
    expect(report.hasOpaquePins());
    expect(!report.completeEdgeClosure().has_value());
};
"math_port_context_raw_trace_walks_data_prototype_accessor_and_cycle_without_getting"_test=[] {
    opts::Context context;std::size_t getterCalls=0;
    const auto root=context.object(),prototype=context.object(),left=context.object(),right=context.object();
    const auto unreachable=context.object();
    const auto getter=context.function(u"getter",[&](opts::Context&,opts::Value,std::span<const opts::Value>){
        ++getterCalls;return opts::Value(1);
    });
    const auto setter=context.function(u"setter",[&](opts::Context&,opts::Value,std::span<const opts::Value>){
        ++getterCalls;return opts::Value(opts::Undefined{});
    });
    context.setPrototype(left,prototype);
    context.set(root,opts::Key(u"left"),opts::Value(left));
    const auto symbolKey=context.symbol(u"edge");
    context.set(root,opts::Key(symbolKey),opts::Value(right));
    context.set(left,opts::Key(u"right"),opts::Value(right));
    context.set(right,opts::Key(u"cycle"),opts::Value(left));
    context.define(left,opts::Key(u"computed"),opts::Descriptor::access(getter,setter));
    const std::array roots{root};
    const auto trace=context.traceReachable(roots);
    expect(contains(trace.objects(),root));expect(contains(trace.objects(),prototype));
    expect(contains(trace.objects(),left));expect(contains(trace.objects(),right));
    expect(contains(trace.objects(),getter));expect(contains(trace.objects(),setter));
    expect(!contains(trace.objects(),unreachable));
    expect(contains(trace.symbols(),symbolKey));
    expect(!trace.objects().empty());
    expect(eq(getterCalls,std::size_t(0)));
};
"math_port_context_raw_trace_uses_registered_native_owner_edges"_test=[] {
    opts::Context context;auto target=nullObject(context);auto view=std::make_shared<NativeRecordFixture>();
    const auto native=context.nativeRecord(view);
    auto visitor=std::make_shared<EdgeTrace>();visitor->edges.emplace_back(target);
    context.registerNativeOwnerTrace(native,opts::NativeOwnerPart::nativeView,visitor);
    const std::array roots{native};
    const auto trace=context.traceReachable(roots);
    expect(contains(trace.objects(),target));expect(!trace.hasOpaquePins());
    expect(eq(view->ownReads,std::size_t(0)));expect(eq(view->keyReads,std::size_t(0)));
};
"math_port_context_raw_trace_uses_current_native_record_prototype_without_observing_view"_test=[] {
    opts::Context context;auto view=std::make_shared<NativeRecordFixture>();
    const auto native=context.nativeRecord(view);
    const auto oldPrototype=nullObject(context),newPrototype=nullObject(context);
    context.setPrototype(native,oldPrototype);
    auto visitor=std::make_shared<NativeRecordPrototypeTrace>();visitor->owner=view;
    context.registerNativeOwnerTrace(native,opts::NativeOwnerPart::nativeView,visitor);
    const std::array roots{native};
    const auto before=context.traceReachable(roots);
    expect(contains(before.objects(),oldPrototype));
    context.setPrototype(native,newPrototype);
    view->throwOnObservation=true;
    const auto after=context.traceReachable(roots);
    expect(contains(after.objects(),newPrototype));expect(!contains(after.objects(),oldPrototype));
    expect(eq(view->ownReads,std::size_t(0)));expect(eq(view->keyReads,std::size_t(0)));
    expect(eq(view->prototypeReads,std::size_t(1)));
    expect(!after.hasOpaquePins());
};
"math_port_context_raw_trace_visits_registered_callback_and_payload_edges"_test=[] {
    opts::Context context;const auto callbackEdge=nullObject(context),payloadEdge=nullObject(context);
    std::size_t calls=0;
    const auto callback=context.function(u"callback",
        [&](opts::Context&,opts::Value,std::span<const opts::Value>){++calls;return opts::Value(opts::Undefined{});},
        std::make_shared<int>(3));
    context.setPrototype(callback,std::nullopt);
    auto callbackTrace=std::make_shared<EdgeTrace>();callbackTrace->edges.emplace_back(callbackEdge);
    auto payloadTrace=std::make_shared<EdgeTrace>();payloadTrace->edges.emplace_back(payloadEdge);
    context.registerNativeOwnerTrace(callback,opts::NativeOwnerPart::callable,callbackTrace);
    context.registerNativeOwnerTrace(callback,opts::NativeOwnerPart::payload,payloadTrace);
    const std::array roots{callback};const auto trace=context.traceReachable(roots);
    expect(contains(trace.objects(),callbackEdge));expect(contains(trace.objects(),payloadEdge));
    expect(!trace.hasOpaquePins());
    expect(eq(calls,std::size_t(0)));
};
"math_port_context_raw_trace_reports_unvisited_opaque_owners_as_pins"_test=[] {
    opts::Context context;const auto owner=nullObject(context);
    context.payload(owner)=std::make_shared<int>(7);
    const std::array roots{owner};
    const auto trace=context.traceReachable(roots);
    expect(trace.hasOpaquePins());expect(eq(trace.opaquePins().size(),std::size_t(1)));
    if(!trace.opaquePins().empty()){
        expect(trace.opaquePins().front().object==owner);
        expect(trace.opaquePins().front().part==opts::NativeOwnerPart::payload);
    }
};
"math_port_context_raw_trace_pins_incomplete_visitor_while_keeping_known_edges"_test=[] {
    opts::Context context;const auto ownerEdge=context.object(),knownEdge=context.object();
    const auto owner=context.function(u"partial owner",{},std::make_shared<int>(1));
    auto partial=std::make_shared<EdgeTrace>();partial->edges.emplace_back(knownEdge);partial->complete=false;
    context.registerNativeOwnerTrace(owner,opts::NativeOwnerPart::payload,partial);
    const std::array roots{owner};const auto report=context.traceReachable(roots);
    expect(contains(report.objects(),knownEdge));expect(!contains(report.objects(),ownerEdge));
    expect(report.hasOpaquePins());
};
"math_port_context_raw_trace_invalidates_payload_hook_on_mutable_access_but_not_const_read"_test=[] {
    opts::Context context;const auto owner=nullObject(context);
    context.setPayload(owner,std::make_shared<int>(1));
    const auto oldEdge=nullObject(context),newHiddenEdge=nullObject(context);
    auto traceOwner=std::make_shared<EdgeTrace>();traceOwner->edges.emplace_back(oldEdge);
    context.registerNativeOwnerTrace(owner,opts::NativeOwnerPart::payload,traceOwner);
    const opts::Context& readOnly=context;
    (void)readOnly.payload(owner);
    const std::array roots{owner};
    const auto stillTraced=context.traceReachable(roots);
    expect(contains(stillTraced.objects(),oldEdge));expect(!stillTraced.hasOpaquePins());

    context.setPayload(owner,std::make_shared<int>(2));
    const auto afterReplacement=context.traceReachable(roots);
    expect(!contains(afterReplacement.objects(),oldEdge));
    expect(!contains(afterReplacement.objects(),newHiddenEdge));
    expect(afterReplacement.hasOpaquePins());
    expect(eq(afterReplacement.opaquePins().size(),std::size_t(1)));
    if(!afterReplacement.opaquePins().empty())
        expect(afterReplacement.opaquePins().front().part==opts::NativeOwnerPart::payload);
};
"math_port_context_payload_alias_before_hook_permanently_blocks_trace_registration"_test=[] {
    opts::Context context;const auto owner=context.object();
    auto& borrowed=context.payload(owner);
    borrowed=std::make_shared<int>(1);
    auto trace=std::make_shared<EdgeTrace>();
    bool rejected=false;
    try{context.registerNativeOwnerTrace(owner,opts::NativeOwnerPart::payload,trace);}
    catch(const opts::Exception&){rejected=true;}
    expect(rejected);
    context.setPayload(owner,std::make_shared<int>(2));
    borrowed=std::make_shared<int>(3); // The escaped alias remains writable.
    const std::array roots{owner};const auto report=context.traceReachable(roots);
    expect(report.hasOpaquePins());
    if(!report.opaquePins().empty())
        expect(report.opaquePins().front().part==opts::NativeOwnerPart::payload);
};
"math_port_context_raw_trace_rejects_duplicate_owner_hook_until_explicit_unregister"_test=[] {
    opts::Context context;const auto target=context.object();
    const auto owner=context.function(u"hook owner",{},std::make_shared<int>(1));
    context.setPrototype(owner,std::nullopt);
    auto first=std::make_shared<EdgeTrace>();first->edges.emplace_back(target);
    auto second=std::make_shared<EdgeTrace>();
    context.registerNativeOwnerTrace(owner,opts::NativeOwnerPart::payload,first);
    bool rejected=false;
    try{context.registerNativeOwnerTrace(owner,opts::NativeOwnerPart::payload,second);}
    catch(const opts::Exception&){rejected=true;}
    expect(rejected);
    context.unregisterNativeOwnerTrace(owner,opts::NativeOwnerPart::payload);
    context.registerNativeOwnerTrace(owner,opts::NativeOwnerPart::payload,second);
    const std::array roots{owner};const auto report=context.traceReachable(roots);
    expect(!contains(report.objects(),target));expect(!report.hasOpaquePins());
};
"math_port_context_raw_trace_recomputes_after_root_replacement"_test=[] {
    opts::Context context;const auto root=nullObject(context),oldValue=nullObject(context),newValue=nullObject(context);
    context.set(root,opts::Key(u"edge"),opts::Value(oldValue));
    const std::array roots{root};const auto before=context.traceReachable(roots);
    expect(contains(before.objects(),oldValue));
    context.set(root,opts::Key(u"edge"),opts::Value(newValue));
    const auto after=context.traceReachable(roots);
    expect(contains(after.objects(),newValue));expect(!contains(after.objects(),oldValue));
};
"math_port_context_raw_trace_handles_native_field_visitor"_test=[] {
    opts::Context context;const auto owner=nullObject(context),target=nullObject(context);
    std::size_t reads=0,writes=0,removes=0;
    const auto fieldKey=context.symbol(u"nativeCell");
    auto traceOwner=std::make_shared<EdgeTrace>();traceOwner->edges.emplace_back(target);
    auto cell=std::make_shared<opts::NativeDataField>(std::make_shared<int>(1),
        [&]{++reads;return opts::Value(opts::Undefined{});},
        [&](opts::Value){++writes;},[&]{++removes;},traceOwner);
    context.bindData(owner,opts::Key(fieldKey),cell);
    const std::array roots{owner};auto trace=context.traceReachable(roots);
    expect(contains(trace.objects(),target));expect(contains(trace.symbols(),fieldKey));
    expect(!trace.hasOpaquePins());
    const auto reclaimable=trace.completeEdgeClosure();
    expect(reclaimable.has_value());
    if(reclaimable){
        expect(reclaimable->contextIdentity()==trace.contextIdentity());
        expect(reclaimable->suppliedRoots().size()==1);
        expect(std::find(reclaimable->objects().begin(),reclaimable->objects().end(),target)!=reclaimable->objects().end());
    }
    auto moved=std::move(trace);
    expect(!trace.isValid());
    expect(!trace.completeEdgeClosure().has_value());
    expect(moved.isValid());
    expect(eq(reads,std::size_t(0)));expect(eq(writes,std::size_t(0)));
    expect(eq(removes,std::size_t(0)));
};
"math_port_native_identity_registry_enumerates_its_actual_strong_handles"_test=[] {
    opts::Context context;
    folia::math::NativeIdentityBindings<std::shared_ptr<int>> identities(context);
    const auto owner=std::make_shared<int>(7);
    const auto identity=context.object();
    identities.bind(owner,identity);
    std::size_t count=0;
    identities.visitCanonicalHandles([&](const auto& storedOwner,opts::ObjectRef storedHandle){
        expect(storedOwner==owner);expect(storedHandle==identity);++count;
    });
    expect(eq(count,std::size_t(1)));
    expect(eq(identities.canonicalHandleCount(),std::size_t(1)));
};
"math_port_context_raw_trace_visits_raw_context_map_values_without_callbacks"_test=[] {
    opts::Context context;opts::MapContextBindings maps(context);
    const auto oldEdge=nullObject(context),newEdge=nullObject(context);
    const auto storage=maps.createStorage();
    storage->set(opts::Value(u"macro"),opts::Value(oldEdge));
    const auto mapObject=maps.bind(storage);
    // This test isolates the registered payload owner's raw edges. The normal
    // Map prototype is covered by the default-intrinsic opaque-pin test above.
    context.setPrototype(mapObject,std::nullopt);
    const std::array roots{mapObject};
    const auto before=context.traceReachable(roots);
    expect(contains(before.objects(),oldEdge));expect(!before.hasOpaquePins());
    storage->set(opts::Value(u"macro"),opts::Value(newEdge));
    const auto after=context.traceReachable(roots);
    expect(contains(after.objects(),newEdge));expect(!contains(after.objects(),oldEdge));
    expect(!after.hasOpaquePins());
};
};
}
