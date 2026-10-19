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
bool contains(std::span<const o::ObjectRef> values,o::ObjectRef value){
    return std::find(values.begin(),values.end(),value)!=values.end();
}
suite tags_raw_edge_trace=[] {
"math_port_context_raw_trace_follows_real_persistent_tags_labels_ids_and_history"_test=[] {
    Graph graph;
    const auto tags=graph.runtime.tagsFactory.getDefault();
    const auto identity=tags->sourceIdentity();
    const std::array roots{identity};
    auto report=graph.context.traceReachable(roots);
    for(const auto field:{u"labels",u"allLabels",u"ids",u"allIds",u"history",u"stack",u"currentTag"}){
        const auto value=graph.context.get(identity,field);
        const auto* object=std::get_if<o::ObjectRef>(&value.data);
        expect(object!=nullptr);
        if(object)expect(contains(report.objects(),*object));
    }
    const auto allLabelsValue=graph.context.get(identity,u"allLabels");
    const auto* allLabelsObject=std::get_if<o::ObjectRef>(&allLabelsValue.data);
    expect(allLabelsObject!=nullptr);
    if(!allLabelsObject)return;
    const auto allLabels=*allLabelsObject;
    const auto oldLabel=graph.context.object(),newLabel=graph.context.object();
    graph.context.set(allLabels,o::Key(u"chapter"),o::Value(oldLabel));
    const auto withOldLabel=graph.context.traceReachable(roots);
    expect(contains(withOldLabel.objects(),oldLabel));
    graph.context.set(allLabels,o::Key(u"chapter"),o::Value(newLabel));
    const auto withNewLabel=graph.context.traceReachable(roots);
    expect(contains(withNewLabel.objects(),newLabel));expect(!contains(withNewLabel.objects(),oldLabel));
    // The native Tags payload remains opaque/pinned in this increment. The
    // context-owned labels/ids/history graph is still enumerated raw.
    expect(withNewLabel.hasOpaquePins());
};
};
}
