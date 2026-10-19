#include "../../tests/support/folia_test.hpp"
import folia.math.output.svg.render_session;
import folia.math.util.promise;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::options;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

common::FontLoadHost host(){
    common::FontLoadHost result;
    result.asyncIsSynchronous=[] {return true;};
    result.warn=[](const std::any&){};
    return result;
}

suite extensionDelegation=[] {
    "common_jax_extension_observes_live_font_method_and_exact_result"_test=[] {
        svg::NativeRenderSession session(host());
        auto& cx=session.context();
        const auto prototype=cx.get(session.services().commonConstructor(),Key(u"prototype"));
        const auto method=std::get<ObjectRef>(cx.get(prototype,Key(u"addExtension")).data);
        const auto descriptor=cx.descriptor(std::get<ObjectRef>(prototype.data),Key(u"addExtension"));
        expect(descriptor.has_value());
        expect(descriptor->writable&&descriptor->enumerable&&descriptor->configurable);
        expect(cx.get(method,Key(u"name"))==Value(u""));
        expect(cx.get(method,Key(u"length"))==Value(2));
        const auto receiver=cx.object(),font=cx.object(),data=cx.object(),result=cx.object();
        std::vector<std::u16string> order;
        const auto extension=cx.function(u"replacement",[&](Context&,Value self,std::span<const Value> args)->Value{
            order.push_back(u"call");
            expect(self==Value(font));
            expect(args.size()==2_u);
            expect(args[0]==Value(data));
            expect(args[1]==Value(u""));
            return result;
        });
        const auto fontGetter=cx.function(u"get font",[&](Context&,Value self,std::span<const Value>)->Value{
            expect(self==Value(receiver));order.push_back(u"font");return font;
        });
        const auto methodGetter=cx.function(u"get addExtension",[&](Context&,Value self,std::span<const Value>)->Value{
            expect(self==Value(font));order.push_back(u"method");return extension;
        });
        cx.define(receiver,Key(u"font"),Descriptor::access(fontGetter,{},true,true));
        cx.define(font,Key(u"addExtension"),Descriptor::access(methodGetter,{},true,true));
        const std::array<Value,1> args{data};
        expect(cx.call(method,receiver,args)==Value(result));
        expect(order==std::vector<std::u16string>{u"font",u"method",u"call"});
        const auto secondFont=cx.object();
        const auto secondResult=cx.symbol(u"returned");
        const auto second=cx.function(u"second",[&](Context&,Value self,std::span<const Value> forwarded)->Value{
            expect(self==Value(secondFont));expect(forwarded.size()==2_u);
            expect(forwarded[0]==Value(data));expect(forwarded[1]==Value(Null{}));return secondResult;
        });
        cx.define(receiver,Key(u"font"),Descriptor::data(Value(secondFont),true,true,true));
        cx.set(secondFont,Key(u"addExtension"),second);
        const std::array<Value,3> extra{data,Null{},u"ignored"};
        expect(cx.call(method,receiver,extra)==Value(secondResult));
    };

    "common_jax_extension_defaults_undefined_prefix_and_preserves_raw_values"_test=[] {
        svg::NativeRenderSession session(host());
        auto& cx=session.context();
        const auto prototype=cx.get(session.services().commonConstructor(),Key(u"prototype"));
        const auto method=std::get<ObjectRef>(cx.get(prototype,Key(u"addExtension")).data);
        const auto receiver=cx.object(),font=cx.object(),rawPrefix=cx.object();
        cx.set(receiver,Key(u"font"),font);
        std::vector<std::vector<Value>> calls;
        const auto callback=cx.function(u"capture",[&](Context&,Value self,std::span<const Value> args)->Value{
            expect(self==Value(font));calls.emplace_back(args.begin(),args.end());return args[1];
        });
        cx.set(font,Key(u"addExtension"),callback);
        const auto throwingCallGetter=cx.function(u"get call",[](Context&,Value,std::span<const Value>)->Value{
            throw Exception("Error",u"A direct Source invocation must not read function.call");
        });
        cx.define(callback,Key(u"call"),Descriptor::access(throwingCallGetter,{},true,true));
        expect(cx.call(method,receiver)==Value(u""));
        expect(calls.back()==std::vector<Value>{Undefined{},u""});
        const std::array<Value,2> omitted{Undefined{},Undefined{}};
        expect(cx.call(method,receiver,omitted)==Value(u""));
        const std::array<Value,2> objectPrefix{u"data",rawPrefix};
        expect(cx.call(method,receiver,objectPrefix)==Value(rawPrefix));
        expect(calls.back()==std::vector<Value>{u"data",rawPrefix});
        const auto thrown=cx.object();
        const auto throwing=cx.function(u"throwing",[&](Context&,Value,std::span<const Value>)->Value{
            throw async::ThrownValue{Value(thrown)};
        });
        cx.set(font,Key(u"addExtension"),throwing);
        bool exact=false;
        try{(void)cx.call(method,receiver);}catch(const async::ThrownValue& error){exact=std::any_cast<Value>(error.value)==Value(thrown);}
        expect(exact);
        const std::array<std::pair<Value,std::u16string_view>,2> nullFonts{{
            {Null{},u"Cannot read properties of null (reading 'addExtension')"},
            {Undefined{},u"Cannot read properties of undefined (reading 'addExtension')"}}};
        for(const auto& [missing,message]:nullFonts){
            cx.set(receiver,Key(u"font"),missing);
            bool sameError=false;
            try{(void)cx.call(method,receiver);}catch(const Exception& error){
                sameError=error.name=="TypeError"&&error.message==message;
            }
            expect(sameError);
        }
        cx.set(receiver,Key(u"font"),font);
        cx.set(font,Key(u"addExtension"),7);
        bool nonCallable=false;
        try{(void)cx.call(method,receiver);}catch(const Exception& error){
            nonCallable=error.name=="TypeError"&&error.message==u"this.font.addExtension is not a function";
        }
        expect(nonCallable);
    };
};
}
