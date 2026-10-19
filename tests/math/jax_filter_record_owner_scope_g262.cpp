#include "support/folia_test.hpp"
import folia.math.core.input_jax_host;
import folia.math.core.output_jax;
import folia.math.output.common;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts=folia::math::options;
namespace core=folia::math::core;
namespace svg=folia::math::output::svg;
static_assert(std::is_move_constructible_v<core::JaxFilterRecordOwnerScope::Token>);
static_assert(!std::is_move_assignable_v<core::JaxFilterRecordOwnerScope::Token>);

bool retired(opts::Context& context,opts::ObjectRef object){
    try{(void)context.kind(object);return false;}
    catch(const opts::Exception&){return true;}
}
struct Document final:NativeObject,core::MathDocumentService {
    opts::Context& context;
    opts::ObjectRef identity;
    explicit Document(opts::Context& owner):context(owner),identity(owner.object()){}
    opts::Context& sourceContext()const override{return context;}
    opts::ObjectRef sourceIdentity()const override{return identity;}
    core::MathRenderActionsReference renderActionsReference()const override{return Missing{};}
    core::MathOutputJaxReference outputJaxReference()const override{return Missing{};}
};
struct InputProbe final:core::AbstractInputJax {
    InputProbe(OptionFunctionBindings& bindings,opts::ObjectRef identity)
        :AbstractInputJax(bindings,identity){}
    mml::NodeValue compile(core::MathItem&,core::MathDocumentReference)override{return PropertyNull{};}
    using AbstractInputJax::executeFilters;
};
struct OutputProbe final:core::AbstractOutputJax {
    OutputProbe(OptionFunctionBindings& bindings,opts::ObjectRef identity,
        FunctionListContextBindings& filters,std::shared_ptr<core::JaxFilterRecordOwnerScope> owner)
        :AbstractOutputJax(bindings,identity,filters,std::move(owner)){}
    core::MathOutput typeset(core::MathItem&,core::MathDocumentReference)override{return PropertyNull{};}
    core::MathOutput escaped(core::MathItem&,core::MathDocumentReference)override{return PropertyNull{};}
    using AbstractOutputJax::executeFilters;
};
folia::math::output::common::FontLoadHost fontHost(){
    folia::math::output::common::FontLoadHost host;
    host.asyncIsSynchronous=[]{return true;};
    host.warn=[](const std::any&){};
    return host;
}
}

