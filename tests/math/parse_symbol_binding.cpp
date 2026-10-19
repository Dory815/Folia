#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.map_handler;
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
};
suite parseSymbolBindings=[] {
    "math_parse_symbol_bridge_retains_actual_fallback_identity"_test=[] {
        Graph graph;
        const auto fallback=std::get<t::ParseSymbolRef>(t::SubHandler::FALLBACK);
        const auto bound=graph.runtime.bindParseSymbol(fallback);
        expect(bound==graph.runtime.bindParseSymbol(fallback));
        expect(bound!=graph.context.symbol(u"fallback"));
        const auto value=graph.runtime.properties.from(o::Value(bound));
        expect(graph.runtime.properties.to(value)==o::Value(bound));
        expect(std::holds_alternative<o::SymbolRef>(value.value));
    };
    "math_parse_symbol_bridge_does_not_merge_same_description_refs"_test=[] {
        Graph graph;
        auto first=std::make_shared<const t::ParseSymbol>(t::ParseSymbol{u"fallback"});
        auto second=std::make_shared<const t::ParseSymbol>(t::ParseSymbol{u"fallback"});
        const auto one=graph.runtime.bindParseSymbol(first);
        const auto two=graph.runtime.bindParseSymbol(second);
        expect(one!=two);
        expect(one==graph.runtime.bindParseSymbol(first));
        first.reset();
        expect(graph.context.symbolString(one).compare(graph.context.symbolString(two))==0);
    };
    "math_parse_symbol_bridge_respects_context_ownership_and_rejects_null_ref"_test=[] {
        Graph first,second;
        const auto fallback=std::get<t::ParseSymbolRef>(t::SubHandler::FALLBACK);
        const auto one=first.runtime.bindParseSymbol(fallback);
        const auto two=second.runtime.bindParseSymbol(fallback);
        expect(one!=two);
        bool foreign=false;
        try{(void)second.context.symbolString(one);}catch(const o::Exception& e){foreign=e.name=="TypeError";}
        expect(foreign);
        bool nullRef=false;
        try{(void)first.runtime.bindParseSymbol({});}catch(const o::Exception& e){nullRef=e.name=="TypeError"&&e.message.compare(u"Native ParseSymbol reference is null")==0;}
        expect(nullRef);
    };
};
}
