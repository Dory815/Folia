#include "support/folia_test.hpp"
import folia.math.util.native_identities;
import folia.math.util.native_data_fields;
namespace {using namespace boost::ut;using namespace folia::math;namespace opts=folia::math::options;
struct RegistryCleanupOwner final:opts::NativeAttachedOwner {
    std::shared_ptr<NativeIdentityBindings<std::shared_ptr<int>>> bindings;
    int& finalized;
    RegistryCleanupOwner(std::shared_ptr<NativeIdentityBindings<std::shared_ptr<int>>> value,int& count)
        :bindings(std::move(value)),finalized(count){}
    ~RegistryCleanupOwner(){bindings->clear();++finalized;}
    bool visitContextEdges(const opts::ContextEdgeVisitor&)const override{return true;}
};
suite context_object_record_retirement=[] {
"math_port_context_destruction_keeps_slots_readable_during_native_registry_cleanup"_test=[] {
    for(const bool ownerFirst:std::array<bool,2>{false,true}){
        int finalized=0;
        auto context=std::make_unique<opts::Context>();
        auto bindings=std::make_shared<NativeIdentityBindings<std::shared_ptr<int>>>(*context);
        std::weak_ptr<NativeIdentityBindings<std::shared_ptr<int>>> weakBindings=bindings;
        auto value=std::make_shared<int>(7);
        std::weak_ptr<int> weakValue=value;
        auto holder=ownerFirst?context->object():opts::ObjectRef{};
        const auto canonical=context->object();
        (void)bindings->bind(value,canonical);
        if(!ownerFirst)holder=context->object();
        context->attachOwner(holder,std::make_shared<RegistryCleanupOwner>(bindings,finalized));
        value.reset();bindings.reset();
        expect(!weakValue.expired()&&!weakBindings.expired());
        context.reset();
        expect(finalized==1) << "the native cleanup owner finishes in either record order";
        expect(weakValue.expired()&&weakBindings.expired());
    }
};
"math_port_context_record_inventory_observes_creation_and_retirement_without_getters"_test=[] {
    opts::Context context;
    const auto before=context.recordCountsForDiagnostics();
    const auto object=context.object();
    std::size_t getterCalls=0;
    std::size_t callableCalls=0;
    auto owner=std::make_shared<int>(9);
    context.bindData(object,opts::Key(u"native"),std::make_shared<opts::NativeDataField>(owner,
        [&getterCalls]{++getterCalls;return opts::Value(9);},
        [](opts::Value){}));
    const auto function=context.function(u"inventory",[&callableCalls](opts::Context&,opts::Value,std::span<const opts::Value>){++callableCalls;return opts::Value(1);});
    const auto created=context.recordCountsForDiagnostics();
    expect(created.slots==before.slots+2);
    expect(created.live==before.live+2);
    expect(created.retired==before.retired);
    expect(created.ownerBearing==before.ownerBearing+2);
    expect(getterCalls==0u);
    expect(callableCalls==0u);

    context.retireObjectRecord(function);
    const auto oneRetired=context.recordCountsForDiagnostics();
    expect(oneRetired.slots==created.slots);
    expect(oneRetired.live+1==created.live);
    expect(oneRetired.retired==created.retired+1);
    expect(oneRetired.ownerBearing+1==created.ownerBearing);
    expect(getterCalls==0u);
    expect(callableCalls==0u);
    context.retireObjectRecord(object);
    const auto bothRetired=context.recordCountsForDiagnostics();
    expect(bothRetired.slots==created.slots);
    expect(bothRetired.live+2==created.live);
    expect(bothRetired.retired==created.retired+2);
    expect(bothRetired.ownerBearing+2==created.ownerBearing);
    expect(getterCalls==0u);
    expect(callableCalls==0u);
};
"math_port_context_object_record_retirement_releases_all_owner_edges_and_keeps_stale_slot"_test=[] {
    opts::Context context;auto type=context.constructor(u"owner");
    NativeIdentityBindings<std::shared_ptr<int>> bindings(context);
    auto owner=std::make_shared<int>(7);std::weak_ptr<int> weak=owner;std::size_t getterCalls=0;
    const auto object=bindings.instance(owner,type);
    context.bindData(object,opts::Key(u"value"),std::make_shared<opts::NativeDataField>(owner,
        [owner,&getterCalls]{++getterCalls;return opts::Value(*owner);},
        [owner](opts::Value){(void)owner;}));
    owner.reset();expect(!weak.expired());
    expect(!bindings.retire(std::make_shared<int>(7)));
    auto canonical=bindings.resolve(object);
    expect(bindings.retire(canonical));
    canonical.reset();
    expect(weak.expired());
    expect(throws<opts::Exception>([&]{(void)context.kind(object);}));
    expect(throws<opts::Exception>([&]{(void)context.get(opts::Value(object),opts::Key(u"value"));}));
    expect(throws<opts::Exception>([&]{(void)context.descriptor(opts::Value(object),opts::Key(u"value"));}));
    expect(getterCalls==0u);expect(bindings.canonicalHandleCount()==0u);
    const auto later=context.object();expect(later.index>object.index);
};
};}
