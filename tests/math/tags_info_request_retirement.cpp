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
o::ObjectRef createTagInfo(Graph& graph){
    const auto* constructor=std::get_if<o::ObjectRef>(&graph.runtime.tagInfoClass.data);
    if(!constructor)throw std::runtime_error("TagInfo constructor was not initialized");
    return graph.context.construct(*constructor);
}
o::ObjectRef createLabel(Graph& graph){
    const auto* constructor=std::get_if<o::ObjectRef>(&graph.runtime.labelClass.data);
    if(!constructor)throw std::runtime_error("Label constructor was not initialized");
    return graph.context.construct(*constructor);
}
bool isRetired(Graph& graph,o::ObjectRef identity){
    try{(void)graph.runtime.tagInfoIdentities.resolve(identity);return false;}
    catch(const o::Exception&){return true;}
}
bool isLabelRetired(Graph& graph,o::ObjectRef identity){
    try{(void)graph.runtime.labelIdentities.resolve(identity);return false;}
    catch(const o::Exception&){return true;}
}
}
suite tagInfoRequestRetirement=[] {
    "math_private_renderer_tag_info_retirement_keeps_current_history_and_stack_roots"_test=[] {
        Graph graph;
        const auto tags=graph.runtime.tagsFactory.create(u"none");
        const auto tagsIdentity=tags->sourceIdentity();
        const auto initial=std::get<o::ObjectRef>(graph.context.get(tagsIdentity,o::Key(u"currentTag")).data);
        const auto current=createTagInfo(graph);
        const auto historyEntry=createTagInfo(graph);
        const auto stackEntry=createTagInfo(graph);
        const auto orphan=createTagInfo(graph);
        graph.context.set(tagsIdentity,o::Key(u"currentTag"),o::Value(current));
        const auto history=graph.context.array();
        const std::array<o::Value,1> historyValues{o::Value(historyEntry)};
        graph.context.arrayPush(history,historyValues);
        graph.context.set(tagsIdentity,o::Key(u"history"),o::Value(history));
        const auto stack=graph.context.array();
        const std::array<o::Value,1> stackValues{o::Value(stackEntry)};
        graph.context.arrayPush(stack,stackValues);
        graph.context.set(tagsIdentity,o::Key(u"stack"),o::Value(stack));

        const auto retired=graph.runtime.retireTagInfoOwnersOutsidePrivateRendererRoots();
        expect(eq(retired,std::size_t(2)));
        expect(eq(graph.runtime.tagInfoIdentities.canonicalHandleCount(),std::size_t(3)));
        expect(isRetired(graph,initial));
        expect(isRetired(graph,orphan));
        expect(!isRetired(graph,current));
        expect(!isRetired(graph,historyEntry));
        expect(!isRetired(graph,stackEntry));

        graph.context.set(tagsIdentity,o::Key(u"history"),o::Value(graph.context.array()));
        graph.context.set(tagsIdentity,o::Key(u"stack"),o::Value(graph.context.array()));
        const auto retiredAfterReplacement=graph.runtime.retireTagInfoOwnersOutsidePrivateRendererRoots();
        expect(eq(retiredAfterReplacement,std::size_t(2)));
        expect(eq(graph.runtime.tagInfoIdentities.canonicalHandleCount(),std::size_t(1)));
        expect(!isRetired(graph,current));
        expect(isRetired(graph,historyEntry));
        expect(isRetired(graph,stackEntry));
    };
    "math_private_renderer_tag_info_retirement_does_not_call_tags_accessors"_test=[] {
        Graph graph;
        const auto tags=graph.runtime.tagsFactory.create(u"none");
        const auto identity=tags->sourceIdentity();
        const auto retained=createTagInfo(graph);
        graph.context.set(identity,o::Key(u"currentTag"),o::Value(retained));
        int getterCalls=0;
        const auto getter=graph.context.function(u"history getter",[&](o::Context&,o::Value,std::span<const o::Value>){
            ++getterCalls;return o::Value(graph.context.array());
        });
        graph.context.define(identity,o::Key(u"history"),o::Descriptor::access(getter,std::nullopt));
        const auto before=graph.runtime.tagInfoIdentities.canonicalHandleCount();
        expect(eq(graph.runtime.retireTagInfoOwnersOutsidePrivateRendererRoots(),std::size_t(0)));
        expect(eq(getterCalls,0));
        expect(eq(graph.runtime.tagInfoIdentities.canonicalHandleCount(),before));
        expect(!isRetired(graph,retained));
    };
    "math_private_renderer_tag_info_retirement_follows_inherited_array_data_without_getters"_test=[] {
        Graph graph;
        const auto tags=graph.runtime.tagsFactory.create(u"none");
        const auto tagsIdentity=tags->sourceIdentity();
        const auto current=createTagInfo(graph);
        const auto inherited=createTagInfo(graph);
        const auto orphan=createTagInfo(graph);
        graph.context.set(tagsIdentity,o::Key(u"currentTag"),o::Value(current));
        const auto history=graph.context.array();
        graph.context.set(history,o::Key(u"length"),o::Value(1));
        const auto prototype=graph.context.objectWithPrototype(graph.context.intrinsics().arrayPrototype);
        graph.context.define(prototype,o::Key(u"0"),o::Descriptor::data(o::Value(inherited)));
        graph.context.setPrototype(history,prototype);
        graph.context.set(tagsIdentity,o::Key(u"history"),o::Value(history));

        expect(eq(graph.runtime.retireTagInfoOwnersOutsidePrivateRendererRoots(),std::size_t(2)));
        expect(!isRetired(graph,current));
        expect(!isRetired(graph,inherited));
        expect(isRetired(graph,orphan));
    };
    "math_private_renderer_tag_info_retirement_fails_closed_on_inherited_array_getter"_test=[] {
        Graph graph;
        const auto tags=graph.runtime.tagsFactory.create(u"none");
        const auto tagsIdentity=tags->sourceIdentity();
        const auto current=createTagInfo(graph);
        const auto inherited=createTagInfo(graph);
        graph.context.set(tagsIdentity,o::Key(u"currentTag"),o::Value(current));
        const auto history=graph.context.array();
        graph.context.set(history,o::Key(u"length"),o::Value(1));
        const auto prototype=graph.context.objectWithPrototype(graph.context.intrinsics().arrayPrototype);
        int getterCalls=0;
        const auto getter=graph.context.function(u"inherited history getter",[&](o::Context&,o::Value,std::span<const o::Value>){
            ++getterCalls;return o::Value(inherited);
        });
        graph.context.define(prototype,o::Key(u"0"),o::Descriptor::access(getter,std::nullopt));
        graph.context.setPrototype(history,prototype);
        graph.context.set(tagsIdentity,o::Key(u"history"),o::Value(history));
        const auto before=graph.runtime.tagInfoIdentities.canonicalHandleCount();

        expect(eq(graph.runtime.retireTagInfoOwnersOutsidePrivateRendererRoots(),std::size_t(0)));
        expect(eq(getterCalls,0));
        expect(eq(graph.runtime.tagInfoIdentities.canonicalHandleCount(),before));
        expect(!isRetired(graph,current));
        expect(!isRetired(graph,inherited));
    };
    "math_private_renderer_label_retirement_keeps_current_and_completed_label_roots"_test=[] {
        Graph graph;
        const auto tags=graph.runtime.tagsFactory.create(u"none");
        const auto identity=tags->sourceIdentity();
        const auto labels=std::get<o::ObjectRef>(graph.context.get(identity,o::Key(u"labels")).data);
        const auto allLabels=std::get<o::ObjectRef>(graph.context.get(identity,o::Key(u"allLabels")).data);
        const auto placeholder=createLabel(graph);
        const auto durable=createLabel(graph);
        const auto current=createLabel(graph);
        const auto orphan=createLabel(graph);
        graph.context.set(labels,o::Key(u"eq:a"),o::Value(durable));
        graph.context.set(allLabels,o::Key(u"eq:a"),o::Value(durable));
        graph.context.set(labels,o::Key(u"eq:current"),o::Value(current));

        expect(eq(graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots(),std::size_t(2)));
        expect(isLabelRetired(graph,placeholder));
        expect(isLabelRetired(graph,orphan));
        expect(!isLabelRetired(graph,durable));
        expect(!isLabelRetired(graph,current));

        graph.context.set(identity,o::Key(u"labels"),o::Value(graph.context.object()));
        expect(eq(graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots(),std::size_t(1)));
        expect(isLabelRetired(graph,current));
        expect(!isLabelRetired(graph,durable));
    };
    "math_private_renderer_label_retirement_fails_closed_without_calling_map_getters"_test=[] {
        Graph graph;
        const auto tags=graph.runtime.tagsFactory.create(u"none");
        const auto identity=tags->sourceIdentity();
        const auto label=createLabel(graph);
        const auto labels=std::get<o::ObjectRef>(graph.context.get(identity,o::Key(u"labels")).data);
        int getterCalls=0;
        const auto getter=graph.context.function(u"label map getter",[&](o::Context&,o::Value,std::span<const o::Value>){
            ++getterCalls;return o::Value(label);
        });
        graph.context.define(labels,o::Key(u"label"),o::Descriptor::access(getter,std::nullopt));
        const auto before=graph.runtime.labelIdentities.canonicalHandleCount();
        expect(eq(graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots(),std::size_t(0)));
        expect(eq(getterCalls,0));
        expect(eq(graph.runtime.labelIdentities.canonicalHandleCount(),before));
        expect(!isLabelRetired(graph,label));
    };
    "math_private_renderer_label_retirement_retains_inherited_data_roots"_test=[] {
        Graph graph;
        const auto tags=graph.runtime.tagsFactory.create(u"none");
        const auto identity=tags->sourceIdentity();
        const auto inherited=createLabel(graph);
        const auto orphan=createLabel(graph);
        const auto prototype=graph.context.objectWithPrototype(graph.context.intrinsics().objectPrototype);
        graph.context.define(prototype,o::Key(u"label"),o::Descriptor::data(o::Value(inherited)));
        const auto labels=graph.context.objectWithPrototype(prototype);
        graph.context.set(identity,o::Key(u"labels"),o::Value(labels));

        expect(eq(graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots(),std::size_t(1)));
        expect(!isLabelRetired(graph,inherited));
        expect(isLabelRetired(graph,orphan));
    };
    "math_private_renderer_label_retirement_fails_closed_on_inherited_accessor"_test=[] {
        Graph graph;
        const auto tags=graph.runtime.tagsFactory.create(u"none");
        const auto identity=tags->sourceIdentity();
        const auto retained=createLabel(graph);
        const auto labels=graph.context.objectWithPrototype(graph.context.intrinsics().objectPrototype);
        const auto prototype=graph.context.objectWithPrototype(graph.context.intrinsics().objectPrototype);
        int getterCalls=0;
        const auto getter=graph.context.function(u"inherited label map getter",[&](o::Context&,o::Value,std::span<const o::Value>){
            ++getterCalls;return o::Value(retained);
        });
        graph.context.define(prototype,o::Key(u"label"),o::Descriptor::access(getter,std::nullopt));
        graph.context.setPrototype(labels,prototype);
        graph.context.set(identity,o::Key(u"labels"),o::Value(labels));
        const auto before=graph.runtime.labelIdentities.canonicalHandleCount();

        expect(eq(graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots(),std::size_t(0)));
        expect(eq(getterCalls,0));
        expect(eq(graph.runtime.labelIdentities.canonicalHandleCount(),before));
        expect(!isLabelRetired(graph,retained));
    };
    "math_private_renderer_label_retirement_keeps_label_map_prototype_and_direct_root"_test=[] {
        Graph graph;
        const auto tags=graph.runtime.tagsFactory.create(u"none");
        const auto identity=tags->sourceIdentity();
        const auto prototypeLabel=createLabel(graph);
        const auto orphan=createLabel(graph);
        const auto labels=graph.context.objectWithPrototype(prototypeLabel);
        graph.context.set(identity,o::Key(u"labels"),o::Value(labels));

        (void)graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots();
        expect(!isLabelRetired(graph,prototypeLabel));
        expect(isLabelRetired(graph,orphan));

        const auto directRoot=createLabel(graph);
        graph.context.set(identity,o::Key(u"labels"),o::Value(directRoot));
        (void)graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots();
        expect(!isLabelRetired(graph,directRoot));
    };
    "math_private_renderer_label_retirement_keeps_modified_object_prototype_data_roots"_test=[] {
        Graph graph;
        (void)graph.runtime.tagsFactory.create(u"none");
        const auto inherited=createLabel(graph);
        const auto orphan=createLabel(graph);
        graph.context.define(graph.context.intrinsics().objectPrototype,o::Key(u"rendererLabelRoot"),
            o::Descriptor::data(o::Value(inherited)));

        expect(eq(graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots(),std::size_t(0)));
        expect(!isLabelRetired(graph,inherited));
        expect(!isLabelRetired(graph,orphan));
    };
    "math_private_renderer_label_retirement_skips_when_object_prototype_accessor_changes"_test=[] {
        Graph graph;
        (void)graph.runtime.tagsFactory.create(u"none");
        const auto inherited=createLabel(graph);
        const auto orphan=createLabel(graph);
        int getterCalls=0;
        const auto getter=graph.context.function(u"object prototype label getter",[&](o::Context&,o::Value,std::span<const o::Value>){
            ++getterCalls;return o::Value(inherited);
        });
        graph.context.define(graph.context.intrinsics().objectPrototype,o::Key(u"rendererLabelGetter"),
            o::Descriptor::access(getter,std::nullopt));

        expect(eq(graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots(),std::size_t(0)));
        expect(eq(getterCalls,0));
        expect(!isLabelRetired(graph,inherited));
        expect(!isLabelRetired(graph,orphan));
    };
    "math_private_renderer_label_retirement_skips_when_object_prototype_chain_changes_to_label"_test=[] {
        Graph graph;
        (void)graph.runtime.tagsFactory.create(u"none");
        const auto inherited=createLabel(graph);
        const auto orphan=createLabel(graph);
        graph.context.setPrototype(inherited,std::nullopt);
        graph.context.setPrototype(graph.context.intrinsics().objectPrototype,inherited);

        expect(eq(graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots(),std::size_t(0)));
        expect(!isLabelRetired(graph,inherited));
        expect(!isLabelRetired(graph,orphan));
    };
    "math_private_renderer_label_retirement_skips_when_object_prototype_chain_changes_to_data_object"_test=[] {
        Graph graph;
        (void)graph.runtime.tagsFactory.create(u"none");
        const auto inherited=createLabel(graph);
        const auto orphan=createLabel(graph);
        const auto parent=graph.context.nullObject();
        graph.context.set(parent,o::Key(u"inheritedLabel"),o::Value(inherited));
        graph.context.setPrototype(graph.context.intrinsics().objectPrototype,parent);

        expect(eq(graph.runtime.retireLabelOwnersOutsidePrivateRendererRoots(),std::size_t(0)));
        expect(!isLabelRetired(graph,inherited));
        expect(!isLabelRetired(graph,orphan));
    };
};
