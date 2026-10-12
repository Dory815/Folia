// Candidate native comparisons for parser-iterator-source.json.
// Kept outside tests/ auto-globs until Root adds the iterator packet to a build.
#include "../../tests/support/folia_test.hpp"
import folia.math.util.options;
import folia.math.input.tex.parser;
namespace {
using namespace boost::ut;
namespace opts=folia::math::options;
suite parser_iterator_host=[] {
    "source_array_iterator_rechecks_live_length_and_never_revives"_test=[] {
        opts::Context context;
        const auto array=context.array(1);
        context.set(opts::Value(array),opts::Key(u"0"),opts::Value(u"a"));
        const opts::Value iterator=context.getIterator(opts::Value(array));
        const auto first=context.iteratorNext(iterator);
        expect(context.get(first,opts::Key(u"value"))==opts::Value(u"a"));
        context.set(opts::Value(array),opts::Key(u"1"),opts::Value(u"b"));
        const auto second=context.iteratorNext(iterator);
        expect(context.get(second,opts::Key(u"value"))==opts::Value(u"b"));
        const auto done=context.iteratorNext(iterator);
        expect(opts::truthy(context.get(done,opts::Key(u"done"))));
        context.set(opts::Value(array),opts::Key(u"2"),opts::Value(u"late"));
        const auto still_done=context.iteratorNext(iterator);
        expect(opts::truthy(context.get(still_done,opts::Key(u"done"))));
    };
    "source_string_iterator_yields_surrogate_pair_as_one_value"_test=[] {
        opts::Context context;
        const opts::Value text(u"A🙂B");
        const auto iterator=context.getIterator(text);
        const auto first=context.iteratorNext(iterator);
        const auto emoji=context.iteratorNext(iterator);
        const auto last=context.iteratorNext(iterator);
        expect(context.get(first,opts::Key(u"value"))==opts::Value(u"A"));
        expect(context.get(emoji,opts::Key(u"value"))==opts::Value(u"🙂"));
        expect(context.get(last,opts::Key(u"value"))==opts::Value(u"B"));
    };
    "source_values_arraylike_fallback_reads_dynamic_length_and_get"_test=[] {
        opts::Context context;
        const auto source=context.object();int length=1,length_reads=0,index_reads=0;
        const auto length_getter=context.function(u"length",[&](auto&,auto,auto)->opts::Value {
            ++length_reads;return opts::Value(length);
        });
        const auto index_getter=context.function(u"0",[&](auto&,auto,auto)->opts::Value {
            ++index_reads;length=2;return opts::Value(u"x");
        });
        context.define(source,opts::Key(u"length"),opts::Descriptor::access(length_getter,{},true,true));
        context.define(source,opts::Key(u"0"),opts::Descriptor::access(index_getter,{},true,true));
        context.define(source,opts::Key(u"1"),opts::Descriptor::data(opts::Value(u"y")));
        const auto iterator=context.getIterator(opts::Value(source));
        const auto first=context.iteratorNext(iterator);
        expect(context.get(first,opts::Key(u"value"))==opts::Value(u"x"));
        const auto second=context.iteratorNext(iterator);
        expect(context.get(second,opts::Key(u"value"))==opts::Value(u"y"));
        const auto done=context.iteratorNext(iterator);
        expect(opts::truthy(context.get(done,opts::Key(u"done"))));
        expect(length_reads==4_i);
        expect(index_reads==1_i);
    };
    "source_read_bounded_iterator_closes_after_requested_values"_test=[] {
        opts::Context context;
        const auto iterable=context.object(),iterator=context.object();
        int next_calls=0,close_calls=0;
        const auto next=context.function(u"next",[&](auto& ctx,auto,auto)->opts::Value {
            const int value=++next_calls;
            const auto result=ctx.object();
            ctx.define(result,opts::Key(u"value"),opts::Descriptor::data(opts::Value(value)));
            ctx.define(result,opts::Key(u"done"),opts::Descriptor::data(opts::Value(false)));
            return opts::Value(result);
        });
        const auto close=context.function(u"return",[&](auto&,auto,auto)->opts::Value {
            ++close_calls;return opts::Value(context.object());
        });
        context.define(iterator,opts::Key(u"next"),opts::Descriptor::data(opts::Value(next)));
        context.define(iterator,opts::Key(u"return"),opts::Descriptor::data(opts::Value(close)));
        const auto method=context.function(u"iterator",[iterator](auto&,auto,auto){return opts::Value(iterator);});
        context.define(iterable,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::data(opts::Value(method)));
        const auto values=context.readIterable(opts::Value(iterable),2);
        const auto* array=std::get_if<opts::ObjectRef>(&values.data);
        expect(array!=nullptr);
        expect(context.arrayLength(*array)==2_u);
        expect(context.get(values,opts::Key(u"0"))==opts::Value(1));
        expect(context.get(values,opts::Key(u"1"))==opts::Value(2));
        expect(next_calls==2_i);
        expect(close_calls==1_i);
    };
    "source_read_without_iterator_returns_original_identity"_test=[] {
        opts::Context context;
        const auto tuple=context.object();
        context.define(tuple,opts::Key(u"0"),opts::Descriptor::data(opts::Value(u"p")));
        expect(context.readIterable(opts::Value(tuple),2)==opts::Value(tuple));
    };
    "source_values_resolves_current_symbol_iterator_method"_test=[] {
        opts::Context context;
        const auto array=context.array(0),iterator=context.object();
        int calls=0;
        const auto next=context.function(u"next",[](auto& ctx,auto,auto)->opts::Value {
            const auto result=ctx.object();
            ctx.define(result,opts::Key(u"done"),opts::Descriptor::data(opts::Value(true)));
            return opts::Value(result);
        });
        context.define(iterator,opts::Key(u"next"),opts::Descriptor::data(opts::Value(next)));
        const auto method=context.function(u"iterator",[&](auto&,opts::Value receiver,auto)->opts::Value {
            expect(receiver==opts::Value(array));++calls;return opts::Value(iterator);
        });
        context.define(array,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::data(opts::Value(method)));
        const auto resolved=context.getIterator(opts::Value(array));
        expect(resolved==opts::Value(iterator));
        expect(calls==1_i);
    };
    "source_helper_uses_current_function_call_property"_test=[] {
        opts::Context context;
        const auto iterable=context.array(0),iterator=context.object();int calls=0;
        const auto method=context.function(u"iterator",[](auto&,auto,auto)->opts::Value {
            throw opts::Exception("Error",u"direct function call bypassed own call");
        });
        const auto customCall=context.function(u"call",[&](auto&,opts::Value receiver,std::span<const opts::Value> args)->opts::Value {
            expect(receiver==opts::Value(method));
            expect(args.size()==1_u);
            expect(args.front()==opts::Value(iterable));
            ++calls;return opts::Value(iterator);
        });
        context.define(method,opts::Key(u"call"),opts::Descriptor::data(opts::Value(customCall)));
        context.define(iterable,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::data(opts::Value(method)));
        expect(context.getIterator(opts::Value(iterable))==opts::Value(iterator));
        expect(calls==1_i);
    };
    "source_helper_accepts_object_with_callable_call_property"_test=[] {
        opts::Context context;
        const auto iterable=context.object(),iterator=context.object(),method=context.object();int calls=0;
        const auto customCall=context.function(u"call",[&](auto&,opts::Value receiver,std::span<const opts::Value> args)->opts::Value {
            expect(receiver==opts::Value(method));expect(args.size()==1_u);
            expect(args.front()==opts::Value(iterable));++calls;return opts::Value(iterator);
        });
        context.define(method,opts::Key(u"call"),opts::Descriptor::data(opts::Value(customCall)));
        context.define(iterable,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::data(opts::Value(method)));
        expect(context.getIterator(opts::Value(iterable))==opts::Value(iterator));
        expect(calls==1_i);
    };
    "source_helper_method_call_error"_test=[] {
        opts::Context context;
        const auto iterable=context.object();
        const auto method=context.function(u"iterator",[](auto&,auto,auto){return opts::Value(opts::Undefined{});});
        context.define(method,opts::Key(u"call"),opts::Descriptor::data(opts::Value(opts::Undefined{})));
        context.define(iterable,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::data(opts::Value(method)));
        std::string outcome="missing throw";
        try{(void)context.getIterator(opts::Value(iterable));}
        catch(const opts::Exception& error){outcome=error.name+"|"+std::string(error.message.begin(),error.message.end());}
        expect(outcome=="TypeError|m.call is not a function");
    };
    "source_values_symbol_getter_precedes_length_fallback"_test=[] {
        opts::Context context;
        const auto source=context.object();int symbol_reads=0,length_reads=0;
        const auto symbol_getter=context.function(u"iterator",[&](auto&,auto,auto)->opts::Value {
            ++symbol_reads;return opts::Value(opts::Undefined{});
        });
        const auto length_getter=context.function(u"length",[&](auto&,auto,auto)->opts::Value {
            ++length_reads;return opts::Value(1);
        });
        context.define(source,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::access(symbol_getter,{},true,true));
        context.define(source,opts::Key(u"length"),opts::Descriptor::access(length_getter,{},true,true));
        (void)context.getIterator(opts::Value(source));
        expect(symbol_reads==1_i);
        expect(length_reads==1_i);
    };
    "source_values_null_fails_during_symbol_lookup"_test=[] {
        opts::Context context;std::string outcome="missing throw";
        try{(void)context.getIterator(opts::Value(opts::Null{}));}
        catch(const opts::Exception& error){outcome=error.name+"|"+std::string(error.message.begin(),error.message.end());}
        expect(outcome=="TypeError|object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    };
    "source_read_iterator_error_precedes_close_error"_test=[] {
        opts::Context context;
        const auto iterable=context.object(),iterator=context.object();
        const auto next=context.function(u"next",[](auto&,auto,auto)->opts::Value {
            throw opts::Exception("Error",u"next-fault");
        });
        const auto close=context.function(u"return",[](auto&,auto,auto)->opts::Value {
            throw opts::Exception("Error",u"close-fault");
        });
        context.define(iterator,opts::Key(u"next"),opts::Descriptor::data(opts::Value(next)));
        context.define(iterator,opts::Key(u"return"),opts::Descriptor::data(opts::Value(close)));
        const auto method=context.function(u"iterator",[iterator](auto&,auto,auto){return opts::Value(iterator);});
        context.define(iterable,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::data(opts::Value(method)));
        std::string outcome="missing throw";
        try { (void)context.readIterable(opts::Value(iterable),2); }
        catch(const opts::Exception& error) { outcome=error.name+"|"+std::string(error.message.begin(),error.message.end()); }
        expect(outcome=="Error|next-fault");
    };
    "source_read_iterator_error_after_yield_precedes_close_error"_test=[] {
        opts::Context context;
        const auto iterable=context.object(),iterator=context.object();
        std::vector<std::string> trace;int nextCalls=0;
        const auto next=context.function(u"next",[&](auto& ctx,auto,auto)->opts::Value {
            trace.push_back("next"+std::to_string(++nextCalls));
            if(nextCalls==1){
                const auto result=ctx.object();
                ctx.define(result,opts::Key(u"value"),opts::Descriptor::data(opts::Value(u"proc")));
                ctx.define(result,opts::Key(u"done"),opts::Descriptor::data(opts::Value(false)));
                return opts::Value(result);
            }
            throw opts::Exception("Error",u"next-fault");
        });
        const auto close=context.function(u"return",[&](auto&,auto,auto)->opts::Value {
            trace.push_back("return");throw opts::Exception("Error",u"close-fault");
        });
        context.define(iterator,opts::Key(u"next"),opts::Descriptor::data(opts::Value(next)));
        context.define(iterator,opts::Key(u"return"),opts::Descriptor::data(opts::Value(close)));
        const auto method=context.function(u"iterator",[iterator](auto&,auto,auto){return opts::Value(iterator);});
        context.define(iterable,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::data(opts::Value(method)));
        std::string outcome="missing throw";
        try{(void)context.readIterable(opts::Value(iterable),2);}
        catch(const opts::Exception& error){outcome=error.name+"|"+std::string(error.message.begin(),error.message.end());}
        expect(outcome=="Error|next-fault");
        expect(trace==std::vector<std::string>{"next1","next2","return"});
    };
    "source_read_close_error_propagates_without_prior_error"_test=[] {
        opts::Context context;
        const auto iterable=context.object(),iterator=context.object();
        const auto next=context.function(u"next",[](auto& ctx,auto,auto)->opts::Value {
            const auto result=ctx.object();
            ctx.define(result,opts::Key(u"done"),opts::Descriptor::data(opts::Value(false)));
            ctx.define(result,opts::Key(u"value"),opts::Descriptor::data(opts::Value(u"x")));
            return opts::Value(result);
        });
        const auto close=context.function(u"return",[](auto&,auto,auto)->opts::Value {
            throw opts::Exception("Error",u"close-fault");
        });
        context.define(iterator,opts::Key(u"next"),opts::Descriptor::data(opts::Value(next)));
        context.define(iterator,opts::Key(u"return"),opts::Descriptor::data(opts::Value(close)));
        const auto method=context.function(u"iterator",[iterator](auto&,auto,auto){return opts::Value(iterator);});
        context.define(iterable,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::data(opts::Value(method)));
        std::string outcome="missing throw";
        try{(void)context.readIterable(opts::Value(iterable),1);}
        catch(const opts::Exception& error){outcome=error.name+"|"+std::string(error.message.begin(),error.message.end());}
        expect(outcome=="Error|close-fault");
    };
    "source_for_of_body_error_closes_current_iterator"_test=[] {
        opts::Context context;
        const auto iterable=context.object(),iterator=context.object();
        int next_calls=0,close_calls=0;
        const auto next=context.function(u"next",[&](auto& ctx,auto,auto)->opts::Value {
            ++next_calls;const auto result=ctx.object();
            ctx.define(result,opts::Key(u"value"),opts::Descriptor::data(opts::Value(u"item")));
            ctx.define(result,opts::Key(u"done"),opts::Descriptor::data(opts::Value(false)));
            return opts::Value(result);
        });
        const auto close=context.function(u"return",[&](auto&,auto,auto)->opts::Value {
            ++close_calls;return opts::Value(context.object());
        });
        context.define(iterator,opts::Key(u"next"),opts::Descriptor::data(opts::Value(next)));
        context.define(iterator,opts::Key(u"return"),opts::Descriptor::data(opts::Value(close)));
        const auto method=context.function(u"iterator",[iterator](auto&,auto,auto){return opts::Value(iterator);});
        context.define(iterable,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::data(opts::Value(method)));
        std::string outcome="missing throw";
        try { context.forEachIterable(opts::Value(iterable),[](opts::Value){throw opts::Exception("Error",u"body-fault");}); }
        catch(const opts::Exception& error) { outcome=error.name+"|"+std::string(error.message.begin(),error.message.end()); }
        expect(outcome=="Error|body-fault");
        expect(next_calls==1_i);
        expect(close_calls==1_i);
    };
    "source_for_of_next_error_closes_previous_step"_test=[] {
        opts::Context context;
        const auto iterable=context.object(),iterator=context.object();
        int next_calls=0,close_calls=0;
        const auto next=context.function(u"next",[&](auto& ctx,auto,auto)->opts::Value {
            if(++next_calls>1)throw opts::Exception("Error",u"next-fault");
            const auto result=ctx.object();
            ctx.define(result,opts::Key(u"value"),opts::Descriptor::data(opts::Value(u"item")));
            ctx.define(result,opts::Key(u"done"),opts::Descriptor::data(opts::Value(false)));
            return opts::Value(result);
        });
        const auto close=context.function(u"return",[&](auto&,auto,auto)->opts::Value {
            ++close_calls;return opts::Value(context.object());
        });
        context.define(iterator,opts::Key(u"next"),opts::Descriptor::data(opts::Value(next)));
        context.define(iterator,opts::Key(u"return"),opts::Descriptor::data(opts::Value(close)));
        const auto method=context.function(u"iterator",[iterator](auto&,auto,auto){return opts::Value(iterator);});
        context.define(iterable,opts::Key(context.intrinsics().symbolIterator),opts::Descriptor::data(opts::Value(method)));
        std::string outcome="missing throw";
        try { context.forEachIterable(opts::Value(iterable),[](opts::Value){}); }
        catch(const opts::Exception& error) { outcome=error.name+"|"+std::string(error.message.begin(),error.message.end()); }
        expect(outcome=="Error|next-fault");
        expect(next_calls==2_i);
        expect(close_calls==1_i);
    };
    "source_function_prototype_call_metadata"_test=[] {
        opts::Context context;
        const auto call=context.get(opts::Value(context.intrinsics().functionPrototype),opts::Key(u"call"));
        const auto* function=std::get_if<opts::ObjectRef>(&call.data);
        expect(function!=nullptr);
        const auto descriptor=context.descriptor(*function,opts::Key(u"length"));
        expect(descriptor.has_value());
        expect(descriptor->value==opts::Value(1));
        const auto inherited=context.descriptor(context.intrinsics().functionPrototype,opts::Key(u"call"));
        expect(inherited.has_value());
        expect(!inherited->enumerable && inherited->writable && inherited->configurable);
    };
    "source_helper_gets_inherited_function_call"_test=[] {
        opts::Context context;
        const auto iterable=context.object(),method=context.function(u"iterator");
        int gets=0,calls=0;
        const auto getter=context.function(u"get call",[&](auto&,opts::Value receiver,auto)->opts::Value {
            expect(receiver==opts::Value(method));++gets;
            return opts::Value(context.function(u"call getter result",[&](auto&,opts::Value thisValue,
                    std::span<const opts::Value> args)->opts::Value {
                expect(thisValue==opts::Value(method));expect(args.size()==1_u);
                expect(args.front()==opts::Value(iterable));++calls;return opts::Value(u"returned");
            }));
        });
        context.define(context.intrinsics().functionPrototype,opts::Key(u"call"),
                       opts::Descriptor::access(getter,{},false,true));
        expect(context.callMember(opts::Value(method),opts::Value(iterable))==opts::Value(u"returned"));
        expect(gets==1_i);expect(calls==1_i);
    };
    "source_helper_does_not_bypass_deleted_call"_test=[] {
        opts::Context context;
        const auto iterable=context.object();
        const auto method=context.function(u"iterator",[](auto&,auto,auto){return opts::Value(opts::Undefined{});});
        context.erase(context.intrinsics().functionPrototype,opts::Key(u"call"));
        std::string outcome="missing throw";
        try{(void)context.callMember(opts::Value(method),opts::Value(iterable),{},u"m.call");}
        catch(const opts::Exception& error){outcome=error.name+"|"+std::string(error.message.begin(),error.message.end());}
        expect(outcome=="TypeError|m.call is not a function");
    };
    "source_arraylike_helper_iterator_is_plain_own_next"_test=[] {
        opts::Context context;
        const auto source=context.object();context.define(source,opts::Key(u"length"),opts::Descriptor::data(opts::Value(0)));
        const auto iterator=context.getIterator(opts::Value(source));
        const auto* object=std::get_if<opts::ObjectRef>(&iterator.data);expect(object!=nullptr);
        expect(context.prototype(*object)==context.intrinsics().objectPrototype);
        expect(context.has(iterator,opts::Key(context.intrinsics().symbolIterator))==false);
        const auto next=context.descriptor(iterator,opts::Key(u"next"));
        expect(next.has_value());expect(next->enumerable);
    };
    "source_configuration_handler_keys_is_live"_test=[] {
        opts::Context context;
        folia::math::tex::ConfigurationHandler handler;
        folia::math::NativeIdentityBindings<folia::math::tex::ConfigurationRef> identities(context);
        const auto object=handler.createContextObject(context,identities);
        const auto keys=context.get(opts::Value(object),opts::Key(u"keys"));
        const auto keysFunction=std::get<opts::ObjectRef>(keys.data);
        const auto iteratorValue=context.call(keysFunction,opts::Value(object));
        const auto iterator=std::get<opts::ObjectRef>(iteratorValue.data);
        expect(context.prototype(iterator)==context.intrinsics().mapIteratorPrototype);
        expect(!context.hasOwn(iteratorValue,opts::Key(u"next")));
        handler.set(u"first",{});
        const auto first=context.iteratorNext(iteratorValue);
        expect(context.get(first,opts::Key(u"value"))==opts::Value(u"first"));
        expect(!opts::truthy(context.get(first,opts::Key(u"done"))));
        handler.set(u"added-before-done",{});
        const auto second=context.iteratorNext(iteratorValue);
        expect(context.get(second,opts::Key(u"value"))==opts::Value(u"added-before-done"));
        expect(!opts::truthy(context.get(second,opts::Key(u"done"))));
        const auto done=context.iteratorNext(iteratorValue);
        expect(opts::truthy(context.get(done,opts::Key(u"done"))));
        handler.set(u"added-after-done",{});
        const auto stillDone=context.iteratorNext(iteratorValue);
        expect(opts::truthy(context.get(stillDone,opts::Key(u"done"))));
    };
    "source_configuration_handler_set_overwrites_in_place"_test=[] {
        opts::Context context;
        folia::math::tex::ConfigurationHandler handler;
        folia::math::NativeIdentityBindings<folia::math::tex::ConfigurationRef> identities(context);
        const auto object=handler.createContextObject(context,identities);
        const auto keys=context.get(opts::Value(object),opts::Key(u"keys"));
        const auto iterator=context.call(std::get<opts::ObjectRef>(keys.data),opts::Value(object));
        handler.set(u"first",{});handler.set(u"second",{});handler.set(u"first",{});
        const auto first=context.iteratorNext(iterator);
        const auto second=context.iteratorNext(iterator);
        expect(context.get(first,opts::Key(u"value"))==opts::Value(u"first"));
        expect(context.get(second,opts::Key(u"value"))==opts::Value(u"second"));
    };
    "source_configuration_handler_methods_metadata"_test=[] {
        opts::Context context;
        folia::math::tex::ConfigurationHandler handler;
        folia::math::NativeIdentityBindings<folia::math::tex::ConfigurationRef> identities(context);
        const auto object=handler.createContextObject(context,identities);
        std::string result;
        for(const auto& [name,length]:std::array<std::pair<std::u16string_view,int>,3>{{{u"set",2},{u"get",1},{u"keys",0}}}) {
            if(!result.empty())result+="/";
            const auto function=context.get(opts::Value(object),opts::Key(std::u16string(name)));
            const auto method=std::get<opts::ObjectRef>(function.data);
            const auto desc=context.descriptor(opts::Value(object),opts::Key(std::u16string(name)));
            const auto fdesc=context.descriptor(opts::Value(method),opts::Key(u"length"));
            result+=std::to_string(static_cast<int>(std::get<double>(fdesc->value.data)))+":"+
                (desc->enumerable?"true":"false")+":"+(desc->writable?"true":"false")+":"+
                (desc->configurable?"true":"false");
        }
        expect(result=="2:true:true:true/1:true:true:true/0:true:true:true");
    };
};
}
