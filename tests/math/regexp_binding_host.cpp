#include "support/folia_test.hpp"
import folia.math.util.regexp_binding;
namespace {using namespace boost::ut;namespace host=folia::math::regexp_host;namespace opts=folia::math::options;using folia::math::NativeBindingPayload;
suite binding_host=[] {
    "math_port_regexp_binding_payload_retains_actual_owner"_test=[] {
        std::weak_ptr<host::RegExp> weak;
        {
            opts::Context context;opts::ObjectRef object;
            {
                host::RegExpBindings bindings(context);
                auto pattern=std::make_shared<host::RegExp>(u"x",u"g");weak=pattern;
                object=bindings.bind(pattern);pattern.reset();
                expect(!weak.expired());
            }
            expect(!weak.expired());
            auto* payload=std::any_cast<NativeBindingPayload<host::RegExpRef>>(&context.payload(object));
            expect(payload!=nullptr);
            payload->identity->lastIndex=1;
            auto match=payload->identity->exec(u"xx");
            expect(match&&match->index==1);
        }
        expect(weak.expired());
    };
    "math_port_regexp_binding_teardown_and_rejections"_test=[] {
        opts::Context context,foreign;host::RegExpBindings bindings(context);
        auto pattern=std::make_shared<host::RegExp>(u"x",u"g");std::weak_ptr<host::RegExp> weak=pattern;
        auto object=bindings.bind(pattern);pattern.reset();
        auto failure=[&](opts::Value value){try{bindings.resolve(value);return std::string("ok");}catch(const opts::Exception&error){return error.name;}};
        expect(eq(failure(foreign.object()),std::string("TypeError")));
        expect(eq(failure(opts::Null{}),std::string("TypeError")));
        expect(eq(failure(context.object()),std::string("Unimplemented")));
        const auto constructor=bindings.constructorIdentity();
        const auto prototype=std::get<opts::ObjectRef>(context.get(opts::Value(constructor),
            opts::Key(u"prototype")).data);
        const auto exec=std::get<opts::ObjectRef>(context.get(opts::Value(prototype),
            opts::Key(u"exec")).data);
        const auto unbound=context.objectWithPrototype(prototype);
        const std::array<opts::Value,1> execArguments{opts::Value(u"x")};
        bool sourceExecTypeError=false;
        try{(void)context.call(exec,opts::Value(unbound),execArguments);}
        catch(const opts::Exception&error){sourceExecTypeError=error.name=="TypeError";}
        expect(sourceExecTypeError);
        bool sourceFlagTypeError=false;
        try{(void)context.get(opts::Value(unbound),opts::Key(u"global"));}
        catch(const opts::Exception&error){sourceFlagTypeError=error.name=="TypeError";}
        expect(sourceFlagTypeError);
        bindings.clear();
        expect(weak.expired());
        bool staleFieldRejected=false;
        try{(void)context.get(opts::Value(object),opts::Key(u"lastIndex"));}
        catch(const opts::Exception&error){staleFieldRejected=error.name=="Unimplemented";}
        expect(staleFieldRejected);
        expect(eq(failure(object),std::string("Unimplemented")));
    };
    "math_port_regexp_source_exec_preserves_identity_error_from_lastindex_coercion"_test=[] {
        opts::Context context;host::RegExpBindings bindings(context);
        const auto regexp=bindings.create(u"x",u"g");
        const auto cursor=context.object();
        const auto valueOf=context.function(u"valueOf",
            [](opts::Context&,opts::Value,std::span<const opts::Value>)->opts::Value{
                throw folia::math::NativeIdentityResolutionFailure("cursor", "user callback");
            });
        context.define(cursor,opts::Key(u"valueOf"),opts::Descriptor::data(opts::Value(valueOf)));
        context.set(opts::Value(regexp),opts::Key(u"lastIndex"),opts::Value(cursor));
        const auto prototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(bindings.constructorIdentity()),opts::Key(u"prototype")).data);
        const auto exec=std::get<opts::ObjectRef>(context.get(opts::Value(prototype),opts::Key(u"exec")).data);
        const std::array<opts::Value,1> arguments{opts::Value(u"x")};
        bool preserved=false;
        try{(void)context.call(exec,opts::Value(regexp),arguments);}
        catch(const folia::math::NativeIdentityResolutionFailure& error){
            preserved=error.name=="Unimplemented";
        }
        expect(preserved);
        expect(context.get(opts::Value(regexp),opts::Key(u"lastIndex"))==opts::Value(cursor));
    };
    "math_port_regexp_binding_clear_disables_retained_context_field_without_releasing_external_owner"_test=[] {
        opts::Context context;host::RegExpBindings bindings(context);
        auto pattern=std::make_shared<host::RegExp>(u"x",u"g");
        std::weak_ptr<host::RegExp> weak=pattern;
        pattern->lastIndex=1;
        const auto object=bindings.bind(pattern);
        const auto alias=bindings.bind(pattern);
        const opts::Value receiver(object);
        expect(alias==object);
        bindings.clear();

        expect(!weak.expired());
        expect(pattern->lastIndex.value==opts::Value(1));
        bool readRejected=false;
        try{(void)context.get(receiver,opts::Key(u"lastIndex"));}
        catch(const opts::Exception&error){readRejected=error.name=="Unimplemented";}
        expect(readRejected);
        bool writeRejected=false;
        try{context.set(receiver,opts::Key(u"lastIndex"),opts::Value(2));}
        catch(const opts::Exception&error){writeRejected=error.name=="Unimplemented";}
        expect(writeRejected);
        expect(pattern->lastIndex.value==opts::Value(1));
        bool resolveRejected=false;
        try{(void)bindings.resolve(receiver);}
        catch(const opts::Exception&error){resolveRejected=error.name=="Unimplemented";}
        expect(resolveRejected);
    };
    "math_port_regexp_binding_last_owner_in_context_payload_destroys_without_context_reentry"_test=[] {
        std::weak_ptr<host::RegExpBindings> bindingsLifetime;
        std::weak_ptr<host::RegExp> regexpLifetime;
        {
            opts::Context context;
            auto bindings=std::make_shared<host::RegExpBindings>(context);
            const auto object=bindings->create(u"x",u"g");
            regexpLifetime=bindings->resolve(opts::Value(object));
            const auto anchor=context.object();
            bindingsLifetime=bindings;
            // Context is the final owner. Its records are destroyed in the
            // same teardown that releases this payload, so the binding dtor
            // must not call back into Context.
            context.setPayload(anchor,bindings);
            bindings.reset();
            expect(!bindingsLifetime.expired());
            expect(!regexpLifetime.expired());
        }
        expect(bindingsLifetime.expired());
        expect(regexpLifetime.expired());
    };
    "math_port_regexp_binding_rejects_expired_source_methods_while_context_lives"_test=[] {
        std::weak_ptr<host::RegExp> regexpLifetime;
        {
            opts::Context context;
            auto bindings=std::make_shared<host::RegExpBindings>(context);
            const auto object=bindings->create(u"x",u"g");
            regexpLifetime=bindings->resolve(opts::Value(object));
            const auto constructor=bindings->constructorIdentity();
            const auto prototype=std::get<opts::ObjectRef>(context.get(opts::Value(constructor),
                opts::Key(u"prototype")).data);
            const auto exec=std::get<opts::ObjectRef>(context.get(opts::Value(prototype),
                opts::Key(u"exec")).data);
            bindings.reset();

            bool fieldRejected=false;
            try{(void)context.get(opts::Value(object),opts::Key(u"lastIndex"));}
            catch(const opts::Exception& error){fieldRejected=error.name=="Unimplemented";}
            expect(fieldRejected);
            bool constructorRejected=false;
            try{(void)context.construct(constructor,std::span<const opts::Value>{});}
            catch(const opts::Exception& error){constructorRejected=error.name=="TypeError";}
            expect(constructorRejected);
            const std::array<opts::Value,1> args{opts::Value(u"x")};
            bool methodRejected=false;
            try{(void)context.call(exec,opts::Value(object),args);}
            catch(const opts::Exception& error){methodRejected=error.name=="TypeError";}
            expect(methodRejected);
            bool flagRejected=false;
            try{(void)context.get(opts::Value(object),opts::Key(u"global"));}
            catch(const opts::Exception& error){flagRejected=error.name=="TypeError";}
            expect(flagRejected);
            expect(!regexpLifetime.expired());
        }
        expect(regexpLifetime.expired());
    };
    "math_port_regexp_binding_owner_really_outlives_context"_test=[] {
        std::shared_ptr<host::RegExpBindings> bindings;
        std::weak_ptr<host::RegExp> regexpLifetime;
        {
            opts::Context context;
            bindings=std::make_shared<host::RegExpBindings>(context);
            const auto object=bindings->create(u"x",u"g");
            regexpLifetime=bindings->resolve(opts::Value(object));
            expect(!regexpLifetime.expired());
        }
        expect(bindings!=nullptr);
        expect(!regexpLifetime.expired());
        bindings.reset();
        expect(regexpLifetime.expired());
    };
};}
