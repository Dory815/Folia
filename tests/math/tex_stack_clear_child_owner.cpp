#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.node_factory_host;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.mml_tree.mml_node;
import folia.math.input.tex.find_tex_host;
import folia.math.util.native_identities;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t = folia::math::tex;
namespace o = folia::math::options;
namespace m = folia::math::mml;

struct Graph {
    o::Context context;
    OptionFunctionBindings functions{context};
    regexp_host::RegExpBindings regexp{context};
    Unicode16Normalization unicode;
    core::InputJaxModule input{context};
    core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context, findMath, regexp};
    t::ParserRuntime runtime{context, functions, regexp, unicode, input, findTeX};
    Graph() { runtime.initializeParserRuntime(); }
};

o::ObjectRef asObject(const o::Value& value) { return std::get<o::ObjectRef>(value.data); }

suite g183ArrayChildSlot = [] {
    "stack_clear_rebinds_a_live_context_array_to_the_native_child_owner"_test = [] {
        Graph graph;
        auto& cx = graph.context;
        auto& runtime = graph.runtime;
        const o::Value baseItemValue = cx.get(runtime.stackItemModule, o::Key(u"BaseItem"));
        const o::ObjectRef item = cx.construct(asObject(baseItemValue));
        const o::Value oldNodesValue = cx.get(o::Value(item), o::Key(u"_nodes"));
        const o::ObjectRef oldNodes = asObject(oldNodesValue);
        const auto oldChildren = runtime.nodeArrayIdentities.resolve(oldNodes);

        const o::Value clear = cx.get(o::Value(item), o::Key(u"Clear"));
        (void)cx.callMember(clear, o::Value(item), {});
        const o::Value currentNodesValue = cx.get(o::Value(item), o::Key(u"_nodes"));
        const o::ObjectRef currentNodes = asObject(currentNodesValue);
        expect(currentNodes != oldNodes);
        expect(cx.kind(currentNodes) == o::ObjectKind::array);
        expect(eq(cx.arrayLength(currentNodes), std::uint32_t(0)));

        const auto currentChildren = runtime.nodeArrayIdentities.resolve(currentNodes);
        expect(currentChildren != oldChildren);
        expect(eq(currentChildren->size(), std::size_t(0)));
        const auto& payload = std::as_const(cx).payload(currentNodes);
        const auto* binding = std::any_cast<NativeBindingPayload<m::Children>>(&payload);
        expect(binding != nullptr);
        expect(binding && binding->identity == currentChildren);

        std::array<std::any, 1> convertedArguments{std::any(currentChildren)};
        const auto accepted = t::node_factory_host::children(convertedArguments, 0);
        expect(accepted == currentChildren);

        const o::Value push = cx.get(currentNodesValue, o::Key(u"push"));
        const std::array<o::Value, 1> pushArgs{o::Value(o::Null{})};
        (void)cx.callMember(push, currentNodesValue, pushArgs);
        expect(eq(cx.arrayLength(currentNodes), std::uint32_t(1)));
        expect(currentChildren->get(0).kind() == tree::ChildKind::Null);
        expect(eq(oldChildren->size(), std::size_t(0)));

        (void)cx.callMember(clear, o::Value(item), {});
        const o::ObjectRef afterSecondClear = asObject(cx.get(o::Value(item), o::Key(u"_nodes")));
        const auto secondChildren = runtime.nodeArrayIdentities.resolve(afterSecondClear);
        expect(afterSecondClear != currentNodes);
        expect(secondChildren != currentChildren);
        expect(eq(secondChildren->size(), std::size_t(0)));
        expect(eq(currentChildren->size(), std::size_t(1)));
    };
};
}
