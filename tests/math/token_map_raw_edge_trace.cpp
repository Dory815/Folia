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
suite token_map_raw_edge_trace=[] {
"math_port_context_raw_trace_follows_persistent_macro_lookup_owner_edges"_test=[] {
    Graph graph;
    auto sourceMethod=std::make_shared<Function>();
    sourceMethod->call=[](const FunctionReceiver&,FunctionArgs)->std::any{return Property(Missing{});};
    const Property functionMap(PropertyList{{"macro body",Property(sourceMethod)}});
    const Property macroTable(PropertyList{{"x",Property(PropertyArray{Property("macro body"),Property("argument")})}});
    auto map=t::TokenMapHost::create<t::MacroMap>(graph.runtime.mapHandler,
        Property("lifetime-macro-map"),macroTable,functionMap);
    const auto firstMacro=std::get<t::MacroRef>(map->lookup(Property("x")));
    const auto oldIdentity=graph.runtime.tokenMaps.bind(firstMacro);
    const auto mapIdentity=graph.runtime.tokenMaps.bind(map);
    const std::array roots{mapIdentity};
    const auto before=graph.context.traceReachable(roots);
    expect(contains(before.objects(),oldIdentity));

    const auto secondMacro=std::make_shared<t::Macro>(Property("x"),t::ParseMethod{Property(PropertyNull{})});
    map->add(Property("x"),t::TokenLookup{secondMacro});
    const auto newIdentity=graph.runtime.tokenMaps.bind(secondMacro);
    const auto after=graph.context.traceReachable(roots);
    expect(contains(after.objects(),newIdentity));expect(!contains(after.objects(),oldIdentity));
    // Macro fields with opaque/native payloads are deliberately pinned by the
    // base Context trace. This test proves the map's raw lookup edges only.
    expect(after.hasOpaquePins());
};
"math_port_raw_macro_token_fields_visit_symbol_edges_and_pin_unknown_functions"_test=[] {
    Graph graph;
    const auto tokenSymbol=graph.context.symbol(u"token-edge");
    const auto argsSymbol=graph.context.symbol(u"args-edge");
    auto token=std::make_shared<t::Token>(Property(tokenSymbol),Property("x"));
    const auto tokenIdentity=graph.runtime.tokenMaps.bind(token);
    const std::array tokenRoots{tokenIdentity};
    const auto tokenTrace=graph.context.traceReachable(tokenRoots);
    expect(std::find(tokenTrace.symbols().begin(),tokenTrace.symbols().end(),tokenSymbol)!=tokenTrace.symbols().end());

    auto args=PropertyArray{};args.push(Property(argsSymbol));
    int callbackCalls=0;
    auto callback=std::make_shared<Function>();
    callback->call=[&callbackCalls](const FunctionReceiver&,FunctionArgs)->std::any{++callbackCalls;return Property(Missing{});};
    auto macro=std::make_shared<t::Macro>(Property(tokenSymbol),
        t::ParseMethod{callback},Property(args));
    const auto macroIdentity=graph.runtime.tokenMaps.bind(macro);
    const std::array macroRoots{macroIdentity};
    const auto macroTrace=graph.context.traceReachable(macroRoots);
    expect(std::find(macroTrace.symbols().begin(),macroTrace.symbols().end(),tokenSymbol)!=macroTrace.symbols().end());
    expect(std::find(macroTrace.symbols().begin(),macroTrace.symbols().end(),argsSymbol)!=macroTrace.symbols().end());
    expect(macroTrace.hasOpaquePins()); // FunctionRef captures are never invoked or assumed empty.
    expect(eq(callbackCalls,0));
};
};
}
