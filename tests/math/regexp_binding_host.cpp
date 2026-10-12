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
        bindings.clear();
        expect(weak.expired());
        expect(eq(failure(object),std::string("Unimplemented")));
    };
};}
