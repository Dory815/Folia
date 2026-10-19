#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.util.function_list_bindings;
import folia.math.util.option_functions;
import folia.math.util.regexp_host;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o = folia::math::options;
namespace t = folia::math::tex;

suite parser_configuration_function_list_source_field = [] {
  "parser_configuration_function_list_is_source_instance_with_live_items"_test = [] {
    o::Context context;
    OptionFunctionBindings functions(context);
    regexp_host::RegExpBindings regexp(context);
    Unicode16Normalization unicode;
    core::InputJaxModule input(context);
    core::FindMathModule findMath(context);
    t::FindTeXModule findTeX(context, findMath, regexp);
    t::ParserRuntime runtime(context, functions, regexp, unicode, input, findTeX);
    runtime.initializeParserRuntime();

    const auto parserCtor = std::get<o::ObjectRef>(runtime.parserConfigurationClass.data);
    const std::array<o::Value, 1> parserArgs{o::Value(context.array())};
    const auto parser = context.construct(parserCtor, parserArgs);
    const auto listValue = context.get(o::Value(parser), o::Key(u"configMethod"));
    const auto list = std::get<o::ObjectRef>(listValue.data);
    expect(std::ranges::equal(context.ownKeys(listValue),std::array<o::Key,1>{o::Key(u"items")}));

    const auto oldArray = std::get<o::ObjectRef>(context.get(listValue, o::Key(u"items")).data);
    const auto add = context.get(listValue, o::Key(u"add"));
    std::vector<double> calls;
    auto nativeCallback = std::make_shared<Function>();
    nativeCallback->call = [&calls](const FunctionReceiver&, FunctionArgs) -> std::any {
      calls.push_back(4.0);
      return {};
    };
    OptionCallBinding callbackBinding{
      [](o::Context&, o::Value receiver) { return FunctionReceiver(receiver); },
      [](o::Context&, std::span<const o::Value>) { return std::vector<std::any>{}; },
      [](o::Context&, const std::any& result) -> o::Value {
        if (result.has_value()) throw std::logic_error("probe must return undefined");
        return o::Undefined{};
      }};
    const auto callback = functions.bind(nativeCallback, std::move(callbackBinding), u"probe");
    (void)context.callMember(add, listValue,
        std::array<o::Value, 2>{callback, o::Value(4.0)});
    expect(context.toNumber(context.get(oldArray, o::Key(u"length"))) == 1.0);
    const auto record = context.get(oldArray, o::Key(u"0"));
    expect(context.get(record, o::Key(u"item")) == o::Value(callback));
    expect(context.toNumber(context.get(record, o::Key(u"priority"))) == 4.0);

    const auto iteratorMethod = context.get(listValue, o::Key(context.intrinsics().symbolIterator));
    const auto iterator = context.callMember(iteratorMethod, listValue);
    const auto next = context.get(iterator, o::Key(u"next"));
    const auto step = context.callMember(next, o::Value(iterator));
    expect(context.get(step, o::Key(u"value")) == record);
    expect(!std::get<bool>(context.get(step, o::Key(u"done")).data));

    const auto execute = context.get(listValue, o::Key(u"execute"));
    expect(std::get<bool>(context.callMember(execute, listValue).data));
    expect(calls == std::vector<double>{4.0});

    const auto other = runtime.functionLists.bind(std::make_shared<FunctionList>());
    const auto replacement = context.get(o::Value(other), o::Key(u"items"));
    context.set(listValue, o::Key(u"items"), replacement);
    expect(context.get(listValue, o::Key(u"items")) == replacement);
    expect(context.toNumber(context.get(oldArray, o::Key(u"length"))) == 1.0);
    (void)list; // The public identity is the bound FunctionList owner itself.
  };
};
} // namespace
