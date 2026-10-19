#include "support/folia_test.hpp"
import folia.math.util.option_types;
import folia.math.util.regexp_binding;

namespace {
using namespace boost::ut;
namespace opts=folia::math::options;
namespace regex=folia::math::regexp_host;

std::vector<opts::Value> values(opts::Context& context,opts::Value value){
    const auto* object=std::get_if<opts::ObjectRef>(&value.data);
    if(!object||context.kind(*object)!=opts::ObjectKind::array)
        throw std::runtime_error("match result must be the Context Array");
    return context.arrayValues(*object);
}
std::vector<std::u16string> strings(opts::Context& context,opts::Value value){
    std::vector<std::u16string> result;
    for(const auto& item:values(context,std::move(value))){
        const auto* text=std::get_if<std::u16string>(&item.data);
        if(!text)throw std::runtime_error("expected match string element");
        result.push_back(*text);
    }
    return result;
}
bool typeError(const std::function<void()>& call){
    try{call();}catch(const opts::Exception& error){return error.name=="TypeError";}
    return false;
}

suite contextStringMatch=[] {
    "string_slice_uses_to_integer_or_infinity_and_utf16_indices"_test=[] {
        opts::Context context;
        const auto slice=context.get(opts::Value(u""),opts::Key(u"slice"));
        const auto call=[&](opts::Value receiver,std::span<const opts::Value> arguments){
            return context.call(std::get<opts::ObjectRef>(slice.data),std::move(receiver),arguments);
        };
        const std::array<opts::Value,2> negative{opts::Value(-4),opts::Value(-1)};
        expect(call(opts::Value(u"abcdef"),negative)==opts::Value(u"cde"));
        const std::array<opts::Value,2> infinities{opts::Value(-std::numeric_limits<double>::infinity()),
            opts::Value(std::numeric_limits<double>::infinity())};
        expect(call(opts::Value(u"abc"),infinities)==opts::Value(u"abc"));
        const std::array<opts::Value,2> nanAndUndefined{opts::Value(std::numeric_limits<double>::quiet_NaN()),
            opts::Value(opts::Undefined{})};
        expect(call(opts::Value(u"abc"),nanAndUndefined)==opts::Value(u"abc"));
        const auto astral=std::u16string(u"\U0001F642");
        const std::array<opts::Value,2> firstCodeUnit{opts::Value(0),opts::Value(1)};
        expect(call(opts::Value(astral),firstCodeUnit)==opts::Value(std::u16string(1,astral[0])));
        std::vector<std::string> trace;
        const auto receiver=context.object(),start=context.object(),end=context.object();
        const auto primitive=[&](std::string label,std::u16string result){
            return context.function(u"coerce",[&,label=std::move(label),result=std::move(result)]
                (opts::Context&,opts::Value,std::span<const opts::Value>){trace.push_back(label);return opts::Value(result);});
        };
        context.set(receiver,opts::Key(u"toString"),opts::Value(primitive("receiver",u"abcd")));
        context.set(start,opts::Key(u"valueOf"),opts::Value(primitive("start",u"1")));
        context.set(end,opts::Key(u"valueOf"),opts::Value(primitive("end",u"3")));
        const std::array<opts::Value,2> coercions{opts::Value(start),opts::Value(end)};
        expect(call(opts::Value(receiver),coercions)==opts::Value(u"bc"));
        expect(trace==std::vector<std::string>{"receiver","start","end"});
    };
    "string_match_calls_custom_symbol_method_with_raw_receiver_and_abstract_call"_test=[] {
        opts::Context context;regex::RegExpBindings bindings(context);
        std::vector<std::string> trace;
        const auto match=context.get(opts::Value(u""),opts::Key(u"match"));
        const auto receiver=context.object(),regexp=context.object();
        const auto toString=context.function(u"receiver toString",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){trace.emplace_back("stringify");return opts::Value(u"raw");});
        context.set(receiver,opts::Key(u"toString"),opts::Value(toString));
        const auto method=context.function(u"custom @@match",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value> args){
                trace.emplace_back("call");
                expect(opts::sameValue(thisValue,opts::Value(regexp)));
                expect(args.size()==1&&opts::sameValue(args[0],opts::Value(receiver)));
                return opts::Value(u"custom result");
            });
        context.set(method,opts::Key(u"call"),opts::Value(0));
        context.define(regexp,opts::Key(context.intrinsics().symbolMatch),opts::Descriptor::data(opts::Value(method)));
        const std::array<opts::Value,1> arguments{opts::Value(regexp)};
        expect(context.call(std::get<opts::ObjectRef>(match.data),opts::Value(receiver),arguments)==opts::Value(u"custom result"));
        expect(trace==std::vector<std::string>{"call"});
        expect(bindings.constructorIdentity().context==regexp.context);
    };
    "string_match_validates_receiver_and_matcher_before_string_coercion"_test=[] {
        opts::Context context;regex::RegExpBindings bindings(context);
        const auto method=context.get(opts::Value(u""),opts::Key(u"match"));
        std::vector<std::string> trace;
        const auto receiver=context.object();
        const auto stringify=context.function(u"toString",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){trace.emplace_back("stringify");return opts::Value(u"text");});
        context.set(receiver,opts::Key(u"toString"),opts::Value(stringify));
        const auto regexp=context.object();
        const auto getter=context.function(u"get @@match",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){trace.emplace_back("get match");return opts::Value(0);});
        context.define(regexp,opts::Key(context.intrinsics().symbolMatch),opts::Descriptor::access(getter,std::nullopt));
        const std::array<opts::Value,1> args{opts::Value(regexp)};
        expect(typeError([&]{(void)context.call(std::get<opts::ObjectRef>(method.data),opts::Value(receiver),args);}));
        expect(trace==std::vector<std::string>{"get match"});
        trace.clear();
        expect(typeError([&]{(void)context.call(std::get<opts::ObjectRef>(method.data),
            opts::Value(opts::Null{}),args);}));
        expect(trace.empty());
        expect(bindings.constructorIdentity().context==regexp.context);
    };
    "string_match_fallback_constructs_a_real_regexp_and_returns_match_array"_test=[] {
        opts::Context context;regex::RegExpBindings bindings(context);
        const auto match=context.get(opts::Value(u""),opts::Key(u"match"));
        const std::array<opts::Value,1> pattern{opts::Value(u"b")};
        const auto result=context.call(std::get<opts::ObjectRef>(match.data),opts::Value(u"abc"),pattern);
        expect(static_cast<bool>(strings(context,result)==std::vector<std::u16string>{u"b"}));
        const auto object=std::get<opts::ObjectRef>(result.data);
        expect(context.get(opts::Value(object),opts::Key(u"index"))==opts::Value(1));
        expect(context.get(opts::Value(object),opts::Key(u"input"))==opts::Value(u"abc"));
        const auto regexp=bindings.create(u"a",u"g");
        const auto matcher=context.get(opts::Value(regexp),opts::Key(context.intrinsics().symbolMatch));
        const std::array<opts::Value,1> input{opts::Value(u"aba")};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(matcher.data),
            opts::Value(regexp),input))==std::vector<std::u16string>{u"a",u"a"}));
    };
    "regexp_match_generic_receiver_reads_input_then_flags_then_exec"_test=[] {
        opts::Context context;regex::RegExpBindings bindings(context);
        std::vector<std::string> trace;
        const auto prototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(bindings.constructorIdentity()),opts::Key(u"prototype")).data);
        const auto matcher=context.get(opts::Value(prototype),opts::Key(context.intrinsics().symbolMatch));
        const auto receiver=context.object();
        const auto input=context.object();
        const auto inputString=context.function(u"input toString",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){trace.emplace_back("input");return opts::Value(u"x");});
        context.set(input,opts::Key(u"toString"),opts::Value(inputString));
        const auto flagsGetter=context.function(u"flags getter",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){trace.emplace_back("flags");return opts::Value(u"");});
        context.define(receiver,opts::Key(u"flags"),opts::Descriptor::access(flagsGetter,std::nullopt));
        const auto result=context.object();
        context.define(result,opts::Key(u"0"),opts::Descriptor::data(opts::Value(u"x")));
        const auto exec=context.function(u"custom exec",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value> args){
                trace.emplace_back("exec");expect(opts::sameValue(thisValue,opts::Value(receiver)));
                expect(args.size()==1&&args[0]==opts::Value(u"x"));return opts::Value(result);
            });
        context.set(exec,opts::Key(u"call"),opts::Value(0));
        context.define(receiver,opts::Key(u"exec"),opts::Descriptor::data(opts::Value(exec)));
        const std::array<opts::Value,1> arguments{opts::Value(input)};
        const auto actual=context.call(std::get<opts::ObjectRef>(matcher.data),opts::Value(receiver),arguments);
        expect(opts::sameValue(actual,opts::Value(result)));
        expect(trace==std::vector<std::string>{"input","flags","exec"});
    };
    "regexp_flags_getter_reads_the_observed_node_order"_test=[] {
        opts::Context context;regex::RegExpBindings bindings(context);
        const auto prototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(bindings.constructorIdentity()),opts::Key(u"prototype")).data);
        const auto flagsDescriptor=context.descriptor(opts::Value(prototype),opts::Key(u"flags"));
        expect(flagsDescriptor&&flagsDescriptor->accessor&&flagsDescriptor->getter.has_value());
        if(!flagsDescriptor||!flagsDescriptor->getter)return;
        const auto getter=*flagsDescriptor->getter;
        const std::array<std::u16string,8> names{u"hasIndices",u"global",u"ignoreCase",u"multiline",
            u"dotAll",u"unicode",u"sticky",u"unicodeSets"};
        std::vector<std::u16string> trace;
        const auto receiver=context.object();
        for(const auto& name:names){
            const auto propertyGetter=context.function(u"flag getter",
                [&,name](opts::Context&,opts::Value,std::span<const opts::Value>){trace.push_back(name);return opts::Value(false);});
            context.define(receiver,opts::Key(name),opts::Descriptor::access(propertyGetter,std::nullopt));
        }
        expect(context.call(getter,opts::Value(receiver))==opts::Value(u""));
        expect(static_cast<bool>(trace==std::vector<std::u16string>(names.begin(),names.end())));
    };
    "regexp_match_global_resets_last_index_before_getting_exec"_test=[] {
        opts::Context context;regex::RegExpBindings bindings(context);
        const auto prototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(bindings.constructorIdentity()),opts::Key(u"prototype")).data);
        const auto matcher=context.get(opts::Value(prototype),opts::Key(context.intrinsics().symbolMatch));
        const auto receiver=context.object();
        std::vector<std::string> trace;
        context.define(receiver,opts::Key(u"flags"),opts::Descriptor::data(opts::Value(u"g")));
        const auto setter=context.function(u"lastIndex setter",
            [&](opts::Context&,opts::Value,std::span<const opts::Value> args){
                expect(args.size()==1&&args[0]==opts::Value(0));trace.emplace_back("set lastIndex");
                return opts::Value(opts::Undefined{});
            });
        context.define(receiver,opts::Key(u"lastIndex"),opts::Descriptor::access(std::nullopt,setter));
        const auto exec=context.function(u"exec getter result",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value>){
                expect(opts::sameValue(thisValue,opts::Value(receiver)));trace.emplace_back("exec call");
                return opts::Value(opts::Null{});
            });
        const auto execGetter=context.function(u"exec getter",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){trace.emplace_back("get exec");return opts::Value(exec);});
        context.define(receiver,opts::Key(u"exec"),opts::Descriptor::access(execGetter,std::nullopt));
        const std::array<opts::Value,1> input{opts::Value(u"x")};
        expect(std::holds_alternative<opts::Null>(context.call(
            std::get<opts::ObjectRef>(matcher.data),opts::Value(receiver),input).data));
        expect(trace==std::vector<std::string>{"set lastIndex","get exec","exec call"});
    };
    "regexp_match_global_empty_advancement_uses_full_unicode_only_when_flagged"_test=[] {
        opts::Context context;regex::RegExpBindings bindings(context);
        const auto prototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(bindings.constructorIdentity()),opts::Key(u"prototype")).data);
        const auto matcher=context.get(opts::Value(prototype),opts::Key(context.intrinsics().symbolMatch));
        const auto input=std::u16string(u"\U0001F642");
        for(const auto& [flags,expected]:std::array<std::pair<std::u16string,std::size_t>,2>{{{u"g",3},{u"gu",2}}}){
            const auto regexp=bindings.create(u"(?:)",flags);
            const std::array<opts::Value,1> arguments{opts::Value(input)};
            const auto result=context.call(std::get<opts::ObjectRef>(matcher.data),opts::Value(regexp),arguments);
            expect(values(context,result).size()==expected);
            for(const auto& item:values(context,result))expect(item==opts::Value(u""));
        }
        const auto empty=bindings.create(u"(?:)",u"g");
        const std::array<opts::Value,1> emptyInput{opts::Value(u"")};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(matcher.data),
            opts::Value(empty),emptyInput))==std::vector<std::u16string>{u""}));
    };
    "regexp_match_preserves_noncallable_exec_fallback_and_rejects_primitive_results"_test=[] {
        opts::Context context;regex::RegExpBindings bindings(context);
        const auto prototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(bindings.constructorIdentity()),opts::Key(u"prototype")).data);
        const auto matcher=context.get(opts::Value(prototype),opts::Key(context.intrinsics().symbolMatch));
        const auto regexp=bindings.create(u"x");
        context.set(regexp,opts::Key(u"exec"),opts::Value(opts::Null{}));
        const std::array<opts::Value,1> input{opts::Value(u"x")};
        const auto fallback=context.call(std::get<opts::ObjectRef>(matcher.data),opts::Value(regexp),input);
        expect(std::holds_alternative<opts::ObjectRef>(fallback.data));
        expect(context.kind(std::get<opts::ObjectRef>(fallback.data))==opts::ObjectKind::array);
        const auto plain=context.object();
        context.set(plain,opts::Key(u"flags"),opts::Value(u""));
        context.set(plain,opts::Key(u"exec"),opts::Value(opts::Null{}));
        expect(typeError([&]{(void)context.call(std::get<opts::ObjectRef>(matcher.data),
            opts::Value(plain),input);}));
        const auto primitiveResult=context.function(u"primitive exec",
            [](opts::Context&,opts::Value,std::span<const opts::Value>){return opts::Value(1);});
        context.set(plain,opts::Key(u"exec"),opts::Value(primitiveResult));
        expect(typeError([&]{(void)context.call(std::get<opts::ObjectRef>(matcher.data),
            opts::Value(plain),input);}));
    };
};
}
