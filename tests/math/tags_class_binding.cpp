#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t=folia::math::tex;
namespace o=folia::math::options;
struct Graph {
    o::Context context;
    OptionFunctionBindings functions{context};
    regexp_host::RegExpBindings regexp{context};
    Unicode16Normalization unicode;
    core::InputJaxModule input{context};
    core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    t::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
    Graph(){runtime.initializeParserRuntime();}
};
struct ProbeTags final:t::AbstractTags {using AbstractTags::AbstractTags;};
suite tagsClassBindings=[] {
    "math_tags_class_binding_initializes_actual_derived_identity"_test=[] {
        Graph graph;
        auto& cx=graph.context;auto& runtime=graph.runtime;
        const auto base=std::get<o::ObjectRef>(runtime.abstractTagsClass.data);
        std::vector<t::TagsRef> allocations;
        t::TagsClassRef descriptor;
        std::size_t initialized=0;
        descriptor=t::TagsFactory::registerSubclass(runtime,u"ProbeTags",[&]()->t::TagsRef{
            auto result=std::make_shared<ProbeTags>(runtime);allocations.push_back(result);return result;
        },base,[&](t::Tags& tags){
            ++initialized;
            auto* actual=dynamic_cast<ProbeTags*>(&tags);expect(actual!=nullptr);
            const auto identity=tags.sourceIdentity();
            expect(cx.get(identity,u"constructor")==o::Value(descriptor->constructor));
            expect(cx.get(identity,u"counter")==o::Value(0));
            expect(cx.get(identity,u"configuration")==o::Value(o::Null{}));
            const auto currentTag=cx.get(identity,u"currentTag");
            expect(cx.get(currentTag,u"env")==o::Value(u""));
            expect(eq(cx.arrayLength(std::get<o::ObjectRef>(cx.get(identity,u"history").data)),std::uint32_t(0)));
            expect(eq(cx.arrayLength(std::get<o::ObjectRef>(cx.get(identity,u"stack").data)),std::uint32_t(0)));
            cx.set(identity,u"derived",true);
        });
        const auto first=cx.construct(descriptor->constructor);
        const auto second=cx.construct(descriptor->constructor);
        runtime.tagsFactory.add(u"native-probe",descriptor);
        const auto made=runtime.tagsFactory.create(u"native-probe");
        expect(eq(initialized,std::size_t(3)));
        expect(eq(allocations.size(),std::size_t(3)));
        expect(runtime.tagIdentities.resolve(first)==allocations[0]);
        expect(runtime.tagIdentities.resolve(second)==allocations[1]);
        expect(made==allocations[2]);
        expect(dynamic_cast<ProbeTags*>(made.get())!=nullptr);
        expect(cx.get(made->sourceIdentity(),u"derived")==o::Value(true));
        expect(cx.prototype(descriptor->constructor)==std::optional(base));
        expect(cx.prototype(std::get<o::ObjectRef>(runtime.noTagsClass.data))==std::optional(base));
        expect(cx.prototype(std::get<o::ObjectRef>(runtime.allTagsClass.data))==std::optional(base));
        const auto prototype=std::get<o::ObjectRef>(cx.get(descriptor->constructor,u"prototype").data);
        expect(cx.prototype(prototype)==std::optional(std::get<o::ObjectRef>(cx.get(base,u"prototype").data)));
        expect(cx.get(descriptor->constructor,u"name")==o::Value(u"ProbeTags"));
        expect(cx.get(descriptor->constructor,u"length")==o::Value(0));
        expect(first!=second);
        for(const auto name:{u"ids",u"currentTag",u"history",u"enTag"})expect(cx.get(first,name)!=cx.get(second,name));
    };
};
}
