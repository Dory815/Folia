#include "support/folia_test.hpp"
import folia.math.input.tex.tex_parser_context;
import folia.math.util.options;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t = folia::math::tex;
namespace o = folia::math::options;

struct Parser final : t::ParserContext {
  Property currentCS() const override { return Missing{}; }
  void setCurrentCS(Property) override {}
};

suite parseCallableSourceReceiver = [] {
  "parse_callable_preserves_source_this_values"_test = [] {
    Parser parser;
    o::Context context;
    const o::Value methodsObject(context.object());
    const o::Value compatibleThis(17.0);
    std::vector<FunctionReceiver> observed;
    const auto callable = t::makeParseCallable([&](const FunctionReceiver &receiver,
                                                t::ParserContext &,
                                                const t::ParseAtom &,
                                                t::ParseRest) -> t::ParseResult {
      observed.push_back(receiver);
      return Property("ok");
    });
    const t::ParseAtom atom{Property("Insert")};
    const std::array<std::any, 2> args{std::any(std::ref(static_cast<t::ParserContext &>(parser))),
                                       std::any(atom)};

    const auto methodCall = callable->call(FunctionReceiver{methodsObject}, args);
    expect(std::any_cast<Property>(methodCall) == Property("ok"));
    expect(observed.size() == 1u);
    expect(observed[0] == FunctionReceiver{methodsObject});

    const auto compatibleCall = callable->call(FunctionReceiver{compatibleThis}, args);
    expect(std::any_cast<Property>(compatibleCall) == Property("ok"));
    expect(observed.size() == 2u);
    expect(observed[1] == FunctionReceiver{compatibleThis});

    const auto undefinedCall = callable->call(FunctionReceiver{Missing{}}, args);
    expect(std::any_cast<Property>(undefinedCall) == Property("ok"));
    expect(std::holds_alternative<Missing>(observed[2]));

    const NativeObjectRef nativeReceiver = std::make_shared<NativeObject>();
    const auto nativeCall = callable->call(FunctionReceiver{nativeReceiver}, args);
    expect(std::any_cast<Property>(nativeCall) == Property("ok"));
    expect(observed.size() == 4u);
    const auto *preservedNative = std::get_if<NativeObjectRef>(&observed[3]);
    expect(preservedNative && *preservedNative == nativeReceiver);
  };
};
}
