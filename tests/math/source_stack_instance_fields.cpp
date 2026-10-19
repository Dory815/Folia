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

struct FieldOwner {
    o::Value value=7.0;
    std::size_t writes=0;
};
struct Probe final:t::BaseItem {
    std::shared_ptr<FieldOwner> field;
    explicit Probe(std::shared_ptr<FieldOwner> value):field(std::move(value)){}
};

struct ProbeClass {
    t::StackItemClassRef descriptor;
    o::ObjectRef constructor;
    std::shared_ptr<std::vector<std::shared_ptr<FieldOwner>>> owners=
        std::make_shared<std::vector<std::shared_ptr<FieldOwner>>>();
    std::shared_ptr<std::size_t> initializers=std::make_shared<std::size_t>(0);
};

ProbeClass registerProbe(Graph& graph) {
    ProbeClass result;
    auto& runtime=graph.runtime;
    const auto baseValue=graph.context.get(runtime.stackItemModule,o::Key(u"BaseItem"));
    const auto baseConstructor=std::get<o::ObjectRef>(baseValue.data);
    auto initializers=result.initializers;
    auto owners=result.owners;
    auto surface=t::StackItemSourceSurface{
        true,
        {},
        {{u"value",o::Value(o::Undefined{}),
          [](t::BaseItem& item){return dynamic_cast<Probe&>(item).field->value;},
          [](t::BaseItem& item,o::Value value){auto& field=*dynamic_cast<Probe&>(item).field;++field.writes;field.value=std::move(value);},
          [initializers](t::BaseItem&){++*initializers;return o::Value(29.0);}}}
    };
    result.descriptor=t::StackItemFactoryBindings::registerSubclass(
        runtime,u"ProbeFieldItem","probe-field-item",0,typeid(Probe),
        [owners](){auto field=std::make_shared<FieldOwner>();owners->push_back(field);return std::make_unique<Probe>(std::move(field));},
        [](t::BaseItem&,tree::FactoryArgs){},
        [](const t::BaseItem* item){return dynamic_cast<const Probe*>(item)!=nullptr;},
        [](){return t::BaseItem::baseErrors();},baseConstructor,false,std::move(surface));
    const auto constructor=runtime.itemClassIdentities.find(result.descriptor);
    result.constructor=*constructor;
    return result;
}

o::ObjectRef baseItemPrototype(Graph& graph) {
    const auto base=std::get<o::ObjectRef>(graph.context.get(graph.runtime.stackItemModule,o::Key(u"BaseItem")).data);
    return std::get<o::ObjectRef>(graph.context.get(o::Value(base),o::Key(u"prototype")).data);
}

suite sourceStackInstanceFields=[] {
    "source_stack_instance_fields_assignment_runs_inherited_setter_once"_test=[] {
        Graph graph;
        auto probe=registerProbe(graph);
        std::size_t setterCalls=0;
        const auto setter=graph.context.function(u"set value",[&setterCalls](auto& cx,auto receiver,std::span<const o::Value> args){
            ++setterCalls;cx.set(receiver,o::Key(u"observed"),args[0]);return o::Value(o::Undefined{});
        });
        graph.context.define(baseItemPrototype(graph),o::Key(u"value"),o::Descriptor::access({},setter));

        const auto instance=graph.context.construct(probe.constructor);
        expect(eq(*probe.initializers,std::size_t(1)));
        expect(eq(setterCalls,std::size_t(1)));
        expect(graph.context.get(instance,o::Key(u"observed"))==o::Value(29.0));
        expect(!graph.context.hasOwn(instance,o::Key(u"value")));
        expect(probe.owners->size()==1&&(*probe.owners)[0]->value==o::Value(7.0));
        expect((*probe.owners)[0]->writes==0);
    };

    "source_stack_instance_fields_rejecting_inherited_setter_does_not_write_owner"_test=[] {
        Graph graph;
        auto probe=registerProbe(graph);
        std::size_t setterCalls=0;
        const auto setter=graph.context.function(u"set value",[&setterCalls](auto&,auto,std::span<const o::Value>)->o::Value{
            ++setterCalls;throw o::Exception("TypeError",u"rejected by inherited setter");
        });
        graph.context.define(baseItemPrototype(graph),o::Key(u"value"),o::Descriptor::access({},setter));

        bool rejected=false;
        try{(void)graph.context.construct(probe.constructor);}
        catch(const o::Exception& error){rejected=error.name=="TypeError";}
        expect(rejected);
        expect(eq(*probe.initializers,std::size_t(1)));
        expect(eq(setterCalls,std::size_t(1)));
        expect(probe.owners->size()==1&&(*probe.owners)[0]->value==o::Value(7.0));
        expect((*probe.owners)[0]->writes==0);
    };

    "source_stack_instance_fields_reused_receiver_obeys_own_nonwritable_descriptor"_test=[] {
        Graph graph;
        auto probe=registerProbe(graph);
        const auto instance=graph.context.construct(probe.constructor);
        auto& field=*(*probe.owners)[0];
        expect(eq(field.writes,std::size_t(1)));
        expect(field.value==o::Value(29.0));

        graph.context.define(instance,o::Key(u"value"),o::Descriptor::data(o::Value(97.0),false,true,true));
        field.writes=0;
        const auto initializersBefore=*probe.initializers;
        bool rejected=false;
        try{(void)graph.context.call(probe.constructor,o::Value(instance));}
        catch(const o::Exception& error){rejected=error.name=="TypeError";}
        expect(rejected);
        expect(eq(*probe.initializers,initializersBefore+1));
        expect(eq(field.writes,std::size_t(0)));
        expect(field.value==o::Value(97.0));
        expect(graph.context.get(instance,o::Key(u"value"))==o::Value(97.0));
    };
};
}
