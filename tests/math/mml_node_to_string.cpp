#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_nodes.mrow;
namespace {
using namespace boost::ut;
namespace o=folia::math::options; namespace m=folia::math::mml; namespace t=folia::math::tex;
struct Graph {
    o::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::regexp_host::RegExpBindings regexp{context};
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input{context};
    folia::math::core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    t::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
};
suite mmlNodeToString=[] {
    "mml_node_toString_metadata_and_output_match_source"_test=[] {
        Graph graph;
        auto factory=std::make_shared<m::MmlFactory>();
        (void)graph.runtime.bindMmlFactory(factory);
        auto* child=factory->create("mi");
        auto children=std::make_shared<folia::math::tree::ChildArray<m::MmlNode>>();
        children->push_back(child);
        auto* row=factory->create("mrow",folia::math::PropertyList{},children);
        const auto identity=graph.runtime.mmlNodeIdentities.find(row);
        expect(identity.has_value());
        if(!identity)return;
        const auto prototype=graph.context.prototype(*identity);
        expect(prototype.has_value());
        if(!prototype)return;
        auto owner=*prototype;
        auto descriptor=graph.context.descriptor(o::Value(owner),o::Key(u"toString"));
        while(!descriptor){
            const auto parent=graph.context.prototype(owner);
            if(!parent)break;
            owner=*parent;
            descriptor=graph.context.descriptor(o::Value(owner),o::Key(u"toString"));
        }
        expect(descriptor.has_value());
        if(!descriptor)return;
        expect(!descriptor->accessor&&descriptor->enumerable&&descriptor->writable&&descriptor->configurable);
        const auto function=std::get_if<o::ObjectRef>(&descriptor->value.data);
        expect(function!=nullptr);
        if(!function)return;
        expect(graph.context.get(o::Value(*function),o::Key(u"name"))==o::Value(u""));
        expect(graph.context.get(o::Value(*function),o::Key(u"length"))==o::Value(0));
        const auto actual=graph.context.call(*function,o::Value(*identity));
        const auto native=row->toString();
        expect(actual==o::Value(std::u16string(native.begin(),native.end())));
        expect(actual==o::Value(u"mrow(mi())"));
    };
};
}
