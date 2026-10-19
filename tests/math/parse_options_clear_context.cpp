#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o=folia::math::options;
namespace t=folia::math::tex;
struct ClearGraph {
    o::Context context;
    OptionFunctionBindings functions{context};
    regexp_host::RegExpBindings regexp{context};
    Unicode16Normalization unicode;
    core::InputJaxModule input{context};
    core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    t::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
    ClearGraph(){runtime.initializeParserRuntime();}
    o::Value clear(){
        const auto prototype=context.get(runtime.parseOptionsClass,u"prototype");
        return context.get(prototype,u"clear");
    }
};
suite parseOptionsClearContext=[] {
    "parse_options_clear_generic_receiver_preserves_source_order"_test=[] {
        ClearGraph graph;
        auto& context=graph.context;
        const auto receiver=context.object(),tags=context.object();
        std::vector<std::u16string> events;
        std::vector<std::pair<std::u16string,o::Value>> writes;
        for(const auto name:{u"parsers",u"root",u"nodeLists",u"error"}){
            const auto setter=context.function(u"",[&,name](o::Context&,o::Value self,
                    std::span<const o::Value> args){
                expect(self==o::Value(receiver));
                events.emplace_back(name);writes.emplace_back(name,args[0]);
                return o::Value(o::Undefined{});
            });
            context.define(receiver,name,o::Descriptor::access(std::nullopt,setter));
        }
        const auto reset=context.function(u"",[&](o::Context&,o::Value self,
                std::span<const o::Value> args){
            expect(self==o::Value(tags)&&args.empty());
            events.emplace_back(u"call:reset");return o::Value(99);
        });
        const auto resetGetter=context.function(u"",[&](o::Context&,o::Value self,
                std::span<const o::Value>){
            expect(self==o::Value(tags));events.emplace_back(u"get:reset");return o::Value(reset);
        });
        context.define(tags,u"resetTag",o::Descriptor::access(resetGetter,std::nullopt));
        const auto tagsGetter=context.function(u"",[&](o::Context&,o::Value self,
                std::span<const o::Value>){
            expect(self==o::Value(receiver));events.emplace_back(u"get:tags");return o::Value(tags);
        });
        context.define(receiver,u"tags",o::Descriptor::access(tagsGetter,std::nullopt));
        const auto clear=graph.clear();
        expect(graph.clear()==clear);
        const std::array<o::Value,2> ignoredArguments{o::Value(17),o::Value(u"ignored")};
        expect(context.callMember(clear,receiver,ignoredArguments)==o::Value(o::Undefined{}));
        expect(static_cast<bool>(events==std::vector<std::u16string>{
            u"parsers",u"root",u"nodeLists",u"error",u"get:tags",u"get:reset",u"call:reset"}));
        expect(writes.size()==4u);
        if(writes.size()==4u){
            expect(context.kind(std::get<o::ObjectRef>(writes[0].second.data))==o::ObjectKind::array);
            expect(writes[1].second==o::Value(o::Null{}));
            expect(context.kind(std::get<o::ObjectRef>(writes[2].second.data))==o::ObjectKind::object);
            expect(writes[3].second==o::Value(false));
        }
        expect(context.get(clear,u"name")==o::Value(u""));
        expect(context.get(clear,u"length")==o::Value(0));
        const auto prototype=std::get<o::ObjectRef>(context.get(graph.runtime.parseOptionsClass,u"prototype").data);
        const auto descriptor=context.descriptor(prototype,u"clear");
        expect(descriptor&&descriptor->writable&&descriptor->enumerable&&descriptor->configurable);
    };
    "parse_options_clear_preserves_writes_before_reset_tag_getter_throws"_test=[] {
        ClearGraph graph;
        auto& context=graph.context;
        const auto receiver=context.object(),tags=context.object();
        context.set(receiver,u"tags",tags);
        const auto failure=context.function(u"",[](o::Context&,o::Value,
                std::span<const o::Value>)->o::Value{throw o::Exception("WitnessError",u"reset getter");});
        context.define(tags,u"resetTag",o::Descriptor::access(failure,std::nullopt));
        expect(throws<o::Exception>([&]{(void)context.callMember(graph.clear(),receiver,{});}));
        expect(context.kind(std::get<o::ObjectRef>(context.get(receiver,u"parsers").data))==o::ObjectKind::array);
        expect(context.get(receiver,u"root")==o::Value(o::Null{}));
        expect(context.kind(std::get<o::ObjectRef>(context.get(receiver,u"nodeLists").data))==o::ObjectKind::object);
        expect(context.get(receiver,u"error")==o::Value(false));
    };
};
}