suite jax_filter_record_owner_scope_g262=[] {
    "partial_filter_binding_retires_only_its_fresh_outer_record"_test=[] {
        opts::Context context;
        const auto before=context.recordCountsForDiagnostics();
        auto owner=std::make_shared<NativeObject>();
        const auto valid=[](std::u16string name)->core::JaxFilterRecordField{
            return {opts::Key(std::move(name)),[]{return opts::Value(opts::Undefined{});},
                [](opts::Value){},[]{}};
        };
        std::array<core::JaxFilterRecordField,3> fields{{
            valid(u"math"),core::JaxFilterRecordField{opts::Key(u"document"),{},
                [](opts::Value){},[]{}},valid(u"data")}};
        bool failed=false;
        try{(void)core::bindJaxFilterRecord(context,owner,{},fields);}
        catch(const opts::Exception&){failed=true;}
        const auto after=context.recordCountsForDiagnostics();
        expect(failed);
        expect(after.slots==before.slots+1&&after.live==before.live&&
            after.retired==before.retired+1&&after.ownerBearing==before.ownerBearing);
    };
    "input_filter_records_follow_their_document_tokens"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        core::InputJaxModule module(context);
        InputProbe jax(functions,context.object());
        jax.bindFilterRecordValueCodec(
            [](const std::any& value){return std::any_cast<opts::Value>(value);},
            [](opts::Value value)->std::any{return value;},{},module.filterRecordOwner());
        Document a(context),b(context);
        const auto data=context.object();
        auto owner=module.filterRecordOwner();
        auto tokenA=owner->begin(a.identity),tokenB=owner->begin(b.identity);
        FunctionList filters;
        opts::ObjectRef recordA{},recordB{};
        auto callback=std::make_shared<Function>();
        callback->call=[&](const FunctionReceiver&,FunctionArgs args)->std::any{
            const auto record=std::any_cast<core::InputJaxFilterDataRef>(args[0]);
            const auto document=std::get<opts::ObjectRef>(record->field(u"document").data);
            if(document==a.identity)recordA=record->sourceIdentity();
            else recordB=record->sourceIdentity();
            expect(record->field(u"data")==opts::Value(data));
            return {};
        };
        filters.add(FunctionValue{FunctionRef(callback)},0);
        (void)jax.executeFilters(filters,Missing{},&a,std::any(opts::Value(data)));
        (void)jax.executeFilters(filters,Missing{},&b,std::any(opts::Value(data)));
        expect(!retired(context,recordA)&&!retired(context,recordB));
        owner->close(tokenB);
        expect(retired(context,recordB)&&!retired(context,recordA));
        owner->close(tokenA);
        expect(retired(context,recordA)&&!retired(context,data));
    };
    "output_native_and_source_filter_calls_share_exact_module_owner"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        core::OutputJaxModule module(context,functions);
        OutputProbe jax(functions,context.object(),module.filterBindings(),module.filterRecordOwner());
        Document a(context),b(context);
        const auto data=context.object();
        auto owner=module.filterRecordOwner();
        auto tokenA=owner->begin(a.identity),tokenB=owner->begin(b.identity);
        FunctionList nativeFilters;
        opts::ObjectRef nativeRecord{};
        auto nativeCallback=std::make_shared<Function>();
        nativeCallback->call=[&](const FunctionReceiver&,FunctionArgs args)->std::any{
            nativeRecord=std::any_cast<core::OutputJaxFilterDataRef>(args[0])->sourceIdentity();
            return {};
        };
        nativeFilters.add(FunctionValue{FunctionRef(nativeCallback)},0);
        (void)jax.executeFilters(nativeFilters,Missing{},&a,std::any(opts::Value(data)));
        opts::ObjectRef sourceRecord{};
        const auto filters=context.object();
        const auto execute=context.function(u"execute",[&](opts::Context&,opts::Value,
            std::span<const opts::Value> args)->opts::Value{
            sourceRecord=std::get<opts::ObjectRef>(args[0].data);
            return opts::Undefined{};
        });
        context.set(filters,opts::Key(u"execute"),opts::Value(execute));
        const auto prototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(module.abstractClass()),opts::Key(u"prototype")).data);
        const auto method=context.get(opts::Value(prototype),opts::Key(u"executeFilters"));
        const std::array<opts::Value,4> args{opts::Value(filters),opts::Undefined{},
            opts::Value(b.identity),opts::Value(data)};
        (void)context.callMember(method,opts::Value(prototype),args,u"executeFilters");
        expect(!retired(context,nativeRecord)&&!retired(context,sourceRecord));
        owner->close(tokenB);
        expect(retired(context,sourceRecord)&&!retired(context,nativeRecord));
        owner->close(tokenA);
        expect(retired(context,nativeRecord)&&!retired(context,data));
        // An ordinary public call with no open request token remains Source-visible.
        (void)context.callMember(method,opts::Value(prototype),args,u"executeFilters");
        expect(!retired(context,sourceRecord));
    };
    "failed_close_keeps_its_token_and_cannot_admit_another_begin"_test=[] {
        opts::Context context;
        core::JaxFilterRecordOwnerScope owner(context),other(context);
        const auto a=context.object(),b=context.object(),c=context.object();
        auto tokenA=owner.begin(a),tokenB=owner.begin(b);
        const auto stale=context.object(),progress=context.object(),bRecord=context.object();
        owner.capture(a,stale);owner.capture(a,progress);owner.capture(b,bRecord);
        context.retireObjectRecord(stale);
        bool failed=false;
        try{owner.close(tokenA);}catch(const opts::Exception&){failed=true;}
        expect(failed&&retired(context,progress)&&!retired(context,bRecord));
        bool blocked=false;
        try{(void)owner.begin(c);}catch(const opts::Exception&){blocked=true;}
        expect(blocked);
        const auto late=context.object();
        blocked=false;
        try{owner.capture(a,late);}catch(const opts::Exception&){blocked=true;}
        expect(blocked);
        context.retireObjectRecord(late);
        owner.close(tokenB);
        expect(retired(context,bRecord));
        failed=false;
        try{owner.close(tokenB);}catch(const opts::Exception&){failed=true;}
        expect(failed);
        failed=false;
        try{owner.close(tokenA);}catch(const opts::Exception&){failed=true;}
        expect(failed);
        auto foreign=other.begin(c);
        failed=false;
        try{owner.close(foreign);}catch(const opts::Exception&){failed=true;}
        expect(failed);
        other.close(foreign);
    };
    "html_begin_rollback_cannot_close_a_shared_jax_document_token"_test=[] {
        svg::NativeRenderSession session(fontHost());
        auto a=svg::createNativeSvgDocument(session);
        auto b=svg::createNativeSvgDocument(session);
        expect(&a->outputJax()==&b->outputJax());
        auto& context=session.context();
        a->beginRequestOwnerScope();
        const auto data=context.object();
        opts::ObjectRef record{};
        const auto filters=context.object();
        const auto execute=context.function(u"execute",[&](opts::Context&,opts::Value,
            std::span<const opts::Value> args)->opts::Value{
            record=std::get<opts::ObjectRef>(args[0].data);return opts::Undefined{};
        });
        context.set(filters,opts::Key(u"execute"),opts::Value(execute));
        const auto jax=session.outputJax();
        const auto method=context.get(opts::Value(jax->instanceIdentity()),opts::Key(u"executeFilters"));
        const std::array<opts::Value,4> args{opts::Value(filters),opts::Undefined{},
            opts::Value(a->sourceIdentity()),opts::Value(data)};
        (void)context.callMember(method,opts::Value(jax->instanceIdentity()),args,u"executeFilters");
        expect(!retired(context,record));
        // A pending close in the later output tracker forces B's transactional
        // begin to roll back its already-opened input token.
        auto outputOwner=jax->filterRecordOwner();
        const auto pendingDocument=context.object(),stale=context.object();
        auto pending=outputOwner->begin(pendingDocument);
        outputOwner->capture(pendingDocument,stale);
        context.retireObjectRecord(stale);
        bool pendingFailed=false;
        try{outputOwner->close(pending);}catch(const opts::Exception&){pendingFailed=true;}
        expect(pendingFailed);
        bool beginFailed=false;
        try{b->beginRequestOwnerScope();}catch(const opts::Exception&){beginFailed=true;}
        expect(beginFailed);
        auto inputOwner=session.services().parser().inputJaxModule().filterRecordOwner();
        auto retryB=inputOwner->begin(b->sourceIdentity());
        inputOwner->close(retryB);
        b->retireCompletedRequestItem();
        expect(!retired(context,record));
        a->retireCompletedRequestItem();
        expect(retired(context,record)&&!retired(context,data));
    };
};
