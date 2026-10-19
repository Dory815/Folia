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
        throw std::runtime_error("split result must be the Context Array");
    return context.arrayValues(*object);
}
std::vector<std::u16string> strings(opts::Context& context,opts::Value value){
    std::vector<std::u16string> result;
    for(const auto& item:values(context,std::move(value))){
        const auto* text=std::get_if<std::u16string>(&item.data);
        if(!text)throw std::runtime_error("expected split string element");
        result.push_back(*text);
    }
    return result;
}
bool typeError(const std::function<void()>& call){
    try{call();}catch(const opts::Exception& error){return error.name=="TypeError";}
    return false;
}

suite contextStringSplit=[] {
    "string_split_uses_utf16_string_separator_and_limit_to_uint32"_test=[] {
        opts::Context context;
        const auto split=context.get(opts::Value(u""),opts::Key(u"split"));
        const std::array<opts::Value,1> separator{opts::Value(u",")};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(u"a,,b,"),separator))==std::vector<std::u16string>{u"a",u"",u"b",u""}));
        const std::array<opts::Value,1> noSeparator{opts::Value(opts::Undefined{})};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(u"abc"),noSeparator))==std::vector<std::u16string>{u"abc"}));
        const std::u16string astral=u"\U0001F642";
        const std::array<opts::Value,1> emptySeparator{opts::Value(u"")};
        const auto units=values(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(astral),emptySeparator));
        expect(units.size()==2);
        expect(units[0]==opts::Value(std::u16string(1,static_cast<char16_t>(0xD83D))));
        expect(units[1]==opts::Value(std::u16string(1,static_cast<char16_t>(0xDE42))));
        const std::array<opts::Value,2> limited{opts::Value(u","),opts::Value(2.9)};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(u"a,b,c"),limited))==std::vector<std::u16string>{u"a",u"b"}));
        const std::array<opts::Value,2> wrapped{opts::Value(u","),opts::Value(-1)};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(u"a,b"),wrapped))==std::vector<std::u16string>{u"a",u"b"}));
        const std::array<opts::Value,2> zero{opts::Value(u","),opts::Value(0)};
        expect(values(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(u"a,b"),zero)).empty());
    };
    "string_split_converts_limit_before_separator_even_when_limit_is_zero"_test=[] {
        opts::Context context;
        const auto split=context.get(opts::Value(u""),opts::Key(u"split"));
        std::vector<std::string> trace;
        const auto receiver=context.object();
        const auto receiverPrimitive=context.function(u"receiver primitive",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("receiver");return opts::Value(u"a,b");
            });
        context.set(receiver,opts::Key(u"toString"),opts::Value(receiverPrimitive));
        const auto separator=context.object();
        const auto separatorPrimitive=context.function(u"separator primitive",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("separator");return opts::Value(u",");
            });
        context.set(separator,opts::Key(u"toString"),opts::Value(separatorPrimitive));
        const auto limit=context.object();
        const auto limitValue=context.function(u"limit valueOf",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("limit");return opts::Value(1);
            });
        context.set(limit,opts::Key(u"valueOf"),opts::Value(limitValue));
        const std::array<opts::Value,2> arguments{opts::Value(separator),opts::Value(limit)};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(receiver),arguments))==std::vector<std::u16string>{u"a"}));
        expect(trace==std::vector<std::string>{"receiver","limit","separator"});
        trace.clear();
        const auto zero=context.function(u"zero valueOf",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("limit");return opts::Value(0);
            });
        context.set(limit,opts::Key(u"valueOf"),opts::Value(zero));
        expect(values(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(receiver),arguments)).empty());
        expect(trace==std::vector<std::string>{"receiver","limit","separator"});
    };
    "string_split_creates_result_indices_without_array_prototype_setters"_test=[] {
        opts::Context context;
        const auto arrayPrototype=context.intrinsics().arrayPrototype;
        unsigned setterCalls=0;
        const auto setter=context.function(u"array index setter",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){++setterCalls;return opts::Value(opts::Undefined{});});
        context.define(arrayPrototype,opts::Key(u"0"),
            opts::Descriptor::access(std::nullopt,setter,true,true));
        const auto split=context.get(opts::Value(u""),opts::Key(u"split"));
        const std::array<opts::Value,1> args{opts::Value(u",")};
        const auto result=values(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(u"a,b"),args));
        expect(setterCalls==0);
        expect(result==std::vector<opts::Value>{opts::Value(u"a"),opts::Value(u"b")});
    };
    "string_split_gets_and_calls_live_symbol_split_before_string_conversion"_test=[] {
        opts::Context context;
        const auto split=context.get(opts::Value(u""),opts::Key(u"split"));
        const auto separator=context.object();
        const auto receiver=context.object();
        std::vector<std::string> trace;
        const auto receiverString=context.function(u"toString",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value>){
                expect(thisValue==opts::Value(receiver));trace.emplace_back("receiver.toString");
                return opts::Value(u"source");
            });
        context.set(receiver,opts::Key(u"toString"),opts::Value(receiverString));
        const auto method=context.function(u"custom split",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value> args){
                expect(thisValue==opts::Value(separator));
                expect(args.size()==2&&args[0]==opts::Value(receiver)&&args[1]==opts::Value(7));
                trace.emplace_back("@@split.call");return opts::Value(receiver);
            });
        const auto getter=context.function(u"get @@split",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value>){
                expect(thisValue==opts::Value(separator));trace.emplace_back("@@split.get");
                return opts::Value(method);
            });
        context.define(separator,opts::Key(context.intrinsics().symbolSplit),
            opts::Descriptor::access(getter,std::nullopt,true,true));
        const std::array<opts::Value,2> arguments{opts::Value(separator),opts::Value(7)};
        const auto result=context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(receiver),arguments);
        expect(result==opts::Value(receiver));
        expect(trace==std::vector<std::string>{"@@split.get","@@split.call"});
        context.define(separator,opts::Key(context.intrinsics().symbolSplit),
            opts::Descriptor::data(opts::Value(5)));
        expect(typeError([&]{(void)context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(u"source"),std::span<const opts::Value>(arguments.data(),1));}));
    };
    "regexp_split_uses_canonical_symbol_method_flags_captures_and_keeps_source_lastindex"_test=[] {
        opts::Context context;
        regex::RegExpBindings bindings(context);
        const auto split=context.get(opts::Value(u""),opts::Key(u"split"));
        const auto splitDescriptor=context.descriptor(opts::Value(context.intrinsics().stringPrototype),
            opts::Key(u"split"));
        expect(splitDescriptor&&splitDescriptor->writable&&!splitDescriptor->enumerable&&splitDescriptor->configurable);
        const auto symbolConstructor=opts::Value(context.intrinsics().symbolConstructor);
        const auto splitSymbolDescriptor=context.descriptor(symbolConstructor,opts::Key(u"split"));
        expect(context.get(symbolConstructor,opts::Key(u"split"))==opts::Value(context.intrinsics().symbolSplit));
        expect(splitSymbolDescriptor&&!splitSymbolDescriptor->writable&&!splitSymbolDescriptor->enumerable&&
               !splitSymbolDescriptor->configurable);
        const auto matchSymbolDescriptor=context.descriptor(symbolConstructor,opts::Key(u"match"));
        expect(context.get(symbolConstructor,opts::Key(u"match"))==opts::Value(context.intrinsics().symbolMatch));
        expect(matchSymbolDescriptor&&!matchSymbolDescriptor->writable&&!matchSymbolDescriptor->enumerable&&
               !matchSymbolDescriptor->configurable);
        expect(context.get(split,opts::Key(u"length"))==opts::Value(2));
        expect(context.get(split,opts::Key(u"name"))==opts::Value(u"split"));
        const auto pattern=bindings.create(u"(,)|(;)" );
        const auto identity=opts::Value(pattern);
        context.set(pattern,opts::Key(u"lastIndex"),opts::Value(9));
        const std::array<opts::Value,1> arguments{identity};
        const auto result=values(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(u"a,b;c"),arguments));
        expect(result.size()==7);
        expect(result[0]==opts::Value(u"a"));expect(result[1]==opts::Value(u","));
        expect(std::holds_alternative<opts::Undefined>(result[2].data));
        expect(result[3]==opts::Value(u"b"));expect(std::holds_alternative<opts::Undefined>(result[4].data));
        expect(result[5]==opts::Value(u";"));expect(result[6]==opts::Value(u"c"));
        expect(context.get(identity,opts::Key(u"lastIndex"))==opts::Value(9));
        expect(context.get(identity,opts::Key(context.intrinsics().symbolSplit))==
               context.get(opts::Value(pattern),opts::Key(context.intrinsics().symbolSplit)));
        const auto symbolSplit=context.get(identity,opts::Key(context.intrinsics().symbolSplit));
        expect(context.get(symbolSplit,opts::Key(u"name"))==opts::Value(u"[Symbol.split]"));
        expect(context.get(symbolSplit,opts::Key(u"length"))==opts::Value(2));
        const auto prototype=std::get<opts::ObjectRef>(context.get(opts::Value(bindings.constructorIdentity()),
            opts::Key(u"prototype")).data);
        const auto actualSplitDescriptor=context.descriptor(opts::Value(prototype),
            opts::Key(context.intrinsics().symbolSplit));
        expect(actualSplitDescriptor&&actualSplitDescriptor->writable&&
            !actualSplitDescriptor->enumerable&&actualSplitDescriptor->configurable);
        const auto flags=context.get(identity,opts::Key(u"flags"));
        expect(flags==opts::Value(u""));
        const auto caseInsensitive=bindings.create(u"a");
        context.define(caseInsensitive,opts::Key(u"flags"),opts::Descriptor::data(opts::Value(u"i")));
        const std::array<opts::Value,1> flagArguments{opts::Value(caseInsensitive)};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(split.data),
            opts::Value(u"Axa"),flagArguments))==std::vector<std::u16string>{u"",u"x",u""}));
    };
    "regexp_split_dispatches_own_symbol_override_and_observes_flags_before_limit"_test=[] {
        opts::Context context;
        regex::RegExpBindings bindings(context);
        const auto split=context.get(opts::Value(u""),opts::Key(u"split"));
        const auto pattern=bindings.create(u"x");
        std::vector<std::string> trace;
        const auto override=context.function(u"custom regex split",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value> args){
                expect(receiver==opts::Value(pattern));
                expect(args.size()==2&&args[0]==opts::Value(u"axb")&&
                       std::holds_alternative<opts::Undefined>(args[1].data));
                trace.emplace_back("call");return opts::Value(u"override");
            });
        context.define(pattern,opts::Key(context.intrinsics().symbolSplit),
            opts::Descriptor::data(opts::Value(override)));
        const std::array<opts::Value,1> arguments{opts::Value(pattern)};
        expect(context.call(std::get<opts::ObjectRef>(split.data),opts::Value(u"axb"),arguments)==
               opts::Value(u"override"));
        expect(trace==std::vector<std::string>{"call"});

        const auto other=bindings.create(u"x");
        const auto flagsGetter=context.function(u"flags getter",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
                expect(receiver==opts::Value(other));trace.emplace_back("flags");
                return opts::Value(u"z");
            });
        context.define(other,opts::Key(u"flags"),opts::Descriptor::access(flagsGetter,std::nullopt));
        const auto limit=context.object();
        const auto valueOf=context.function(u"valueOf",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){trace.emplace_back("limit");return opts::Value(0);});
        context.set(limit,opts::Key(u"valueOf"),opts::Value(valueOf));
        const std::array<opts::Value,2> invalidFlags{opts::Value(other),opts::Value(limit)};
        bool syntax=false;
        try{(void)context.call(std::get<opts::ObjectRef>(split.data),opts::Value(u"axb"),invalidFlags);}
        catch(const opts::Exception& error){syntax=error.name=="SyntaxError";}
        expect(syntax);
        expect(trace==std::vector<std::string>{"call","flags"});
    };
    "regexp_flags_getter_reads_live_component_accessors_in_source_order"_test=[] {
        opts::Context context;
        regex::RegExpBindings bindings(context);
        const auto identity=bindings.create(u"x");
        std::vector<std::u16string> trace;
        const std::array<std::pair<std::u16string_view,bool>,8> flags{{
            {u"hasIndices",false},{u"global",true},{u"ignoreCase",false},
            {u"multiline",false},{u"dotAll",false},{u"unicode",true},
            {u"sticky",false},{u"unicodeSets",false}}};
        for(const auto [name,result]:flags){
            const auto getter=context.function(u"flag getter",
                [&,name,result](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
                    expect(receiver==opts::Value(identity));trace.emplace_back(name);
                    return opts::Value(result);
                });
            context.define(identity,opts::Key(std::u16string(name)),
                opts::Descriptor::access(getter,std::nullopt));
        }
        expect(context.get(opts::Value(identity),opts::Key(u"flags"))==opts::Value(u"gu"));
        expect(std::ranges::equal(trace,std::vector<std::u16string>{u"hasIndices",u"global",u"ignoreCase",
            u"multiline",u"dotAll",u"unicode",u"sticky",u"unicodeSets"}));
    };
    "regexp_split_uses_species_constructor_and_custom_exec_in_source_order"_test=[] {
        opts::Context context;
        regex::RegExpBindings bindings(context);
        const auto original=bindings.create(u"x",u"g");
        const auto holder=context.object();
        std::vector<std::string> trace;
        opts::ObjectRef speciesConstructor{};
        const auto speciesGetter=context.function(u"species getter",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
                expect(receiver==opts::Value(holder));trace.emplace_back("species");
                return opts::Value(speciesConstructor);
            });
        context.define(holder,opts::Key(context.intrinsics().symbolSpecies),
            opts::Descriptor::access(speciesGetter,std::nullopt));
        context.set(original,opts::Key(u"constructor"),opts::Value(holder));
        const auto flagsGetter=context.function(u"flags getter",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
                expect(receiver==opts::Value(original));trace.emplace_back("flags");
                return opts::Value(u"g");
            });
        context.define(original,opts::Key(u"flags"),opts::Descriptor::access(flagsGetter,std::nullopt));

        opts::ObjectRef splitter{};
        auto calls=std::make_shared<unsigned>(0);
        speciesConstructor=context.nativeConstructor(u"Species",
            [&](opts::Context& cx,std::span<const opts::Value> args){
                expect(args.size()==2&&args[0]==opts::Value(original)&&args[1]==opts::Value(u"gy"));
                trace.emplace_back("construct");
                splitter=cx.object();
                const auto exec=cx.function(u"exec",
                    [&,calls](opts::Context& inner,opts::Value receiver,std::span<const opts::Value> args){
                        expect(receiver==opts::Value(splitter));
                        expect(args.size()==1&&args[0]==opts::Value(u"abc"));
                        const auto call=(*calls)++;
                        trace.emplace_back(call==0?"exec0":call==1?"exec1":"exec2");
                        if(call==0){
                            inner.set(receiver,opts::Key(u"lastIndex"),opts::Value(1));
                            const auto match=inner.array(0);
                            inner.define(match,opts::Key(u"0"),opts::Descriptor::data(opts::Value(u"x")));
                            inner.define(match,opts::Key(u"1"),opts::Descriptor::data(opts::Value(42)));
                            return opts::Value(match);
                        }
                        return opts::Value(opts::Null{});
                    });
                cx.set(exec,opts::Key(u"call"),opts::Value(0));
                cx.define(splitter,opts::Key(u"exec"),opts::Descriptor::data(opts::Value(exec)));
                return splitter;
            });
        const auto input=context.object();
        const auto inputPrimitive=context.function(u"input primitive",
            [&](opts::Context&,opts::Value,std::span<const opts::Value> args){
                expect(args.size()==1&&args[0]==opts::Value(u"string"));
                trace.emplace_back("input");return opts::Value(u"abc");
            });
        context.define(input,opts::Key(context.intrinsics().symbolToPrimitive),
            opts::Descriptor::data(opts::Value(inputPrimitive)));
        const auto limit=context.object();
        const auto limitValue=context.function(u"limit valueOf",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("limit");return opts::Value(10);
            });
        context.set(limit,opts::Key(u"valueOf"),opts::Value(limitValue));
        const auto splitMethod=context.get(opts::Value(original),opts::Key(context.intrinsics().symbolSplit));
        const std::array<opts::Value,2> args{opts::Value(input),opts::Value(limit)};
        const auto result=values(context,context.call(std::get<opts::ObjectRef>(splitMethod.data),
            opts::Value(original),args));
        expect(result==std::vector<opts::Value>{opts::Value(u""),opts::Value(42),opts::Value(u"bc")});
        expect(trace==std::vector<std::string>{"input","species","flags","construct","limit","exec0","exec1","exec2"});
        expect(context.get(opts::Value(splitter),opts::Key(u"lastIndex"))==opts::Value(2));

        const auto invalid=bindings.create(u"x");
        const auto invalidHolder=context.object();
        context.set(invalid,opts::Key(u"constructor"),opts::Value(invalidHolder));
        const auto nonConstructor=context.function(u"not constructable",
            [](opts::Context&,opts::Value,std::span<const opts::Value>){return opts::Value(opts::Undefined{});});
        context.define(invalidHolder,opts::Key(context.intrinsics().symbolSpecies),
            opts::Descriptor::data(opts::Value(nonConstructor)));
        std::vector<std::string> invalidTrace;
        const auto invalidFlags=context.function(u"flags getter",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                invalidTrace.emplace_back("flags");return opts::Value(u"");
            });
        context.define(invalid,opts::Key(u"flags"),opts::Descriptor::access(invalidFlags,std::nullopt));
        bool rejected=false;
        try{(void)context.call(std::get<opts::ObjectRef>(splitMethod.data),opts::Value(invalid),args);}
        catch(const opts::Exception& error){rejected=error.name=="TypeError";}
        expect(rejected&&invalidTrace.empty());
    };
    "regexp_exec_falls_back_for_noncallable_exec_only_on_native_regex_objects"_test=[] {
        opts::Context context;
        regex::RegExpBindings bindings(context);
        const auto prototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(bindings.constructorIdentity()),opts::Key(u"prototype")).data);
        const auto splitMethod=context.get(opts::Value(prototype),
            opts::Key(context.intrinsics().symbolSplit));
        const auto original=bindings.create(u"x");
        const auto nativeHolder=context.object();
        const auto nativeSpecies=context.nativeConstructor(u"NativeSpecies",
            [&](opts::Context& cx,std::span<const opts::Value>){
                const auto splitter=bindings.create(u"x",u"y");
                cx.set(splitter,opts::Key(u"exec"),opts::Value(0));
                return splitter;
            });
        context.define(nativeHolder,opts::Key(context.intrinsics().symbolSpecies),
            opts::Descriptor::data(opts::Value(nativeSpecies)));
        context.set(original,opts::Key(u"constructor"),opts::Value(nativeHolder));
        const std::array<opts::Value,1> text{opts::Value(u"axb")};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(splitMethod.data),
            opts::Value(original),text))==std::vector<std::u16string>{u"a",u"b"}));
        const std::array<opts::Value,1> empty{opts::Value(u"")};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(splitMethod.data),
            opts::Value(original),empty))==std::vector<std::u16string>{u""}));

        const auto plainOriginal=bindings.create(u"x");
        const auto plainHolder=context.object();
        const auto plainSpecies=context.nativeConstructor(u"PlainSpecies",
            [&](opts::Context& cx,std::span<const opts::Value>){
                const auto splitter=cx.object();
                cx.set(splitter,opts::Key(u"exec"),opts::Value(opts::Null{}));
                return splitter;
            });
        context.define(plainHolder,opts::Key(context.intrinsics().symbolSpecies),
            opts::Descriptor::data(opts::Value(plainSpecies)));
        context.set(plainOriginal,opts::Key(u"constructor"),opts::Value(plainHolder));
        expect(typeError([&]{(void)context.call(std::get<opts::ObjectRef>(splitMethod.data),
            opts::Value(plainOriginal),text);}));
    };
    "regexp_call_constructor_identity_fast_path_observes_match_and_constructor_once"_test=[] {
        opts::Context context;
        regex::RegExpBindings bindings(context);
        const auto constructor=bindings.constructorIdentity();
        const auto callConstructor=[&](opts::ObjectRef pattern){
            const std::array<opts::Value,1> args{opts::Value(pattern)};
            return context.call(constructor,opts::Value(opts::Undefined{}),args);
        };

        const auto ordinary=bindings.create(u"x",u"g");
        context.set(ordinary,opts::Key(u"constructor"),opts::Value(constructor));
        expect(callConstructor(ordinary)==opts::Value(ordinary));

        std::vector<std::string> trace;
        const auto generic=context.object();
        const auto genericMatch=context.function(u"get generic @@match",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("match");return opts::Value(true);
            });
        context.define(generic,opts::Key(context.intrinsics().symbolMatch),
            opts::Descriptor::access(genericMatch,std::nullopt));
        const auto genericConstructor=context.function(u"get generic constructor",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("constructor");return opts::Value(constructor);
            });
        context.define(generic,opts::Key(u"constructor"),
            opts::Descriptor::access(genericConstructor,std::nullopt));
        expect(callConstructor(generic)==opts::Value(generic));
        expect(trace==std::vector<std::string>{"match","constructor"});

        trace.clear();
        const auto matchGetter=context.function(u"get @@match",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("match");return opts::Value(false);
            });
        const auto noMatch=bindings.create(u"x",u"g");
        context.define(noMatch,opts::Key(context.intrinsics().symbolMatch),
            opts::Descriptor::access(matchGetter,std::nullopt));
        const auto noMatchConstructor=context.function(u"get constructor",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("constructor");return opts::Value(constructor);
            });
        context.define(noMatch,opts::Key(u"constructor"),
            opts::Descriptor::access(noMatchConstructor,std::nullopt));
        const auto noMatchClone=std::get<opts::ObjectRef>(callConstructor(noMatch).data);
        expect(noMatchClone!=noMatch);
        expect(bindings.resolve(opts::Value(noMatchClone))->pattern()==u"x");
        expect(bindings.resolve(opts::Value(noMatchClone))->flags()==u"g");
        expect(trace==std::vector<std::string>{"match"});

        trace.clear();
        const auto matchTrue=context.function(u"get @@match",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("match");return opts::Value(true);
            });
        const auto otherConstructor=context.object();
        const auto constructorGetter=context.function(u"get constructor",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("constructor");return opts::Value(otherConstructor);
            });
        const auto differentConstructor=bindings.create(u"x",u"g");
        context.define(differentConstructor,opts::Key(context.intrinsics().symbolMatch),
            opts::Descriptor::access(matchTrue,std::nullopt));
        context.define(differentConstructor,opts::Key(u"constructor"),
            opts::Descriptor::access(constructorGetter,std::nullopt));
        const auto cloned=std::get<opts::ObjectRef>(callConstructor(differentConstructor).data);
        expect(cloned!=differentConstructor);
        expect(bindings.resolve(opts::Value(cloned))->pattern()==u"x");
        expect(trace==std::vector<std::string>{"match","constructor"});
    };
    "regexp_split_accepts_generic_object_receiver_and_rejects_primitives_before_input_conversion"_test=[] {
        opts::Context context;
        regex::RegExpBindings bindings(context);
        const auto splitMethod=context.get(opts::Value(bindings.constructorIdentity()),
            opts::Key(u"prototype"));
        const auto prototype=std::get<opts::ObjectRef>(splitMethod.data);
        const auto method=context.get(opts::Value(prototype),opts::Key(context.intrinsics().symbolSplit));
        std::vector<std::string> trace;
        const auto generic=context.object();
        context.set(generic,opts::Key(u"constructor"),opts::Value(bindings.constructorIdentity()));
        context.set(generic,opts::Key(u"flags"),opts::Value(u""));
        const auto genericString=context.function(u"toString",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
                expect(receiver==opts::Value(generic));trace.emplace_back("generic.toString");
                return opts::Value(u"x");
            });
        context.set(generic,opts::Key(u"toString"),opts::Value(genericString));
        const std::array<opts::Value,1> input{opts::Value(u"axb")};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(method.data),
            opts::Value(generic),input))==std::vector<std::u16string>{u"a",u"b"}));
        expect(trace==std::vector<std::string>{"generic.toString"});

        trace.clear();
        const auto inputObject=context.object();
        const auto inputPrimitive=context.function(u"input primitive",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("input.toPrimitive");return opts::Value(u"axb");
            });
        context.set(inputObject,opts::Key(context.intrinsics().symbolToPrimitive),
            opts::Value(inputPrimitive));
        const std::array<opts::Value,1> primitiveArguments{opts::Value(inputObject)};
        expect(typeError([&]{(void)context.call(std::get<opts::ObjectRef>(method.data),
            opts::Value(1),primitiveArguments);}));
        expect(trace.empty());
    };
    "regexp_constructor_uses_isregexp_source_for_generic_symbol_match_receiver"_test=[] {
        opts::Context context;
        regex::RegExpBindings bindings(context);
        const auto regexpPrototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(bindings.constructorIdentity()),opts::Key(u"prototype")).data);
        const auto splitMethod=context.get(opts::Value(regexpPrototype),
            opts::Key(context.intrinsics().symbolSplit));
        const auto receiver=context.object();
        context.set(receiver,opts::Key(u"constructor"),opts::Value(bindings.constructorIdentity()));
        std::vector<std::string> trace;
        const auto matchGetter=context.function(u"get @@match",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value>){
                expect(thisValue==opts::Value(receiver));trace.emplace_back("match");
                return opts::Value(true);
            });
        context.define(receiver,opts::Key(context.intrinsics().symbolMatch),
            opts::Descriptor::access(matchGetter,std::nullopt));
        const auto sourceFlags=context.object();
        const auto flagsPrimitive=context.function(u"flags primitive",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("string flags");return opts::Value(u"");
            });
        context.set(sourceFlags,opts::Key(context.intrinsics().symbolToPrimitive),
            opts::Value(flagsPrimitive));
        const auto flagsGetter=context.function(u"get flags",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value>){
                expect(thisValue==opts::Value(receiver));trace.emplace_back("flags");
                return opts::Value(sourceFlags);
            });
        context.define(receiver,opts::Key(u"flags"),opts::Descriptor::access(flagsGetter,std::nullopt));
        const auto sourceValue=context.object();
        const auto sourcePrimitive=context.function(u"source primitive",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("string source");return opts::Value(u"x");
            });
        context.set(sourceValue,opts::Key(context.intrinsics().symbolToPrimitive),
            opts::Value(sourcePrimitive));
        const auto sourceGetter=context.function(u"get source",
            [&](opts::Context&,opts::Value thisValue,std::span<const opts::Value>){
                expect(thisValue==opts::Value(receiver));trace.emplace_back("source");
                return opts::Value(sourceValue);
            });
        context.define(receiver,opts::Key(u"source"),opts::Descriptor::access(sourceGetter,std::nullopt));
        const auto toString=context.function(u"toString",
            [&](opts::Context&,opts::Value,std::span<const opts::Value>){
                trace.emplace_back("toString");return opts::Value(u"z");
            });
        context.set(receiver,opts::Key(u"toString"),opts::Value(toString));
        const std::array<opts::Value,1> arguments{opts::Value(u"axb")};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(splitMethod.data),
            opts::Value(receiver),arguments))==std::vector<std::u16string>{u"a",u"b"}));
        expect(trace==std::vector<std::string>{"flags","string flags","match","source","string source"});

        trace.clear();
        const std::array<opts::Value,1> constructorArguments{opts::Value(receiver)};
        const auto constructed=context.construct(bindings.constructorIdentity(),constructorArguments);
        const auto constructedOwner=bindings.resolve(opts::Value(constructed));
        expect(constructedOwner->pattern()==u"x"&&constructedOwner->flags().empty());
        expect(trace==std::vector<std::string>{"match","source","flags","string source","string flags"});

        const auto branded=bindings.create(u"x");
        context.define(branded,opts::Key(u"source"),opts::Descriptor::data(opts::Value(u"z")));
        const std::array<opts::Value,1> brandedArguments{opts::Value(u"x")};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(splitMethod.data),
            opts::Value(branded),brandedArguments))==std::vector<std::u16string>{u"",u""}));

        const auto brandedNoMatch=bindings.create(u"x",u"g");
        std::vector<std::string> nativeBrandTrace;
        const auto brandedMatchGetter=context.function(u"get @@match",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
                expect(receiver==opts::Value(brandedNoMatch));nativeBrandTrace.emplace_back("match");
                return opts::Value(false);
            });
        context.define(brandedNoMatch,opts::Key(context.intrinsics().symbolMatch),
            opts::Descriptor::access(brandedMatchGetter,std::nullopt));
        const auto brandedSourceGetter=context.function(u"get source",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
                expect(receiver==opts::Value(brandedNoMatch));nativeBrandTrace.emplace_back("source");
                return opts::Value(u"z");
            });
        context.define(brandedNoMatch,opts::Key(u"source"),
            opts::Descriptor::access(brandedSourceGetter,std::nullopt));
        const std::array<opts::Value,1> brandedNoMatchArguments{opts::Value(u"axb")};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(splitMethod.data),
            opts::Value(brandedNoMatch),brandedNoMatchArguments))==std::vector<std::u16string>{u"a",u"b"}));
        expect(nativeBrandTrace==std::vector<std::string>{"match"});
    };
    "regexp_split_default_species_uses_live_prototype_exec_on_the_clone"_test=[] {
        opts::Context context;
        regex::RegExpBindings bindings(context);
        const auto original=bindings.create(u"x");
        const auto prototype=std::get<opts::ObjectRef>(context.get(
            opts::Value(bindings.constructorIdentity()),opts::Key(u"prototype")).data);
        std::vector<opts::ObjectRef> receivers;
        const auto customExec=context.function(u"exec",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value> args){
                expect(args.size()==1&&args[0]==opts::Value(u"abc"));
                receivers.push_back(std::get<opts::ObjectRef>(receiver.data));
                return opts::Value(opts::Null{});
            });
        context.set(prototype,opts::Key(u"exec"),opts::Value(customExec));
        const auto splitMethod=context.get(opts::Value(u""),opts::Key(u"split"));
        const std::array<opts::Value,1> args{opts::Value(original)};
        expect(static_cast<bool>(strings(context,context.call(std::get<opts::ObjectRef>(splitMethod.data),
            opts::Value(u"abc"),args))==std::vector<std::u16string>{u"abc"}));
        expect(receivers.size()==3);
        for(const auto receiver:receivers){
            expect(receiver!=original);
            expect(bindings.resolve(opts::Value(receiver))!=nullptr);
        }
    };
};
}
