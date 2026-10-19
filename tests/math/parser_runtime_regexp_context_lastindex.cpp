#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;

namespace {
using namespace boost::ut;
namespace host=folia::math::regexp_host;
namespace tex=folia::math::tex;
namespace opts=folia::math::options;
using folia::math::OptionFunctionBindings;
using folia::math::Property;
using folia::math::Unicode16Normalization;

struct Graph {
  opts::Context context;
  OptionFunctionBindings functions{context};
  host::RegExpBindings regexp{context};
  Unicode16Normalization unicode;
  folia::math::core::InputJaxModule input{context};
  folia::math::core::FindMathModule findMath{context};
  tex::FindTeXModule findTeX{context,findMath,regexp};
  tex::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
  Graph(){runtime.initializeParserRuntime();}
};

suite parser_runtime_regexp_context_lastindex=[] {
  "math_parser_runtime_regexp_context_test_uses_live_raw_lastindex_through_alias"_test=[] {
    Graph graph;
    const auto regexp=graph.regexp.create(u"x",u"y");
    const auto holder=graph.context.object();
    graph.context.set(holder,opts::Key(u"regexp"),regexp);
    const auto copied=opts::copy(graph.context,holder);
    const auto alias=graph.context.get(copied,opts::Key(u"regexp"));
    expect(alias==regexp);
    const auto source=graph.runtime.makeRegExpContext(alias);
    expect(graph.runtime.regexpContextValue(source)==alias);

    graph.context.set(alias,opts::Key(u"lastIndex"),opts::Value(u"1"));
    expect(source->test(Property("ax")));
    const auto afterString=graph.context.descriptor(alias,opts::Key(u"lastIndex"));
    expect(afterString&&afterString->value==opts::Value(2));

    int conversions=0;
    const auto valueOf=graph.context.function(u"valueOf",[&](opts::Context&,opts::Value,std::span<const opts::Value>){
      ++conversions;return opts::Value(1);
    });
    const auto raw=graph.context.object();
    graph.context.set(raw,opts::Key(u"valueOf"),opts::Value(valueOf));
    graph.context.set(alias,opts::Key(u"lastIndex"),opts::Value(raw));
    expect(source->test(Property("ax")));
    expect(conversions==1);
    const auto afterObject=graph.context.descriptor(alias,opts::Key(u"lastIndex"));
    expect(afterObject&&afterObject->value==opts::Value(2));
  };
};
}
