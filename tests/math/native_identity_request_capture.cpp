#include "support/folia_test.hpp"
import folia.math.util.native_identities;
namespace {using namespace boost::ut;using namespace folia::math;namespace opts=folia::math::options;
suite native_identity_request_capture=[] {
"math_port_native_identity_request_capture_retires_transient_and_preserves_promoted_binding"_test=[] {
    opts::Context context;NativeIdentityBindings<std::shared_ptr<int>> bindings(context);
    const auto type=context.constructor(u"identity");
    auto durable=std::make_shared<int>(1);auto transient=std::make_shared<int>(2);
    auto promoted=std::make_shared<int>(3);
    const auto durableObject=bindings.instance(durable,type);
    expect(bindings.ownsHandle(durableObject));
    const auto capture=bindings.beginRequestCapture();
    const auto transientObject=bindings.instance(transient,type);
    const auto promotedObject=bindings.instance(promoted,type);
    expect(bindings.ownsHandle(transientObject));expect(bindings.ownsHandle(promotedObject));
    bindings.promoteFromRequestCapture(capture,promoted);
    bindings.retireCapturedRequestBindings(capture);
    expect(bindings.resolve(durableObject)==durable);
    expect(bindings.resolve(promotedObject)==promoted);
    expect(bindings.ownsHandle(durableObject));expect(bindings.ownsHandle(promotedObject));
    expect(!bindings.ownsHandle(transientObject));
    expect(throws<opts::Exception>([&]{(void)bindings.resolve(transientObject);}));
    expect(throws<opts::Exception>([&]{(void)context.kind(transientObject);}));
    expect(bindings.canonicalHandleCount()==2u);
};
"math_port_native_identity_registry_enumerates_actual_handles_and_finds_pointee"_test=[] {
    opts::Context context;NativeIdentityBindings<std::shared_ptr<int>> bindings(context);
    auto first=std::make_shared<int>(11),second=std::make_shared<int>(22);
    const auto type=context.constructor(u"identity");
    const auto firstObject=bindings.instance(first,type);
    const auto secondObject=bindings.instance(second,type);
    expect(bindings.findPointee(first.get())==firstObject);
    expect(bindings.findPointee(second.get())==secondObject);
    expect(!bindings.findPointee(static_cast<int*>(nullptr)).has_value());
    std::size_t visited=0;
    bindings.visitCanonicalHandles([&](const auto& owner,opts::ObjectRef handle){
        expect((owner==first&&handle==firstObject)||(owner==second&&handle==secondObject));
        ++visited;
    });
    expect(visited==2u);
    expect(bindings.canonicalHandleCount()==2u);
};
"math_port_native_identity_erase_keeps_source_record_while_retire_tombstones_handle"_test=[] {
    opts::Context context;const auto type=context.constructor(u"identity");
    NativeIdentityBindings<std::shared_ptr<int>> bindings(context);
    auto erasedOwner=std::make_shared<int>(31);auto* erasedPointer=erasedOwner.get();
    std::weak_ptr<int> erasedWeak=erasedOwner;
    const auto erasedObject=bindings.instance(erasedOwner,type);
    context.define(erasedObject,opts::Key(u"marker"),opts::Descriptor::data(opts::Value(41)));
    erasedOwner.reset();
    expect(bindings.erase(bindings.resolve(erasedObject)));
    expect(!bindings.erase(std::make_shared<int>(999)));
    expect(!bindings.findPointee(erasedPointer).has_value());
    expect(bindings.canonicalHandleCount()==0u);
    expect(erasedWeak.expired());
    expect(context.get(opts::Value(erasedObject),opts::Key(u"marker"))==opts::Value(41));
    (void)context.kind(erasedObject); // erase removes the binding, not the Source object.

    auto retiredOwner=std::make_shared<int>(51);auto* retiredPointer=retiredOwner.get();
    std::weak_ptr<int> retiredWeak=retiredOwner;
    auto retiredIdentity=retiredOwner;
    const auto retiredObject=bindings.instance(retiredIdentity,type);
    retiredOwner.reset();retiredIdentity.reset();
    auto canonical=bindings.resolve(retiredObject);
    expect(bindings.retire(canonical));
    canonical.reset();
    expect(retiredWeak.expired());
    expect(!bindings.findPointee(retiredPointer).has_value());
    expect(bindings.canonicalHandleCount()==0u);
    expect(throws<opts::Exception>([&]{(void)context.kind(retiredObject);}));
    expect(throws<opts::Exception>([&]{(void)context.get(opts::Value(retiredObject),opts::Key(u"marker"));}));
};
"math_port_native_identity_resolution_failure_is_derived_and_keeps_base_diagnostic"_test=[] {
    opts::Context context;NativeIdentityBindings<std::shared_ptr<int>> bindings(context);
    bool caughtDerived=false;
    try{(void)bindings.resolve(context.object());}
    catch(const NativeIdentityResolutionFailure& error){
        caughtDerived=true;
        expect(error.name=="Unimplemented");
        expect(error.message==u"Context object has no binding for this native reference type");
        expect(!error.requestedType.empty());
        expect(error.payloadType=="<empty>");
    }
    expect(caughtDerived);

    const auto wrongPayload=context.object();
    context.setPayload(wrongPayload,std::string("unrelated payload"));
    bool caughtBase=false;
    try{(void)bindings.resolve(wrongPayload);}
    catch(const opts::Exception& error){
        caughtBase=true;
        expect(error.name=="Unimplemented");
        expect(error.message==u"Context object has no binding for this native reference type");
    }
    expect(caughtBase);
};

"math_port_native_identity_nested_capture_tokens_retire_exact_owner_records"_test=[] {
    opts::Context context;NativeIdentityBindings<std::shared_ptr<int>> bindings(context);
    const auto type=context.constructor(u"identity");
    auto outerValue=std::make_shared<int>(10),innerValue=std::make_shared<int>(20),laterValue=std::make_shared<int>(30);
    const auto outer=bindings.beginRequestCapture();
    const auto outerObject=bindings.instance(outerValue,type);
    const auto inner=bindings.beginRequestCapture();
    const auto innerObject=bindings.instance(innerValue,type);
    bindings.retireCapturedRequestBindings(inner);
    expect(bindings.ownsHandle(outerObject));expect(!bindings.ownsHandle(innerObject));
    const auto laterObject=bindings.instance(laterValue,type);
    bindings.retireCapturedRequestBindings(outer);
    expect(!bindings.ownsHandle(outerObject));expect(!bindings.ownsHandle(laterObject));

    auto olderValue=std::make_shared<int>(31),newerValue=std::make_shared<int>(32),stillNewer=std::make_shared<int>(33);
    const auto older=bindings.beginRequestCapture();
    const auto olderObject=bindings.instance(olderValue,type);
    const auto newer=bindings.beginRequestCapture();
    const auto newerObject=bindings.instance(newerValue,type);
    bindings.retireCapturedRequestBindings(older);
    expect(!bindings.ownsHandle(olderObject));expect(bindings.ownsHandle(newerObject));
    const auto afterOlder=bindings.instance(stillNewer,type);
    bindings.retireCapturedRequestBindings(newer);
    expect(!bindings.ownsHandle(newerObject));expect(!bindings.ownsHandle(afterOlder));
};
"math_port_native_identity_failed_capture_retirement_keeps_same_token_retryable"_test=[] {
    opts::Context context;NativeIdentityBindings<std::shared_ptr<int>> bindings(context);
    const auto type=context.constructor(u"identity");auto value=std::make_shared<int>(40);
    const auto token=bindings.beginRequestCapture();const auto object=bindings.instance(value,type);
    const auto original=std::any_cast<NativeBindingPayload<std::shared_ptr<int>>>(context.payload(object));
    context.setPayload(object,std::string("wrong payload"));
    expect(throws<opts::Exception>([&]{bindings.retireCapturedRequestBindings(token);}));
    context.setPayload(object,original);
    bindings.retireCapturedRequestBindings(token);
    expect(!bindings.ownsHandle(object));
};
};}
