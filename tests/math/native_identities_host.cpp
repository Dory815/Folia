#include "support/folia_test.hpp"
import folia.math.util.native_identities;
import folia.math.core.mml_tree.mml_factory;
namespace {using namespace boost::ut;using namespace folia::math;namespace opts=folia::math::options;
suite native_identity_host=[] {
"math_port_native_identity_host_lifetime"_test=[] {
    std::weak_ptr<int> weak;
    {opts::Context context;auto type=context.constructor(u"owner");
     {NativeIdentityBindings<std::shared_ptr<int>> binding(context);auto owner=std::make_shared<int>(7);weak=owner;
      auto object=binding.instance(owner,type);expect(eq(binding.resolve(object),owner));owner.reset();}
     expect(!weak.expired());}
    expect(weak.expired());
};
"math_port_native_identity_host_reject_conflicts"_test=[] {
    opts::Context context,foreign;NativeIdentityBindings<std::shared_ptr<int>> binding(context);
    auto a=std::make_shared<int>(1),b=std::make_shared<int>(2);auto object=context.object();binding.bind(a,object);
    expect(throws<opts::Exception>([&]{binding.bind(b,object);}));
    expect(throws<opts::Exception>([&]{binding.bind(a,foreign.object());}));
    expect(throws<opts::Exception>([&]{binding.bind({},context.object());}));
    expect(throws<opts::Exception>([&]{binding.resolve(context.object());}));
    expect(throws<opts::Exception>([&]{binding.resolve(foreign.object());}));
    expect(eq(binding.resolve(object),a));
    auto unused=context.object();expect(binding.bind(a,unused)==object);
};
"math_port_native_identity_host_reject_noncanonical_payload"_test=[] {
    opts::Context context;NativeIdentityBindings<std::shared_ptr<int>> binding(context);
    auto native=std::make_shared<int>(1);auto canonical=binding.bind(native,context.object());
    auto unrelated=context.object();context.payload(unrelated)=NativeBindingPayload<std::shared_ptr<int>>{native,{}};
    expect(throws<opts::Exception>([&]{binding.resolve(unrelated);}));
    context.payload(canonical)=std::make_shared<int>(2);
    expect(throws<opts::Exception>([&]{binding.resolve(canonical);}));
};
"math_port_native_identity_host_raw_owner_retained_and_released"_test=[] {
    opts::Context context;NativeIdentityBindings<int*> binding(context);
    auto owner=std::make_shared<int>(7);std::weak_ptr<int> weak=owner;auto* raw=owner.get();
    auto type=context.constructor(u"raw");
    expect(throws<opts::Exception>([&]{binding.instance(raw,type);}));
    auto object=binding.instance(raw,type,owner);owner.reset();
    expect(binding.bind(raw,context.object())==object);
    expect(!weak.expired());expect(eq(*binding.resolve(object),7));
    binding.clear();expect(weak.expired());
    expect(throws<opts::Exception>([&]{binding.resolve(object);}));
    expect(!binding.find(raw).has_value());
};
"math_port_native_identity_host_payload_outlives_registry"_test=[] {
    opts::Context context;std::weak_ptr<int> weak;opts::ObjectRef object;
    {NativeIdentityBindings<int*> binding(context);auto owner=std::make_shared<int>(11);weak=owner;
     object=binding.instance(owner.get(),context.constructor(u"raw"),owner);}
    expect(!weak.expired());
    const auto& payload=std::any_cast<const NativeBindingPayload<int*>&>(context.payload(object));
    expect(eq(*payload.identity,11));
    context.payload(object).reset();expect(weak.expired());
};
"math_port_native_identity_host_actual_mml_graph_lifetime"_test=[] {
    opts::Context context;NativeIdentityBindings<mml::MmlNode*> binding(context);
    auto factory=std::make_shared<mml::MmlFactory>();std::weak_ptr<mml::MmlFactory> weak=factory;
    auto* node=factory->create("mi");node->setProperty("alive",19);
    auto object=binding.instance(node,context.constructor(u"MmlMi"),factory);
    factory.reset();expect(!weak.expired());
    expect(binding.resolve(object)->getProperty("alive")==Property(19));
    binding.clear();expect(weak.expired());
    expect(throws<opts::Exception>([&]{binding.resolve(object);}));
};
};}
