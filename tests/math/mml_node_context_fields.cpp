#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.util.option_functions;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.core.mml_tree.mml_nodes.mrow;
namespace {
using namespace boost::ut;
namespace t=folia::math::tex; namespace o=folia::math::options; namespace m=folia::math::mml;
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
suite mmlNodeContextFields=[] {
    "inferred_mrow_uses_pinned_own_key_order_and_actual_factory_identity"_test=[] {
        Graph graph;
        auto factory=std::make_shared<m::MmlFactory>();
        const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
        auto* row=dynamic_cast<m::MmlMrow*>(factory->create("inferredMrow"));
        expect(row!=nullptr);
        if(!row)return;
        const auto identity=graph.runtime.mmlNodeIdentities.find(row);
        expect(identity.has_value());
        if(!identity)return;
        const std::vector<std::u16string> expected={u"factory",u"parent",u"properties",u"childNodes",
            u"prevClass",u"prevLevel",u"texclass",u"attributes",u"_core"};
        expect(graph.context.enumerableStringKeys(*identity)==expected);
        expect(graph.context.get(*identity,u"factory")==o::Value(factoryIdentity));
        expect(graph.context.get(*identity,u"_core")==o::Value(o::Null{}));
        const auto nodePrototype=graph.context.prototype(*identity);
        expect(nodePrototype.has_value());
        if(nodePrototype){
            const auto kindDescriptor=graph.context.descriptor(o::Value(*nodePrototype),o::Key(u"kind"));
            expect(kindDescriptor.has_value());
            if(kindDescriptor){
                expect(kindDescriptor->accessor);
                expect(!kindDescriptor->enumerable&&kindDescriptor->configurable);
                if(kindDescriptor->getter){
                    expect(graph.context.get(o::Value(*kindDescriptor->getter),o::Key(u"name"))==o::Value(u"get"));
                    expect(graph.context.get(o::Value(*kindDescriptor->getter),o::Key(u"length"))==o::Value(0));
                }
            }
        }
        expect(graph.context.get(*identity,u"kind")==o::Value(u"inferredMrow"));
    };
    "child_nodes_context_alias_tracks_its_actual_child_array_owner"_test=[] {
        Graph graph;
        auto factory=std::make_shared<m::MmlFactory>();
        (void)graph.runtime.bindMmlFactory(factory);
        auto* row=factory->create("inferredMrow");
        auto* child=factory->create("mi");
        const auto identity=graph.runtime.mmlNodeIdentities.find(row);
        expect(identity.has_value());
        if(!identity)return;
        const auto oldArray=std::get<o::ObjectRef>(graph.context.get(*identity,u"childNodes").data);
        row->childNodes->push_back(m::NodeValue(child));
        expect(eq(graph.context.arrayLength(oldArray),std::uint32_t(1)));
        const auto replacement=std::make_shared<folia::math::tree::ChildArray<m::MmlNode>>();
        const auto replacementIdentity=t::bindNodeArray(graph.runtime,replacement);
        graph.context.set(*identity,u"childNodes",o::Value(replacementIdentity));
        expect(row->childNodes==replacement);
        expect(eq(graph.context.arrayLength(oldArray),std::uint32_t(1)));
        replacement->push_back(m::NodeValue(child));
        expect(eq(graph.context.arrayLength(replacementIdentity),std::uint32_t(1)));
    };
    "text_node_owns_and_exposes_its_actual_text_field_after_empty_node_fields"_test=[] {
        Graph graph;
        auto factory=std::make_shared<m::MmlFactory>();
        (void)graph.runtime.bindMmlFactory(factory);
        auto* text=dynamic_cast<m::TextNode*>(factory->create("text"));
        expect(text!=nullptr);
        if(!text)return;
        const auto identity=graph.runtime.mmlNodeIdentities.find(text);
        expect(identity.has_value());
        if(!identity)return;
        const std::vector<std::u16string> expected={u"factory",u"parent",u"properties",u"childNodes",u"text"};
        expect(graph.context.enumerableStringKeys(*identity)==expected);
        expect(graph.context.get(*identity,u"text")==o::Value(u""));
        graph.context.set(*identity,u"text",o::Value(u"through Context"));
        expect(text->getText()==u"through Context");
        (void)text->setText(u"through actual setter");
        expect(graph.context.get(*identity,u"text")==o::Value(u"through actual setter"));
    };
    "raw_mml_fields_and_mrow_core_remain_on_the_actual_node_owner"_test=[] {
        Graph graph;
        auto factory=std::make_shared<m::MmlFactory>();
        (void)graph.runtime.bindMmlFactory(factory);
        auto* row=dynamic_cast<m::MmlMrow*>(factory->create("inferredMrow"));
        auto* op=factory->create("mo");
        expect(row!=nullptr);
        if(!row)return;
        const auto identity=graph.runtime.mmlNodeIdentities.find(row);
        expect(identity.has_value());
        if(!identity)return;
        const auto properties=std::get<o::ObjectRef>(graph.context.get(*identity,u"properties").data);
        graph.context.set(properties,u"probe",o::Value(17));
        expect(row->getProperty("probe")==folia::math::Property(17));
        row->childNodes->push_back(m::NodeValue(op));
        op->setParent(m::NodeValue(row));
        expect(row->core().requireNode()==op);
        expect(graph.context.get(*identity,u"_core")==o::Value(0));
    };
    "mml_source_fields_preserve_replacement_child_arrays_and_factory_identity"_test=[] {
        Graph graph;
        auto factory=std::make_shared<m::MmlFactory>();
        const auto factoryIdentity=graph.runtime.bindMmlFactory(factory);
        auto* row=factory->create("inferredMrow");
        const auto identity=graph.runtime.mmlNodeIdentities.find(row);
        expect(identity.has_value());
        if(!identity)return;
        const auto children=graph.context.array();
        graph.context.set(*identity,u"childNodes",o::Value(children));
        expect(graph.context.get(*identity,u"childNodes")==o::Value(children));
        auto other=std::make_shared<m::MmlFactory>();
        const auto otherIdentity=graph.runtime.bindMmlFactory(other);
        graph.context.set(*identity,u"factory",o::Value(otherIdentity));
        expect(graph.context.get(*identity,u"factory")==o::Value(otherIdentity));
        expect(otherIdentity!=factoryIdentity);
    };
};
}
