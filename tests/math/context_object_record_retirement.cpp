#include "support/folia_test.hpp"
import folia.math.util.native_identities;
import folia.math.util.native_data_fields;
namespace {using namespace boost::ut;using namespace folia::math;namespace opts=folia::math::options;
suite context_object_record_retirement=[] {
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
